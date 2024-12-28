/* The basic interpreter functions of REXX/imc    (C) Ian Collier 1992 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>

#include "const.h"
#include "rexxsaa.h"
#include "globals.h"
#include "functions.h"

static void doaddress(char**, int);                 // Address a command to an environment
static void parse(char**, int*, int, char*, int*);  // PARSE strings with a template
static char uc1(char, int);                         // Uppercase c, if up
static void pset1(char*, int, char*, int, int);     // Tokenise a string into variables
static void pset(char*, int, char*, int, int);      // Assign a PARSE result to a variable
static int findsigl(int*);                          // Find most recent line number
static void getcallargs(char**, int*, int);         // Unstack parameters in a CALL instruction
static void doconds(void);                          // Check for delayed conditions and trap them
static int gettrap(char**, int, int*);              // Get a trap name after "call/signal on"
static void testvarname(char**, char*, int);        // Test the symbol against a stored name
static void skipstmt(void);                         // Skip the current instruction
static void stepdo(void);                           // Step past the current DO
static void stepselect(void);                       // Step past the current SELECT
static void stepif(void);                           // Step past the current IF
static void stepwhen(void);                         // Step past the current WHEN
static void findend(void);                          // Find the next END

static program *oldprog = 0;    // while loading a new one
static int oldstmts = 0;        // old number of statements
static int tmpstack = 0;        // whether a temporary pstack item is present
static int haltline = 0;        // line number where halt occurred
static char *signalto;          // name of label to go to
static int ippc;                // statement number of INTERPRET
static int interpreting = 0;    // =1 while tokenising INTERPRETed data
extern char version[];          // version string (for parse)
extern char *psource;           // source string (for parse)

/*
 * when extra data has been found on the end of a clause,
 * the following distinguishes between "unexpected ')'
 * or ','" and "invalid data"
 */
#define Edata (c==')'||c==','?Erpar:Exend)

/* memory allocation. */

/* mtest and dtest (macros, except during debug) check that the given REXX
   structure is large enough; if not they try to extend it and die if realloc
   fails.  dtest returns 1 if the area moved and sets mtest_diff to the
   distance between the old and new pointers */
/* The non-debug version of mtest is now a macro; the debug version calls
   this function. Note that in
      mtest(memptr,alloc,length,extend)
   memptr and alloc are identifiers. length is an expression which is
   evaluated exactly once. extend is an expression which is evaluated
   zero or one times. In all other ways, mtest acts like a function.
*/

/* allocm(size) allocates "size" bytes of memory and returns the answer; it dies if malloc returns an error. */
char *allocm(unsigned size) {
  char *pointer;

  if ((pointer = malloc(size)) == cnull) {
    die(Emem);
  }
  return pointer;
}

/*
 * Error (exception) handler: cleans up, prints message,
 * and does all the usual things that happen at error time
 */
void die(int rc) {
  int i = 0, l = 0;
  int catch;        // whether the error is to be caught by SIGNAL ON xxx
  int lev;          // nesting level of interpreter() to longjmp to if catch!=0
  int bit;          // bit to test against "trap" flags in signal structure
  int stmt = 0;     // where to signal to if the error is caught
  char rcb[20];     // for printing the rc
  char *ptr;        // for traversing the program stack
  char *edata;      // saved copy of errordata
  int errline = 0;  // which line number to say the error occurred in
  int sigl = 0;     // what to set SIGL to

recurse:
  edata = errordata;
  if (prog) {
    errline = sigl = prog[ppc].num;
  }
  errordata = cnull;  // clear this now for next time; it has been saved
  // find out whether the error is to be caught
  switch (rc) {  // find out what trap has occurred, based on rc
    case Ehalt:      bit = Ihalt;      break;
    case Enovalue:   bit = Inovalue;   break;
    case Eerror:     bit = Ierror;     break;
    case Efailure:   bit = Ifailure;   break;
    case Enotready:  bit = Inotready;  break;
    //   Esys:       -- never catch 'failure in system service'
    //   Emem:       -- or 'out of memory' or 'OK'
    case 0:          bit = 0;          break;
    default:         bit = Isyntax;
  }
  // now check that bit against the signal stack
  catch = (interpreting || ppc > 0) && interplev >= 0 && (sgstack[interplev].bits & (1 << bit));
  if (catch) {  // catch > 0 if signal was on or inherited
    for (lev = interplev; !(sgstack[lev].bitson & (1 << bit)); lev--) {
      // find lev = level in which signal is caught
    }
    if (lev <= interact) {
      catch = 0;  // do not trap errors in interactive command
    }
  }
  // error has been trapped while tokenising for INTERPRET,
  // so silently get rid of the source string and point to the program instead
  if (catch && interpreting) {
    free(prog[0].source);
    free(prog[0].line);
    free((char *) prog);
    prog = oldprog;
    stmts = oldstmts;
    errline = sigl = prog[ppc = ippc].num;
    interpreting = 0;
  }
  if (catch) {
    stmt = sgstack[lev].ppc[bit];
    sgstack[lev].bits &= ~(1 << bit);   // turn off trapping immediately
    sgstack[lev].bitson &= ~(1 << bit);
    sgstack[interplev].bits &= ~(1 << bit);
    if (stmt < 0) {
      // if label not found, then see whether this is to be caught.
      // if not then display an appropriate message.
      // (if it is then it will be caught later).
      catch = bit != Isyntax && (sgstack[lev].bits & (1 << Isyntax));
      if (!catch) {
        // stack the trapped line so that it appears in the traceback
        ptr = (char *) pstack(20, sizeof(struct errorstack));
        ((struct errorstack *) ptr)->prg = prog;
        ((struct errorstack *) ptr)->stmts = stmts;
        ppc = -stmt;
        findsigl(&lev);  // find the SIGNAL ON instruction.
        if (bit == Inotready) {
          tracestr("      +++ ");
          tracestr(sigdata[bit]);
          tracestr(": ");
          tracestr(message(Eerrno + lasterror));
        } else {
          tracestr("      +++ ");
          tracestr(message(rc));
        }
        if (edata && rc == Enovalue) {
          tracestr(" on ");
        }
        if (edata) {
          tracestr(edata);
        }
        tracechar('\n');   // this writes an informative message about the trapped condition
        rc = Elabel;
        goto recurse;  // report the label not found error.
      }
    }
  } else {
    lev = 0;  // not caught: catch and lev are zero
  }
  trcresult = 0;  // not nested inside scanning()
  if (prog && !ppc && rc >= 0) {  // error occurred while tokenising
    // noisily get rid of the new program
    ptr = prog[stmts].source;  // the error occurred after this point
    if (!ptr) {  //  default: start of the last line encountered
      ptr = prog[stmts].source = source[lines];
      prog[stmts].num = lines;
    }
    errline = prog[stmts].num;
    for (i = 64; i-- && ptr[0] != '\n' && ptr[0]; ptr++) {
      // try and find a place to stop
    }
    if (i < 0) {
      ptr[-1] = ptr[-2] = ptr[-3] = '.';  // elipsis if not at end of line
    }
    prog[stmts++].sourcend = ptr;
    source[++lines] = ptr + 1;
    i = pstacklev;
    if (!interpreting) {
      pstacklev = 0;  // don't indent a program line
    } else {
      pstacklev++;  // do indent an interpret
    }
    printstmt(stmts - 1, 0, 1);
    pstacklev = i;
    if (interpreting) {   // free interpreted text then continue to
      free(prog[0].source);  // report an error in the program
      free(prog[0].line);
      free((char *) prog);
      prog = oldprog;
      stmts = oldstmts;
      errline = prog[ppc = ippc].num;
      interpreting = 0;
    } else {   // report the error and exit
      tracestr("Error ");
      tracenum(rc, 1);
      tracestr(" running ");
      tracestr(fname);
      tracestr(", line ");
      tracenum(errline, 1);
      tracestr(": ");
      tracestr(message(rc));
      if (edata) {
        tracestr(edata);
      }
      tracechar('\n');
      longjmp(*exitbuf, rc);
    }
  }
  // get the name of the file in which the error occurred
  if (edata && rc == -3) {  // the error data for rc -3 is a file name,
    strcpy(fname, edata);  // which will be printed by message()
  } else if (source) {  // by default, use the current file name.
    strcpy(fname, source[0]);  // if that doesn't exist, fname will already be OK
  }
  // clean program stack and print out traceback
  if (rc == Enoend && ppc == stmts) {
    // if an END is missing, don't use EOF as the error line
    tmpstack = 0;
    i = unpstack();
    if (i <= 10 || i == 15) {  // the start of the thing that needed END
      ppc = newppc;
      errline = sigl = prog[ppc].num;
      delpstack();
    }
  }
  if (rc && source) {  // source exists and it is a real error
    if (tmpstack) {
      tmpstack = 0;
      delpstack();  // remove temporary stack item
    }
    if (!catch) {
      printstmt(ppc, 0, 1);  // print the line in error
    }
    while (pstacklev) {
      i = unpstack();  // find out what the next entry is
      if (i == 16) {
        break;  // stop if interactive reached
      }
      if (catch && (i == 11 || i == 12) && interplev == lev) {
          break;   // at CALL entry and the level is right to catch the error */
      }
      freestack(delpstack(), i);  // delete stack entry and clean up
      if (!catch) {
        printstmt(newppc, 0, 1);  // print a traceback line
      }
      if (!sigl && interplev != interact) {  // if in INTERPRET (sigl==0) then
        sigl = prog[newppc].num;  // point sigl to the INTERPRET
      }
    }
  }
  if (!errline) {  // don't say "error in line 0" if it happened during INTERPRET.
    errline = sigl;  // flag the INTERPRET instruction
  }
  if (interact >= 0 && interplev == interact) {
    // the error occurred while interpreting interactive data.
    // print the message and jump back to interactive trace mode.
    fputs(message(rc), ttyout);
    if (edata && rc == Enovalue) {
      fputs(" on ", ttyout);
    }
    if (edata) {
      fputs(edata, ttyout);
    }
    fputc('\n', ttyout);
    longjmp(interactbuf, 1);
  }
  if (catch) {  // the error has been caught. jump to the right label
    sprintf(rcb, "%d", rc);  // set the special variable rc
    if (bit == Isyntax || bit == Ihalt || bit == Inovalue) {
      varset("RC", 2, rcb, strlen(rcb));
    }
    if (stmt <= 0) {  // if "label not found" was caught, go and catch it
      rc = Elabel;
      goto recurse;
    }
    sgstack[interplev].type = 1;   // store the information for CONTITION
    sgstack[interplev].which = bit;
    sgstack[interplev].data = sigdata[bit];
    sigdata[bit] = 0;
    if (bit == Inovalue && edata) {
      strcpy(sgstack[interplev].data = allocm(strlen(edata)), edata);
    }
    if (bit == Isyntax) {
      l = edata ? strlen(edata) : 0;
      l += strlen(ptr = message(rc));
      strcpy(sgstack[interplev].data = allocm(l + 1), ptr);
      if (edata) {
        strcat(sgstack[interplev].data, edata);
      }
    }
    if (bit == Ihalt) {
      sigl = haltline;  // in the case of halt, use stored sigl
    }
    sprintf(rcb, "%d", sigl);   // set the special variable sigl
    varset("SIGL", 4, rcb, strlen(rcb));
    ppc = stmt;
    longjmp(sgstack[interplev].jmp, 1);
  }
  // print the error message
  if (rc > 0) {
    tracestr("Error ");
    tracenum(rc, 1);
    if (ppc < 0) {
      tracestr(" interpreting arguments: ");
    } else {
      tracestr(" running ");
      tracestr(fname);
      tracestr(", line ");
      tracenum(errline, 1);
      tracestr(": ");
    }
    tracestr(message(rc));
    if (edata) {
      tracestr(edata);
    }
    tracechar('\n');
  }
  if (rc < 0) {
    tracestr(message(rc));
    if (rc == -3 && edata) {
      tracechar(' ');
    }
    if (edata) {
      tracestr(edata);
    }
    tracechar('\n');
  }
  // finally, exit...
  longjmp(*exitbuf, rc);
}

