// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <memory.h>
#include <regex.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>
#define CURSES_H_INCLUDED

#define ESLASH '/'
#define ESTR_SLASH (uchar *)"/"
#define OSLASH '\\'
#define OSTR_SLASH (uchar *)"\\"
#define ISLASH ESLASH
#define ISTR_SLASH ESTR_SLASH
#define CURRENT_DIR (uchar *)"."

#include "thedefs.h"

#define MAX_SLK_FORMAT 4

#define set_colour(attr) ((colour_support) ? (((attr)->pair) ? COLOR_PAIR((attr)->pair) | (attr)->mod : (attr)->mod) \
                                           : ((attr)->mono))

#define ATTR2PAIR(fg,bg) (bg|(fg<<3))
#define FOREFROMPAIR(p)  (p>>3)
#define BACKFROMPAIR(p)  (p&0x07)

#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define min(a,b)        (((a) < (b)) ? (a) : (b))

#define is_termresized()  (ncurses_screen_resized)

#define QUIT          (-127)
#define SKIP          (-126)
#define QUITOK        (-125)
#define RAW_KEY       (128)

#define MAX_SCREENS         2   /* maximum number of screens */
#define VIEW_WINDOWS        6   /* number of windows per view */
#define MAX_INT             32766       /* maximum size for integer */
#define MAX_LONG            2147483001L /* maximum size for long */
#define WORD_DELIMS        (uchar *)" \t"

#define TOP_OF_FILE        (uchar *)"*** Top of File ***"
#define BOTTOM_OF_FILE     (uchar *)"*** Bottom of File ***"

#define DIRECTION_NONE      0
#define DIRECTION_FORWARD   1
#define DIRECTION_BACKWARD  (-1)

#define UNDEFINED_OPERAND (-1)

#define STATAREA_OFFSET  10

/* the first 6 windows MUST be numbered 0-5 */

#define WINDOW_FILEAREA  0
#define WINDOW_PREFIX    1
#define WINDOW_COMMAND   2
#define WINDOW_ARROW     3
#define WINDOW_IDLINE    4
#define WINDOW_GAP       5
#define WINDOW_STATAREA  6
#define WINDOW_ERROR     7
#define WINDOW_DIVIDER   8
#define WINDOW_RESERVED  9
#define WINDOW_SLK      10
#define WINDOW_FILETABS 11
#define MAX_THE_WINDOWS 12      /* one more than maximum window number */
#define WINDOW_ALL      99

#define MAX_PREFIX_WIDTH     20
#define DEFAULT_PREFIX_WIDTH  6
#define DEFAULT_PREFIX_GAP    0
#define DEFAULT_PREFIX_GAP_LINE    FALSE
#define DEFAULT_FILETABS_GAP_WIDTH 1

#define MAX_CTLCHARS                64
#define MAX_DELIMITER_LENGTH        20
#define MAX_PARSER_NAME_LENGTH      30

#define MAX_SELECT_LEVEL            50000

#define CURRENT_SCREEN              screen[current_screen]
#define other_screen                ((current_screen==0)?1:0)
#define OTHER_SCREEN                screen[other_screen]

#define PREVIOUS_VIEW               (vd_current->prev)
#define NEXT_VIEW                   (vd_current->next)

#define CURRENT_VIEW                (vd_current)
#define CURRENT_FILE                (vd_current->file_for_view)
#define CURRENT_WINDOW              (CURRENT_SCREEN.win[vd_current->current_window])
#define CURRENT_PREV_WINDOW         (CURRENT_SCREEN.win[vd_current->previous_window])
#define CURRENT_WINDOW_FILEAREA     (CURRENT_SCREEN.win[WINDOW_FILEAREA])
#define CURRENT_WINDOW_PREFIX       (CURRENT_SCREEN.win[WINDOW_PREFIX])
#define CURRENT_WINDOW_GAP          (CURRENT_SCREEN.win[WINDOW_GAP])
#define CURRENT_WINDOW_COMMAND      (CURRENT_SCREEN.win[WINDOW_COMMAND])
#define CURRENT_WINDOW_ARROW        (CURRENT_SCREEN.win[WINDOW_ARROW])
#define CURRENT_WINDOW_IDLINE       (CURRENT_SCREEN.win[WINDOW_IDLINE])

#define VIEW_VIEW(view)             (vd_current)
#define VIEW_FILE(view)             (view->file_for_view)
#define VIEW_WINDOW(scr,view)       (screen[(scr)].win[view->current_window])
#define VIEW_PREV_WINDOW(scr,view)  (screen[(scr)].win[view->previous_window])

#define OTHER_VIEW                  (OTHER_SCREEN.screen_view)
#define OTHER_FILE                  (OTHER_VIEW->file_for_view)
#define OTHER_WINDOW                (OTHER_SCREEN.win[OTHER_VIEW->current_window])
#define OTHER_PREV_WINDOW           (OTHER_SCREEN.win[OTHER_VIEW->previous_window])
#define OTHER_WINDOW_FILEAREA       (OTHER_SCREEN.win[WINDOW_FILEAREA])
#define OTHER_WINDOW_PREFIX         (OTHER_SCREEN.win[WINDOW_PREFIX])
#define OTHER_WINDOW_GAP            (OTHER_SCREEN.win[WINDOW_GAP])
#define OTHER_WINDOW_COMMAND        (OTHER_SCREEN.win[WINDOW_COMMAND])
#define OTHER_WINDOW_ARROW          (OTHER_SCREEN.win[WINDOW_ARROW])
#define OTHER_WINDOW_IDLINE         (OTHER_SCREEN.win[WINDOW_IDLINE])

#define SCREEN_VIEW(scr)            (screen[(scr)].screen_view)
#define SCREEN_FILE(scr)            (screen[(scr)].screen_view->file_for_view)
#define SCREEN_WINDOW(scr)          (screen[(scr)].win[screen[(scr)].screen_view->current_window])
#define SCREEN_PREV_WINDOW(scr)     (screen[(scr)].win[screen[(scr)].screen_view->previous_window])
#define SCREEN_WINDOW_FILEAREA(scr) (screen[(scr)].win[WINDOW_FILEAREA])
#define SCREEN_WINDOW_PREFIX(scr)   (screen[(scr)].win[WINDOW_PREFIX])
#define SCREEN_WINDOW_GAP(scr)      (screen[(scr)].win[WINDOW_GAP])
#define SCREEN_WINDOW_COMMAND(scr)  (screen[(scr)].win[WINDOW_COMMAND])
#define SCREEN_WINDOW_ARROW(scr)    (screen[(scr)].win[WINDOW_ARROW])
#define SCREEN_WINDOW_IDLINE(scr)   (screen[(scr)].win[WINDOW_IDLINE])

#define MARK_VIEW (vd_mark)
#define MARK_FILE (vd_mark->file_for_view)

/* split/join actions */

#define SPLTJOIN_SPLIT           1
#define SPLTJOIN_JOIN            2
#define SPLTJOIN_SPLTJOIN        3

/* adjustments for calculating rec_len */

#define ADJUST_DELETE            1
#define ADJUST_INSERT            2
#define ADJUST_OVERWRITE         3

/* column command types */

#define COLUMN_CAPPEND           1
#define COLUMN_CINSERT           2
#define COLUMN_CREPLACE          3
#define COLUMN_COVERLAY          4

/* block move command types */

