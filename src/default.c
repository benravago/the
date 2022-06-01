// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Default settings and profile processing
 */

#include "the.h"
#include "proto.h"

/*--------------------------- global data -----------------------------*/

bool BEEPx;
bool CAPREXXOUTx;
bool ERROROUTPUTx;
line_t CAPREXXMAXx;
bool CLEARSCREENx;
int CLEARERRORKEYx;
bool CLOCKx;
char_t CMDARROWSTABCMDx;
char_t EOLx;
bool HEXDISPLAYx;
bool INSERTMODEx;
bool LINEND_STATUSx;            /* not really global, but used in command_line() */
char_t LINEND_VALUEx;         /* not really global, but used in command_line() */
char_t NONDISPx;
char_t PREFIXx;               /* not really global but used in set_screen_defaults() */
bool REPROFILEx;
bool DONT_CALL_DEFSORTx = FALSE;
char_t TABI_ONx;
char_t TABI_Nx;
char_t EQUIVCHARx;            /* equivalence character */
char_t EQUIVCHARstr[2];       /* equivalence character as a string */
row_t STATUSLINEx = 'B';
bool FILETABSx = FALSE;         /* file tabs are off by default */
bool TYPEAHEADx;
bool scroll_cursor_stay;        /* determine how page scrolling is handled */
bool MOUSEx;                    /* mouse support */
bool SLKx = FALSE;              /* soft label key support */
bool SBx = FALSE;               /* scrollbar support */
bool UNTAAx = FALSE;            /* unsigned numerical targets are absolute */
int READONLYx = READONLY_OFF;   /* SET READONLY OFF */
bool PAGEWRAPx = TRUE;          /* wrap pgup/pgdn at extremities of file */
int DEFSORTx = DIRSORT_NAME;    /* sort on file attr */
int DIRORDERx = DIRSORT_ASC;    /* sort ascending */
int popup_escape_key = -1;
int popup_escape_keys[MAXIMUM_POPUP_KEYS];
PARSER_DETAILS *first_parser = NULL, *last_parser = NULL;
PARSER_MAPPING *first_parser_mapping = NULL, *last_parser_mapping = NULL;
bool CTLCHARx = FALSE;
char_t ctlchar_escape = 0;
COLOUR_ATTR ctlchar_attr[MAX_CTLCHARS]; /* colour attributes for ctlchar */
char_t ctlchar_char[MAX_CTLCHARS];    /* escape chars for ctlchar */
bool ctlchar_protect[MAX_CTLCHARS];     /* protected ctlchar */
char_t INTERFACEx = INTERFACE_CLASSIC;        /* behavioural interface */
int TARGETSAVEx = TARGET_ALL;
bool save_for_repeat = 1;
char_t BACKUP_SUFFIXx[101];
int COMMANDCALLSx = 0;
int FUNCTIONCALLSx = 0;
bool inDIALOG = FALSE;          /* indicates if we are running a dialog or alert */
char_t ERRORFORMATx = 'N';    /* normal error text format */
int last_command_index = -1;

/*--------------------------- default parsers -------------------------*/

typedef struct {
  char *filename;
  char *name;
  char *contents;
} DEFAULT_PARSERS;

DEFAULT_PARSERS default_parsers[] = {

  { "*REXX.TLD", "REXX",
    "* REXX\n:case\nignore\n:option\nrexx\nfunction ( noblank\n:number\nrexx\n:identifier\n[a-zA-Z!?_@#$.] [a-zA-Z0-9.!?_@#$]\n" ":string\nsingle\ndouble\n:comment\npaired /* */ nest\nline -- any\n"
    ":label\ndelimiter : firstnonblank\n:match\n( )\ndo,select end when,otherwise\n:header\nline #! column 1\n" ":keyword\naddress type 5\narg type 7\nby type 2\ncall type 5\ncaseless type 2\ndigits type 2\ndo type 5\ndrop type 1\n"
    "else type 1\nend type 1\nengineering type 2\nexit type 1\nexpose type 2\nfor type 2\nforever type 2\n" "form type 2\nfuzz type 2\nhalt\nif type 1\ninterpret type 1\niterate type 1\nleave type 1\nlower type 2\nname type 2\nnop type 1\n"
    "novalue\nnumeric type 5\noff type 2\non type 2\noptions type 1\notherwise type 1\nparse type 5\nprocedure type 5\n" "pull type 7\npush type 1\nqueue type 1\nreturn type 1\nsay type 1\nscientific type 2\nselect type 1\n"
    "signal type 5\nsource type 2\nsyntax\nto type 2\nthen type 1\ntrace type 5\nuntil type 2\nupper type 2\nvalue type 2\n" "var type 2\nversion type 2\nwhen type 1\nwhile type 2\nwith type 2\nstem type 2\noutput type 2\ninput type 2\n"
    "error type 2\nappend type 2\nreplace type 2\nnormal type 2\nstream type 2\ndefault type 1\n" "command type 2 alt e\nsystem type 2 alt e\nos2environment type 2 alt e\nenvironment type 2 alt e\n"
    "cmd type 2 alt e\npath type 2 alt e\nregina type 2 alt e\nrexx type 2 alt e\n" ".line alt e\n.mn alt e\n.rc alt e\n.result alt e\n.rs alt e\n.sigl alt e\n" ":function\n"
    "abbrev\ncenter\ncentre\nchangestr\ncompare\ncopies\ncountstr\ndatatype\ndelstr\ndelword\nbeep\ndirectory\n" "insert\nlastpos\nleft\nlength\nlower\noverlay\npos\nreverse\nright\nspace\nstrip\nsubstr\n"
    "subword\ntranslate\nverify\nword\nwordindex\nwordlength\nwordpos\nwords\nxrange\n" "abs\nformat\nmax\nmin\nsign\ntrunc\naddress\narg\ncondition\ndigits\nerrortext\nform\n"
    "fuzz\nsourceline\ntrace\nb2x\nbitand\nbitor\nbitxor\nc2d\nc2x\nd2c\nd2x\nx2b\nx2c\nx2d\n" "charin\ncharout\nchars\nlinein\nlineout\nlines\nqualify\nstream\ndate\nqueued\nrandom\n" "symbol\ntime\nupper\nvalue\n"
    ":postcompare\nclass [-\\/\\+\\=\\*\\<\\>|%&,] alt w\n"
    },
  { "*C.TLD", "C",
    "* C\n:case\nrespect\n:option\npreprocessor #\n:number\nc\n:identifier\n[a-zA-Z_] [a-zA-Z0-9_]\n" ":string\nsingle backslash\ndouble backslash\n:comment\npaired /* */ nonest\nline // any\n"
    ":label\ndelimiter : column 1\n:match\n( )\n{ }\n#ifdef,#if,#ifndef #endif #else,#elif,#elseif\n" ":keyword\n#define\n#elif\n#else\n#endif\n#error\n#if\n#ifdef\n#ifndef\n#include\n#line\n#pragma\n#undef\n"
    "auto\nbool\nbreak\ncase\nchar\nconst\ncontinue\ndefault\ndefined alt f\ndelete\ndo\ndouble\nelse\nenum\n" "extern\nfloat\nfor\ngoto\nif\nint\nlong\nnew\nregister\nreturn\nshort\nsigned\nsizeof\nstatic\nstruct\n"
    "switch\nthis\ntypedef\nunion\nunsigned\nvoid\nvolatile\nwhile\nclass\nprivate\npublic\n" ":postcompare\nclass [-\\/\\+\\=\\*\\<\\>|%&!,] alt x\n"
    },
  { "*SH.TLD", "SH",
    "* SH\n:case\nrespect\n:number\nc\n" ":string\nsingle backslash\ndouble backslash\n:identifier\n[a-zA-Z_] [a-zA-Z0-9_]\n" ":comment\nline # any\n:header\nline #! column 1\n"
    ":keyword\nif\nfi\nelif\nfor\nuntil\ncase\nesac\nwhile\nthen\nelse\ntest alt 4\nshift alt 4\ndo\ndone\nin\n" "continue alt 4\nbreak alt 4\ncd alt 4\necho alt 4\neval alt 4\nexec alt 4\n"
    "exit alt 4\nexport alt 4\ngetopts alt 4\nset alt 4\nunset alt 4\ntrap alt 4\n"
    },
  { "*TLD.TLD", "TLD",
    "* TLD\n:case\nignore\n" ":comment\nline * firstnonblank\n" ":header\nline : column 1\n"
    },
  { "*HTML.TLD", "HTML",
    "* HTML\n" ":case\nignore\n" ":identifier\n[a-zA-Z] [a-zA-Z0-9]\n" ":string\ndouble\n" ":comment\npaired <!-- --> nonest\n" ":markup\ntag < >\nreference & ;\n"
    },
  { "*DIR.TLD", "DIR",
    "* DIR\n" ":case\nignore\n" ":directory\n" "directory alt a\n" "executable\n" "link\n" "extensions .bak alt 8\n" "extensions .the alt 1\n" "extensions .c .cc .cpp .h .hpp alt 2\n" "extensions .rex .rexx .cmd alt 3\n"
    "extensions .exe .dll alt 6\n" "extensions .zip .Z .gz .tgz alt w\n" "extensions Makefile .mak alt 4\n"
    },
  { NULL, NULL, NULL }
};

