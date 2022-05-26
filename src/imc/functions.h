/* The functions of REXX/imc            This is a 132 column file            (C) Ian Collier 1992 */

/* Usually the mem test routines use inline code for greater efficiency.
   mtest: reallocate an area only if it is too small
   dtest: the same, but also return 1 if the area moved and set mtest_diff to
          the difference
   Recently fixed to save the old value when realloc fails
*/
#define mtest(memptr,alloc,length,extend) {char *old;         \
        (alloc<(length))&&(                                   \
        old=memptr,                                           \
        (memptr=realloc(memptr,(unsigned)(alloc+=(extend))))||(memptr=old,alloc-=(extend),die(Emem),1)  \
        );}
#define dtest(memptr,alloc,length,extend) (                   \
        (alloc<(length))&&(                                   \
        mtest_old=memptr,                                     \
        (memptr=realloc(memptr,(unsigned)(alloc+=(extend))))||(memptr=mtest_old,alloc-=(extend),die(Emem),1), \
        mtest_diff=memptr-mtest_old ))

#define whattype(c)      (types[(unsigned char)(c)])
#define alphanum(c)      (alphs[(unsigned char)(c)])
#define rexxsymboldot(c) (whattype(c)>0)
#define rexxsymbol(c)    (symbs[(unsigned char)(c)])

/* In ANSI C, you can take the address of any object and the result is of type pointer to that object.  In K&R C, this
   fails when the object is an array.  The following kludge takes the address of a jmp_buf, which is likely to be an
   array.  On systems where it isn't, this macro has to be changed... */

#define addressof(x) (&(x))

/* in rexx.c */
int main(int argc, char *argv[]);
char *allocm(unsigned size);
void die(int rc);
char *interpreter(int *anslen, int start,       /* Interpret program lines */
                  char *callname, long calltype, char *args[], int arglen[], int inherit, int delay);
static void doaddress(char **line, int env);    /* Address a command to an environment */
static void parse(char *list[], int len[], int up, char *line, int *ptr);       /* PARSE strings with a template */
static char uc1(int c, int up); /* Uppercase c, if up */
static void pset1(char *list, int listlen, char *val, int len, int up); /* Tokenise a string into variables */
static void pset(char *varname, int namelen, char *val, int len, int up);       /* Assign a PARSE result to a variable */
static int findsigl(int *level);        /* Find most recent line number */
static void getcallargs(char *args[], int arglen[], int argc);  /* Unstack parameters in a CALL instruc */
int rxcall(int stmt, char *name, int argc, int lit, long calltype);     /* Call a procedure */
char *rxinterp(char *exp, int len, int *rlen, char *name, long calltype,        /* INTERPRET a string */
               char *args[], int arglen[]);
static void doconds(void);      /* Check for delayed conditions and trap them */
void settrace(char *);          /* Set trace according to the given option */
int setoption(char *option, int len);   /* Set an option from the OPTIONS instruction */
static int gettrap(char **lineptr, int on, int *stmt);  /* Get a trap name after "call/signal on" */
static void testvarname(char **line, char *var, int len);       /* Test the symbol against a stored name */
static void skipstmt(void);     /* Skip the current instruction */
static void stepdo(void);       /* Step past the current DO */
static void stepselect(void);   /* Step past the current SELECT */
static void stepif(void);       /* Step past the current IF */
static void stepwhen(void);     /* Step past the current WHEN */
static void findend(void);      /* Find the next END */
void on_halt(void);             /* Find the line number at which halt occurred */

/* in calc.c */

char *scanning(char *line, int *ptr, int *len); /* Evaluate an expression */
void tracelast(char *type);     /* Trace the last value on the stack */
void traceline(char *type, char *exp, int len); /* Trace any string */
void stack(char *exp, int len); /* Stack a string literally */
void stackq(char *exp, int len, int quote);     /* Stack a string, collapsing quotes */
void stackx(char *exp, int len);        /* Stack a hex constant */
void stackb(char *exp, int len);        /* Stack a binary constant */
void stackint(int i);           /* Stack an integer */
void stacknull(void);           /* Stack a null parameter */
void binplus(int op);           /* Add the last two values */
void binmin(int op);            /* Subtract the last value from the previous */
void binmul(int op);            /* Multiply the last two values */
void bindiv(int op);            /* Divide the previous value by the last */
void binexp(int op);            /* Raise the previous value to the last-th power */
void rxdup(void);               /* Duplicate the last value */
void binrel(int op);            /* Compare the last two values */
void binbool(int op);           /* Combine the last two logical values */
void bincat(int op);            /* Concatenate the last two values */
void unnot(int op);             /* Negate logically the last value */
void unmin(int op);             /* Negate arithmetically the last value */
void unplus(int op);            /* Reformat the last numeric value */
void strip(void);               /* Strip surrounding space from the last value */
int num(int *minus, int *exp, int *zero, int *len);     /* Get the last value as a number */
int getint(int flg);            /* Get the last value as an integer */
int isint(int num, int len, int exp);   /* Is the last value an integer? */
char *delete(int *len);         /* Delete and return the last value */
int isnull(void);               /* Is the last value null? */
void stacknum(char *num, int len, int exp, int minus);  /* Stack a number */
void getvarname(char *line, int *ptr, char *varname, int *namelen, int maxlen); /* Get a symbol from a program line */
void skipvarname(char *line, int *ptr); /* Skip a symbol in a program line */
int gettoken(char *line, int *ptr, char *varname, int maxlen, int ex);  /* Get a token from a program line */