#define COMMAND_COPY             1
#define COMMAND_DELETE           2
#define COMMAND_DUPLICATE        3
#define COMMAND_MOVE_COPY_SAME   4
#define COMMAND_MOVE_COPY_DIFF   5
#define COMMAND_MOVE_DELETE_SAME 6
#define COMMAND_MOVE_DELETE_DIFF 7
#define COMMAND_OVERLAY_COPY     8
#define COMMAND_OVERLAY_DELETE   9

/* block move source types */

#define SOURCE_UNKNOWN      0
#define SOURCE_COMMAND      1
#define SOURCE_PREFIX       2
#define SOURCE_BLOCK        3
#define SOURCE_BLOCK_RESET  4

/* return code constants */

#define RC_OK                  0
#define RC_NOT_COMMAND        -1
#define RC_INVALID_ENVIRON    -3
#define RC_TOF_EOF_REACHED     1
#define RC_TARGET_NOT_FOUND    2
#define RC_TRUNCATED           3
#define RC_NO_LINES_CHANGED    4
#define RC_INVALID_OPERAND     5
#define RC_COMMAND_NO_FILES    6
#define RC_FILE_CHANGED       12
#define RC_ACCESS_DENIED      12
#define RC_DISK_FULL          13
#define RC_BAD_FILEID         20
#define RC_BAD_DRIVE          24
#define RC_FILE_NOT_FOUND     28
#define RC_OUT_OF_MEMORY      94
#define RC_NOREXX_ERROR       98
#define RC_SYSTEM_ERROR       99
#define RC_IO_ERROR          100
#define RC_READV_TERM        101
#define RC_READV_TERM_MOUSE  102
#define RC_TERMINATE_MACRO   111

/* global parameters */

#define EOLOUT_NONE              0
#define EOLOUT_LF                1
#define EOLOUT_CRLF              2
#define EOLOUT_CR                3

/* extract return codes */

#define EXTRACT_ARG_ERROR        (-2)
#define EXTRACT_VARIABLES_SET    (-1)

/* attribute defines */

#define ATTR_FILEAREA       0
#define ATTR_CURLINE        1
#define ATTR_BLOCK          2
#define ATTR_CBLOCK         3
#define ATTR_CMDLINE        4
#define ATTR_IDLINE         5
#define ATTR_MSGLINE        6
#define ATTR_ARROW          7
#define ATTR_PREFIX         8
#define ATTR_CPREFIX        9
#define ATTR_PENDING       10
#define ATTR_SCALE         11
#define ATTR_TOFEOF        12
#define ATTR_CTOFEOF       13
#define ATTR_TABLINE       14
#define ATTR_SHADOW        15
#define ATTR_STATAREA      16
#define ATTR_DIVIDER       17
#define ATTR_RESERVED      18
#define ATTR_NONDISP       19
#define ATTR_HIGHLIGHT     20
#define ATTR_CHIGHLIGHT    21
#define ATTR_THIGHLIGHT    22
#define ATTR_SLK           23
#define ATTR_GAP           24
#define ATTR_CGAP          25
#define ATTR_ALERT         26
#define ATTR_DIALOG        27
#define ATTR_BOUNDMARK     28
#define ATTR_FILETABS      29
#define ATTR_FILETABSDIV   30
#define ATTR_CURSORLINE    31
#define ATTR_DIA_BORDER    32
#define ATTR_DIA_EDITFIELD 33
#define ATTR_DIA_BUTTON    34
#define ATTR_DIA_ABUTTON   35
#define ATTR_POP_BORDER    36
#define ATTR_POP_CURLINE   37
#define ATTR_POPUP         38
#define ATTR_POP_DIVIDER   39
#define ATTR_MAX           40

/* ecolour defines */

#define ECOLOUR_COMMENTS               0
#define ECOLOUR_STRINGS                1
#define ECOLOUR_NUMBERS                2
#define ECOLOUR_KEYWORDS               3
#define ECOLOUR_LABEL                  4
#define ECOLOUR_PREDIR                 5
#define ECOLOUR_HEADER                 6
#define ECOLOUR_EXTRA_R_PAREN          7
#define ECOLOUR_LEVEL_1_PAREN          8
#define ECOLOUR_LEVEL_1_KEYWORD        9
#define ECOLOUR_LEVEL_1_PREDIR        10
#define ECOLOUR_LEVEL_2_PAREN         11
#define ECOLOUR_LEVEL_3_PAREN         12
#define ECOLOUR_LEVEL_4_PAREN         13
#define ECOLOUR_LEVEL_5_PAREN         14
#define ECOLOUR_LEVEL_6_PAREN         15
#define ECOLOUR_LEVEL_7_PAREN         16
#define ECOLOUR_LEVEL_8_PAREN         17
#define ECOLOUR_INC_STRING            18
#define ECOLOUR_HTML_TAG              19
#define ECOLOUR_HTML_CHAR             20
#define ECOLOUR_FUNCTIONS             21
#define ECOLOUR_DIRECTORY             22
#define ECOLOUR_LINK                  23
#define ECOLOUR_EXECUTABLE            24
#define ECOLOUR_NOTUSED1              25
#define ECOLOUR_ALT_KEYWORD_1         26
#define ECOLOUR_ALT_KEYWORD_2         27
#define ECOLOUR_ALT_KEYWORD_3         28
#define ECOLOUR_ALT_KEYWORD_4         29
#define ECOLOUR_ALT_KEYWORD_5         30
#define ECOLOUR_ALT_KEYWORD_6         31
#define ECOLOUR_ALT_KEYWORD_7         32
#define ECOLOUR_ALT_KEYWORD_8         33
#define ECOLOUR_ALT_KEYWORD_9         34
#define ECOLOUR_MAX                   35

/* display line types */

#define LINE_LINE                  0
#define LINE_TABLINE               1
#define LINE_SCALE                 2
#define LINE_BOUNDS                4
#define LINE_SHADOW                8
#define LINE_TOF                  16
#define LINE_EOF                  32
#define LINE_RESERVED             64
#define LINE_OUT_OF_BOUNDS_ABOVE 128
#define LINE_OUT_OF_BOUNDS_BELOW 256
#define LINE_HEXSHOW             512

/* target types */

#define TARGET_ERR          (-1)
#define TARGET_UNFOUND        0x00000
#define TARGET_ABSOLUTE       0x00001
#define TARGET_RELATIVE       0x00002
#define TARGET_STRING         0x00004
#define TARGET_POINT          0x00008
#define TARGET_BLANK          0x00010
#define TARGET_ALL            0x00020
#define TARGET_BLOCK          0x00040
#define TARGET_BLOCK_ANY      0x00080
#define TARGET_BLOCK_CURRENT  0x00100
#define TARGET_SPARE          0x00200
#define TARGET_FIND           0x00400
#define TARGET_NFIND          0x00800
#define TARGET_FINDUP         0x01000
#define TARGET_NFINDUP        0x02000
#define TARGET_REGEXP         0x04000
#define TARGET_NEW            0x08000
#define TARGET_CHANGED        0x10000
#define TARGET_TAGGED         0x20000
#define TARGET_ALTERED        0x40000
#define TARGET_NORMAL         TARGET_ABSOLUTE|TARGET_RELATIVE|TARGET_STRING|TARGET_POINT|TARGET_BLANK|TARGET_NEW|TARGET_CHANGED|TARGET_TAGGED|TARGET_ALTERED

/* compatiblility modes */

#define COMPAT_THE            1
#define COMPAT_XEDIT          2
#define COMPAT_KEDIT          4
#define COMPAT_KEDITW         8
#define COMPAT_ISPF          16

/* cursor commands */

