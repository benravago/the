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

short extract_point_settings(short,CHARTYPE *);
short extract_prefix_settings(short,CHARTYPE *,CHARTYPE);
short extract_colour_settings(short,CHARTYPE *,CHARTYPE,CHARTYPE *,bool,bool);
short extract_autocolour_settings(short,CHARTYPE *,CHARTYPE,CHARTYPE *,bool);
void get_etmode(CHARTYPE *,CHARTYPE *);

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
extern ExtractFunction extract_dirinclude;
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
extern ExtractFunction extract_rexx;
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
extern ExtractFunction extract_xterminal;
extern ExtractFunction extract_zone;

CHARTYPE query_num0[10]; /* DO NOT USE THIS FOR DATA !! */
CHARTYPE query_num1[10];
CHARTYPE query_num2[10];
CHARTYPE query_num3[40];
CHARTYPE query_num4[40];
CHARTYPE query_num5[10];
CHARTYPE query_num6[10];
CHARTYPE query_num7[10];
CHARTYPE query_num8[10];
CHARTYPE query_rsrvd[MAX_FILE_NAME+1];



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
QUERY_ITEM _THE_FAR query_item[] =
{
   {(CHARTYPE *)"alt",             3, 3,ITEM_ALT,           2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_alt                    },
   {(CHARTYPE *)"arbchar",         7, 3,ITEM_ARBCHAR,       3, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_arbchar                },
   {(CHARTYPE *)"autocolor",       9, 9,ITEM_AUTOCOLOR,     3, 0,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_autocolor              },
   {(CHARTYPE *)"autocolour",     10,10,ITEM_AUTOCOLOUR,    3, 0,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_autocolour             },
   {(CHARTYPE *)"autosave",        8, 2,ITEM_AUTOSAVE,      1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_autosave               },
   {(CHARTYPE *)"autoscroll",     10, 6,ITEM_AUTOSCROLL,    1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_autoscroll             },
   {(CHARTYPE *)"backup",          6, 4,ITEM_BACKUP,        2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_backup                 },
   {(CHARTYPE *)"beep",            4, 4,ITEM_BEEP,          1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_beep                   },
   {(CHARTYPE *)"block",           5, 5,ITEM_BLOCK,         0, 6,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_block                  },
   {(CHARTYPE *)"case",            4, 4,ITEM_CASE,          6, 6,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_case                   },
   {(CHARTYPE *)"clearerrorkey",  13, 6,ITEM_CLEARERRORKEY, 1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_clearerrorkey          },
   {(CHARTYPE *)"clearscreen",    11, 6,ITEM_CLEARSCREEN,   1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_clearscreen            },
   {(CHARTYPE *)"clock",           5, 5,ITEM_CLOCK,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_clock                  },
   {(CHARTYPE *)"cmdarrows",       9, 4,ITEM_CMDARROWS,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_cmdarrows              },
   {(CHARTYPE *)"cmdline",         7, 3,ITEM_CMDLINE,       2, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_cmdline                },
   {(CHARTYPE *)"color",           5, 5,ITEM_COLOR,         1, ATTR_MAX,   LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_color                  },
   {(CHARTYPE *)"coloring",        8, 8,ITEM_COLORING,      3, 3,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_coloring               },
   {(CHARTYPE *)"colour",          6, 5,ITEM_COLOUR,        1, ATTR_MAX,   LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_colour                 },
   {(CHARTYPE *)"colouring",       9, 9,ITEM_COLOURING,     3, 3,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT|QUERY_MODIFY,extract_colouring              },
   {(CHARTYPE *)"column",          6, 3,ITEM_COLUMN,        1, 1,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_column                 },
   {(CHARTYPE *)"compat",          6, 6,ITEM_COMPAT,        3, 3,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_compat                 },
   {(CHARTYPE *)"ctlchar",         7, 3,ITEM_CTLCHAR,       3, 3,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_ctlchar                },
   {(CHARTYPE *)"curline",         7, 3,ITEM_CURLINE,       6, 6,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_curline                },
   {(CHARTYPE *)"cursor",          6, 4,ITEM_CURSOR,        8, 8,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_cursor                 },
   {(CHARTYPE *)"cursorstay",     10, 8,ITEM_CURSORSTAY,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_cursorstay             },
   {(CHARTYPE *)"define",          6, 6,ITEM_DEFINE,        1, 0,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_define                 },
   {(CHARTYPE *)"defsort",         7, 7,ITEM_DEFSORT,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_defsort                },
   {(CHARTYPE *)"dirfileid",       9, 9,ITEM_DIRFILEID,     2, 2,          LVL_FILE,                         QUERY_EXTRACT             ,extract_dirfileid              },
   {(CHARTYPE *)"dirinclude",     10, 4,ITEM_DIRINCLUDE,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_dirinclude             },
   {(CHARTYPE *)"display",         7, 4,ITEM_DISPLAY,       2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_display                },
   {(CHARTYPE *)"ecolor",          6, 6,ITEM_ECOLOR,        2, ECOLOUR_MAX,LVL_VIEW,QUERY_QUERY|             QUERY_EXTRACT             ,extract_ecolor                 },
   {(CHARTYPE *)"ecolour",         7, 7,ITEM_ECOLOUR,       2, ECOLOUR_MAX,LVL_VIEW,QUERY_QUERY|             QUERY_EXTRACT             ,extract_ecolour                },
   {(CHARTYPE *)"efileid",         7, 6,ITEM_EFILEID,       2, 2,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT             ,extract_efileid                },
   {(CHARTYPE *)"eof",             3, 3,ITEM_EOF,           1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_eof                    },
   {(CHARTYPE *)"eolout",          6, 3,ITEM_EOLOUT,        1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_eolout                 },
   {(CHARTYPE *)"equivchar",       9, 6,ITEM_EQUIVCHAR,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_equivchar              },
   {(CHARTYPE *)"errorformat",    11, 6,ITEM_ERRORFORMAT,   1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_errorformat            },
   {(CHARTYPE *)"erroroutput",    11, 8,ITEM_ERROROUTPUT,   1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_erroroutput            },
   {(CHARTYPE *)"etmode",          6, 6,ITEM_ETMODE,        2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_etmode                 },
   {(CHARTYPE *)"fext",            4, 2,ITEM_FEXT,          1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_ftype                  },
   {(CHARTYPE *)"field",           5, 5,ITEM_FIELD,         4, 4,          LVL_VIEW,QUERY_QUERY|             QUERY_EXTRACT             ,extract_field                  },
   {(CHARTYPE *)"fieldword",       9, 9,ITEM_FIELDWORD,     3, 3,          LVL_VIEW,                         QUERY_EXTRACT             ,extract_fieldword              },
   {(CHARTYPE *)"filename",        8, 5,ITEM_FILENAME,      1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_filename               },
   {(CHARTYPE *)"filestatus",     10,10,ITEM_FILESTATUS,    3, 3,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_filestatus             },
   {(CHARTYPE *)"filetabs",        8, 8,ITEM_FILETABS,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_filetabs               },
   {(CHARTYPE *)"fmode",           5, 2,ITEM_FMODE,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_fmode                  },
   {(CHARTYPE *)"fname",           5, 2,ITEM_FNAME,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_fname                  },
   {(CHARTYPE *)"fpath",           5, 2,ITEM_FPATH,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_fpath                  },
   {(CHARTYPE *)"ftype",           5, 2,ITEM_FTYPE,         1, 1,          LVL_FILE,QUERY_QUERY|             QUERY_EXTRACT|QUERY_MODIFY,extract_ftype                  },
   {(CHARTYPE *)"fullfname",       9, 6,ITEM_FULLFNAME,     1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_fullfname              },
   {(CHARTYPE *)"getenv",          6, 6,ITEM_GETENV,        1, 1,          LVL_GLOB,                         QUERY_EXTRACT             ,extract_getenv                 },
   {(CHARTYPE *)"header",          4, 3,ITEM_HEADER,       11,11,          LVL_GLOB,QUERY_QUERY|             QUERY_EXTRACT             ,extract_header                 },
   {(CHARTYPE *)"hex",             3, 3,ITEM_HEX,           1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_hex                    },
   {(CHARTYPE *)"hexdisplay",     10, 7,ITEM_HEXDISPLAY,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_hexdisplay             },
   {(CHARTYPE *)"hexshow",         7, 4,ITEM_HEXSHOW,       2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_hexshow                },
   {(CHARTYPE *)"highlight",       9, 4,ITEM_HIGHLIGHT,     1, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_highlight              },
   {(CHARTYPE *)"idline",          6, 2,ITEM_IDLINE,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_idline                 },
   {(CHARTYPE *)"impmacro",        8, 6,ITEM_IMPMACRO,      1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_impmacro               },
   {(CHARTYPE *)"impos",           5, 5,ITEM_IMPOS,         1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_impos                  },
   {(CHARTYPE *)"inputmode",       9, 6,ITEM_INPUTMODE,     1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_inputmode              },
   {(CHARTYPE *)"insertmode",     10, 6,ITEM_INSERTMODE,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_insertmode             },
   {(CHARTYPE *)"interface",       9, 3,ITEM_INTERFACE,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_interface              },
   {(CHARTYPE *)"lastkey",         7, 7,ITEM_LASTKEY,       4, 4,          LVL_GLOB,                         QUERY_EXTRACT             ,extract_lastkey                },
   {(CHARTYPE *)"lastmsg",         7, 4,ITEM_LASTMSG,       1, 1,          LVL_GLOB,                         QUERY_EXTRACT             ,extract_lastmsg                },
   {(CHARTYPE *)"lastop",          6, 6,ITEM_LASTOP,        9, 9,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_lastop                 },
   {(CHARTYPE *)"lastrc",          6, 6,ITEM_LASTRC,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_lastrc                 },
   {(CHARTYPE *)"length",          6, 3,ITEM_LENGTH,        1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_length                 },
   {(CHARTYPE *)"line",            4, 2,ITEM_LINE,          1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_line                   },
   {(CHARTYPE *)"lineflag",        8, 8,ITEM_LINEFLAG,      3, 3,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_lineflag               },
   {(CHARTYPE *)"linend",          6, 5,ITEM_LINEND,        2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_linend                 },
   {(CHARTYPE *)"lscreen",         7, 2,ITEM_LSCREEN,       6, 6,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_lscreen                },
   {(CHARTYPE *)"macro",           5, 5,ITEM_MACRO,         1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_macro                  },
   {(CHARTYPE *)"macroext",        8, 6,ITEM_MACROEXT,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_macroext               },
   {(CHARTYPE *)"macropath",       9, 6,ITEM_MACROPATH,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_macropath              },
   {(CHARTYPE *)"margins",         7, 3,ITEM_MARGINS,       3, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_margins                },
   {(CHARTYPE *)"monitor",         7, 7,ITEM_MONITOR,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_monitor                },
   {(CHARTYPE *)"mouse",           5, 5,ITEM_MOUSE,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_mouse                  },
   {(CHARTYPE *)"msgline",         7, 4,ITEM_MSGLINE,       4, 4,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_msgline                },
   {(CHARTYPE *)"msgmode",         7, 4,ITEM_MSGMODE,       2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_msgmode                },
   {(CHARTYPE *)"nbfile",          6, 3,ITEM_NBFILE,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_nbfile                 },
   {(CHARTYPE *)"nbscope",         7, 3,ITEM_NBSCOPE,       2, 2,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_nbscope                },
   {(CHARTYPE *)"newlines",        8, 4,ITEM_NEWLINES,      1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_newlines               },
   {(CHARTYPE *)"nondisp",         7, 4,ITEM_NONDISP,       1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_nondisp                },
   {(CHARTYPE *)"number",          6, 3,ITEM_NUMBER,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_number                 },
   {(CHARTYPE *)"pagewrap",        8, 8,ITEM_PAGEWRAP,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_pagewrap               },
   {(CHARTYPE *)"parser",          6, 6,ITEM_PARSER,        5, 0,          LVL_FILE,QUERY_QUERY             |QUERY_EXTRACT             ,extract_parser                 },
   {(CHARTYPE *)"pending",         7, 4,ITEM_PENDING,       7, 1,          LVL_VIEW,                         QUERY_EXTRACT             ,extract_pending                },
   {(CHARTYPE *)"point",           5, 1,ITEM_POINT,         1, 1,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_point                  },
   {(CHARTYPE *)"position",        8, 3,ITEM_POSITION,      3, 3,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_position               },
   {(CHARTYPE *)"prefix",          6, 3,ITEM_PREFIX,        0, 4,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_prefix                 },
   {(CHARTYPE *)"printer",         7, 7,ITEM_PRINTER,       1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_printer                },
   {(CHARTYPE *)"profile",         4, 7,ITEM_PROFILE,       1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_profile                },
   {(CHARTYPE *)"pscreen",         7, 4,ITEM_PSCREEN,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_pscreen                },
   {(CHARTYPE *)"readonly",        8, 8,ITEM_READONLY,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_readonly               },
   {(CHARTYPE *)"readv",           5, 5,ITEM_READV,         4, 1,          LVL_GLOB,QUERY_READV                                        ,extract_readv                  },
   {(CHARTYPE *)"reprofile",       9, 6,ITEM_REPROFILE,     1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_reprofile              },
   {(CHARTYPE *)"reserved",        8, 5,ITEM_RESERVED,      1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_reserved               },
   {(CHARTYPE *)"rexx",            4, 4,ITEM_REXX,          1, 1,          LVL_GLOB,QUERY_QUERY                                        ,extract_rexx                   },
   {(CHARTYPE *)"rexxhalt",        5, 5,ITEM_REXXHALT,      2, 2,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_rexxhalt               },
   {(CHARTYPE *)"rexxoutput",     10, 7,ITEM_REXXOUTPUT,    2, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_rexxoutput             },
   {(CHARTYPE *)"ring",            4, 4,ITEM_RING,          1, 0,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_ring                   },
   {(CHARTYPE *)"scale",           5, 4,ITEM_SCALE,         2, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_scale                  },
   {(CHARTYPE *)"scope",           5, 5,ITEM_SCOPE,         1, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_scope                  },
   {(CHARTYPE *)"screen",          6, 3,ITEM_SCREEN,        2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_screen                 },
   {(CHARTYPE *)"select",          6, 3,ITEM_SELECT,        2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_select                 },
   {(CHARTYPE *)"shadow",          6, 4,ITEM_SHADOW,        1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_shadow                 },
   {(CHARTYPE *)"showkey",         7, 4,ITEM_SHOWKEY,       1, 1,          LVL_FILE,                         QUERY_EXTRACT             ,extract_showkey                },
   {(CHARTYPE *)"size",            4, 2,ITEM_SIZE,          1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_size                   },
   {(CHARTYPE *)"slk",             3, 3,ITEM_SLK,           2, 0,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_slk                    },
   {(CHARTYPE *)"statopt",         7, 7,ITEM_STATOPT,       1, 1,          LVL_GLOB,QUERY_QUERY             |QUERY_EXTRACT             ,extract_statopt                },
   {(CHARTYPE *)"statusline",     10, 7,ITEM_STATUSLINE,    1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_statusline             },
   {(CHARTYPE *)"stay",            4, 4,ITEM_STAY,          1, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_stay                   },
   {(CHARTYPE *)"synelem",         7, 4,ITEM_SYNELEM,       1, 0,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_synelem                },
   {(CHARTYPE *)"synonym",         7, 3,ITEM_SYNONYM,       1, 0,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_synonym                },
   {(CHARTYPE *)"tabkey",          6, 4,ITEM_TABKEY,        2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabkey                 },
   {(CHARTYPE *)"tabline",         7, 4,ITEM_TABLINE,       2, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabline                },
   {(CHARTYPE *)"tabs",            4, 4,ITEM_TABS,          1, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabs                   },
   {(CHARTYPE *)"tabsin",          6, 5,ITEM_TABSIN,        2, 2,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabsin                 },
   {(CHARTYPE *)"tabsout",         7, 5,ITEM_TABSOUT,       2, 2,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tabsout                },
   {(CHARTYPE *)"targetsave",     10,10,ITEM_TARGETSAVE,    1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_targetsave             },
   {(CHARTYPE *)"terminal",        8, 4,ITEM_TERMINAL,      1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_terminal               },
   {(CHARTYPE *)"thighlight",      5, 5,ITEM_THIGHLIGHT,    1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_thighlight             },
   {(CHARTYPE *)"timecheck",       9, 9,ITEM_TIMECHECK,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_timecheck              },
   {(CHARTYPE *)"tof",             3, 3,ITEM_TOF,           1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_tof                    },
   {(CHARTYPE *)"tofeof",          6, 6,ITEM_TOFEOF,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_tofeof                 },
   {(CHARTYPE *)"trailing",        8, 8,ITEM_TRAILING,      1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_trailing               },
   {(CHARTYPE *)"typeahead",       9, 5,ITEM_TYPEAHEAD,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_typeahead              },
   {(CHARTYPE *)"ui",              2, 2,ITEM_UI,            1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_ui                     },
   {(CHARTYPE *)"undoing",         7, 7,ITEM_UNDOING,       1, 1,          LVL_FILE,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_undoing                },
   {(CHARTYPE *)"untaa",           5, 5,ITEM_UNTAA,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_untaa                  },
   {(CHARTYPE *)"utf8",            4, 4,ITEM_UTF8,          1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_utf8                   },
   {(CHARTYPE *)"verify",          6, 1,ITEM_VERIFY,        1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_verify                 },
   {(CHARTYPE *)"vershift",        8, 4,ITEM_VERSHIFT,      1, 1,          LVL_VIEW,QUERY_QUERY             |QUERY_EXTRACT             ,extract_vershift               },
   {(CHARTYPE *)"version",         7, 7,ITEM_VERSION,       5, 5,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT             ,extract_version                },
   {(CHARTYPE *)"width",           5, 1,ITEM_WIDTH,         1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_width                  },
   {(CHARTYPE *)"word",            4, 4,ITEM_WORD,          1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_word                   },
   {(CHARTYPE *)"wordwrap",        8, 5,ITEM_WORDWRAP,      1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_wordwrap               },
   {(CHARTYPE *)"wrap",            4, 2,ITEM_WRAP,          1, 1,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_wrap                   },
   {(CHARTYPE *)"xterminal",       9, 5,ITEM_XTERMINAL,     1, 1,          LVL_GLOB,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_xterminal              },
   {(CHARTYPE *)"zone",            4, 1,ITEM_ZONE,          2, 2,          LVL_VIEW,QUERY_QUERY|QUERY_STATUS|QUERY_EXTRACT|QUERY_MODIFY,extract_zone                   },
};
#define NUMBER_QUERY_ITEM ( sizeof( query_item ) / sizeof( QUERY_ITEM ) )

