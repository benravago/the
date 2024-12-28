/* The calculating routines of REXX/imc         (C) Ian Collier 1992 */

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>

#include "const.h"
#include "rexxsaa.h"
#include "globals.h"
#include "functions.h"

// the following identifiers are arranged in order as defined by the constants OPpower, OPmul, ... , OPggeq.

// binops contains the character codes of all the binary operators,
// and binpri contains their priorities.

static char binops[] = {
  POWER,
  '*', '/',
  '+', '-',
  ' ', CONCAT,
  '=', EQU, '<', LEQ, '>', GEQ, NEQ,
  '&',
  LXOR, '|',
  '%', MOD,
  NNEQ, LESS, GRTR, LLEQ, GGEQ
};
static char binpri[] = {
  10,
  8, 8,
  7, 7,
  6, 6,
  5, 5, 5, 5, 5, 5, 5,
  3,
  2, 2,
  8, 8,
  5, 5, 5, 5, 5
};

// This array contains the addresses of all the functions which are called to implement the various operators,
// in the order such that binprg(OPxyz) is the routine to implement the xyz operator

static void (*binprg[])(char) = {
  binexp,
  binmul, bindiv,
  binplus, binmin,
  bincat, bincat,
  binrel, binrel, binrel, binrel, binrel, binrel, binrel,
  binbool,
  binbool, binbool,
  bindiv, bindiv,
  binrel, binrel, binrel, binrel, binrel,
  unmin, unplus, unnot
};

/*
 * scanning() is the expression evaluator,
 * called 30 times from various parts of the interpreter to collect an expression from a program line.
 *
 * The program line is given as the "line" parameter, and the character pointer as "ptr".
 * An expression will be evaluated and placed on the calculator stack.
 * In addition its address on the stack will be returned and its length will be placed in len.
 * On exit, ptr will point to the character immediately following the expression (it will not point to a space).
 */
char *scanning(char *line, int *ptr, int *len) {
  int lp;
  char *exp;
  int expn;
  int explen;
  int t;
  int n;
  int dot;
  int endnum;
  char quote;
  char varname[maxvarname];
  char *vg;
  char op, pri;
  char ch, c1;

  int intermed = trcflag & Tintermed;   // whether to trace intermediate results
  static char what[4] = ">>>";  // the trace prefix to use

  struct {
    char op;
    char pri;
  } opstack[maxopstack];  // an operation stack

  int opptr = 1;   // the operation stack pointer

  opstack[0].pri = 0;  // the bottom of stack marker
  trcresult++;  // count levels - trace result only on outer level
  for (;;) {  // loop until expression has finished
    lp = 1;
    while (lp) {  // loop until a value has been stacked
      if (opptr >= maxopstack - 1) {
        die(Eopstk);
      }
      switch (line[*ptr]) {
        case ')':
          die(Erpar);
        case ',':
          die(Ecomma);
        case '(':  // parenthesised expressions are stacked by calling scanning() recursively
          (*ptr)++;
          scanning(line, ptr, &explen);
          if (line[(*ptr)++] != ')') {
            die(Elpar);  // it must end with ')'
          }
          lp = 0;
          what[1] = 0;  // prevent the value from being traced, since scanning() already traced it
          break;
        case '+': // for unary '+' stack code OPplus, priority 11
          (*ptr)++,
          opstack[opptr].op = OPplus;
          opstack[opptr++].pri = 11;
          break;
        case '-':  // for unary '-' stack code OPneg, priority 11
          (*ptr)++,
          opstack[opptr].op = OPneg;
          opstack[opptr++].pri = 11;
          break;
        case '\\':  // for '\' stack code OPnot, priority 11
          (*ptr)++,
          opstack[opptr].op = OPnot;
          opstack[opptr++].pri = 11;
          break;
        case '\'':  // quoted expression...
        case '\"':
          quote = line[(*ptr)++];
          expn = *ptr;
          while (line[expn++] != quote || line[expn] == quote) {
            if (line[expn - 1] == quote) {
              expn++;  // search for close quote
            }
          }
          if (line[expn] == 'X' && !rexxsymboldot(line[expn + 1])) {
            stackx(line + *ptr, expn - *ptr - 1);  // stack hex
            expn++;
          } else {
            if (line[expn] == 'B' && !rexxsymboldot(line[expn + 1])) {
              stackb(line + *ptr, expn - *ptr - 1);  // stack bin
              expn++;
            } else {
              stackq(line + *ptr, expn - *ptr - 1, quote);  // stack string
            }
          }
          // a string constant has been found,
          // but if it is followed by '(' it is a function call which bypasses the internal label search.
          if (line[expn] == '(') {
            (*ptr) = expn + 1;
            exp = delete(&n);
            if (n > maxvarname - 1) {
              die(Elong);
            }
            memcpy(varname, exp, n);  // the function name is stored in varname
            t = 1;
            goto rxfncall;
          }
          (*ptr) = expn;  // step past string constant in line.
          lp = 0;   // signal "stacked a value"
          what[1] = 'L';
          break;
        default: { // stack a variable or literal or call a function.
          if (line[*ptr] < 0) {
            die(Ebadexpr);
          }
          if (!(t = rexxsymbol(line[*ptr])) && line[*ptr] != '.') {
            die(Ebadexpr);
          }
          what[1] = 'L';
          if (t != 1) {
            // a constant symbol; special processing is required
            // to ensure that if the symbol is a number in exponential format
            // then any "+" or "-" in the number is treated as part of it
            dot = 0;
            // dot is the count of dots in the number;
            n = 1;
            // n is a flag meaning:
            //   0 an ordinary constant symbol,
            //   1 an exponent is allowed,
            //   2 this is the character after "E".
            endnum = 0;
            // endnum is the position of any "+" or "-" in case the exponent is badly formed (e.g. in 1e+23.4)
            for (expn = *ptr;; expn++) {
              c1 = line[expn];
              if (!n) { // stop whenever a non-symbol character is found
                if (!rexxsymboldot(c1)) {
                  // but remove a "+" or "-" if the exponent was empty
                  if (endnum && endnum + 1 == expn) {
                    expn = endnum;
                  }
                  break;
                } else {
                  if (endnum && (c1 < '0' || c1 > '9')) {
                    expn = endnum;
                    break;  // remove a non-numeric exponent following a sign
                  }
                }
              } else {
                if (alphanum(c1) < 2) { // not number or dot
                  if (n == 2 && (c1 == '-' || c1 == '+')) {
                    n = 0;  // OK to have + or - after an E
                    endnum = expn;
                    continue;
                  }
                  if (n == 1 && c1 == 'E') {
                    n = 2;  // expect an optional sign next
                    continue;
                  }
                  n = 0;  // it's not a number any more
                  if (!rexxsymboldot(c1)) {
                    break;  // allow only symbol characters
                  }
                }
                if (n == 2) {
                  n = 0;  // no sign after the 'E'
                } else if (c1 == '.' && dot++ > 0) {
                  n = 0;  // a second dot found
                }
              }
            }
            if (expn == *ptr) {
              die(Ebadexpr);  // the symbol has zero length
            }
            stack(line + *ptr, expn - *ptr);  // stack the constant symbol
            (*ptr) = expn;  // step past it.
          } else {
            // a symbol follows.
            // if a '(' follows the symbol, then it is a function call
            n = 0;
            t = 0;  // t=0 -> search internal definitions
            for (expn = *ptr; rexxsymboldot(varname[n++] = line[expn++]);) {
              if (n >= maxvarname - 1) {
                die(Elong);
              }
            }
            // the symbol has been copied to varname
            if (--n > 0 && varname[n - 1] != '.' && varname[n] == '(') {
              *ptr = expn;  // step past the symbol
              // a function call has now been found.
              // varname holds its name, and n holds its length.
              // t!=0 if the name was in quotes, t=0 otherwise.
        rxfncall: // goto
              varname[n] = 0;  // the name is nul-terminated (it cannot contain a nul character)
              n = 0;
              if (line[*ptr] != ')') {  // unless no arguments given...
                for (;;) {  // get each one by calling scanning()
                  if ((ch = line[*ptr]) == ',' || ch == ')') {
                    stacknull();
                  } else {
                    scanning(line, ptr, &explen);
                  }
                  n++;
                  if ((ch = line[*ptr]) == ',') {
                    ++ * ptr;
                  } else {
                    if (ch != ')') {
                      die(Elpar);
                    } else {
                      break;
                    }
                  }
                }
              }
              ++*ptr;  // step past the ')'.  n contains the arg count.
              if (!rxcall(0, varname, n, t, RXFUNCTION)) {  // this is where the function gets called
                die(Enoresult);
              }
              lp = 0;
              what[1] = 'F';
              break;
            }
            // else ignore the result of the search for a function name
            // and try to get a variable instead
            getvarname(line, ptr, varname, &explen, maxvarname);
            vg = varget(varname, explen, &expn);
            if (vg == cnull) {  // see if novalue errors are caught
              if ((varname[0] & 128) && !memchr(varname, '.', explen)) {
                varname[explen++] = '.';  // add a dot to undefined stem
              }
              varname[0] &= 127;  // if OK stack the variable's name
              varname[explen] = 0;
              if ((sgstack[interplev].bits & (1 << Inovalue)) && (interact < 0 || interact + 1 != interplev)) {
                errordata = varname;
                die(Enovalue);
              }
              stack(varname, explen);
            } else {
              what[1] = 'V';
              stack(vg, expn);  // it was found
            }
          }
          lp = 0;
        } // default
      }
    }
    if (intermed && what[1]) {
      tracelast(what);
    }
    // at this point a (possibly null) list of operators and their priorities have been stacked,
    // and a value has just been placed on the calculator stack.
    // the next character will either be a terminator or an (explicit or implicit) binary operator.
    // special case: if it is a unary operator (i.e. "logical not"), then it is a syntax error.
    ch = line[*ptr];
    if (ch == '\\') {
      die(Ebadexpr);
    }
    if ((c1 = line[*ptr]) == -1 || !c1 || c1 == ')' || c1 == ',' || c1 < SYMBOL) {
      pri = 0;
    }
    // terminators are ';', 'EOL', ')', ',' and all `words'.
    // priority 0 signals that a terminator was found.
    else {
      (*ptr)++;  // go past the op
      for (op = 0; binops[(int)op] != ch && op < 24; op++) {
        // "op" holds its index
      }
      if (op < 24) {
        pri = binpri[(int)op];  // "pri" holds its priority
      } else
        (*ptr)--;   // The char is not a binary operator, so it must be an implicit concatenation, priority 6.
        op = OPcat;
        pri = 6;
    }
    // having found the next operator and its priority (priority 16 highest, 0 meaning no further operators),
    //   we now examine previous operations to see whether they should be done now.
    // if not, another value is stacked unless the priorities of both the current operator
    //   and the top stacked operator are zero, in which case evaluation has finished.
    while (opstack[opptr - 1].pri >= pri && opstack[opptr - 1].pri) {
      opptr--;
      eworkptr = 0;
      binprg[(int)opstack[opptr].op] (opstack[opptr].op);  // this does the op
      if (intermed) {
        what[1] = (opstack[opptr].op > 23 ? 'P' : 'O');  // trace the op's result
        tracelast(what);
      }
    }
    if (!pri) {
      break;
    }
    // the binary operator just encountered is stacked before finding the next value.
    opstack[opptr].op = op;
    opstack[opptr++].pri = pri;
  }
  // evaluation has finished, so the top stack value is returned.
  (*len) = *((int *) (cstackptr + ecstackptr) - 1);
  if (!--trcresult && (trcflag & Tresults)) {  // trace the result
    tracelast(">>>");
  }
  if (!trcresult) {
    timeflag &= (~2);  // clear timestamp after a result
  }
  return cstackptr + ecstackptr - align(*len) - four;
}