#define CURSOR_START          (-1)
#define CURSOR_ERROR          (-2)
#define CURSOR_HOME           0
#define CURSOR_HOME_LAST      1
#define CURSOR_SCREEN         2
#define CURSOR_ESCREEN        3
#define CURSOR_CUA            4
#define CURSOR_COLUMN         5
#define CURSOR_FILE           6
#define CURSOR_CMDLINE        7
#define CURSOR_KEDIT          8
#define CURSOR_MOUSE          9
#define CURSOR_GOTO          10
#define CURSOR_PREFIX        11

/* defines for tabbing to fields */

#define WHERE_WINDOW_MASK             0x0000FF00L
#define WHERE_WINDOW_FILEAREA         0x00000100L
#define WHERE_WINDOW_PREFIX_LEFT      0x00000200L
#define WHERE_WINDOW_PREFIX_RIGHT     0x00000400L
#define WHERE_WINDOW_CMDLINE_TOP      0x00000800L
#define WHERE_WINDOW_CMDLINE_BOTTOM   0x00001000L
#define WHERE_ROW_MASK                0x000000FFL
#define WHERE_SCREEN_MASK             0x000F0000L
#define WHERE_SCREEN_FIRST            0x00010000L
#define WHERE_SCREEN_LAST             0x00020000L
#define WHERE_SCREEN_ONLY             0x00040000L

/* defines for headers for syntax highlighting */

#define HEADER_NUMBER                 0x001
#define HEADER_COMMENT                0x002
#define HEADER_STRING                 0x004
#define HEADER_KEYWORD                0x008
#define HEADER_FUNCTION               0x010
#define HEADER_HEADER                 0x020
#define HEADER_LABEL                  0x040
#define HEADER_MATCH                  0x080
#define HEADER_COLUMN                 0x100
#define HEADER_POSTCOMPARE            0x200
#define HEADER_MARKUP                 0x400
#define HEADER_DIRECTORY              0x800
#define HEADER_ALL                    (HEADER_NUMBER|HEADER_COMMENT|HEADER_STRING|HEADER_KEYWORD|HEADER_FUNCTION|HEADER_HEADER|HEADER_LABEL|HEADER_MATCH|HEADER_COLUMN|HEADER_POSTCOMPARE|HEADER_MARKUP|HEADER_DIRECTORY)

/* defines for syntax highlighting status */

#define THE_SYNTAX_NONE               ' '
#define THE_SYNTAX_UNKNOWN            '?'
#define THE_SYNTAX_COMMENT            'C'
#define THE_SYNTAX_FUNCTION           'F'
#define THE_SYNTAX_HEADER             'H'
#define THE_SYNTAX_INCOMPLETESTRING   's'
#define THE_SYNTAX_KEYWORD            'K'
#define THE_SYNTAX_LABEL              'L'
#define THE_SYNTAX_MARKUP             'M'
#define THE_SYNTAX_MATCH              'm'
#define THE_SYNTAX_NUMBER             'N'
#define THE_SYNTAX_POSTCOMPARE        'P'
#define THE_SYNTAX_PREPROCESSOR       '#'
#define THE_SYNTAX_STRING             'S'
#define THE_SYNTAX_DIRECTORY          'd'
#define THE_SYNTAX_LINK               'l'
#define THE_SYNTAX_EXTENSION          'e'
#define THE_SYNTAX_EXECUTABLE         'E'

/* defines for EXTRACT DEFINE */

#define KEY_TYPE_ALL        0
#define KEY_TYPE_KEY        1
#define KEY_TYPE_MOUSE      2

#define THE_MAX_SCREEN_WIDTH          1000

/* Structure for generic linked list item */

struct thelist {
  struct thelist *prev;
  struct thelist *next;
  void *data;
};
typedef struct thelist THELIST;

/* prototype for a linked list delete function */

typedef void (*THELIST_DEL)(void *);

/* struct for LASTOP fields */

struct lastop_struct {
  uchar *value;
  short value_len;
  uchar *command;
  int min_len;
};
typedef struct lastop_struct LASTOP;

#define LASTOP_ALTER                  0
#define LASTOP_CHANGE                 1
#define LASTOP_CLOCATE                2
#define LASTOP_COUNT                  3
#define LASTOP_FIND                   4
#define LASTOP_LOCATE                 5
#define LASTOP_SCHANGE                6
#define LASTOP_TFIND                  7
#define LASTOP_SEARCH                 8
#define LASTOP_MAX                    9

#define PPC_OPERANDS                  3

struct pending_prefix_command {
  struct pending_prefix_command *next;
  struct pending_prefix_command *prev;
  uchar ppc_orig_command[MAX_PREFIX_WIDTH + 1];     /* prefix command */
  uchar ppc_command[MAX_PREFIX_WIDTH + 1];          /* prefix command */
  uchar ppc_op[PPC_OPERANDS][MAX_PREFIX_WIDTH + 1]; /* operands */
  long ppc_line_number;                             /* line number in file */
  long ppc_cmd_param;                               /* prefix command target */
  short ppc_cmd_idx;                                /* prefix command index */
  bool ppc_block_command;                           /* is it a BLOCK command */
  bool ppc_shadow_line;                             /* was command entered on SHADOW line ? */
  bool ppc_set_by_pending;                          /* was command created by SET PENDING ? */
  bool ppc_processed;                               /* has command been processed ? */
  bool ppc_current_command;                         /* is this command the current executing one ? */
};
typedef struct pending_prefix_command THE_PPC;

typedef struct {
  unsigned int new_flag;
  unsigned int changed_flag;
  unsigned int tag_flag;
  unsigned int save_tag_flag;
  unsigned int unused1;
  unsigned int unused2;
  unsigned int unused3;
  unsigned int unused4;
} lineflags;

struct line {
  struct line *prev;            /* pointer to previous line */
  struct line *next;            /* pointer to next line */
  uchar *name;                  /* used for other structures; NOT for a LINE in THE */
  THELIST *first_name;          /* pointer to first name for list of names */
  uchar *line;                  /* pointer to contents of line */
  long length;                  /* number of characters in line */
  THE_PPC *pre;
  ushort select;                /* select level for each line */
  ushort save_select;           /* saved select level (used by ALL) */
  lineflags flags;
};
typedef struct line LINE;

struct colour_attr {
  int pair;                     /* pair number for colour */
  chtype mod;                   /* colour modifier */
  chtype mono;                  /* mono attributes */
};
typedef struct colour_attr COLOUR_ATTR;

struct reserved {
  struct reserved *prev;        /* pointer to previous reserved line */
  struct reserved *next;        /* pointer to next reserved line */
  uchar *line;                  /* pointer to contents of line */
  uchar *disp;                  /* pointer to contents of line after CTLCHAR applied */
  uchar *spec;                  /* row position specification */
  short length;                 /* length of reserved line */
  short disp_length;            /* length of reserved line after CTLCHAR applied */
  short base;                   /* row base */
  short off;                    /* row offset from base */
  chtype *highlighting;         /* array of colours for highlighting */
  COLOUR_ATTR *attr;            /* colour attributes */
  bool autoscroll;              /* does the reserved line scroll with autoscroll settings */
};
typedef struct reserved RESERVED;