QUERY_ITEM _THE_FAR function_item[] =
{
   {(CHARTYPE *)"after",           5, 5,ITEM_AFTER_FUNCTION,         0, 1,LVL_VIEW,0,extract_after_function         },
   {(CHARTYPE *)"alt",             3, 3,ITEM_ALT_FUNCTION,           0, 1,LVL_FILE,0,extract_alt_function           },
   {(CHARTYPE *)"altkey",          6, 6,ITEM_ALTKEY_FUNCTION,        0, 1,LVL_GLOB,0,extract_altkey_function        },
   {(CHARTYPE *)"batch",           5, 5,ITEM_BATCH_FUNCTION,         0, 1,LVL_GLOB,0,extract_batch_function         },
   {(CHARTYPE *)"before",          6, 6,ITEM_BEFORE_FUNCTION,        0, 1,LVL_VIEW,0,extract_before_function        },
   {(CHARTYPE *)"blank",           5, 5,ITEM_BLANK_FUNCTION,         0, 1,LVL_VIEW,0,extract_blank_function         },
   {(CHARTYPE *)"block",           5, 5,ITEM_BLOCK_FUNCTION,         0, 1,LVL_VIEW,0,extract_block_function         },
   {(CHARTYPE *)"bottomedge",     10,10,ITEM_BOTTOMEDGE_FUNCTION,    0, 1,LVL_VIEW,0,extract_bottomedge_function    },
   {(CHARTYPE *)"command",         7, 7,ITEM_COMMAND_FUNCTION,       0, 1,LVL_VIEW,0,extract_command_function       },
   {(CHARTYPE *)"ctrl",            4, 4,ITEM_CTRL_FUNCTION,          0, 1,LVL_GLOB,0,extract_ctrl_function          },
   {(CHARTYPE *)"current",         7, 7,ITEM_CURRENT_FUNCTION,       0, 1,LVL_VIEW,0,extract_current_function       },
   {(CHARTYPE *)"dir",             3, 3,ITEM_DIR_FUNCTION,           0, 1,LVL_FILE,0,extract_dir_function           },
   {(CHARTYPE *)"end",             3, 3,ITEM_END_FUNCTION,           0, 1,LVL_VIEW,0,extract_end_function           },
   {(CHARTYPE *)"eof",             3, 3,ITEM_EOF_FUNCTION,           0, 1,LVL_VIEW,0,extract_eof_function           },
   {(CHARTYPE *)"first",           5, 5,ITEM_FIRST_FUNCTION,         0, 1,LVL_VIEW,0,extract_first_function         },
   {(CHARTYPE *)"focuseof",        8, 8,ITEM_FOCUSEOF_FUNCTION,      0, 1,LVL_VIEW,0,extract_focuseof_function      },
   {(CHARTYPE *)"focustof",        8, 8,ITEM_FOCUSTOF_FUNCTION,      0, 1,LVL_VIEW,0,extract_focustof_function      },
   {(CHARTYPE *)"inblock",         7, 7,ITEM_INBLOCK_FUNCTION,       0, 1,LVL_VIEW,0,extract_inblock_function       },
   {(CHARTYPE *)"incommand",       9, 9,ITEM_INCOMMAND_FUNCTION,     0, 1,LVL_VIEW,0,extract_incommand_function     },
   {(CHARTYPE *)"initial",         7, 7,ITEM_INITIAL_FUNCTION,       0, 1,LVL_GLOB,0,extract_initial_function       },
   {(CHARTYPE *)"inprefix",        8, 8,ITEM_INPREFIX_FUNCTION,      0, 1,LVL_VIEW,0,extract_inprefix_function      },
   {(CHARTYPE *)"insertmode",     10,10,ITEM_INSERTMODE_FUNCTION,    0, 1,LVL_GLOB,0,extract_inprefix_function      },
   {(CHARTYPE *)"leftedge",        8, 8,ITEM_LEFTEDGE_FUNCTION,      0, 1,LVL_VIEW,0,extract_leftedge_function      },
   {(CHARTYPE *)"modifiable",     10,10,ITEM_MODIFIABLE_FUNCTION,    0, 1,LVL_VIEW,0,extract_modifiable_function    },
   {(CHARTYPE *)"rightedge",       9, 9,ITEM_RIGHTEDGE_FUNCTION,     0, 1,LVL_VIEW,0,extract_rightedge_function     },
   {(CHARTYPE *)"run_os",          6, 6,ITEM_RUN_OS_FUNCTION,        0, 1,LVL_GLOB,0,NULL                           },
   {(CHARTYPE *)"shadow",          6, 6,ITEM_SHADOW_FUNCTION,        0, 1,LVL_VIEW,0,extract_shadow_function        },
   {(CHARTYPE *)"shift",           5, 5,ITEM_SHIFT_FUNCTION,         0, 1,LVL_GLOB,0,extract_shift_function         },
   {(CHARTYPE *)"spacechar",       9, 9,ITEM_SPACECHAR_FUNCTION,     0, 1,LVL_VIEW,0,extract_spacechar_function     },
   {(CHARTYPE *)"tof",             3, 3,ITEM_TOF_FUNCTION,           0, 1,LVL_VIEW,0,extract_tof_function           },
   {(CHARTYPE *)"topedge",         7, 7,ITEM_TOPEDGE_FUNCTION,       0, 1,LVL_VIEW,0,extract_topedge_function       },
   {(CHARTYPE *)"valid_target",   12,12,ITEM_VALID_TARGET_FUNCTION,  0, 1,LVL_VIEW,0,NULL                           },
   {(CHARTYPE *)"verone",          6, 6,ITEM_VERONE_FUNCTION,        0, 1,LVL_VIEW,0,extract_verone_function        },
};
#define NUMBER_FUNCTION_ITEM ( sizeof( function_item ) / sizeof( QUERY_ITEM ) )