/*
 * Interprets a program, or part of a program.
 * Called by main() and the REXX instructions which cause control to move temporarily.
 * The return value is NULL, or the address of a string,
 *  determined by what is specified on EXIT or RETURN from the rexx program.
 * The length of the result (if any) is stored in anslen.
 */
char *interpreter(int *anslen, int start, char *callname, long calltype, char *args[], int arglen[], int inherit, int delay) {
  char *lineptr;                // pointer to the current program line
  char *tmpptr;
  int tmpchr, tmpppc;
  char c, c2;
  int len;
  int env;
  int i, m, e, z;
  int up;                       // whether to uppercase (during PARSE)
  char *exp;
  int l;
  int chkend;                   // whether to check for a line terminator
  char varname[maxvarname];
  int varlen;
  char *varref;
  int reflen;
  char *parselist[maxargs + 1]; // list of strings to PARSE
  int parselen[maxargs + 1];    // lengths of those strings
  int stype, sllen, sslen;      // used for DO and END
  char *slimit, *sstep, *svar;
  int ilimit, istep, istart;
  int whilep, untilp;           // values of WHILE and UNTIL conditions
  char *entry;                  // address of a program stack entry
  int fr;                       // number following FOR in a DO instruction
  int s;
  int *lptr;
  struct fileinfo *info;
  long filepos;

  ppc = start;

  // save stack details in case of signal or signal on or exit.
  // the return from _setjmp is:
  //  0 when called initially,
  //  1 when jumped to after an error is trapped,
  //  2 during SIGNAL (when the stack is cleared) and
  // -1 when jumped to on EXIT
  if (inherit) {
    sgstack[interplev].bits = sgstack[interplev - 1].bits;
    sgstack[interplev].callon = sgstack[interplev - 1].callon;
    sgstack[interplev].delay = sgstack[interplev - 1].delay | ((1 << delay) & ~1);
    sgstack[interplev].type = sgstack[interplev - 1].type;
    sgstack[interplev].which = sgstack[interplev - 1].which;
    for (l = 0; l < Imax; l++) {
      sgstack[interplev].ppc[l] = sgstack[interplev - 1].ppc[l];
    }
  } else {
    sgstack[interplev].bits = 0;
    sgstack[interplev].callon = 0;
    sgstack[interplev].delay = 0;
    sgstack[interplev].type = 0;
    sgstack[interplev].data = 0;
  }
  sgstack[interplev].bitson = 0;
  sgstack[interplev].data = 0;
  if (delay) {
    sgstack[interplev].which = delay;
    sgstack[interplev].type = 2;
    sgstack[interplev].data = sigdata[delay];
    sigdata[delay] = 0;
  }
  if ((s = setjmp(sgstack[interplev].jmp)) < 0) {
    // after EXIT, return from external call with the result
    if (!returnval) {
      return cnull;
    }
    stack(returnval, returnlen);
    free(returnfree);
    return delete(anslen);
  }
  // save the arguments (done here in case of a "signal on")
  curargs = args, curarglen = arglen;
  if (s == 2) {
    goto signal;
  }
  if (s == 0 && trcflag & Tclauses) {
    printstmt(ppc - 1, 1, 0);  // trace opening comments
  }
  // loop for each statement
  while (ppc < stmts) {
    lineptr = prog[ppc].line;
    ecstackptr = 0;  // clear the calculator stack
    eworkptr = 0;  // clear the workspace
    nextvar = 0;  // RXSHV_NEXTV starts from scratch
    // tracing - check for labels
    if (*lineptr == LABEL && (trcflag & Tlabels)) {
      printstmt(ppc, 0, 0);
      interactive();
    }
    chkend = 1;  // do check for line terminator
    // trace clauses
    if (trcflag & Tclauses) {
      if ((c = *lineptr) == END && pstacklev) {
        tmpchr = epstackptr;
        unpstack();  // at an END, print out also the DO
        delpstack();  // this un-indents the END and the DO
        printstmt(ppc, 0, 0);  // whilst maintaining a consistent stack
        printstmt(newppc, 0, 0);
        pstacklev++;  // put the deleted stack entry back.
        epstackptr = tmpchr;
      } else {
        printstmt(ppc, 0, 0);
      }
      interactive();
    }
    // Select what to do on the first character of the line
    if (*lineptr < 0) {         /* i.e. a keyword */
      switch (c2 = *lineptr++) {
        case SAYN:  // if a parameter is given, print it on stdout.
        case SAY: {  // with SAY, follow it with a newline
          if (*lineptr) {
            tmpchr = 0;
            exp = scanning(lineptr, &tmpchr, &len);
            lineptr += tmpchr;
            delete(&len);
            if (c2 == SAY) {
              exp[len++] = '\n';
            }
          } else {
            if (c2 == SAYN) {
              break;
            }
            len = 1;
            exp = pull;
            exp[0] = '\n';
          }
          if (c2 == SAY && exitlist[RXSIO]) {
            RXSIOSAY_PARM rxs;
            rxs.rxsio_string.strptr = exp;
            rxs.rxsio_string.strlength = len - 1;
            exp[len - 1] = 0;
            if (exitcall(RXSIO, RXSIOSAY, (PEXIT)&rxs) == RXEXIT_HANDLED) {
              break;
            }
            exp[len - 1] = '\n';
          }
          // mirror the charout function to print the data
          if (!(info = (struct fileinfo *) hashget(1, "stdout", &l))) {
            break;
          }
          if (info->lastwr && (filepos = ftell(info->fp)) >= 0 && filepos != info->wrpos) {
            info->wrpos = filepos;
            info->wrline = 0;  // position has been disturbed
          }
          if (info->lastwr == 0) {
            fseek(info->fp, info->wrpos, 0);
          }
          info->lastwr = 1;
          if (fwrite(exp, len, 1, info->fp)) {
            if (info->wrline) {
              info->wrline++;
            }
            info->wrchars = 0;
            if (info->persist && (info->wrpos = ftell(info->fp)) < 0) {
              info->wrpos = 0;
            }
          } else {
            fseek(info->fp, info->wrpos, 0);
          }
          if (c2 == SAYN) {
            fflush(info->fp);
          }
          break;
        }
        case DO: {
          tmpstack = 1;  // stack the current position.
          entry = (char *) pstack(stype = 0, sizeof(struct minstack));
          if (!(c = *lineptr)) {  // non-repetitive.
            tmpstack = 0;
            break;  // do nothing.
          }
          if (c > 0) {  // a repetition count or a variable follows
            tmpchr = 0;
            varref = lineptr;   // save the var's reference
            getvarname(lineptr, &tmpchr, varname, &varlen, maxvarname);
            reflen = tmpchr;
            if (lineptr[tmpchr] != '=') {  // not a variable unless followed
              varname[0] = 0;  // by "="
            }
          } else {
            varname[0] = 0;  // a keyword follows
          }
          if (varname[0]) {
            // a variable clause was found.
            // begin by getting the start value, then get the "TO", "BY" and "FOR" values.
            // the values are stored as integer offsets in the calculator stack, in case the stack moves.
            // FOR is stored as an integer value.
            tmpchr++;   // character after '='
            scanning(lineptr, &tmpchr, &len);
            lineptr += tmpchr;
            unplus(OPplus);  // do "name = expri + 0"
            istart = nodelete(&len) - cstackptr;
            sllen = -1;  // limit=default (=null) (length -1)
            sslen = -1;  // step=default (=1)
            fr = -1;  // for=default (=null)
            while ((c = *lineptr) == TO || c == BY || c == FOR) {
              tmpchr = 1;
              if (c == TO) {
                slimit = scanning(lineptr, &tmpchr, &sllen);
                ilimit = slimit - cstackptr;
              } else if (c == BY) {
                sstep = scanning(lineptr, &tmpchr, &sslen);
                istep = sstep - cstackptr;
              } else {   // c==FOR
                scanning(lineptr, &tmpchr, &i);
                if ((fr = getint(1)) < 0) {
                  die(Erange);
                }
              }
              lineptr += tmpchr;
            }
            // a keyword or line terminator must follow:
            if (c > 0) {
              die(Exdo);
            }
            // now stack the parameters in the correct order.
            // this leaves unused copies further down the stack,
            // but these are cleared at the end of the command anyway.
            // Once stacked they will be copied into the program stack.
            i = reflen + sllen + sslen + len + 64 + ecstackptr;
            // make sure cstack doesn't move while stacking data
            mtest(cstackptr, cstacklen, i, i - ecstackptr);
            stack(istart + cstackptr, len);
            varset(varname, varlen, istart + cstackptr, len);   // var = start
            tmpchr = ecstackptr;  // save the address of the following:
            if (sllen >= 0) {
              stack(ilimit + cstackptr, sllen);
              sllen = 1;
            } else {
              stack(cnull, sllen = 0);  // sllen now is "limit given?"
            }
            if (sslen >= 0) {
              stack(istep + cstackptr, sslen);
            } else {
              stack("1", 1);
            }
            stack(varref, reflen + 1);  // variable name plus the '='
            i = ecstackptr - tmpchr;  // i is the length of all that data
            dtest(pstackptr, pstacklen, epstackptr + i + 30, i + 300, entry);  // stack the data on the pstack
            memcpy((char *) &((struct minstack *) entry)->len, cstackptr + tmpchr, i);
            epstackptr += i - 2 * four;  // now add the FOR num, the length, and the type
            (*(int *) (pstackptr + epstackptr)) = fr,
            (*(int *) (pstackptr + (epstackptr += four))) = i + four + sizeof(struct minstack);
            (*(int *) (pstackptr + (epstackptr += four))) = stype = 10;
            epstackptr += four;
            // having constructed the program stack, make an initial test on the data
            delete(&l);  // delete the varname
            if (num(&m, &e, &z, &l) < 0) {
              die(Enum);  // test the step
            }
            delete(&l);  // delete the step
            if (sllen) {
              binmin(4);  // if limit supplied, sub from value
            } else {
              stack("0", 1);  // else just stack 0
            }
            if (!m) {
              unmin(20);  // negate that if step >= 0
            }
            num(&m, &e, &z, &l);  // test the answer
            if (m || !fr) {  // if that<0 or FOR==0 then leave
              sllen = 1;
              tmpstack = 0;
              goto leaveit;
            }
          }
          // end of control variable processing; start of numeric count processing
          else if (c > 0) {
            tmpchr = 0;
            scanning(lineptr, &tmpchr, &len);
            lineptr += tmpchr;
            if ((i = getint(1)) < 0) {
              die(Erange);  // i is the number
            }
            if (!i) {  // i==0 so leave already
              sllen = 1;
              tmpstack = 0;
              goto leaveit;
            }
            // make a FOR stack containing the counter
            ((struct forstack *) entry)->fornum = i;
            ((struct forstack *) entry)->len = sizeof(struct forstack);
            ((struct forstack *) entry)->type = stype = 15;
            epstackptr += sizeof(struct forstack) - sizeof(struct minstack);
          }
          // next deal with any other data (while/until/forever) first update the character pointer to the current position
          ((struct minstack *) entry)->pos = lineptr;
          c = *lineptr;
          if (c == FOREVER) {  // like UNTIL but no expression follows
            if (!stype) {
              ((struct minstack *) entry)->type = 8;
            }
            c = *++lineptr;
          }
          if (c == WHILE || c == UNTIL) {  // s/if/while for multiple conds
            if (!stype) {  // no control variable or counter
              ((struct minstack *) entry)->type = stype = 8;
            }
            if (c == WHILE) {  // evaluate and test the value now
              tmpchr = 1;
              scanning(lineptr, &tmpchr, &len);
              lineptr += tmpchr;
              if (num(&m, &e, &z, &l) < 0) {
                die(Enum);
              }
              if (*lineptr > 0) {
                die(Exdo);
              }
              if (z) {
                sllen = 1;
                tmpstack = 0;
                goto leaveit;
              }
            } else {
              for (c = 1; c && c != WHILE && c != UNTIL; c = *++lineptr) {
                // but jump past an UNTIL value.
              }
            }
          }
          // finish off DO processing
          if (*lineptr) {
            die(Exdo);  // check for invalid data at end
          }
          tmpstack = 0;  // stack entry is no longer temporary
          break;
        }
        case END: {
          if (!pstacklev) {
            die(Eend);  // no data on stack
          }
          exp = pstackptr + epstackptr; // get top stack entry
          stype = *((int *) exp - 1);
          exp -= *((int *) exp - 2);
          tmpppc = ((struct minstack *) exp)->stmt;
          tmpptr = ((struct minstack *) exp)->pos;
          if (stype == 2) {  // top entry is SELECT
            goto when;
          }
          if (stype > 10 && stype != 15) {
            die(Eend);  // top entry is not DO
          }
          if (!stype) {  // a non-repetitive DO - just continue
            delpstack();
            break;
          }
          // first the UNTIL condition(s) are evaluated.
          // tmpppc and tmpptr point to them in the original DO instruction.
          // variable untilp will be set to true if the loop needs to be exited
          whilep = 0;
          untilp = 0;
          c = *tmpptr;
          if (c == FOREVER) {
            c = *++tmpptr;  // ignore FOREVER
          }
          if (c == UNTIL) {
            tmpchr = 1;
            scanning(tmpptr, &tmpchr, &len);
            tmpptr += tmpchr;
            eworkptr = 0;
            len = num(&m, &e, &z, &l);
            // so len>=0 if the number was valid, z=1 if it was zero.
            if (len < 0) {
              die(Enum);
            }
            untilp = !z;
          }
          if ((c = *tmpptr) && c != WHILE) {  // should be nothing after UNTIL
            delpstack();  // if not, remove the DO's stack entry but
            ppc = tmpppc;  // flag the error in the DO statement, not the END
            die(Edata);
          }
          // now, the UNTIL condition is tested before incrementing the control variable (if any).
          // a symbol after the END, if any, must be skipped if the loop is to be left at this point.
          if (untilp) {
            if ((c = *lineptr) == SELECT) {
              die(Exend);
            }
            if (c > 0) {
              if (stype != 10) {
                die(Exend);  // must be DO with control
              }
              svar = pstackptr + epstackptr - 3 * four;  // point to FOR field
              svar -= four;   // point to variable length
              svar -= align(varlen = *(int *) svar);  // point to name
              testvarname(&lineptr, svar, varlen - 1);  // check it matches
            }
            delpstack();
            // check for conditions before leaving
            tmpchr = ppc;
            ppc = tmpppc;
            doconds();
            ppc = tmpchr;
            break;
          }
          // The integer counter (if any) is decremented, tested and added to the WHILE condition */
          if (stype == 10 || stype == 15) {
            svar = pstackptr + epstackptr - 3 * four;   // point to FOR field
            if ((fr = *(int *) svar) > 0) {   // get the FOR field
              (*(int *) svar) = --fr;  // fr now holds its new value
            }
            if (!fr) {
              whilep = 1;
            }
          }
          if (stype == 10) {
            // the top stack entry is DO with variable. Increment the variable
            svar -= four;  // point to variable length
            svar -= align(varlen = *(int *) svar);  // point to name
            testvarname(&lineptr, svar, varlen - 1);   // check it matches
            tmpchr = 0;   // evaluate the symbol's current name
            getvarname(svar, &tmpchr, varname, &varlen, maxvarname);
            if (!(exp = varget(varname, varlen, &len))) {
              die(Enum);
            }
            // no value; Er, what about NOVALUE?
            stack(exp, len);  // stack the variable's value
            sslen = *((int *) svar - 1);  // get the step's length
            sstep = svar - align(sslen) - four;  // and the step's address
            sllen = *((int *) sstep - 1);  // get the limit's length
            slimit = sstep - align(sllen) - four;  // and the limit's value
            stack(sstep, sslen), num(&m, &e, &z, &l);  // get the step's sign in m
            binplus(OPadd);  // add step to value
            tmpchr = ecstackptr;  // get the result without deleting it (by saving ecstackptr)
            exp = delete(&len);
            ecstackptr = tmpchr;
            varset(varname, varlen, exp, len);  // set the var's new value
            if (sllen) {  // if limit was given,
              stack(slimit, sllen);  // subtract it from value
              binmin(4);
            } else {
              stack("0", 1);  // else just stack 0
            }
            if (!m) {
              unmin(20);  // negate if step>=0
            }
            num(&m, &e, &z, &l);  // get sign in m
            if (m || !fr) {  // if loop has finished then
              whilep = 1;  // pretend the WHILE was false
            }
          } else if ((c = *lineptr) == SELECT || c > 0) { // end if(DO with control variable)
            die(Exend); // otherwise if END is followed by anything, it is an error.
          }
          // finally, the WHILE condition(s) are evaluated.
          // tmpptr points to them in the original DO instruction.
          if (!whilep && *tmpptr == WHILE) {
            tmpchr = 1;
            scanning(tmpptr, &tmpchr, &len);
            tmpptr += tmpchr;
            eworkptr = 0;
            len = num(&m, &e, &z, &l);
            if (len < 0) {
              die(Enum);
            }
            whilep = z;
            if ((c = *tmpptr)) {  // should be nothing after WHILE
              delpstack();
              ppc = tmpppc;  // flag the error in the DO statement
              die(Edata);
            }
          }
          // so now leave if whilep is true, but iterate if it is false.
          tmpchr = ppc;
          ppc = tmpppc;
          doconds();  // check for trapped conditions in the DO
          if (whilep) {
            ppc = tmpchr;
            delpstack();
          } else {
            lineptr = tmpptr;  // copy the character ptr to the end of the DO clause */
          }
          break;
        }
        case IF: {
          tmpchr = 0;
          scanning(lineptr, &tmpchr, &len);
          if (num(&m, &e, &z, &l) < 0) {
            die(Enum);  // !z is the given value
          }
          if (!(c = *lineptr)) {
            die(Edata);  // line end reached
          }
          doconds();  // trap conditions before continuing
          if (ppc + 1 == stmts || prog[++ppc].line[0] != THEN) {
            die(Enothen);
          }
          if (++ppc == stmts) {
            die(Eprogend);
          }
          chkend = 0; // we will be already at start of a stmt
          if (!z) {
            break;  // true: continue with THEN
          }
          skipstmt();  // false: skip THEN
          if (prog[ppc].line[0] == ELSE) {  // if the next word is ELSE
            if (++ppc == stmts) {
              die(Eprogend);  // check for more statements
            } else {
              break;  // Do the stmt after the ELSE.
            }
          }
          // usually it would be skipped
          break;
        }
        case ELSE: {
          chkend = 0;  // we will be already at start of a stmt
          if (++ppc == stmts) {
            die(Eprogend);  // check for more statements
          }
          skipstmt();  // skip the ELSE statement
          break;
        }
        case SELECT: {
          chkend = 0;   // we will be already at start of a stmt
          if (*lineptr) {
            s = 1;   // s means a value is given, and is on the stack
            tmpchr = 0;
            scanning(lineptr, &tmpchr, &len);
            lineptr += tmpchr;
          } else {
            s = 0;  // it is a standard SELECT with no value
          }
          if ((c = *lineptr)) {
            die(Edata);
          }
          pstack(2, sizeof(struct minstack));  // stack SELECT entry
          if (++ppc == stmts) {
            die(Eprogend);  // check for more statements
          }
          z = 1;
          while ((lineptr = prog[ppc].line)[0] == WHEN) {
            if (trcflag & Tclauses) {
              printstmt(ppc - 1, 1, 0), printstmt(ppc, 0, 0);
            }
            tmpchr = 1;
            if (s) {
              rxdup();  // duplicate the SELECT value
            }
            scanning(lineptr, &tmpchr, &len);   // what comes after WHEN
            lineptr += tmpchr;
            if ((c = *lineptr)) {
              die(Edata);
            }
            doconds();  // trap conditions before continuing
            if (1 + ppc == stmts) {
              die(Enothen);
            }
            if (prog[++ppc].line[0] != THEN) {
              die(Enothen);  // find a THEN
            }
            if (++ppc == stmts) {
              die(Eprogend);  // check for more statements
            }
            if (s) {
              binrel(OPequ);  // Compare value with SELECT value
            }
            if (num(&m, &e, &z, &l) < 0) {
              die(Enum);  // test the result
            }
            delete(&l);
            if (!z) {
              break;  // True: follow this WHEN
            }
            if ((c = prog[ppc].line[0]) == WHEN || c == OTHERWISE) {
              die(Ewhen);
            }
            skipstmt();
          }
          if (z) {
            if ((lineptr = prog[ppc].line)[0] != OTHERWISE) {
              die(Enowhen);  // no correct alternative: error
            }
            if (++ppc == stmts) {
              die(Eprogend);  // check for more statements
            }
          }
          break;
        }
        case OTHERWISE: // for OTHERWISE and WHEN, just escape out of the current SELECT construction.
        case WHEN: {
          if ((!pstacklev) || unpstack() != 2) {
            die(Ewhen);  // the WHEN wasn't inside a SELECT
          }
        when: // goto
          while (prog[ppc].line[0] == WHEN) {  // find an END by repeatedly
            if (1 + ppc >= stmts) {
              die(Enothen);  // skipping WHENs
            }
            if (prog[1 + ppc].line[0] != THEN) {
              die(Enothen);
            }
            if ((ppc += 2) == stmts) {
              die(Enoend);
            }
            skipstmt();
          }
          if (prog[ppc].line[0] == OTHERWISE) { // and step over any OTHERWISE
            findend();
          } else if (prog[ppc].line[0] != END) {
            die(Enowhen);
          }
          c = prog[ppc].line[1];  // the character after END
          if (c && c != SELECT) {  // must be SELECT or terminator
            die(Exend);
          }
          epstackptr -= sizeof(struct minstack);  // delete stack entry
          pstacklev--;
          lineptr = prog[ppc].line + 1 + (c != 0);
          chkend = 1;  // do check for linend char
          break;
        }
        case OPTIONS: {  // split the option into tokens and call setoption
          tmpchr = 0;
          exp = scanning(lineptr, &tmpchr, &len);
          lineptr += tmpchr;
          while (len) {
            while (len && *exp == ' ') {
              exp++, len--;
            }
            if (!len) {
              break;
            }
            tmpptr = exp;
            while (len && *exp != ' ') {
              exp++, len--;
            }
            setoption(tmpptr, exp - tmpptr);
          }
          break;
        }
        case PARSE: {
          up = 0;
          if (*lineptr == UPPER) {
            lineptr++;
            up = 1;  // up="upper case?"
          }
          i = 1;   // one argument to parse usually
          // depending on the next keyword, copy the appropriate data into parselist[] and parselen[], setting i to the number of strings
          switch (lineptr++[0]) {
            case ARG:
              for (i = 0; args[i] != cnull; i++) {
                parselist[i] = args[i];
                if ((parselen[i] = arglen[i]) < 0) {
                  parselen[i] = 0;
                }
              }
              break;
            case SOURCE:
              parselist[0] = psource;
              parselen[0] = strlen(psource);
              break;
            case PULL:  // first try the REXX data stack
            case LINEIN:  // mirrors the linein() function
              if (!(info = (struct fileinfo *) hashget(1, "stdin", &l))) {
                // if it was closed by the user, signal on notready or else just use an empty string
                rcset(Eeof, Enotready, "stdin");
                l = 0;
              } else {
                if (info->lastwr == 0 && (filepos = ftell(info->fp)) >= 0 && filepos != info->rdpos) {
                  info->rdpos = filepos;
                  info->rdline = 0;  // position has been disturbed
                }
                clearerr(info->fp);
                if (info->lastwr) {
                  fseek(info->fp, info->rdpos, 0);
                }
                info->lastwr = 0;
                c = (sgstack[interplev].callon & (1 << Ihalt)) | (sgstack[interplev].delay & (1 << Ihalt));
                if (!c) {
                  on_interrupt(SIGINT, 1);
                }
                l = 0;
                while ((s = getc(info->fp)) != EOF && s != '\n') {
                  mtest(pull, pulllen, l + 1, 256);
                  pull[l++] = s;
                }
                on_interrupt(SIGINT, 0);
                if (delayed[Ihalt] && !c) {
                  delayed[Ihalt] = 0;
                  fseek(info->fp, info->rdpos, 0);  // reset to start of line, if possible
                  die(Ehalt);
                }
                if (info->rdline) {
                  info->rdline++;
                }
                info->rdchars = 0;
                if (s == EOF && !l) {
                  rxseterr(info);
                }
                if ((info->rdpos = ftell(info->fp)) < 0) {
                  info->rdpos = 0;
                }
                if (info->errnum || setrcflag) {
                  rcset(info->errnum - Eerrno, Enotready, "stdin");
                }
              }
              parselist[0] = pull;
              parselen[0] = l;
              break;
            case VALUE:
              i = 0;
              if (*lineptr == WITH) {
                parselist[0] = "";
                parselen[0] = 0;
              } else {
                for (;;) {
                  tmpchr = 0;
                  parselist[i] = scanning(lineptr, &tmpchr, &parselen[i]);
                  lineptr += tmpchr;
                  if ((c = *lineptr) == WITH) {
                    break;
                  }
                  if (c != ',' || i == maxargs) {
                    die(Eparse);
                  }
                  while (*lineptr == ',') {
                    lineptr++;
                    parselist[++i] = "";
                    parselen[i] = 0;
                  }
                }
              }
              i++;
              lineptr++;
              if (parselen[0]) {
                // copy the first expression to pull.
                // if it's not copied then the calculator stack might move when a literal pattern from the template is being stacked.
                // bug remains for the subsequent expressions, which we hope no one uses!
                mtest(pull, pulllen, parselen[0], parselen[0] - pulllen);
                memcpy(pull, parselist[0], parselen[0]);
                parselist[0] = pull;
              }
              break;
            case VAR:
              tmpchr = 0;
              getvarname(lineptr, &tmpchr, varname, &varlen, maxvarname);
              lineptr += tmpchr;
              if (varname[0] == 0) {
                die(Enosymbol);
              }
              if ((exp = varget(varname, varlen, &parselen[0])) == cnull) {
                if ((varname[0] & 128) && !memchr(varname, '.', varlen)) {
                  varname[varlen++] = '.';  // add dot to a stem
                }
                varname[0] &= 127;
                varname[varlen] = 0;
                if (sgstack[interplev].bits & (1 << Inovalue)) {
                  errordata = varname;
                  die(Enovalue);  // a novalue error was caught
                }
                parselist[0] = varname;
                parselen[0] = strlen(varname);
              } else {
                // copy the variable's value to pull.
                // we can't use the value pointer itself because that might move while the template is being interpreted
                mtest(pull, pulllen, parselen[0], parselen[0] - pulllen);
                memcpy(parselist[0] = pull, exp, parselen[0]);
              }
              break;
            case VERSION:
              parselist[0] = version;
              parselen[0] = strlen(version);
              break;
            case NUMERIC:  // make details [len(pull)>25]
              sprintf(pull, "%d %d %s", precision, fuzz - precision, numform ? "ENGINEERING" : "SCIENTIFIC");
              parselist[0] = pull;
              parselen[0] = strlen(pull);
              break;
            default: {
              die(Eform);  // an invalid subkeyword was found
            }
          }
          parselist[i] = cnull; // terminate the list
          // now would be a good time to uppercase, I think...
          if (*lineptr) {  // if a template supplied,
            tmpchr = 0;
            parse(parselist, parselen, up, lineptr, &tmpchr);
            lineptr += tmpchr;
          }
          break;
        }
        case EXIT: {  // get the value if any and jump back to the outermost level of interpretation in the current program.
          if (*lineptr) {
            tmpchr = 0;
            returnval = scanning(lineptr, &tmpchr, &returnlen);
            if ((c = lineptr[tmpchr])) {
              die(Edata);
            }
            // this way the result doesn't get destroyed if the calc stack is freed by the following code
            returnfree = cstackptr;
            cstackptr = allocm(cstacklen = returnlen + 16);
          } else {
            returnval = 0;
          }
          while (pstacklev) {
            stype = unpstack();
            // delete every program stack entry until an external call
            if (!prog[ppc].num) { // if an error occurs during INTERPRET,
              ppc = newppc;  // blame the INTERPRET instr.
            }
            freestack(delpstack(), stype);
          }
          longjmp(sgstack[interplev].jmp, -1);
        }
        case RETURN: {  // just return, with the given value if any
          if (*lineptr) {
            tmpchr = 0;
            scanning(lineptr, &tmpchr, &len);
            if ((c = lineptr[tmpchr])) {
              die(Edata);
            }
            return delete(anslen);
          }
          if (calltype == RXFUNCTION) {
            die(Ereturn);
          }
          return anslen[0] = 0, cnull;
        }
        case CALL: {
          if ((c = *lineptr) == ON || c == OFF) {  // set or clear a trap
            findsigl(&istart);  // find the start level
            prog = oldprog;
            stmts = oldstmts;   // number to affect
            i = gettrap(&lineptr, c == ON, &l);  // get the trap name
            if (i == Isyntax || i == Inovalue) {
              die(Etrap);
            }
            if (c == ON) {
              if (!l) {
                if (prog[ppc].num) {
                  l = -ppc;
                } else {
                  sprintf(workptr, ": \'%s\'", varnamebuf), errordata = workptr, die(Elabel);
                }
              }
              for (e = istart; e <= interplev; e++) {
                sgstack[e].bits &= ~(1 << i);
                sgstack[e].bitson &= ~(1 << i);
                sgstack[e].delay &= ~(1 << i);
                sgstack[e].callon |= (1 << i);
                sgstack[e].ppc[i] = l;
              }
            } else {
              for (l = istart; l <= interplev; l++) {
                sgstack[l].bits &= ~(1 << i);
                sgstack[l].bitson &= ~(1 << i);
                sgstack[l].delay &= ~(1 << i);
                sgstack[l].callon &= ~(1 << i);
              }
            }
            break;
          }
          tmpchr = 0;   // get details, then call rxcall()
          z = gettoken(lineptr, &tmpchr, varname, maxvarname, 0) - 1;
          lineptr += tmpchr;
          // so varname holds the routine name, z=0 if it wasn't quoted
          i = m = 0;  // i=arg count; m=last character was comma
          if (*lineptr == ' ') {
            lineptr++;  // a space may follow the name
          }
          while ((c = *lineptr)) {  // now loop for arguments
            if (c == ',') {
              stacknull();
            } else {
              tmpchr = 0;
              scanning(lineptr, &tmpchr, &len);
              lineptr += tmpchr;
            }
            i++;
            if ((m = (*lineptr == ','))) {
              lineptr++;
            }
          }
          if (m) {
            stacknull();
            i++;
          }
          doconds();  // before calling, check for conditions
          if (rxcall(0, varname, i, z, RXSUBROUTINE)) {  // call it
            exp = delete(&len), // a result was given, so set RESULT
            varset("RESULT", 6, exp, len);
          } else {
            varset("RESULT", 6, cnull, -1);  // no result, so drop RESULT
          }
          timeflag &= (~2);  // in case of "call time" don't make a lasting timestamp
          break;
        }
        case SIGNAL: {
          // go down stack to find l=most recent nonzero line no
          l = findsigl(&istart);
          prog = oldprog;
          stmts = oldstmts;
          if ((c = *lineptr) == ON || c == OFF) {  // set or clear a trap
            i = gettrap(&lineptr, c == ON, &l);  // get the trap name
            if (c == ON) {
              if (!l) {
                if (prog[ppc].num) {
                  l = -ppc;  // flag the stmt in error
                } else {
                  sprintf(workptr, ": \'%s\'", varnamebuf);
                  errordata = workptr;
                  die(Elabel);  // die if we are interpreted
                }
              }
              sgstack[istart].ppc[i] = l;
              sgstack[istart].bitson |= (1 << i);
              for (l = istart; l <= interplev; l++) {
                sgstack[l].bits |= (1 << i);
                sgstack[l].callon &= ~(1 << i);
                sgstack[l].delay &= ~(1 << i);
              }
            } else {
              for (l = istart; l <= interplev; l++) {
                sgstack[l].bits &= ~(1 << i);
                sgstack[l].bitson &= ~(1 << i);
                sgstack[l].callon &= ~(1 << i);
                sgstack[l].delay &= ~(1 << i);
              }
            }
            break;
          }
          // else signal to a given label name.
          // get the name, set the source line number and clear the machine stack first
          tmpchr = 0;
          gettoken(lineptr, &tmpchr, varname, maxvarname, 1);
          signalto = varname;
          if (lineptr[tmpchr]) {
            die(Edata);
          }
          doconds();  // before going, check for conditions
          ppc = l;
          if (istart != interplev) {  // clear the stack if necessary
            longjmp(sgstack[istart].jmp, 2);
          }
          // code to transfer control to a label starts here
        signal: // goto
          while (pstacklev && ((stype = unpstack()) < 11 || stype > 13)) {
            freestack(delpstack(), stype);
          }
          for (lptr = (int *) labelptr; (l = *lptr) && strcasecmp(signalto, (char *) (lptr + 2)); lptr += 2 + align(l + 1) / four) {
            // quit all current DO, SELECT, INTERPRET constructs
          }
          if (!l) {  // the label wasn't found
            sprintf(workptr, ": \'%s\'", signalto);
            errordata = workptr;
            die(Elabel);
          }
          // before jumping, save current ppc in variable SIGL
          sprintf(varname, "%d", prog[ppc].num);
          varset("SIGL", 4, varname, strlen(varname));
          ppc = lptr[1];
          chkend = 0;
          break;
        }
        case ITERATE: {  // find the END and jump to it
          tmpchr = epstackptr;
          istart = pstacklev;
          sllen = 1;
          if ((c = *lineptr)) {
            if (rexxsymbol(c) < 1) {
              die(Enosymbol);
            }
            varref = lineptr;
            reflen = 0;
            skipvarname(lineptr, &reflen);
            if ((c = lineptr[reflen])) {
              die(Edata);
            }
          } else {
            reflen = 0;
            if (*lineptr) {
              die(Enosymbol);  // symbol expected; we got something else
            }
          }
          // so (varref,reflen) is a control variable or a null string
          for (;;) {
            // delete stack items until the right loop found.
            // the number of ENDs needed is counted in sllen
            while (pstacklev && (stype = unpstack()) < 8) {  // not a loop
              delpstack();
              sllen++;
            }
            if (!pstacklev || (stype > 10 && stype != 15)) {  // function call
              epstackptr = tmpchr;
              pstacklev = istart;
              die(Eleave);  // so the required loop is not active
            }
            if (stype == 8 || stype == 15) {  // un-named DO loop
              if (!reflen) {
                break;  // OK if no name found
              } else {
                delpstack();
                sllen++;
                continue;
              }
            }
            // otherwise the top stack entry is a DO with variable
            svar = pstackptr + epstackptr - 4 * four;
            svar -= align(len = *(int *) svar);   // point to the name
            if (!(reflen && (len - 1 != reflen || memcmp(varref, svar, reflen)))) {
              break;  // the correct DO loop has been found
            }
            sllen++, delpstack();
          }
          stype = *((int *) (pstackptr + epstackptr) - 1);  // the type of loop being iterated
          while (sllen--) {   // find the right number of ENDs
            findend();
            if (sllen) {
              if (++ppc == stmts) {
                die(Enoend);
              }
            }
          }
          // now test the name following the END
          if (stype == 10) {
            svar = pstackptr + epstackptr - 4 * four;
            svar -= align(len = *(int *) svar);
            lineptr = prog[ppc].line + 1;
            testvarname(&lineptr, svar, len - 1);
          } else if ((c = prog[ppc].line[1])) {
            die(Edata);
          }
          chkend = 0;  // already at the start of a statement
          break;
        }
        case LEAVE: {  // LEAVE is essentially the same as ITERATE, but it goes past the END after finding it
          tmpchr = epstackptr;
          istart = pstacklev;
          sllen = 1;
          if ((c = *lineptr)) {
            if (rexxsymbol(c) < 1) {
              die(Enosymbol);
            }
            varref = lineptr;
            reflen = 0;
            skipvarname(lineptr, &reflen);
            if ((c = lineptr[reflen])) {
              die(Edata);
            }
          } else {
            reflen = 0;
            if (*lineptr) {
              die(Enosymbol);
            }
          }
          for (;;) {
            while (pstacklev && ((stype = unpstack()) < 8)) {
              delpstack();
              sllen++;
            }
            if (!pstacklev || (stype > 10 && stype != 15)) {
              epstackptr = tmpchr;
              pstacklev = istart;
              die(Eleave);
            }
            if (stype == 8 || stype == 15) {
              if (!reflen) {
                break;
              } else {
                delpstack();
                sllen++;
                continue;
              }
            }
            svar = pstackptr + epstackptr - 4 * four;
            svar -= align(len = *(int *) svar);
            if (!(reflen && (len - 1 != reflen || memcmp(varref, svar, reflen)))) {
              break;
            }
            sllen++;
            delpstack();
          }
        leaveit:
          // find the "sllen"th END and jump past it
          if (ppc + 1 == stmts) {
            die(Enoend);  // get past the LEAVE or, more
          }
          ppc++;   // importantly, the DO
          stype = *((int *) (pstackptr + epstackptr) - 1);
          while (sllen--) {
            findend();
            if (sllen) {
              if (++ppc == stmts) {
                die(Enoend);
              }
            }
          }
          lineptr = prog[ppc].line + 1;
          if (stype == 10) {   // test the name given after END
            svar = pstackptr + epstackptr - 4 * four;
            svar -= align(len = *(int *) svar);
            testvarname(&lineptr, svar, len - 1);
          } else if ((c = *lineptr)) {
            die(Edata);
          }
          delpstack();  // delete stack entry and continue past the END
          // fall-through
        }
        case LABEL:  // same as NOP
        case NOP: {
          break;  // do nothing, like it says...
        }
        case INTERPRET: {  // get the details and call rxinterp
          tmpchr = 0;
          exp = scanning(lineptr, &tmpchr, &len);
          lineptr += tmpchr;
          if (trcflag & Tclauses) {  // trace the interpret data
            traceprefix(prog[ppc].num, "*~*");
            for (i = 0; i < traceindent * pstacklev; i++) {
              tracechar(' ');
            }
            traceput(exp, len);
            tracechar('\n');
          }
          exp = rxinterp(exp, len, anslen, callname, calltype, args, arglen);
          if (*anslen >= 0) {
            return exp;  // "interpret 'return x'" causes x to be returned from rxinterp;  convey it back to the caller
          }
          break;
        }
        case PROCEDURE: {  // make a new variable table, then examine the instruction and copy or hide variables
          if (epstackptr && *((int *) (pstackptr + epstackptr) - 1) == 11) {
            // inside internal function; signal that PROCEDURE has been done
            (*((int *) (pstackptr + epstackptr) - 1))++;
            newlevel();  // make a complete new level of variables
          } else if (epstackptr || !varstkptr || !exposeflag) {
            die(Eprocedure);  //  not inside a function or no 'options expose'
          }
          if (!(c = *lineptr)) {
            break;  // OK if no further data follows
          }
          lineptr++;
          i = 1;  // i.e. start of data
          if (c == EXPOSE) {  // expose all the given variables with varcopy
            while (i || (c = *lineptr) == ' ' || c == '(') {
              if (!i && c != '(') {
                lineptr++;  // step over the space
              }
              i = 0;
              if ((c = *lineptr) == '(') {
                lineptr++;
              }
              tmpchr = 0;
              getvarname(lineptr, &tmpchr, varname, &varlen, maxvarname);
              lineptr += tmpchr;
              if (!varname[0]) {
                die(Enosymbol);
              }
              varcopy(varname, varlen);
              if (c == '(') {   // expose a list of variables
                if (lineptr++[0] != ')') {
                  die(Elpar);
                }
                if ((c = *lineptr) && c != ' ') {  // space is not required,
                  i = 1;  // but if omitted remember not to skip it
                }
                exp = varget(varname, varlen, &len);
                tmpchr = 0;   // prepare to parse the list of symbols
                if (exp && len > 0) {
                  mtest(workptr, worklen, len + 1, len - worklen + 1);
                  for (c = 0, l = 0; l < len; l++) {  // copy the list in uc
                    if (!c && ((c2 = exp[l]) == '\'' || c2 == '\"')) {
                      c = c2;
                    } else if ((c2 = exp[l]) == c) {
                      c = 0;  // c is quote flag
                    }
                    workptr[l] = c ? c2 : uc(c2);  // uppercase and copy
                  }
                  if (c) {
                    die(Equote);
                  }
                  workptr[len] = 0;   // now add a terminator
                  while (l || workptr[tmpchr] == ' ') {
                    if (!l) {
                      tmpchr++;  // step over the space
                    }
                    l = 0;
                    getvarname(workptr, &tmpchr, varname, &varlen, maxvarname);
                    if (!varname[0]) {
                      die(Enosymbol);
                    }
                    varcopy(varname, varlen);
                  }
                  // should now have reached the end of the list
                  if (tmpchr != len) {
                    die(Enosymbol);
                  }
                }
              }
            }
          } else {
            if (c != HIDE) {
              die(Eform);  // invalid subkeyword
            } else {
              // copy the entire variable table, then delete the named variables with vardel
              vardup();
              while (i || (c = *lineptr) == ' ') {
                if (!i) {
                  lineptr++;
                }
                i = 0;
                tmpchr = 0;
                getvarname(lineptr, &tmpchr, varname, &varlen, maxvarname);
                lineptr += tmpchr;
                if (!varname[0]) {
                  die(Enosymbol);
                }
                vardel(varname, varlen);
              }
            }
          }
          break;
        }
        case NUMERIC: {        /* get parameter, and set global variable */
          tmpchr = 0;
          if ((c = lineptr++[0]) == FORM) {
            gettoken(lineptr, &tmpchr, varname, maxvarname, 1);
            lineptr += tmpchr;
            if (!strcmp(varname, "SCIENTIFIC")) {
              numform = 0;
            } else {
              if (!strcmp(varname, "ENGINEERING")) {
                numform = 1;
              } else {
                die(Eform);  // invalid subkeyword
              }
            }
            break;
          }
          if (c > 0) {
            die(Eform);  // a word must follow, not characters
          }
          if (c >= -1) {
            die(Enosymbol);  // nothing followed
          }
          scanning(lineptr, &tmpchr, &len);   // an integer must follow
          lineptr += tmpchr;
          i = getint(1);
          if (i < 0 || i > maxdigits) {
            die(Erange);
          }
          if (c == DIGITS) {
            if (!i) {
              die(Erange);
            } else {
              precision = i;
              fuzz = i;
            }
          } else {
            if (c == FUZZ) {
              if ((i = precision - i) < 1) {
                die(Erange);
              }
              fuzz = i;
            } else {
              die(Eform);  // invalid subkeyword
            }
          }
          break;
        }
        case THEN: {  // can't have THEN in the middle of a program
          die(Ethen);
        }
        case TRACE: {  // get the data and set trcflag as appropriate
          tmpchr = 0;
          if (*lineptr) {
            gettoken(lineptr, &tmpchr, varname, maxvarname, 1);
            lineptr += tmpchr;
          } else {
            varname[0] = 0;
          }
          if ((!(trcflag & Tinteract) && interact < 0) || (interact == interplev - 1 && interact >= 0)) {
            // if interactive trace is on, do not interpret any trace instruction except in the actual command.
            // moreover, use the saved trace flag as the initial value of trcflag.
            // this trace instruction makes the program continue operating (trclp=0).
            if (interact >= 0) {
              trclp = 0;
              trcflag = otrcflag;
            }
            settrace(varname);
          }
          break;
        }
        case DROP: {  // go along the list, setting each variable to a null value (with length -1); varset() does the DROP.
          i = 1;
          while (i || (c = *lineptr) == ' ' || c == '(') {
            if (!i && c != '(') {
              lineptr++;
            }
            i = 0;
            if ((c = *lineptr) == '(') {
              lineptr++;
            }
            tmpchr = 0;
            getvarname(lineptr, &tmpchr, varname, &varlen, maxvarname);
            lineptr += tmpchr;
            if (!varname[0]) {
              die(Enosymbol);
            }
            if (c == '(') {  // drop a list of variables
              if (lineptr++[0] != ')') {
                die(Elpar);
              }
              if ((c = *lineptr) && c != ' ') {   // space is not required,
                i = 1;  // but if omitted remember not to skip it
              }
              exp = varget(varname, varlen, &len);
              tmpchr = 0;  // prepare to parse the list of symbols
              if (exp && len > 0) {
                mtest(workptr, worklen, len + 1, len - worklen + 1);
                for (c = 0, l = 0; l < len; l++) {  // copy the list in uc
                  if (!c && ((c2 = exp[l]) == '\'' || c2 == '\"')) {
                    c = c2;
                  } else if ((c2 = exp[l]) == c) {
                    c = 0;  // c is quote flag
                  }
                  workptr[l] = c ? c2 : uc(c2); // uppercase and copy
                }
                if (c) {
                  die(Equote);
                }
                workptr[len] = 0;  // Now add a terminator
                while (l || workptr[tmpchr] == ' ') {
                  l = 0;
                  while (workptr[tmpchr] == ' ') {
                    tmpchr++;
                  }
                  getvarname(workptr, &tmpchr, varname, &varlen, maxvarname);
                  if (!varname[0]) {
                    die(Enosymbol);
                  }
                  varset(varname, varlen, cnull, -1);
                }
                // should now have reached the end of the list
                if (tmpchr != len) {
                  die(Enosymbol);
                }
              }
            } else {  // don't remove this "else" !!
              varset(varname, varlen, cnull, -1);
            }
          }
          break;
        }
        case ADDRESS: {  // get parameter; perhaps follwed by a command
          if (*lineptr) {  // Something follows...
            tmpchr = 0;
            i = gettoken(lineptr, &tmpchr, varname, maxvarname, 1);
            lineptr += tmpchr;
            if (strlen(varname) > maxenviron) {
              die(Elong);
            }
            env = envsearch(varname);
            if (env < 0) {
              die(Emem);
            }
          } else {
            i = -1;
          }
          if (*lineptr == ' ') {
            lineptr++;  // environment may be followed by a space
          }
          if (!*lineptr) {  // permanent env change
            l = address1;
            address1 = address2;
            address2 = l;    // swap buffers
            if (i >= 0) {
              address1 = env;  // copy given value
            }
            break;
          }
          if (!i) {
            break;  // error: No command follows "ADDRESS VALUE"
          }
          doaddress(&lineptr, env);   // do the following command in given environment
          break;
        }
        case PUSH:
        case QUEUE: {
          die(Eexist);  // not implemented here !!!
        }
        // anything else is a syntax error; however, under normal circumstances we should never get here.
        default: {
          die(Esyntax);
        }
      }
    } else { // (*lineptr)
      // the instruction starts with a printable character.
      // try an assignment, and then a command to the environment.
      varname[0] = 0;
      if (rexxsymbol(c = *lineptr) == 1) {  // the character is the start of a symbol
        tmpchr = 0;
        getvarname(lineptr, &tmpchr, varname, &l, maxvarname);
        if (lineptr[tmpchr] == '=') {  // it is an assignment if the next character is '='
          tmpchr++;
          exp = scanning(lineptr, &tmpchr, &len);
          lineptr += tmpchr;
          varset(varname, l, exp, len);
        } else {
          varname[0] = 0;
        }
      }
      // if no assignment was found it must be a command
      if (!varname[0]) {
        doaddress(&lineptr, address1);
      }
    }
    // end of processing for each clause.
    // now if chkend is set, we need to check for a clause terminator and step to the next statement.
    // if chkend is not set, we are already pointing to the next clause.
    doconds();  // test and carry out any signals
    if (chkend) {
      if ((c = *lineptr)) {
        die(Edata);  // if end-of-line not found, error
      }
      if (trcflag & Tclauses) {
        printstmt(ppc, 1, 0);  // trace intervening comments
      }
      ppc++;
    }
  }
  anslen[0] = -1;
  return cnull;  // end of program, so return
}