struct prefix_commands {
  uchar *cmd;                   /* prefix command */
  long cmd_len;                 /* length of prefix command */
  bool action_prefix;           /* is command an action or a target */
  bool multiples_allowed;       /* are multiples allowed */
  bool full_target_allowed;     /* full target multiple allowed */
  bool block_prefix_command;    /* is command a block command */
  bool target_required;         /* does command require target */
  bool valid_on_tof;            /* is command allowed on Top of File line */
  bool valid_on_bof;            /* is command allowed on Bottom of File line */
  bool valid_in_readonly;       /* TRUE if command valid in readonly mode */
  short (*function)  (THE_PPC *, short, long);
  long default_target;          /* number of lines to process if not specified */
  bool ignore_scope;            /* TRUE if scope to be ignored when finding target */
  bool use_last_not_in_scope;   /* TRUE if starting at end of shadow lines */
  int priority;                 /* priority of prefix command */
  short (*post_function)  (THE_PPC *, short, long);
  bool text_arg;                /* is argument a plain text arg like for '.' ? */
  bool allowed_on_shadow_line;  /* is command allowed on shadow line ? */
};
typedef struct prefix_commands PREFIX_COMMAND;

#define PC_IS_ACTION             TRUE
#define PC_NOT_ACTION            FALSE
#define PC_MULTIPLES             TRUE
#define PC_NO_MULTIPLES          FALSE
#define PC_FULL_TARGET           TRUE
#define PC_NO_FULL_TARGET        FALSE
#define PC_IS_BLOCK              TRUE
#define PC_NOT_BLOCK             FALSE
#define PC_TARGET_REQD           TRUE
#define PC_TARGET_NOT_REQD       FALSE
#define PC_VALID_TOF             TRUE
#define PC_INVALID_TOF           FALSE
#define PC_VALID_BOF             TRUE
#define PC_INVALID_BOF           FALSE
#define PC_VALID_RO              TRUE
#define PC_INVALID_RO            FALSE
#define PC_IGNORE_SCOPE          TRUE
#define PC_RESPECT_SCOPE         FALSE
#define PC_USE_LAST_IN_SCOPE     TRUE
#define PC_NO_USE_LAST_IN_SCOPE  FALSE

struct parse_comments {
  struct parse_comments *prev;
  struct parse_comments *next;
  bool line_comment;                           /* TRUE if this is a LINE comment */
  long column;                                 /* 0-ANY MAX_INT-FIRSTNONBLANK other-column */
  uchar start_delim[MAX_DELIMITER_LENGTH + 1]; /* delimiter string for start of comment */
  uchar end_delim[MAX_DELIMITER_LENGTH + 1];   /* delimiter string for end of comment */
  short len_start_delim;
  short len_end_delim;
  bool nested;                                 /* can paired comments be nested */
  bool single_line;                            /* is this comment only allowed on a single line ? */
};
typedef struct parse_comments PARSE_COMMENTS;

struct parse_keywords {
  struct parse_keywords *prev;
  struct parse_keywords *next;
  uchar *keyword;
  short keyword_length;
  uchar alternate;
  uchar type;
};
typedef struct parse_keywords PARSE_KEYWORDS;

struct parse_functions {
  struct parse_functions *prev;
  struct parse_functions *next;
  uchar *function;
  short function_length;
  uchar alternate;
};
typedef struct parse_functions PARSE_FUNCTIONS;

struct parse_extension {
  struct parse_extension *prev;
  struct parse_extension *next;
  uchar *extension;
  short extension_length;
  uchar alternate;
};
typedef struct parse_extension PARSE_EXTENSION;

struct parse_headers {
  struct parse_headers *prev;
  struct parse_headers *next;
  uchar header_delim[MAX_DELIMITER_LENGTH + 1];
  short len_header_delim;
  long header_column;           /* 0-ANY MAX_INT-FIRSTNONBLANK other-column */
  uchar alternate;
};
typedef struct parse_headers PARSE_HEADERS;

struct parse_postcompare {
  struct parse_postcompare *prev;
  struct parse_postcompare *next;
  bool is_class_type;
  regex_t pattern_buffer;
  uchar *string;
  short string_length;
  uchar alternate;
};
typedef struct parse_postcompare PARSE_POSTCOMPARE;

#define MAX_PARSER_COLUMNS 20

struct parser_details {
  struct parser_details *prev;
  struct parser_details *next;
  /*
   * Parser-level features
   */
  uchar parser_name[MAX_PARSER_NAME_LENGTH + 1];
  uchar filename[MAX_FILE_NAME + 1];
  regex_t body_pattern_buffer;
  bool have_body_pattern_buffer;
  /*
   * string features
   */
  bool have_string;
  bool check_single_quote;
  bool backslash_single_quote;
  bool check_double_quote;
  bool backslash_double_quote;
  uchar string_delimiter;
  bool backslash_delimiter;
  /*
   * comments features
   */
  PARSE_COMMENTS *first_comments;
  PARSE_COMMENTS *current_comments;
  bool have_paired_comments;
  bool have_line_comments;
  /*
   * keyword features
   */
  PARSE_KEYWORDS *first_keyword;
  PARSE_KEYWORDS *current_keyword;
  short min_keyword_length;
  /*
   * function features
   */
  PARSE_FUNCTIONS *first_function;
  PARSE_FUNCTIONS *current_function;
  short min_function_length;
  regex_t function_pattern_buffer;
  bool have_function_pattern_buffer;
  bool have_function_option_alternate;
  uchar function_option_alternate;
  /*
   * case features
   */
  bool case_sensitive;
  /*
   * option
   */
  bool rexx_option;
  bool preprocessor_option;
  bool function_option;
  uchar preprocessor_char;
  uchar function_char;
  bool function_blank;
  /*
   * match features - minimal at the moment
   */
  bool have_match;
  /*
   * header features
   */
  PARSE_HEADERS *first_header;
  PARSE_HEADERS *current_header;
  bool have_headers;
  /*
   * label features
   */
  uchar label_delim[11];
  short len_label_delim;
  long label_column;            /* 0-ANY MAX_INT-FIRSTNONBLANK other-column */
  /*
   * markup features
   */
  bool have_markup_tag;
  uchar markup_tag_start_delim[MAX_DELIMITER_LENGTH + 1];
  short len_markup_tag_start_delim;
  uchar markup_tag_end_delim[MAX_DELIMITER_LENGTH + 1];
  short len_markup_tag_end_delim;
  bool have_markup_reference;
  uchar markup_reference_start_delim[MAX_DELIMITER_LENGTH + 1];
  short len_markup_reference_start_delim;
  uchar markup_reference_end_delim[MAX_DELIMITER_LENGTH + 1];
  short len_markup_reference_end_delim;
  /*
   * postcompare features
   */
  bool have_postcompare;
  PARSE_POSTCOMPARE *first_postcompare;
  PARSE_POSTCOMPARE *current_postcompare;
  /*
   * column features
   */
  bool have_columns;
  long first_column[MAX_PARSER_COLUMNS];
  long last_column[MAX_PARSER_COLUMNS];
  short number_columns;
  uchar column_alternate[MAX_PARSER_COLUMNS];
  /*
   * number features
   */
  regex_t number_pattern_buffer;
  bool have_number_pattern_buffer;
  /*
   *  link features
   */
  bool have_directory_link;
  uchar link_option_alternate;
  /*
   *  directory features
   */
  bool have_directory_directory;
  uchar directory_option_alternate;
  /*
   * extensions features
   */
  bool have_extensions;
  PARSE_EXTENSION *first_extension;
  PARSE_EXTENSION *current_extension;
  /*
   *  executable features
   */
  bool have_executable;
  uchar executable_option_alternate;
};
typedef struct parser_details PARSER_DETAILS;

struct parser_mapping {
  struct parser_mapping *prev;
  struct parser_mapping *next;
  uchar *parser_name;
  uchar *filemask;
  uchar *magic_number;
  int magic_number_length;
  PARSER_DETAILS *parser;
};
typedef struct parser_mapping PARSER_MAPPING;

