/***********************************************************************/
/* COMM4.C - Commands P-S                                              */
/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */
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

static char RCSid[] = "$Id: comm4.c,v 1.1 1999/06/25 06:11:56 mark Exp mark $";

#include <the.h>
#include <proto.h>

/*man-start*********************************************************************
COMMAND
     popup - display popup menu

SYNTAX
     POPUP [MOUSE|TEXT|CENTER|CENTRE] /item1[/item2/...]
     POPUP ESCAPE keyname

DESCRIPTION
     The POPUP command allows the user to create and display a popup
     menu containing a list of selectable options.

     The location of the popup menu is specified by the first parameter.
     'MOUSE' specifies that the top left corner of the popup menu is to
     be displayed where the mouse

     On return from the popup menu, the following Rexx variables are set:

          popup.0 = 1
          popup.1 = Item selected or empty string if no item selected.

     The second form of the POPUP command allows the user to specify
     the keyname that can be used to quit from the popup window without
     making a selection.  By default 'q' will quit.

     If mouse support is available, an item is selectable by clicking
     the first mouse button on the item.  To quit from the popup window
     without making a selection, click the mouse outside the popup
     window, or on the border of the window.

     Keyboard keys that take effect in the POPUP command are CURU, CURD
     and ENTER.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: First form is compatible. KEDIT does not require second form.

SEE ALSO
     <DIALOG>, <ALERT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Popup(CHARTYPE *params)
#else
short Popup(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define POP_PARAMS  2
#define MAX_POPUP_LINES 100
/*--------------------------- local data ------------------------------*/
   CHARTYPE *word[POP_PARAMS+1];
   CHARTYPE strip[POP_PARAMS];
   unsigned short num_params=0;
   register short i=0,j=0;
   short rc=RC_OK,num_items=0,len=0;
   short height=0,width=0;
   int x,y;
   unsigned short begy,begx;
   CHARTYPE *args[MAX_POPUP_LINES],*ptr=NULL;
   CHARTYPE delim;
   bool invalid_item=FALSE;
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   char location='M';
#else
   char location='C';