static void doaddress(char **lineptr, int env) {
  char *cmd;
  char *cmdcopy;
  char *ans;
  int anslen;
  int len;
  int i;
  int code;

  if (trcflag & Tcommands) {  // trace command before interpretation
    printstmt(ppc, 0, 0);
  }
  i = 0;  // the lineptr points to a command to be  executed in an environment
  cmdcopy = scanning(*lineptr, &i, &len);  // get a copy of the command for later
  anslen = cmdcopy - cstackptr;
  rxdup();
  cmdcopy = anslen + cstackptr;
  cmd = delete(&len);  // get the command
  cmdcopy[len] = 0;
  (*lineptr) += i;
  if ((trcflag & Tcommands) || (trcflag & ~Tinteract) == (Tclauses | Tlabels)) {
    // trace command before execution
    traceprefix(prog[ppc].num, "*~*");
    for (i = 0; i < traceindent * pstacklev; i++) {
      tracechar(' ');
    }
    traceput(cmd, len);
    tracechar('\n');
    interactive();
  }
  code = envcall(env, cmd, len, &ans, &anslen);
  if ((code == Efailure && (trcflag & Tfailures)) || (code && (trcflag & (Tclauses | Terrors | Tcommands)))) {
    // trace return code
    if (!(trcflag & (Tcommands | Tclauses))) {
      printstmt(ppc, 0, 0);
    }
    tracestr("      +++ RC=");
    traceput(ans, anslen);
    tracestr(" +++\n");
    interactive();
  }
  rcstringset(code, ans, anslen, code, cmdcopy);  // set RC unless this is an interactive command
}

