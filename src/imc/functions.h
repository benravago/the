/* The functions of REXX/imc            This is a 132 column file            (C) Ian Collier 1992 */


/*
 * The mem test routines use inline code for greater efficiency.
 *   mtest: reallocate an area only if it is too small
 *   dtest: the same, but also the add memory delta to memdiff
 */

#define mtest(memptr,alloc,length,extend) { \
  if (alloc < (length)) { \
    size_t N = alloc + (extend); \
    void *P = realloc(memptr, N); \
    if (!P) die(Emem); \
    memptr = P; \
    alloc = N; \
  } \
}

#define dtest(memptr,alloc,length,extend,memdiff) { \
  if (alloc < (length)) { \
    size_t N = alloc + (extend); \
    void *P = realloc(memptr, N); \
    if (!P) die(Emem); \
    memdiff += ((char*)P) - memptr; \
    memptr = P; \
    alloc = N; \
  } \
}

#define whattype(c)      (types[(unsigned char)(c)])
#define alphanum(c)      (alphs[(unsigned char)(c)])
#define rexxsymboldot(c) (whattype(c)>0)
#define rexxsymbol(c)    (symbs[(unsigned char)(c)])

/* in rexx.c */

char *allocm(unsigned);
void die(int);

char *interpreter(int*, int, char*, long, char**, int*, int, int);   // Interpret program lines
int rxcall(int, char*, int, int, long);                              // Call a procedure
char *rxinterp(char*, int, int*, char*, long, char**, int*);         // INTERPRET a string

void settrace(char*);        // Set trace according to the given option
int setoption(char*, int);   // Set an option from the OPTIONS instruction

void on_halt(void);          // Find the line number at which halt occurred

/* in calc.c */

char *scanning(char*, int*, int*);               // Evaluate an expression
void tracelast(char*);                           // Trace the last value on the stack
void traceline(char*, char*, int);               // Trace any string
void stack(char*, int);                          // Stack a string literally
void stackq(char*, int, char);                   // Stack a string, collapsing quotes
void stackx(char*, int);                         // Stack a hex constant
void stackb(char*, int);                         // Stack a binary constant
void stackint(int);                              // Stack an integer
void stacknull(void);                            // Stack a null parameter
void binplus(char);                              // Add the last two values
void binmin(char);                               // Subtract the last value from the previous
void binmul(char);                               // Multiply the last two values
void bindiv(char);                               // Divide the previous value by the last
void binexp(char);                               // Raise the previous value to the last-th power
void rxdup(void);                                // Duplicate the last value
void binrel(char);                               // Compare the last two values
void binbool(char);                              // Combine the last two logical values
void bincat(char);                               // Concatenate the last two values
void unnot(char);                                // Negate logically the last value
void unmin(char);                                // Negate arithmetically the last value
void unplus(char);                               // Reformat the last numeric value
void strip(void);                                // Strip surrounding space from the last value
int num(int*, int*, int*, int*);                 // Get the last value as a number
int getint(int);                                 // Get the last value as an integer
int isint(int, int, int);                        // Is the last value an integer?
char *delete(int*);                              // Delete and return the last value
int isnull(void);                                // Is the last value null?
void stacknum(char*, int, int, int);             // Stack a number
void getvarname(char*, int*, char*, int*, int);  // Get a symbol from a program line
void skipvarname(char*, int*);                   // Skip a symbol in a program line
int gettoken(char*, int*, char*, int, int);      // Get a token from a program line

/* in util.c */

char *message(int);                                          // Return an error message string
void rcset(int i, int, char*);                               // Set the variable rc to the given value
void rcstringset(int, char*, int, int, char*);               // Set the variable rc to the given string
void printrc(int);                                           // Print a return code
int exitcall(long, long, PEXIT);                             // Call an exit
char *varsearch(char*, int, int*, int*);                     // Search for a simple symbol or stem
char *tailsearch(char*, char*, int, int*);                   // Search for a tail within a stem
char *valuesearch(char*, int, int*, int*, char**);           // Search for any variable
void printtree(int);                                         // Print the entire variable table
void update(int value, int, int);                            // Update the pointers for a variable level
void varcreate(char*, char*, char*, int, int, int);          // Create a simple symbol
void stemcreate(char*, char*, char*, int, int, int);         // Create a stem
void tailcreate(char*, char*, char*, char*, int, int, int);  // Create a tail in a stem
void varset(char*, int, char*, int);                         // Assign any value to any variable
char *varget(char*, int, int *len);                          // Get any variable's value
void newlevel(void);                                         // Make a new level in the variable table
void varcopy(char*, int);                                    // Expose a variable from the previous level
void vardup(void);                                           // Duplicate the entire variables level
void vardel(char*, int);                                     // Remove one of the duplicate variables
char uc(char);                                               // Uppercase a character
void *pstack(int, int);                                      // Make a new program stack entry
int unpstack(void);                                          // Examine the top program stack entry
void *delpstack(void);                                       // Delete and return the top p-stack entry
int strcmpi(char*, char*);                                   // Is s1 an initial substring of s2?
void printstmt(int, int, int);                               // Print a source statement
void freestack(void*, int);                                  // Clean up after deleting a p-stack entry
void traceput(char*, int);                                   // Print a counted string of trace output
void tracestr(char*);                                        // Print a string of trace output
void tracechar(char);                                        // Print a character of trace output
void tracenum(int, int);                                     // Print a number to trace output
void traceprefix(int, char*);                                // Print a number and 3-character prefix
void interactive(void);                                      // Pause execution, if in interactive trace
void tokenise(char*, int, int, int);                         // Preprocess source code
char *load(char*, int*);                                     // Load a program from disk
void process(int);                                           // Tokenise characters
void expand(int);                                            // Expand a token
void display(int, int);                                      // Display a program line
char *rexxext(void);                                         // Return the default filetype
int which(char*, int, char*);                                // Search for a program or subroutine
char *hashsearch(int, char*, int*);                          // Search for a hash table entry
void *hashget(int, char*, int*);                             // Return the value associated with a key
void **hashfind(int, char*, int*);                           // Return a pointer to a key's value element
struct fileinfo *fileinit(char*, char*, FILE*);              // Store an open file's details
void funcinit(char*, void*, int(*)(), int);                  // Store an external function's details
void libsearch(void);                                        // Ssearch for *.rxlib files
int fileclose(char*);                                        // Free a file's hash table entry

int on_interrupt(int, int);  // siginterrupt()

/* in rxfn.c */

int rxfn(char*, int);            // try to call a builtin function
int rxseterr(struct fileinfo*);  // set rc to indicate a file's status
char *nodelete(int*);            // get last value without deleting it

/* in shell.c */

int shell(char*);       // interpret a shell command
void hashclear(void);   // clear the shell's hashed path table

/* in interface.c */

void envinit(void);                             // initialise the environment table
int envsearch(char*);                           // search for an environment name
int envcall(int, char*, int, char**, int*);     // call an environment
int funccall(unsigned long (*)(), char*, int);  // call a function by SAA calling sequence
int unixcall(char*, char*, int);                // call a function by Unix calling sequence
void hashfree(void);                            // free all memory used by hash tables

