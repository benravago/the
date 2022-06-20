/* QUERY.C - Functions related to QUERY,STATUS and EXTRACT             */
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2013 Mark Hessling
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

#include <the.h>
#include <proto.h>

#include <query.h>

short extract_point_settings(short, char_t *);
short extract_prefix_settings(short, char_t *, char_t);
short extract_colour_settings(short, char_t *, char_t, char_t *, bool, bool);
short extract_autocolour_settings(short, char_t *, char_t, char_t *, bool);
void get_etmode(char_t *, char_t *);

extern ExtractFunction extract_after_function;
extern ExtractFunction extract_alt;
extern ExtractFunction extract_alt_function;
extern ExtractFunction extract_altkey_function;
extern ExtractFunction extract_arbchar;
extern ExtractFunction extract_autocolour;
extern ExtractFunction extract_autocolor;
extern ExtractFunction extract_autosave;
extern ExtractFunction extract_autoscroll;
extern ExtractFunction extract_backup;
extern ExtractFunction extract_batch_function;
extern ExtractFunction extract_before_function;
extern ExtractFunction extract_beep;
extern ExtractFunction extract_blank_function;
extern ExtractFunction extract_block;
extern ExtractFunction extract_block_function;
extern ExtractFunction extract_bottomedge_function;
extern ExtractFunction extract_case;
extern ExtractFunction extract_clearerrorkey;
extern ExtractFunction extract_clearscreen;
extern ExtractFunction extract_clock;
extern ExtractFunction extract_command_function;
extern ExtractFunction extract_cmdarrows;
extern ExtractFunction extract_cmdline;
extern ExtractFunction extract_color;
extern ExtractFunction extract_coloring;
extern ExtractFunction extract_colour;
extern ExtractFunction extract_colouring;
extern ExtractFunction extract_column;
extern ExtractFunction extract_compat;
extern ExtractFunction extract_ctrl_function;
extern ExtractFunction extract_ctlchar;
extern ExtractFunction extract_curline;
extern ExtractFunction extract_cursor;
extern ExtractFunction extract_cursorstay;
extern ExtractFunction extract_current_function;
extern ExtractFunction extract_define;
extern ExtractFunction extract_defsort;
extern ExtractFunction extract_dir_function;
extern ExtractFunction extract_dirfileid;
extern ExtractFunction extract_end_function;
extern ExtractFunction extract_display;
extern ExtractFunction extract_ecolor;
extern ExtractFunction extract_ecolour;
extern ExtractFunction extract_efileid;
extern ExtractFunction extract_eof;
extern ExtractFunction extract_eof_function;
extern ExtractFunction extract_eolout;
extern ExtractFunction extract_equivchar;
extern ExtractFunction extract_errorformat;
extern ExtractFunction extract_erroroutput;
extern ExtractFunction extract_etmode;
extern ExtractFunction extract_field;
extern ExtractFunction extract_fieldword;
extern ExtractFunction extract_first_function;
extern ExtractFunction extract_focuseof_function;
extern ExtractFunction extract_focustof_function;
extern ExtractFunction extract_filename;
extern ExtractFunction extract_filestatus;
extern ExtractFunction extract_filetabs;
extern ExtractFunction extract_fmode;
extern ExtractFunction extract_fname;
extern ExtractFunction extract_fpath;
extern ExtractFunction extract_ftype;
extern ExtractFunction extract_fullfname;
extern ExtractFunction extract_getenv;
extern ExtractFunction extract_hex;
extern ExtractFunction extract_header;
extern ExtractFunction extract_hexdisplay;
extern ExtractFunction extract_hexshow;
extern ExtractFunction extract_highlight;
extern ExtractFunction extract_idline;
extern ExtractFunction extract_impmacro;
extern ExtractFunction extract_impos;
extern ExtractFunction extract_inblock_function;
extern ExtractFunction extract_incommand_function;
extern ExtractFunction extract_initial_function;
extern ExtractFunction extract_inprefix_function;
extern ExtractFunction extract_inputmode;
extern ExtractFunction extract_insertmode;
extern ExtractFunction extract_insertmode_function;
extern ExtractFunction extract_interface;
extern ExtractFunction extract_lastkey;
extern ExtractFunction extract_lastmsg;
extern ExtractFunction extract_lastop;
extern ExtractFunction extract_lastrc;
extern ExtractFunction extract_leftedge_function;
extern ExtractFunction extract_length;
extern ExtractFunction extract_line;
extern ExtractFunction extract_lineflag;
extern ExtractFunction extract_linend;
extern ExtractFunction extract_lscreen;
extern ExtractFunction extract_macro;
extern ExtractFunction extract_macroext;
extern ExtractFunction extract_macropath;
extern ExtractFunction extract_margins;
extern ExtractFunction extract_modifiable_function;
extern ExtractFunction extract_monitor;
extern ExtractFunction extract_mouse;
extern ExtractFunction extract_msgline;
extern ExtractFunction extract_msgmode;
extern ExtractFunction extract_nbfile;
extern ExtractFunction extract_nbscope;
extern ExtractFunction extract_newlines;
extern ExtractFunction extract_nondisp;
extern ExtractFunction extract_number;
extern ExtractFunction extract_parser;
extern ExtractFunction extract_pagewrap;
extern ExtractFunction extract_pending;
extern ExtractFunction extract_point;
extern ExtractFunction extract_position;
extern ExtractFunction extract_prefix;
extern ExtractFunction extract_printer;
extern ExtractFunction extract_profile;
extern ExtractFunction extract_pscreen;
extern ExtractFunction extract_reprofile;
extern ExtractFunction extract_readonly;
extern ExtractFunction extract_readv;
extern ExtractFunction extract_reserved;
extern ExtractFunction extract_rexxhalt;
extern ExtractFunction extract_rexxoutput;
extern ExtractFunction extract_rightedge_function;
extern ExtractFunction extract_ring;
extern ExtractFunction extract_scale;
extern ExtractFunction extract_scope;
extern ExtractFunction extract_screen;
extern ExtractFunction extract_select;
extern ExtractFunction extract_shadow;
extern ExtractFunction extract_shadow_function;
extern ExtractFunction extract_shift_function;
extern ExtractFunction extract_showkey;
extern ExtractFunction extract_size;
extern ExtractFunction extract_slk;
extern ExtractFunction extract_spacechar_function;
extern ExtractFunction extract_statopt;
extern ExtractFunction extract_statusline;
extern ExtractFunction extract_stay;
extern ExtractFunction extract_synelem;
extern ExtractFunction extract_synonym;
extern ExtractFunction extract_tabkey;
extern ExtractFunction extract_tabline;
extern ExtractFunction extract_tabs;
extern ExtractFunction extract_tabsin;
extern ExtractFunction extract_tabsout;
extern ExtractFunction extract_targetsave;
extern ExtractFunction extract_terminal;
extern ExtractFunction extract_thighlight;
extern ExtractFunction extract_timecheck;
extern ExtractFunction extract_tof;
extern ExtractFunction extract_tof_function;
extern ExtractFunction extract_tofeof;
extern ExtractFunction extract_topedge_function;
extern ExtractFunction extract_trailing;
extern ExtractFunction extract_typeahead;
extern ExtractFunction extract_ui;
extern ExtractFunction extract_undoing;
extern ExtractFunction extract_untaa;
extern ExtractFunction extract_utf8;
extern ExtractFunction extract_verify;
extern ExtractFunction extract_vershift;
extern ExtractFunction extract_verone_function;
extern ExtractFunction extract_version;
extern ExtractFunction extract_width;
extern ExtractFunction extract_word;
extern ExtractFunction extract_wordwrap;
extern ExtractFunction extract_wrap;
extern ExtractFunction extract_zone;