CHARTYPE _THE_FAR *block_name[] = {
                          (CHARTYPE *)"",
                          (CHARTYPE *)"LINE",
                          (CHARTYPE *)"BOX",
                          (CHARTYPE *)"STREAM",
                          (CHARTYPE *)"COLUMN",
                          (CHARTYPE *)"WORD",
                          (CHARTYPE *)"CUA"
                         };

VALUE item_values[MAX_VARIABLES_RETURNED];

THE_PPC *in_range( THE_PPC *found_ppc, THE_PPC *curr_ppc, LINETYPE first_in_range, LINETYPE last_in_range )
{
   if ( found_ppc == NULL )
   {
      if ( curr_ppc->ppc_line_number >= first_in_range
      &&   curr_ppc->ppc_line_number <= last_in_range )
         found_ppc = curr_ppc;
   }
   else
   {
      if ( curr_ppc->ppc_line_number < found_ppc->ppc_line_number
      &&   curr_ppc->ppc_line_number >= first_in_range
      &&   curr_ppc->ppc_line_number <= last_in_range )
         found_ppc = curr_ppc;
   }
   return found_ppc;
}

void set_key_values(int key, bool mouse_key)
{
   CHARTYPE *keyname=NULL;
   int shift=0;
   int mod_shift, mod_ctrl, mod_alt;

   if ( mouse_key )
   {
      int b,ba,bm,w;
      CHARTYPE scrn;
      if ( get_mouse_info( &b, &ba, &bm ) == RC_OK )
      {
         which_window_is_mouse_in( &scrn, &w );
         key = mouse_info_to_key( w, b, ba, bm );
         item_values[1].value = mouse_key_number_to_name( key, query_rsrvd, &shift );
         item_values[1].len = strlen( (DEFCHAR *)item_values[1].value );
      }
      mod_shift = SHIFT_MOUSE_SHIFT;
      mod_ctrl = SHIFT_MOUSE_CTRL;
      mod_alt = SHIFT_MOUSE_ALT;
   }
   else
   {
      keyname = get_key_name( key, &shift );
      if (keyname == NULL)
      {
         item_values[1].value = (CHARTYPE *)"";
         item_values[1].len = 0;
      }
      else
      {
         item_values[1].value = keyname;
         item_values[1].len = strlen( (DEFCHAR *)keyname );
      }
      mod_shift = SHIFT_SHIFT;
      mod_ctrl = SHIFT_CTRL;
      mod_alt = SHIFT_ALT;
   }
   if (key < 256
   &&  key >= 0)
   {
      sprintf((DEFCHAR *)query_num1,"%d",key);
      item_values[2].value = query_num1;
      item_values[2].len = strlen( (DEFCHAR *)query_num1 );
   }
   else
   {
      item_values[2].value = (CHARTYPE *)"";
      item_values[2].len = 0;
   }
   sprintf( (DEFCHAR *)query_num2, "%d", key );
   item_values[3].value = query_num2;
   item_values[3].len = strlen( (DEFCHAR *)query_num2 );
   memset( (DEFCHAR *)query_num3, '0', 8 );
   query_num3[8] = '\0';
   if ( key != -1 )
   {
      if ( INSERTMODEx )
         query_num3[0] = '1';
      if ( shift & mod_alt )
         query_num3[4] = '1';
      if ( shift & mod_ctrl )
         query_num3[5] = '1';
      if ( shift & mod_shift )
         query_num3[6] = query_num3[7] = '1';
   }
   item_values[4].value = query_num3;
   item_values[4].len = 8;
   return;
}