/* parse the arglist (each argument i of length arglen[i]) by the template written at line+ptr */
static void parse(char *arglist[], int arglen[], int up, char *line, int *ptr) {
  char *srch;            // a string to search for
  int srchlen;           // the length of the search string
  int i = 0;             // which string is being parsed
  int j;                 // the current position within the string (0-based)
  int l;                 // the length of the string being parsed
  int lastexpr;          // the start position of the last expression
  int startvar, lenvar;  // the position of a variable list
  char c;
  int k, m1, e1, z1, l1, pos = 0;

  for (;;) {  // loop for each template separated by commas
    if (arglist[i] == cnull) {  // no strings left, so parse a null string
      l = 0;
    } else {
      l = arglen[i];  // l holds the string length
    }
    j = 0;
    lastexpr = -1;
    for (;;) {
      if (line[*ptr] == ' ') {
        ++ * ptr;
      }
      // a space may separate the previous piece of template from the next
      startvar = *ptr;  // collect space-separated list of symbols or dots
      while (rexxsymbol(c = line[*ptr]) == 1 || (c == '.' && !rexxsymboldot(line[*ptr + 1]))) {
        if (c != '.') {
          skipvarname(line, ptr);
        } else {
          (*ptr)++;
        }
        if (line[*ptr] == ' ') {
          ++ * ptr;
        }
      }
      lenvar = *ptr - startvar;  // we now have the list stored for later
      if (c <= 0 || c == ',') {  // parse rest of line
        pset1(line + startvar, lenvar, arglist[i] + j, l - j, up);
        break;
      }
      if (c == '(') {  // parse expression
        (*ptr)++, srch = scanning(line, ptr, &srchlen);
        if (line[(*ptr)++] != ')') {
          die(Elpar);
        }
      } else if (c == '\'' || c == '\"') {  // parse string literal
        srch = line + ++(*ptr);
        while (line[(*ptr)++] != c || line[*ptr] == c) {
          if (line[*ptr - 1] == c) {
            (*ptr)++;  // search for close quote
          }
        }
        srchlen = (*ptr + line) - srch - 1;
        // stack the string, whether hex, binary or ordinary
        if (line[*ptr] == 'X' && !rexxsymboldot(line[*ptr + 1])) {
          stackx(srch, srchlen);
          (*ptr)++;
        } else if (line[*ptr] == 'B' && !rexxsymboldot(line[*ptr + 1])) {
          stackb(srch, srchlen);
          (*ptr)++;
        } else {
          stackq(srch, srchlen, c);
        }
        srch = delete(&srchlen);
      } else {
        // parse numeric. c holds the sign (+,-,=) if any.
        // stack the number; leave srchlen positive or else get the integer in pos and leave srchlen negative
        if ((c == '+' || c == '-' || c == '=') && line[++*ptr] == '(') {
          ++*ptr;
          scanning(line, ptr, &srchlen);
          if (line[(*ptr)++] != ')') {
            die(Elpar);
          }
          pos = getint(1);
          srchlen = -1;
        } else {
          for (k = *ptr; rexxsymboldot(line[*ptr]); (*ptr)++) {
            // no-op
          }
          if (k == *ptr) {
            die(Eparse);
          }
          stack(srch = line + k, srchlen = *ptr - k);
        }
        if (c == '=' || c == '+' || c == '-' || num(&m1, &e1, &z1, &l1) >= 0) {
          // a number has now been found.
          // it is used as an absolute position, or an offset from the last position, or from the *start* of the previous search string
          if (srchlen >= 0) {
            pos = getint(1);  // now pos holds the number
          }
          k = lastexpr >= 0 ? lastexpr : j;  // k holds the old position
          if (c == '+') {
            j = k;
            k += pos;
          } else if (c == '-') {
            j = k;
            k -= pos;
          } else {
            k = pos - 1;  // absolute positions are 1-based, so decrement
          }
          if (k < 0) {
            k = 0;  // make sure position is within the line
          }
          if (k > l) {
            k = l;
          }
          // now, j holds the old position (i.e. start position), and k holds the new (i.e. stop position).
          if (k <= j) {  // parse from j to end of line
            pset1(line + startvar, lenvar, arglist[i] + j, l - j, up);
          } else {  // parse from j to k
            pset1(line + startvar, lenvar, arglist[i] + j, k - j, up);
          }
          j = k;  // in each case now move to the new position
          lastexpr = -1;  // no previous search string
          continue;
        } else {
          die(Eparse);  // a non-numeric symbol was found
        }
      }
      // now, a search string has been found, and it is stored in srch, and has length srchlen.
      if (srchlen == 0) {  // the null string matches the end of the line.
        k = l;
      } else {
        for (k = j; k <= l - srchlen; k++) {
          for (l1 = 0; l1 < srchlen && uc1(arglist[i][k + l1], up) == srch[l1]; l1++) {
            // do the search
          }
          if (l1 == srchlen) {
            break;
          }
        }
      }
      if (k > l - srchlen) {
        k = l;  // not found, so move to end of line
      }
      pset1(line + startvar, lenvar, arglist[i] + j, k - j, up);
      if (k == l) {
        j = k;
        lastexpr = -1;
      } else {
        j = k + srchlen;
        lastexpr = k;  // move to end of string, but save the start position
      }
    }
    // end of loop: continue round if a comma is found, otherwise break.
    if (line[*ptr] != ',') {
      break;
    }
    (*ptr)++;
    if (arglist[i]) {
      i++;
    }
  }
}