char_t query_num0[10];        /* DO NOT USE THIS FOR DATA !! */
char_t query_num1[10];
char_t query_num2[10];
char_t query_num3[40];
char_t query_num4[40];
char_t query_num5[10];
char_t query_num6[10];
char_t query_num7[10];
char_t query_num8[10];
char_t query_rsrvd[MAX_FILE_NAME + 1];

/***********************************************************************
 * Keep the following items in alphabetic order of name.
 * Columns:
 *   query name
 *   name length
 *   min abbrev length
 *   unique number for item
 *   number of returned values for query/extract)
 *   number of implied extract functions
 *   level of item; global, file, view
 *   valid query types
 *   function to call to set item values
 ***********************************************************************/
QUERY_ITEM query_item[] = {
  { (char_t *) "alt", 3, 3, ITEM_ALT, 2, 2, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_alt },
  { (char_t *) "arbchar", 7, 3, ITEM_ARBCHAR, 3, 3, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_arbchar },
  { (char_t *) "autocolor", 9, 9, ITEM_AUTOCOLOR, 3, 0, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT, extract_autocolor },
  { (char_t *) "autocolour", 10, 10, ITEM_AUTOCOLOUR, 3, 0, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT, extract_autocolour },
  { (char_t *) "autosave", 8, 2, ITEM_AUTOSAVE, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_autosave },
  { (char_t *) "autoscroll", 10, 6, ITEM_AUTOSCROLL, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_autoscroll },
  { (char_t *) "backup", 6, 4, ITEM_BACKUP, 2, 2, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_backup },
  { (char_t *) "beep", 4, 4, ITEM_BEEP, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_beep },
  { (char_t *) "block", 5, 5, ITEM_BLOCK, 0, 6, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_block },
  { (char_t *) "case", 4, 4, ITEM_CASE, 6, 6, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_case },
  { (char_t *) "clearerrorkey", 13, 6, ITEM_CLEARERRORKEY, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_clearerrorkey },
  { (char_t *) "clearscreen", 11, 6, ITEM_CLEARSCREEN, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_clearscreen },
  { (char_t *) "clock", 5, 5, ITEM_CLOCK, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_clock },
  { (char_t *) "cmdarrows", 9, 4, ITEM_CMDARROWS, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_cmdarrows },
  { (char_t *) "cmdline", 7, 3, ITEM_CMDLINE, 2, 3, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_cmdline },
  { (char_t *) "color", 5, 5, ITEM_COLOR, 1, ATTR_MAX, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT, extract_color },
  { (char_t *) "coloring", 8, 8, ITEM_COLORING, 3, 3, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_coloring },
  { (char_t *) "colour", 6, 5, ITEM_COLOUR, 1, ATTR_MAX, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT, extract_colour },
  { (char_t *) "colouring", 9, 9, ITEM_COLOURING, 3, 3, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_colouring },
  { (char_t *) "column", 6, 3, ITEM_COLUMN, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_column },
  { (char_t *) "compat", 6, 6, ITEM_COMPAT, 3, 3, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_compat },
  { (char_t *) "ctlchar", 7, 3, ITEM_CTLCHAR, 3, 3, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_ctlchar },
  { (char_t *) "curline", 7, 3, ITEM_CURLINE, 6, 6, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_curline },
  { (char_t *) "cursor", 6, 4, ITEM_CURSOR, 8, 8, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_cursor },
  { (char_t *) "cursorstay", 10, 8, ITEM_CURSORSTAY, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_cursorstay },
  { (char_t *) "define", 6, 6, ITEM_DEFINE, 1, 0, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_define },
  { (char_t *) "defsort", 7, 7, ITEM_DEFSORT, 2, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_defsort },
  { (char_t *) "dirfileid", 9, 9, ITEM_DIRFILEID, 2, 2, LVL_FILE, QUERY_EXTRACT, extract_dirfileid },
  { (char_t *) "display", 7, 4, ITEM_DISPLAY, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_display },
  { (char_t *) "ecolor", 6, 6, ITEM_ECOLOR, 2, ECOLOUR_MAX, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_ecolor },
  { (char_t *) "ecolour", 7, 7, ITEM_ECOLOUR, 2, ECOLOUR_MAX, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_ecolour },
  { (char_t *) "efileid", 7, 6, ITEM_EFILEID, 2, 2, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT, extract_efileid },
  { (char_t *) "eof", 3, 3, ITEM_EOF, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_eof },
  { (char_t *) "eolout", 6, 3, ITEM_EOLOUT, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_eolout },
  { (char_t *) "equivchar", 9, 6, ITEM_EQUIVCHAR, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_equivchar },
  { (char_t *) "errorformat", 11, 6, ITEM_ERRORFORMAT, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_errorformat },
  { (char_t *) "erroroutput", 11, 8, ITEM_ERROROUTPUT, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_erroroutput },
  { (char_t *) "etmode", 6, 6, ITEM_ETMODE, 2, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_etmode },
  { (char_t *) "fext", 4, 2, ITEM_FEXT, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_ftype },
  { (char_t *) "field", 5, 5, ITEM_FIELD, 4, 4, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_field },
  { (char_t *) "fieldword", 9, 9, ITEM_FIELDWORD, 3, 3, LVL_VIEW, QUERY_EXTRACT, extract_fieldword },
  { (char_t *) "filename", 8, 5, ITEM_FILENAME, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_filename },
  { (char_t *) "filestatus", 10, 10, ITEM_FILESTATUS, 3, 3, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_filestatus },
  { (char_t *) "filetabs", 8, 8, ITEM_FILETABS, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_filetabs },
  { (char_t *) "fmode", 5, 2, ITEM_FMODE, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_fmode },
  { (char_t *) "fname", 5, 2, ITEM_FNAME, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_fname },
  { (char_t *) "fpath", 5, 2, ITEM_FPATH, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_fpath },
  { (char_t *) "ftype", 5, 2, ITEM_FTYPE, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT | QUERY_MODIFY, extract_ftype },
  { (char_t *) "fullfname", 9, 6, ITEM_FULLFNAME, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_fullfname },
  { (char_t *) "getenv", 6, 6, ITEM_GETENV, 1, 1, LVL_GLOB, QUERY_EXTRACT, extract_getenv },
  { (char_t *) "header", 4, 3, ITEM_HEADER, 11, 11, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_header },
  { (char_t *) "hex", 3, 3, ITEM_HEX, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_hex },
  { (char_t *) "hexdisplay", 10, 7, ITEM_HEXDISPLAY, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_hexdisplay },
  { (char_t *) "hexshow", 7, 4, ITEM_HEXSHOW, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_hexshow },
  { (char_t *) "highlight", 9, 4, ITEM_HIGHLIGHT, 1, 3, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_highlight },
  { (char_t *) "idline", 6, 2, ITEM_IDLINE, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_idline },
  { (char_t *) "impmacro", 8, 6, ITEM_IMPMACRO, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_impmacro },
  { (char_t *) "impos", 5, 5, ITEM_IMPOS, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_impos },
  { (char_t *) "inputmode", 9, 6, ITEM_INPUTMODE, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_inputmode },
  { (char_t *) "insertmode", 10, 6, ITEM_INSERTMODE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_insertmode },
  { (char_t *) "interface", 9, 3, ITEM_INTERFACE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_interface },
  { (char_t *) "lastkey", 7, 7, ITEM_LASTKEY, 4, 4, LVL_GLOB, QUERY_EXTRACT, extract_lastkey },
  { (char_t *) "lastmsg", 7, 4, ITEM_LASTMSG, 1, 1, LVL_GLOB, QUERY_EXTRACT, extract_lastmsg },
  { (char_t *) "lastop", 6, 6, ITEM_LASTOP, 9, 9, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_lastop },
  { (char_t *) "lastrc", 6, 6, ITEM_LASTRC, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_lastrc },
  { (char_t *) "length", 6, 3, ITEM_LENGTH, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_length },
  { (char_t *) "line", 4, 2, ITEM_LINE, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_line },
  { (char_t *) "lineflag", 8, 8, ITEM_LINEFLAG, 3, 3, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_lineflag },
  { (char_t *) "linend", 6, 5, ITEM_LINEND, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_linend },
  { (char_t *) "lscreen", 7, 2, ITEM_LSCREEN, 6, 6, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_lscreen },
  { (char_t *) "macro", 5, 5, ITEM_MACRO, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_macro },
  { (char_t *) "macroext", 8, 6, ITEM_MACROEXT, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_macroext },
  { (char_t *) "macropath", 9, 6, ITEM_MACROPATH, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_macropath },
  { (char_t *) "margins", 7, 3, ITEM_MARGINS, 3, 3, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_margins },
  { (char_t *) "monitor", 7, 7, ITEM_MONITOR, 2, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_monitor },
  { (char_t *) "mouse", 5, 5, ITEM_MOUSE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_mouse },
  { (char_t *) "msgline", 7, 4, ITEM_MSGLINE, 4, 4, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_msgline },
  { (char_t *) "msgmode", 7, 4, ITEM_MSGMODE, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_msgmode },
  { (char_t *) "nbfile", 6, 3, ITEM_NBFILE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_nbfile },
  { (char_t *) "nbscope", 7, 3, ITEM_NBSCOPE, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_nbscope },
  { (char_t *) "newlines", 8, 4, ITEM_NEWLINES, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_newlines },
  { (char_t *) "nondisp", 7, 4, ITEM_NONDISP, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_nondisp },
  { (char_t *) "number", 6, 3, ITEM_NUMBER, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_number },
  { (char_t *) "pagewrap", 8, 8, ITEM_PAGEWRAP, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_pagewrap },
  { (char_t *) "parser", 6, 6, ITEM_PARSER, 5, 0, LVL_FILE, QUERY_QUERY | QUERY_EXTRACT, extract_parser },
  { (char_t *) "pending", 7, 4, ITEM_PENDING, 7, 1, LVL_VIEW, QUERY_EXTRACT, extract_pending },
  { (char_t *) "point", 5, 1, ITEM_POINT, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_point },
  { (char_t *) "position", 8, 3, ITEM_POSITION, 3, 3, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_position },
  { (char_t *) "prefix", 6, 3, ITEM_PREFIX, 0, 4, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_prefix },
  { (char_t *) "printer", 7, 7, ITEM_PRINTER, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_printer },
  { (char_t *) "profile", 4, 7, ITEM_PROFILE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_profile },
  { (char_t *) "pscreen", 7, 4, ITEM_PSCREEN, 2, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_pscreen },
  { (char_t *) "readonly", 8, 8, ITEM_READONLY, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_readonly },
  { (char_t *) "readv", 5, 5, ITEM_READV, 4, 1, LVL_GLOB, QUERY_READV, extract_readv },
  { (char_t *) "reprofile", 9, 6, ITEM_REPROFILE, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_reprofile },
  { (char_t *) "reserved", 8, 5, ITEM_RESERVED, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_reserved },
  { (char_t *) "rexxhalt", 5, 5, ITEM_REXXHALT, 2, 2, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_rexxhalt },
  { (char_t *) "rexxoutput", 10, 7, ITEM_REXXOUTPUT, 2, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_rexxoutput },
  { (char_t *) "ring", 4, 4, ITEM_RING, 1, 0, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_ring },
  { (char_t *) "scale", 5, 4, ITEM_SCALE, 2, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_scale },
  { (char_t *) "scope", 5, 5, ITEM_SCOPE, 1, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_scope },
  { (char_t *) "screen", 6, 3, ITEM_SCREEN, 2, 2, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_screen },
  { (char_t *) "select", 6, 3, ITEM_SELECT, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_select },
  { (char_t *) "shadow", 6, 4, ITEM_SHADOW, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_shadow },
  { (char_t *) "showkey", 7, 4, ITEM_SHOWKEY, 1, 1, LVL_FILE, QUERY_EXTRACT, extract_showkey },
  { (char_t *) "size", 4, 2, ITEM_SIZE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_size },
  { (char_t *) "slk", 3, 3, ITEM_SLK, 2, 0, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_slk },
  { (char_t *) "statopt", 7, 7, ITEM_STATOPT, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_EXTRACT, extract_statopt },
  { (char_t *) "statusline", 10, 7, ITEM_STATUSLINE, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_statusline },
  { (char_t *) "stay", 4, 4, ITEM_STAY, 1, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_stay },
  { (char_t *) "synelem", 7, 4, ITEM_SYNELEM, 1, 0, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_synelem },
  { (char_t *) "synonym", 7, 3, ITEM_SYNONYM, 1, 0, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_synonym },
  { (char_t *) "tabkey", 6, 4, ITEM_TABKEY, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_tabkey },
  { (char_t *) "tabline", 7, 4, ITEM_TABLINE, 2, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_tabline },
  { (char_t *) "tabs", 4, 4, ITEM_TABS, 1, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_tabs },
  { (char_t *) "tabsin", 6, 5, ITEM_TABSIN, 2, 2, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_tabsin },
  { (char_t *) "tabsout", 7, 5, ITEM_TABSOUT, 2, 2, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_tabsout },
  { (char_t *) "targetsave", 10, 10, ITEM_TARGETSAVE, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_targetsave },
  { (char_t *) "terminal", 8, 4, ITEM_TERMINAL, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_terminal },
  { (char_t *) "thighlight", 5, 5, ITEM_THIGHLIGHT, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_thighlight },
  { (char_t *) "timecheck", 9, 9, ITEM_TIMECHECK, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_timecheck },
  { (char_t *) "tof", 3, 3, ITEM_TOF, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_tof },
  { (char_t *) "tofeof", 6, 6, ITEM_TOFEOF, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_tofeof },
  { (char_t *) "trailing", 8, 8, ITEM_TRAILING, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_trailing },
  { (char_t *) "typeahead", 9, 5, ITEM_TYPEAHEAD, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_typeahead },
  { (char_t *) "ui", 2, 2, ITEM_UI, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_ui },
  { (char_t *) "undoing", 7, 7, ITEM_UNDOING, 1, 1, LVL_FILE, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_undoing },
  { (char_t *) "untaa", 5, 5, ITEM_UNTAA, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_untaa },
  { (char_t *) "utf8", 4, 4, ITEM_UTF8, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_utf8 },
  { (char_t *) "verify", 6, 1, ITEM_VERIFY, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_verify },
  { (char_t *) "vershift", 8, 4, ITEM_VERSHIFT, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_EXTRACT, extract_vershift },
  { (char_t *) "version", 7, 7, ITEM_VERSION, 5, 5, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT, extract_version },
  { (char_t *) "width", 5, 1, ITEM_WIDTH, 1, 1, LVL_GLOB, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_width },
  { (char_t *) "word", 4, 4, ITEM_WORD, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_word },
  { (char_t *) "wordwrap", 8, 5, ITEM_WORDWRAP, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_wordwrap },
  { (char_t *) "wrap", 4, 2, ITEM_WRAP, 1, 1, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_wrap },
  { (char_t *) "zone", 4, 1, ITEM_ZONE, 2, 2, LVL_VIEW, QUERY_QUERY | QUERY_STATUS | QUERY_EXTRACT | QUERY_MODIFY, extract_zone },
};