/* trace the last value on the stack */
void tracelast(char *type) {
  char *exp = cstackptr + ecstackptr - four;
  int len = *(int *) exp;

  exp -= align(len);
  traceline(type, exp, len);
}

/* trace a result or other string */
void traceline(char *type, char *exp, int len) {
  int i;

  traceput("      ", 6);
  traceput(type, 3);
  traceput("   ", 3);
  for (i = 0; i < traceindent * pstacklev; i++) {
    tracechar(' ');
  }
  tracechar('"');
  traceput(exp, len);
  traceput("\"\n", 2);
}

/* stack a copy of exp whose length is explen */
void stack(char *exp, int len) {
  int alen = align(len);

  mtest(cstackptr, cstacklen, ecstackptr + alen + 2 * four, len + 256);
  memcpy(cstackptr + ecstackptr, exp, len);  // the string goes on first
  ecstackptr += alen;   // it is padded into alignment
  (*(int *) (cstackptr + ecstackptr)) = len;  // the length is then appended.
  ecstackptr += four;
}

/*
 * Stack a copy of exp whose length is explen, reducing double quotes to single quotes
 * The type of quote mark to reduce from double to single
 * It is guaranteed that this always occurs in pairs
 */
void stackq(char *exp, int len, char quote) {
  int i = 0, l = 0;
  char *p;
  char c;

  mtest(cstackptr, cstacklen, ecstackptr + len + 3 * four, len + 256);
  for (p = cstackptr + ecstackptr; i < len; i++) {   // copy the string
    p[l] = c = exp[i];
    l++;
    if (c == quote) {
      i++;  // omit the next character after a quote.
    }
  }
  ecstackptr += align(l);   // pad the string
  (*(int *) (cstackptr + ecstackptr)) = l;  // and append the length.
  ecstackptr += four;
}