PARSER_MAPPING default_parser_mapping[] = {

  /*
   * The following describes this table for those interested in
   * updating it manually.
   * Parameter 1 - Always NULL
   * Parameter 2 - Always NULL
   * Parameter 3 - Name of parser to associate mapping
   * Parameter 4 - Filemask - NULL if using "magic numbers"
   * Parameter 4 - Magic number string - NULL if using filemask
   * Parameter 5 - Length of "magic number" - 0 if using filemask
   * Parameter 6 - Always NULL
   */
  { NULL, NULL, (char_t *) "C", (char_t *) "*.c", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "C", (char_t *) "*.cc", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "C", (char_t *) "*.cpp", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "C", (char_t *) "*.h", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "C", (char_t *) "*.hpp", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "REXX", (char_t *) "*.cmd", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "REXX", (char_t *) "*.rex", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "REXX", (char_t *) "*.rexx", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "REXX", (char_t *) "*.the", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "REXX", (char_t *) ".therc", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "TLD", (char_t *) "*.tld", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "HTML", (char_t *) "*.htm", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "HTML", (char_t *) "*.html", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "DIR", (char_t *) "DIR.DIR", NULL, 0, NULL },
  { NULL, NULL, (char_t *) "REXX", NULL, (char_t *) "rexx", 4, NULL },
  { NULL, NULL, (char_t *) "REXX", NULL, (char_t *) "regina", 6, NULL },
  { NULL, NULL, (char_t *) "REXX", NULL, (char_t *) "rxx", 3, NULL },
  { NULL, NULL, (char_t *) "SH", NULL, (char_t *) "sh", 2, NULL },
  { NULL, NULL, (char_t *) "SH", NULL, (char_t *) "ksh", 3, NULL },
  { NULL, NULL, (char_t *) "SH", NULL, (char_t *) "bash", 4, NULL },
  { NULL, NULL, (char_t *) "SH", NULL, (char_t *) "zsh", 3, NULL },
  { NULL, NULL, NULL, NULL, NULL, 0, NULL }
};

void set_global_defaults(void) {
  int i;

  /*
   * Set defaults for all environments first...
   */
  BEEPx = FALSE;
  CAPREXXOUTx = FALSE;
  ERROROUTPUTx = FALSE;
  CAPREXXMAXx = 1000L;
  INSERTMODEx = FALSE;
  CLEARERRORKEYx = -1;
  LINEND_VALUEx = '#';
  EQUIVCHARx = '=';
  EQUIVCHARstr[0] = '=';
  EQUIVCHARstr[1] = '\0';
  REPROFILEx = FALSE;
  TABI_ONx = FALSE;
  TABI_Nx = 8;
  UNTAAx = FALSE;
  READONLYx = READONLY_OFF;
  MOUSEx = TRUE;
  if (display_length) {         /* if display_length has been specified) */
    EOLx = EOLOUT_NONE;
  } else {
    EOLx = EOLOUT_LF;
  }
  /*
   * Set CTLCHAR defaults...
   */
  for (i = 0; i < MAX_CTLCHARS; i++) {
    memset((char_t *) & ctlchar_attr[i], 0, sizeof(COLOUR_ATTR));
    ctlchar_char[i] = 0;
    ctlchar_protect[i] = FALSE;
  }
  /*
   * If STATUSLINE is OFF before we come here, leave it OFF.
   */
  if (STATUSLINEx != 'O') {
    STATUSLINEx = 'B';
  }
  TYPEAHEADx = FALSE;
  scroll_cursor_stay = TRUE;
  prefix_width = DEFAULT_PREFIX_WIDTH;
  strcpy((char *) BACKUP_SUFFIXx, ".bak");
  /*
   * Set defaults for individual environments next...
   */
  set_global_look_defaults();
  set_global_feel_defaults();
  return;
}

void set_global_look_defaults(void) {
  switch (compatible_look) {

    case COMPAT_THE:
      CLEARSCREENx = FALSE;
      CLOCKx = TRUE;
      HEXDISPLAYx = TRUE;
      PREFIXx = PREFIX_ON | PREFIX_LEFT;
      prefix_gap = 0;
      NONDISPx = '#';
      break;

    case COMPAT_KEDITW:
    case COMPAT_KEDIT:
      CLEARSCREENx = TRUE;
      CLOCKx = TRUE;
      HEXDISPLAYx = FALSE;
      NONDISPx = '#';
      PREFIXx = PREFIX_OFF;
      prefix_gap = 1;
      break;

    case COMPAT_XEDIT:
      CLEARSCREENx = TRUE;
      CLOCKx = FALSE;
      HEXDISPLAYx = FALSE;
      NONDISPx = '"';
      PREFIXx = PREFIX_ON | PREFIX_LEFT;
      prefix_gap = 1;
      break;

    case COMPAT_ISPF:
      CLEARSCREENx = TRUE;
      CLOCKx = FALSE;
      HEXDISPLAYx = FALSE;
      NONDISPx = '"';
      PREFIXx = PREFIX_ON | PREFIX_LEFT;
      prefix_gap = 1;
      break;
  }
}

void set_global_feel_defaults(void) {
  switch (compatible_feel) {

    case COMPAT_THE:
      CMDARROWSTABCMDx = FALSE;
      LINEND_STATUSx = FALSE;
      break;

    case COMPAT_KEDIT:
    case COMPAT_KEDITW:
      CMDARROWSTABCMDx = TRUE;
      LINEND_STATUSx = FALSE;
      break;

    case COMPAT_XEDIT:
      CMDARROWSTABCMDx = TRUE;
      LINEND_STATUSx = TRUE;
      break;

    case COMPAT_ISPF:
      CMDARROWSTABCMDx = TRUE;
      LINEND_STATUSx = TRUE;
      break;
  }
}