#define NUMBER_QUERY_ITEM ( sizeof( query_item ) / sizeof( QUERY_ITEM ) )

QUERY_ITEM function_item[] = {
  { (char_t *) "after", 5, 5, ITEM_AFTER_FUNCTION, 0, 1, LVL_VIEW, 0, extract_after_function },
  { (char_t *) "alt", 3, 3, ITEM_ALT_FUNCTION, 0, 1, LVL_FILE, 0, extract_alt_function },
  { (char_t *) "altkey", 6, 6, ITEM_ALTKEY_FUNCTION, 0, 1, LVL_GLOB, 0, extract_altkey_function },
  { (char_t *) "batch", 5, 5, ITEM_BATCH_FUNCTION, 0, 1, LVL_GLOB, 0, extract_batch_function },
  { (char_t *) "before", 6, 6, ITEM_BEFORE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_before_function },
  { (char_t *) "blank", 5, 5, ITEM_BLANK_FUNCTION, 0, 1, LVL_VIEW, 0, extract_blank_function },
  { (char_t *) "block", 5, 5, ITEM_BLOCK_FUNCTION, 0, 1, LVL_VIEW, 0, extract_block_function },
  { (char_t *) "bottomedge", 10, 10, ITEM_BOTTOMEDGE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_bottomedge_function },
  { (char_t *) "command", 7, 7, ITEM_COMMAND_FUNCTION, 0, 1, LVL_VIEW, 0, extract_command_function },
  { (char_t *) "ctrl", 4, 4, ITEM_CTRL_FUNCTION, 0, 1, LVL_GLOB, 0, extract_ctrl_function },
  { (char_t *) "current", 7, 7, ITEM_CURRENT_FUNCTION, 0, 1, LVL_VIEW, 0, extract_current_function },
  { (char_t *) "dir", 3, 3, ITEM_DIR_FUNCTION, 0, 1, LVL_FILE, 0, extract_dir_function },
  { (char_t *) "end", 3, 3, ITEM_END_FUNCTION, 0, 1, LVL_VIEW, 0, extract_end_function },
  { (char_t *) "eof", 3, 3, ITEM_EOF_FUNCTION, 0, 1, LVL_VIEW, 0, extract_eof_function },
  { (char_t *) "first", 5, 5, ITEM_FIRST_FUNCTION, 0, 1, LVL_VIEW, 0, extract_first_function },
  { (char_t *) "focuseof", 8, 8, ITEM_FOCUSEOF_FUNCTION, 0, 1, LVL_VIEW, 0, extract_focuseof_function },
  { (char_t *) "focustof", 8, 8, ITEM_FOCUSTOF_FUNCTION, 0, 1, LVL_VIEW, 0, extract_focustof_function },
  { (char_t *) "inblock", 7, 7, ITEM_INBLOCK_FUNCTION, 0, 1, LVL_VIEW, 0, extract_inblock_function },
  { (char_t *) "incommand", 9, 9, ITEM_INCOMMAND_FUNCTION, 0, 1, LVL_VIEW, 0, extract_incommand_function },
  { (char_t *) "initial", 7, 7, ITEM_INITIAL_FUNCTION, 0, 1, LVL_GLOB, 0, extract_initial_function },
  { (char_t *) "inprefix", 8, 8, ITEM_INPREFIX_FUNCTION, 0, 1, LVL_VIEW, 0, extract_inprefix_function },
  { (char_t *) "insertmode", 10, 10, ITEM_INSERTMODE_FUNCTION, 0, 1, LVL_GLOB, 0, extract_inprefix_function },
  { (char_t *) "leftedge", 8, 8, ITEM_LEFTEDGE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_leftedge_function },
  { (char_t *) "modifiable", 10, 10, ITEM_MODIFIABLE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_modifiable_function },
  { (char_t *) "rightedge", 9, 9, ITEM_RIGHTEDGE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_rightedge_function },
  { (char_t *) "run_os", 6, 6, ITEM_RUN_OS_FUNCTION, 0, 1, LVL_GLOB, 0, NULL },
  { (char_t *) "shadow", 6, 6, ITEM_SHADOW_FUNCTION, 0, 1, LVL_VIEW, 0, extract_shadow_function },
  { (char_t *) "shift", 5, 5, ITEM_SHIFT_FUNCTION, 0, 1, LVL_GLOB, 0, extract_shift_function },
  { (char_t *) "spacechar", 9, 9, ITEM_SPACECHAR_FUNCTION, 0, 1, LVL_VIEW, 0, extract_spacechar_function },
  { (char_t *) "tof", 3, 3, ITEM_TOF_FUNCTION, 0, 1, LVL_VIEW, 0, extract_tof_function },
  { (char_t *) "topedge", 7, 7, ITEM_TOPEDGE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_topedge_function },
  { (char_t *) "valid_target", 12, 12, ITEM_VALID_TARGET_FUNCTION, 0, 1, LVL_VIEW, 0, NULL },
  { (char_t *) "verone", 6, 6, ITEM_VERONE_FUNCTION, 0, 1, LVL_VIEW, 0, extract_verone_function },
};