/* Interpret exp (whose length is len) as a hex constant and stack it */
void stackx(char *exp, int len) {
  int l = 0, o;
  unsigned char m, n;
  char d;

  mtest(cstackptr, cstacklen, ecstackptr + len / 2 + 3 * four, len / 2 + 256);
  /* while(exp[0]==' '&&len)exp++,len--;  >>> leading spaces OK if uncommented */
  if (len && (exp[0] == ' ' || exp[0] == '\t')) {
    die(Ehex);  // leading spaces not OK
  }
  for (o = 0; o < len && exp[o] != ' ' && exp[o] != '\t'; o++) {
    // find length of first chunk
  }
  (o % 2) ? (o = 1) : (o = 2);  // if odd, the first hex byte has 1 digit, otherwise 2
  while (len) {
    while ((exp[0] == ' ' || exp[0] == '\t') && len) {  // skip spaces
      exp++;
      len--;
    }
    /* if(len==0)break;  >>> OK for trailing blanks if uncommented */
    if (len < o) {
      die(Ehex);  // error if less than a whole byte exists
    }
    for (m = (n = 0); m < o; m++) {  // for one byte...
      d = (*(exp++)) - '0';   // convert a digit to hex
      len--;
      if (d < 0) {
        die(Ehex);
      }
      if (d > 9) {
        if ((d -= 7) < 10) {
          die(Ehex);
        }
      }
      if (d > 15) {
        if ((d -= 32) < 10) {
          die(Ehex);
        }
      }
      if (d > 15) {
        die(Ehex);
      }
      n = n * 16 + d;  // and accumulate
    }
    o = 2;  // each byte except the first has 2 digits
    cstackptr[ecstackptr++] = n;  // stack each byte
    l++;
  }
  ecstackptr += toalign(l);  // pad the string
  (*(int *) (cstackptr + ecstackptr)) = l;  // and append the length.
  ecstackptr += four;
}

/* Interpret exp as a binary constant and stack it */
void stackb(char *exp, int len) {
  int l;
  int al = align(len / 8 + 1);  // maximum amount of space needed
  unsigned char c = 0, n, b, d;

  if (!len) {
    stack(exp, len);
    return;  // ''b is allowed
  }
  mtest(cstackptr, cstacklen, ecstackptr + al + 2 * four, al + 256);
  if (len && (exp[0] == ' ' || exp[0] == '\t')) {
    die(Ebin);  // leading spaces not OK
  }
  for (l = b = 0; l < len; l++) {
    b += (exp[l] != ' ' && exp[l] != '\t');  // find number of digits (nonblanks)
  }
  l = 0;
  n = (((b - 1) % 8) >= 4) + 1;  // number of nybbles in first byte
  b = (b - 1) % 4 + 1;  // number of bits in first nybble
  while (len) {
    while ((exp[0] == ' ' || exp[0] == '\t') && len) {  // skip spaces
      exp++;
      len--;
    }
    if (len < b) {
      die(Ehex);  // error if less than one nybble exists
    }
    while (b--) {  // for each bit of the nybble...
      d = exp++[0] - '0';
      if (d > 1) {
        die(Ebin);
      }
      c = (c << 1) | d;  // add to the current character
      len--;
    }
    b = 4;  // next nybble has 4 bits
    if (!--n) {  // a byte was completed
      cstackptr[ecstackptr++] = c;  // stack each byte
      l++;
      n = 2;  // next byte has 2 nybbles
    }
  }
  if (n != 2) {
    die(Ebin);  // half a byte was encountered
  }
  ecstackptr += toalign(l);  // pad the string
  (*(int *) (cstackptr + ecstackptr)) = l;  // and append the length.
  ecstackptr += four;
}

/* stack an integer i */
void stackint(int i) {
  char num[20];

  sprintf(num, "%d", i);
  stack(num, strlen(num));
}

/* Stack a null - i.e. a value with length -1 */
void stacknull() {
  mtest(cstackptr, cstacklen, ecstackptr + 2 * four, 256);
  (*(int *) (cstackptr + ecstackptr)) = -1;
  ecstackptr += four;
}

/*
 * The various binary and unary operators follow.
 * Each one operates on the top 1 or 2 values on the calculator stack, deletes them, and stacks a result.
 * The single parameter to each routine is the operator number (e.g. OPplus),
 * which serves to distinguish between two or more operators implemented by the same routine.
 * Some routines do not use the operator number.
 * Formatting of the result of arithmetic operators, including rounding to the required precision, is handled by stacknum().
 */

/* Implements OPadd - the binary + operator */
void binplus(char op) {
  int n1, n2, n3;
  int m1, m2;
  int z1, z2;
  int e1, e2, e3;
  int l1, l2, l3;
  int i;
  int c = 0;
  int d1, d2;

  // The two numbers are fetched and deleted from the stack
  if ((n2 = num(&m2, &e2, &z2, &l2)) < 0) {
    die(Enum);
  }
  delete(&l3);
  if ((n1 = num(&m1, &e1, &z1, &l1)) < 0) {
    die(Enum);
  }
  delete(&l3);
  if (z1 && z2) {
    stack("0", 1);
    return;
  }
  if (z1) {
    stacknum(workptr + n2, l2, e2, m2);
    return;
  }
  if (z2) {
    stacknum(workptr + n1, l1, e1, m1);
    return;
  }
  if (e1 < e2) {
    n3 = n2; n2 = n1; n1 = n3;
    n3 = m2; m2 = m1; m1 = n3;
    e3 = e2; e2 = e1; e1 = e3;
    l3 = l2; l2 = l1; l1 = l3;  // now e1 >= e2 always
  }
  n3 = eworkptr + 1;  // initialise a third number.
  e3 = e1;
  if (m1 == m2) {  // add two numbers; the sign of the result is m1
    l3 = (l1 > l2 + e1 - e2) ? l1 : (l2 + e1 - e2);
    if (l3 > precision + 2) {
      l3 = precision + 2;
    }
    mtest(workptr, worklen, eworkptr + l3 + 2, l3 + 256);  // make space for 3rd num
    for (i = l3 - 1; i >= 0; i--) {
      if (i >= l1) {
        d1 = 0;
      } else {
        d1 = workptr[n1 + i] - '0';  // get each digit from n1
      }
      d2 = i + e2 - e1;  // this gives the position of ...
      if (d2 < 0 || d2 >= l2) { // the corresponding digit in n2
        d2 = 0;
      } else {
        d2 = workptr[n2 + d2] - '0';  // get the digit from n2
      }
      d2 += d1 + c;  // add with carry
      c = d2 / 10;
      d2 %= 10;
      workptr[n3 + i] = d2 + '0';  // store the answer
    }
    if (c) {  // carry to the left
      n3--;
      workptr[n3] = '0' + c;
      l3++;
      e3++;
    }
  } else {  // subtract the smaller from the larger. The sign of n1-n2 is m1
    if (e1 == e2) {  // compare to see which is the largest
      for (i = 0; i < l1 && i < l2; i++) {
        if (workptr[n1 + i] < workptr[n2 + i]) {  // swap numbers
          n3 = n2; n2 = n1; n1 = n3;
          l3 = l2; l2 = l1; l1 = l3;
          m1 = m2;
          break;
        }
        if (workptr[n1 + i] > workptr[n2 + i]) {
          break;  // order OK
        }
      }
      if ((i == l1 && i == l2) || i >= precision) {
        stack("0", 1);
        return;  // numbers are equal; return zero
      }
      if (i == l1) {
        n3 = n2; n2 = n1; n1 = n3;
        l3 = l2; l2 = l1; l1 = l3;
        m1 = m2;
      }
    }
    // n1 is an initial segment of n2; swap since n1<n2 at this point, n1>n2.
    // now the subtraction goes exactly like the earlier addition.
    l3 = (l1 > l2 + e1 - e2) ? l1 : (l2 + e1 - e2);
    if (l3 > precision + 2) {
      l3 = precision + 2;
    }
    mtest(workptr, worklen, eworkptr + precision, precision + 256);
    n3 = eworkptr;
    for (i = l3; i >= 0; i--) {
      if (i >= l1) {
        d1 = '0';
      } else {
        d1 = workptr[n1 + i];
      }
      d2 = i + e2 - e1;
      if (d2 < 0 || d2 >= l2) {
        d2 = '0';
      } else {
        d2 = workptr[n2 + d2];
      }
      d1 -= d2 + c;
      if (d1 < 0) {
        d1 += 10;
        c = 1;
      } else {
        c = 0;
      }
      if (i < precision) {
        workptr[n3 + i] = d1 + '0';
      }
    }
    if (l3 > precision) {
      l3 = precision;
    }
    while (l3 && workptr[n3] == '0') {
      l3--;
      n3++;
      e3--;
    }
  }
  stacknum(workptr + n3, l3, e3, m1);   // after the operation the result is stacked
}

