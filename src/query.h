/***********************************************************************/
/* QUERY.H -                                                           */
/* This file contains defines   related to QUERY,STATUS and EXTRACT    */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-1999 Mark Hessling
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
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 * PO Box 203, Bellara, QLD 4507, AUSTRALIA
 * Author of THE, a Free XEDIT/KEDIT editor and, Rexx/SQL
 * Maintainer of PDCurses: Public Domain Curses and, Regina Rexx interpreter
 * Use Rexx ? join the Rexx Language Association: http://www.rexxla.org
 */

/*
$Id: query.h,v 1.2 1999/07/06 04:06:04 mark Exp mark $
*/


/*---------------------------------------------------------------------*/
/* The order of these items determine the order they appear as a result*/
/* of the STATUS command, so they should be in alphabetical order.     */
/*---------------------------------------------------------------------*/
/* The following are item number defines for EXTRACT/QUERY/STATUS.     */
/*---------------------------------------------------------------------*/
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
#define ITEM_DIRINCLUDE                      28
#define ITEM_DISPLAY                         29
#define ITEM_ECOLOR                          30
#define ITEM_ECOLOUR                         31
#define ITEM_EOF                             32
#define ITEM_EOLOUT                          33
#define ITEM_ETMODE                          34
#define ITEM_FEXT                            35
#define ITEM_FIELD                           36
#define ITEM_FILENAME                        37
#define ITEM_FILESTATUS                      38
#define ITEM_FMODE                           39
#define ITEM_FNAME                           40
#define ITEM_FPATH                           41
#define ITEM_FTYPE                           42
#define ITEM_FULLFNAME                       43
#define ITEM_GETENV                          44
#define ITEM_HEX                             45
#define ITEM_HEXDISPLAY                      46
#define ITEM_HEXSHOW                         47
#define ITEM_HIGHLIGHT                       48
#define ITEM_IDLINE                          49
#define ITEM_IMPMACRO                        50
#define ITEM_IMPOS                           51
#define ITEM_INPUTMODE                       52
#define ITEM_INSERTMODE                      53
#define ITEM_LASTKEY                         54
#define ITEM_LASTMSG                         55
#define ITEM_LASTRC                          56
#define ITEM_LENGTH                          57
#define ITEM_LINE                            58
#define ITEM_LINEFLAG                        59
#define ITEM_LINEND                          60
#define ITEM_LSCREEN                         61
#define ITEM_MACRO                           62
#define ITEM_MACROEXT                        63
#define ITEM_MACROPATH                       64
#define ITEM_MARGINS                         65
#define ITEM_MONITOR                         66
#define ITEM_MOUSE                           67
#define ITEM_MSGLINE                         68
#define ITEM_MSGMODE                         69
#define ITEM_NBFILE                          70
#define ITEM_NEWLINES                        71
#define ITEM_NONDISP                         72
#define ITEM_NUMBER                          73
#define ITEM_PARSER                          74
#define ITEM_PENDING                         75
#define ITEM_POINT                           76
#define ITEM_POSITION                        77
#define ITEM_PREFIX                          78
#define ITEM_PRINTER                         79
#define ITEM_READONLY                        80
#define ITEM_READV                           81
#define ITEM_REPROFILE                       82
#define ITEM_RESERVED                        83
#define ITEM_REXX                            84
#define ITEM_REXXOUTPUT                      85
#define ITEM_RING                            86
#define ITEM_SCALE                           87
#define ITEM_SCOPE                           88
#define ITEM_SCREEN                          89
#define ITEM_SELECT                          90
#define ITEM_SHADOW                          91
#define ITEM_SHOWKEY                         92
#define ITEM_SIZE                            93
#define ITEM_STATUSLINE                      94
#define ITEM_STAY                            95
#define ITEM_TABKEY                          96
#define ITEM_TABLINE                         97
#define ITEM_TABS                            98
#define ITEM_TABSIN                          99
#define ITEM_TABSOUT                        100
#define ITEM_TERMINAL                       101
#define ITEM_TIMECHECK                      102
#define ITEM_TOF                            103
#define ITEM_TOFEOF                         104
#define ITEM_TYPEAHEAD                      105
#define ITEM_UNDOING                        106
#define ITEM_UNTAA                          107
#define ITEM_VERIFY                         108
#define ITEM_VERSHIFT                       109
#define ITEM_VERSION                        110
#define ITEM_WIDTH                          111
#define ITEM_WORD                           112
#define ITEM_WORDWRAP                       113
#define ITEM_WRAP                           114
#define ITEM_XTERMINAL                      115
#define ITEM_ZONE                           116

/*---------------------------------------------------------------------*/
/* The following are item number defines for the boolean functions.    */
/*---------------------------------------------------------------------*/
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
#define ITEM_LEFTEDGE_FUNCTION               21
#define ITEM_MODIFIABLE_FUNCTION             22
#define ITEM_RIGHTEDGE_FUNCTION              23
#define ITEM_RUN_OS_FUNCTION                 24
#define ITEM_SHADOW_FUNCTION                 25
#define ITEM_SHIFT_FUNCTION                  26
#define ITEM_SPACECHAR_FUNCTION              27
#define ITEM_TOF_FUNCTION                    28
#define ITEM_TOPEDGE_FUNCTION                29
#define ITEM_VALID_TARGET_FUNCTION           30
#define ITEM_VERONE_FUNCTION                 31
/*---------------------------------------------------------------------*/
/* The following are item number defines for the 'other' functions.    */
/*---------------------------------------------------------------------*/