#define NUMBER_FUNCTION_ITEM ( sizeof( function_item ) / sizeof( QUERY_ITEM ) )

char_t *block_name[] = {
  (char_t *) "",
  (char_t *) "LINE",
  (char_t *) "BOX",
  (char_t *) "STREAM",
  (char_t *) "COLUMN",
  (char_t *) "WORD",
  (char_t *) "CUA"
};

VALUE item_values[MAX_VARIABLES_RETURNED];

THE_PPC *in_range(THE_PPC * found_ppc, THE_PPC * curr_ppc, line_t first_in_range, line_t last_in_range) {
  if (found_ppc == NULL) {
    if (curr_ppc->ppc_line_number >= first_in_range && curr_ppc->ppc_line_number <= last_in_range)
      found_ppc = curr_ppc;
  } else {
    if (curr_ppc->ppc_line_number < found_ppc->ppc_line_number && curr_ppc->ppc_line_number >= first_in_range && curr_ppc->ppc_line_number <= last_in_range)
      found_ppc = curr_ppc;
  }
  return found_ppc;
}

void set_key_values(int key, bool mouse_key) {
  char_t *keyname = NULL;
  int shift = 0;
  int mod_shift, mod_ctrl, mod_alt;

  if (mouse_key) {
    int b, ba, bm, w;
    char_t scrn;

    if (get_mouse_info(&b, &ba, &bm) == RC_OK) {
      which_window_is_mouse_in(&scrn, &w);
      key = mouse_info_to_key(w, b, ba, bm);
      item_values[1].value = mouse_key_number_to_name(key, query_rsrvd, &shift);
      item_values[1].len = strlen((char *) item_values[1].value);
    }
    mod_shift = SHIFT_MOUSE_SHIFT;
    mod_ctrl = SHIFT_MOUSE_CTRL;
    mod_alt = SHIFT_MOUSE_ALT;
  } else {
    keyname = get_key_name(key, &shift);
    if (keyname == NULL) {
      item_values[1].value = (char_t *) "";
      item_values[1].len = 0;
    } else {
      item_values[1].value = keyname;
      item_values[1].len = strlen((char *) keyname);
    }
    mod_shift = SHIFT_SHIFT;
    mod_ctrl = SHIFT_CTRL;
    mod_alt = SHIFT_ALT;
  }
  if (key < 256 && key >= 0) {
    sprintf((char *) query_num1, "%d", key);
    item_values[2].value = query_num1;
    item_values[2].len = strlen((char *) query_num1);
  } else {
    item_values[2].value = (char_t *) "";
    item_values[2].len = 0;
  }
  sprintf((char *) query_num2, "%d", key);
  item_values[3].value = query_num2;
  item_values[3].len = strlen((char *) query_num2);
  memset((char *) query_num3, '0', 8);
  query_num3[8] = '\0';
  if (key != -1) {
    if (INSERTMODEx)
      query_num3[0] = '1';
    if (shift & mod_alt)
      query_num3[4] = '1';
    if (shift & mod_ctrl)
      query_num3[5] = '1';
    if (shift & mod_shift)
      query_num3[6] = query_num3[7] = '1';
  }
  item_values[4].value = query_num3;
  item_values[4].len = 8;
  return;
}

