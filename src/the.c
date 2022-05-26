/* THE.C - The Hessling Editor                                         */
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2020 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling, mark@rexx.org  http://www.rexx.org/
 */

#define MAIN 1
#include <the.h>
#include <proto.h>
#include <time.h>

static void handle_signal(int);
static void display_info(char_t *);
static void init_signals(void);

/*--------------------------- global data -----------------------------*/
WINDOW *statarea = NULL, *error_window = NULL, *divider = NULL, *filetabs = NULL;
VIEW_DETAILS *vd_current = (VIEW_DETAILS *) NULL;
VIEW_DETAILS *vd_first = (VIEW_DETAILS *) NULL;
VIEW_DETAILS *vd_last = (VIEW_DETAILS *) NULL;
VIEW_DETAILS *vd_mark = (VIEW_DETAILS *) NULL;
VIEW_DETAILS *filetabs_start_view = (VIEW_DETAILS *) NULL;
char_t number_of_views = 0;   /* number of views */
line_t number_of_files = 0;   /* number of files */
char_t display_screens = 1;   /* number of screens */
char_t current_screen = 0;
SCREEN_DETAILS screen[MAX_SCREENS];     /* 2 screen structures */
short screen_rows[MAX_SCREENS];
short screen_cols[MAX_SCREENS];
bool rexx_support;              /* initially Rexx support */
bool batch_only = FALSE;        /* are we running in batch ? */
bool horizontal = TRUE;
bool first_screen_display = FALSE;
bool single_instance_server = FALSE;
short save_coord_x[VIEW_WINDOWS];
short save_coord_y[VIEW_WINDOWS];

LINE *next_line = NULL, *curr_line = NULL;
LINE *first_file_name = NULL, *current_file_name = NULL;
LINE *editv = NULL;
LINE *first_option = NULL, *last_option = NULL;
bool error_on_screen = FALSE;
bool colour_support = TRUE;     /* indicates if colour is supported */
bool initial = TRUE;
bool been_interactive = FALSE;
char_t *rec = NULL;
length_t rec_len = 0;         /* length of rec */
char_t *trec = NULL;
length_t trec_len = 0;
length_t max_trec_len = 0;
char_t *brec = NULL;
length_t brec_len = 0;
char_t *cmd_rec = NULL;
length_t cmd_rec_len = 0;     /* length of cmd_rec */
length_t cmd_verify_col = 1;  /* display offset of cmd 1-based like verify_col */
char_t *pre_rec = NULL;
length_t pre_rec_len = 0;     /* length of pre_cmd_rec */
char_t *profile_command_line = NULL;
char_t *target_buffer = NULL; /* used in get_item_values() */
length_t target_buffer_len = 0;       /* length of target buffer */
bool focus_changed = FALSE;     /* indicates if focus line has changed */
bool current_changed = FALSE;   /* indicates if current line has changed */
bool in_profile = FALSE;        /* indicates if processing profile */
bool in_nomsg = FALSE;          /* indicates if running from NOMSG command */
bool in_reprofile = FALSE;      /* indicates if reprocessing profile */
int profile_file_executions = 0;        /* number of times profile file has executed */
bool execute_profile = TRUE;    /* indicates if we are to process a profile */
bool in_macro = FALSE;          /* indicates if processing Rexx macro */
bool interactive_in_macro = FALSE;      /* indicates if we should disply the screen if in a macro */
bool in_readv = FALSE;          /* indicates if processing READV CMDLINE */
bool file_read = FALSE;         /* indicates if we have read the file */
bool curses_started = FALSE;    /* indicates if curses has started */
bool the_readonly = FALSE;      /* indicates if running THE in readonly mode */
bool be_quiet = FALSE;          /* do not display error message header if TRUE */

char_t *the_version = (char_t *) THE_VERSION;
char_t *the_release = (char_t *) THE_VERSION_DATE;
char_t *the_copyright = (char_t *) "Copyright 1991-2020 Mark Hessling";
char_t term_name[20];         /* $TERM value */
char_t *tempfilename = (char_t *) NULL;
char_t *stdinprofile = (char_t *) NULL;
char_t user_home_dir[MAX_FILE_NAME + 1];

#define THE_PROFILE_FILE ".therc"

#define THE_FIFO_FILE ".thefifo"
#define THE_PID_FILE ".thepid"
char_t fifo_name[MAX_FILE_NAME + 1];
char_t pid_name[MAX_FILE_NAME + 1];

char_t *rexxoutname = (char_t *) "REXX.$$$";
char_t *keyfilename = (char_t *) "KEY$$$.$$$";
char_t rexx_pathname[MAX_FILE_NAME + 1];
char_t rexx_filename[10];
char_t *dirfilename = (char_t *) "DIR.DIR";

char_t macro_suffix[12] = ".the";     /* default extension for macros */
char_t dir_pathname[MAX_FILE_NAME + 1];
char_t dir_filename[10];
char_t key_pathname[MAX_FILE_NAME + 1];
char_t key_filename[15];
char_t curr_path[MAX_FILE_NAME + 1];
char_t sp_path[MAX_FILE_NAME + 1];
char_t sp_fname[MAX_FILE_NAME + 1];
char_t dir_path[MAX_FILE_NAME + 1];  /* for dir and ls commands */
char_t dir_files[MAX_FILE_NAME + 1]; /* for dir and ls commands */
char_t rexx_macro_name[MAX_FILE_NAME + 1];   /* current rexx macro name */
char_t rexx_macro_parameters[MAX_FILE_NAME + 1];     /* current rexx macro parameters */

char_t the_home_dir[MAX_FILE_NAME + 1];
char_t the_help_file[MAX_FILE_NAME + 1];