/* Return the uppercase of c, only if up is true. */
static char uc1(char c, int up) {
  if (up) {
    return uc(c);
  }
  return c;
}

/* parse a value with a space-separated list of names */
static void pset1(char *list, int listlen, char *val, int len, int up) {
  static char varname[maxvarname];  // for storing variable names
  int varlen;  // the length of a variable name
  int ptr;

  if (!listlen) {
    return;  // no names - nothing to do
  }
  if (!len) {
    val = "";  // protect against NULL values (omitted arguments)
  }
  while (listlen) {
    varname[0] = varlen = ptr = 0;
    if (list[0] != '.') {  // get the next name, unless we are at "."
      getvarname(list, &ptr, varname, &varlen, maxvarname);
    } else {
      ptr++;
    }
    if (list[ptr] == ' ') {
      ptr++;
    }
    list += ptr;   // step past the name just encountered
    if (listlen -= ptr) {  // not end of name list: return first token stripped
      while (len && val[0] == ' ') {
        val++;
        len--;
      }
      for (ptr = 0; ptr < len && val[ptr] != ' '; ptr++) {
        // no-op
      }
    } else {
      ptr = len;  // return remains of string, unstripped
    }
    pset(varname, varlen, val, ptr, up);
    val += ptr;
    if (len -= ptr) {
      val++;
      len--;  // absorb one space if necessary
    }
  }
}