/* structure for repeating targets */

struct rtarget {
  uchar *string;                /* pointer to target */
  long length;                  /* length of specified target: string */
  long found_length;            /* length of matching string */
  long start;                   /* starting column of found string target */
  uchar boolean;                /* boolean operator */
  bool not_target;              /* TRUE if NOT target */
  long numeric_target;          /* numeric target value */
  long target_type;             /* type of target */
  bool negative;                /* TRUE if direction backward */
  bool found;                   /* TRUE if this repeating target was found */
  bool have_compiled_re;        /* TRUE if we have a compiled RE */
  regex_t pattern_buffer;       /* compiled RE for REGEXP */
};
typedef struct rtarget RTARGET;

/* structure for targets */

struct target {
  uchar *string;                /* pointer to original target */
  long num_lines;               /* number of lines to target */
  long true_line;               /* line number to start with */
  long last_line;               /* line number of last line in target */
  RTARGET *rt;                  /* pointer to repeating targets */
  short num_targets;            /* number of targets found */
  short spare;                  /* index to which repeating target is spare */
  bool ignore_scope;            /* TRUE if scope to be ignored when finding target */
  bool search_semantics;        /* TRUE if SEARCHing */
  long focus_column;            /* used when SEARCHing */
  bool all_tag_command;         /* true if finding rtargets with TAG or ALL */
};
typedef struct target TARGET;

typedef struct {
  uchar autosave;
  short backup;
  COLOUR_ATTR *attr;
  COLOUR_ATTR *ecolour;         /* array of ECOLOURS for this file */
  uchar eolout;
  bool tabsout_on;
  bool display_actual_filename;
  bool undoing;
  bool timecheck;
  uchar tabsout_num;
  short trailing;               /* how to handle trailing spaces on file write */
  bool colouring;               /* specifies if syntax highlighting is on */
  bool autocolour;              /* specifies if AUTOCOLOUR is on */
  PARSER_DETAILS *parser;       /* parser to use for syntax highlighting */
} PRESERVED_FILE_DETAILS;

typedef struct {
  uchar autosave;               /* number of alterations before autosaving */
  short backup;                 /* indicates type of backup file to be saved */
  COLOUR_ATTR *attr;            /* colour attributes */
  COLOUR_ATTR *ecolour;         /* array of ECOLOURS for this file */
  uchar eolout;                 /* indicates how lines are terminated on output */
  bool tabsout_on;              /* indicates if tabs to replace spaces on file */
  bool display_actual_filename;
  bool undoing;
  bool timecheck;               /* file time stamp checking */
  uchar tabsout_num;            /* length of tab stops on file */
  short trailing;               /* how to handle trailing spaces on file write */
  bool colouring;               /* specifies if syntax highlighting is on */
  bool autocolour;              /* specifies if AUTOCOLOUR is on */
  PARSER_DETAILS *parser;       /* parser to use for syntax highlighting */
  /*
   * All settings above this line are saveable.
   * Ensure that PRESERVED_FILE_DETAILS structure reflects this.
   */
  PRESERVED_FILE_DETAILS *preserved_file_details;
  ushort status_row;            /* row on which status line is displayed */
  uchar pseudo_file;            /* indicates if file is a "pseudo" file and if so, what sort */
  uchar disposition;            /* indicates if file is new or existing */
  unsigned short autosave_alt;  /* number of alterations since last autosave */
  unsigned short save_alt;      /* number of alterations since last save */
  uchar *autosave_fname;        /* file name for AUTOSAVE file */
  FILE *fp;                     /* file handle for this file */
  uchar *fname;                 /* file name */
  uchar *fpath;                 /* file path */
  uchar *actualfname;           /* filename specified */
  uchar *efileid;               /* original full filename */
  uchar *display_name;          /* optional filename to display */
  unsigned short fmode;         /* file mode of file */
  uid_t uid;                    /* userid of file */
  gid_t gid;                    /* groupid of file */
  long modtime;                 /* timestamp of file modification */
  LINE *first_line;             /* pointer to first line */
  LINE *last_line;              /* pointer to last line */
  LINE *editv;                  /* pointer for EDITV variables */
  long number_lines;            /* number of actual lines in file */
  long max_line_length;         /* Maximum line length in file */
  uchar file_views;             /* number of views of current file */
  RESERVED *first_reserved;     /* pointer to first reserved line */
  THE_PPC *first_ppc;           /* first pending prefix command */
  THE_PPC *last_ppc;            /* last pending prefix command */
  uchar eolfirst;               /* indicates termination of first line read */
  int readonly;                 /* have we set the file to be readonly */
} FILE_DETAILS;

typedef struct {
  struct view_details *prev;    /* pointer to previous view */
  struct view_details *next;    /* pointer to next view */
  bool arbchar_status;          /* indicates if arbchar is on */
  uchar arbchar_single;         /* single arbitrary character value */
  uchar arbchar_multiple;       /* multiple arbitrary character value */
  bool arrow_on;                /* indicates if arrow is displayed */
  uchar case_enter;             /* indicates case of data entered */
  uchar case_enter_cmdline;     /* indicates case of data entered on cmdline */
  uchar case_enter_prefix;      /* indicates case of data entered in prefix */
  uchar case_locate;            /* indicates case of data located */
  uchar case_change;            /* indicates case of data changed */
  uchar case_sort;              /* indicates case significance for sorting */
  ushort cmd_line;              /* position of command line */
  ushort current_row;           /* row which is current row */
  short current_base;           /* indicates relative position of current line */
  short current_off;            /* offset from current_base for current_row */
  ushort display_low;           /* low range of display level */
  ushort display_high;          /* high range of display level */
  bool hex;                     /* TRUE if hex conversion is done on string operands */
  bool hexshow_on;              /* status of hexshow */
  short hexshow_base;           /* base position for starting row of hexshow */
  short hexshow_off;            /* offset from base of start of hexshow */
  uchar highlight;              /* lines to highlight, if any */
  ushort highlight_high;        /* high select level of highlighted lines */
  ushort highlight_low;         /* low select level of highlighted lines */
  bool id_line;                 /* TRUE if IDLINE displayed */
  bool imp_macro;               /* indicates if implied macro processing is on */
  bool imp_os;                  /* indicates if implied os processing is on */
  uchar inputmode;              /* indicates type of input processing */
  bool linend_status;           /* indicates if multiple commands allowed on command line */
  uchar linend_value;           /* specifies command delimiter */
  bool macro;                   /* indicates if macros are executed before commands */
  long margin_left;             /* left margin column 1 based */
  long margin_right;            /* right margin column 1 based */
  long margin_indent;           /* paragraph indentation */
  bool margin_indent_offset_status;     /* TRUE if paragraph indentation is an offset from left margin */
  short msgline_base;           /* indicates relative position of msgline */
  short msgline_off;            /* offset from msgline_base for msgline */
  ushort msgline_rows;          /* number of rows in msgline */
  bool msgmode_status;          /* indicates if messages are to be displayed */
  bool newline_aligned;         /* TRUE if adding a new line results in cursor appearing under 1st non-blank of previous line */
  bool number;                  /* indicates if numbers in prefix are to be displayed */
  bool position_status;         /* TRUE if LINE/COL is displayed on IDLINE */
  uchar prefix;                 /* indicates if and where prefix is displayed */
  short prefix_width;           /* overall width of prefix */
  short prefix_gap;             /* width of gap between prefix and filearea */
  bool prefix_gap_line;         /* is vertical line shown in prefix gap */
  bool scale_on;                /* indicates if scale is displayed */
  short scale_base;             /* base position on which scale row is displayed */
  short scale_off;              /* offset from base position on which scale row is displayed */
  bool scope_all;               /* indicates if commands act on All lines or only those Displayed */
  bool shadow;                  /* indicates if shadow lines are to be displayed */
  bool stay;                    /* indicates status of STAY */
  bool synonym;                 /* indicates if synonym processing is in effect */
  bool tab_on;                  /* indicates if tab line is to be displayed */
  short tab_base;               /* base position on which tab line is displayed */
  short tab_off;                /* offset from base position on which tab line is displayed */
  bool tabsinc;                 /* tab increment or 0 if fixed tabs */
  ushort numtabs;               /* number of tab stops defined */
  long tabs[MAX_NUMTABS];       /* tab settings for each tab stop */
  long verify_col;              /* left col for current verify */
  long verify_start;            /* col of start of verify */
  long verify_end;              /* col of end of verify */
  bool verify_end_max;          /* TRUE if verify end was specified as * */
  uchar word;                   /* word setting */
  bool wordwrap;                /* wordwrap setting */
  bool wrap;                    /* wrap setting */
  bool tofeof;                  /* true if want to display TOF/EOF lines */
  long zone_start;              /* col of start of zone */
  long zone_end;                /* col of end of zone */
  bool zone_end_max;            /* TRUE if zone end was specified as * */
  long autoscroll;              /* 0 - no autoscroll, -1 half, other number */
  uchar boundmark;              /* type of boundmark */
  long syntax_headers;          /* which syntax headers to be applied */
  bool thighlight_on;           /* indicates if THIGHLIGHT is on */
  bool thighlight_active;       /* indicates if THIGHLIGHT is active */
  TARGET thighlight_target;     /* details of target to highlight */
} PRESERVED_VIEW_DETAILS;