#endif
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("comm4.c:   Popup");
#endif
   if (blank_field(params))
   {
      display_error(3,params,FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_OPERAND);
   }
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,POP_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 2
   &&  equal(word[0],(CHARTYPE *)"ESCAPE",6))
   {
      /*
       * Process the second form of this command
       */
      popup_escape_key = find_key_name(word[1]);
      if (popup_escape_key == -1)
      {
         display_error(13,word[1],FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         rc = RC_INVALID_OPERAND;
      }
#ifdef THE_TRACE
      trace_return();
#endif
      return rc;
   }
/*---------------------------------------------------------------------*/
/* The first form is only valid from a Rexx macro.                     */
/*---------------------------------------------------------------------*/
   if (!in_macro
   ||  !rexx_support)
   {
      display_error(53,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_ENVIRON);
   }
/*---------------------------------------------------------------------*/
/* Check if the first character of params is a the start of a keyword. */
/*---------------------------------------------------------------------*/
   if (*(params) == 'M'
   ||  *(params) == 'm'
   ||  *(params) == 'T'
   ||  *(params) == 't'
   ||  *(params) == 'C'
   ||  *(params) == 'c')
   {
      if (num_params < 2)
      {
         display_error(3,params,FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_INVALID_OPERAND);
      }
      if (equal((CHARTYPE *)"mouse",word[0],5))
      {
         location = 'M';
      }
      else if (equal((CHARTYPE *)"text",word[0],4))
      {
         location = 'T';
      }
      else if (equal((CHARTYPE *)"center",word[0],5))
      {
         location = 'C';
      }
      else if (equal((CHARTYPE *)"centre",word[0],5))
      {
         location = 'C';
      }
      else
      {
         display_error(1,params,FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_INVALID_OPERAND);
      }
      ptr = word[1];
   }
   else
      ptr = params;
/*---------------------------------------------------------------------*/
/* The first character is saved as the delimiter...                    */
/*---------------------------------------------------------------------*/
   delim = *(ptr);
   ptr++;                                /* throw away first delimiter */
   len = strlen((DEFCHAR *)ptr);
/*---------------------------------------------------------------------*/
/* Check that we have at least one menu item...                        */
/*---------------------------------------------------------------------*/
   if (len == 0)
      invalid_item = TRUE;
   else
   {
      if (len == 1 && (*(ptr) == delim))
         invalid_item = TRUE;
   }
   if (invalid_item)
   {
      display_error(1,ptr,FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Allow for no trailing delimiter...                                  */
/*---------------------------------------------------------------------*/
   if ((*(ptr+len-1) == delim))
      num_items = 0;
   else
      num_items = 1;
/*---------------------------------------------------------------------*/
/* Replace all / with nul character to give us seperate strings.       */
/*---------------------------------------------------------------------*/
   args[0] = ptr;
   for (i=0,j=1;i<len;i++)
   {
      if (*(ptr+i) == delim)
      {
         *(ptr+i) = '\0';
         num_items++;
         args[j++] = ptr+i+1;
      }
   }
/*---------------------------------------------------------------------*/
/* Ensure that the number of items fit on the screen...                */
/*---------------------------------------------------------------------*/
   height = num_items + 2;
   if (height > terminal_lines)
   {
      display_error(0,(CHARTYPE *)"too many rows",FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Work out the maximum width of the menu items...                     */
/*---------------------------------------------------------------------*/
   for (i=0;i<num_items;i++)
   {
      len = strlen((DEFCHAR *)args[i]);
      if ( len > width)
         width = len;
   }
   width += 4;
   if (width > terminal_cols)
   {
      display_error(0,(CHARTYPE *)"too many columns",FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* If the location specified is MOUSE and this wasn't called from a    */
/* mouse key, or no mouse support is available, return an error.       */
/*---------------------------------------------------------------------*/
   switch(location)
   {
      case 'M':
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
         get_saved_mouse_pos(&y,&x);
#else
         x = y = -1;
#endif
         break;
      case 'C':
         x = (terminal_cols - width) / 2;
         y = (terminal_lines - height) / 2;
         break;
      case 'T':
         /*
          * Get the current window text position and get the
          * global offset from the window start coordinates.
          */
         getyx(CURRENT_WINDOW,y,x);
         getbegyx(CURRENT_WINDOW,begy,begx);
         y = (y + begy);
         x = (x + begx);
         break;
      default:
         break;
   }
/*---------------------------------------------------------------------*/
/* If the location specified is MOUSE and this wasn't called from a    */
/* mouse key, or no mouse support is available, return an error.       */
/*---------------------------------------------------------------------*/
   if (x == -1)
   {
      display_error(0,(CHARTYPE *)"No mouse support",FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_ENVIRON);
   }
/*---------------------------------------------------------------------*/
/* Make the window fit by adjusting the top left corner if it would    */
/* run over the right edge.                                            */
/*---------------------------------------------------------------------*/
   if (x + width > terminal_cols)
   {
      x = terminal_cols - width;
   }
   if (y + height > terminal_lines)
   {
      y = terminal_lines - height;
   }
   rc = execute_popup( y, x, height, width, num_items, args);
#ifdef THE_TRACE
   trace_return();
#endif
   return(rc);
}

/*man-start*********************************************************************
COMMAND
     preserve - save various editor settings

SYNTAX
     PREServe

DESCRIPTION
     The PRESERVE command saves various editing settings at the time
     the command is issued.  These settings can then be restored by
     using the <RESTORE> command.

     The following view level settings are saved:
         ARBCHAR
         ARROW
         CASE
         CMDLINE
         CURLINE
         DISPLAY
         HEX
         HEXSHOW
         HIGHLIGHT
         IDLINE
         IMPMACRO
         IMPOS
         INPUTMODE
         LINEND
         MACRO
         MARGINS
         MSGLINE
         MSGMODE
         NEWLINE
         NUMBER
         POSITION
         PREFIX
         SCALE
         SCOPE
         SHADOW
         STAY
         SYNONYM
         TABLINE
         TABS
         VERIFY
         VERSHIFT
         WORD
         WORDWRAP
         ZONE

     The following file level settings are saved:
         AUTOSAVE
         BACKUP
         COLOUR
         EOLOUT
         TABSOUT

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <RESTORE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Preserve(CHARTYPE *params)
#else
short Preserve(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Preserve");
#endif
/*
 * Don't allow any parameters
 */
 if (!blank_field(params))
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*
 * If we already have preserved settings, don't allocate more
 * memory, just use what's there...
 */
 if (CURRENT_VIEW->preserved_view_details == NULL)
   {
    /*
     * Allocate memory for preserved VIEW and FILE details
     */
    if ((CURRENT_VIEW->preserved_view_details = (PRESERVED_VIEW_DETAILS *)(*the_malloc)(sizeof(PRESERVED_VIEW_DETAILS))) == NULL)
      {
       display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OUT_OF_MEMORY);
      }
    if ((CURRENT_FILE->preserved_file_details = (PRESERVED_FILE_DETAILS *)(*the_malloc)(sizeof(PRESERVED_FILE_DETAILS))) == NULL)
      {
       display_error(30,(CHARTYPE *)"",FALSE);
       (*the_free)(CURRENT_VIEW->preserved_view_details);
       CURRENT_VIEW->preserved_view_details = NULL;
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OUT_OF_MEMORY);
      }
   }
 if ((CURRENT_FILE->preserved_file_details->attr = (COLOUR_ATTR *)(*the_malloc)(ATTR_MAX*sizeof(COLOUR_ATTR))) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
    (*the_free)(CURRENT_VIEW->preserved_view_details);
    CURRENT_VIEW->preserved_view_details = NULL;
    (*the_free)(CURRENT_FILE->preserved_file_details);
    CURRENT_FILE->preserved_file_details = NULL;
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
/*
 * Save the VIEW details...
 */
 CURRENT_VIEW->preserved_view_details->arbchar_status              = CURRENT_VIEW->arbchar_status;
 CURRENT_VIEW->preserved_view_details->arbchar_single              = CURRENT_VIEW->arbchar_single;
 CURRENT_VIEW->preserved_view_details->arbchar_multiple            = CURRENT_VIEW->arbchar_multiple;
 CURRENT_VIEW->preserved_view_details->arrow_on                    = CURRENT_VIEW->arrow_on;
 CURRENT_VIEW->preserved_view_details->case_enter                  = CURRENT_VIEW->case_enter;
 CURRENT_VIEW->preserved_view_details->case_locate                 = CURRENT_VIEW->case_locate;
 CURRENT_VIEW->preserved_view_details->case_change                 = CURRENT_VIEW->case_change;
 CURRENT_VIEW->preserved_view_details->case_sort                   = CURRENT_VIEW->case_sort;
 CURRENT_VIEW->preserved_view_details->cmd_line                    = CURRENT_VIEW->cmd_line;
 CURRENT_VIEW->preserved_view_details->current_row                 = CURRENT_VIEW->current_row;
 CURRENT_VIEW->preserved_view_details->current_base                = CURRENT_VIEW->current_base;
 CURRENT_VIEW->preserved_view_details->current_off                 = CURRENT_VIEW->current_off;
 CURRENT_VIEW->preserved_view_details->display_low                 = CURRENT_VIEW->display_low;
 CURRENT_VIEW->preserved_view_details->display_high                = CURRENT_VIEW->display_high;
 CURRENT_VIEW->preserved_view_details->hex                         = CURRENT_VIEW->hex;
 CURRENT_VIEW->preserved_view_details->hexshow_on                  = CURRENT_VIEW->hexshow_on;
 CURRENT_VIEW->preserved_view_details->hexshow_base                = CURRENT_VIEW->hexshow_base;
 CURRENT_VIEW->preserved_view_details->hexshow_off                 = CURRENT_VIEW->hexshow_off;
 CURRENT_VIEW->preserved_view_details->highlight                   = CURRENT_VIEW->highlight;
 CURRENT_VIEW->preserved_view_details->highlight_high              = CURRENT_VIEW->highlight_high;
 CURRENT_VIEW->preserved_view_details->highlight_low               = CURRENT_VIEW->highlight_low;
 CURRENT_VIEW->preserved_view_details->id_line                     = CURRENT_VIEW->id_line;
 CURRENT_VIEW->preserved_view_details->imp_macro                   = CURRENT_VIEW->imp_macro;
 CURRENT_VIEW->preserved_view_details->imp_os                      = CURRENT_VIEW->imp_os;
 CURRENT_VIEW->preserved_view_details->inputmode                   = CURRENT_VIEW->inputmode;
 CURRENT_VIEW->preserved_view_details->linend_status               = CURRENT_VIEW->linend_status;
 CURRENT_VIEW->preserved_view_details->linend_value                = CURRENT_VIEW->linend_value;
 CURRENT_VIEW->preserved_view_details->macro                       = CURRENT_VIEW->macro;
 CURRENT_VIEW->preserved_view_details->margin_left                 = CURRENT_VIEW->margin_left;
 CURRENT_VIEW->preserved_view_details->margin_right                = CURRENT_VIEW->margin_right;
 CURRENT_VIEW->preserved_view_details->margin_indent               = CURRENT_VIEW->margin_indent;
 CURRENT_VIEW->preserved_view_details->margin_indent_offset_status = CURRENT_VIEW->margin_indent_offset_status;
 CURRENT_VIEW->preserved_view_details->msgline_base                = CURRENT_VIEW->msgline_base;
 CURRENT_VIEW->preserved_view_details->msgline_off                 = CURRENT_VIEW->msgline_off;
 CURRENT_VIEW->preserved_view_details->msgline_rows                = CURRENT_VIEW->msgline_rows;
 CURRENT_VIEW->preserved_view_details->msgmode_status              = CURRENT_VIEW->msgmode_status;
 CURRENT_VIEW->preserved_view_details->newline_aligned             = CURRENT_VIEW->newline_aligned;
 CURRENT_VIEW->preserved_view_details->number                      = CURRENT_VIEW->number;
 CURRENT_VIEW->preserved_view_details->position_status             = CURRENT_VIEW->position_status;
 CURRENT_VIEW->preserved_view_details->prefix                      = CURRENT_VIEW->prefix;
 CURRENT_VIEW->preserved_view_details->prefix_width                = CURRENT_VIEW->prefix_width;
 CURRENT_VIEW->preserved_view_details->prefix_gap                  = CURRENT_VIEW->prefix_gap;
 CURRENT_VIEW->preserved_view_details->scale_on                    = CURRENT_VIEW->scale_on;
 CURRENT_VIEW->preserved_view_details->scale_base                  = CURRENT_VIEW->scale_base;
 CURRENT_VIEW->preserved_view_details->scale_off                   = CURRENT_VIEW->scale_off;
 CURRENT_VIEW->preserved_view_details->scope_all                   = CURRENT_VIEW->scope_all;
 CURRENT_VIEW->preserved_view_details->shadow                      = CURRENT_VIEW->shadow;
 CURRENT_VIEW->preserved_view_details->stay                        = CURRENT_VIEW->stay;
 CURRENT_VIEW->preserved_view_details->synonym                     = CURRENT_VIEW->synonym;
 CURRENT_VIEW->preserved_view_details->tab_on                      = CURRENT_VIEW->tab_on;
 CURRENT_VIEW->preserved_view_details->tab_base                    = CURRENT_VIEW->tab_base;
 CURRENT_VIEW->preserved_view_details->tab_off                     = CURRENT_VIEW->tab_off;
 CURRENT_VIEW->preserved_view_details->tabsinc                     = CURRENT_VIEW->tabsinc;
 CURRENT_VIEW->preserved_view_details->numtabs                     = CURRENT_VIEW->numtabs;
 CURRENT_VIEW->preserved_view_details->verify_col                  = CURRENT_VIEW->verify_col;
 CURRENT_VIEW->preserved_view_details->verify_start                = CURRENT_VIEW->verify_start;
 CURRENT_VIEW->preserved_view_details->verify_end                  = CURRENT_VIEW->verify_end;
 CURRENT_VIEW->preserved_view_details->word                        = CURRENT_VIEW->word;
 CURRENT_VIEW->preserved_view_details->wordwrap                    = CURRENT_VIEW->wordwrap;
 CURRENT_VIEW->preserved_view_details->zone_start                  = CURRENT_VIEW->zone_start;
 CURRENT_VIEW->preserved_view_details->zone_end                    = CURRENT_VIEW->zone_end;
 memcpy(CURRENT_VIEW->preserved_view_details->tabs,CURRENT_VIEW->tabs,sizeof(CURRENT_VIEW->tabs));
/*
 * Save the FILE details...
 */
 CURRENT_FILE->preserved_file_details->autosave                  = CURRENT_FILE->autosave;
 CURRENT_FILE->preserved_file_details->backup                    = CURRENT_FILE->backup;
 CURRENT_FILE->preserved_file_details->eolout                    = CURRENT_FILE->eolout;
 CURRENT_FILE->preserved_file_details->tabsout_on                = CURRENT_FILE->tabsout_on;
 CURRENT_FILE->preserved_file_details->tabsout_num               = CURRENT_FILE->tabsout_num;
 memcpy(CURRENT_FILE->preserved_file_details->attr,CURRENT_FILE->attr,sizeof(CURRENT_FILE->attr));

#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     prevwindow - switch focus of editing session to another file

SYNTAX
     PREVWindow

DESCRIPTION
     The PREVWINDOW command moves the focus of the editing session to
     the other screen (if <SET SCREEN> 2 is in effect) or to the previous 
     file in the <ring>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <NEXTWINDOW>, <EDIT>, <SET SCREEN>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Prevwindow(CHARTYPE *params)
#else
short Prevwindow(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Prevwindow");
#endif
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (display_screens == 1)
   {
    rc = Xedit((CHARTYPE *)"-");
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 current_screen = (current_screen == 0) ? 1 : 0;
 CURRENT_VIEW = CURRENT_SCREEN.screen_view;
 if (curses_started)
   {
    if (CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
      {
       wattrset(CURRENT_WINDOW_COMMAND,set_colour(CURRENT_FILE->attr+ATTR_CMDLINE));
       touchwin(CURRENT_WINDOW_COMMAND);
       wnoutrefresh(CURRENT_WINDOW_COMMAND);
      }
    if (CURRENT_WINDOW_ARROW != (WINDOW *)NULL)
      {
       wattrset(CURRENT_WINDOW_ARROW,set_colour(CURRENT_FILE->attr+ATTR_ARROW));
       redraw_window(CURRENT_WINDOW_ARROW);
       wnoutrefresh(CURRENT_WINDOW_ARROW);
      }
    if (statarea != (WINDOW *)NULL)
      {
       wattrset(statarea,set_colour(CURRENT_FILE->attr+ATTR_STATAREA));
       redraw_window(statarea);
      }
    if (CURRENT_WINDOW_IDLINE != (WINDOW *)NULL)
      {
       wattrset(CURRENT_WINDOW_IDLINE,set_colour(CURRENT_FILE->attr+ATTR_IDLINE));
       redraw_window(CURRENT_WINDOW_IDLINE);
      }
    if (display_screens > 1
    &&  !horizontal)
      {
       wattrset(divider,set_colour(CURRENT_FILE->attr+ATTR_DIVIDER));
       draw_divider();
       wnoutrefresh(divider);
      }
   }
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 build_screen(current_screen);
 if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
   {
    CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
    build_screen(current_screen);
   }
 display_screen(current_screen);

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     print - send text to default printer or print spooler

SYNTAX
     PRint [target] [n]
     PRint LINE [text]
     PRint STRING [text]
     PRint FORMfeed
     PRint CLOSE

DESCRIPTION
     The PRINT command writes a portion of the current file to the default
     printer or print spooler, or text entered on the command line.
  
     PRINT [<'target'>] ['n']
        Sends text from the file contents up to the <'target'> to the printer
        followed by a CR/LF (DOS) or LF(UNIX) after each line.
        When ['n'] is specified, this sends a formfeed after ['n'] successive
        lines of text.
     PRINT 'LINE' ['text']
        Sends the remainder of the 'text' on the command line to the printer
        followed by a LF(UNIX), CR(MAC) or CR/LF (DOS).
     PRINT 'STRING' ['text']
        Sends the remainder of the 'text' on the command line to the printer
        without any trailing line terminator.
     PRINT 'FORMfeed'
         Sends a formfeed (^L) character to the printer.
     PRINT 'CLOSE'
         Closes the printer spooler.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SET PRINTER>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Print(CHARTYPE *params)
#else
short Print(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define PRT_PARAMS  2
 CHARTYPE *word[PRT_PARAMS+1];
 CHARTYPE strip[PRT_PARAMS];
 unsigned short num_params=0;
 short page_break=0;
 short rc=RC_OK;
 short target_type=TARGET_NORMAL|TARGET_ALL|TARGET_BLOCK_CURRENT|TARGET_SPARE;
 TARGET target;
#if defined(UNIX)
 CHARTYPE *line_term = (CHARTYPE *)"\n";
#else
 CHARTYPE *line_term = (CHARTYPE *)"\n\r";
#endif
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Print");
#endif
/*---------------------------------------------------------------------*/
/* Under Win32, startup the printing system, the first time this       */
/* command is called. This makes startup of THE faster.                */
/*---------------------------------------------------------------------*/
#ifdef WIN32
 if (!StartedPrinter)
 {
    StartTextPrnt();
    StartedPrinter = TRUE;
 }
#endif
/*---------------------------------------------------------------------*/
/* Split parameters up...                                              */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_NONE;
 num_params = param_split(params,word,PRT_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)
   {
    num_params = 1;
    word[0] = (CHARTYPE *)"1";
   }
/*---------------------------------------------------------------------*/
/* If first argument is LINE...                                        */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"line",word[0],4))
   {
    print_line(FALSE,0L,0L,0,(CHARTYPE *)word[1],line_term,0);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* If first argument is STRING...                                      */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"string",word[0],5))
   {
    print_line(FALSE,0L,0L,0,(CHARTYPE *)word[1],(CHARTYPE *)"",0);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* If first argument is FORMFEED...                                    */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"formfeed",word[0],4))
   {
    if (num_params > 1)
      {
       display_error(1,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    print_line(FALSE,0L,0L,0,(CHARTYPE *)"",(CHARTYPE *)"\f",0);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* If first argument is CLOSE...                                       */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"close",word[0],5))
   {
    if (num_params > 1)
      {
       display_error(1,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    print_line(TRUE,0L,0L,0,(CHARTYPE *)"",(CHARTYPE *)"",0);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* ...treat all other options as targets...                            */
/*---------------------------------------------------------------------*/
 initialise_target(&target);
 if ((rc = validate_target(params,&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
    free_target(&target);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
 if (target.spare == (-1))
    page_break = 0;
 else
   {
    if (!valid_positive_integer(strtrunc(target.rt[target.spare].string)))
      {
       display_error(4,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    page_break = atoi((DEFCHAR *)strtrunc(target.rt[target.spare].string));
   }
 print_line(FALSE,target.true_line,target.num_lines,page_break,
            (CHARTYPE *)"",line_term,target.rt[0].target_type);
 free_target(&target);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     put - write part of a file to another

SYNTAX
     PUT [target] [filename]

DESCRIPTION
     The PUT command writes a portion of the current file, defined by
     <'target'> to another file, either explicit or temporary.

     When no 'filename' is supplied the temporary file used for <PUT>
     and <GET> commands is overwritten.

     When a 'filename' is supplied the portion of the file written out
     is appended to the specified file.

     If 'CLIP:' is used in place of 'filename', the portion of the file
     specified by <'target'> is written to the clipboard.
     This option only available for Win32 ports of THE.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <PUTD>, <GET>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Put(CHARTYPE *params)
#else
short Put(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Put");
#endif
 rc = execute_put(params,FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     putd - write part of a file to another and delete

SYNTAX
     PUTD [target] [filename]

DESCRIPTION
     The PUTD command writes a portion of the current file, defined by
     <'target'> to another file, either explicit or temporary, and then
     deletes the lines written.

     When no 'filename' is supplied the temporary file used for <PUT>
     and <GET> commands is overwritten.

     When a 'filename' is supplied the portion of the file written out
     is appended to the specified file.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <PUT>, <GET>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Putd(CHARTYPE *params)
#else
short Putd(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Putd");
#endif
 rc = execute_put(params,TRUE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     qquit - exit from the current file without saving changes

SYNTAX
     QQuit

DESCRIPTION
     The QQUIT command exits the user from the current file, whether
     changes made to the file have been saved or not.

     The previous file in the <ring> then becomes the current file.

     If the current file is the only file in the <ring>, THE terminates.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <QUIT>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Qquit(CHARTYPE *params)
#else
short Qquit(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Qquit");
#endif
/*---------------------------------------------------------------------*/
/* No arguments are allowed; error if any are present.                 */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,(CHARTYPE *)params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 free_view_memory(TRUE,TRUE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     query - display various option settings

SYNTAX
     Query item

DESCRIPTION
     The QUERY command displays the various settings for options set
     by THE.

     For a complete list of 'item's that can be extracted, see the section;
     <QUERY, EXTRACT and STATUS>.

     Results of the QUERY command are displayed at the top of the
     display window, and ignore the setting of <SET MSGLINE>.

COMPATIBILITY
     XEDIT: Compatible functionality, but not all options.
     KEDIT: Compatible functionality, but not all options.

SEE ALSO
     <STATUS>, <MODIFY>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Query(CHARTYPE *params)
#else
short Query(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define QUE_PARAMS  2
 CHARTYPE *word[QUE_PARAMS+1];
 CHARTYPE strip[QUE_PARAMS];
 unsigned short num_params=0;
 register short i=0;
 short itemno=0;
 CHARTYPE save_msgline_base = CURRENT_VIEW->msgline_base;
 short save_msgline_off = CURRENT_VIEW->msgline_off;
 ROWTYPE save_msgline_rows = CURRENT_VIEW->msgline_rows;
 bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
 CHARTYPE item_type=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Query");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_NONE;
 num_params = param_split(params,word,QUE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if ((itemno = find_query_item(word[0],strlen((DEFCHAR *)word[0]),&item_type)) == (-1)
 || !(item_type & QUERY_QUERY))
 {
     display_error(1,params,FALSE);
#ifdef THE_TRACE
     trace_return();
#endif
     return(RC_INVALID_OPERAND);
 }

 itemno = get_item_values(1,itemno,(CHARTYPE *)word[1],QUERY_QUERY,0L,NULL,0L);
 /*
  * Save the current position and size of the message line so we can
  * restore it. Do it after we have queried the status, otherwise
  * the status of msgline will be stuffed!
  */
 CURRENT_VIEW->msgline_base   = POSITION_TOP;
 CURRENT_VIEW->msgline_off    = 1;
 CURRENT_VIEW->msgline_rows   = min(terminal_lines-1,itemno);
 CURRENT_VIEW->msgmode_status = TRUE;
 if (itemno != EXTRACT_ARG_ERROR
 &&  itemno != EXTRACT_VARIABLES_SET)
   {
    strcpy((DEFCHAR *)temp_cmd,"");
    for (i=0;i<itemno+1;i++)
      {
       strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)item_values[i].value);
       strcat((DEFCHAR *)temp_cmd," ");
      }
    display_error(0,temp_cmd,TRUE);
   }
 CURRENT_VIEW->msgline_base   = save_msgline_base;
 CURRENT_VIEW->msgline_off    = save_msgline_off;
 CURRENT_VIEW->msgline_rows   = save_msgline_rows;
 CURRENT_VIEW->msgmode_status = save_msgmode_status;
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     quit - exit from the current file if no changes made

SYNTAX
     QUIT

DESCRIPTION
     The QUIT command exits the user from the current file, provided
     that any changes made to the file have been saved, otherwise an
     error message is displayed.

     The previous file in the <ring> then becomes the current file.

     If the current file is the only file in the <ring>, THE terminates.

COMPATIBILITY
     XEDIT: Does not support return code option.
     KEDIT: Compatible.

SEE ALSO
     <QQUIT>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Quit(CHARTYPE *params)
#else
short Quit(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Quit");
#endif
/*---------------------------------------------------------------------*/
/* No arguments are allowed; error if any are present.                 */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,(CHARTYPE *)params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if (CURRENT_FILE->save_alt > 0)
   {
    display_error(22,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_FILE_CHANGED);
   }
 free_view_memory(TRUE,TRUE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     readv - read keystrokes and pass to macro

SYNTAX
     READV Cmdline [initial text]
     READV KEY

DESCRIPTION
     The READV command allows a Rexx macro to interact with the user
     by accepting either individual keystrokes ('KEY') or a complete
     line of text ('Cmdline').

     The READV 'Cmdline' can take optional 'initial text' to be
     displayed on the command line.

     The 'macro' obtains the entered information by setting Rexx
     variables. These are set as follows.

     'KEY' option:

          readv.0 = 4
          readv.1 = name of key (empty if unknown)
          readv.2 = ASCII value of key (null if not an ASCII code)
          readv.3 = curses key value (or ASCII code if an ASCII code)
          readv.4 = shift status (see below)

     'CMDLINE' option:

          readv.0 = 1
          readv.1 = contents of command line

     While editting the command in READV 'Cmdline', any key redefinitions
     you have made will be in effect.  Therefore you can use your
     "normal" editting keys to edit the line.  THE will allow the 
     following commands to be executed while in READV 'Cmdline':

          <CURSOR> LEFT, <CURSOR> RIGHT, <CURSOR> DOWN, <CURSOR> UP,
          <SOS FIRSTCHAR>, <SOS ENDCHAR>, <SOS STARTENDCHAR>,
          <SOS DELEND>, <SOS DELCHAR>, <SOS DELCHAR>,
          <SOS TABB>, <SOS TABF>, <SOS TABWORDB>, <SOS TABWORDF>,
          <SOS UNDO>, <SOS DELWORD>, <SET INSERTMODE>, <TEXT>

     Either of the keys, ENTER, RETURN and NUMENTER will terminate
     READV 'Cmdline', irrespective of what THE commands have been
     assigned.

     The shift status of the key is an eight character string of
     0 or 1; each position represented by the following.

          Position
             1      1 if INSERTMODE is ON
             2      always 0
             3      always 0
             4      always 0
             5      1 if ALT key pressed
             6      1 if CTRL key pressed
             7      1 if SHIFT key pressed
             8      same as position 7

COMPATIBILITY
     XEDIT: Similar to READ CMDLINE option.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Readv(CHARTYPE *params)
#else
short Readv(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define REA_PARAMS  2
   CHARTYPE *word[REA_PARAMS+1];
   CHARTYPE strip[REA_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK,itemno=0,num_values=0;
   unsigned short y=0,x=0;
   CHARTYPE item_type=0;
   bool cursor_on_cmdline=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function( "comm4.c:   Readv" );
#endif
   if ( !in_macro
   ||   !rexx_support )
   {
      display_error( 53, (CHARTYPE *)"", FALSE );
#ifdef THE_TRACE
      trace_return();
#endif
      return( RC_INVALID_ENVIRON );
   }
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_NONE;
   num_params = param_split( params, word, REA_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );
   if ( num_params == 0 )
   {
      display_error( 1, params, FALSE );
#ifdef THE_TRACE
      trace_return();
#endif
      return( RC_INVALID_OPERAND );
   }

   getyx( CURRENT_WINDOW, y, x );
   (void)THERefresh( (CHARTYPE *)"" );
#if defined(USE_EXTCURSES)
   getyx( CURRENT_WINDOW, y, x );
   wmove( CURRENT_WINDOW, y, x );
   wrefresh( CURRENT_WINDOW );
#endif
   if ( equal( (CHARTYPE *)"key", word[0], 3) )
   {
      /*
       * Find the item in the list of valid extract options...
       */
      if ( ( itemno = find_query_item( (CHARTYPE *)"READV", 5, &item_type ) ) == (-1) )
      {
         display_error( 1, params, FALSE );
#ifdef THE_TRACE
         trace_return();
#endif
         return( RC_INVALID_OPERAND );
      }
      /*
       * Get the current settings for the valid item...
       */
      num_values = get_item_values( 1, itemno, NULL, QUERY_READV, 0L, NULL, 0L );
      /*
       * If the arguments to the item are invalid, return with an error.
       */
       if ( num_values == EXTRACT_ARG_ERROR )
      {
#ifdef THE_TRACE
          trace_return();
#endif
          return( RC_INVALID_OPERAND );
      }
      /*
       * If the Rexx variables have already been set, don't try to set them.
       */
      if ( num_values != EXTRACT_VARIABLES_SET )
         rc = set_extract_variables( itemno );
      if ( error_on_screen )
         clear_msgline( -1 );
   }
   else
   {
      if ( equal( (CHARTYPE *)"cmdline", word[0], 1 ) )
      {
         if ( CURRENT_VIEW->current_window == WINDOW_COMMAND )
            cursor_on_cmdline = TRUE;
         rc = readv_cmdline( word[1], NULL, -1 );
         set_rexx_variable( (CHARTYPE *)"READV", cmd_rec, cmd_rec_len, 1 );
         set_rexx_variable( (CHARTYPE *)"READV", (CHARTYPE *)"1", 1, 0 );
         wmove(CURRENT_WINDOW_COMMAND,0,0);
         my_wclrtoeol(CURRENT_WINDOW_COMMAND);
         memset(cmd_rec,' ',max_line_length);
         cmd_rec_len = 0;
         wmove(CURRENT_WINDOW_COMMAND,0,0);
         if ( !cursor_on_cmdline )
            THEcursor_home( TRUE );
      }
      else
      {
         display_error( 1, word[0], FALSE );
#ifdef THE_TRACE
         trace_return();
#endif
         return( RC_INVALID_OPERAND );
      }
   }
   initial = FALSE;
#ifdef THE_TRACE
   trace_return();
#endif
   return( rc );
}
/*man-start*********************************************************************
COMMAND
     recover - recover changed or deleted lines

SYNTAX
     RECover [n|*]

DESCRIPTION
     The RECOVER command restores the last 'n', or all '*' changed or 
     deleted lines back into the body of the file.

COMPATIBILITY
     XEDIT: Also recovers changes to lines, not just lines deleted.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Recover(CHARTYPE *params)
#else
short Recover(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define REC_PARAMS  2
 CHARTYPE strip[REC_PARAMS];
 CHARTYPE *word[REC_PARAMS+1];
 unsigned short num_params=0;
 short num=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Recover");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied. The one and only   */
/* parameter should be a positive integer greater than zero or '*'.    */
/* If no parameter is supplied, 1 is assumed.                          */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,REC_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 switch(num_params)
   {
    case 0:
         num = 1;
         break;
    case 1:
         if (strcmp((DEFCHAR *)word[0],"*") == 0)
            num = 99;
         else
           {
            if (!valid_positive_integer(word[0]))
              {
               display_error(4,word[0],FALSE);
#ifdef THE_TRACE
               trace_return();
#endif
               return(RC_INVALID_OPERAND);
              }
            num = atoi((DEFCHAR *)word[0]);
           }
         break;
    default:
         display_error(1,word[1],FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_INVALID_OPERAND);
    }
 get_from_recovery_list(num);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     redraw - redraw the current screen

SYNTAX
     REDRAW

DESCRIPTION
     The REDRAW command redraws the current contents of the screen.
     This is usually used when some outside influence has affected 
     the display.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <REFRESH>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Redraw(CHARTYPE *params)
#else
short Redraw(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Redraw");
#endif
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 erase();
 refresh();
 restore_THE();
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     refresh - refresh the contents of the current screen

SYNTAX
     REFRESH

DESCRIPTION
     The REFRESH command refreshes what is being displayed on the screen.
     This is usually used from within a <macro> to indicate the progress
     of the <macro>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <REDRAW>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THERefresh(CHARTYPE *params)
#else
short THERefresh(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 bool save_in_macro=in_macro;
 unsigned short y=0,x=0;
 LINETYPE new_focus_line=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   THERefresh");
#endif
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (!curses_started)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_ENVIRON);
   }
 getyx(CURRENT_WINDOW,y,x);
 in_macro = FALSE;
 if (display_screens > 1)
   {
    prepare_view(other_screen);
    display_screen(other_screen);
    if (!horizontal)
      {
       touchwin(divider);
       wnoutrefresh(divider);
      }
   }
 show_statarea();
#if defined(HAVE_SLK_INIT)
 if (SLKx)
   {
    slk_touch();
    slk_noutrefresh();
   }
#endif
 CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base,
                                                  CURRENT_VIEW->current_off,
                                                  CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE);
 build_screen(current_screen); 
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
    if (curses_started)
       getyx(CURRENT_WINDOW,y,x);
    if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
      {
       new_focus_line = get_focus_line_in_view(current_screen,CURRENT_VIEW->focus_line,y);
       post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
       CURRENT_VIEW->focus_line = new_focus_line;
       pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      }
    y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,CURRENT_VIEW->current_row);
    if (curses_started)
       wmove(CURRENT_WINDOW,y,x);
   }
 display_screen(current_screen);
 if (error_on_screen)
   expose_msgline();
 wmove(CURRENT_WINDOW,y,x);
 wrefresh(CURRENT_WINDOW);
 in_macro = save_in_macro;
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     repeat - repeat the last command

SYNTAX
     REPEat [target]

DESCRIPTION
     The REPEAT command advances the current line and executes the
     last command. It is equivalent to <NEXT> 1 (or <UP> 1) and <=> for
     the specified number of times specified by <'target'>.

     To determine how many lines on which to execute the last command,
     THE uses the target to determine how many lines from the current
     position to the target.  This is the number of times the last
     command is executed.

     If the last command to be executed, changes the current line,
     (because it has a target specification), the next execution of
     the last command will begin from where the previous execution of
     last command ended.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Repeat(CHARTYPE *params)
#else
short Repeat(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE num_lines=0L;
 short rc=RC_OK;
 short direction=0;
 TARGET target;
 short target_type=TARGET_NORMAL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Repeat");
#endif
 if (strcmp("",(DEFCHAR *)params) == 0)
    params = (CHARTYPE *)"+1";
 initialise_target(&target);
 if ((rc = validate_target(params,&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
    free_target(&target);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*---------------------------------------------------------------------*/
/* Determine in which direction we are working.                        */
/*---------------------------------------------------------------------*/
 if (target.num_lines < 0L)
   {
    direction = DIRECTION_BACKWARD;
    num_lines = target.num_lines * (-1L);
   }
 else
   {
    direction = DIRECTION_FORWARD;
    num_lines = target.num_lines;
   }
 free_target(&target);
/*---------------------------------------------------------------------*/
/* Repeat the last command until the number of lines has been reached  */
/* or the last command returns non-zero.                               */
/*---------------------------------------------------------------------*/
 in_repeat = TRUE;

 while(num_lines-- > 0)
   {
    rc = advance_current_or_focus_line((LINETYPE)direction);
    if (rc != RC_OK)
       break;
    rc = command_line(last_command_for_repeat,COMMAND_ONLY_FALSE);
    if (rc != RC_OK)
        break;
   }
 in_repeat = FALSE;
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     replace - replace the current line with supplied text

SYNTAX
     Replace [text]

DESCRIPTION
     The REPLACE command replaces the <focus line> with the supplied
     'text'.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Replace(CHARTYPE *params)
#else
short Replace(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short len_params=0;
 LINETYPE true_line=0L;
 LINE *curr=NULL;
 SELECTTYPE current_select=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Replace");
#endif

 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if (CURRENT_VIEW->hex)
   {
    if ((len_params = convert_hex_strings(params)) == (-1))
      {
       display_error(32,params,FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
   }
 else
   len_params = strlen((DEFCHAR *)params);
 true_line = get_true_line(TRUE);
 if (TOF(true_line)
 ||  BOF(true_line))
   {
    display_error(38,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY); /* ?? */
   }
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
 current_select = curr->select;
 add_to_recovery_list(curr->line,curr->length);
 curr = delete_LINE(CURRENT_FILE->first_line,CURRENT_FILE->last_line,curr,DIRECTION_FORWARD);
 curr = curr->prev;
 if ((curr = add_LINE(CURRENT_FILE->first_line,curr,
                          params,len_params,current_select,TRUE)) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 increment_alt(CURRENT_FILE);

 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);

 build_screen(current_screen); 
 display_screen(current_screen);

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     reset - cancel the marked block or prefix commands or both

SYNTAX
     RESet ALL|Block|Prefix

DESCRIPTION
     The RESET command unmarks any marked <block> or outstanding prefix
     commands or both.

COMPATIBILITY
     XEDIT: Adds Block and All options.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Reset(CHARTYPE *params)
#else
short Reset(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define RES_PARAMS  1
 CHARTYPE *word[RES_PARAMS+1];
 CHARTYPE strip[RES_PARAMS];
 unsigned short num_params=0;
 THE_PPC *curr_ppc=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Reset");
#endif
 strip[0]=STRIP_BOTH;
 num_params = param_split(params,word,RES_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params > 1)
   {
    display_error(1,word[1],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Reset the marked block, if any.                                     */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"block",word[0],1)
 ||  equal((CHARTYPE *)"all",word[0],3)
 ||  num_params == 0)
   {
    if (MARK_VIEW != (VIEW_DETAILS *)NULL)
      {
       MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
       MARK_VIEW = (VIEW_DETAILS *)NULL;
       build_screen(current_screen); 
       display_screen(current_screen);
      }
   }
/*---------------------------------------------------------------------*/
/* Reset the pending prefix commands, if any.                          */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"prefix",word[0],1)
 ||  equal((CHARTYPE *)"all",word[0],3)
 ||  num_params == 0)
   {
    curr_ppc = CURRENT_FILE->first_ppc;
    while(curr_ppc != NULL)
       curr_ppc = delete_pending_prefix_command(curr_ppc,CURRENT_FILE,(LINE *)NULL);
    memset(pre_rec,' ',MAX_PREFIX_WIDTH+1);
    pre_rec_len = 0;
    build_screen(current_screen); 
    display_screen(current_screen);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     restore - restore various editor settings

SYNTAX
     REStore

DESCRIPTION
     The RESTORE command restores various editing settings at the time
     the command is issued.  These settings must have been saved with
     the <PRESERVE> command.

     If an attempt is made to restore settings that have not been preserved,
     an error results.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <PRESERVE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Restore(CHARTYPE *params)
#else
short Restore(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 bool rebuild_screen=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Restore");
#endif
/*
 * Don't allow any parameters
 */
 if (!blank_field(params))
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*
 * If we don't have any preserved settings, return an error.
 */
 if (CURRENT_VIEW->preserved_view_details == NULL)
   {
    display_error(51,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 /*
  * Before putting the settings back, we have to do some special things
  * with PREFIX, ARROW, CMDLINE, ID_LINE, ...
  */
 if (CURRENT_VIEW->prefix != CURRENT_VIEW->preserved_view_details->prefix
 ||  CURRENT_VIEW->prefix_width != CURRENT_VIEW->preserved_view_details->prefix_width
 ||  CURRENT_VIEW->prefix_gap != CURRENT_VIEW->preserved_view_details->prefix_gap
 ||  CURRENT_VIEW->arrow_on != CURRENT_VIEW->preserved_view_details->arrow_on
 ||  CURRENT_VIEW->cmd_line != CURRENT_VIEW->preserved_view_details->cmd_line
 ||  CURRENT_VIEW->id_line != CURRENT_VIEW->preserved_view_details->id_line)
    rebuild_screen = TRUE;
 /*
  * Restore the VIEW details...
  */
 CURRENT_VIEW->arbchar_status              = CURRENT_VIEW->preserved_view_details->arbchar_status;
 CURRENT_VIEW->arbchar_single              = CURRENT_VIEW->preserved_view_details->arbchar_single;
 CURRENT_VIEW->arbchar_multiple            = CURRENT_VIEW->preserved_view_details->arbchar_multiple;
 CURRENT_VIEW->arrow_on                    = CURRENT_VIEW->preserved_view_details->arrow_on;
 CURRENT_VIEW->case_enter                  = CURRENT_VIEW->preserved_view_details->case_enter;
 CURRENT_VIEW->case_locate                 = CURRENT_VIEW->preserved_view_details->case_locate;
 CURRENT_VIEW->case_change                 = CURRENT_VIEW->preserved_view_details->case_change;
 CURRENT_VIEW->case_sort                   = CURRENT_VIEW->preserved_view_details->case_sort;
 CURRENT_VIEW->cmd_line                    = CURRENT_VIEW->preserved_view_details->cmd_line;
 CURRENT_VIEW->current_row                 = CURRENT_VIEW->preserved_view_details->current_row;
 CURRENT_VIEW->current_base                = CURRENT_VIEW->preserved_view_details->current_base;
 CURRENT_VIEW->current_off                 = CURRENT_VIEW->preserved_view_details->current_off;
 CURRENT_VIEW->display_low                 = CURRENT_VIEW->preserved_view_details->display_low;
 CURRENT_VIEW->display_high                = CURRENT_VIEW->preserved_view_details->display_high;
 CURRENT_VIEW->hex                         = CURRENT_VIEW->preserved_view_details->hex;
 CURRENT_VIEW->hexshow_on                  = CURRENT_VIEW->preserved_view_details->hexshow_on;
 CURRENT_VIEW->hexshow_base                = CURRENT_VIEW->preserved_view_details->hexshow_base;
 CURRENT_VIEW->hexshow_off                 = CURRENT_VIEW->preserved_view_details->hexshow_off;
 CURRENT_VIEW->highlight                   = CURRENT_VIEW->preserved_view_details->highlight;
 CURRENT_VIEW->highlight_high              = CURRENT_VIEW->preserved_view_details->highlight_high;
 CURRENT_VIEW->highlight_low               = CURRENT_VIEW->preserved_view_details->highlight_low;
 CURRENT_VIEW->id_line                     = CURRENT_VIEW->preserved_view_details->id_line;
 CURRENT_VIEW->imp_macro                   = CURRENT_VIEW->preserved_view_details->imp_macro;
 CURRENT_VIEW->imp_os                      = CURRENT_VIEW->preserved_view_details->imp_os;
 CURRENT_VIEW->inputmode                   = CURRENT_VIEW->preserved_view_details->inputmode;
 CURRENT_VIEW->linend_status               = CURRENT_VIEW->preserved_view_details->linend_status;
 CURRENT_VIEW->linend_value                = CURRENT_VIEW->preserved_view_details->linend_value;
 CURRENT_VIEW->macro                       = CURRENT_VIEW->preserved_view_details->macro;
 CURRENT_VIEW->margin_left                 = CURRENT_VIEW->preserved_view_details->margin_left;
 CURRENT_VIEW->margin_right                = CURRENT_VIEW->preserved_view_details->margin_right;
 CURRENT_VIEW->margin_indent               = CURRENT_VIEW->preserved_view_details->margin_indent;
 CURRENT_VIEW->margin_indent_offset_status = CURRENT_VIEW->preserved_view_details->margin_indent_offset_status;
 CURRENT_VIEW->msgline_base                = CURRENT_VIEW->preserved_view_details->msgline_base;
 CURRENT_VIEW->msgline_off                 = CURRENT_VIEW->preserved_view_details->msgline_off;
 CURRENT_VIEW->msgline_rows                = CURRENT_VIEW->preserved_view_details->msgline_rows;
 CURRENT_VIEW->msgmode_status              = CURRENT_VIEW->preserved_view_details->msgmode_status;
 CURRENT_VIEW->newline_aligned             = CURRENT_VIEW->preserved_view_details->newline_aligned;
 CURRENT_VIEW->number                      = CURRENT_VIEW->preserved_view_details->number;
 CURRENT_VIEW->position_status             = CURRENT_VIEW->preserved_view_details->position_status;
 CURRENT_VIEW->prefix                      = CURRENT_VIEW->preserved_view_details->prefix;
 CURRENT_VIEW->prefix_width                = CURRENT_VIEW->preserved_view_details->prefix_width;
 CURRENT_VIEW->prefix_gap                  = CURRENT_VIEW->preserved_view_details->prefix_gap;
 CURRENT_VIEW->scale_on                    = CURRENT_VIEW->preserved_view_details->scale_on;
 CURRENT_VIEW->scale_base                  = CURRENT_VIEW->preserved_view_details->scale_base;
 CURRENT_VIEW->scale_off                   = CURRENT_VIEW->preserved_view_details->scale_off;
 CURRENT_VIEW->scope_all                   = CURRENT_VIEW->preserved_view_details->scope_all;
 CURRENT_VIEW->shadow                      = CURRENT_VIEW->preserved_view_details->shadow;
 CURRENT_VIEW->stay                        = CURRENT_VIEW->preserved_view_details->stay;
 CURRENT_VIEW->synonym                     = CURRENT_VIEW->preserved_view_details->synonym;
 CURRENT_VIEW->tab_on                      = CURRENT_VIEW->preserved_view_details->tab_on;
 CURRENT_VIEW->tab_base                    = CURRENT_VIEW->preserved_view_details->tab_base;
 CURRENT_VIEW->tab_off                     = CURRENT_VIEW->preserved_view_details->tab_off;
 CURRENT_VIEW->tabsinc                     = CURRENT_VIEW->preserved_view_details->tabsinc;
 CURRENT_VIEW->numtabs                     = CURRENT_VIEW->preserved_view_details->numtabs;
 CURRENT_VIEW->verify_col                  = CURRENT_VIEW->preserved_view_details->verify_col;
 CURRENT_VIEW->verify_start                = CURRENT_VIEW->preserved_view_details->verify_start;
 CURRENT_VIEW->verify_end                  = CURRENT_VIEW->preserved_view_details->verify_end;
 CURRENT_VIEW->word                        = CURRENT_VIEW->preserved_view_details->word;
 CURRENT_VIEW->wordwrap                    = CURRENT_VIEW->preserved_view_details->wordwrap;
 CURRENT_VIEW->zone_start                  = CURRENT_VIEW->preserved_view_details->zone_start;
 CURRENT_VIEW->zone_end                    = CURRENT_VIEW->preserved_view_details->zone_end;
 memcpy(CURRENT_VIEW->tabs,CURRENT_VIEW->preserved_view_details->tabs,sizeof(CURRENT_VIEW->preserved_view_details->tabs)); /* FGC */
/*
 * Restore the FILE details...
 */
 CURRENT_FILE->autosave                  = CURRENT_FILE->preserved_file_details->autosave;
 CURRENT_FILE->backup                    = CURRENT_FILE->preserved_file_details->backup;
 CURRENT_FILE->eolout                    = CURRENT_FILE->preserved_file_details->eolout;
 CURRENT_FILE->tabsout_on                = CURRENT_FILE->preserved_file_details->tabsout_on;
 CURRENT_FILE->tabsout_num               = CURRENT_FILE->preserved_file_details->tabsout_num;
 memcpy(CURRENT_FILE->attr,CURRENT_FILE->preserved_file_details->attr,sizeof(CURRENT_FILE->preserved_file_details->attr));
 /*
  * Free any memory for preserved VIEW and FILE details
  */
 (*the_free)(CURRENT_VIEW->preserved_view_details);
 CURRENT_VIEW->preserved_view_details = NULL;
 (*the_free)(CURRENT_FILE->preserved_file_details->attr);
 CURRENT_FILE->preserved_file_details->attr = NULL;
 (*the_free)(CURRENT_FILE->preserved_file_details);
 CURRENT_FILE->preserved_file_details = NULL;
 /*
  * Now that all the settings are back in place apply any screen
  * changes...
  */
 if (rebuild_screen)
   {
    set_screen_defaults();
    if (set_up_windows(current_screen) != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OK);
      }
    build_screen(current_screen);
    display_screen(current_screen);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     rexx - execute Rexx instructions

SYNTAX
     REXX rexx instructions

DESCRIPTION
     The REXX command allows  the user to enter Rexx instructions
     from the command line.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <MACRO>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THERexx(CHARTYPE *params)
#else
short THERexx(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 short macrorc=0;
 bool save_in_macro=in_macro;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   THERexx");
#endif
#ifdef NOREXX
#else
 if (rexx_support)
   {
/*---------------------------------------------------------------------*/
/* Set in_macro = TRUE to stop multiple show_page()s being performed.  */
/*---------------------------------------------------------------------*/
    in_macro = TRUE;
/*---------------------------------------------------------------------*/
/* Save the values of the cursor position for EXTRACT command..        */
/*---------------------------------------------------------------------*/
    get_cursor_position(&original_screen_line,&original_screen_column,
                     &original_file_line,&original_file_column);
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
    rc = execute_macro_instore(params, &macrorc, NULL, NULL, NULL, 0);
/*---------------------------------------------------------------------*/
/* Set in_macro = FALSE to indicate we are out of the macro and do a   */
/* show_page() now as long as there are still file(s) in the ring.     */
/*---------------------------------------------------------------------*/
    in_macro = save_in_macro;
    if (number_of_files > 0)
      {
       if (display_screens > 1)
         {
          build_screen(other_screen);
          display_screen(other_screen);
         }
       build_screen(current_screen);
       display_screen(current_screen);
      }
   }
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     rgtleft - scroll the screen to the left or right

SYNTAX
     RGTLEFT [n]

DESCRIPTION
     The RGTLEFT command scrolls the screen 'n' columns to the right
     if the value of <vershift> is less than or equal to 0, or if
     the value of <vershift> is greater than 0, the screen is
     scrolled 'n' columns to the left.

     If 'n' is not specified, the screen scrolls by three quarters the
     number of columns displayed.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <LEFT>, <RIGHT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Rgtleft(CHARTYPE *params)
#else
short Rgtleft(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LINETYPE shift_val=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Rgtleft");
#endif
/*---------------------------------------------------------------------*/
/* Validate only parameter, a positive integer. 3/4 if no argument.    */
/*---------------------------------------------------------------------*/
 if (blank_field(params))
    shift_val = min(CURRENT_SCREEN.cols[WINDOW_FILEAREA],
                  1 + CURRENT_VIEW->verify_end - CURRENT_VIEW->verify_start)*3/4;
 else
   {
    if (valid_positive_integer(params))
       shift_val = atol((DEFCHAR *)params);
   }
 if (shift_val == (-1))                            /* invalid argument */
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if ((LINETYPE)CURRENT_VIEW->verify_col - (LINETYPE)CURRENT_VIEW->verify_start > 0)
    shift_val = -shift_val;
 CURRENT_VIEW->verify_col = max(1,CURRENT_VIEW->verify_col+shift_val);
 build_screen(current_screen);
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     right - scroll the screen to the right

SYNTAX
     RIght [n|HALF|FULL]

DESCRIPTION
     The RIGHT command scrolls the screen to the right.

     If 'n' is supplied, the screen scrolls by that many columns.

     If 'HALF' is specified the screen is scrolled by half the number
     of columns in the <filearea>.

     If 'FULL' is specified the screen is scrolled by the number
     of columns in the <filearea>.

     If no parameter is supplied, the screen is scrolled by one
     column. 

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <LEFT>, <RGTLEFT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Right(CHARTYPE *params)
#else
short Right(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LINETYPE shift_val=-1L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Right");
#endif
/*---------------------------------------------------------------------*/
/* Validate only parameter, HALF or positive integer. 1 if no argument.*/
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"half",params,4))
    shift_val = CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2;
 if (equal((CHARTYPE *)"full",params,4))
    shift_val = CURRENT_SCREEN.cols[WINDOW_FILEAREA];
 if (blank_field(params))
    shift_val = 1L;
 if (shift_val == (-1))         /* argument not HALF,FULL or empty ... */
   {
    if (valid_positive_integer(params))
      {
       shift_val = atol((DEFCHAR *)params);
       if (shift_val != 0)
          shift_val = shift_val;
      }
   }
 if (shift_val == (-1))                            /* invalid argument */
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* If the argument is 0, restore the original verify columns display.  */
/*---------------------------------------------------------------------*/
 if (shift_val == 0L)
    CURRENT_VIEW->verify_col = CURRENT_VIEW->verify_start;
 else
    CURRENT_VIEW->verify_col = max(1,CURRENT_VIEW->verify_col+shift_val);
#ifdef MSWIN
 Win31HScroll(CURRENT_VIEW->verify_col);
#endif
 build_screen(current_screen);
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     save - save changes to current file

SYNTAX
     SAVE [filename]

DESCRIPTION
     The SAVE command writes the current file to disk. If a 'filename' is
     supplied, the current file is saved in that file, unless the file
     already exists which will result in an error message being
     displayed. Both 'Alterations' counters on the <idline> are
     reset to zero.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SSAVE>, <FILE>, <FFILE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Save(CHARTYPE *params)
#else
short Save(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Save");
#endif
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if ((rc = save_file(CURRENT_FILE,params,FALSE,CURRENT_FILE->number_lines,1L,NULL,FALSE,0,max_line_length,TRUE,FALSE,FALSE)) != RC_OK)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Only set the alteration count to zero if save was successful.       */
/*---------------------------------------------------------------------*/
 CURRENT_FILE->autosave_alt = CURRENT_FILE->save_alt = 0;
/*---------------------------------------------------------------------*/
/* If autosave is on at the time of Saving, remove the .aus file...    */
/*---------------------------------------------------------------------*/
 if (CURRENT_FILE->autosave > 0)
    rc = remove_aus_file(CURRENT_FILE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     schange - selectively change strings

SYNTAX
     SCHange /string1/string2/ [target] [n] [m]

DESCRIPTION
     The SCHANGE command changes one string of text to another only
     after confirming each individual change with the user.

     The first parameter to the change command is the old and new
     string values, seperated by delimiters.
     The allowable delimiters are '/' '\' and '@'.

     The second parameter is the <target>; how many lines are to be                                                       
     searched for occurrences of 'string1' to be changed.

     'n' determines how many occurrences of 'string1' are to be 
     changed to 'string2' on each line.

     'm' determines from which occurrence of 'string1' on the line 
     changes are to commence.

COMPATIBILITY
     XEDIT: Functionally compatible, but syntax different.
     KEDIT: Compatible.

DEFAULT
     1 1 1

SEE ALSO
     <CHANGE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Schange(CHARTYPE *params)
#else
short Schange(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Schange");
#endif
 rc = execute_change_command(params,TRUE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set - execute various set commands

SYNTAX
     SET set_command [set_command parameter(s) ...]

DESCRIPTION
     The SET command is a front end to existing <SET> commands. It treats
     the first parameter it receives as a command and executes it.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Set(CHARTYPE *params)
#else
short Set(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Set");
#endif
 rc = execute_set_sos_command(TRUE,params);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     shift - move text left or right

SYNTAX
     SHift Left|Right [n] [target]

DESCRIPTION
     The SHIFT command moves text in the direction specified the number
     of columns 'n' for the specified <'target'> lines.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Shift(CHARTYPE *params)
#else
short Shift(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define SHI_PARAMS  3
 CHARTYPE *word[SHI_PARAMS+1];
 CHARTYPE strip[SHI_PARAMS];
 short shift_left=(-1);
 LINETYPE num_lines=0L,true_line=0L;
 short num_cols=0,num_params=0;
 short rc=RC_OK;
 short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
 short save_target_type=TARGET_UNFOUND;
 TARGET target;
 bool num_lines_based_on_scope=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Shift");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_NONE;
 num_params = param_split(params,word,SHI_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)                                     /* no params */
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Validate first parameter:                                           */
/*    must be Left or Right                                            */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"left",word[0],1))
     shift_left = TRUE;
 else if (equal((CHARTYPE *)"right",word[0],1))
     shift_left = FALSE;
 else 
   {
    display_error(1,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Validate second parameter (if there is one)                         */
/*    If present, must be valid positive integer.                      */
/*    If not present, default to 1.                                    */
/*---------------------------------------------------------------------*/
 if (num_params < 2)
    num_cols = 1;
 else
   {
    if (!valid_positive_integer(strtrunc(word[1])))
      {
       display_error(4,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    num_cols = atoi((DEFCHAR *)word[1]);
   }
/*---------------------------------------------------------------------*/
/* Validate third  parameter (if there is one)                         */
/*    If present, must be valid target.                                */
/*    If not present, default to 1.                                    */
/*---------------------------------------------------------------------*/
 if (num_params < 3)                                      /* no target */
   {
    num_lines = 1L;
    true_line = get_true_line(TRUE);
   }
 else
   {
    initialise_target(&target);
    if ((rc = validate_target(word[2],&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
      {
       free_target(&target);
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
    if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
      {
       if (MARK_VIEW->mark_type == M_STREAM
       &&  target.num_lines > 1)
         {
          display_error(62,(CHARTYPE*)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
      }
    num_lines = target.num_lines;
    true_line = target.true_line;
    save_target_type = target.rt[0].target_type;
    num_lines_based_on_scope = (save_target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
    free_target(&target);
   }
/*---------------------------------------------------------------------*/
/* Now we are here, everything's OK, do the actual shift...            */
/*---------------------------------------------------------------------*/
 rc = execute_shift_command(shift_left,num_cols,true_line,num_lines,num_lines_based_on_scope,save_target_type,FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     showkey - display current key value and command assignation

SYNTAX
     SHOWkey [ALL]

DESCRIPTION

     With no parameter, the SHOWKEY command prompts the user to enter 
     a key and responds with the key name and associated command 
     (if applicable).
     To exit from SHOWKEY, press the space bar.

     With 'ALL' specified, a new file is added to the <ring> with all
     default key mappings and any key mappings assigned with the <DEFINE> 
     command shown.  The key mappings are displayed as <DEFINE> commands.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short ShowKey(CHARTYPE *params)
#else
short ShowKey(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int key=0;
 short rc=RC_OK;
 bool mouse_key=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   ShowKey");
#endif
/*---------------------------------------------------------------------*/
/* If no arguments, show key definitions as prompted.                  */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)params,"") == 0)
   {
    display_prompt((CHARTYPE *)"Press the key to be translated...spacebar to exit");
    key = 0;
    while(key != ' ')
      {
       while(1)
         {
#ifdef CAN_RESIZE
          if (is_termresized())
            {
             (void)THE_Resize(0,0);
             (void)THERefresh((CHARTYPE *)"");
            }
#endif
          key = my_getch(stdscr);
#if defined(XCURSES)
          if (key == KEY_SF || key == KEY_SR)
             continue;
#endif
#ifdef CAN_RESIZE
          if (is_termresized())
             continue;
#endif     
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
          if (key == KEY_MOUSE)
            {
             int b,ba,bm,w;
             CHARTYPE scrn;
             if (get_mouse_info(&b,&ba,&bm) != RC_OK)
                continue;
             which_window_is_mouse_in(&scrn,&w);
             mouse_key = TRUE;
             key = mouse_info_to_key(w,b,ba,bm);
            }
          else
             mouse_key = FALSE;
#endif
          break;
         }
       clear_msgline(-1);
       display_prompt(get_key_definition(key,FALSE,TRUE,mouse_key));
      }
    clear_msgline(-1);
   }
 else
/*---------------------------------------------------------------------*/
/* If an argument, it must be ALL.                                     */
/*---------------------------------------------------------------------*/
   {
    if (equal((CHARTYPE *)"all",params,3))
       rc = display_all_keys();
    else
      {
       display_error(1,(CHARTYPE *)params,FALSE);
       rc = RC_INVALID_OPERAND;
      }
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sort - sort selected lines in a file

SYNTAX
     SORT target [[sort field 1] [...] [sort field 10]]

DESCRIPTION
     The SORT command sort a portion of a file based on the 'sort field'
     specifications.

     A 'sort field' specification consists of:

          order flag   - [Ascending|Descending]
          left column  - left column of field to sort on
          right column - right column of field to sort on

     The right column MUST be >= left column.

     Only 10 sort fields are allowed.

     <'target'> can be any valid target including ALL, *, -*, and BLOCK.

COMPATIBILITY
     XEDIT: XEDIT only allows ordering flag for all fields
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sort(CHARTYPE *params)
#else
short Sort(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Sort");
#endif
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 rc = execute_sort(params);
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 build_screen(current_screen); 
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos - execute various sos commands

SYNTAX
     SOS sos_command [sos_command ...]

DESCRIPTION
     The SOS command is a front end to existing <SOS> commands. It treats
     each parameter it receives as a command and executes it.

     The SOS command will execute each command until the list of commands
     has been exhausted, or until one of the commands returns a non-zero
     return code.

COMPATIBILITY
     XEDIT: XEDIT only permits 1 command
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos(CHARTYPE *params)
#else
short Sos(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
#define SOS_PARAMS  10
 CHARTYPE strip[SOS_PARAMS];
 CHARTYPE *word[SOS_PARAMS+1];
 short num_params=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Sos");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 strip[3]=STRIP_BOTH;
 strip[4]=STRIP_BOTH;
 strip[5]=STRIP_BOTH;
 strip[6]=STRIP_BOTH;
 strip[7]=STRIP_BOTH;
 strip[8]=STRIP_BOTH;
 strip[9]=STRIP_BOTH;
 num_params = param_split(params,word,SOS_PARAMS,WORD_DELIMS,TEMP_TMP_CMD,strip,FALSE);
 if (num_params == 0)                                     /* no params */
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* For each "command" go an execute it.                                */
/*---------------------------------------------------------------------*/
 for (i=0;i<num_params;i++)
   {
    if ((rc = execute_set_sos_command(FALSE,word[i])) != RC_OK)
       break;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     split - split a line into two lines

SYNTAX
     SPlit [ALigned] [Column|CURSOR]

DESCRIPTION
     The SPLIT command splits the <focus line> into two lines.

     If 'Aligned' is specified, the first non-blank character of the new
     line is positioned under the first non-blank character of the
     <focus line>. 

     If 'Aligned' is not specified, the text of the new line starts in 
     column 1.

     If 'Column' (the default) is specified, the current line is split at 
     the current column location.

     If 'CURSOR' is specified, the focus line is split at the cursor
     position.

COMPATIBILITY
     XEDIT: Compatible.
            Does not support Before/After/Colno options
     KEDIT: Compatible.

SEE ALSO
     <JOIN>, <SPLTJOIN>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Split(CHARTYPE *params)
#else
short Split(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define SPT_PARAMS  2
 CHARTYPE *word[SPT_PARAMS+1];
 CHARTYPE strip[SPT_PARAMS];
 unsigned short num_params=0;
 short rc=RC_OK;
 bool aligned=FALSE;
 bool cursorarg=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Split");
#endif
/*---------------------------------------------------------------------*/
/* Split parameters up...                                              */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,SPT_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)
   {
    aligned = FALSE;
    cursorarg = FALSE;
   }
 else
   {
    if (equal((CHARTYPE *)"aligned",word[0],2))
      {
       aligned = TRUE;
       if (equal((CHARTYPE *)"cursor",word[1],6))
         {
          cursorarg = TRUE;
         }
       else
         {
          if (equal((CHARTYPE *)"column",word[1],1))
            {
             cursorarg = FALSE;
            }
          else
            {
             display_error(1,(CHARTYPE *)word[1],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_ENVIRON);
            }
         }
      }
    else
      {
       if (equal((CHARTYPE *)"cursor",word[0],6))
         {
          aligned = FALSE;
          cursorarg = TRUE;
         }
       else
         {
          if (equal((CHARTYPE *)"column",word[0],1))
            {
             aligned = FALSE;
             cursorarg = FALSE;
            }
          else
            {
             display_error(1,(CHARTYPE *)word[0],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_ENVIRON);
            }
         }
      }
   }
 rc = execute_split_join(SPLTJOIN_SPLIT,aligned,cursorarg);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     spltjoin - split/join two lines

SYNTAX
     spltjoin

DESCRIPTION
     The SPLTJOIN command splits the <focus line> into two or joins the
     <focus line> with the next line depending on the position of the
     cursor. 

     If the cursor is after the last column of a line, the <JOIN>
     command is executed, otherwise the <SPLIT> command is executed.

     The text in the new line is aligned with the text in the <focus line>.

     This command can only be used by assigning it to a function key.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <JOIN>, <SPLIT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Spltjoin(CHARTYPE *params)
#else
short Spltjoin(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Spltjoin");
#endif
 rc = execute_split_join(SPLTJOIN_SPLTJOIN,TRUE,TRUE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     ssave - force SAVE to specified file

SYNTAX
     SSave [filename]

DESCRIPTION
     The SSAVE command writes the current file to disk. If a 'filename' is
     supplied, the current file is saved in that file, otherwise the
     current name of the file is used.

     If a 'filename' is supplied and that 'filename' already exists, 
     the previous contents of that 'filename' will be replaced with the 
     current file.

     Both 'Alterations' counters on the <idline> are reset to zero.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SAVE>, <FILE>, <FFILE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Ssave(CHARTYPE *params)
#else
short Ssave(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Ssave");
#endif
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if ((rc = save_file(CURRENT_FILE,params,TRUE,CURRENT_FILE->number_lines,1L,NULL,FALSE,0,max_line_length,TRUE,FALSE,FALSE)) != RC_OK)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Only set the alteration count to zero if save was successful.       */
/*---------------------------------------------------------------------*/
 CURRENT_FILE->autosave_alt = CURRENT_FILE->save_alt = 0;
/*---------------------------------------------------------------------*/
/* If autosave is on at the time of SSaving, remove the .aus file...   */
/*---------------------------------------------------------------------*/
 if (CURRENT_FILE->autosave > 0)
    rc = remove_aus_file(CURRENT_FILE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     status - display current settings of various variables

SYNTAX
     STATus [filename]

DESCRIPTION
     The STATUS command, without the optional 'filename', displays a full
     screen of current settings for various variables. 

     With the 'filename', the STATUS command creates a file containing a
     series of <SET> commands with the current values of these settings.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible. KEDIT does not support ['filename'] option.

SEE ALSO
     <QUERY>, <EXTRACT>, <MODIFY>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Status(CHARTYPE *params)
#else
short Status(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 int key=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Status");
#endif
 if (strcmp((DEFCHAR *)params,"") == 0)
   {
    if (batch_only)
      {
       display_error(24,(CHARTYPE *)"status",FALSE);
       rc = RC_INVALID_ENVIRON;
      }
    else
      {
       rc = show_status();
       while(1)
         {
#ifdef CAN_RESIZE
          if (is_termresized())
            {
             (void)THE_Resize(0,0);
             (void)show_status();
            }
#endif
          key = my_getch(stdscr);
#if defined(XCURSES)
          if (key == KEY_SF || key == KEY_SR)
             continue;
#endif
#if defined(KEY_MOUSE)
          if (key == KEY_MOUSE)
             continue;
#endif
#ifdef CAN_RESIZE
          if (is_termresized())
             continue;
#endif     
          break;
         }
       THERefresh((CHARTYPE *)"");
       restore_THE();
      }
   }
 else
    rc = save_status(params);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     suspend - suspend THE and return to operating system

SYNTAX
     SUSPend

DESCRIPTION
     The SUSPEND command suspends the current editing session and 
     returns control to the operating system. Under DOS and OS/2 this
     is the equivalent of <OSNOWAIT>. Under UNIX, the process gets placed
     in the background until it is brought to the foreground.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <OSNOWAIT>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Suspend(CHARTYPE *params)
#else
short Suspend(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
#if defined(UNIX) && !defined(XCURSES)
 void (*func)();
#endif
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("comm4.c:   Suspend");
#endif
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(2,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
#if defined(UNIX) && !defined(XCURSES)
 if (strcmp("/bin/sh",getenv("SHELL")) == 0)
   {
    display_error(40,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 suspend_curses();
 func = signal(SIGTSTP,SIG_DFL);
 kill(0,SIGTSTP);
 signal(SIGTSTP,func);
 resume_curses();
 Redraw((CHARTYPE *)"");
#else
 rc = execute_os_command(params,FALSE,FALSE);
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
