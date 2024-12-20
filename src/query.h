// SPDX-FileCopyrightText: 2002 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * The order of these items determine the order they appear as a result of the STATUS command,
 * so they should be in alphabetical order.
 */

/*
 * The following are item number defines for EXTRACT/QUERY/STATUS.
 */
#define ITEM_ALT                              0
#define ITEM_ARBCHAR                          1
#define ITEM_AUTOCOLOR                        2
#define ITEM_AUTOCOLOUR                       3
#define ITEM_AUTOSAVE                         4
#define ITEM_AUTOSCROLL                       5
#define ITEM_BACKUP                           6
#define ITEM_BEEP                             7
#define ITEM_BLOCK                            8
#define ITEM_CASE                             9
#define ITEM_CLEARERRORKEY                   10
#define ITEM_CLEARSCREEN                     11
#define ITEM_CLOCK                           12
#define ITEM_CMDARROWS                       13
#define ITEM_CMDLINE                         14
#define ITEM_COLOR                           15
#define ITEM_COLORING                        16
#define ITEM_COLOUR                          17
#define ITEM_COLOURING                       18
#define ITEM_COLUMN                          19
#define ITEM_COMPAT                          20
#define ITEM_CTLCHAR                         21
#define ITEM_CURLINE                         22
#define ITEM_CURSOR                          23
#define ITEM_CURSORSTAY                      24
#define ITEM_DEFINE                          25
#define ITEM_DEFSORT                         26
#define ITEM_DIRFILEID                       27
#define ITEM_DISPLAY                         28
#define ITEM_ECOLOR                          29
#define ITEM_ECOLOUR                         30
#define ITEM_EFILEID                         31
#define ITEM_EOF                             32
#define ITEM_EOLOUT                          33
#define ITEM_EQUIVCHAR                       34
#define ITEM_ERRORFORMAT                     35
#define ITEM_ERROROUTPUT                     36
#define ITEM_ETMODE                          37
#define ITEM_FDISPLAY                        38
#define ITEM_FEXT                            39
#define ITEM_FIELD                           40
#define ITEM_FIELDWORD                       41
#define ITEM_FILENAME                        42
#define ITEM_FILESTATUS                      43
#define ITEM_FILETABS                        44
#define ITEM_FNAME                           45
#define ITEM_FPATH                           46
#define ITEM_FTYPE                           47
#define ITEM_FULLFNAME                       48
#define ITEM_GETENV                          49
#define ITEM_HEADER                          50
#define ITEM_HEX                             51
#define ITEM_HEXDISPLAY                      52
#define ITEM_HEXSHOW                         53
#define ITEM_HIGHLIGHT                       54
#define ITEM_IDLINE                          55
#define ITEM_IMPMACRO                        56
#define ITEM_IMPOS                           57
#define ITEM_INPUTMODE                       58
#define ITEM_INSERTMODE                      59
#define ITEM_INTERFACE                       60
#define ITEM_LASTKEY                         61
#define ITEM_LASTMSG                         62
#define ITEM_LASTOP                          63
#define ITEM_LASTRC                          64
#define ITEM_LENGTH                          65
#define ITEM_LINE                            66
#define ITEM_LINEFLAG                        67
#define ITEM_LINEND                          68
#define ITEM_LSCREEN                         69
#define ITEM_MACRO                           70
#define ITEM_MACROEXT                        71
#define ITEM_MACROPATH                       72
#define ITEM_MARGINS                         73
#define ITEM_MONITOR                         74
#define ITEM_MOUSE                           75
#define ITEM_MOUSECLICK                      76
#define ITEM_MSGLINE                         77
#define ITEM_MSGMODE                         78
#define ITEM_NBFILE                          79
#define ITEM_NBSCOPE                         80
#define ITEM_NEWLINES                        81
#define ITEM_NONDISP                         82
#define ITEM_NUMBER                          83
#define ITEM_PAGEWRAP                        84
#define ITEM_PARSER                          85
#define ITEM_PENDING                         86
#define ITEM_POINT                           87
#define ITEM_POSITION                        88
#define ITEM_PREFIX                          89
#define ITEM_PRINTER                         90
#define ITEM_PROFILE                         91
#define ITEM_PSCREEN                         92
#define ITEM_READONLY                        93
#define ITEM_READV                           94
#define ITEM_REPROFILE                       95
#define ITEM_RESERVED                        96
#define ITEM_REXXHALT                        97
#define ITEM_REXXOUTPUT                      98
#define ITEM_RING                            99
#define ITEM_SCALE                          100
#define ITEM_SCOPE                          101
#define ITEM_SCREEN                         102
#define ITEM_SELECT                         103
#define ITEM_SHADOW                         104
#define ITEM_SHOWKEY                        105
#define ITEM_SIZE                           106
#define ITEM_SLK                            107
#define ITEM_STATOPT                        108
#define ITEM_STATUSLINE                     109
#define ITEM_STAY                           110
#define ITEM_SYNELEM                        111
#define ITEM_SYNONYM                        112
#define ITEM_TABKEY                         113
#define ITEM_TABLINE                        114
#define ITEM_TABS                           115
#define ITEM_TABSIN                         116
#define ITEM_TABSOUT                        117
#define ITEM_TARGETSAVE                     118
#define ITEM_TERMINAL                       119
#define ITEM_THIGHLIGHT                     120
#define ITEM_TIMECHECK                      121
#define ITEM_TOF                            122
#define ITEM_TOFEOF                         123
#define ITEM_TRAILING                       124
#define ITEM_TYPEAHEAD                      125
#define ITEM_UI                             126
#define ITEM_UNDOING                        127
#define ITEM_UNTAA                          128
#define ITEM_UTF8                           129
#define ITEM_VERIFY                         130
#define ITEM_VERSHIFT                       131
#define ITEM_VERSION                        132
#define ITEM_WIDTH                          133
#define ITEM_WORD                           134
#define ITEM_WORDWRAP                       135
#define ITEM_WRAP                           136
#define ITEM_ZONE                           137