short set_boolean_value(bool flag, short num) {
  if (flag) {
    item_values[num].value = (char_t *) "1";
    item_values[num].len = 1;
  } else {
    item_values[num].value = (char_t *) "0";
    item_values[num].len = 1;
  }
  return 1;
}

short set_on_off_value(bool flag, short num) {
  if (flag) {
    item_values[num].value = (char_t *) "ON";
    item_values[num].len = 2;
  } else {
    item_values[num].value = (char_t *) "OFF";
    item_values[num].len = 3;
  }
  return 1;
}

short find_query_item(char* item_name, int len, char* query_type) {
  int rc = 0;

  /*
   * Using the supplied abbreviation for an item, find the entry in the
   * query_item[] array. It should return with the index pointer (for
   * QUERY_EXTRACT) or an index to the first item in the function_item[]
   * rexx.c for other query types.
   */
  set_compare_exact(FALSE);
  rc = search_query_item_array(query_item, number_query_item(), sizeof(QUERY_ITEM), (char *) item_name, len);
  if (rc != (-1))
    *query_type = query_item[rc].query;
  return (rc);
}
short show_status(void) {
#define STATUS_COLS 6
  register short i = 0, j = 0, k = 0;
  short lineno = 0, colno = 0;
  short number_variables = 0;
  short item_width = 0, column = 0, column_width = 0, col[STATUS_COLS];

  /*
   * For each item that is displayable, display it...
   */
  column = 0;
  column_width = COLS / STATUS_COLS;
  col[0] = 0;
  for (i = 1; i < STATUS_COLS; i++)
    col[i] = col[i - 1] + column_width + 1;
  wclear(stdscr);
  for (i = 0; i < NUMBER_QUERY_ITEM; i++) {
    /*
     * Get only those settings that are queryable...
     */
    if (query_item[i].query & QUERY_STATUS) {
      number_variables = get_item_values(1, query_item[i].item_number, (char_t *) "", QUERY_STATUS, 0L, NULL, 0L);
      item_width = 0;
      /*
       * Obtain the total length of the setting values...
       */
      for (j = 0; j < number_variables + 1; j++)
        item_width += item_values[j].len + 1;
      item_width--;             /* reduce by 1 for last value's blank at end */
      /*
       * If the length of the variables is > the screen width, go to next
       * line.
       */
      if (item_width + col[column] > COLS) {
        column = colno = 0;
        lineno++;
      } else
        colno = col[column];
      /*
       * Display the variables. For the first value, display in BOLD.
       */
      for (j = 0; j < number_variables + 1; j++) {
        if (j == 0)
          attrset(A_BOLD);
        mvaddstr(lineno, colno, (char *) item_values[j].value);
        attrset(A_NORMAL);
        colno += item_values[j].len + 1;
      }
      colno--;
      /*
       * Now have to determine where to display the next set of variables...
       * If the just-displayed values go past the last column, or we are
       * already in the last column, go to the next line...
       */
      if (colno >= col[STATUS_COLS - 1] || column == STATUS_COLS - 1) {
        lineno++;
        column = 0;
      } else {
        /*
         * ...else if the the just-displayed values go past the 2nd column...
         * If the just-displayed values go past the last column, go on to the
         * next line...
         */
        for (k = 0; k < STATUS_COLS; k++) {
          if (colno < col[k]) {
            column = k;
            break;
          }
        }
      }
    }
  }
  mvaddstr(terminal_lines - 2, 0, HIT_ANY_KEY);
  refresh();

  return (RC_OK);
}
short save_status(char* filename) {
  register short i = 0, j = 0;
  short number_variables = 0, rc = RC_OK;
  FILE *fp = NULL;

  /*
   * Get the fully qualified filename from the supplied filename.
   */
  strrmdup(strtrans(filename, OSLASH, ISLASH), ISLASH, TRUE);
  if ((rc = splitpath(filename)) != RC_OK) {
    display_error(10, filename, FALSE);
    return (rc);
  }
  /*
   * splitpath() has set up sp_path  and sp_fname. Append the filename.
   */
  strcat((char *) sp_path, (char *) sp_fname);
  /*
   * If the file already exists, display an error.
   */
  if (file_exists(sp_path) == THE_FILE_EXISTS) {
    display_error(8, filename, FALSE);
    return (rc);
  }
  fp = fopen((char *) sp_path, "w");
  /*
   * For each item that is modifiable, get its value...
   */
  fprintf(fp, "/* This file generated by STATUS command of THE %s %s*/\n", the_version, the_release);
  for (i = 0; i < NUMBER_QUERY_ITEM; i++) {
    /*
     * Get only those settings that are modifiable...
     */
    if (query_item[i].query & QUERY_MODIFY) {
      number_variables = get_item_values(1, query_item[i].item_number, (char_t *) "", QUERY_MODIFY, 0L, NULL, 0L);
      fputs("'set", fp);
      /*
       * Write the variables to the file...
       */
      for (j = 0; j < number_variables + 1; j++) {
        fputc(' ', fp);
        fputs((char *) item_values[j].value, fp);
      }
      fprintf(fp, "'\n");
    }
  }
  fflush(fp);
  fclose(fp);
  return (RC_OK);
}
short set_extract_variables(short itemno) {
  register short i = 0;
  short rc = RC_OK, number_values = 0;

  number_values = atoi((char *) item_values[0].value);
  for (i = 0; i < number_values + 1; i++) {
    if ((rc = set_rexx_variable(query_item[itemno].name, item_values[i].value, item_values[i].len, i)) != RC_OK)
      break;
  }
  return (rc);
}