short set_boolean_value(bool flag, short num)
{
   if (flag)
   {
      item_values[num].value = (CHARTYPE *)"1";
      item_values[num].len = 1;
   }
   else
   {
      item_values[num].value = (CHARTYPE *)"0";
      item_values[num].len = 1;
   }
   return 1;
}

short set_on_off_value(bool flag, short num)
{
   if (flag)
   {
      item_values[num].value = (CHARTYPE *)"ON";
      item_values[num].len = 2;
   }
   else
   {
      item_values[num].value = (CHARTYPE *)"OFF";
      item_values[num].len = 3;
   }
   return 1;
}

short find_query_item(CHARTYPE *item_name,int len,CHARTYPE *query_type)
{
   int rc=0;

   /*
    * Using the supplied abbreviation for an item, find the entry in the
    * query_item[] array. It should return with the index pointer (for
    * QUERY_EXTRACT) or an index to the first item in the function_item[]
    * rexx.c for other query types.
    */
   set_compare_exact( FALSE );
   rc = search_query_item_array( query_item,
                                 number_query_item(),
                                 sizeof( QUERY_ITEM ),
                                 (DEFCHAR *)item_name,
                                 len);
   if (rc != (-1))
      *query_type = query_item[rc].query;
   return(rc);
}
short show_status(void)
{
#define STATUS_COLS 6
   register short i=0,j=0,k=0;
   short lineno=0,colno=0;
   short number_variables=0;
   short item_width=0,column=0,column_width=0,col[STATUS_COLS];

   /*
    * For each item that is displayable, display it...
    */
   column = 0;
   column_width = COLS / STATUS_COLS;
   col[0] = 0;
   for (i=1;i<STATUS_COLS;i++)
      col[i] = col[i-1]+column_width+1;
   wclear(stdscr);
   for (i=0; i<NUMBER_QUERY_ITEM; i++)
   {
      /*
       * Get only those settings that are queryable...
       */
      if (query_item[i].query & QUERY_STATUS)
      {
         number_variables = get_item_values(1,query_item[i].item_number,(CHARTYPE *)"",QUERY_STATUS,0L,NULL,0L);
         item_width = 0;
         /*
          * Obtain the total length of the setting values...
          */
         for (j=0;j<number_variables+1;j++)
            item_width += item_values[j].len+1;
         item_width--;     /* reduce by 1 for last value's blank at end */
         /*
          * If the length of the variables is > the screen width, go to next
          * line.
          */
         if (item_width+col[column] > COLS)
         {
            column = colno = 0;
            lineno++;
         }
         else
            colno = col[column];
         /*
          * Display the variables. For the first value, display in BOLD.
          */
         for (j=0;j<number_variables+1;j++)
         {
            if (j == 0)
               attrset(A_BOLD);
            mvaddstr(lineno,colno,(DEFCHAR *)item_values[j].value);
            attrset(A_NORMAL);
            colno += item_values[j].len+1;
         }
         colno--;
         /*
          * Now have to determine where to display the next set of variables...
          * If the just-displayed values go past the last column, or we are
          * already in the last column, go to the next line...
          */
         if (colno >= col[STATUS_COLS-1] || column == STATUS_COLS-1)
         {
            lineno++;
            column = 0;
         }
         else
         {
            /*
             * ...else if the the just-displayed values go past the 2nd column...
             * If the just-displayed values go past the last column, go on to the
             * next line...
             */
            for (k=0;k<STATUS_COLS;k++)
            {
               if (colno < col[k])
               {
                  column = k;
                  break;
               }
            }
         }
      }
   }
   mvaddstr(terminal_lines-2,0,HIT_ANY_KEY);
   refresh();

   return(RC_OK);
}
short save_status(CHARTYPE *filename)
{
   register short i=0,j=0;
   short number_variables=0,rc=RC_OK;
   FILE *fp=NULL;

   /*
    * Get the fully qualified filename from the supplied filename.
    */
   strrmdup( strtrans( filename, OSLASH,ISLASH ), ISLASH, TRUE );
   if ( ( rc = splitpath( filename ) ) != RC_OK )
   {
      display_error( 10, filename, FALSE );
      return(rc);
   }
   /*
    * splitpath() has set up sp_path  and sp_fname. Append the filename.
    */
   strcat( (DEFCHAR *)sp_path, (DEFCHAR *)sp_fname );
   /*
    * If the file already exists, display an error.
    */
   if ( file_exists( sp_path ) == THE_FILE_EXISTS )
   {
      display_error( 8, filename, FALSE );
      return(rc);
   }
   fp = fopen( (DEFCHAR *)sp_path, "w" );
   /*
    * For each item that is modifiable, get its value...
    */
   fprintf( fp, "/* This file generated by STATUS command of THE %s %s*/\n", the_version, the_release );
   for ( i = 0; i < NUMBER_QUERY_ITEM; i++ )
   {
      /*
       * Get only those settings that are modifiable...
       */
      if ( query_item[i].query & QUERY_MODIFY )
      {
         number_variables = get_item_values( 1, query_item[i].item_number, (CHARTYPE *)"", QUERY_MODIFY, 0L, NULL, 0L );
         fputs( "'set", fp );
         /*
          * Write the variables to the file...
          */
         for ( j = 0; j < number_variables + 1; j++ )
         {
            fputc( ' ', fp );
            fputs( (DEFCHAR *)item_values[j].value, fp );
         }
         fprintf( fp, "'\n" );
      }
   }
   fflush( fp );
   fclose( fp );
   return(RC_OK);
}
short set_extract_variables(short itemno)
{
   register short i=0;
   short rc=RC_OK,number_values=0;

   number_values = atoi((DEFCHAR *)item_values[0].value);
   for (i=0;i<number_values+1;i++)
   {
      if ((rc = set_rexx_variable(query_item[itemno].name,item_values[i].value,item_values[i].len,i)) != RC_OK)
         break;
   }
   return(rc);
}