/* OPsub, the binary - operator, is implemented by negating and adding. */
void binmin(char op) {
  unmin(op), binplus(op);
}

/* OPmul, the binary * operator */
void binmul(char p) {
  int n1, n2, m1, m2, e1, e2, z1, z2, l1, l2;
  int n3, l3;
  int i, j, k;
  int c, d, d1;

  // get each number and delete from the stack
  if ((n1 = num(&m1, &e1, &z1, &l1)) < 0) {
    die(Enum);
  }
  delete(&l3);
  if ((n2 = num(&m2, &e2, &z2, &l2)) < 0) {
    die(Enum);
  }
  delete(&l3);
  if (l1 > precision + 2) {
    l1 = precision + 2;
  }
  if (l2 > precision + 2) {
    l2 = precision + 2;
  }
  l3 = l1 + l2;
  if (z1 || z2) {
    stack("0", 1);
    return;  // zero times x is zero
  }
  if (l1 < l2) {
    i = l2, l2 = l1, l1 = i, i = n2, n2 = n1, n1 = i;   // make sure n2 is the shorter
  }
  mtest(workptr, worklen, eworkptr + l3, l3 + 256);  // make room for the answer
  n3 = eworkptr;  // this is where it goes
  for (i = 0; i < l3; workptr[n3 + (i++)] = '0') {
    // initially it is zero
  }
  for (i = l2 - 1; i >= 0; i--) { // now a long multiplication
    c = 0;
    d1 = workptr[n2 + i] - '0';
    for (j = l1 - 1; j >= 0; j--) {
      k = i + j + 1;
      d = (workptr[n1 + j] - '0') * d1 + c + workptr[n3 + k] - '0';
      c = d / 10;
      d %= 10;
      workptr[n3 + k] = d + '0';
    }
    workptr[n3 + i] += c;
  }
  if (abs(e1 += e2 + 1) + 2 > maxexp) {
    die(Eoflow);  // calculate the exponent
  }
  for (; l3 > 0 && workptr[n3] == '0'; e1--, n3++, l3--) {
    // remove leading zeros
  }
  stacknum(workptr + n3, l3, e1, m1 ^ m2);  // stack the answer
}

/*
 * OPdiv,  the binary /  operator;
 * OPidiv, the binary %  operator, and OPmod, the binary // operator are all handled here
 */
void bindiv(char op) {
  int n1, n2, m1, m2, e1, e2, z1, z2, l1, l2;
  int n3, l3;
  int i, j;
  int c, d, mul;

  if ((n2 = num(&m2, &e2, &z2, &l2)) < 0) {
    die(Enum);
  }
  delete(&l3);
  if ((n1 = num(&m1, &e1, &z1, &l1)) < 0) {
    die(Enum);
  }
  delete(&l3);
  if (z2) {
    die(Edivide);  // anything divided by zero
  }
  if (z1) {
    stack("0", 1);
    return;  // zero divided by anything
  }
  if (l1 > precision + 2) {
    l1 = precision + 2;
  }
  if (l2 > precision + 2) {
    l2 = precision + 2;
  }
  l3 = precision + 2;  // the number of digits in the quotient
  if (op != OPdiv) {
    l3 = e1 - e2 + 1;  // for % and //, the number of digits in the integer quotient.
  }
  if (l3 <= 0) {  // the integer result is a fraction
    if (op == OPidiv) {
      stack("0", 1);  // integer quotient is zero
    } else {
      while (l1 > 0 && workptr[n1 + l1 - 1] == '0') {
        l1--;  // remove trailing zeros
      }
      stacknum(workptr + n1, l1, e1, m1);  // remainder result is n1
    }
    return;
  }
  if (l3 > precision + 2) {
    l3 = precision + 2;
  }
  if (op == OPmod && l3 > precision) {
    stack("0", 1);
    return;  // if l3 > precision, return remainder 0
  }
  // Now extend n1 to length l2+l3-1 with zeros
  mtest(workptr, worklen, eworkptr + l3 + l3 + l2, l3 + l3 + l2 + 256);
  if (l1 < l2 + l3) {
    n3 = eworkptr + l2 + l3 - l1;
  } else {
    n3 = eworkptr;
  }
  for (i = l1; i < l2 + l3 - 1; i++) {
    workptr[n1 + i] = '0';
  }
  for (i = 0; i < l3; i++) {  // loop for each digit of result
    workptr[n3 + i] = '0';  // start each result digit at zero
    for (;;) {
      c = 0;
      z1 = 1;
      if (i) {
        d = workptr[n1 + i - 1] * 10 + workptr[n1 + i] - '0' * 11;
      } else {
        d = workptr[n1 + i] - '0';
      }
      mul = d / (workptr[n2] - '0' + 1);  // the next digit can't be lower than this
      if (mul == 0) {
        mul = 1;  // continue until subtraction fails
      }
      for (j = l2 - 1; j >= -i; j--) {  // do a subtraction
        if (j >= 0) {
          d = workptr[n2 + j] - '0';
        } else {
          d = 0;
        }
        d = workptr[n1 + i + j] - d * mul - c - '0';
        if (d) {
          z1 = 0;
        }
        c = 0;
        while (d < 0) {
          d += 10, c++;
        }
        workptr[n1 + i + j] = d + '0';
      }
      if (z1) {
        workptr[n3 + i] += mul;
        break;  // exactly zero resulted
      }
      if (!c) {
        workptr[n3 + i] += mul;
        continue;  // a positive value resulted
      }
      c = 0;
      for (j = l2 - 1; j >= -i; j--) {  // add back a failed subtraction
        if (j >= 0) {
          d = workptr[n2 + j] - '0';
        } else {
          d = 0;
        }
        d += workptr[n1 + i + j] + c;
        if (d > '9') {
          d -= 10, c = 1;
        } else {
          c = 0;
        }
        workptr[n1 + i + j] = d;
      }
      break;  // this result digit is found
    }
    if (z1 && i >= l1 - l2) {  // exact division
      l3 = i + 1;
      if (op == OPmod) {
        stack("0", 1);
        return;  // zero remainder
      }
      break;  // the entire result is found
    }
  }  // end of division: the result can be stacked
  if (op == OPmod) {  // stack the remainder
    if (l1 < l2 + l3) {
      l1 = l2 + l3 - 1;
    }
    for (; l1 && workptr[n1] == '0'; l1--, e1--, n1++) {
      // no-op
    }
    for (; l1 > 0 && workptr[n1 + l1 - 1] == '0'; l1--) {
      // no-op
    }
    stacknum(workptr + n1, l1, e1, m1);
  } else {  // stack the quotient
    for (; l3 > 0 && workptr[n3] == '0'; e2++, n3++, l3--) {
      // no-op
    }
    while (l3 > 0 && workptr[n3 + l3 - 1] == '0') {
      l3--;
    }
    if (abs(e1 -= e2) > maxexp) {
      die(Eoflow);
    }
    stacknum(workptr + n3, l3, e1, m1 ^ m2);
  }
}