char_t the_macro_path[MAX_FILE_NAME + 1];
char_t the_macro_path_buf[MAX_FILE_NAME + 1];
char_t **the_macro_dir;
int max_macro_dirs = 0;
int total_macro_dirs = 0;

char_t spooler_name[MAX_FILE_NAME + 1];

char_t *prf_arg = (char_t *) NULL;
char_t *local_prf = (char_t *) NULL;
char_t *specified_prf = (char_t *) NULL;

char_t tabkey_insert = 'C';
char_t tabkey_overwrite = 'T';

struct stat stat_buf;

length_t display_length = 0;

short lastrc = 0;
short compatible_look = COMPAT_THE;
short compatible_feel = COMPAT_THE;
short compatible_keys = COMPAT_THE;
short prefix_width = DEFAULT_PREFIX_WIDTH;
short prefix_gap = DEFAULT_PREFIX_GAP;

chtype etmode_table[256];
bool etmode_flag[256];

bool ncurses_screen_resized = FALSE;

int lastkeys[8];
int lastkeys_is_mouse[8];
int current_key = -1;

#define DEFAULT_LINES 24
#define DEFAULT_COLS  80

short terminal_lines = DEFAULT_LINES;
short terminal_cols = DEFAULT_COLS;

   /*
    * Following are for my_getopt function(s).
    */
extern char *optarg;
extern int optind;

   /*
    * Following are for original cursor position for EXTRACT /CURSOR/
    */
line_t original_screen_line = (-1L);
line_t original_screen_column = (-1L);
line_t original_file_line = (-1L);
line_t original_file_column = (-1L);

   /*
    * Following are for startup LINE and COLUMN
    */
line_t startup_line = 0L;
length_t startup_column = 0;

   /*
    * Following are XCurses data
    */
int gotOutput = 0;

char_t *linebuf;              /* Buffer for one terminal line, at least 81 elems */
chtype *linebufch;              /* Buffer for one terminal line in chtype-mode, >= 81 */
length_t linebuf_size = 0;
int max_slk_labels = 0;
int slk_format_switch = 0;

LASTOP lastop[LASTOP_MAX] = {
  { NULL, 0, (char_t *) "alter", 2 },
  { NULL, 0, (char_t *) "change", 1 },
  { NULL, 0, (char_t *) "clocate", 2 },
  { NULL, 0, (char_t *) "count", 3 },
  { NULL, 0, (char_t *) "find", 1 },
  { NULL, 0, (char_t *) "locate", 1 },
  { NULL, 0, (char_t *) "schange", 3 },
  { NULL, 0, (char_t *) "tfind", 2 },
  { NULL, 0, (char_t *) "search", 3 },
};

   /*
    * Globals to support RECORD command
    */
FILE *record_fp = NULL;
int record_key = 0;
char_t *record_status = NULL;

   /*
    * Globals to support 16 colours
    */
short colour_offset_bits = 3;

void atexit_handler(void) {
  if (number_of_views > 0) {
    Cancel((char_t *) "SAVE");
  }
}