/* trace and assign a result from the parse command */
static void pset(char *varname, int namelen, char *val, int len, int up) {
  char *sp;                     // Some work space
  static char what[4] = ">>>";  // Trace message prefix
  static char buff[255];        // A fixed length workspace
  int x;

  if (trcflag & (Tresults | Tintermed)) {  // trace the result
    what[1] = (varname[0] ? '=' : '.');
    if (!(up && len)) {
      traceline(what, val, len);
    } else {
      sp = allocm((unsigned) len);
      for (x = 0; x < len; x++) {
        sp[x] = uc(val[x]);
      }
      traceline(what, sp, len);
      free(sp);
    }
  }
  if (varname[0]) {   // assign, unless the name was "."
    if (!(up && len)) {   // straightforward, unless it needs to be uppercased
      varset(varname, namelen, val, len);
    } else {
      sp = (len < 256 ? buff : allocm((unsigned) len));  // make some space
      for (x = 0; x < len; x++) {
        sp[x] = uc(val[x]);  // uppercase into the space
      }
      varset(varname, namelen, sp, len);  // assign the uppercase val
      if (len > 255) {
        free(sp);  // now free the space
      }
    }
  }
}

/*
 * Save the current program,
 * go down stack to find the most recent non-interpreted instruction,
 * and store the proper program in "prog".
 * "level" gets the interplev of this instruction.
 */