/* OPpower, the binary ** operator */
void binexp(char op) {
  int pow, n, m, e, z, l, pm = 0, c = four * 8 - 1;     // ahem! 8 bits per byte here
  char *ptr;

  pow = getint(1);  // the exponent must be an integer.
  if (pow < 0) {
    pow = -pow, pm = 1;  // find x**(abs(y)) first, then calculate x**y
  }
  if ((n = num(&m, &e, &z, &l)) < 0) {
    die(Enum);  /* A copy of the first operand */
  }
  if (pow == 0) {
    delete(&l);
    stack("1", 1);  // x ** 0 is 1
    return;
  }
  if (z) {
    return;  // 0 ** x is 0 - note the zero operand is still stacked
  }
  while (pow > 0) {  // get the MSB of the num into the MSB of the int
    pow <<= 1;
    c--;
  }
  precision += 2;  // temporarily increase precision for good result
  while ((c--) > 0) {  // for each bit of the exponent
    rxdup();
    binmul(op); // square the intermediate result
    if ((pow <<= 1) < 0) {  // if the next bit of the exponent is set,
      stacknum(workptr + n, l, e, m);
      binmul(op);  // multiply the number in
    }
  }
  if (pm) {  // the exponent was negative, so invert the number
    mtest(cstackptr, cstacklen, ecstackptr + 2 * four, 256);
    ptr = cstackptr + ecstackptr - four;
    l = align(*(int *) ptr);
    ptr -= l;  // ptr points to the stack entry containing the result
    l += four;  // l contains its whole length
    n = four + align(1);
    for (c = l - 1; c >= 0; c--) {
      ptr[c + n] = ptr[c];  // make two ints-worth of space
    }
    ptr[0] = '1';  // store the stack entry "1" in the space
    *(int *) (ptr + align(1)) = 1;
    ecstackptr += n;
    bindiv(2);  // now divide 1 by the result.
  }
  precision -= 2;  // restore the old precision
  n = num(&m, &e, &z, &l);  // prepare to reformat the number to the new precision ...
  delete(&c);   // by unstacking and restacking
  eworkptr = 0;
  while (l > 0 && workptr[n + l - 1] == '0') {
    l--;  // first remove trailing zeros
  }
  stacknum(workptr + n, l, e, m);
}

/* Duplicate the top stack entry */
void rxdup() {
  char *ptr = cstackptr + ecstackptr;
  int len = align(*((int *) ptr - 1)) + four;

  dtest(cstackptr, cstacklen, ecstackptr + len, len + 256, ptr);
  memcpy(ptr, ptr - len, len);  // simple, really...
  ecstackptr += len;
}

/* Implements all the comparison operators. */
void binrel(char op) {
  int len1, len2;
  int i;
  int ans = 0;
  unsigned char *ptr1, *ptr2;
  int n, m, m2, e1, e2, z, l;

  n = num(&m2, &e2, &z, &l);  // test to see whether the top value is a number
  ptr2 = (unsigned char *) delete(&len2);  // delete the top value
  n = (n < 0 || num(&m, &e1, &z, &l) < 0);  // test to see whether both are numbers
  if (op == OPeequ || op >= OPnneq) {  // the strict comparison operators
    ptr1 = (unsigned char *) delete(&len1);
    // now see which one is greater, before calculating the required result
    if (op > OPnneq || len1 == len2) {
      for (i = 0; i < len1 && i < len2 && ptr1[i] == ptr2[i]; i++) {
        // no-op
      }
      if (i == len1) {
        ans = -!(i == len2);  // string1 is a prefix of string2
      } else if (i == len2) {
        ans = 1;  // string2 is a prefix of string1
      } else {
        ans = ptr1[i] - ptr2[i];  // strings differ at this character
      }
    } else {
      ans = 1;  // for == and \== with lengths different, report not equal
    }
    switch (op) {
      case OPeequ:
        ans = !ans;
        break;
      case OPnneq:
        ans = (ans != 0);
        break;
      case OPlless:
        ans = (ans < 0);
        break;
      case OPggrtr:
        ans = (ans > 0);
        break;
      case OPlleq:
        ans = (ans <= 0);
        break;
      case OPggeq:
        ans = (ans >= 0);
    }
  } else {
    if (!n) {  // both numeric
      if (!n && (m ^ m2)) {  // different signs:
        z = 0;
        delete(&l);
      }  // sign of result = sign of num1
      else {
        ecstackptr += align(len2) + four;   // restack string2
        i = precision;
        precision = fuzz;  // add fuzz
        binmin(op);   // compare using binary -
        precision = i;
        n = num(&m, &i, &z, &l);  // get flags,
        delete(&l);  // then discard the result
      }
    } else {  // string comparison; first strip spaces
      ecstackptr += align(len2) + four; // restack string2
      strip();
      ptr2 = (unsigned char *) delete(&len2);  // get pointers and lengths of
      strip();   // the stripped strings and
      ptr1 = (unsigned char *) delete(&len1);  // delete them from the stack.
      for (i = 0; i < len1 && i < len2 && ptr1[i] == ptr2[i]; i++){
        // compare
      }
      if (i == len1) {
        while (i < len2 && ptr2[i] == ' ') {
          i++;  // act as if the shorter ...
        }
      }
      if (i == len2) {
        while (i < len1 && ptr1[i] == ' ') {
          i++;  // were padded wth spaces
        }
      }
      // now set flags as for numeric comp.
      if (i >= len1) {
        if (i >= len2) {
          z = 1;
          m = 0;
        } else {
          z = 0;
          m = (ptr2[i] > ' ');
        }
      } else {
        if (i >= len2) {
          z = 0;
          m = (ptr1[i] < ' ');
        } else {
          z = 0;
          m = (ptr1[i] < ptr2[i]);
        }
      }
    }
    // this gives the correct result for the required op
    if (z) {
      if (op == OPequ || op == OPleq || op == OPgeq) {
        ans = 1;
      }
    } else if ((op == OPgrtr && !m) || op == OPneq) {
      ans = 1;
    }
    if (m) {
      if (op == OPless || op == OPleq) {
        ans = 1;
      }
    } else if (op == OPgeq) {
      ans = 1;
    }
  }
  ptr1 = (unsigned char *) (cstackptr + ecstackptr);    // The result is stacked
  ecstackptr += align(1) + four;
  (*ptr1) = '0' + ans;
  (*(int *) (ptr1 + align(1))) = 1;
}