void set_file_defaults(FILE_DETAILS * filep) {
  /*
   * Set defaults for all environments first...
   */
  filep->autosave = 0;
  filep->autosave_alt = 0;
  filep->save_alt = 0;
  filep->tabsout_on = FALSE;
  filep->display_actual_filename = TRUE;
  filep->tabsout_num = 8;
  filep->eolout = EOLx;
  filep->timecheck = TRUE;
  filep->undoing = TRUE;
  filep->autocolour = TRUE;
  set_up_default_colours(filep, (COLOUR_ATTR *) NULL, ATTR_MAX);
  set_up_default_ecolours(filep);
  filep->trailing = TRAILING_ON;
  /*
   * Set defaults for individual environments next...
   */
  switch (compatible_feel) {

    case COMPAT_THE:
      filep->colouring = TRUE;
      filep->backup = BACKUP_KEEP;
      break;

    case COMPAT_XEDIT:
      filep->colouring = TRUE;
      filep->backup = BACKUP_OFF;
      break;

    case COMPAT_ISPF:
      filep->colouring = TRUE;
      filep->backup = BACKUP_OFF;
      break;

    case COMPAT_KEDITW:
      filep->colouring = TRUE;
      filep->backup = BACKUP_OFF;
      break;

    case COMPAT_KEDIT:
      filep->colouring = FALSE;
      filep->backup = BACKUP_OFF;
      break;
  }
}

void set_view_defaults(VIEW_DETAILS * viewp) {
  register int i = 0;
  short tabinc = 0;

  /*
   * Set defaults for all environments first...
   */
  viewp->arbchar_status = FALSE;
  viewp->arbchar_single = '?';
  viewp->arbchar_multiple = '$';
  viewp->arrow_on = TRUE;
  viewp->case_locate = CASE_IGNORE;
  viewp->case_change = CASE_RESPECT;
  viewp->case_sort = CASE_RESPECT;
  viewp->cmd_line = 'B';
  viewp->cmdline_col = 0;
  viewp->id_line = TRUE;
  viewp->in_ring = FALSE;
  viewp->current_row = 0;       /* this is set once we know the screen size */
  viewp->current_base = POSITION_MIDDLE;
  viewp->current_off = 0;
  viewp->display_high = 0;
  viewp->display_low = 0;
  viewp->hex = FALSE;
  viewp->highlight = HIGHLIGHT_NONE;
  viewp->highlight_high = 0;
  viewp->highlight_low = 0;
  viewp->inputmode = INPUTMODE_LINE;
  viewp->macro = FALSE;
  viewp->margin_left = 1;
  viewp->margin_right = 72;
  viewp->margin_indent = 0;
  viewp->margin_indent_offset_status = TRUE;
  viewp->msgmode_status = TRUE;
  viewp->position_status = TRUE;
  viewp->newline_aligned = TRUE;
  viewp->scale_base = POSITION_MIDDLE;
  viewp->scale_off = 1;
  viewp->scope_all = FALSE;
  viewp->shadow = TRUE;
  viewp->tab_base = POSITION_BOTTOM;
  viewp->tab_on = FALSE;
  viewp->numtabs = 32;
  viewp->tabsinc = 0;
  viewp->tofeof = TRUE;
  viewp->verify_start = 1;
  viewp->verify_col = 1;
  viewp->verify_end = max_line_length;
  viewp->verify_end_max = TRUE;
  viewp->hexshow_on = FALSE;
  viewp->hexshow_base = POSITION_TOP;
  viewp->hexshow_off = 7;
  viewp->word = 'N';
  viewp->wordwrap = FALSE;
  viewp->wrap = FALSE;
  viewp->zone_start = 1;
  viewp->synonym = TRUE;
  viewp->zone_end = max_line_length;
  viewp->zone_end_max = TRUE;
  viewp->autoscroll = (-1);     /* scroll half */
  viewp->boundmark = BOUNDMARK_OFF;     /* normal default is BOUNDMARK_ZONE */
  viewp->syntax_headers = HEADER_ALL;   /* ALL headers applied */
  viewp->thighlight_active = FALSE;

  initialise_target(&viewp->thighlight_target);

  if (viewp->cmd_line == 'O') {
    viewp->current_window = WINDOW_FILEAREA;
    viewp->previous_window = WINDOW_FILEAREA;
  } else {
    viewp->current_window = WINDOW_COMMAND;
    viewp->previous_window = WINDOW_FILEAREA;
  }

  /*
   * Set defaults for individual environments next...
   */

  switch (compatible_look) {

    case COMPAT_THE:
      viewp->number = TRUE;
      viewp->prefix = PREFIX_ON | PREFIX_LEFT;
      viewp->prefix_width = 6;
      viewp->prefix_gap = 0;
      viewp->scale_on = FALSE;
      viewp->tab_off = (-3);
      viewp->msgline_base = POSITION_TOP;
      viewp->msgline_off = 2;
      viewp->msgline_rows = 5;
      break;

    case COMPAT_XEDIT:
      viewp->number = FALSE;
      viewp->prefix = PREFIX_ON | PREFIX_LEFT;
      viewp->prefix_width = 6;
      viewp->prefix_gap = 1;
      viewp->scale_on = TRUE;
      viewp->tab_off = (-3);
      viewp->msgline_base = POSITION_TOP;
      viewp->msgline_off = 2;
      viewp->msgline_rows = 2;
      break;

    case COMPAT_ISPF:
      viewp->number = FALSE;
      viewp->prefix = PREFIX_ON | PREFIX_LEFT;
      viewp->prefix_width = 7;
      viewp->prefix_gap = 1;
      viewp->scale_on = TRUE;
      viewp->tab_off = (-3);
      viewp->msgline_base = POSITION_TOP;
      viewp->msgline_off = 2;
      viewp->msgline_rows = 2;
      viewp->cmd_line = 'T';
      break;

    case COMPAT_KEDITW:
    case COMPAT_KEDIT:
      viewp->number = FALSE;
      viewp->prefix = PREFIX_OFF;
      viewp->prefix_width = 6;
      viewp->prefix_gap = 1;
      viewp->scale_on = FALSE;
      viewp->tab_off = (-2);
      viewp->msgline_base = POSITION_TOP;
      viewp->msgline_off = 2;
      viewp->msgline_rows = 5;
      break;
  }

  switch (compatible_feel) {

    case COMPAT_THE:
      viewp->case_enter = viewp->case_enter_cmdline = viewp->case_enter_prefix = CASE_MIXED;
      viewp->imp_macro = TRUE;
      viewp->imp_os = TRUE;
      viewp->linend_status = FALSE;
      viewp->linend_value = '#';
      viewp->stay = TRUE;
      viewp->tab_off = (-3);
      viewp->thighlight_on = TRUE;
      tabinc = 8;
      break;

    case COMPAT_XEDIT:
      viewp->case_enter = viewp->case_enter_cmdline = viewp->case_enter_prefix = CASE_UPPER;
      viewp->imp_macro = TRUE;
      viewp->imp_os = TRUE;
      viewp->linend_status = TRUE;
      viewp->linend_value = '#';
      viewp->stay = FALSE;
      viewp->tab_off = (-3);
      viewp->thighlight_on = FALSE;
      tabinc = 3;
      break;

    case COMPAT_ISPF:
      viewp->case_enter = viewp->case_enter_cmdline = viewp->case_enter_prefix = CASE_UPPER;
      viewp->imp_macro = TRUE;
      viewp->imp_os = TRUE;
      viewp->linend_status = TRUE;
      viewp->linend_value = '#';
      viewp->stay = FALSE;
      viewp->tab_off = (-3);
      viewp->thighlight_on = FALSE;
      tabinc = 3;
      break;

    case COMPAT_KEDITW:
    case COMPAT_KEDIT:
      viewp->case_enter = viewp->case_enter_cmdline = viewp->case_enter_prefix = CASE_MIXED;
      viewp->imp_macro = FALSE;
      viewp->imp_os = FALSE;
      viewp->linend_status = FALSE;
      viewp->linend_value = '#';
      viewp->stay = TRUE;
      viewp->tab_off = (-2);
      viewp->thighlight_on = TRUE;
      tabinc = 8;
      break;
  }

  for (i = 0; i < MAX_NUMTABS; i++) {
    viewp->tabs[i] = 1 + (i * tabinc);
  }
}

