// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Definitions of various CUA behaviours
 * CUA_NONE:
 * - no affect. ie no automatic processing is done, but the
 *   individual command can handle CUA behaviour itself. eg
 *   CURSOR command would need to handle CUA behaviour.
 *
 * CUA_DELETE_BLOCK
 * - save first line/column position (not anchor position)
 * - delete the marked CUA block (ignores other blocks )
 * - move cursor to first line/column (not anchor position)
 *
 * CUA_RESET_BLOCK
 * - reset CUA block
 */

/*
 * Definitions of various THIGHLIGHT behaviours
 * THIGHLIGHT_NONE:
 * - no affect. ie no automatic resetting of the THIGHLIGHT
 *   area.
 *
 * THIGHLIGHT_RESET_ALL
 * - reset the THIGHLIGHT area in all circumstances
 *
 * THIGHLIGHT_RESET_FOCUS
 * - reset CUA THIGHLIGHT area if this command executed on
 *   the focus line and this line contains the THIGHLIGHT area
 */

struct commands {
   char *text;
   unsigned short min_len;
   int funkey;
   short (*function)(char*);
   bool valid_batch_command;
   bool set_command;
   bool sos_command;
   bool valid_in_readonly;
   bool edit_command;
   bool strip_param;
   bool valid_readv;
   bool valid_input_full;
   bool valid_for_reprofile;
   short cua_behaviour;
   short thighlight_behaviour;
   char *params;
};
typedef struct commands COMMAND;

#include "proto.h"

/*---------------------- *** IMPORTANT *** ----------------------------
 * Make sure that there is no chance two or more commands can be matched based on number of significant characters.
 * To allow a command to be referenced from the command line, it must have a min_len > 0.
 * When the same command is in the table more than once (because multiple keys are defined), only one of the definitions can have a min_len > 0.
 */