struct view_details {
  struct view_details *prev;    /* pointer to previous view */
  struct view_details *next;    /* pointer to next view */
  bool arbchar_status;          /* indicates if arbchar is on */
  uchar arbchar_single;         /* single arbitrary character value */
  uchar arbchar_multiple;       /* multiple arbitrary character value */
  bool arrow_on;                /* indicates if arrow is displayed */
  uchar case_enter;             /* indicates case of data entered */
  uchar case_enter_cmdline;     /* indicates case of data entered on cmdline */
  uchar case_enter_prefix;      /* indicates case of data entered in prefix */
  uchar case_locate;            /* indicates case of data located */
  uchar case_change;            /* indicates case of data changed */
  uchar case_sort;              /* indicates case significance for sorting */
  ushort cmd_line;              /* position of command line */
  ushort current_row;           /* row which is current row */
  short current_base;           /* indicates relative position of current line */
  short current_off;            /* offset from current_base for current_row */
  ushort display_low;           /* low range of display level */
  ushort display_high;          /* high range of display level */
  bool hex;                     /* TRUE if hex conversion is done on string operands */
  bool hexshow_on;              /* status of hexshow */
  short hexshow_base;           /* base position for starting row of hexshow */
  short hexshow_off;            /* offset from base of start of hexshow */
  uchar highlight;              /* lines to highlight, if any */
  ushort highlight_high;        /* high select level of highlighted lines */
  ushort highlight_low;         /* low select level of highlighted lines */
  bool id_line;                 /* TRUE if IDLINE displayed */
  bool imp_macro;               /* indicates if implied macro processing is on */
  bool imp_os;                  /* indicates if implied os processing is on */
  uchar inputmode;              /* indicates type of input processing */
  bool linend_status;           /* indicates if multiple commands allowed on command line */
  uchar linend_value;           /* specifies command delimiter */
  bool macro;                   /* indicates if macros are executed before commands */
  long margin_left;             /* left margin column 1 based */
  long margin_right;            /* right margin column 1 based */
  long margin_indent;           /* paragraph indentation */
  bool margin_indent_offset_status;     /* TRUE if paragraph indentation is an offset from left margin */
  short msgline_base;           /* indicates relative position of msgline */
  short msgline_off;            /* offset from msgline_base for msgline */
  ushort msgline_rows;          /* number of rows in msgline */
  bool msgmode_status;          /* indicates if messages are to be displayed */
  bool newline_aligned;         /* TRUE if adding a new line results in cursor appearing under 1st non-blank of previous line */
  bool number;                  /* indicates if numbers in prefix are to be displayed */
  bool position_status;         /* TRUE if LINE/COL is displayed on IDLINE */
  uchar prefix;                 /* indicates if and where prefix is displayed */
  short prefix_width;           /* overall width of prefix */
  short prefix_gap;             /* width of gap between prefix and filearea */
  bool prefix_gap_line;         /* is vertical line shown in prefix gap */
  bool scale_on;                /* indicates if scale is displayed */
  short scale_base;             /* base position on which scale row is displayed */
  short scale_off;              /* offset from base position on which scale row is displayed */
  bool scope_all;               /* indicates if commands act on All lines or only those Displayed */
  bool shadow;                  /* indicates if shadow lines are to be displayed */
  bool stay;                    /* indicates status of STAY */
  bool synonym;                 /* indicates if synonym processing is in effect */
  bool tab_on;                  /* indicates if tab line is to be displayed */
  short tab_base;               /* base position on which tab line is displayed */
  short tab_off;                /* offset from base position on which tab line is displayed */
  bool tabsinc;                 /* tab increment or 0 if fixed tabs */
  ushort numtabs;               /* number of tab stops defined */
  long tabs[MAX_NUMTABS];       /* tab settings for each tab stop */
  long verify_col;              /* left col for current verify */
  long verify_start;            /* col of start of verify */
  long verify_end;              /* col of end of verify */
  bool verify_end_max;          /* TRUE if verify end was specified as * */
  uchar word;                   /* word setting */
  bool wordwrap;                /* wordwrap setting */
  bool wrap;                    /* wrap setting */
  bool tofeof;                  /* true if want to display TOF/EOF lines */
  long zone_start;              /* col of start of zone */
  long zone_end;                /* col of end of zone */
  bool zone_end_max;            /* TRUE if zone end was specified as * */
  long autoscroll;              /* 0 - no autoscroll, -1 half, other number */
  uchar boundmark;              /* type of boundmark */
  long syntax_headers;          /* which syntax headers to be applied */
  bool thighlight_on;           /* indicates if THIGHLIGHT is on */
  bool thighlight_active;       /* indicates if THIGHLIGHT is active */
  TARGET thighlight_target;     /* details of target to highlight */
  /*
   * All settings above this line are saveable.
   * Ensure that PRESERVED_VIEW_DETAILS structure reflects this.
   */
  PRESERVED_VIEW_DETAILS *preserved_view_details;
  long current_line;            /* line in file displayed on current row */
  long current_column;          /* column in line of last column target */
  short y[VIEW_WINDOWS];        /* y coordinate for each window */
  short x[VIEW_WINDOWS];        /* x coordinate for each window */
  long focus_line;              /* line in file where cursor is */
  short mark_type;              /* type of marked block */
  long mark_start_line;         /* first line to be marked */
  long mark_end_line;           /* last line to be marked */
  bool marked_line;             /* TRUE if line marked */
  bool in_ring;                 /* TRUE if file already in edit ring */
  long mark_start_col;          /* first column marked */
  long mark_end_col;            /* last column marked */
  bool marked_col;              /* TRUE if column marked */
  int cmdline_col;              /* column to display in cmdline */
  uchar current_window;         /* current window for current screen */
  uchar previous_window;        /* previous window for current screen */
  FILE_DETAILS *file_for_view;  /* pointer to file structure */
};
typedef struct view_details VIEW_DETAILS;