short get_number_dynamic_items(int qitem) {
  int number_variables = 0;
  PARSER_MAPPING *mapping = NULL;
  PARSER_DETAILS *details = NULL;
  DEFINE *synonym = NULL;

  switch (qitem) {
    case ITEM_RING:
      number_variables = number_of_files;
      break;
    case ITEM_PARSER:
      for (details = first_parser; details != NULL; details = details->next, number_variables++);
      break;
    case ITEM_AUTOCOLOR:
    case ITEM_AUTOCOLOUR:
      for (mapping = first_parser_mapping; mapping != NULL; mapping = mapping->next, number_variables++);
      break;
    case ITEM_SYNONYM:
      for (synonym = first_synonym; synonym != NULL; synonym = synonym->next, number_variables++);
    default:
      break;
  }
  return number_variables;
}

short get_item_values(int qitem, short itemno, char* itemargs, char query_type, line_t argc, char* arg, line_t arglen) {
  short number_variables = 1;

  /*
   * Only use itemno to reference query_item[] array if it is NOT a
   * boolean function...
   */
  if (qitem) {
    number_variables = (query_item[itemno].ext_func) (query_item[itemno].number_values, itemno, itemargs, query_type, argc, arg, arglen);
  } else {
    number_variables = (function_item[itemno].ext_func) (1, itemno, itemargs, query_type, argc, arg, arglen);
  }
  /*
   * If an "error" condition exists, do not set any values.
   */
  if (number_variables >= 0) {
    if (number_variables == 0) {
      /*
       * The number of variables needs to be set dynamically for the
       * following items. Any item that has a value of 0 for item_values
       * column of query_item, needs to be catered for here.
       */
      number_variables = get_number_dynamic_items(query_item[itemno].item_number);
    }
    switch (query_type) {
      case QUERY_EXTRACT:
      case QUERY_FUNCTION:
      case QUERY_READV:
        sprintf((char *) query_num0, "%d", number_variables);
        item_values[0].value = query_num0;
        item_values[0].len = strlen((char *) query_num0);
        break;
      case QUERY_STATUS:
      case QUERY_QUERY:
      case QUERY_MODIFY:
        item_values[0].value = query_item[itemno].name;
        item_values[0].len = strlen((char *) query_item[itemno].name);
        break;
      default:
        break;
    }
  }
  return (number_variables);
}
short extract_point_settings(short itemno, char_t * params) {
  register short i = 0;
  short number_variables = query_item[itemno].number_values;
  char_t num4[15];
  LINE *curr = NULL;
  line_t true_line = 0L;
  short rc = RC_OK;
  THELIST *curr_name;
  int len, total_len = 0;

  if (strcmp((char *) params, "") == 0) {    /* get name for focus line only */
    true_line = (compatible_feel == COMPAT_XEDIT) ? CURRENT_VIEW->current_line : get_true_line(TRUE);
    curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines);
    if (curr->first_name == NULL)       /* line not named */
      number_variables = 0;
    else {
      total_len = sprintf((char *) query_rsrvd, "%ld", true_line);
      curr_name = curr->first_name;
      while (curr_name) {
        len = strlen((char *) curr_name->data);
        if (total_len + len + 1 > sizeof(query_rsrvd)) {
          break;
        }
        strcat((char *) query_rsrvd, " ");
        strcat((char *) query_rsrvd, (char *) curr_name->data);
        total_len += len + 1;
        curr_name = curr_name->next;
      }
      item_values[1].value = query_rsrvd;
      item_values[1].len = total_len;
      number_variables = 1;
    }
  } else {
    if (strcmp((char *) params, "*") != 0) { /* argument must be "*" */
      display_error(1, (char_t *) params, FALSE);
      number_variables = EXTRACT_ARG_ERROR;
    } else {
      curr = CURRENT_FILE->first_line;
      for (true_line = 0, i = 0; curr != NULL; true_line++) {
        if (curr->first_name != NULL) { /* line is named */
          total_len = sprintf((char *) query_rsrvd, "%ld", true_line);
          curr_name = curr->first_name;
          while (curr_name) {
            len = strlen((char *) curr_name->data);
            if (total_len + len + 1 > sizeof(query_rsrvd)) {
              break;
            }
            strcat((char *) query_rsrvd, " ");
            strcat((char *) query_rsrvd, (char *) curr_name->data);
            total_len += len + 1;
            curr_name = curr_name->next;
          }
          rc = set_rexx_variable(query_item[itemno].name, query_rsrvd, total_len, ++i);
          if (rc == RC_SYSTEM_ERROR) {
            display_error(54, (char_t *) "", FALSE);
            return (EXTRACT_ARG_ERROR);
          }
        }
        curr = curr->next;
      }
      sprintf((char *) num4, "%d", i);
      rc = set_rexx_variable(query_item[itemno].name, num4, strlen((char *) num4), 0);
      if (rc == RC_SYSTEM_ERROR) {
        display_error(54, (char_t *) "", FALSE);
        number_variables = EXTRACT_ARG_ERROR;
      } else
        number_variables = EXTRACT_VARIABLES_SET;
    }
  }
  return (number_variables);
}