/*                                                      Batch/ Set / SOS / RO   /edit/strip     /readv/input/reprof/CUA             /THIGHLIGHT  */
COMMAND command[] = {

 {"cursor",      0,  KEY_RIGHT,       Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"right"},  /* comm1.c */
 {"cursor",      0,  KEY_LEFT,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"left"},  /* comm1.c */
 {"cursor",      0,  KEY_DOWN,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"down"},  /* comm1.c */
 {"cursor",      0,  KEY_UP,          Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"up"},  /* comm1.c */
 {"add",         1,  (-1),            Add,              TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""}, /* comm1.c */
 {"alert",       5,  (-1),            Alert,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"all",         3,  (-1),            All,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"arbchar",     3,  (-1),            Arbchar,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"alt",         3,  (-1),            Alt,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"autocolour", 10,  (-1),            Autocolour,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"autocolor",   9,  (-1),            Autocolour,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"autosave",    2,  (-1),            Autosave,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"autoscroll",  6,  (-1),            Autoscroll,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"backward",    2,  KEY_PPAGE,       Backward,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""}, /* comm1.c */
 {"backward",    2,  KEY_PrevScreen,  Backward,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""}, /* comm1.c */
 {"backup",      4,  (-1),            Backup,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"beep",        4,  (-1),            BeepSound,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"bottom",      1,  (-1),            Bottom,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"boundmark",   9,  (-1),            Boundmark,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"cancel",      3,  (-1),            Cancel,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"cappend",     2,  (-1),            Cappend,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"case",        4,  (-1),            Case,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"ccancel",     2,  (-1),            Ccancel,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"cdelete",     2,  (-1),            Cdelete,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* comm1.c */
 {"cfirst",      2,  (-1),            Cfirst,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"change",      1,  (-1),            Change,           TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm1.c */
 {"cinsert",     2,  (-1),            Cinsert,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* comm1.c */
 {"clast",       3,  (-1),            Clast,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"clipboard",   9,  (-1),            THEClipboard,     TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"clearerrorkey", 6,(-1),            Clearerrorkey,    TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"clearscreen", 6,  (-1),            Clearscreen,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"clocate",     2,  (-1),            Clocate,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"clock",       5,  (-1),            Clock,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"cmatch",      6,  KEY_F(7),        Cmatch,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"cmdarrows",   4,  (-1),            Cmdarrows,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"cmdline",     3,  (-1),            Cmdline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"cmsg",        4,  (-1),            Cmsg,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"command",     7,  (-1),            THECommand,       TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"colour",      6,  (-1),            Colour,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"color",       5,  (-1),            Colour,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"colouring",   9,  (-1),            Colouring,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"coloring",    8,  (-1),            Colouring,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"compat",      6,  (-1),            Compat,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"compress",    4,  (-1),            Compress,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm1.c */
 {"copy",        4,  (-1),            Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm1.c */
 {"controlchar" ,8,  KEY_F(11),       ControlChar,      FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm1.c */
 {"copy",        0,  KEY_C_c,         Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,"block reset"},  /* comm1.c */
 {"copy",        0,  KEY_C_k,         Copy,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,"block"},  /* comm1.c */
 {"cursor",      3,  KEY_HOME,        Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"home save"},  /* comm1.c */
 {"cursor",      0,  KEY_F16,         Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"home save"},  /* comm1.c */
 {"cursor",      0,  KEY_F(12),       Cursor,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"prefix"},  /* comm1.c */
 {"ctlchar",     3,  (-1),            Ctlchar,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"coverlay",    3,  (-1),            Coverlay,         TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm1.c */
 {"creplace",    2,  (-1),            Creplace,         TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm1.c */
 {"curline",     4,  (-1),            Curline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"cursorstay",  8,  (-1),            CursorStay,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"define",      3,  (-1),            Define,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"delete",      3,  (-1),            DeleteLine,       TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,""}, /* comm2.c */
 {"delete",      0,  KEY_C_g,         DeleteLine,       FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,"block"}, /* comm2.c */
 {"dialog",      6,  (-1),            Dialog,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"directory",   3,  (-1),            Directory,        FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"ls",          2,  (-1),            Directory,        FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"defsort",     7,  (-1),            Defsort,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"display",     4,  (-1),            Display,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"down",        1,  (-1),            THENext,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""}, /* comm3.c */
 {"duplicate",   3,  (-1),            Duplicate,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"duplicate",   0,  KEY_C_d,         Duplicate,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"1 BLOCK"},  /* comm2.c */
 {"enter",       0,  KEY_C_m,         Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"enter",       0,  KEY_ENTER,       Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"enter",       5,  KEY_RETURN,      Enter,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"ecolor",      6,  (-1),            Ecolour,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"ecolour",     7,  (-1),            Ecolour,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"editv",       5,  (-1),            THEEditv,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"emsg",        4,  (-1),            Emsg,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"eolout",      4,  (-1),            Eolout,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"equivchar",   6,  (-1),            Equivchar,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"errorformat", 6,  (-1),            Errorformat,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"erroroutput", 8,  (-1),            Erroroutput,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"etmode",      6,  (-1),            Etmode,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"expand",      3,  (-1),            Expand,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"extract",     3,  (-1),            Extract,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"ffile",       2,  (-1),            Ffile,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"file",        4,  KEY_F(3),        File,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"file",        0,  KEY_PF3,         File,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"fillbox",     4,  KEY_C_f,         Fillbox,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"filetabs",    8,  (-1),            THEFiletabs,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"find",        1,  (-1),            Find,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"findup",      5,  (-1),            Findup,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"fup",         2,  (-1),            Findup,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"fext",        2,  (-1),            Fext,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"filename",    5,  (-1),            Filename,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"fmode",       2,  (-1),            Fmode,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"fname",       2,  (-1),            Fname,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"fpath",       2,  (-1),            Fpath,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"ftype",       2,  (-1),            Fext,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"forward",     2,  KEY_NPAGE,       Forward,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""}, /* comm2.c */
 {"forward",     2,  KEY_NextScreen,  Forward,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""}, /* comm2.c */
 {"fullfname",   6,  (-1),            Fullfname,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"get",         3,  (-1),            Get,              TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm2.c */
 {"header",      4,  (-1),            THEHeader,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"help",        4,  KEY_F(1),        Help,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"help",        0,  KEY_PF1,         Help,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"hex",         3,  (-1),            Hex,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"hexdisplay",  7,  (-1),            Hexdisplay,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"hexshow",     4,  (-1),            Hexshow,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"highlight",   4,  (-1),            Highlight,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"hit",         3,  (-1),            Hit,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"idline",      2,  (-1),            Idline,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"impmacro",    5,  (-1),            Impmacro,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"impos",       5,  (-1),            Impos,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"impcmscp",    3,  (-1),            Impos,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"input",       1,  (-1),            Input,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm2.c */
 {"inputmode",   6,  (-1),            Inputmode,        TRUE, TRUE, FALSE,FALSE,FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"interface",   3,  (-1),            THEInterface,     TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"insertmode",  7,  KEY_IC,          Insertmode,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"toggle"},  /* commset1.c */
 {"locate",      1,  KEY_F(11),       Locate,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"left",        2,  (-1),            Left,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"lineflag",    8,  (-1),            Lineflag,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"linend",      5,  (-1),            Linend,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"lastop",      6,  (-1),            Lastop,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"lowercase",   3,  (-1),            Lowercase,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"macro",       5,  (-1),            Macro,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"macro",       5,  (-1),            SetMacro,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"macroext",    6,  (-1),            Macroext,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"macropath",   6,  (-1),            Macropath,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"margins",     3,  (-1),            Margins,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"mark",        4,  (-1),            Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,""},  /* comm3.c */
 {"modify",      3,  (-1),            Modify,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"mouse",       5,  (-1),            Mouse,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 { "",           0,  KEY_MOUSE,       THEMouse,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE, CUA_NONE         ,THIGHLIGHT_NONE         ,""},    /* comm3.c */
 {"mark",        0,  KEY_C_l,         Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,"line"},  /* comm3.c */
 {"mark",        0,  KEY_C_b,         Mark,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_ALL    ,"box"},  /* comm3.c */
 {"move",        4,  KEY_C_v,         THEMove,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,"block reset"},  /* comm3.c */
 {"nextwindow",  5,  KEY_F(2),        Nextwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"nextwindow",  5,  KEY_PF2,         Nextwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"overlaybox",  8,  KEY_C_o,         Overlaybox,       FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm3.c */
 {"nfind",       2,  (-1),            Nfind,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm3.c */
 {"nfindup",     6,  (-1),            Nfindup,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm3.c */
 {"nfup",        3,  (-1),            Nfindup,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm3.c */
 {"msg",         3,  (-1),            Msg,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"msgline",     4,  (-1),            Msgline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"msgmode",     4,  (-1),            Msgmode,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"newlines",    4,  (-1),            Newlines,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"next",        1,  (-1),            THENext,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""}, /* comm3.c */
 {"nomsg",       5,  (-1),            Nomsg,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"nondisp",     4,  (-1),            Nondisp,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset1.c */
 {"nop",         3,  (-1),            Nop,              FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"number",      3,  (-1),            Number,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"osnowait",    3,  (-1),            Osnowait,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"osquiet",     3,  (-1),            Osquiet,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"osredir",     3,  (-1),            Osredir,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"os",          2,  (-1),            Os,               FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"dosnowait",   4,  (-1),            Osnowait,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"dosquiet",    4,  (-1),            Osquiet,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"dos",         3,  (-1),            Os,               FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"!",           1,  (-1),            Os,               FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm3.c */
 {"pagewrap",    8,  (-1),            Pagewrap,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"parser",      6,  (-1),            Parser,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"pending",     4,  (-1),            Pending,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"point",       1,  (-1),            Point,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"popup",       5,  (-1),            Popup,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"point",       0,  KEY_F(31),       Point,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,".a"},/* commset2.c */
 {"locate",      0,  KEY_F(32),       Locate,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,".a"},/* commset2.c */
 {"position",    3,  (-1),            Position,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"prefix",      3,  (-1),            Prefix,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"preserve",    4,  (-1),            Preserve,         TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"prevwindow",  5,  (-1),            Prevwindow,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"printer",     7,  (-1),            THEPrinter,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"print",       2,  (-1),            Print,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"pscreen",     4,  (-1),            Pscreen,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"put",         3,  (-1),            Put,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"putd",        4,  (-1),            Putd,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm4.c */
 {"qquit",       2,  KEY_F(13),       Qquit,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"quit",        4,  (-1),            Quit,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"query",       1,  (-1),            Query,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"record",      6,  (-1),            THERecord,        FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"recover",     3,  KEY_F(8),        Recover,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm4.c */
 {"readv",       5,  (-1),            Readv,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"readonly",    8,  (-1),            THEReadonly,      TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"redit",       5,  (-1),            Redit,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"redraw",      6,  KEY_C_r,         Redraw,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"refresh",     7,  (-1),            THERefresh,       FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"repeat",      4,  (-1),            Repeat,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"replace",     1,  (-1),            Replace,          TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* comm4.c */
 {"reprofile",   6,  (-1),            Reprofile,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"reserved",    5,  (-1),            Reserved,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"reset",       3,  (-1),            Reset,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"reset",       0,  KEY_C_u,         Reset,            FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"block"},  /* comm4.c */
 {"restore",     3,  (-1),            Restore,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"rexxhalt",    8,  (-1),            Rexxhalt,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"rexxoutput",  7,  (-1),            Rexxoutput,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"rgtleft",     7,  (-1),            Rgtleft,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"rexx",        4,  (-1),            THERexx,          TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"right",       2,  (-1),            Right,            TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"=",           1,  (-1),            Reexecute,        TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"?",           1,  KEY_F(6),        Retrieve,         FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"save",        4,  (-1),            Save,             TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"scale",       4,  (-1),            Scale,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"schange",     3,  (-1),            Schange,          FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm4.c */
 {"screen",      3,  (-1),            THEScreen,        FALSE,TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"scope",       5,  (-1),            Scope,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"search",      3,  (-1),            THESearch,        TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"select",      3,  (-1),            Select,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"set",         3,  (-1),            Set,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"shadow",      4,  (-1),            Shadow,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"shift",       2,  (-1),            Shift,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm4.c */
 {"showkey",     4,  (-1),            ShowKey,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"slk",         3,  (-1),            Slk,              TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"sort",        4,  (-1),            Sort,             TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm4.c */
 {"sos",         3,  (-1),            Sos,              FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"span",        4,  (-1),            Span,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"spill",       5,  (-1),            Spill,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"bottomedge",  7,  (-1),            Sos_bottomedge,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"current",     4,  (-1),            Sos_current,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"endchar",     4,  (-1),            Sos_endchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"execute",     2,  (-1),            Sos_execute,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* commsos.c */
 {"firstchar",   7,  (-1),            Sos_firstchar,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"firstcol",    7,  (-1),            Sos_firstcol,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"instab",      6,  (-1),            Sos_instab,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"lastcol",     6,  (-1),            Sos_lastcol,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"leftedge",    5,  (-1),            Sos_leftedge,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"marginl",     7,  (-1),            Sos_marginl,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"marginr",     7,  (-1),            Sos_marginr,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"pastecmdline",6,  KEY_C_p,         Sos_pastecmdline, FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"parindent",   6,  (-1),            Sos_parindent,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"prefix",      3,  (-1),            Sos_prefix,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"qcmnd",       2,  (-1),            Sos_qcmnd,        FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"rightedge",   6,  (-1),            Sos_rightedge,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"tabfieldb",   9,  (-1),            Sos_tabfieldb,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"tabfieldf",   8,  (-1),            Sos_tabfieldf,    FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"topedge",     4,  (-1),            Sos_topedge,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"blockstart",  6,  (-1),            Sos_blockstart,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"blockend",    6,  (-1),            Sos_blockend,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"cursoradj",   7,  KEY_C_a,         Sos_cursoradj,    FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"cursorshift",11,  (-1),            Sos_cursorshift,  FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"cuadelback",  8,  (-1),            Sos_cuadelback,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"cuadelchar",  7,  (-1),            Sos_cuadelchar,   FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"addline",     3,  KEY_F(4),        Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* commsos.c */
 {"addline",     3,  KEY_PF4,         Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* commsos.c */
 {"tabb",        4,  (-1),            Sos_tabb,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"delback",     4,  KEY_C_h,         Sos_delback,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"delback",     4,  KEY_BACKSPACE,   Sos_delback,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"delchar",     4,  KEY_Remove,      Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"delchar",     4,  KEY_DELETE,      Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"delchar",     4,  KEY_DC,          Sos_delchar,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"delend",      4,  KEY_C_e,         Sos_delend,       FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"delline",     4,  KEY_F(9),        Sos_delline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* commsos.c */
 {"delword",     4,  KEY_C_w,         Sos_delword,      FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"doprefix",    5,  KEY_C_p,         Sos_doprefix,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"edit",        4,  KEY_C_x,         Sos_edit,         FALSE,FALSE,TRUE, TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"startendchar",9,  KEY_F(20),       Sos_startendchar, FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"makecurr",    8,  KEY_F(5),        Sos_makecurr,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"tabf",        3,  KEY_C_i,         Sos_tabf,         FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"tabwordb",    8,  KEY_F(18),       Sos_tabwordb,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"tabwordf",    7,  KEY_F(19),       Sos_tabwordf,     FALSE,FALSE,TRUE, TRUE, FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commsos.c */
 {"undo",        4,  KEY_C_q,         Sos_undo,         FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,TRUE, FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_FOCUS  ,""},  /* commsos.c */
 {"split",       2,  KEY_C_s,         Split,            FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,"aligned cursor"},  /* comm4.c */
 {"join",        1,  KEY_C_j,         Join,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,"aligned cursor"},  /* comm2.c */
 {"spltjoin",    8,  KEY_F(10),       Spltjoin,         FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm4.c */
 {"lineadd",     5,  (-1),            Sos_addline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* commsos.c */
 {"linedel",     5,  (-1),            Sos_delline,      FALSE,FALSE,TRUE, FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* commsos.c */
 {"ssave",       2,  (-1),            Ssave,            TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"status",      4,  (-1),            Status,           TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"statusline",  7,  (-1),            Statusline,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"statopt",     7,  (-1),            Statopt,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"stay",        4,  (-1),            Stay,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"synonym",     3,  (-1),            Synonym,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"suspend",     4,  KEY_C_z,         Suspend,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm4.c */
 {"tabpre",      0,  (-1),            Tabpre,           FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"tabfile",     7,  (-1),            Tabfile,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"tabline",     4,  (-1),            Tabline,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"tabkey",      4,  (-1),            Tabkey,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"tabs",        4,  (-1),            Tabs,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"tabsin",      5,  (-1),            Tabsin,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"tabsout",     5,  (-1),            Tabsout,          TRUE, TRUE, FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"tag",         3,  (-1),            Tag,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"targetsave", 10,  (-1),            Targetsave,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"text",        4,  (-1),            Text,             FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_NONE,TRUE, FALSE,TRUE ,CUA_DELETE_BLOCK ,THIGHLIGHT_RESET_FOCUS  ,""},  /* comm5.c */
 {"thighlight",  5,  (-1),            THighlight,       TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"timecheck",   9,  (-1),            Timecheck,        TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"toascii",     7,  (-1),            Toascii,          FALSE,FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm5.c */
 {"tofeof",      6,  (-1),            Tofeof,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"top",         3,  (-1),            Top,              TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"trailing",    8,  (-1),            Trailing,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"trunc",       5,  (-1),            Trunc,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"typeahead",   5,  (-1),            THETypeahead,     TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"undoing",     7,  (-1),            Undoing,          TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"up",          1,  (-1),            Up,               TRUE, FALSE,FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"uppercase",   3,  (-1),            Uppercase,        TRUE, FALSE,FALSE,FALSE,FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_RESET_ALL    ,""},  /* comm5.c */
 {"verify",      1,  (-1),            Verify,           TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"width",       5,  (-1),            Width,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"word",        4,  (-1),            Word,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"wrap",        2,  (-1),            Wrap,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"wordwrap",    5,  (-1),            Wordwrap,         TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"edit",        1,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"xedit",       1,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"kedit",       1,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"the",         3,  (-1),            Xedit,            TRUE, FALSE,FALSE,TRUE, TRUE, STRIP_BOTH,FALSE,FALSE,FALSE,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* comm5.c */
 {"untaa",       5,  (-1),            Untaa,            TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {"zone",        1,  (-1),            Zone,             TRUE, TRUE, FALSE,TRUE, FALSE,STRIP_BOTH,FALSE,FALSE,TRUE ,CUA_RESET_BLOCK  ,THIGHLIGHT_NONE         ,""},  /* commset2.c */
 {NULL,                      0,  (-1),            NULL,             FALSE,FALSE,FALSE,FALSE,FALSE,STRIP_NONE,FALSE,FALSE,TRUE ,CUA_NONE         ,THIGHLIGHT_NONE         ,""}
};