short get_profile(char * prf_file, char * prf_arg) {
  FILE *fp = NULL;
  short rc = RC_OK;
  bool save_in_macro = in_macro;
  short macrorc = 0;

  in_macro = TRUE;
  /*
   * If REXX is supported, process the macro as a REXX macro...
   */
  if (rexx_support) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    rc = execute_macro_file(prf_file, prf_arg, &macrorc, FALSE);
    if (rc != RC_OK) {
      display_error(54, (char_t *) "", FALSE);
      rc = RC_SYSTEM_ERROR;
    }
  } else {
    /*
     * ...otherwise, process the file as a non-REXX macro file...
     */
    if ((fp = fopen((char *) prf_file, "r")) == NULL) {
      rc = RC_ACCESS_DENIED;
      display_error(8, prf_file, FALSE);
    }
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    if (rc == RC_OK) {
      rc = execute_command_file(fp);
      fclose(fp);
    }
    if (rc == RC_SYSTEM_ERROR) {
      display_error(53, (char_t *) "", FALSE);
    }
    if (rc == RC_NOREXX_ERROR) {
      display_error(52, (char_t *) "", FALSE);
    }
  }
  in_macro = save_in_macro;
  return (RC_OK);
}

short defaults_for_first_file(void) {
  register int i = 0;

  /*
   * Add to view linked list.
   */
  if ((CURRENT_VIEW = vll_add(vd_first, vd_current, sizeof(VIEW_DETAILS))) == (VIEW_DETAILS *) NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  if (vd_first == (VIEW_DETAILS *) NULL) {
    vd_first = vd_last = CURRENT_VIEW;
  }
  set_view_defaults(CURRENT_VIEW);

  /*
   * We now have CURRENT_VIEW and real screen sizes set, we can calculate the CURLINE value.
   */
  CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);

  CURRENT_VIEW->mark_type = M_NONE;
  CURRENT_VIEW->marked_line = FALSE;
  CURRENT_VIEW->marked_col = FALSE;
  CURRENT_VIEW->mark_start_line = 0;
  CURRENT_VIEW->mark_end_line = 0;
  CURRENT_VIEW->mark_start_col = 0;
  CURRENT_VIEW->mark_end_col = 0;
  CURRENT_VIEW->focus_line = 0L;
  CURRENT_VIEW->current_line = 0L;
  CURRENT_VIEW->current_column = 1;

  for (i = 0; i < VIEW_WINDOWS; i++) {
    CURRENT_VIEW->x[i] = CURRENT_VIEW->y[i] = 0;
  }

  CURRENT_FILE = (FILE_DETAILS *) NULL;

  CURRENT_SCREEN.screen_view = CURRENT_VIEW;

  number_of_views++;
  if (CURRENT_VIEW->cmd_line == 'O') {
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
    CURRENT_VIEW->previous_window = WINDOW_FILEAREA;
  } else {
    CURRENT_VIEW->current_window = WINDOW_COMMAND;
    CURRENT_VIEW->previous_window = WINDOW_FILEAREA;
  }

  CURRENT_VIEW->preserved_view_details = (PRESERVED_VIEW_DETAILS *) NULL;

  return (RC_OK);
}