short extract_prefix_settings(short itemno, char_t * params, char_t query_type) {
  register short i = 0;

#define PRE_PARAMS  3
  char_t *word[PRE_PARAMS + 1];
  char_t strip[PRE_PARAMS];
  unsigned short num_params = 0;
  short number_variables = query_item[itemno].number_values;
  char_t num4[15];
  LINE *curr = NULL;
  short rc = RC_OK;
  char_t *tmpbuf = NULL;

  strip[0] = STRIP_BOTH;
  strip[1] = STRIP_BOTH;
  strip[2] = STRIP_BOTH;
  num_params = param_split(params, word, PRE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE);
  switch (num_params) {
      /*
       * None or 1 parameters, error.
       */
    case 0:
    case 1:
      display_error(3, (char_t *) "", FALSE);
      number_variables = EXTRACT_ARG_ERROR;
      break;
      /*
       * 2  parameters, Synonym and name (or *).
       */
    case 2:
      if (!equal((char_t *) "synonym", word[0], 1)) {
        display_error(13, (char_t *) "", FALSE);
        number_variables = EXTRACT_ARG_ERROR;
        break;
      }
      break;
    default:
      /*
       * Too many parameters, error.
       */
      display_error(2, (char_t *) "", FALSE);
      number_variables = EXTRACT_ARG_ERROR;
      break;
  }
  /*
   * If the validation of parameters is successful...
   */
  if (number_variables >= 0) {
    if (strcmp((char *) word[1], "*") == 0) {
      /*
       * Get oldname for all synonyms...
       */
      curr = first_prefix_synonym;
      i = 0;
      while (curr != NULL) {
        tmpbuf = (char_t *) malloc(sizeof(char_t) * (strlen((char *) curr->name) + strlen((char *) curr->line) + 2));
        if (tmpbuf == (char_t *) NULL) {
          display_error(30, (char_t *) "", FALSE);
          return (EXTRACT_ARG_ERROR);
        }
        strcpy((char *) tmpbuf, (char *) curr->name);
        strcat((char *) tmpbuf, " ");
        strcat((char *) tmpbuf, (char *) curr->line);
        if (query_type == QUERY_EXTRACT)
          rc = set_rexx_variable(query_item[itemno].name, tmpbuf, strlen((char *) tmpbuf), ++i);
        else {
          display_error(0, tmpbuf, FALSE);
          rc = RC_OK;
        }
        free(tmpbuf);
        if (rc == RC_SYSTEM_ERROR) {
          display_error(54, (char_t *) "", FALSE);
          return (EXTRACT_ARG_ERROR);
        }
        curr = curr->next;
      }
      if (query_type == QUERY_EXTRACT) {
        sprintf((char *) num4, "%d", i);
        rc = set_rexx_variable(query_item[itemno].name, num4, strlen((char *) num4), 0);
        if (rc == RC_SYSTEM_ERROR) {
          display_error(54, (char_t *) "", FALSE);
          number_variables = EXTRACT_ARG_ERROR;
        } else
          number_variables = EXTRACT_VARIABLES_SET;
      } else {
        number_variables = EXTRACT_VARIABLES_SET;
      }
    } else {
      /*
       * Get oldname for named synonym...
       */
      item_values[1].value = find_prefix_synonym(word[1]);
      item_values[1].len = strlen((char *) item_values[1].value);
      number_variables = 1;
    }
  }
  return (number_variables);
}
void get_etmode(char_t * onoff, char_t * list) {
  bool on_flag = FALSE;
  bool off_flag = FALSE;
  bool last_state = FALSE;
  char tmp[15];
  register int i = 0;

  strcpy((char *) list, "");
  last_state = TRUE;
  for (i = 0; i < 256; i++) {
    if (etmode_flag[i] == TRUE)
      off_flag = TRUE;
    else
      on_flag = TRUE;
    if (last_state != etmode_flag[i]) {
      if (last_state == FALSE) {
        sprintf(tmp, "%d", i - 1);
        strcat((char *) list, tmp);
      } else {
        sprintf(tmp, " %d-", i);
        strcat((char *) list, tmp);
      }
      last_state = etmode_flag[i];
    }
  }
  if (*(list + strlen((char *) list) - 1) == '-') {
    sprintf(tmp, "%d", 255);
    strcat((char *) list, tmp);
  }
  if (on_flag) {
    strcpy((char *) onoff, "ON");
    if (!off_flag)
      strcpy((char *) list, "");
  } else {
    strcpy((char *) onoff, "OFF");
    strcpy((char *) list, "");
  }
  return;
}
short extract_colour_settings(short itemno, char_t * buffer, char_t query_type, char_t * params, bool us, bool isecolour) {
  short rc = RC_OK;
  register int i = 0, maxnum;
  int start = 0, end = 0, number_variables = 0, off = 0;
  bool found = FALSE;
  char_t *attr_string = NULL, tmparea[2];
  row_t save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
  char_t *ptr, *area, *colour = (char_t *) "colour ", *color = (char_t *) "color ", *ecolour = (char_t *) "ecolour ", *ecolor = (char_t *) "ecolor ";
  COLOUR_ATTR *attr;

  tmparea[1] = '\0';
  maxnum = (isecolour) ? ECOLOUR_MAX : ATTR_MAX;
  attr = (isecolour) ? CURRENT_FILE->ecolour : CURRENT_FILE->attr;
  if (blank_field(params)
      || strcmp((char *) params, "*") == 0) {
    start = 0;
    end = maxnum;
  } else {
    if (isecolour) {
      if (strlen((char *) params) != 1) {
        return (EXTRACT_ARG_ERROR);
      }
      if (*params >= 'A' && *params <= 'Z')
        i = *params - 'A';
      else if (*params >= 'a' && *params <= 'z')
        i = *params - 'a';
      else if (*params >= '1' && *params <= '9')
        i = *params - '1' + 26;
      else {
        return (EXTRACT_ARG_ERROR);
      }
      found = TRUE;
    } else {
      for (i = 0; i < maxnum; i++) {
        if (equal(valid_areas[i].area, params, valid_areas[i].area_min_len)) {
          found = TRUE;
          break;
        }
      }
    }
    if (!found) {
      display_error(1, params, FALSE);
      return (EXTRACT_ARG_ERROR);
    }
    start = i;
    end = i + 1;
  }
  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, end - start);
    CURRENT_VIEW->msgmode_status = TRUE;
  }

  for (i = start; i < end; i++) {
    attr_string = get_colour_strings(attr + i);
    if (attr_string == (char_t *) NULL) {
      return (EXTRACT_ARG_ERROR);
    }
    if (us) {
      if (isecolour)
        ptr = ecolor;
      else
        ptr = color;
    } else {
      if (isecolour)
        ptr = ecolour;
      else
        ptr = colour;
    }
    if (isecolour) {
      if (i > 25)
        tmparea[0] = (char_t) (i - 25) + '0';
      else
        tmparea[0] = (char_t) (i + 'A');
      area = tmparea;
    } else {
      area = valid_areas[i].area;
    }
    sprintf((char *) query_rsrvd, "%s%s %s", (query_type == QUERY_QUERY) ? (char *) ptr : "", area, attr_string);
    free(attr_string);

    if (query_type == QUERY_QUERY)
      display_error(0, query_rsrvd, TRUE);
    else {
      number_variables++;
      item_values[number_variables].len = strlen((char *) query_rsrvd);
      memcpy((char *) trec + off, (char *) query_rsrvd, (item_values[number_variables].len) + 1);
      item_values[number_variables].value = trec + off;
      off += (item_values[number_variables].len) + 1;
    }
  }
  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = save_msgline_rows;
    CURRENT_VIEW->msgmode_status = save_msgmode_status;
    rc = EXTRACT_VARIABLES_SET;
  } else
    rc = number_variables;
  return (rc);
}
short extract_autocolour_settings(short itemno, char_t * buffer, char_t query_type, char_t * params, bool us) {
  short rc = RC_OK;
  register int i = 0;
  int number_variables = 0, off = 0;
  bool found = FALSE;
  row_t save_msgline_rows = CURRENT_VIEW->msgline_rows;
  bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
  char_t *ptr, *ptr_mask = NULL, *ptr_magic = NULL, *ptr_parser = NULL;
  PARSER_MAPPING *curr;

  ptr = (us) ? (char_t *) "autocolor " : (char_t *) "autocolour ";
  if (blank_field(params)
      || strcmp((char *) params, "*") == 0) {
    if (query_type == QUERY_QUERY) {
      for (i = 0, curr = first_parser_mapping; curr != NULL; curr = curr->next, i++);
      CURRENT_VIEW->msgline_rows = min(terminal_lines - 1, i);
      CURRENT_VIEW->msgmode_status = TRUE;
    }
    for (curr = first_parser_mapping; curr != NULL; curr = curr->next) {
      sprintf((char *) query_rsrvd, "%s%s%s %s%s", (query_type == QUERY_QUERY) ? (char *) ptr : "", (curr->filemask) ? (char *) curr->filemask : "", (curr->magic_number) ? (char *) curr->magic_number : "", (char *) curr->parser->parser_name,
              (curr->magic_number) ? " MAGIC" : "");

      if (query_type == QUERY_QUERY)
        display_error(0, query_rsrvd, TRUE);
      else {
        number_variables++;
        item_values[number_variables].len = strlen((char *) query_rsrvd);
        memcpy((char *) trec + off, (char *) query_rsrvd, (item_values[number_variables].len) + 1);
        item_values[number_variables].value = trec + off;
        off += (item_values[number_variables].len) + 1;
      }
    }
  } else {
    if (query_type == QUERY_QUERY) {
      CURRENT_VIEW->msgline_rows = 1;
      CURRENT_VIEW->msgmode_status = TRUE;
    }
    /*
     * Find a match for the supplied mask or magic number
     */
    for (curr = first_parser_mapping; curr != NULL; curr = curr->next) {
      if (curr->filemask && strcmp((char *) params, (char *) curr->filemask) == 0) {
        ptr_mask = curr->filemask;
        ptr_magic = (char_t *) "";
        found = TRUE;
        break;
      }
      if (curr->magic_number && strcmp((char *) params, (char *) curr->magic_number) == 0) {
        ptr_mask = curr->magic_number;
        ptr_magic = (char_t *) "MAGIC";
        found = TRUE;
        break;
      }
    }
    if (found)
      ptr_parser = (char_t *) curr->parser->parser_name;
    else {
      ptr_mask = params;
      ptr_magic = (char_t *) "";
      ptr_parser = (char_t *) "NULL";
    }
    if (query_type == QUERY_QUERY) {
      sprintf((char *) query_rsrvd, "%s%s %s%s", (query_type == QUERY_QUERY) ? (char *) ptr : "", ptr_mask, ptr_parser, ptr_magic);
      display_error(0, query_rsrvd, TRUE);
    } else {
      item_values[1].value = ptr_mask;
      item_values[1].len = strlen((char *) ptr_mask);
      item_values[2].value = ptr_parser;
      item_values[2].len = strlen((char *) ptr_parser);
      item_values[3].value = ptr_magic;
      item_values[3].len = strlen((char *) ptr_magic);
      number_variables = 3;
    }
  }

  if (query_type == QUERY_QUERY) {
    CURRENT_VIEW->msgline_rows = save_msgline_rows;
    CURRENT_VIEW->msgmode_status = save_msgmode_status;
    rc = EXTRACT_VARIABLES_SET;
  } else
    rc = number_variables;

  return (rc);
}

int number_function_item(void) {
  return NUMBER_FUNCTION_ITEM;
}

int number_query_item(void) {
  return NUMBER_QUERY_ITEM;
}

void format_options(char_t * buf) {
  LINE *curr = first_option;
  int tail, col, itemno, linelen, valuelen, number_variables, max_col = 0, off;

  while (curr != NULL) {
    itemno = curr->length;
    tail = curr->flags.new_flag;
    number_variables = get_item_values(1, query_item[itemno].item_number, (char_t *) "", QUERY_EXTRACT, 0L, NULL, 0L);
    if (number_variables >= tail) {
      col = curr->select;
      linelen = curr->line ? strlen((char *) curr->line) : 0;
      valuelen = (curr->save_select) ? min(curr->save_select, item_values[tail].len) : item_values[tail].len;
      if (curr->line) {
        memcpy(buf + col, (char *) curr->line, linelen);
      }
      memcpy(buf + col + linelen, (char *) item_values[tail].value, valuelen);
      off = col + linelen + valuelen;
      if (off > max_col)
        max_col = off;
    }
    curr = curr->next;
  }
  buf[max_col] = '\0';
  return;
}