int main(int argc, char *argv[]) {
  register short i = 0;
  short c = 0;
  int length;
  bool trap_signals = TRUE;
  bool run_single_instance = FALSE;
  short rc = RC_OK;
  char *envptr = NULL;
  int slk_format = 0;
  char mygetopt_opts[100];
  int my_argc;
  char **my_argv;
  char *the_arguments;

  /*
   * Set our locale
   */
  setlocale(LC_ALL, ".utf8");
  /*
   * Ensure that CURRENT_VIEW is NULL before starting. This is to ensure
   * that any errors generated before CURRENT_VIEW is assigned are
   * handled gracefully.
   */
  CURRENT_VIEW = (VIEW_DETAILS *) NULL;
  /*
   * Set up our memory management calls. This is where you can specify a
   * debugging memory manager.
   */
  if (getenv("NO_FLISTS")) {
    the_malloc = malloc;
    the_calloc = calloc;
    the_free = free;
    the_realloc = realloc;
  } else {
    the_malloc = get_a_block;
    the_calloc = NULL;
    the_free = give_a_block;
    the_realloc = resize_a_block;
    init_memory_table();
  }
  /*
   * Set up flag to indicate that we are not interactive...yet.
   */
  in_profile = TRUE;
  execute_profile = TRUE;
  in_macro = FALSE;
  /*
   * Initialise LASTKEY array...
   */
  for (i = 0; i < 8; i++) {
    lastkeys[i] = -1;
    lastkeys_is_mouse[i] = 0;
  }
  /*
   * Initialise the printer spooler.
   */
  strcpy((char *) spooler_name, (char *) "lpr");
  /*
   * Get all environment variables here. Some may be overridden by
   * command-line switches. (future possibility)
   */
  if ((envptr = getenv("HOME")) != NULL) {
    if (((envptr == NULL) ? 0 : strlen((char *) envptr)) > MAX_FILE_NAME) {
      cleanup();
      display_error(7, (char_t *) envptr, FALSE);
      return (7);
    }
    strcpy((char *) user_home_dir, envptr);
  } else
    strcpy((char *) user_home_dir, "./");
  if (*(user_home_dir + strlen((char *) user_home_dir) - 1) != ISLASH)
    strcat((char *) user_home_dir, (char *) ISTR_SLASH);
  if ((envptr = getenv("TERM")) != NULL)
    strcpy((char *) term_name, envptr);
  else
    strcpy((char *) term_name, "default");
  /*
   * Get THE_HOME_DIR first (as all other paths rely on this value)
   */
  if ((envptr = getenv("THE_HOME_DIR")) != NULL) {
    strcpy((char *) the_home_dir, envptr);
    strrmdup(strtrans(the_home_dir, OSLASH, ISLASH), ISLASH, TRUE);
    if ((the_home_dir[strlen((char *) the_home_dir) - 1]) != ISLASH) {
      strcat((char *) the_home_dir, (char *) ISTR_SLASH);
    }
  } else {
    strcpy((char *) the_home_dir, user_home_dir);
    strcat((char *) the_home_dir, ".the/"); // was THE_HOME_DIRECTORY
  }
  /*
   * Get THE_MACRO_PATH environment variable. If not set set up default
   * to be THE_HOME_DIR followed by the current directory.
   */
  if ((envptr = getenv("THE_MACRO_PATH")) != NULL)
    Macropath((char_t *) envptr);
  else {
    strcpy((char *) the_macro_path, (char *) the_home_dir);
    if (strlen((char *) the_macro_path) == 0)
      strcpy((char *) the_macro_path, ".");
    strcat((char *) the_macro_path, ":.");
    Macropath(the_macro_path);
  }
  /*
   * Get THE_WIDTH environment variable. If not set use the builtin default
   * or the value from -w command line switch.
   */
  if ((envptr = getenv("THE_WIDTH")) != NULL) {
    if ((rc = valid_positive_integer_against_maximum((char_t *) envptr, MAX_WIDTH_NUM)) == 0) {
      length_t tmplen = atol(envptr);

      if (tmplen >= 10L && tmplen <= MAX_WIDTH_NUM) {
        max_line_length = tmplen;
      }
    }
  }
  /*
   * Add a hack for MacOS X to allow arguments to be passed as environment variables
   * instead of on the command line. This is to allow the ncurses version of THE
   * to accept files dropped on it from Finder
   * We inject values into our own argv[] array from environment variables. Only do this
   * if we only have 1 argument: argv[0] and THE_ARGC is set
   */
  the_arguments = getenv("THE_ARGC");
  if (argc == 1 && the_arguments != NULL) {
    my_argc = atoi(the_arguments) + 1;
    my_argv = (char **) malloc(sizeof(char *) * my_argc);
    if (my_argv) {
      char buf[100];

      /* use existing argv[0] */
      my_argv[0] = argv[0];
      for (i = 1; i < my_argc; i++) {
        sprintf(buf, "THE_ARGV_%d", i);
        my_argv[i] = getenv(buf);
      }
    }
  } else {
    my_argc = argc;
    my_argv = argv;
  }
  /*
   * Process the command line arguments.
   */
  strcpy(mygetopt_opts, "Rqk::sSbmnrl:c:p:w:a:u:h");
  strcat(mygetopt_opts, "1::");
  while ((c = getopt(my_argc, my_argv, mygetopt_opts)) != EOF) {
    switch ((char) c) {
      case 'R':                /* 'run' option from 'the' */
        break;
      case 's':                /* don't trap signals */
        trap_signals = FALSE;
        break;
      case 'q':                /* quiet - don't display profile heading message */
        be_quiet = TRUE;
        break;
      case 'k':                /* allow Soft Label Keys */
        SLKx = TRUE;
        if (optarg == NULL) {
          slk_format = 3;
          slk_format_switch = 4;
          max_slk_labels = 12;
          break;
        }
        slk_format = atoi(optarg);
        if (slk_format == 0) {
          cleanup();
          STARTUPCONSOLE();
          display_error(4, (char_t *) optarg, FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        if (slk_format > MAX_SLK_FORMAT) {
          char buf[MAX_FILE_NAME + 1];

          cleanup();
          sprintf((char *) buf, "SLK format must be >= 1 and <= %d", MAX_SLK_FORMAT);
          STARTUPCONSOLE();
          display_error(6, (char_t *) buf, FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        slk_format_switch = slk_format;
        switch (slk_format) {
          case 1:
          case 2:
            max_slk_labels = 8;
            slk_format--;
            break;
          case 3:
          case 4:
            max_slk_labels = 12;
            slk_format--;
            break;
          case 5:
            slk_format = 55;
            max_slk_labels = 10;
            break;
        }
        break;
      case 'S':                /* allow scrollbar */
        SBx = TRUE;
        break;
      case 'l':                /* set current line on startup */
        startup_line = (line_t) atol(optarg);
        if (startup_line < 0L) {
          cleanup();
          STARTUPCONSOLE();
          display_error(5, (char_t *) "startup line MUST be > 0", FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        break;
      case 'c':                /* set current column on startup */
        startup_column = (length_t) atoi(optarg);
        if (startup_column == 0) {
          cleanup();
          STARTUPCONSOLE();
          display_error(5, (char_t *) "startup column MUST be > 0", FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        break;
      case 'b':                /* batch processing */
        batch_only = TRUE;
        break;
      case 'm':                /* force into MONO */
        colour_support = FALSE;
        break;
      case 'n':                /* do not execute any profile file */
        execute_profile = FALSE;
        break;
      case 'r':                /* run in readonly mode */
        the_readonly = TRUE;
        break;
      case 'p':                /* profile file name */
        if ((specified_prf = (char_t *) malloc((strlen(optarg) + 1) * sizeof(char_t))) == NULL) {
          cleanup();
          STARTUPCONSOLE();
          display_error(30, (char_t *) "", FALSE);
          CLOSEDOWNCONSOLE();
          return (2);
        }
        strcpy((char *) specified_prf, (char *) optarg);
        break;
      case 'a':                /* profile arguments */
        if ((prf_arg = (char_t *) malloc((strlen(optarg) + 1) * sizeof(char_t))) == NULL) {
          cleanup();
          STARTUPCONSOLE();
          display_error(30, (char_t *) "", FALSE);
          CLOSEDOWNCONSOLE();
          return (3);
        }
        strcpy((char *) prf_arg, (char *) optarg);
        break;
      case 'w':                /* width of line */
        if ((rc = valid_positive_integer_against_maximum((char_t *) optarg, MAX_WIDTH_NUM)) != 0) {
          cleanup();
          /* safe to use mygetopt_opts as we are bailing out */
          if (rc == 4)
            sprintf(mygetopt_opts, "%s", optarg);
          else
            sprintf(mygetopt_opts, "- width MUST be <= %ld", MAX_WIDTH_NUM);
          STARTUPCONSOLE();
          display_error(rc, (char_t *) mygetopt_opts, FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        max_line_length = atol(optarg);
        if (max_line_length < 10L) {
          cleanup();
          STARTUPCONSOLE();
          display_error(5, (char_t *) "- width MUST be >= 10", FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        if (max_line_length > MAX_WIDTH_NUM) {
          cleanup();
          /* safe to use mygetopt_opts as we are bailing out */
          sprintf(mygetopt_opts, "- width MUST be <= %ld", MAX_WIDTH_NUM);
          STARTUPCONSOLE();
          display_error(6, (char_t *) mygetopt_opts, FALSE);
          CLOSEDOWNCONSOLE();
          return (5);
        }
        break;
      case 'u':                /* display length */
        display_length = (unsigned short) atoi(optarg);
        if (display_length == 0) {
          cleanup();
          STARTUPCONSOLE();
          display_error(5, (char_t *) "- display length MUST be > 0", FALSE);
          CLOSEDOWNCONSOLE();
          return (4);
        }
        break;
      case 'h':
        cleanup();
        STARTUPCONSOLE();
        display_info((char_t *) my_argv[0]);
        CLOSEDOWNCONSOLE();
        return (0);
        break;
      case '1':                /* allow single instances */
        run_single_instance = TRUE;
        /*
         * If no optional parameter supplied, use the user's $HOME
         * for the path, otherwise use the supplied value
         */
        if (optarg == NULL) {
          strcpy((char *) fifo_name, (char *) user_home_dir);
          strcat((char *) fifo_name, (char *) THE_FIFO_FILE);
          strcpy((char *) pid_name, (char *) user_home_dir);
          strcat((char *) pid_name, (char *) THE_PID_FILE);
          break;
        }
        strcpy((char *) fifo_name, (char *) optarg);
        break;
      default:
        break;
    }
  }

  if (optind < my_argc) {
    while (optind < my_argc) {
      /* for each trailing arg; assumed to be filenames, add each to a list of filenames to be edited */
      if ((current_file_name = add_LINE(first_file_name, current_file_name, strrmdup(strtrans((char_t *) my_argv[optind], OSLASH, ISLASH), ISLASH, TRUE), strlen(my_argv[optind]), 0, TRUE)) == NULL) {
        cleanup();
        STARTUPCONSOLE();
        display_error(30, (char_t *) "", FALSE);
        CLOSEDOWNCONSOLE();
        return (6);
      }
      if (first_file_name == NULL)
        first_file_name = current_file_name;
      optind++;
    }
  } else {
    /* add the current dir to the list of files to be edited */
    if ((current_file_name = add_LINE(first_file_name, current_file_name, CURRENT_DIR, strlen((char *) CURRENT_DIR), 0, TRUE)) == NULL) {
      cleanup();
      STARTUPCONSOLE();
      display_error(30, (char_t *) "", FALSE);
      CLOSEDOWNCONSOLE();
      return (7);
    }
    if (first_file_name == NULL)
      first_file_name = current_file_name;
  }
  /*
   * Allocate some memory to working variables...
   * Done here before processing single_instance mode because
   * we use trec in initialise_fifo();
   */
  rc = allocate_working_memory();
  if (rc) {
    cleanup();
    STARTUPCONSOLE();
    display_error(30, (char_t *) "", FALSE);
    CLOSEDOWNCONSOLE();
    return (rc);
  }
  /*
   * Allocate linebuf here in case EXTRACT /IDLINE/ is called in profile.
   */
  if ((length = COLS) <= THE_MAX_SCREEN_WIDTH)
    length = THE_MAX_SCREEN_WIDTH + 1;

  linebuf_size = length;
  if ((linebuf = (char_t *) malloc(linebuf_size)) == NULL) {
    cleanup();
    STARTUPCONSOLE();
    display_error(30, (char_t *) "", FALSE);
    CLOSEDOWNCONSOLE();
    return (30);
  }
  /*
   * If running under XCurses or SDL Curses, and running in single window mode, then
   * check if the FIFO $HOME/.thefifo exists.
   * If it does, then we are the client, so send the running instance
   * an edit command for each file
   * If not, then create the FIFO and continue on...
   */
  if (run_single_instance) {
    if (initialise_fifo(first_file_name, startup_line, startup_column, the_readonly)) {
      cleanup();
      return (0);
    }
  }
  /*
   * Check any command line conflicts...
   */
  if (display_length > 0 && display_length > max_line_length) {
    cleanup();
    STARTUPCONSOLE();
    display_error(6, (char_t *) "- width MUST be >= display length", FALSE);
    CLOSEDOWNCONSOLE();
    return (8);
  }
  /*
   * Override any default paths,filenames etc if supplied on command line
   */
  if (specified_prf && *specified_prf == '-' && batch_only) {
    FILE *fp = NULL;
    int num = 0;

    if ((stdinprofile = (char_t *) strdup(tmpnam(NULL))) == NULL)     // thetmpnam ( "PRF" )
    {
      cleanup();
      return (31);
    }
    if ((fp = fopen((char *) stdinprofile, "w")) == NULL) {
      free(stdinprofile);
      free(specified_prf);
      cleanup();
      return (8);
    }
    for (;;) {
      num = fread(rexx_pathname, sizeof(char), MAX_FILE_NAME, stdin);
      fwrite(rexx_pathname, sizeof(char), num, fp);
      if (feof(stdin))
        break;
    }
    fclose(fp);
    if (setup_profile_files(stdinprofile) != RC_OK) {
      cleanup();
      return (8);
    }
  } else {
    if (execute_profile && setup_profile_files(specified_prf) != RC_OK) {
      cleanup();
      return (8);
    }
  }
  if (specified_prf != NULL)
    free(specified_prf);
  /*
   * Allocate memory to pre_rec and set it to blanks.
   */
  if ((pre_rec = (char_t *) malloc((MAX_PREFIX_WIDTH + 1) * sizeof(char_t))) == NULL) {
    cleanup();
    STARTUPCONSOLE();
    display_error(30, (char_t *) "", FALSE);
    CLOSEDOWNCONSOLE();
    return (16);
  }
  memset(pre_rec, ' ', MAX_PREFIX_WIDTH + 1);
  pre_rec_len = 0;
  /*
   * Set up filename for directory temporary file (DIR.DIR).
   */
  strcpy((char *) dir_pathname, (char *) user_home_dir);

  strcat((char *) dir_pathname, (char *) dirfilename);
  if (splitpath(dir_pathname) != RC_OK) {
    cleanup();
    STARTUPCONSOLE();
    display_error(7, dir_pathname, FALSE);
    CLOSEDOWNCONSOLE();
    return (18);
  }
  strcpy((char *) dir_pathname, (char *) sp_path);
  strcpy((char *) dir_filename, (char *) sp_fname);

  /*
   * Set up a temporary file name for output from PUT command to go...
   */
  if ((tempfilename = (char_t *) strdup(tmpnam(NULL))) == NULL)       // thetmpnam ( "TMP" )
  {
    cleanup();
    return (31);
  }
  /*
   * Trap signals to exit gracefully, unless user has specified they not
   * be trapped.
   */
  if (trap_signals)
    init_signals();
  /*
   * Initialise rexx support. If no Rexx available, set flag...
   */
  rexx_support = TRUE;
  if (initialise_rexx() != RC_OK)
    rexx_support = FALSE;
  /*
   * Create the builtin parsers...
   */
  if (construct_default_parsers() != RC_OK) {
    cleanup();
    return (32);
  }
  if (construct_default_parser_mapping() != RC_OK) {
    cleanup();
    return (33);
  }
  /*
   * Set SCREEN values up...
   */
  for (i = 0; i < VIEW_WINDOWS; i++)
    CURRENT_SCREEN.win[i] = (WINDOW *) NULL;
  /*
   * Set up global defaults.
   */
  set_global_defaults();
  /*
   * Initialise command array to empty strings.
   */
  init_command();
  /*
   * Set up default screens using the default values of terminal_lines
   * and terminal_cols. These will be altered after initscr().
   */
  screen[0].sl = screen[1].sl = NULL;
  if (batch_only) {
    set_screen_defaults();
    /*
     * Read each file into memory and apply the profile file to each of the
     * files.
     */
    current_file_name = first_file_name;
    while (current_file_name != NULL) {
      if ((rc = get_file((char_t *) current_file_name->line)) != RC_OK) {
        cleanup();
        if (rc == RC_DISK_FULL) {
          STARTUPCONSOLE();
          display_error(57, (char_t *) "...probably", FALSE);
          CLOSEDOWNCONSOLE();
        }
        return (21);
      }
      pre_process_line(CURRENT_VIEW, 0L, (LINE *) NULL);
      if (execute_profile) {
        if (local_prf != (char_t *) NULL)
          rc = get_profile(local_prf, prf_arg);
        if (error_on_screen) {
          error_on_screen = FALSE;
        }
      }
      current_file_name = current_file_name->next;
    }
    first_file_name = lll_free(first_file_name);
    /*
     * If THE has been used only in batch, exit here.
     */
    if (number_of_files != 0) {
      sprintf((char *) rec, "%ld", number_of_files);
      STARTUPCONSOLE();
      display_error(77, rec, FALSE);
      CLOSEDOWNCONSOLE();
    }
    cleanup();
    return (0);
  }                             /* if (batch_only) */
  /*
   * If the platform supports the mouse, set up the default commands.
   */
  initialise_mouse_commands();
/*traceon();*/
/*
 * Initialise Soft Label Keys
 */
#if MAX_SLK == 0
  if (SLKx)
    slk_init(1);
#else
  if (SLKx)
    slk_init(slk_format);
#endif
  /*
   * Start up curses. This is done ONLY for interactive sessions!
   */
  initscr();
  curses_started = TRUE;

  /*
   * Save the value of LINES and COLS and use these for all screen
   * sizing calculations. This is because BSD scrolls if a character is
   * displayed in the bottom right corner of the screen :-(
   */
  terminal_lines = LINES;
  terminal_cols = COLS;
  if ((linebufch = (chtype *) malloc(linebuf_size * sizeof(chtype))) == NULL) {
    cleanup();
    return (30);
  }
  /*
   * Determine if colour support available.
   */
  if (colour_support) {         /* if default setting not overridden on command line */
    colour_support = FALSE;
    if (has_colors()) {
      start_color();
      colour_support = TRUE;
      init_colour_pairs();
    }
  }
  /*
   * Set various terminal characteristics...
   */
  cbreak();
  raw();
  nonl();
  noecho();
  keypad(stdscr, TRUE);
  notimeout(stdscr, TRUE);
  def_prog_mode();
  (void) THETypeahead((char_t *) "OFF");
  /*
   * Set up mouse support if enabled in curses library.
   */
  mousemask(ALL_MOUSE_EVENTS, (mmask_t *) NULL);
  /*
   * Set up variables and values dependent on LINES and COLS now with
   * values set by initscr().
   */
  set_screen_defaults();

  /*
   * wnoutrefresh() is called here so that the first call to getch() on
   * stdscr does not clear the screen.
   */
  wnoutrefresh(stdscr);
  if (SLKx)
    slk_noutrefresh();
  /*
   * Create the statusline window...
   */
  if (create_statusline_window() != RC_OK) {
    cleanup();
    STARTUPCONSOLE();
    display_error(0, (char_t *) "creating status line window", FALSE);
    CLOSEDOWNCONSOLE();
    return (23);
  }
  if (create_filetabs_window() != RC_OK) {
    cleanup();
    STARTUPCONSOLE();
    display_error(0, (char_t *) "creating filetabs window", FALSE);
    CLOSEDOWNCONSOLE();
    return (23);
  }
  /*
   * Set up ETMODE tables...
   */
  (void) Etmode((char_t *) "OFF");
  /*
   * Add the default settings to statusline before the profile
   * file is executed so they can be overridden
   */
  Statopt((char_t *) "ON NBFILE.1 13 0 Files=");
  Statopt((char_t *) "ON WIDTH.1 23 0 Width=");
  /*
   * Read each file into memory and apply the profile file to each of the
   * files.
   */
  current_file_name = first_file_name;
  while (current_file_name != NULL) {
    rc = EditFile((char_t *) current_file_name->line, TRUE);
    if (rc != RC_OK) {
      cleanup();
      return (24);
    }
    current_file_name = current_file_name->next;
  }
  first_file_name = lll_free(first_file_name);
  /*
   * If THE has only been used to process a profile file, then exit.
   */
  if (number_of_files == 0) {
    cleanup();
    return (0);
  }
  /*
   * We are no longer executing the profile file.
   */
  in_profile = FALSE;
  been_interactive = TRUE;
  atexit(atexit_handler);
  /*
   * This is where it all happens!!!
   */
  editor();
  /*
   * Finalise rexx support...
   */
  finalise_rexx();
  /*
   * Free up the dynamically allocated memory.
   */
  if (first_define != NULL)
    first_define = dll_free(first_define);
  if (first_synonym != NULL)
    first_synonym = dll_free(first_synonym);
  if (first_prefix_synonym != NULL)
    first_prefix_synonym = lll_free(first_prefix_synonym);
  if (first_mouse_define != NULL)
    first_mouse_define = dll_free(first_mouse_define);
  free(rec);
  free(trec);
  free(brec);
  free(cmd_rec);
  free(pre_rec);
  free(linebuf);
  free(linebufch);
  if (profile_command_line != NULL)
    free(profile_command_line);
  if (screen[0].sl != NULL)
    free(screen[0].sl);
  if (screen[1].sl != NULL)
    free(screen[1].sl);
  if (the_macro_dir)
    free(the_macro_dir);
  /*
   * Free memory for temp_params and tmp_cmd.
   */
  free_temp_space(TEMP_PARAM);
  free_temp_space(TEMP_SET_PARAM);
  free_temp_space(TEMP_MACRO);
  free_temp_space(TEMP_TEMP_CMD);
  free_temp_space(TEMP_TMP_CMD);

  if (local_prf != NULL)
    free(local_prf);
  if (prf_arg != NULL)
    free(prf_arg);
  free_recovery_list();

  if (target_buffer != NULL)
    free(target_buffer);

  if (divider != (WINDOW *) NULL) {
    delwin(divider);
    divider = (WINDOW *) NULL;
  }
  if (error_window != (WINDOW *) NULL) {
    delwin(error_window);
    error_window = (WINDOW *) NULL;
  }
  if (last_message != NULL)
    free(last_message);
  /*
   * Destroy the builtin parsers...
   */
  destroy_all_parsers();
  first_parser_mapping = mappingll_free(first_parser_mapping);
  /*
   * If the user wants a clearscreen done before exiting, do it...
   */
  if (CLEARSCREENx) {
    wclear(stdscr);
    move(0, 0);
    attrset(A_NORMAL);
    refresh();
  } else
    /*
     * ...otherwise, get the cursor to the bottom line.
     */
  {
    if (statarea != (WINDOW *) NULL) {
      mvwaddstr(statarea, 0, 4, "     ");
      wattrset(statarea, A_NORMAL);
      mvwaddstr(statarea, 0, 0, "THE - END");
      wrefresh(statarea);
    }
  }
  if (statarea != (WINDOW *) NULL) {
    delwin(statarea);
    statarea = (WINDOW *) NULL;
  }
  if (filetabs != (WINDOW *) NULL) {
    delwin(filetabs);
    filetabs = (WINDOW *) NULL;
  }
  last_option = first_option = lll_free(first_option);
  cleanup();
  CLOSEDOWNCONSOLE();
  return (0);
}

int on_interrupt(int sig, int flag); /* in imc/util.c */

static void init_signals(void) {
  signal(SIGQUIT, handle_signal);
  signal(SIGHUP, handle_signal);
  signal(SIGABRT, handle_signal);
  signal(SIGFPE, handle_signal);
  signal(SIGSEGV, handle_signal);
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
#if defined(SIGBUS)
  signal(SIGBUS, handle_signal);
#endif
  signal(SIGWINCH, handle_signal);
  signal(SIGWINCH, handle_signal);
  on_interrupt(SIGWINCH, 1);
#if defined(SIGPIPE)
  signal(SIGPIPE, SIG_IGN);
#endif
  return;
}
void init_colour_pairs(void) {
  short fg, bg;

  /*
   * Force the use of 8 colours instead of using COLORS;
   * THE only knows about 8 basic colours. Change suggested
   * by William McBrine
   */
  for (fg = 0; fg < 8; fg++) {
    for (bg = 0; bg < 8; bg++) {
      if (ATTR2PAIR(fg, bg) <= COLOR_PAIRS) {
        init_pair((short) ATTR2PAIR(fg, bg), fg, bg);
      }
    }
  }
  return;
}
int setup_profile_files(char_t * specified_prf) {
  int rc = RC_OK;
  char *envptr = NULL;
  short errnum = 0;

  /*
   * If a profile specified on the command line, set it up as the local
   * profile. It MUST exist and be readable, otherwise an error.
   */
  if (specified_prf != (char_t *) NULL) {
    if ((local_prf = (char_t *) malloc((MAX_FILE_NAME + 1) * sizeof(char_t))) == NULL)
      return (RC_OUT_OF_MEMORY);
    rc = get_valid_macro_file_name(specified_prf, local_prf, macro_suffix, &errnum);
    if (rc != RC_OK) {
      display_error(9, specified_prf, FALSE);
      free(local_prf);
      local_prf = NULL;
      return (rc);
    }
    if (file_exists(local_prf) != THE_FILE_EXISTS) {
      display_error(9, local_prf, FALSE);
      free(local_prf);
      local_prf = NULL;
      return (RC_FILE_NOT_FOUND);
    }
    /*
     * If the file is not readable, error.
     */
    if (!file_readable(local_prf)) {
      display_error(8, local_prf, FALSE);
      free(local_prf);
      local_prf = NULL;
      return (RC_ACCESS_DENIED);
    }
    return (rc);
  }
  /*
   * If a profile specified with THE_PROFILE_FILE, set it up as the local
   * profile. It does have to exist and be readable..
   */
  if ((envptr = getenv("THE_PROFILE_FILE")) != NULL) {
    if ((local_prf = (char_t *) malloc((MAX_FILE_NAME + 1) * sizeof(char_t))) == NULL)
      return (RC_OUT_OF_MEMORY);
    strcpy((char *) local_prf, envptr);
    if (file_exists(local_prf) != THE_FILE_EXISTS) {
      display_error(9, local_prf, FALSE);
      free(local_prf);
      local_prf = NULL;
      return (RC_FILE_NOT_FOUND);
    }
    /*
     * If the file is not readable, error.
     */
    if (!file_readable(local_prf)) {
      display_error(8, local_prf, FALSE);
      free(local_prf);
      local_prf = NULL;
      return (RC_ACCESS_DENIED);
    }
    return (rc);
  }
  /*
   * No specific profile, so check for default profiles.
   */
  if ((local_prf = (char_t *) malloc((MAX_FILE_NAME + 1) * sizeof(char_t))) == NULL)
    return (RC_OUT_OF_MEMORY);
  /*
   * For Unix, use a local profile first...
   */
  strcpy((char *) local_prf, (char *) user_home_dir);
  strcat((char *) local_prf, (char *) THE_PROFILE_FILE);
  strrmdup(strtrans(local_prf, OSLASH, ISLASH), ISLASH, TRUE);
  if (file_readable(local_prf))
    return (rc);
  /*
   * If no local profile, see if a global profile exists...
   */
  /*
   * Lastly try for a default profile file in THE_HOME_DIR if set
   * or directory where THE executable lives
   */
  strcpy((char *) local_prf, (char *) the_home_dir);
  strcat((char *) local_prf, (char *) THE_PROFILE_FILE);
  strrmdup(strtrans(local_prf, OSLASH, ISLASH), ISLASH, TRUE);
  if (file_readable(local_prf))
    return (rc);
  /*
   * To get here, no profile files to be executed.
   */
  free(local_prf);
  local_prf = (char_t *) NULL;
  return (rc);
}
static void display_info(char_t * argv0) {
  fprintf(stdout, "\nTHE %s %2s %s. All rights reserved.\n", the_version, the_release, the_copyright);
  fprintf(stdout, "THE is distributed under the terms of the GNU General Public License \n");
  fprintf(stdout, "and comes with NO WARRANTY. See the file COPYING for details.\n");
  fprintf(stdout, "\nUsage:\n\n%s [-hnmrsbq] [-p profile] [-a profile_arg] [-l line_num] [-c col_num] [-w width] [-u display_length] [-k[fmt]] [[dir] [file [...]]]\n", argv0);
  fprintf(stdout, "\nwhere:\n\n");
  fprintf(stdout, "-h,--help              show this message\n");
  fprintf(stdout, "-n                     do not execute a profile file\n");
  fprintf(stdout, "-m                     force display into mono\n");
  fprintf(stdout, "-r                     run THE in read-only mode\n");
  fprintf(stdout, "-s                     turn off signal trapping (Unix only)\n");
  fprintf(stdout, "-b                     run in batch mode\n");
  fprintf(stdout, "-q                     run quietly in batch\n");
  fprintf(stdout, "-k[fmt]                allow Soft Label Key display and set format\n");
  fprintf(stdout, "-l line_num            specify line number to make current\n");
  fprintf(stdout, "-c column_num          specify column number to make current\n");
  fprintf(stdout, "-p profile             filename of profile file\n");
  fprintf(stdout, "-a profile_arg         argument(s) to profile file (only with Rexx)\n");
  fprintf(stdout, "-w width               maximum width of line (default 1000)\n");
  fprintf(stdout, "-u display_length      display length in non-line mode\n");
  fprintf(stdout, "[dir [file [...]]]     file(s) and/or directory to be edited\n\n");
  fflush(stdout);
  return;
}
int allocate_working_memory(void) {
  /*
   * Allocate some memory to rec.
   */
  if (rec == NULL)
    rec = (char_t *) malloc((max_line_length + 5) * sizeof(char_t));
  else
    rec = (char_t *) realloc(rec, (max_line_length + 5) * sizeof(char_t));
  if (rec == NULL)
    return (10);
  /*
   * Allocate some memory to trec; buffer for file line.
   */
  max_trec_len = trec_len = max((length_t) (30 * 80), (length_t) ((max_line_length + 2) * 2));
  if (trec == NULL)
    trec = (char_t *) malloc(trec_len * sizeof(char_t));
  else
    trec = (char_t *) realloc(trec, trec_len * sizeof(char_t));
  if (trec == NULL)
    return (11);
  /*
   * Allocate some memory to brec; buffer for reading files.
   */
  brec_len = max((length_t) (30 * 80), (length_t) ((max_line_length + 2)));
  if (brec == NULL)
    brec = (char_t *) malloc(brec_len * sizeof(char_t));
  else
    brec = (char_t *) realloc(brec, brec_len * sizeof(char_t));
  if (brec == NULL)
    return (11);
  /*
   * Allocate memory to cmd_rec and set it to blanks.
   */
  if (cmd_rec == NULL)
    cmd_rec = (char_t *) malloc((max_line_length + 2) * sizeof(char_t));
  else
    cmd_rec = (char_t *) realloc(cmd_rec, (max_line_length + 2) * sizeof(char_t));
  if (cmd_rec == NULL)
    return (12);
  memset(cmd_rec, ' ', max_line_length);
  cmd_rec_len = 0;
  /*
   * Allocate some memory for temporary space...
   */
  if (allocate_temp_space(max_line_length, TEMP_PARAM) != RC_OK)
    return (13);
  if (allocate_temp_space(max_line_length, TEMP_SET_PARAM) != RC_OK)
    return (21);
  if (allocate_temp_space(max_line_length, TEMP_TMP_CMD) != RC_OK)
    return (14);
  if (allocate_temp_space(max_line_length, TEMP_TEMP_CMD) != RC_OK)
    return (15);
  if (allocate_temp_space(max_line_length, TEMP_MACRO) != RC_OK)
    return (19);
  /*
   * Allocate some memory to profile_command_line. (only really need to
   * do this if NO Rexx support).
   */
  if (profile_command_line == NULL)
    profile_command_line = (char_t *) malloc((max_line_length + 2) * sizeof(char_t));
  else
    profile_command_line = (char_t *) realloc(profile_command_line, (max_line_length + 2) * sizeof(char_t));
  if (profile_command_line == NULL)
    return (17);
  return (0);
}

void cleanup(void) {

  if (curses_started) {
    if (error_on_screen && error_window != NULL) {
      display_error(0, (char_t *) HIT_ANY_KEY, FALSE);
      wrefresh(error_window);
      /*
       * Real hack here. If we have an error caused by editing the first file
       * like line too long, then we need to ignore all KEY_RESIZE events; XCurses
       * sends a resize on startup every time!
       */
      while (getch() == KEY_RESIZE);
    }
    INSERTMODEx = FALSE;
/*      draw_cursor(TRUE);*/
    endwin();
    curses_started = FALSE;
  }
  if (!CLEARSCREENx && been_interactive)
    printf("\n");
  /*
   * If we are the initial instance of THE running in single instance
   * mode, remove the FIFO
   */
  if (single_instance_server) {
    close_fifo();
  }
  if (tempfilename) {
    if (file_exists(tempfilename) == THE_FILE_EXISTS)
      remove_file(tempfilename);
    free(tempfilename);
  }
  if (stdinprofile) {
    if (file_exists(stdinprofile) == THE_FILE_EXISTS)
      remove_file(stdinprofile);
    free(stdinprofile);
  }

  /*
   * Free up the working memory
   */
  the_free_flists();
  return;
}

static void handle_signal(int err) {
  FILE_DETAILS *cf;
  VIEW_DETAILS *curr;
  bool process_view = FALSE;
  FILE_DETAILS *first_view_file = NULL;
  register int j = 0;

  /*
   * If a SIGWINCH is caught, set a global flag to indicate that the screen has resized
   * Only do this for NCURSES.
   */
  if (err == SIGWINCH) {
    ncurses_screen_resized = TRUE;
    signal(SIGWINCH, handle_signal);
    return;
  }
  /*
   * For each file in the ring, execute an AUTOSAVE on it and then
   * die.
   */
  if (curses_started) {
    endwin();
    curses_started = FALSE;
  }
  fprintf(stderr, "\nTHE terminated with signal: %d\n\n", err);
  signal(err, SIG_IGN);         /* ignore any more of these signals while autosaving */
  curr = vd_current;
  for (j = 0; j < number_of_files;) {
    process_view = TRUE;
    if (curr->file_for_view->file_views > 1) {
      if (first_view_file == curr->file_for_view)
        process_view = FALSE;
      else
        first_view_file = curr->file_for_view;
    }
    if (process_view) {
      j++;
      cf = curr->file_for_view;
      if (!cf->pseudo_file && cf->save_alt > 0) {
        fprintf(stderr, "Attempting to autosave: %s%s\n", cf->fpath, cf->fname);
        save_file(cf, cf->autosave_fname, TRUE, cf->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, TRUE);
      }
    }
    curr = curr->next;
    if (curr == NULL)
      curr = vd_first;
  }
  /*
   * Lets not push our luck in the signal handler, and just die...
   */
  CLOSEDOWNCONSOLE();
  exit(25);
}

/*
 * Following code allows the program to be built as a Win32 Console app
 * or as a Win32 Windows app.  The idea and code is based on similar code
 * in FLTK.
 */