/* OPand, OPxor, OPor - binary &, |, && operators */
void binbool(char op) {
  int z1;
  int minus, exp, zero, len;

  if (num(&minus, &exp, &z1, &len) < 0) { // get the two numbers and ...
    die(Enum);
  }
  delete(&len); // delete the second
  if (num(&minus, &exp, &zero, &len) < 0) {
    die(Enum);
  }
  switch (op) {
    case OPand:
      if (z1) {
        delete(&len);
        stack("0", 1);
      }
      break;
    case OPxor:
      if (!z1) {
        unnot(op);
      }
      break;
    case OPor:
      if (!z1) {
        delete(&len);
        stack("1", 1);
      }
  }
}

/* OPcat, OPspc - concatenation without/with a space */
void bincat(char op) {
  int len1, len2, alen1, alen2, count;
  char *ptr = cstackptr + ecstackptr - four;
  char *aptr;
  char *ptr1, *ptr2;

  alen1 = align(len1 = *(int *) ptr);
  ptr1 = ptr - alen1;
  ptr2 = ptr1 - four;
  alen2 = align(len2 = *(int *) ptr2);
  ptr2 -= alen2;
  aptr = ptr2 + len2;
  if (op == OPspc) {
    *(aptr++) = ' ';
    len2++;
  }
  for (count = len1; count--; aptr++[0] = ptr1++[0]) {
    // no-op
  }
  ptr2 += align(len1 + len2);
  *(int *) ptr2 = len1 + len2;
  ecstackptr = ptr2 - cstackptr + four;
}

/* OPnot, unary \ operator */
void unnot(char op) {
  int minus, exp, zero, len;

  if (num(&minus, &exp, &zero, &len) < 0) {
    die(Enum);
  }
  delete(&len);
  if (zero) {
    stack("1", 1);
  } else {
    stack("0", 1);
  }
}

/* OPneg, unary - */
void unmin(char op) {
  int a, b;
  int minus, exp, zero, len;

  if ((a = num(&minus, &exp, &zero, &len)) < 0) {
    die(Enum);
  }
  delete(&b);
  if (zero) {
    stack("0", 1);
  } else {
    stacknum(workptr + a, len, exp, !minus);  // restack with opposite sign
  }
}

/* OPplus, unary +  -just unstack and restack, thus checking that it is numeric and reformatting it */
void unplus(char op) {
  int a, b;
  int minus, exp, zero, len;

  if ((a = num(&minus, &exp, &zero, &len)) < 0) {
    die(Enum);
  }
  delete(&b);
  if (zero) {
    stack("0", 1);
  } else {
    stacknum(workptr + a, len, exp, minus);
  }
}

/* Strip leading and trailing spaces from the top stack value */
void strip() {
  char *ptr = cstackptr + ecstackptr - four;
  char *ptr1;
  int len = *(int *) ptr;
  int i;

  ptr -= align(len);  // point to the value
  ptr1 = ptr;  // save the original pointer
  ecstackptr -= align(len);  // discount the original length
  for (; len > 0 && ptr[0] == ' '; len--, ptr++) {
    // skip leading spaces
  }
  for (; len > 0 && ptr[len - 1] == ' '; len--) {
    // remove trailing spaces
  }
  if (ptr == ptr1) {
    ptr1 += len;  // if no leading spaces leave the value in place
  } else {
    for (i = 0; i < len; i++, (*(ptr1++)) = (*(ptr++))) {
      // else move it back
    }
  }
  (*(int *) (ptr1 + toalign(len))) = len;
  ecstackptr += align(len);
}

/*
 * Examines the `last value' on the calculator stack to determine whether it is numeric (always leaves the number stacked).
 * If not, then the answer is -1, otherwise the answer is an offset within the workspace where a sequence of digits is to be found.
 * In this case, the sequence of digits, when a decimal point is added between the 1st and 2nd digits
 * and the result is multiplied by 10**exp, is equal to the magnitude of the number.
 * `minus' is 1 if the number is negative; `zero' is 1 if the number is zero, and `len' is the length of the sequence of digits.
 */