short defaults_for_other_files(VIEW_DETAILS * base_view) {
  register int i = 0;

  if ((CURRENT_VIEW = vll_add(vd_first, vd_current, sizeof(VIEW_DETAILS))) == (VIEW_DETAILS *) NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  if (CURRENT_VIEW->next == (VIEW_DETAILS *) NULL) {
    vd_last = CURRENT_VIEW;
  }
  if (vd_first == (VIEW_DETAILS *) NULL) {
    vd_first = vd_last = CURRENT_VIEW;
  }
  if (base_view) {
    CURRENT_VIEW->current_line = base_view->current_line;
    CURRENT_VIEW->current_column = base_view->current_column;
    CURRENT_VIEW->display_high = base_view->display_high;
    CURRENT_VIEW->display_low = base_view->display_low;
    CURRENT_VIEW->hex = base_view->hex;
    CURRENT_VIEW->highlight = base_view->highlight;
    CURRENT_VIEW->highlight_high = base_view->highlight_high;
    CURRENT_VIEW->highlight_low = base_view->highlight_low;
    CURRENT_VIEW->newline_aligned = base_view->newline_aligned;
    CURRENT_VIEW->scope_all = base_view->scope_all;
    CURRENT_VIEW->shadow = base_view->shadow;
    CURRENT_VIEW->stay = base_view->stay;
    CURRENT_VIEW->hexshow_on = base_view->hexshow_on;
    CURRENT_VIEW->hexshow_base = base_view->hexshow_base;
    CURRENT_VIEW->hexshow_off = base_view->hexshow_off;
    CURRENT_VIEW->verify_start = base_view->verify_start;
    CURRENT_VIEW->verify_col = base_view->verify_col;
    CURRENT_VIEW->verify_end = base_view->verify_end;
    CURRENT_VIEW->verify_end_max = base_view->verify_end_max;
    CURRENT_VIEW->zone_start = base_view->zone_start;
    CURRENT_VIEW->zone_end = base_view->zone_end;
    CURRENT_VIEW->zone_end_max = base_view->zone_end_max;
    CURRENT_VIEW->autoscroll = base_view->autoscroll;
    CURRENT_VIEW->boundmark = base_view->boundmark;
    CURRENT_VIEW->syntax_headers = base_view->syntax_headers;

    for (i = 0; i < VIEW_WINDOWS; i++) {
      CURRENT_VIEW->y[i] = base_view->y[i];
      CURRENT_VIEW->x[i] = base_view->x[i];
    }

    CURRENT_VIEW->inputmode = base_view->inputmode;
    CURRENT_VIEW->focus_line = base_view->focus_line;
    CURRENT_VIEW->mark_type = base_view->mark_type;
    CURRENT_VIEW->marked_line = base_view->marked_line;
    CURRENT_VIEW->marked_col = base_view->marked_col;
    CURRENT_VIEW->mark_start_line = base_view->mark_start_line;
    CURRENT_VIEW->mark_end_line = base_view->mark_end_line;
    CURRENT_VIEW->mark_start_col = base_view->mark_start_col;
    CURRENT_VIEW->mark_end_col = base_view->mark_end_col;
    CURRENT_VIEW->thighlight_on = base_view->thighlight_on;
    CURRENT_VIEW->thighlight_active = FALSE;
    CURRENT_VIEW->arbchar_status = base_view->arbchar_status;
    CURRENT_VIEW->arbchar_single = base_view->arbchar_single;
    CURRENT_VIEW->arbchar_multiple = base_view->arbchar_multiple;
    CURRENT_VIEW->arrow_on = base_view->arrow_on;
    CURRENT_VIEW->case_enter = base_view->case_enter;
    CURRENT_VIEW->case_enter_cmdline = base_view->case_enter_cmdline;
    CURRENT_VIEW->case_enter_prefix = base_view->case_enter_prefix;
    CURRENT_VIEW->case_locate = base_view->case_locate;
    CURRENT_VIEW->case_change = base_view->case_change;
    CURRENT_VIEW->case_sort = base_view->case_sort;
    CURRENT_VIEW->cmd_line = base_view->cmd_line;
    CURRENT_VIEW->id_line = base_view->id_line;
    CURRENT_VIEW->current_base = base_view->current_base;
    CURRENT_VIEW->current_off = base_view->current_off;
    CURRENT_VIEW->current_row = base_view->current_row;
    CURRENT_VIEW->imp_macro = base_view->imp_macro;
    CURRENT_VIEW->imp_os = base_view->imp_os;
    CURRENT_VIEW->linend_status = base_view->linend_status;
    CURRENT_VIEW->linend_value = base_view->linend_value;
    CURRENT_VIEW->macro = base_view->macro;
    CURRENT_VIEW->margin_left = base_view->margin_left;
    CURRENT_VIEW->margin_right = base_view->margin_right;
    CURRENT_VIEW->margin_indent = base_view->margin_indent;
    CURRENT_VIEW->margin_indent_offset_status = base_view->margin_indent_offset_status;
    CURRENT_VIEW->msgmode_status = TRUE;
    CURRENT_VIEW->position_status = base_view->position_status;
    CURRENT_VIEW->msgline_base = base_view->msgline_base;
    CURRENT_VIEW->msgline_off = base_view->msgline_off;
    CURRENT_VIEW->msgline_rows = base_view->msgline_rows;
    CURRENT_VIEW->number = base_view->number;
    CURRENT_VIEW->prefix = base_view->prefix;
    CURRENT_VIEW->prefix_width = base_view->prefix_width;
    CURRENT_VIEW->prefix_gap = base_view->prefix_gap;
    CURRENT_VIEW->scale_base = base_view->scale_base;
    CURRENT_VIEW->scale_off = base_view->scale_off;
    CURRENT_VIEW->scale_on = base_view->scale_on;
    CURRENT_VIEW->tab_base = base_view->tab_base;
    CURRENT_VIEW->tab_off = base_view->tab_off;
    CURRENT_VIEW->tab_on = base_view->tab_on;
    CURRENT_VIEW->tofeof = base_view->tofeof;
    CURRENT_VIEW->word = base_view->word;
    CURRENT_VIEW->wordwrap = base_view->wordwrap;
    CURRENT_VIEW->wrap = base_view->wrap;
    CURRENT_VIEW->numtabs = base_view->numtabs;
    CURRENT_VIEW->tabsinc = base_view->tabsinc;

    memcpy(CURRENT_VIEW->tabs, base_view->tabs, MAX_NUMTABS * sizeof(length_t));
  } else {
    set_view_defaults(CURRENT_VIEW);

    CURRENT_VIEW->focus_line = 0L;
    CURRENT_VIEW->current_line = 0L;
    CURRENT_VIEW->current_column = 1;
    CURRENT_VIEW->mark_type = M_NONE;
    CURRENT_VIEW->marked_line = FALSE;
    CURRENT_VIEW->marked_col = FALSE;
    CURRENT_VIEW->mark_start_line = 0;
    CURRENT_VIEW->mark_end_line = 0;
    CURRENT_VIEW->mark_start_col = 0;
    CURRENT_VIEW->mark_end_col = 0;
    for (i = 0; i < VIEW_WINDOWS; i++) {
      CURRENT_VIEW->y[i] = CURRENT_VIEW->x[i] = 0;
    }
    CURRENT_VIEW->arbchar_status = PREVIOUS_VIEW->arbchar_status;
    CURRENT_VIEW->arbchar_single = PREVIOUS_VIEW->arbchar_single;
    CURRENT_VIEW->arbchar_multiple = PREVIOUS_VIEW->arbchar_multiple;
    CURRENT_VIEW->arrow_on = PREVIOUS_VIEW->arrow_on;
    CURRENT_VIEW->case_enter = PREVIOUS_VIEW->case_enter;
    CURRENT_VIEW->case_enter_cmdline = PREVIOUS_VIEW->case_enter_cmdline;
    CURRENT_VIEW->case_enter_prefix = PREVIOUS_VIEW->case_enter_prefix;
    CURRENT_VIEW->case_locate = PREVIOUS_VIEW->case_locate;
    CURRENT_VIEW->case_change = PREVIOUS_VIEW->case_change;
    CURRENT_VIEW->case_sort = PREVIOUS_VIEW->case_sort;
    CURRENT_VIEW->cmd_line = PREVIOUS_VIEW->cmd_line;
    CURRENT_VIEW->id_line = PREVIOUS_VIEW->id_line;
    CURRENT_VIEW->current_base = PREVIOUS_VIEW->current_base;
    CURRENT_VIEW->current_off = PREVIOUS_VIEW->current_off;
    CURRENT_VIEW->current_row = PREVIOUS_VIEW->current_row;
    CURRENT_VIEW->imp_macro = PREVIOUS_VIEW->imp_macro;
    CURRENT_VIEW->imp_os = PREVIOUS_VIEW->imp_os;
    CURRENT_VIEW->inputmode = PREVIOUS_VIEW->inputmode;
    CURRENT_VIEW->highlight = PREVIOUS_VIEW->highlight;
    CURRENT_VIEW->highlight_high = PREVIOUS_VIEW->highlight_high;
    CURRENT_VIEW->highlight_low = PREVIOUS_VIEW->highlight_low;
    CURRENT_VIEW->newline_aligned = PREVIOUS_VIEW->newline_aligned;
    CURRENT_VIEW->linend_status = PREVIOUS_VIEW->linend_status;
    CURRENT_VIEW->linend_value = PREVIOUS_VIEW->linend_value;
    CURRENT_VIEW->macro = PREVIOUS_VIEW->macro;
    CURRENT_VIEW->margin_left = PREVIOUS_VIEW->margin_left;
    CURRENT_VIEW->margin_right = PREVIOUS_VIEW->margin_right;
    CURRENT_VIEW->margin_indent = PREVIOUS_VIEW->margin_indent;
    CURRENT_VIEW->margin_indent_offset_status = PREVIOUS_VIEW->margin_indent_offset_status;
    CURRENT_VIEW->msgmode_status = TRUE;
    CURRENT_VIEW->position_status = PREVIOUS_VIEW->position_status;
    CURRENT_VIEW->msgline_base = PREVIOUS_VIEW->msgline_base;
    CURRENT_VIEW->msgline_off = PREVIOUS_VIEW->msgline_off;
    CURRENT_VIEW->msgline_rows = PREVIOUS_VIEW->msgline_rows;
    CURRENT_VIEW->number = PREVIOUS_VIEW->number;
    CURRENT_VIEW->prefix = PREVIOUS_VIEW->prefix;
    CURRENT_VIEW->prefix_width = PREVIOUS_VIEW->prefix_width;
    CURRENT_VIEW->prefix_gap = PREVIOUS_VIEW->prefix_gap;
    CURRENT_VIEW->scale_base = PREVIOUS_VIEW->scale_base;
    CURRENT_VIEW->scale_off = PREVIOUS_VIEW->scale_off;
    CURRENT_VIEW->scale_on = PREVIOUS_VIEW->scale_on;
    CURRENT_VIEW->tab_base = PREVIOUS_VIEW->tab_base;
    CURRENT_VIEW->tab_off = PREVIOUS_VIEW->tab_off;
    CURRENT_VIEW->tab_on = PREVIOUS_VIEW->tab_on;
    CURRENT_VIEW->tofeof = PREVIOUS_VIEW->tofeof;
    CURRENT_VIEW->word = PREVIOUS_VIEW->word;
    CURRENT_VIEW->wordwrap = PREVIOUS_VIEW->wordwrap;
    CURRENT_VIEW->wrap = PREVIOUS_VIEW->wrap;
    CURRENT_VIEW->numtabs = PREVIOUS_VIEW->numtabs;
    CURRENT_VIEW->tabsinc = PREVIOUS_VIEW->tabsinc;

    memcpy(CURRENT_VIEW->tabs, PREVIOUS_VIEW->tabs, MAX_NUMTABS * sizeof(length_t));
  }

  CURRENT_FILE = (FILE_DETAILS *) NULL;

  CURRENT_SCREEN.screen_view = CURRENT_VIEW;

  number_of_views++;
  if (CURRENT_VIEW->cmd_line == 'O') {
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
    CURRENT_VIEW->previous_window = WINDOW_FILEAREA;
  } else {
    CURRENT_VIEW->current_window = WINDOW_COMMAND;
    CURRENT_VIEW->previous_window = WINDOW_FILEAREA;
  }

  CURRENT_VIEW->preserved_view_details = (PRESERVED_VIEW_DETAILS *) NULL;
  initialise_target(&CURRENT_VIEW->thighlight_target);

  return (RC_OK);
}

short default_file_attributes(FILE_DETAILS * fd) {
  RESERVED *curr = NULL;
  short rc = RC_OK;

  set_file_defaults(CURRENT_FILE);

  CURRENT_FILE->fname = (char_t *) NULL;
  CURRENT_FILE->fpath = (char_t *) NULL;
  CURRENT_FILE->autosave_fname = (char_t *) NULL;
  CURRENT_FILE->file_views = 1;
  CURRENT_FILE->first_line = (LINE *) NULL;
  CURRENT_FILE->last_line = (LINE *) NULL;
  CURRENT_FILE->editv = (LINE *) NULL;
  CURRENT_FILE->first_reserved = (RESERVED *) NULL;
  CURRENT_FILE->fmode = 0;
  CURRENT_FILE->modtime = 0;
  CURRENT_FILE->pseudo_file = PSEUDO_REAL;
  CURRENT_FILE->disposition = FILE_NORMAL;
  CURRENT_FILE->first_ppc = CURRENT_FILE->last_ppc = NULL;
  CURRENT_FILE->preserved_file_details = NULL;
  CURRENT_FILE->fp = NULL;
  CURRENT_FILE->parser = NULL;
  /*
   * Set defaults for the current file based on the settings for the previous file.
   * The defaults to copy are:
   * - colours, reserved lines.
   */
  if (fd != NULL) {
    CURRENT_FILE->trailing = fd->trailing;
    CURRENT_FILE->display_actual_filename = fd->display_actual_filename;
    memcpy(CURRENT_FILE->attr, fd->attr, ATTR_MAX * sizeof(COLOUR_ATTR));
    /*
     * If in XEDIT compatibility mode, don't copy reserved lines to the new file.
     */
    if (compatible_feel != COMPAT_XEDIT) {
      curr = fd->first_reserved;
      while (curr != NULL) {
        if (add_reserved_line(curr->spec, curr->line, curr->base, curr->off, curr->attr, curr->autoscroll) == NULL) {
          rc = RC_OUT_OF_MEMORY;
          break;
        }
        curr = curr->next;
      }
    }
  }
  return (rc);
}

void set_screen_defaults(void) {
  register int i = 0;
  register int j = 0;
  row_t cmdline = 0;
  char_t prefix = 0;
  row_t start_row = 0, number_rows_less = 0, idline_rows = 0;
  short my_prefix_width = 0, my_prefix_gap = 0;
  bool my_arrow = TRUE;

  /*
   * Before doing any resizing, free up any memory associated with previous screens.
   */
  if (screen[0].sl != NULL) {
    /*
     * Free up allocated pointers for each SHOW_LINE
     */
    for (i = 0; i < screen[0].rows[WINDOW_FILEAREA]; i++) {
      if (screen[0].sl[i].highlight_type) {
        free((void *) screen[0].sl[i].highlight_type);
        screen[0].sl[i].highlight_type = NULL;
      }
    }
    free(screen[0].sl);
    screen[0].sl = NULL;
  }
  if (screen[1].sl != NULL) {
    /*
     * Free up allocated pointers for each SHOW_LINE
     */
    for (i = 0; i < screen[1].rows[WINDOW_FILEAREA]; i++) {
      if (screen[1].sl[i].highlight_type) {
        free((void *) screen[1].sl[i].highlight_type);
        screen[1].sl[i].highlight_type = NULL;
      }
    }
    free(screen[1].sl);
    screen[1].sl = NULL;
  }
  /*
   * Set values that affect the placement of each screen depending on the position of the status line...
   */
  switch (STATUSLINEx) {

    case 'B':
      start_row = 0;
      number_rows_less = 1;
      break;

    case 'T':
      start_row = 1;
      number_rows_less = 1;
      break;

    case 'O':
    case 'G':
      start_row = 0;
      number_rows_less = 0;
      break;
  }
  /*
   * If FILETABS is ON, then set its row to the first or second line of the screen depending if STATUSLINE is on top or not.
   */
  if (FILETABSx) {
    start_row++;
    number_rows_less++;
  }
  /*
   * Determine size of overall screen dimensions...
   */
  if (horizontal) {
    if (screen_rows[0] == 0) {
      screen[0].screen_rows = (terminal_lines - number_rows_less) / display_screens;
    } else {
      screen[0].screen_rows = screen_rows[0];
    }
    screen[0].screen_cols = terminal_cols;
    screen[0].screen_start_row = start_row;
    screen[0].screen_start_col = 0;
    if (display_screens == 1) {
      screen[1].screen_rows = 0;
      screen[1].screen_cols = 0;
      screen[1].screen_start_row = 0;
      screen[1].screen_start_col = 0;
    } else {
      screen[1].screen_rows = (terminal_lines - number_rows_less) - screen[0].screen_rows;
      screen[1].screen_cols = terminal_cols;
      screen[1].screen_start_row = screen[0].screen_rows + start_row;
      screen[1].screen_start_col = 0;
    }
  } else {
    if (screen_cols[0] == 0) {
      screen[0].screen_cols = (terminal_cols / display_screens) - ((display_screens == 1) ? 0 : 1);
    } else {
      screen[0].screen_cols = screen_cols[0];
    }
    screen[0].screen_rows = terminal_lines - number_rows_less;
    screen[0].screen_start_row = start_row;
    screen[0].screen_start_col = 0;
    if (display_screens == 1) {
      screen[1].screen_rows = 0;
      screen[1].screen_cols = 0;
      screen[1].screen_start_row = 0;
      screen[1].screen_start_col = 0;
    } else {
      screen[1].screen_rows = terminal_lines - number_rows_less;
      screen[1].screen_cols = terminal_cols - screen[0].screen_cols - 2;
      screen[1].screen_start_row = start_row;
      screen[1].screen_start_col = screen[0].screen_cols + 2;
    }
  }
  /*
   * Reset window sizes to zero...
   */
  for (i = 0; i < MAX_SCREENS; i++) {
    for (j = 0; j < VIEW_WINDOWS; j++) {
      screen[i].start_row[j] = 0;
      screen[i].start_col[j] = 0;
      screen[i].rows[j] = 0;
      screen[i].cols[j] = 0;
    }
  }
  /*
   * Determine size of each window for each screen...
   */
  for (i = 0; i < display_screens; i++) {
    if (screen[i].screen_view == (VIEW_DETAILS *) NULL) {
      cmdline = 'B';
      idline_rows = 1;
      prefix = PREFIXx;
      my_prefix_width = DEFAULT_PREFIX_WIDTH;
      my_prefix_gap = DEFAULT_PREFIX_GAP;
      my_arrow = TRUE;
    } else {
      cmdline = screen[i].screen_view->cmd_line;
      idline_rows = (screen[i].screen_view->id_line) ? 1 : 0;
      prefix = screen[i].screen_view->prefix;
      my_prefix_width = screen[i].screen_view->prefix_width;
      my_prefix_gap = screen[i].screen_view->prefix_gap;
      my_arrow = screen[i].screen_view->arrow_on;
    }
    switch (cmdline) {

      case 'T':                /* command line on top */
        screen[i].start_row[WINDOW_FILEAREA] = screen[i].screen_start_row + 1 + idline_rows;
        screen[i].rows[WINDOW_FILEAREA] = screen[i].screen_rows - 1 - idline_rows;
        screen[i].start_row[WINDOW_COMMAND] = screen[i].screen_start_row + idline_rows;
        screen[i].rows[WINDOW_COMMAND] = 1;
        screen[i].cols[WINDOW_COMMAND] = screen[i].screen_cols - ((my_arrow) ? my_prefix_width : 0);
        screen[i].start_row[WINDOW_ARROW] = screen[i].start_row[WINDOW_COMMAND];
        screen[i].rows[WINDOW_ARROW] = ((my_arrow) ? 1 : 0);
        screen[i].cols[WINDOW_ARROW] = ((my_arrow) ? my_prefix_width : 0);
        break;

      case 'B':                /* command line on bottom */
        screen[i].start_row[WINDOW_FILEAREA] = screen[i].screen_start_row + idline_rows;
        screen[i].rows[WINDOW_FILEAREA] = screen[i].screen_rows - 1 - idline_rows;
        screen[i].start_row[WINDOW_COMMAND] = screen[i].screen_start_row + screen[i].screen_rows - 1;
        screen[i].rows[WINDOW_COMMAND] = 1;
        screen[i].cols[WINDOW_COMMAND] = screen[i].screen_cols - ((my_arrow) ? my_prefix_width : 0);
        screen[i].start_row[WINDOW_ARROW] = screen[i].start_row[WINDOW_COMMAND];
        screen[i].rows[WINDOW_ARROW] = ((my_arrow) ? 1 : 0);
        screen[i].cols[WINDOW_ARROW] = ((my_arrow) ? my_prefix_width : 0);
        break;

      case 'O':                /* command line off */
        screen[i].start_row[WINDOW_FILEAREA] = screen[i].screen_start_row + idline_rows;
        screen[i].rows[WINDOW_FILEAREA] = screen[i].screen_rows - idline_rows;
        screen[i].start_row[WINDOW_COMMAND] = 0;
        screen[i].rows[WINDOW_COMMAND] = 0;
        screen[i].cols[WINDOW_COMMAND] = 0;
        screen[i].start_row[WINDOW_ARROW] = 0;
        screen[i].rows[WINDOW_ARROW] = 0;
        screen[i].cols[WINDOW_ARROW] = 0;
        break;
    }
    screen[i].start_col[WINDOW_ARROW] = screen[i].screen_start_col;
    screen[i].start_col[WINDOW_COMMAND] = screen[i].screen_start_col + ((my_arrow) ? my_prefix_width : 0);
    if (idline_rows == 1) {
      screen[i].start_row[WINDOW_IDLINE] = screen[i].screen_start_row;
      screen[i].start_col[WINDOW_IDLINE] = screen[i].screen_start_col;
      screen[i].rows[WINDOW_IDLINE] = 1;
      screen[i].cols[WINDOW_IDLINE] = screen[i].screen_cols;
    }
    if (prefix) {
      screen[i].start_row[WINDOW_PREFIX] = screen[i].start_row[WINDOW_FILEAREA];
      screen[i].start_row[WINDOW_GAP] = screen[i].start_row[WINDOW_FILEAREA];
      screen[i].rows[WINDOW_PREFIX] = screen[i].rows[WINDOW_FILEAREA];
      screen[i].rows[WINDOW_GAP] = screen[i].rows[WINDOW_FILEAREA];
      screen[i].cols[WINDOW_PREFIX] = my_prefix_width - my_prefix_gap;
      screen[i].cols[WINDOW_GAP] = my_prefix_gap;
      screen[i].cols[WINDOW_FILEAREA] = screen[i].screen_cols - my_prefix_width;
      if ((prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
        screen[i].start_col[WINDOW_PREFIX] = screen[i].screen_start_col;
        screen[i].start_col[WINDOW_GAP] = screen[i].screen_start_col + (my_prefix_width - my_prefix_gap);
        screen[i].start_col[WINDOW_FILEAREA] = screen[i].screen_start_col + my_prefix_width;
      } else {
        screen[i].start_col[WINDOW_FILEAREA] = screen[i].screen_start_col;
        screen[i].start_col[WINDOW_PREFIX] = ((screen[i].screen_start_col + screen[i].screen_cols + my_prefix_gap) - my_prefix_width);
        screen[i].start_col[WINDOW_GAP] = (screen[i].screen_start_col + screen[i].screen_cols) - my_prefix_width;
      }
    } else {
      screen[i].cols[WINDOW_FILEAREA] = screen[i].screen_cols;
      screen[i].start_col[WINDOW_FILEAREA] = screen[i].screen_start_col;
      screen[i].rows[WINDOW_PREFIX] = 0;
      screen[i].cols[WINDOW_PREFIX] = 0;
      screen[i].rows[WINDOW_GAP] = 0;
      screen[i].cols[WINDOW_GAP] = 0;
    }
  }
  /*
   * We now have the size of each screen, so we can allocate the display line arrays.
   */
  if ((screen[0].sl = (SHOW_LINE *) malloc(screen[0].rows[WINDOW_FILEAREA] * sizeof(SHOW_LINE))) == NULL) {
    cleanup();
    display_error(30, (char_t *) "", FALSE);
    exit(1);
  }
  memset(screen[0].sl, 0, screen[0].rows[WINDOW_FILEAREA] * sizeof(SHOW_LINE));
  if (display_screens > 1) {
    if ((screen[1].sl = (SHOW_LINE *) malloc(screen[1].rows[WINDOW_FILEAREA] * sizeof(SHOW_LINE))) == NULL) {
      cleanup();
      display_error(30, (char_t *) "", FALSE);
      exit(1);
    }
    memset(screen[1].sl, 0, screen[1].rows[WINDOW_FILEAREA] * sizeof(SHOW_LINE));
  }
}

short set_THE_key_defaults(int prey, int prex) {
  short rc = RC_OK;
  /*
   * This function is for resetting all default values for THE compatibility mode.
   */
  Define((char_t *) "F2");
  Define((char_t *) "F3");
  Define((char_t *) "F4");
  Define((char_t *) "F6");
  Define((char_t *) "F7");
  Define((char_t *) "F8");
  Define((char_t *) "F9");
  Define((char_t *) "F11");
  Define((char_t *) "CURD");
  Define((char_t *) "CURL");
  Define((char_t *) "CURR");
  Define((char_t *) "CURU");
  Define((char_t *) "C-M");
  Define((char_t *) "F16");
  Define((char_t *) "S-TAB");
  Define((char_t *) "C-I");
  return (rc);
}

short set_XEDIT_key_defaults(int prey, int prex) {
  short rc = RC_OK;
  /*
   * This function is for resetting all default values for XEDIT compatibility mode.
   */
  Define((char_t *) "F2 sos lineadd");
  Define((char_t *) "F3 quit");
  Define((char_t *) "F4 sos tabf");
  Define((char_t *) "F6 ?");
  Define((char_t *) "F7 backward");
  Define((char_t *) "F8 forward");
  Define((char_t *) "F9 =");
  Define((char_t *) "F11 spltjoin");
  Define((char_t *) "CURD cursor screen down");
  Define((char_t *) "CURL cursor screen left");
  Define((char_t *) "CURR cursor screen right");
  Define((char_t *) "CURU cursor screen up");
  Define((char_t *) "C-M sos doprefix execute");
  Define((char_t *) "S-TAB sos tabfieldb");
  Define((char_t *) "C-I sos tabfieldf");
  return (rc);
}

short set_ISPF_key_defaults(int prey, int prex) {
  short rc = RC_OK;
  /*
   * This function is for resetting all default values for XEDIT compatibility mode.
   */
  Define((char_t *) "F2 sos lineadd");
  Define((char_t *) "F3 quit");
  Define((char_t *) "F4 sos tabf");
  Define((char_t *) "F6 ?");
  Define((char_t *) "F7 up");
  Define((char_t *) "F8 down");
  Define((char_t *) "F9 =");
  Define((char_t *) "F11 spltjoin");
  Define((char_t *) "CURD cursor screen down");
  Define((char_t *) "CURL cursor screen left");
  Define((char_t *) "CURR cursor screen right");
  Define((char_t *) "CURU cursor screen up");
  Define((char_t *) "C-M sos doprefix execute");
  Define((char_t *) "S-TAB sos tabfieldb");
  Define((char_t *) "C-I sos tabfieldf");
  return (rc);
}

short set_KEDIT_key_defaults(int prey, int prex) {
  short rc = RC_OK;
  /*
   * This function is for resetting all default values for KEDIT compatibility mode.
   */
  Define((char_t *) "F2 sos lineadd");
  Define((char_t *) "F3 quit");
  Define((char_t *) "F4 sos tab");
  Define((char_t *) "F5 sos makecurr");
  Define((char_t *) "F6 ?");
  Define((char_t *) "F7 ");
  Define((char_t *) "F8 dup");
  Define((char_t *) "F9 =");
  Define((char_t *) "F11 spltjoin");
  Define((char_t *) "F12 cursor home");
  Define((char_t *) "CURD");
  Define((char_t *) "CURL cursor kedit left");
  Define((char_t *) "CURR cursor kedit right");
  Define((char_t *) "CURU");
  Define((char_t *) "C-M");
  Define((char_t *) "S-TAB");
  Define((char_t *) "C-I");
  return (rc);
}

short construct_default_parsers(void) {
  short rc = RC_OK;
  PARSER_DETAILS *curr;
  int i;
  char_t tmp[20];

  for (i = 0;; i++) {
    if (default_parsers[i].contents == NULL) {
      break;
    }
    rc = construct_parser((char_t*)default_parsers[i].contents, strlen(default_parsers[i].contents), &curr, (char_t*)default_parsers[i].name, (char_t*)default_parsers[i].filename);
    if (rc != RC_OK) {
      break;
    }
    if (rexx_support) {
      /*
       * If we have a Rexx interpreter, register an implied extract function for the number of parsers we now have.
       */
      sprintf((char *) tmp, "parser.%d", i + 1);
      MyRexxRegisterFunctionExe(tmp);
    }
  }

  return (rc);
}

short destroy_all_parsers(void) {
  short rc = RC_OK;
  PARSER_DETAILS *curr = first_parser;

  while (curr->next != NULL) {
    destroy_parser(curr);
    curr = curr->next;
  }
  first_parser = last_parser = parserll_free(first_parser);
  return (rc);
}

short construct_default_parser_mapping(void) {
  short rc = RC_OK;
  PARSER_MAPPING *curr;
  PARSER_DETAILS *currp;
  int i;

  for (i = 0;; i++) {
    if (default_parser_mapping[i].parser_name == NULL) {
      break;
    }
    last_parser_mapping = curr = mappingll_add(first_parser_mapping, last_parser_mapping, sizeof(PARSER_MAPPING));
    if (first_parser_mapping == NULL) {
      first_parser_mapping = curr;
    }
    if (default_parser_mapping[i].filemask) {
      curr->filemask = (char_t *) malloc(1 + strlen((char *) default_parser_mapping[i].filemask) * sizeof(char_t));
      if (curr->filemask == NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) curr->filemask, (char *) default_parser_mapping[i].filemask);
    }
    if (default_parser_mapping[i].magic_number) {
      curr->magic_number = (char_t *) malloc((1 + default_parser_mapping[i].magic_number_length) * sizeof(char_t));
      if (curr->magic_number == NULL) {
        display_error(30, (char_t *) "", FALSE);
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) curr->magic_number, (char *) default_parser_mapping[i].magic_number);
      curr->magic_number_length = default_parser_mapping[i].magic_number_length;
    }
    curr->parser = NULL;
    for (currp = first_parser; currp != NULL; currp = currp->next) {
      if (strcasecmp((char *) currp->parser_name, (char *) default_parser_mapping[i].parser_name) == 0) {
        curr->parser = currp;
        break;
      }
    }
    if (rexx_support) {
      /*
       * If we have a Rexx interpreter, register an implied extract function for the number of parsers we now have.
       */
      char_t tmp[20];

      sprintf((char *) tmp, "autocolor.%d", i + 1);
      MyRexxRegisterFunctionExe(tmp);
      sprintf((char *) tmp, "autocolour.%d", i + 1);
      MyRexxRegisterFunctionExe(tmp);
    }
  }
  return (rc);
}

char_t *find_default_parser(char_t * name) {
  int i;
  char_t *contents = NULL;

  for (i = 0;; i++) {
    if (default_parsers[i].filename == NULL) {
      break;
    }
    if (strcasecmp((char *) default_parsers[i].filename, (char *) name) == 0) {
      contents = (char_t*)default_parsers[i].contents;
      break;
    }
  }
  return (contents);
}