/*
 * The following are item number defines for the boolean functions.
 */
#define ITEM_AFTER_FUNCTION                   0
#define ITEM_ALT_FUNCTION                     1
#define ITEM_ALTKEY_FUNCTION                  2
#define ITEM_BATCH_FUNCTION                   3
#define ITEM_BEFORE_FUNCTION                  4
#define ITEM_BLANK_FUNCTION                   5
#define ITEM_BLOCK_FUNCTION                   6
#define ITEM_BOTTOMEDGE_FUNCTION              7
#define ITEM_COMMAND_FUNCTION                 8
#define ITEM_CTRL_FUNCTION                    9
#define ITEM_CURRENT_FUNCTION                10
#define ITEM_DIR_FUNCTION                    11
#define ITEM_END_FUNCTION                    12
#define ITEM_EOF_FUNCTION                    13
#define ITEM_FIRST_FUNCTION                  14
#define ITEM_FOCUSEOF_FUNCTION               15
#define ITEM_FOCUSTOF_FUNCTION               16
#define ITEM_INBLOCK_FUNCTION                17
#define ITEM_INCOMMAND_FUNCTION              18
#define ITEM_INITIAL_FUNCTION                19
#define ITEM_INPREFIX_FUNCTION               20
#define ITEM_INSERTMODE_FUNCTION             21
#define ITEM_LEFTEDGE_FUNCTION               22
#define ITEM_MODIFIABLE_FUNCTION             23
#define ITEM_RIGHTEDGE_FUNCTION              24
#define ITEM_RUN_OS_FUNCTION                 25
#define ITEM_SHADOW_FUNCTION                 26
#define ITEM_SHIFT_FUNCTION                  27
#define ITEM_SPACECHAR_FUNCTION              28
#define ITEM_TOF_FUNCTION                    29
#define ITEM_TOPEDGE_FUNCTION                30
#define ITEM_VALID_TARGET_FUNCTION           31
#define ITEM_VERONE_FUNCTION                 32

/* query.c */

short extract_point_settings(short, uchar *);
short extract_prefix_settings(short, uchar *, uchar);
short extract_colour_settings(short, uchar *, uchar, uchar *, bool, bool);
short extract_autocolour_settings(short, uchar *, uchar, uchar *, bool);

void get_etmode(uchar *, uchar *);

short set_boolean_value(bool flag, short num);
short set_on_off_value(bool flag, short num);
void set_key_values(int key, bool mouse_key);

THE_PPC *in_range(THE_PPC * found_ppc, THE_PPC * curr_ppc, long first_in_range, long last_in_range);


_declare_ uchar query_num0[50];  // DO NOT USE THIS FOR DATA !!
_declare_ uchar query_num1[50];
_declare_ uchar query_num2[50];
_declare_ uchar query_num3[50];
_declare_ uchar query_num4[50];
_declare_ uchar query_num5[50];
_declare_ uchar query_num6[50];
_declare_ uchar query_num7[50];
_declare_ uchar query_num8[50];

_declare_ uchar query_rsrvd[MAX_FILE_NAME+100];

#ifdef _declare_
_declare_ uchar** block_name;
#else
uchar* block_name[] =
{
  (uchar *)"",
  (uchar *)"LINE",
  (uchar *)"BOX",
  (uchar *)"STREAM",
  (uchar *)"COLUMN",
  (uchar *)"WORD",
  (uchar *)"CUA"
};
#endif