/* in util.c */
char *message(int rc);          /* Return an error message string */
void rcset(int i, int type, char *desc);        /* Set the variable rc to the given value */
void rcstringset(int rc, char *rcval, int rclen, int type, char *desc); /* Set the variable rc to the given string */
void printrc(int i);            /* Print a return code */
int exitcall( /*(long main, long sub, PEXIT parm) */ ); /* call an exit */
char *varsearch(char *name, int len, int *level, int *exist);   /* Search for a simple symbol or stem */
char *tailsearch(char *stem, char *name, int len, int *exist);  /* Search for a tail within a stem */
char *valuesearch(char *name, int namelen, int *level, int *exist, char **stem);        /* Search for any variable */
void printtree(int lev);        /* Print the entire variable table */
void update(int value, int amount, int level);  /* Update the pointers for a variable level */
void varcreate(char *varptr, char *name, char *value, int namelen, int len, int lev);   /* Create a simple symbol */
void stemcreate(char *varptr, char *name, char *value, int namelen, int len, int lev);  /* Create a stem */
void tailcreate(char *stem, char *tailptr, char *name, char *value, int namelen, int len, int level);   /* Create a tail in a stem */
void varset(char *name, int varlen, char *value, int len);      /* Assign any value to any variable */
char *varget(char *name, int varlen, int *len); /* Get any variable's value */
void newlevel(void);            /* Make a new level in the variable table */
void varcopy(char *name, int varlen);   /* Expose a variable from the previous level */
void vardup(void);              /* Duplicate the entire variables level */
void vardel(char *name, int len);       /* Remove one of the duplicate variables */
char uc(int c);                 /* Uppercase a character */
void *pstack(int type, int len);        /* Make a new program stack entry */
int unpstack(void);             /* Examine the top program stack entry */
void *delpstack(void);          /* Delete and return the top p-stack entry */
int strcmpi(char *s1, char *s2);        /* Is s1 an initial substring of s2? */
void printstmt(int stmt, int after, int error); /* Print a source statement */
void freestack(void *ptr, int i);       /* Clean up after deleting a p-stack entry */
void traceput(char *str, int len);      /* Print a counted string of trace output */
void tracestr(char *str);       /* Print a string of trace output */
void tracechar(int ch);         /* Print a character of trace output */
void tracenum(int num, int len);        /* Print a number to trace output */
void traceprefix(int num, char *prefix);        /* Print a number and 3-character prefix */
void interactive(void);         /* Pause execution, if in interactive trace */
void tokenise(char *input, int ilen, int dolabels, int line1);  /* Preprocess source code */
char *load(char *name, int *sourcelen); /* Load a program from disk */
void process(int c);            /* Tokenise characters */
void expand(int c);             /* Expand a token */
void display(int line, int ptr);        /* Display a program line */
char *rexxext(void);            /* Return the default filetype */
int which(char *gn, int opt, char *fn); /* Search for a program or subroutine */
char *hashsearch(int hash, char *name, int *exist);     /* Search for a hash table entry */
void *hashget(int hash, char *name, int *exist);        /* Return the value associated with a key */
void **hashfind(int hash, char *name, int *exist);      /* Return a pointer to a key's value element */
struct fileinfo *fileinit(char *name, char *filename, FILE * fp);       /* Store an open file's details */
void funcinit(char *name, void *handle, int (*address)(), int saa);     /* Store an external function's details */
void libsearch(void);           /* Ssearch for *.rxlib files */
int fileclose(char *name);      /* Free a file's hash table entry */
int on_interrupt(int sig, int flag); /* siginterrupt() */

/* in rxfn.c */
int rxfn(char *name, int argc); /* Try to call a builtin function */
int rxseterr(struct fileinfo *f);       /* Set rc to indicate a file's status */
char *nodelete(int *l);         /* Get last value without deleting it */

/* in shell.c */
int shell(char *command);       /* Interpret a shell command */
void hashclear(void);           /* Clear the shell's hashed path table */

/* in interface.c */
void envinit(void);             /* Initialise the environment table */
int envsearch(char *name);      /* Search for an environment name */
int envcall(int num, char *cmd, int len, char **ans, int *anslen);      /* call an environment */
int funccall(unsigned long (*func)(), char *name, int argc);    /* call a function by SAA calling sequence */
int unixcall(char *name, char *callname, int argc);     /* call a function by Unix calling sequence */
void hashfree(void);            /* Free all memory used by hash tables */
static void halt_handler();     /* handle halt signals */
static void pipe_handler();     /* handle broken pipe signals */
static void error_handler();    /* handle error signals */
static void sigtrace();         /* Go into trace mode, or exit */