short get_number_dynamic_items( int qitem )
{
   int number_variables=0;
   PARSER_MAPPING *mapping=NULL;
   PARSER_DETAILS *details=NULL;
   DEFINE *synonym=NULL;

   switch(qitem)
   {
      case ITEM_RING:
         number_variables = number_of_files;
         break;
      case ITEM_PARSER:
         for(details=first_parser;details!=NULL;details=details->next,number_variables++);
         break;
      case ITEM_AUTOCOLOR:
      case ITEM_AUTOCOLOUR:
         for(mapping=first_parser_mapping;mapping!=NULL;mapping=mapping->next,number_variables++);
         break;
      case ITEM_SYNONYM:
         for (synonym=first_synonym;synonym!=NULL;synonym=synonym->next,number_variables++);
      default:
         break;
   }
   return number_variables;
}

short get_item_values(int qitem,short itemno,CHARTYPE *itemargs,CHARTYPE query_type,LINETYPE argc,CHARTYPE *arg,LINETYPE arglen)
{
   short number_variables = 1;

   /*
    * Only use itemno to reference query_item[] array if it is NOT a
    * boolean function...
    */
   if (qitem)
   {
      number_variables = (query_item[itemno].ext_func)(
          query_item[itemno].number_values,
          itemno,
          itemargs,
          query_type,
          argc,
          arg,
          arglen);
   }
   else
   {
      number_variables = (function_item[itemno].ext_func)(
          1,
          itemno,
          itemargs,
          query_type,
          argc,
          arg,
          arglen);
   }
   /*
    * If an "error" condition exists, do not set any values.
    */
   if (number_variables >= 0)
   {
      if (number_variables == 0)
      {
         /*
          * The number of variables needs to be set dynamically for the
          * following items. Any item that has a value of 0 for item_values
          * column of query_item, needs to be catered for here.
          */
         number_variables = get_number_dynamic_items( query_item[itemno].item_number );
      }
      switch(query_type)
      {
         case QUERY_EXTRACT:
         case QUERY_FUNCTION:
         case QUERY_READV:
            sprintf((DEFCHAR *)query_num0,"%d",number_variables);
            item_values[0].value = query_num0;
            item_values[0].len = strlen((DEFCHAR *)query_num0);
            break;
         case QUERY_STATUS:
         case QUERY_QUERY:
         case QUERY_MODIFY:
            item_values[0].value = query_item[itemno].name;
            item_values[0].len = strlen((DEFCHAR *)query_item[itemno].name);
            break;
         default:
            break;
      }
   }
   return(number_variables);
}
short extract_point_settings(short itemno,CHARTYPE *params)
{
   register short i=0;
   short number_variables = query_item[itemno].number_values;
   CHARTYPE num4[15];
   LINE *curr=NULL;
   LINETYPE true_line=0L;
   short rc=RC_OK;
   THELIST *curr_name;
   int len,total_len=0;

   if ( strcmp( (DEFCHAR *)params, "" ) == 0 ) /* get name for focus line only */
   {
      true_line = (compatible_feel==COMPAT_XEDIT)?CURRENT_VIEW->current_line:get_true_line( TRUE );
      curr = lll_find( CURRENT_FILE->first_line, CURRENT_FILE->last_line, true_line, CURRENT_FILE->number_lines );
      if ( curr->first_name == NULL )  /* line not named */
         number_variables = 0;
      else
      {
         total_len = sprintf( (DEFCHAR *)query_rsrvd, "%ld", true_line );
         curr_name = curr->first_name;
         while( curr_name )
         {
            len = strlen( (DEFCHAR *)curr_name->data );
            if ( total_len + len + 1 > sizeof(query_rsrvd) )
            {
               break;
            }
            strcat( (DEFCHAR *)query_rsrvd, " " );
            strcat( (DEFCHAR *)query_rsrvd, (DEFCHAR *)curr_name->data );
            total_len += len+1;
            curr_name = curr_name->next;
         }
         item_values[1].value = query_rsrvd;
         item_values[1].len = total_len;
         number_variables = 1;
      }
   }
   else
   {
      if ( strcmp( (DEFCHAR *)params, "*" ) != 0 )     /* argument must be "*" */
      {
         display_error( 1, (CHARTYPE *)params, FALSE );
         number_variables = EXTRACT_ARG_ERROR;
      }
      else
      {
         curr = CURRENT_FILE->first_line;
         for( true_line=0,i=0; curr != NULL; true_line++ )
         {
            if ( curr->first_name != NULL )  /* line is named */
            {
               total_len = sprintf( (DEFCHAR *)query_rsrvd, "%ld", true_line );
               curr_name = curr->first_name;
               while( curr_name )
               {
                  len = strlen( (DEFCHAR *)curr_name->data );
                  if ( total_len + len + 1 > sizeof(query_rsrvd) )
                  {
                     break;
                  }
                  strcat( (DEFCHAR *)query_rsrvd, " " );
                  strcat( (DEFCHAR *)query_rsrvd, (DEFCHAR *)curr_name->data );
                  total_len += len+1;
                  curr_name = curr_name->next;
               }
               rc = set_rexx_variable( query_item[itemno].name, query_rsrvd, total_len, ++i );
               if ( rc == RC_SYSTEM_ERROR )
               {
                  display_error( 54, (CHARTYPE *)"", FALSE );
                  return( EXTRACT_ARG_ERROR );
               }
            }
            curr = curr->next;
         }
         sprintf( (DEFCHAR *)num4, "%d", i );
         rc = set_rexx_variable( query_item[itemno].name, num4, strlen( (DEFCHAR *)num4 ), 0 );
         if ( rc == RC_SYSTEM_ERROR )
         {
            display_error( 54, (CHARTYPE *)"", FALSE );
            number_variables = EXTRACT_ARG_ERROR;
         }
         else
            number_variables = EXTRACT_VARIABLES_SET;
      }
   }
   return( number_variables );
}