/* structure for each line to be displayed */

struct show_line {
  uchar *contents;              /* pointer to contents of line */
  RESERVED *rsrvd;              /* pointer to reserved line struct if a reserved line */
  long number_lines_excluded;   /* number of lines excluded */
  long line_number;             /* line number within file */
  LINE *current;                /* pointer to current line */
  short line_type;              /* type of line */
  bool prefix_enterable;        /* TRUE if prefix can be tabbed to */
  bool main_enterable;          /* TRUE if filearea can be tabbed to */
  bool highlight;               /* TRUE if line is highlighted */

  /* NOTE: The following entries are only for displaying.
   * They are NOT updated by a call to build_screen.
   * Therefore you can't access them.
   */

  long length;                  /* number of characters in line */
  chtype normal_colour;         /* normal colour for line */
  chtype other_colour;          /* other colour for line */
  long other_start_col;         /* start column of other colour from col 0 */
  long other_end_col;           /* end column of other colour from col 0 */
  chtype prefix_colour;         /* colour of prefix */
  chtype gap_colour;            /* colour of prefix gap */
  uchar prefix[MAX_PREFIX_WIDTH + 1];               /* contents of prefix area */
  chtype prefix_highlighting[MAX_PREFIX_WIDTH + 1]; /* array of colours for syntax highlighting */
  uchar gap[MAX_PREFIX_WIDTH + 1];                  /* contents of prefix gap */
  chtype gap_highlighting[MAX_PREFIX_WIDTH + 1];    /* array of colours for syntax highlighting */
  /*
   * The following 2 array MUST be the same size
   */
  chtype highlighting[THE_MAX_SCREEN_WIDTH];     /* array of colours for syntax highlighting */
  // uchar highlight_type[THE_MAX_SCREEN_WIDTH]; /* array of syntax types for later querying */
  unsigned char *highlight_type;
  bool is_highlighting;                   /* TRUE if this line contains syntax highlighting */
  bool is_current_line;                   /* TRUE if this line is the current line */
  bool is_cursor_line;                    /* TRUE if this line is the cursor line of the filearea */
  bool is_cursor_line_filearea_different; /* TRUE if the filearea/cursorline colours are different */
};
typedef struct show_line SHOW_LINE;

/* structure for each screen */

typedef struct {
  ushort screen_start_row;        /* start row of screen */
  ushort screen_start_col;        /* start col of screen */
  ushort screen_rows;             /* physical rows */
  ushort screen_cols;             /* physical cols */
  ushort rows[VIEW_WINDOWS];      /* rows in window */
  ushort cols[VIEW_WINDOWS];      /* cols in window */
  ushort start_row[VIEW_WINDOWS]; /* start row of window */
  ushort start_col[VIEW_WINDOWS]; /* start col of window */
  WINDOW *win[VIEW_WINDOWS];      /* curses windows for the screen display */
  VIEW_DETAILS *screen_view;      /* view being displayed in this screen */
  SHOW_LINE *sl;                  /* pointer to SHOW_DETAILS structure for screen */
} SCREEN_DETAILS;

/* structure for colour definitions */

typedef struct {
  chtype fore;
  chtype back;
  chtype mod;
  chtype mono;
} COLOUR_DEF;

/* structure for regular expression syntaxes */

struct regexp_syntax {
  char *name;
  int value;
};

#define STATUS_ROW         (screen_rows-1)
#define COMMAND_ROW        (screen_rows-2)
#define ERROR_ROW          1
#define TAB_ROW            6
#define TAB_ON             NO
#define SCALE_ROW          6
#define SCALE_ON           NO
#define CURSOR_ROW         COMMAND_ROW
#define CURSOR_COL         5
#define CURRENT_ROW_POS    0
#define CURRENT_ROW        0
#define ZONE_START         1

/* column where filename starts in DIR.DIR */

#define FILE_START         38

/* defines for base value for relative row positions */

#define POSITION_TOP       0
#define POSITION_MIDDLE    1
#define POSITION_BOTTOM    2

/* defines for function_key() function calling */

#define OPTION_NORMAL      0
#define OPTION_EXTRACT     1
#define OPTION_READV       2

/* defines for pseudo file types */

#define PSEUDO_REAL        0
#define PSEUDO_DIR         1
#define PSEUDO_REXX        2
#define PSEUDO_KEY         3
#define PSEUDO_REMOTE      4

/* defines for prefix settings */

#define PREFIX_OFF           0x00
#define PREFIX_ON            0x10
#define PREFIX_NULLS         0x20
#define PREFIX_LEFT          0x01
#define PREFIX_RIGHT         0x02
#define PREFIX_LOCATION_MASK 0x0F
#define PREFIX_STATUS_MASK   0xF0

/* defines for query types */

#define QUERY_NONE         0
#define QUERY_QUERY        1
#define QUERY_STATUS       2
#define QUERY_EXTRACT      4
#define QUERY_FUNCTION     8
#define QUERY_MODIFY      16
#define QUERY_READV       32

/* defines for case settings */

#define CASE_MIXED         (uchar)'M'
#define CASE_UPPER         (uchar)'U'
#define CASE_LOWER         (uchar)'L'
#define CASE_IGNORE        (uchar)'I'
#define CASE_RESPECT       (uchar)'R'

/* type of marked blocks - do not change these values!! SET BLOCK needs them */

#define M_NONE             0
#define M_LINE             1
#define M_BOX              2
#define M_STREAM           3
#define M_COLUMN           4
#define M_WORD             5
#define M_CUA              6

/* defines for temporary space allocation */

#define TEMP_PARAM         1
#define TEMP_MACRO         2
#define TEMP_TMP_CMD       3
#define TEMP_TEMP_CMD      4
#define TEMP_SET_PARAM     5

/* defines for [SET] BACKUP */

#define BACKUP_OFF         1
#define BACKUP_TEMP        2
#define BACKUP_KEEP        3
#define BACKUP_ON          BACKUP_KEEP
#define BACKUP_INPLACE     4

/* defines for [SET] DIRSORT */

#define DIRSORT_NONE       0
#define DIRSORT_DIR        1
#define DIRSORT_SIZE       2
#define DIRSORT_NAME       3
#define DIRSORT_DATE       4
#define DIRSORT_TIME       5
#define DIRSORT_ASC        0
#define DIRSORT_DESC       1

/* box opertaions */

#define BOX_C           1
#define BOX_M           2
#define BOX_D           3
#define BOX_F           4
#define BOX_S           5

/* defines for highlighting */

#define HIGHLIGHT_NONE     0
#define HIGHLIGHT_TAG      1
#define HIGHLIGHT_ALT      2
#define HIGHLIGHT_SELECT   3

/* defines for INPUTMODE */

#define INPUTMODE_OFF      0
#define INPUTMODE_FULL     1
#define INPUTMODE_LINE     2

/* defines for EDITV - suprise! */

#define EDITV_GET      1
#define EDITV_PUT      2
#define EDITV_SET      3
#define EDITV_SETL     4
#define EDITV_LIST     5
#define EDITV_GETSTEM  6 /* THE extension */

#define FILE_NORMAL        0
#define FILE_READONLY      1
#define FILE_NEW          99

#define COMMAND_ONLY_TRUE    TRUE
#define COMMAND_ONLY_FALSE   FALSE

#define ADDCHAR            0
#define INSCHAR            1