static int findsigl(int *level) {
  int sigl;
  int instr;
  int type;
  int len;
  char *ptr;
  int eptr;
  int lev = interplev;

  oldprog = prog;
  oldstmts = stmts;
  sigl = prog[instr = ppc].num;
  for (ptr = pstackptr + (eptr = epstackptr); !sigl && eptr;) {
    type = *((int *) ptr - 1);
    ptr -= (len = *((int *) ptr - 2));  // point to start of entry
    eptr -= len;
    if (type == 14) {
      prog = ((struct interpstack *) ptr)->prg;
      stmts = ((struct interpstack *) ptr)->stmts;
      lev--;
    }
    sigl = prog[instr = ((struct minstack *) ptr)->stmt].num;
  }
  *level = lev;
  return instr;
}

/*
 * This function deletes argc arguments from the current calculator stack
 * and stores their addresses and lengths in the given arrays
 */
static void getcallargs(char *args[], int arglen[], int argc) {
  int i;

  for (i = argc - 1; i >= 0; i--) {
    args[i] = delete(&arglen[i]);
  }
  args[argc] = cnull;
}

/*
 * A 'call' command interpreter.
 * The integer result is 1 if the call returned a value (placed on the calculator stack), 0 otherwise.
 * 'lit' indicates whether (=1) or not (=0) the name was a quoted literal; if lit=1 the internal label table is not searched
 */
int rxcall(int stmt, char *name, int argc, int lit, long calltype) {
  char *lptr;                   // a label pointer
  struct procstack *sptr;       // a program stack item pointer
  int l;
  char *result;                 // the result returned by the subroutine
  int rlen;                     // the length of the result
  char *args[maxargs + 1];      // the arguments given by the CALL instruction
  int arglen[maxargs];          // the lengths of the arguments
  RXSTRING rxargs[maxargs];     // more arguments (terribly inefficient)
  RXSTRING rxresult;            // and another result
  short rxrc;
  int type = 0;                 // the type of a program stack entry
  char file[maxvarname + 5];    // the name of a program file to load
  int ext = 0;                  // whether the subroutine is external or internal
  funcinfo *data;               // data about an already loaded function
  char *callname;               // by what name the subroutine was called
  char **oldcarg = curargs;     // the saved parameters of the current program...
  int *oldcarglen = curarglen;
  char *oldcstackptr;
  char oldtrcres = trcresult;
  long oldsec = timestamp.tv_sec;
  long oldmic = timestamp.tv_usec;
  char *flname;                 // the file name to load
  int w = 0;                    // what kind of file it is
  int saa = 0;                  // whether func is registered as saa
  char c;
  int delay = 0;
  int sigl = 0;                 // line to come from
  int registerit = 0;           // whether this function should be hashed
  int callflags = 0;            // flags for RexxStartProgram

  while (argc && isnull()) {
    argc--;
    delete(&w);  // the last arg should not be null
  }
  if (argc > maxargs) {
    die(Emanyargs);  // too much to handle
  }
  if (!name) {  // called as a condition trap, so no need to search for a label
    delay = lit;  // as a parameter-saving device, the delayed signal was passed as the "lit" parameter.
    name = conditions[delay];   // the real name has been lost; use the condition name.
    if (delay == Ihalt) {
      sigl = haltline;
    }
  } else {
    if (!lit) {
      for (lptr = labelptr; (l = *(int *) lptr) && strcasecmp(name, lptr + 2 * four); lptr += align(l + 1) + 2 * four) {
        // check for internal label
      }
      if (l) {
        stmt = ((int *) lptr)[1];
      }
    }
    if (lit || !l) {  // no label, so try built-in and then external
      if ((l = rxfn(name, argc)) > 0) {
        return 1;  // OK, builtin was executed
      }
      if ((callname = strrchr(name, '/'))) {  // get base name for "callname"
        callname++;
      } else {
        callname = name;
      }
      if ((data = (funcinfo *) hashget(2, callname, &w))) {  // function is hashed
        if (data->dlfunc) {  // function has already been loaded
          if (data->saa) {  // saa calling sequence
            l = funccall((unsigned long (*)()) data->dlfunc, callname, argc);
          } else {
            l = (data->dlfunc) (callname, argc);  // imc calling sequence
          }
          if (l < 0) {
            die(-l);
          }
          return l;
        } else {
          flname = data->name, saa = data->saa;
        }
        if (saa & RXDIGITS) {
          saa &= ~RXDIGITS, callflags |= RXDIGITS;
        }
      } else {   // make the file name in lower case in the workspace
        ext = strlen(name);
        mtest(workptr, worklen, ext + 1, worklen - ext + 1);
        for (l = 0; (c = name[l]); l++) {
          workptr[l] = c >= 'A' && c <= 'Z' ? name[l] | 32 : name[l];
        }
        workptr[l] = 0;
        flname = workptr;
        if (flname[0] != '/') {
          registerit = 1;
        }
      }
      if (!(w = which(flname, 2, file))) {  // search for the file, but...
        sprintf(workptr, ": \'%s\'", name);
        errordata = workptr;
        die(Eundef);  // die if not found
      }
      if (registerit) {
        funcinit(name, (void *) file, (int (*)()) 0, saa);
      }
      if (w == 1) {  // the file is a Rexx program, so start it
        for (l = argc - 1; l >= 0; l--) {
          rxargs[l].strptr = delete(&w);
          if (w >= 0) {
            rxargs[l].strlength = w;
          } else {
            rxargs[l].strptr = 0;
            rxargs[l].strlength = -1;
          }
        }
        rxresult.strptr = 0;
        l = RexxStartProgram((char *) 0, (long) argc, rxargs, file, callname, (RXSTRING *) 0, envtable[address0].name, calltype, callflags | RXEXITS, (PRXSYSEXIT) 0, &rxrc, &rxresult);
        if (l == -Ehalt) {
          die(Ehalt);
        } else if (l == -Esig) {
          longjmp(*exitbuf, Esig);
        } else if (l) {
          die(Eincalled);
        }
        if (!rxresult.strptr) {
          return 0;
        }
        stack(rxresult.strptr, rxresult.strlength);
        free(rxresult.strptr);
        return 1;
      } else if (w == 3) {  // the file is a Unix program
        return unixcall(file, callname, argc);
      }
    }
  }
  // the subroutine is Rexx and stmt is the statement to go to; now set SIGL as appropriate
  l = findsigl(&rlen);
  l = prog[l].num;   // get the "real" program and find line
  if (!sigl) {
    sigl = l;  // set SIGL unless it was already given by a "halt"
  }
  sprintf(file, "%d", sigl);
  varset("SIGL", 4, file, strlen(file));   // ("file" is unused in this case)
  getcallargs(args, arglen, argc);
  oldcstackptr = cstackptr;
  cstackptr = allocm(100);
  // we now stack a program stack item...
  sptr = (struct procstack *) pstack(11, sizeof(struct procstack2));
  sptr->csp = oldcstackptr;
  sptr->ecsp = ecstackptr;
  sptr->csl = cstacklen;
  sptr->trc = trcflag;
  sptr->tim = timeflag;
  sptr->mic = microsecs;
  sptr->sec = secs;
  sptr->address1 = address1;
  sptr->address2 = address2;
  sptr->form = numform;
  sptr->digits = precision;
  sptr->fuzz = fuzz;
  sptr->stmts = oldstmts;
  sptr->prg = oldprog;
  cstacklen = 100;   // we allocated the new stack earlier (can't think why...)
  ecstackptr = 0;   // clear the stack now
  if (++interplev >= sigstacklen) {  // we might need more space on the sgstack
    if (!(sgstack = (struct sigstruct *) realloc((char *) sgstack, sizeof(struct sigstruct) * (sigstacklen += 10)))) {
      die(Emem);
    }
  }
  trcresult = 0;
  result = interpreter(&rlen, stmt, name, calltype, args, arglen, 1, delay);
  // now, clean up, reclaim all the new structures, delete the program stack entry, replace the old values of certain things, etc
  trcresult = oldtrcres;
  interplev--;
  oldcstackptr = cstackptr;
  timestamp.tv_sec = oldsec;
  timestamp.tv_usec = oldmic;
  while (type < 11 || type > 12) {  // clear up all entries until ours
    type = unpstack();
    sptr = (struct procstack *) delpstack();
  }
  cstackptr = sptr->csp;
  ecstackptr = sptr->ecsp;
  cstacklen = sptr->csl;
  trcflag = sptr->trc;
  timeflag = (timeflag & 4) | (sptr->tim & 3);
  microsecs = sptr->mic;
  secs = sptr->sec;
  address1 = sptr->address1;
  address2 = sptr->address2;
  numform = sptr->form;
  precision = sptr->digits;
  fuzz = sptr->fuzz;
  if (result) {
    stack(result, rlen);
  }
  free(oldcstackptr);   // now the result has been used, free the old stack
  stmts = sptr->stmts;
  prog = (sptr->prg);
  if (type > 11) {  // reclaim procedural variables
    varstkptr--;
  }
  curargs = oldcarg;
  curarglen = oldcarglen;
  ppc = newppc;
  if (rlen < 0) {  // the program fell off the end, so EXIT
    returnfree = 0;
    returnval = 0;
    while (pstacklev) {
      type = unpstack();
      freestack(delpstack(), type);
    }
    longjmp(sgstack[interplev].jmp, -1);  // interplev=0 I hope...
  }
  return result != cnull;
}