short extract_prefix_settings(short itemno,CHARTYPE *params,CHARTYPE query_type)
{
   register short i=0;
#define PRE_PARAMS  3
   CHARTYPE *word[PRE_PARAMS+1];
   CHARTYPE strip[PRE_PARAMS];
   unsigned short num_params=0;
   short number_variables = query_item[itemno].number_values;
   CHARTYPE num4[15];
   LINE *curr=NULL;
   short rc=RC_OK;
   CHARTYPE *tmpbuf=NULL;

   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(params,word,PRE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   switch(num_params)
   {
      /*
       * None or 1 parameters, error.
       */
      case 0:
      case 1:
         display_error(3,(CHARTYPE *)"",FALSE);
         number_variables = EXTRACT_ARG_ERROR;
         break;
      /*
       * 2  parameters, Synonym and name (or *).
       */
      case 2:
         if (!equal((CHARTYPE *)"synonym",word[0],1))
         {
            display_error(13,(CHARTYPE *)"",FALSE);
            number_variables = EXTRACT_ARG_ERROR;
            break;
         }
         break;
    default:
         /*
          * Too many parameters, error.
          */
         display_error(2,(CHARTYPE *)"",FALSE);
         number_variables = EXTRACT_ARG_ERROR;
         break;
   }
   /*
    * If the validation of parameters is successful...
    */
   if (number_variables >= 0)
   {
      if (strcmp((DEFCHAR *)word[1],"*") == 0)
      {
         /*
          * Get oldname for all synonyms...
          */
         curr = first_prefix_synonym;
         i = 0;
         while(curr != NULL)
         {
            tmpbuf = (CHARTYPE *)(*the_malloc)(sizeof(CHARTYPE)*(strlen((DEFCHAR *)curr->name)+strlen((DEFCHAR *)curr->line)+2));
            if (tmpbuf == (CHARTYPE *)NULL)
            {
               display_error(30,(CHARTYPE *)"",FALSE);
               return(EXTRACT_ARG_ERROR);
            }
            strcpy((DEFCHAR *)tmpbuf,(DEFCHAR *)curr->name);
            strcat((DEFCHAR *)tmpbuf," ");
            strcat((DEFCHAR *)tmpbuf,(DEFCHAR *)curr->line);
            if ( query_type == QUERY_EXTRACT )
               rc = set_rexx_variable(query_item[itemno].name,tmpbuf,strlen((DEFCHAR *)tmpbuf),++i);
            else
            {
               display_error( 0, tmpbuf, FALSE );
               rc = RC_OK;
            }
            (*the_free)(tmpbuf);
            if (rc == RC_SYSTEM_ERROR)
            {
               display_error(54,(CHARTYPE *)"",FALSE);
               return(EXTRACT_ARG_ERROR);
            }
            curr = curr->next;
         }
         if ( query_type == QUERY_EXTRACT )
         {
            sprintf((DEFCHAR *)num4,"%d",i);
            rc = set_rexx_variable(query_item[itemno].name,num4,strlen((DEFCHAR *)num4),0);
            if (rc == RC_SYSTEM_ERROR)
            {
               display_error(54,(CHARTYPE *)"",FALSE);
               number_variables = EXTRACT_ARG_ERROR;
            }
            else
               number_variables = EXTRACT_VARIABLES_SET;
         }
         else
         {
            number_variables = EXTRACT_VARIABLES_SET;
         }
      }
      else
      {
         /*
          * Get oldname for named synonym...
          */
         item_values[1].value = find_prefix_synonym(word[1]);
         item_values[1].len = strlen((DEFCHAR *)item_values[1].value);
         number_variables = 1;
      }
   }
   return(number_variables);
}
void get_etmode(CHARTYPE *onoff,CHARTYPE *list)
{
   bool on_flag=FALSE;
   bool off_flag=FALSE;
   bool last_state=FALSE;
   char tmp[15];
   register int i=0;

   strcpy((DEFCHAR *)list,"");
   last_state = TRUE;
   for (i=0;i<256;i++)
   {
      if (etmode_flag[i] == TRUE)
         off_flag = TRUE;
      else
         on_flag = TRUE;
      if (last_state != etmode_flag[i])
      {
         if (last_state == FALSE)
         {
            sprintf(tmp,"%d",i-1);
            strcat((DEFCHAR *)list,tmp);
         }
         else
         {
            sprintf(tmp," %d-",i);
            strcat((DEFCHAR *)list,tmp);
         }
         last_state = etmode_flag[i];
      }
   }
   if (*(list+strlen((DEFCHAR *)list)-1) == '-')
   {
      sprintf(tmp,"%d",255);
      strcat((DEFCHAR *)list,tmp);
   }
   if (on_flag)
   {
      strcpy((DEFCHAR *)onoff,"ON");
      if (!off_flag)
         strcpy((DEFCHAR *)list,"");
   }
   else
   {
      strcpy((DEFCHAR *)onoff,"OFF");
      strcpy((DEFCHAR *)list,"");
   }
   return;
}
short extract_colour_settings(short itemno,CHARTYPE *buffer,CHARTYPE query_type,CHARTYPE *params,bool us,bool isecolour)
{
   short rc=RC_OK;
   register int i=0,maxnum;
   int start=0,end=0,number_variables=0,off=0;
   bool found=FALSE;
   CHARTYPE *attr_string=NULL,tmparea[2];
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr,*area,*colour=(CHARTYPE*)"colour ",*color=(CHARTYPE*)"color ",*ecolour=(CHARTYPE*)"ecolour ",*ecolor=(CHARTYPE*)"ecolor ";
   COLOUR_ATTR *attr;

   tmparea[1] = '\0';
   maxnum = (isecolour) ? ECOLOUR_MAX : ATTR_MAX;
   attr = (isecolour) ? CURRENT_FILE->ecolour : CURRENT_FILE->attr;
   if (blank_field(params)
   ||  strcmp((DEFCHAR*)params,"*") == 0)
   {
     start = 0;
     end = maxnum;
   }
   else
   {
      if (isecolour)
      {
         if (strlen((DEFCHAR *)params) != 1)
         {
            return(EXTRACT_ARG_ERROR);
         }
         if (*params >= 'A' && *params <= 'Z')
            i = *params - 'A';
         else if (*params >= 'a' && *params <= 'z')
            i = *params - 'a';
         else if (*params >= '1' && *params <= '9')
            i = *params - '1' + 26;
         else
         {
            return(EXTRACT_ARG_ERROR);
         }
         found = TRUE;
      }
      else
      {
         for (i=0;i<maxnum;i++)
         {
            if (equal(valid_areas[i].area,params,valid_areas[i].area_min_len))
            {
               found = TRUE;
               break;
            }
         }
      }
      if (!found)
      {
         display_error(1,params,FALSE);
         return(EXTRACT_ARG_ERROR);
      }
      start = i;
      end = i+1;
   }
   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,end-start);
      CURRENT_VIEW->msgmode_status = TRUE;
   }

   for (i=start;i<end;i++)
   {
      attr_string = get_colour_strings(attr+i);
      if (attr_string == (CHARTYPE *)NULL)
      {
         return(EXTRACT_ARG_ERROR);
      }
      if (us)
      {
         if (isecolour)
            ptr = ecolor;
         else
            ptr = color;
      }
      else
      {
         if (isecolour)
            ptr = ecolour;
         else
            ptr = colour;
      }
      if (isecolour)
      {
        if ( i > 25 )
           tmparea[0] = (CHARTYPE)(i-25)+'0';
        else
           tmparea[0] = (CHARTYPE)(i+'A');
         area = tmparea;
      }
      else
      {
         area = valid_areas[i].area;
      }
      sprintf((DEFCHAR *)query_rsrvd,"%s%s %s",
            (query_type == QUERY_QUERY) ? (DEFCHAR *)ptr : "",
            area,
            attr_string);
      (*the_free)(attr_string);

      if (query_type == QUERY_QUERY)
         display_error(0,query_rsrvd,TRUE);
      else
      {
         number_variables++;
         item_values[number_variables].len = strlen((DEFCHAR *)query_rsrvd);
         memcpy((DEFCHAR*)trec+off,(DEFCHAR*)query_rsrvd,(item_values[number_variables].len)+1);
         item_values[number_variables].value = trec+off;
         off += (item_values[number_variables].len)+1;
      }
   }
   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;
   return(rc);
}
short extract_autocolour_settings(short itemno,CHARTYPE *buffer,CHARTYPE query_type,CHARTYPE *params,bool us)
{
   short rc=RC_OK;
   register int i=0;
   int number_variables=0,off=0;
   bool found=FALSE;
   ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE *ptr,*ptr_mask=NULL,*ptr_magic=NULL,*ptr_parser=NULL;
   PARSER_MAPPING *curr;

   ptr = (us) ? (CHARTYPE *)"autocolor " : (CHARTYPE *)"autocolour ";
   if (blank_field(params)
   ||  strcmp((DEFCHAR*)params,"*") == 0)
   {
      if (query_type == QUERY_QUERY)
      {
         for (i=0,curr=first_parser_mapping;curr!=NULL;curr=curr->next,i++);
         CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,i);
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      for (curr=first_parser_mapping;curr!=NULL;curr=curr->next)
      {
         sprintf((DEFCHAR *)query_rsrvd,"%s%s%s %s%s",
           (query_type == QUERY_QUERY) ? (DEFCHAR *)ptr : "",
           (curr->filemask) ? (DEFCHAR *)curr->filemask : "",
           (curr->magic_number) ? (DEFCHAR *)curr->magic_number : "",
            (DEFCHAR *)curr->parser->parser_name,
           (curr->magic_number) ? " MAGIC" : "");

         if (query_type == QUERY_QUERY)
            display_error(0,query_rsrvd,TRUE);
         else
         {
            number_variables++;
            item_values[number_variables].len = strlen((DEFCHAR *)query_rsrvd);
            memcpy((DEFCHAR*)trec+off,(DEFCHAR*)query_rsrvd,(item_values[number_variables].len)+1);
            item_values[number_variables].value = trec+off;
            off += (item_values[number_variables].len)+1;
         }
      }
   }
   else
   {
      if (query_type == QUERY_QUERY)
      {
         CURRENT_VIEW->msgline_rows   = 1;
         CURRENT_VIEW->msgmode_status = TRUE;
      }
      /*
       * Find a match for the supplied mask or magic number
       */
      for (curr=first_parser_mapping;curr!=NULL;curr=curr->next)
      {
         if (curr->filemask
         &&  strcmp((DEFCHAR *)params,(DEFCHAR *)curr->filemask) == 0)
         {
            ptr_mask = curr->filemask;
            ptr_magic = (CHARTYPE *)"";
            found = TRUE;
            break;
         }
         if (curr->magic_number
         &&  strcmp((DEFCHAR *)params,(DEFCHAR *)curr->magic_number) == 0)
         {
            ptr_mask = curr->magic_number;
            ptr_magic = (CHARTYPE *)"MAGIC";
            found = TRUE;
            break;
         }
      }
      if (found)
         ptr_parser = (CHARTYPE *)curr->parser->parser_name;
      else
      {
         ptr_mask = params;
         ptr_magic = (CHARTYPE *)"";
         ptr_parser = (CHARTYPE *)"NULL";
      }
      if (query_type == QUERY_QUERY)
      {
         sprintf((DEFCHAR *)query_rsrvd,"%s%s %s%s",
            (query_type == QUERY_QUERY) ? (DEFCHAR *)ptr : "",
            ptr_mask,
            ptr_parser,
            ptr_magic);
         display_error(0,query_rsrvd,TRUE);
      }
      else
      {
         item_values[1].value = ptr_mask;
         item_values[1].len = strlen((DEFCHAR *)ptr_mask);
         item_values[2].value = ptr_parser;
         item_values[2].len = strlen((DEFCHAR *)ptr_parser);
         item_values[3].value = ptr_magic;
         item_values[3].len = strlen((DEFCHAR *)ptr_magic);
         number_variables = 3;
      }
   }

   if (query_type == QUERY_QUERY)
   {
      CURRENT_VIEW->msgline_rows   = save_msgline_rows;
      CURRENT_VIEW->msgmode_status = save_msgmode_status;
      rc = EXTRACT_VARIABLES_SET;
   }
   else
      rc = number_variables;

   return(rc);
}

int number_function_item( void )
{
   return NUMBER_FUNCTION_ITEM;
}

int number_query_item( void )
{
   return NUMBER_QUERY_ITEM;
}

void format_options( CHARTYPE *buf )
{
   LINE *curr=first_option;
   int tail,col,itemno,linelen,valuelen,number_variables,max_col=0,off;


   while(curr != NULL)
   {
      itemno = curr->length;
      tail = curr->flags.new_flag;
      number_variables = get_item_values(1,query_item[itemno].item_number,(CHARTYPE *)"",QUERY_EXTRACT,0L,NULL,0L);
      if (number_variables >= tail)
      {
         col = curr->select;
         linelen = curr->line?strlen((DEFCHAR *)curr->line):0;
         valuelen = (curr->save_select)?min(curr->save_select,item_values[tail].len):item_values[tail].len;
         if (curr->line)
         {
            memcpy(buf+col,(DEFCHAR *)curr->line,linelen);
         }
         memcpy(buf+col+linelen,(DEFCHAR *)item_values[tail].value,valuelen);
         off = col+linelen+valuelen;
         if (off > max_col)
            max_col = off;
      }
      curr = curr->next;
   }
   buf[max_col] = '\0';
   return;
}