#define LVL_GLOB  1
#define LVL_FILE  2
#define LVL_VIEW  3

#define CHAR_OTHER     0
#define CHAR_ALPHANUM  1
#define CHAR_SPACE     2

/* Following defines for KEY shift status */

#define SHIFT_ALT            1
#define SHIFT_CTRL           2
#define SHIFT_SHIFT          4
#define SHIFT_MODIFIER_ONLY  8

/* Following defines for MOUSE shift status - yes they are different to the KEY shift status */

#define SHIFT_MOUSE_SHIFT    1
#define SHIFT_MOUSE_CTRL     2
#define SHIFT_MOUSE_ALT      4

#define INTERFACE_CLASSIC    1
#define INTERFACE_CUA        2

/* defines for SET TRAILING */

#define TRAILING_OFF       0
#define TRAILING_ON        1
#define TRAILING_SINGLE    2
#define TRAILING_EMPTY     3
#define TRAILING_REMOVE    4

/* defines for behaviour of commands when a CUA block is current */

#define CUA_NONE           0
#define CUA_DELETE_BLOCK   1
#define CUA_RESET_BLOCK    2

/* defines for behaviour of commands with a THIGHLIGHT area */

#define THIGHLIGHT_NONE        0
#define THIGHLIGHT_RESET_ALL   1
#define THIGHLIGHT_RESET_FOCUS 2


/* defines for BOUNDMARK */

#define BOUNDMARK_OFF       0
#define BOUNDMARK_ZONE      1
#define BOUNDMARK_TRUNC     2
#define BOUNDMARK_MARGINS   3
#define BOUNDMARK_TABS      4
#define BOUNDMARK_VERIFY    5

/* following defines for strstrip() function */

#define STRIP_NONE         0
#define STRIP_LEADING      1
#define STRIP_TRAILING     2
#define STRIP_INTERNAL     4
#define STRIP_BOTH         (STRIP_LEADING|STRIP_TRAILING)
#define STRIP_ALL          (STRIP_LEADING|STRIP_TRAILING|STRIP_INTERNAL)

/* following defines for button types for DIALOG command */

#define BUTTON_OK          0
#define BUTTON_OKCANCEL    1
#define BUTTON_YESNO       2
#define BUTTON_YESNOCANCEL 3

/* following defines for icon types for DIALOG command */

#define ICON_NONE          0
#define ICON_EXCLAMATION   1
#define ICON_INFORMATION   2
#define ICON_QUESTION      3
#define ICON_STOP          4

/* following defines for SET READONLY */

#define READONLY_OFF       0
#define READONLY_ON        1
#define READONLY_FORCE     2

/* following defines for get_key_definition() format */

#define THE_KEY_DEFINE_SHOW   1
#define THE_KEY_DEFINE_DEFINE 2
#define THE_KEY_DEFINE_RAW    3
#define THE_KEY_DEFINE_QUERY  4

/* following defines for file_exists() */

#define THE_FILE_UNKNOWN       0
#define THE_FILE_EXISTS        1
#define THE_FILE_NAME_TOO_LONG 2

/* Following are used for determining the button action of the mouse */

#define BUTTON_RELEASED 0
#define BUTTON_PRESSED 1
#define BUTTON_CLICKED 2
#define BUTTON_DOUBLE_CLICKED 3
#define BUTTON_MOVED 5

#define HIT_ANY_KEY "Hit any key to continue..."

/* useful macros */

#define TOF(line)            ((line == 0L) ? TRUE : FALSE)
#define BOF(line)            ((line == CURRENT_FILE->number_lines+1L) ? TRUE : FALSE)
#define VIEW_TOF(view,line)  ((line == 0L) ? TRUE : FALSE)
#define VIEW_BOF(view,line)  ((line == view->file_for_view->number_lines+1L) ? TRUE : FALSE)
#define CURRENT_TOF          ((CURRENT_VIEW->current_line == 0L) ? TRUE : FALSE)
#define CURRENT_BOF          ((CURRENT_VIEW->current_line == CURRENT_FILE->number_lines+1L) ? TRUE : FALSE)
#define FOCUS_TOF            ((CURRENT_VIEW->focus_line == 0L) ? TRUE : FALSE)
#define FOCUS_BOF            ((CURRENT_VIEW->focus_line == CURRENT_FILE->number_lines+1L) ? TRUE : FALSE)
#define VIEW_FOCUS_TOF(view) ((view->focus_line == 0L) ? TRUE : FALSE)
#define VIEW_FOCUS_BOF(view) ((view->focus_line == view->file_for_view->number_lines+1L) ? TRUE : FALSE)
#define IN_VIEW(view,line)   ((line >= (view->current_line - (long)view->current_row)) && (line <= (view->current_line + ((long)CURRENT_SCREEN.rows[WINDOW_FILEAREA] - (long)view->current_row))))
#define IN_SCOPE(view,line)  ((line)->select >= (view)->display_low && (line)->select <= (view)->display_high)

/* system specific redefines */

#define ISREADONLY(x)  (the_readonly || (READONLYx==READONLY_FORCE) || (READONLYx==READONLY_ON && x->disposition == FILE_READONLY) || !(x->readonly==READONLY_OFF))
#define STATUSLINEON() ((STATUSLINEx == 'T') || (STATUSLINEx == 'B'))

extern VIEW_DETAILS *vd_current;
extern uchar current_screen;
extern SCREEN_DETAILS screen[MAX_SCREENS];

#ifdef MAIN
long max_line_length = MAX_LENGTH_OF_LINE;
#else
extern long max_line_length;
#endif

/* structure for passing queryable values parameters */

struct query_values {
  uchar *value;                 /* value of item */
  long len;                     /* length of string representation of value */
};
typedef struct query_values VALUE;

/* structure for function key redefinition */

struct defines {
  struct defines *prev;
  struct defines *next;
  int def_funkey;
  short def_command;
  uchar *def_params;
  uchar *synonym;
  uchar *pcode;
  int pcode_len;
  uchar linend;
};
typedef struct defines DEFINE;

/* structure for window areas */
struct window_areas {
  uchar *area;                  /* window area - used for COLOUR command */
  short area_min_len;           /* min abbrev for area name */
  short area_window;            /* window where area is */
  bool actual_window;           /* TRUE if area is a window */
};
typedef struct window_areas AREAS;

typedef short (ExtractFunction) (short, short, uchar *, uchar, long, uchar *, long);

/* structure for query and implied extract */

struct query_item {
  uchar *name;                  /* name of item */
  short name_length;            /* length of function name */
  short min_len;                /* minimum length of abbreviation */
  short item_number;            /* unique number for item */
  short number_values;          /* number of values returned (from query/extract) */
  short item_values;            /* number of values this item can have (implied extract) (excludes 0th value ) */
  short level;                  /* level of item; global, file, view */
  uchar query;                  /* valid query response ? */
  ExtractFunction *ext_func;    /* function that generates extract details */
};
typedef struct query_item QUERY_ITEM;

/* maximum number of variables that can be returned via EXTRACT */
/* this MUST be max of ECOLOUR_MAX and ATTR_MAX */

#define MAX_VARIABLES_RETURNED               ECOLOUR_MAX

/* structure for list of TLD headers */

typedef struct {
  char *the_header_name;
  int the_header_name_len;
  long the_header;
} the_header_mapping;

#include "vars.h"

#define THE_SEARCH_SEMANTICS                   TRUE
#define THE_NOT_SEARCH_SEMANTICS               FALSE

#define MAX_WIDTH_NUM  2000000000L