/*
 * A function to execute the `interpret' command.
 * The return is either null, or a pointer to a result string which was given in a RETURN instruction.
 */
char *rxinterp(char *exp, int len, int *rlen, char *name, long calltype, char *args[], int arglen[]) {
  // void process();          // the tokeniser used by load()
  struct interpstack *sptr;   // a program stack item pointer
  char *result;               // the result to be returned, if any
  int type = 0;

  if (!len) {   // interpret null string is OK immediately
    *rlen = -1;
    return cnull;
  }
  result = allocm(len + 1);
  memcpy(result, exp, len);
  result[len] = '\n';
  // tokenise...
  oldstmts = stmts;
  oldprog = prog;
  ippc = ppc;
  interpreting = 1;
  tokenise(result, len + 1, 1, 0);
  interpreting = 0;
  ppc = ippc;
  // fill in a program stack entry
  sptr = (struct interpstack *) pstack(14, sizeof(struct interpstack));
  sptr->stmts = oldstmts;
  sptr->prg = oldprog;
  ecstackptr = 0;
  if (++interplev >= sigstacklen) {  // might need some more space for the sgstack
    if (!(sgstack = (struct sigstruct *) realloc((char *) sgstack, sizeof(struct sigstruct) * (sigstacklen += 10)))) {
      die(Emem);
    }
  }
  // this is where the string gets interpreted
  result = interpreter(rlen, 1, name, calltype, args, arglen, 1, 0);
  // if it returned with RETURN it could be within DO structures, etc which should be removed from the stack.
  type = unpstack();
  if (*rlen >= 0) {
    while (type != 14) {
      delpstack();
      type = unpstack();
    }
  } else { // otherwise all structures should be complete
    if (type != 14) {
      die(Enoend);
    }
  }
  interplev--;
  sptr = (struct interpstack *) delpstack();
  ppc = newppc;
  free(prog[0].source);   // the interpreted string
  free(prog[0].line);   // the tokenised string
  free((char *) prog);   // the statement table
  stmts = ((struct interpstack *) sptr)->stmts;
  prog = ((struct interpstack *) sptr)->prg;
  return result;
}

/* check for delayed conditions and trap them */
static void doconds() {
  int cond;
  struct errorstack *tmpptr;
  int len;

  for (cond = 0; cond < Imax; cond++) {
    if (delayed[cond]) {
      if ((sgstack[interplev].callon & (1 << cond)) && !(sgstack[interplev].delay & (1 << cond))) {
        delayed[cond] = 0;
        if (sgstack[interplev].ppc[cond] < 0) {  // report an undefined label
          tmpptr = (struct errorstack *) pstack(20, sizeof(struct errorstack));
          tmpptr->prg = prog;
          tmpptr->stmts = stmts;
          ppc = -sgstack[interplev].ppc[cond];
          findsigl(&cond);
          errordata = 0;
          die(Elabel);
        }
        // now call the condition routine
        if (rxcall(sgstack[interplev].ppc[cond], cnull, 0, cond, RXSUBROUTINE)) {
          delete(&len);  // Ignore the return value
        }
        cond--;  // check this signal again
      } else if (cond != Ihalt) {
        delayed[cond] = 0;  // cancel delayed conditions
      }
    }
  }
  // check for interruption
  if (delayed[Ihalt] && !(sgstack[interplev].delay & (1 << Ihalt))) {
    delayed[Ihalt] = 0, die(Ehalt);
  }
}

/* sets the trace flag according to the given option */
void settrace(char *option) {
  char c;

  if (!*option) {
    otrcflag = trcflag = Tfailures;
    return;
  }
  while ((c = *option++) == '?') {
    trcflag ^= Tinteract;
  }
  interactmsg = (trcflag & Tinteract);
  switch (c & 0xdf) {
    case 'A': c = Tclauses;                  break;
    case 'C': c = Tcommands | Terrors;       break;
    case 'E': c = Terrors;                   break;
    case 'F': c = Tfailures;                 break;
    case 'I': c = Tclauses | Tintermed;      break;
    case 'L': c = Tlabels;                   break;
    case 'N': c = Tfailures;                 break;
    case 'O': c = trcflag = interactmsg = 0; break;
    case 'R': c = Tclauses | Tresults;       break;
    case 0:   break;
    default:  die(Etrace);
  }
  otrcflag = trcflag = (trcflag & Tinteract) | c;
}

/*
 * Interpret an option from the OPTIONS instruction or a commandline parameter.
 * Return 1 if the option was processed.
 * This routine does not raise errors.
 */
int setoption(char *option, int len) {
  static char buffer[maxvarname];
  char *ptr = memchr(option, '=', len);
  FILE *fp;
  int equals = ptr ? ptr - option : 0;

  if (len >= maxvarname) {
    return 0;
  }
  if (equals >= 5 && !strncasecmp(option, "tracefile", equals)) {
    option += ++equals;
    len -= equals;
    if (!len || memchr(option, 0, len)) {
      return 0;
    }
    if (option[0] == '\'' || option[0] == '\"') {
      if (option[len - 1] != option[0]) {
        return 0;
      }
      option++;
      len -= 2;
    }
    memcpy(buffer, option, len);
    buffer[len] = 0;
    if (!strcmp(buffer, "stdout")) {
      fp = stdout;
    } else if (!strcmp(buffer, "stderr")) {
      fp = stderr;
    } else if (!(fp = fopen(buffer, "a"))) {
      perror(buffer);
    }
    if (fp) {
      if (traceout && traceout != stderr && traceout != stdout) {
        fclose(traceout);
      }
      traceout = fp;
      printf("Writing trace output to %s\n", buffer);
    }
    return 1;
  }
  if (len == 5 && !strncasecmp(option, "setrc", len)) {
    return setrcflag = 1;
  }
  if (len == 7 && !strncasecmp(option, "nosetrc", len)) {
    return setrcflag = 0, 1;
  }
  if (len <= 6 && len >= 3 && !strncasecmp(option, "expose", len)) {
    return exposeflag = 1;
  }
  if (len <= 8 && len >= 5 && !strncasecmp(option, "noexpose", len)) {
    return exposeflag = 0, 1;
  }
  if (len <= 7 && len >= 4 && !strncasecmp(option, "sigpipe", len)) {
    return sigpipeflag = 1;
  }
  if (len <= 9 && len >= 6 && !strncasecmp(option, "nosigpipe", len)) {
    return sigpipeflag = 0, 1;
  }
  return 0;
}

/* get a trap name after "call/signal on"; return the trap number */
static int gettrap(char **lineptr, int on, int *stmt) {
  int l = 0;
  int i;
  int *lptr;
  int tmpchr = 1;

  gettoken(*lineptr, &tmpchr, varnamebuf, varnamelen, 0);
  lineptr[0] += tmpchr;
  for (i = 0; i < Imax && strcasecmp(varnamebuf, conditions[i]); i++) {
    // no-op
  }
  if (i == Imax) {
    die(Etrap);
  }
  if (on && **lineptr == NAME) {
    tmpchr = 1, gettoken(*lineptr, &tmpchr, varnamebuf, varnamelen, 0);
    if (!varnamebuf[0]) {
      die(Enostring);
    }
    lineptr[0] += tmpchr;
  }
  // varnamebuf now holds the name to go to on error
  if (on) {
    for (lptr = (int *) labelptr; (l = *lptr) && strcasecmp(varnamebuf, (char *) (lptr + 2)); lptr += 2 + align(l + 1) / four) {
      // no-op
    }
    if (l) {
      l = lptr[1];  // l holds the stmt to go to on error
    }
  }
  *stmt = l;
  return i;
}

/*
 * Check that any symbol in the current line, pointed to by lineptr,
 * matches the stored control variable name, var, of length len.
 */
static void testvarname(char **lineptr, char *var, int len) {
  char c;
  char *varref;
  int reflen;

  if ((c = **lineptr)) {  // if the symbol name is supplied:
    if (c < 0) {
      die(Exend);  // die if it is a keyword [SELECT]
    }
    if (rexxsymbol(c) < 1) {
      die(Enosymbol);  // or an invalid symbol
    }
    varref = *lineptr;  // save start addr of symbol
    reflen = 0;
    skipvarname(*lineptr, &reflen);   // go to end of symbol
    if (len != reflen || memcmp(varref, var, len)) {
      die(Exend);  // die if it is the wrong symbol
    }
    lineptr[0] += reflen;
  }
}

/* Skips the current instruction */
static void skipstmt() {
  if (ppc == stmts) {
    die(Enoend);
  }
  switch (prog[ppc].line[0]) {  // test for block instructions
    case DO:       stepdo();     return;
    case SELECT:   stepselect(); return;
    case IF:       stepif();     return;
    case WHEN:     stepwhen();   return;
    default:       ppc++;        return;  // skip one statement
  }
}

/*
 * The following functions, stepdo(), stepselect(), stepif() and stepwhen(),
 * do the work of skipstmt() in the special cases of DO, SELECT and IF instructions.
 */

static void stepdo() {
  pstack(0, sizeof(struct minstack));
  // in case of error, report loop start as well as end
  if (++ppc == stmts) {
    die(Enoend);  // go past DO
  }
  while (prog[ppc].line[0] != END) {
    skipstmt();  // find END
  }
  if (prog[ppc].line[1] < 0) {
    die(Exend);  // report error for "END SELECT"
  }
  delpstack();
  ppc++;  // go past END
}

static void stepselect() {
  char c;

  pstack(0, sizeof(struct minstack));
  if (++ppc == stmts) {
    die(Enoend);  // go past SELECT
  }
  while (prog[ppc].line[0] != END) {
    skipstmt();  // find END
  }
  if ((c = prog[ppc].line[1]) && c != SELECT) {
    die(Exend);  // report error for "END x"
  }
  delpstack();
  if (++ppc == stmts) {
    die(Enoend);  // go past END
  }
}

static void stepif() {
  if (++ppc == stmts) {
    die(Enoend);  // go past IF
  }
  if (prog[ppc].line[0] != THEN) {
    die(Enothen);  // find THEN
  }
  if (++ppc == stmts) {
    die(Enoend);  // go past THEN
  }
  skipstmt();  // skip the statement after THEN
  if (prog[ppc].line[0] == ELSE) {  // an ELSE clause is optional
    if (++ppc == stmts) {
      die(Enoend);  // go past ELSE
    }
    skipstmt();  // skip the statement after ELSE
  }
}

static void stepwhen() {
  if (++ppc == stmts) {
    die(Enoend);  // go past WHEN
  }
  if (prog[ppc].line[0] != THEN) {
    die(Enothen);  // find THEN
  }
  if (++ppc == stmts) {
    die(Enoend);  // go past THEN
  }
  skipstmt();  // skip the statement after THEN
}

/* This function is called inside a SELECT, LEAVE or ITERATE to find the closing END statement. */
static void findend() {
  while (prog[ppc].line[0] != END) {
    skipstmt();
  }
}

/* when a halt occurs, this function is called to set the haltline variable. */
void on_halt() {
  int errstmt;
  int dummy;

  if (prog) {
    errstmt = findsigl(&dummy);
    haltline = prog[errstmt].num;  // find the line number at which halt occurred
    prog = oldprog;
    stmts = oldstmts;
  }
}