int num(int *minus, int *exp, int *zero, int *len) {
  register char *ptr1 = cstackptr + ecstackptr - four;
  register int ptr2 = eworkptr;
  int ans = ptr2;
  register int len1 = *(int *) ptr1;
  int dot = 0;
  int etmp, emin;
  register int myzero = 1;      // fast-access copy of "*zero"
  register int myexp = -1;      // and of *exp
  register int ch;

  if (len1 < 0) {
    die(Ecall);  // doing any numeric operation whatsoever on a 'null' is strictly forbidden!
  }
  ptr1 -= align(len1);  // ptr1 points to the value, len1 holds its length
  mtest(workptr, worklen, ptr2 + len1, len1 + 256);
  (*minus) = 0;  // assume positive; set negative if '-' found
  for (; ptr1[0] == ' ' && len1 > 0; len1--, ptr1++) {
    // skip leading spaces
  }
  for (; len1 > 0 && ptr1[len1 - 1] == ' '; len1--) {
    // and trailing spaces
  }
  if ((ch = ptr1[0]) == '-') { // deal with leading sign
    ptr1++;
    len1--;
    (*minus) = 1;
  } else if (ch == '+') {
    ptr1++;
    len1--;
  }
  for (; ptr1[0] == ' ' && len1 > 0; len1--, ptr1++){
    // and spaces after it
  }
  if (len1 <= 0 || (ch = ptr1[0]) > '9' || (ch == '.' && (len1 == 1 || ptr1[1] > '9'))) {
    return -1;  // initial check for validity
  }
  for (; len1 > 0; ptr1++, len1--) {  // now, examine each character of the number
    if ((ch = ptr1[0]) == '0' && myzero) {
      if (dot) {
        myexp--;  // each leading 0 after the dot divides by 10
      }
      continue;
    }
    if (ch >= '0' && ch <= '9') {
      myzero = 0;  // either zero was false already, or a non-zero digit was found.
      if (!dot) {
        myexp++;  // each figure before the dot multiplies by 10
      }
      workptr[ptr2++] = ch;
    } else if (ch == '.') {
      if (dot) {
        return -1;  // count the number of dots
      }
      dot = 1;
    } else if ((ch & 0xdf) == 'E') {  // now deal with an exponent
      ptr1++;
      len1--;
      emin = etmp = 0;
      if ((ch = ptr1[0]) == '-') {
        emin = 1;
        ptr1++;
        len1--;
      } else if (ch == '+') {
        ptr1++;
        len1--;
      }
      if (len1 <= 0) {
        return -1;
      }
      for (; len1 > 0; ptr1++, len1--) {
        if ((ch = ptr1[0] - '0') < 0 || ch > 9) {
          return -1;  // must contain digits
        }
        etmp = etmp * 10 + ch;
        if (etmp > maxexp) {
          die(Eoflow);
        }
      }
      *len = ptr2 - ans;
      eworkptr = ptr2;
      if ((*zero = myzero)) {
        *exp = *minus = 0; // no exponent if zero
        return ans;
      }
      if (emin) {
        etmp = -etmp;  // otherwise set the exponent and return.
      }
      (*exp) = myexp + etmp;
      return ans;
    } else {
      return -1;  // each character must be a digit or "E" or "."
    }
  }
  *len = ptr2 - ans;
  if ((*zero = myzero)) {
    *exp = *minus = 0;  // no exponent or sign if zero
  } else {
    *exp = myexp;
  }
  eworkptr = ptr2;  // protect this number from being overwritten,
  return ans;  // and return it.
}

/*
 * Get an integer from the calculator stack. Error if overflow.
 * if flg=1 and the number was non-integral an error is raised
 */
int getint(int flg) {
  int n, minus, exp, zero, len, d;
  unsigned ans = 0;

  if ((n = num(&minus, &exp, &zero, &len)) < 0) { // first get the number...
    die(Enum);
  }
  delete(&d);  // and delete it
  if (zero) {
    return 0;
  }
  for (; len > 0 && workptr[n + len - 1] == '0'; len--) {
    // remove trailing zeros
  }
  if (flg && len > exp + 1) {
    die(Enonint); // not an integer
  }
  for (; len > 0 && exp >= 0; exp--, len--, n++) {  // now collect all the digits
    ans = ans * 10 + workptr[n] - '0';
    if ((int) ans < 0) {
      die(Erange);
    }
  }
  for (; exp >= 0; exp--, ans *= 10) {  // take account of the exponent.
    if ((int) ans < 0) {
      die(Erange);
    }
  }
  if (minus) {
    ans = -ans;  // and finally, give the correct sign.
  }
  return ans;
}

/* Given components of a number obtained by num(), tell whether it represents an integer. */
int isint(int num, int len, int exp) {

  for (; len > 0 && workptr[num + len - 1] == '0'; len--) {
    // no-op
  }
  return len <= exp + 1 && exp < precision && (exp < 9 || (exp == 9 && workptr[num] < '2'));
}

/*
 * Delete an item from the calculator stack, returning its position and setting "len" to its length.
 * The item is not actually deleted from memory, so it can still be examined,
 * but it will probably soon be overwritten by new values on the stack.
 */
char *delete(int* len) {
  char *ptr = cstackptr + ecstackptr - four;

  (*len) = *(int *) ptr;
  if (*len >= 0) {
    ptr -= align(*len);
    ecstackptr -= align(*len);
  } else {
    ptr = (char *) -1;  // I don't think this value is ever used
  }
  ecstackptr -= four;
  return ptr;
}

/* Tell whether the top value on the stack is null, i.e. has length = -1 */
int isnull() {
  char *ptr = cstackptr + ecstackptr - four;

  return (*(int *) ptr) < 0;
}

/*
 * Stack the number given by sequence of digits `num' of length `len' and exponent `exp'.
 * minus=0 if the number is positive, 1 if negative.
 */
void stacknum(char *num, int len, int exp, int minus) {
  char *ptr1;
  int len1 = 0;
  int i;

  mtest(cstackptr, cstacklen, ecstackptr + len + 30, len + 256);
  ptr1 = cstackptr + ecstackptr;
  if (len <= 0) {
    num = "0";
    len = 1;
    exp = 0;
    minus = 0;
  }
  if (len > precision) {  // round to the correct number of digits
    if (num[len = precision] >= '5') {
      for (i = len - 1; i >= 0; i--) {
        if (++num[i] <= '9') {
          break;
        }
        num[i] = '0';
      }
      if (i < 0) {  // change 999995 to 10000E+1 (or whatever)
        for (i = len - 2; i >= 0; i--) {
          num[i + 1] = num[i];
        }
        num[0] = '1', exp++;
      }
    }  // now stack the digits, starting with a sign if negative
  }
  if (minus) {
    ptr1[len1++] = '-';
  }
  if (len - exp - 1 <= 2 * precision && exp < precision) {  // stack with no exponent
    if (exp < 0) {  // begin with 0.00...0
      ptr1[len1++] = '0';
      ptr1[len1++] = '.';
      for (i = -1; i > exp; i--) {
        ptr1[len1++] = '0';
      }
    }
    while (len > 0) {  // stack the digits
      ptr1[len1++] = num[0], num++, len--, exp--;
      if (len && exp == -1) {
        ptr1[len1++] = '.';  // remembering the decimal point
      }
    }
    while (exp > -1) {
      ptr1[len1++] = '0';   // add zeros up to the decimal pt
      exp--;
    }
  } else {  // stack floating point in appropriate form with exponent
    ptr1[len1++] = num++[0];  // the (first) digit before the "."
    len--;
    if (numform) { // for engineering, up to two more digits are required before "."
      while (exp % 3) {
        exp--;
        ptr1[len1++] = (len-- > 0 ? (num++)[0] : '0');
      }
    }
    if (len > 0) {  // now the "." and the rest of the digits.
      ptr1[len1++] = '.';
      while (len--) {
        ptr1[len1++] = (num++)[0];
      }
    }
    if (exp) {  // add the exponent
      ptr1[len1++] = 'E';
      ptr1[len1++] = exp < 0 ? '-' : '+';
      exp = abs(exp);
      if (exp > maxexp) {
        die(Eoflow);
      }
      for (i = 1; i <= exp; i *= 10);
      i /= 10;
      for (; i >= 1; i /= 10) {
        ptr1[len1++] = exp / i + '0';
        exp %= i;
      }
    }
  }
  *(int *) (ptr1 + align(len1)) = len1;  // finish off the stack entry.
  ecstackptr += align(len1) + four;
}

/*
 * Go along a program line, accumulating characters to form a variable name.
 * If it is a compound symbol, the substitution in the tail is performed here also.
 * If the character pointer does not point to a valid symbol, then on exit varname[0]=namelen=0.
 */
void getvarname(char *line, int *ptr, char *varname, int *namelen, int maxlen) {
  char *exp, *exp1, *vg;
  char c;
  int expn;
  int explen;
  int disp = trcflag & Tintermed;  // whether to trace compound symbols
  char quote;

  maxlen -= 2;  // a safety margin :-)
  if (rexxsymbol(line[*ptr]) < 1) {  // test the starting character
    varname[0] = 0;
    (*namelen) = 0;
    return;
  }
  for (exp = varname; rexxsymbol(line[*ptr]); exp++, (*ptr)++) {
    if (exp - varname > maxlen) {
      die(Elong);  // copy the stem or simple symbol
    }
    (*exp) = line[*ptr];
  }
  if (line[*ptr] == '.') {
    varname[0] |= 128;  // flag: not a simple symbol
    if ((c = line[*ptr + 1]) <= ' ' || !(rexxsymboldot(c) || c == '(' || c == '\'' || c == '\"')) {
      (*ptr)++;  // stem - step past the final dot
    } else {
      disp |= 4;  // compound symbol (so display it)
    }
  }
  while (line[*ptr] == '.') {  // loop to interpret the qualifiers of a comp.symb.
    (*ptr)++, (*(exp++)) = '.';  // step past and copy the dot
    if (line[*ptr] <= ' ') {
      break;  // stop if at a space, terminator, or token
    }
    switch (line[*ptr]) {  // what kind of qualifier is it?
      case '.':  // null qualifier
        break;
      case '(':  // parenthesised expression e.g. stem.(a+b)
        (*ptr)++,
        exp1 = scanning(line, ptr, &explen);  // get the expression
        ecstackptr = exp1 - cstackptr;  // delete the expression
        if (exp + explen - varname > maxlen) {
          die(Elong);
        }
        memcpy(exp, exp1, explen);  // copy to the varname
        exp += explen;
        if (line[(*ptr)++] != ')') {
          die(Elpar);  // expect ')'
        }
        break;
      case '\'':  // quoted expression e.g. stem.'tail'
      case '\"':
        quote = line[(*ptr)++];
        for (expn = *ptr; (line[expn++]) != quote;) {
          // find the matching quote
        }
        if (exp + expn - *ptr - varname > maxlen) {
          die(Elong);
        }
        memcpy(exp, line + *ptr, expn - *ptr - 1);  // copy the string
        exp += expn - *ptr - 1;
        (*ptr) = expn;
        break;
      default:
        exp1 = exp;  // the usual qualifier - a symbol
        while (rexxsymbol(line[*ptr])) {  // append the symbol to the name
          if (exp - varname > maxlen) {
            die(Elong);
          }
          (*(exp++)) = line[(*ptr)++];
        }
        if (exp1 != exp && rexxsymbol(exp1[0]) == 1) {  // non-null non-constant
          (*exp) = 0, vg = varget(exp1, exp - exp1, &explen);   // see if the sym has a val
          if (vg != cnull) {  // if so, substitute
            exp = exp1;
            if (exp + explen - varname > maxlen) {
              die(Elong);
            }
            memcpy(exp, vg, explen), exp += explen;
          }
        }
    }
  }
  if (disp == 12) {  // tracing compound symbols
    c = varname[0];
    varname[0] &= 0x7f;   // don't print the "stem" flag
    traceline(">C>", varname, exp - varname);
    varname[0] = c;
  }
  (*exp) = 0, (*namelen) = exp - varname;
}

/*
 * Skip a variable name in a program line
 * It is guaranteed to start with a valid char
 */
void skipvarname(char *line, int *ptr) {
  char quote;
  int paren;

  while (rexxsymbol(line[*ptr])) {
    (*ptr)++;
  }
  while (line[*ptr] == '.') {  // loop to skip the qualifiers of a compound symbol
    (*ptr)++;  // step past the dot
    if (line[*ptr] <= ' ') {
      break;  // stop if at a space, terminator, or token
    }
    switch (line[*ptr]) {  // what kind of qualifier is it?
      case '.':  // null qualifier
        break;
      case '(':  // parenthesised expression e.g. stem.(a+b)
        (*ptr)++;
        for (paren = 1; paren && line[*ptr] && line[*ptr] != -1; (*ptr)++) {
          if (line[*ptr] == '(') {  // find matching ')'
            paren++;
          } else if (line[*ptr] == ')') {
            paren--;
          }
        }
        if (paren) {
          die(Elpar);
        }
        break;
      case '\'':  // quoted expression e.g. stem.'tail'
      case '\"':
        quote = line[(*ptr)++];
        while ((line[(*ptr)++]) != quote) {
          // Find the matching quote
        }
        break;
      default:   // the usual qualifier - a symbol
        while (rexxsymbol(line[*ptr])) {
          (*ptr)++;
        }
    }
  }
}

/*
 * Go along a program line and form a token,
 * i.e. a sequence of characters which are valid in symbols
 * - such as the word coming after SIGNAL.
 *
 * The token may be quoted, in which case the return value is 2,
 * or a symbol, in which case 1 is returned.
 *
 * If ex is non-zero and the token is obviously not a symbol or a quoted string,
 * then it may be an entire expression, perhaps preceded by VALUE
 * - as in SIGNAL VALUE x.  In that case 0 is returned.
 *
 * The token is terminated with a null after it has been collected.
 */
int gettoken(char *line, int *ptr, char *varname, int maxlen, int ex) {
  char *exp;
  int i;
  int explen;
  char quote;

  if (rexxsymboldot(line[*ptr])) {  // it's just a simple symbol
    for (i = 0; rexxsymboldot(line[*ptr]); varname[i++] = line[(*ptr)++])
      if (i >= maxlen - 2) {
        die(Elong);
      }
    varname[i] = 0;
    return 1;
  }
  if ((quote = line[(*ptr)++]) == '\'' || quote == '\"') {  // it's a string constant
    for (i = 0; (varname[i++] = line[(*ptr)++]) != quote;)
      if (i >= maxlen - 1) {
        die(Elong);
      }
    varname[--i] = 0;
    // if (!i) die(Enostring);  -- We allow null strings except where checked individually
    return 2;
  }
  if (!ex) {
    die(Enostring);  // it must be an expression; is that allowed?
  }
  if (line[--*ptr] == VALUE) {
    (*ptr)++;  // VALUE is optional here
  }
  if (!line[*ptr]) {
    die(Enostring);
  }
  scanning(line, ptr, &explen);  // get the expression
  exp = delete(&explen);
  if (explen > maxlen - 1) {
    die(Elong);
  }
  memcpy(varname, exp, explen);  // and copy it
  varname[explen] = 0;
  if (line[*ptr] == ')') {
    die(Erpar);  // save some work by flagging extra ')'s now.  This must be the end of a clause.
  }
  return 0;
}
