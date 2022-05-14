/***********************************************************************/
/* COMMSET2.C - SET commands O-Z                                       */
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

static char RCSid[] = "$Id: commset2.c,v 1.1 1999/06/25 06:11:56 mark Exp mark $";

#include <the.h>
#include <proto.h>

/*#define DEBUG 1*/

/*--------------------------- global data -----------------------------*/
bool rexx_output=FALSE;

/*man-start*********************************************************************
COMMAND
     set parser - associates a language definition file with a parser

SYNTAX
     [SET] PARSER parser file

DESCRIPTION
     The SET PARSER defines a new syntax highlighting <parser>; 'parser' based
     on a language definition file; 'file'.

     The 'file' is looked for in the directories specified by <SET MACROPATH>.

     To specify one of the builtin parsers, prefix the filename with '*'.
     Therefore to define a <parser> called FRED using the builtin C parser,
     the command would be: SET PARSER FRED *C.TLD.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SET COLORING>, <SET ECOLOUR>, <SET AUTOCOLOR>, <SET MACROPATH>

STATUS  
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Parser(CHARTYPE *params)
#else
short Parser(params)
CHARTYPE *params;
#endif

/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define PAR_PARAMS  2
   CHARTYPE *word[PAR_PARAMS+1];
   CHARTYPE strip[PAR_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK,errnum;
   int num;
   PARSER_DETAILS *curr=NULL,*old_parser=NULL;
   CHARTYPE *tldname=NULL,*buffer;
   bool redisplay_screen=FALSE,free_buffer;
   VIEW_DETAILS *curr_vd=vd_first;
   PARSER_MAPPING *curr_mapping=first_parser_mapping;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("commset2.c:Parser");
#endif
   /*---------------------------------------------------------------------*/
   /* Validate parameters.                                                */
   /*---------------------------------------------------------------------*/
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,PAR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   /*---------------------------------------------------------------------*/
   /* If no arguments, error.                                             */
   /*---------------------------------------------------------------------*/
   if (num_params < 2)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_OPERAND);
   }
   /*
    * Try to find an existing parser with the same name as
    * supplied in first parameter.
    * If we find one and addition of the new parser works, then
    * this parser is purged.
    */
   old_parser = parserll_find(first_parser,word[0]);
   if (word[1][0] == '*')
   {
      /*
       * Use one of the builtin parsers
       */
      buffer = find_default_parser(word[1],tldname);
      if (buffer == NULL)
      {
         display_error(199,word[1],FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_INVALID_OPERAND);
      }
      num = strlen((DEFCHAR *)buffer);
      free_buffer = FALSE;
   }
   else
   {
      /*
       * Read the TLD file from disk.
       */
      if ((tldname = (CHARTYPE *)(*the_malloc)((MAX_FILE_NAME+1)*sizeof(CHARTYPE))) == NULL)
      {
         display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_OUT_OF_MEMORY);
      }
      /*
       * Find a .tld file in MACROPATH
       */
      rc = get_valid_macro_file_name(word[1],tldname,(CHARTYPE *)".tld",&errnum);
      if (rc != RC_OK)
      {
         display_error(errnum,word[1],FALSE);
         (*the_free)(tldname);
#ifdef THE_TRACE
         trace_return();
#endif
         return(rc);
      }
      /*
       * We now have the .tld file in tldname.
       * Now read it into a chunk of memory and construct a parser from
       * it.
       */
      buffer = read_file_into_memory(tldname,&num);
      if (buffer == NULL)
      {
         (*the_free)(tldname);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_INVALID_OPERAND);
      }
      free_buffer = TRUE;
   }
   /*
    * We now have a chunk of memory pointing to a TLD.
    * Construct a parser from it.
    */
   rc = construct_parser(buffer,num,&curr,word[0],tldname);
   if (rc != RC_OK)
   {
      if (free_buffer)
      {
         (*the_free)(buffer);
         (*the_free)(tldname);
      }
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_INVALID_OPERAND);
   }
   /*
    * Now we have successfully constructed a parser, delete the parser
    * with the same name (if we found one)
    * But first, find all files in the ring that have this parser
    * specified, and change to the new parser.
    * Also change the pointer in the parser_mapping list.
    * Check the current file (and the file in the other screen) if they
    * had the old parser; then we need to redisplay the screen.
    */
   if (old_parser)
   {
      if (CURRENT_FILE->parser == old_parser)
         redisplay_screen = TRUE;
      if (display_screens > 1
      &&  SCREEN_FILE(other_screen)->parser == old_parser)
         redisplay_screen = TRUE;
      while (curr_vd != (VIEW_DETAILS *)NULL)
      {
         if (curr_vd->file_for_view->parser == old_parser)
         {
            curr_vd->file_for_view->parser = curr;
         }
         curr_vd = curr_vd->next;
      }
      for(;curr_mapping!=NULL;curr_mapping=curr_mapping->next)
      {
         if (curr_mapping->parser == old_parser)
            curr_mapping->parser = curr;
      }
      destroy_parser(old_parser);
      parserll_del(&first_parser,NULL,old_parser,DIRECTION_FORWARD);
   }
   else
   {
      if (rexx_support)
      {
         /*
          * If we have a Rexx interpreter, register an implied extract
          * function for the number of parsers we now have.
          */
         CHARTYPE tmp[20];
         int i;
         for(i=0,old_parser=first_parser;old_parser!=NULL;old_parser=old_parser->next,i++);
         sprintf((DEFCHAR *)tmp,"parser.%d",i);
         MyRexxRegisterFunctionExe(tmp);
      }
   }
   if (free_buffer)
   {
      (*the_free)(buffer);
      (*the_free)(tldname);
   }
   if (redisplay_screen)
   {
      display_screen(current_screen);
      if (display_screens > 1)
         display_screen(other_screen);
   }
#ifdef THE_TRACE
   trace_return();
#endif
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     set pending - set status of pending prefix commands

SYNTAX
     [SET] PENDing ON string
     [SET] PENDing OFF
     [SET] PENDing BLOCK string

DESCRIPTION
     The SET PENDING command allows the user to insert or remove commands
     from the pending prefix list.

     ON string, simulates the user typing 'string' in the <prefix area>
     of the <focus line>.

     OFF, removes any pending prefix command from the focus line.

     BLOCK string, simulates the user typing 'string' in the PREFIX
     area of the focus line and identifies the prefix command to be 
     a BLOCK command.

COMPATIBILITY
     XEDIT: Does not support ERROR option.
     KEDIT: N/A

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Pending(CHARTYPE *params)
#else
short Pending(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define PEND_ON    1
#define PEND_OFF   2
#define PEND_BLOCK 3
/*--------------------------- local data ------------------------------*/
#define PEN_PARAMS  3
 CHARTYPE *word[PEN_PARAMS+1];
 CHARTYPE strip[PEN_PARAMS];
 unsigned short num_params=0;
 short rc=RC_OK;
 LINE *curr=NULL;
 LINETYPE true_line=0L;
 short command=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Pending");
#endif
/*---------------------------------------------------------------------*/
/* Validate parameters.                                                */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,PEN_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
/*---------------------------------------------------------------------*/
/* If no arguments, error.                                             */
/*---------------------------------------------------------------------*/
 if (num_params == 0)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* If more than 2 arguments, error.                                    */
/*---------------------------------------------------------------------*/
 if (num_params > 2)
   {
    display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Validate first parameter...                                         */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"off",word[0],3))
    command = PEND_OFF;
 else
    if (equal((CHARTYPE *)"on",word[0],2))
       command = PEND_ON;
    else
       if (equal((CHARTYPE *)"block",word[0],5))
          command = PEND_BLOCK;
       else
         {
          display_error(1,word[0],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
 true_line = get_true_line(TRUE);
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 switch(command)
   {
/*---------------------------------------------------------------------*/
/* PENDING ON and PENDING BLOCK...                                     */
/*---------------------------------------------------------------------*/
    case PEND_ON:
    case PEND_BLOCK:
/*---------------------------------------------------------------------*/
/* The second argument must be present and <= PREFIX_WIDTH.            */
/*---------------------------------------------------------------------*/
         if (num_params != 2)
           {
            display_error(3,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_OPERAND;
            break;
           }
         if (strlen((DEFCHAR *)word[1]) > CURRENT_VIEW->prefix_width)
           {
            display_error(1,word[1],FALSE); /* different error ?? */
            rc = RC_INVALID_OPERAND;
            break;
           }
/*---------------------------------------------------------------------*/
/* Copy the string into pre_rec and set its length.                    */
/*---------------------------------------------------------------------*/
         memset(pre_rec,' ',MAX_PREFIX_WIDTH);
         strcpy((DEFCHAR *)pre_rec,(DEFCHAR *)word[1]);
         pre_rec_len = strlen((DEFCHAR *)word[1]);
         pre_rec[pre_rec_len] = ' ';
         pre_rec[CURRENT_VIEW->prefix_width] = '\0';
         curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
         if (command == PEND_BLOCK)
            add_prefix_command(curr,true_line,TRUE);
         else
            add_prefix_command(curr,true_line,FALSE);
         break;
/*---------------------------------------------------------------------*/
/* PENDING OFF...                                                      */
/*---------------------------------------------------------------------*/
    case PEND_OFF:
         if (num_params != 1)
           {
            display_error(2,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_OPERAND;
            break;
           }
         curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
         (void)delete_pending_prefix_command(curr->pre,CURRENT_FILE,curr);
         memset(pre_rec,' ',MAX_PREFIX_WIDTH);
         pre_rec_len = 0;
         break;
   }
 if (rc == RC_OK)
   {
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
     set point - assign a name to the current line

SYNTAX
     [SET] Point .name [OFF]

DESCRIPTION
     The SET POINT command assignes the specified name to the 
     <focus line>, or removes the name from the line with the specified 
     name.
     A valid line name must start with a '.' followed by alphanumeric
     characters. eg. .a .fred and .3AB are valid names.

     When a line is moved within the same file, its line name stays
     with the line.

COMPATIBILITY
     XEDIT: Compatible. See below.
     KEDIT: Compatible. See below.
     Does not allow for multiple names for the same line.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Point(CHARTYPE *params)
#else
short Point(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define POI_PARAMS  2
 CHARTYPE *word[POI_PARAMS+1];
 CHARTYPE strip[POI_PARAMS];
 unsigned short num_params=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Point");
#endif
/*---------------------------------------------------------------------*/
/* Validate parameters.                                                */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,POI_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Turning on line name...                                             */
/*---------------------------------------------------------------------*/
 if (num_params == 1)
   {
    if (word[0][0] != '.'
    ||  !isalnum(word[0][1]))
      {
       display_error(18,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if ((rc = execute_set_point(word[0],get_true_line(TRUE),TRUE)) != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
/*---------------------------------------------------------------------*/
/* Turning off line name...                                            */
/*---------------------------------------------------------------------*/
 else
   {
    if (!equal((CHARTYPE *)"off",word[1],3))
      {
       display_error(1,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if ((rc = execute_set_point(word[0],get_true_line(TRUE),FALSE)) != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set position - determine if LINE/COL is displayed on idline

SYNTAX
     [SET] POSition ON|OFF

DESCRIPTION
     The SET POSITION command allows the user to turn on or off the
     display of LINE/COL on the <idline>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     ON

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Position(CHARTYPE *params)
#else
short Position(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Position");
#endif
 rc = execute_set_on_off(params,&CURRENT_VIEW->position_status);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set prefix - set prefix area attributes

SYNTAX
     [SET] PREfix ON [Left|Right] [n [m]]
     [SET] PREfix Nulls [Left|Right] [n [m]]
     [SET] PREfix OFF
     [SET] PREfix Synonym newname oldname

DESCRIPTION
     The first form of the SET PREFIX command allows the user to display
     the <prefix area> and optionally to select the position were the 
     prefix should be displayed. 

     The second form of the SET PREFIX command is functionally the same
     as the first form. The difference is that when the prefix area
     is displayed with <SET NUMBER> ON, numbers are displyed with leading
     spaces rather than zeros; with <SET NUMBER> OFF, blanks are displayed
     instead of equal signs.

     The third form, turns the display of the prefix area off.
     Executed from within the profile, the only effect is that the
     defaults for all files is changed.
     Executed from the command line, the SET PREFIX command changes the
     current window displays to reflect the required options.

     The fourth form of the SET PREFIX command allows the user to specify
     a synonym for a prefix command or Rexx prefix macro. The 'newname'
     is the command entered in the prefix area and 'oldname' corresponds
     to an existing prefix command or a Rexx macro file in the MACROPATH
     ending in .the or whatever the value of <SET MACROEXT> is at the time the
     prefix command is executed. The 'oldname' can also be the fully
     qualified filename of a Rexx macro.

     The first and second forms of the SET PREFIX command allows the user 
     to specify the width of the prefix area and optionally a gap between 
     the prefix area and the filearea.
     'm' can be specified as an unsigned number between 2 and 20 inclusive.
     'n' can be specified as an unsigned number between 0 and 18, but less
     than the number specified in 'm'.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.
     Specification of prefix width is a THE-only option.

DEFAULT
     ON Left 6 0

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Prefix(CHARTYPE *params)
#else
short Prefix(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define PRE_PARAMS  5
 CHARTYPE *word[PRE_PARAMS+1];
 CHARTYPE strip[PRE_PARAMS];
 CHARTYPE prefix=PREFIX_OFF;
 CHARTYPE previous_prefix=CURRENT_VIEW->prefix;
 unsigned short num_params=0;
 short my_prefix_width=0,my_prefix_gap=0;
 short previous_prefix_width=0;
 short previous_prefix_gap=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Prefix");
#endif
/*---------------------------------------------------------------------*/
/* Set the default values for the prefix width and gap...              */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW)
   {
    previous_prefix_width = CURRENT_VIEW->prefix_width;
    previous_prefix_gap = CURRENT_VIEW->prefix_gap;
   }
 else
   {
    previous_prefix_width = DEFAULT_PREFIX_WIDTH;
    previous_prefix_gap = DEFAULT_PREFIX_GAP;
   }
/*---------------------------------------------------------------------*/
/* Parse the parameters...                                             */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 strip[3]=STRIP_BOTH;
 num_params = param_split(params,word,PRE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (equal((CHARTYPE *)"on",word[0],2)
 ||  equal((CHARTYPE *)"nulls",word[0],1))
   {
    if (num_params > 4)
      {
       display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (equal((CHARTYPE *)"left",word[1],1))
       prefix = PREFIX_LEFT;
    else
       if (equal((CHARTYPE *)"right",word[1],1))
          prefix = PREFIX_RIGHT;
       else
          if (num_params == 1)       /* no left/right, default to left */
             prefix = PREFIX_LEFT;
          else
            {
             display_error(1,word[1],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
    if (equal((CHARTYPE *)"on",word[0],2))
       CURRENT_VIEW->prefix = prefix | PREFIX_ON;
    else
       CURRENT_VIEW->prefix = prefix | PREFIX_NULLS;
    if (num_params > 2)
      {
       if (!valid_positive_integer(word[2]))
         {
          display_error(1,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
       my_prefix_width = atoi((DEFCHAR *)word[2]);
       if (my_prefix_width > 20)
         {
          display_error(6,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
       if (my_prefix_width < 2)
         {
          display_error(5,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
       if (num_params == 3)
           my_prefix_gap = 0;
       else
         {
          if (!valid_positive_integer(word[3]))
            {
             display_error(1,word[3],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
          my_prefix_gap = atoi((DEFCHAR *)word[3]);
          if (my_prefix_gap >= my_prefix_width)
            {
             display_error(6,word[3],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
         }
       CURRENT_VIEW->prefix_width = my_prefix_width;
       CURRENT_VIEW->prefix_gap = my_prefix_gap;
      }
   }
 else
   {
    if (equal((CHARTYPE *)"off",word[0],3))
      {
       if (num_params > 1)
         {
          display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
       if (strcmp((DEFCHAR *)word[1],"") == 0)
         {
          CURRENT_VIEW->prefix = PREFIX_OFF;
          switch(CURRENT_VIEW->current_window)
            {
             case WINDOW_FILEAREA:
                  break;
             case WINDOW_PREFIX:
                  CURRENT_VIEW->current_window = WINDOW_FILEAREA;
                  break;
             case WINDOW_COMMAND:
                  CURRENT_VIEW->previous_window = WINDOW_FILEAREA;
                  break;
            }
         }
       else
         {
          display_error(2,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
      }
    else
       if (equal((CHARTYPE *)"synonym",word[0],1))
         {
          if (num_params < 3)
            {
             display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
          if (num_params > 3)
            {
             display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
          if (strlen((DEFCHAR *)word[1]) > MAX_PREFIX_WIDTH)
            {
             display_error(37,word[1],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
          rc = add_prefix_synonym(word[1],word[2]);
#ifdef THE_TRACE
          trace_return();
#endif
          return(rc);
         }
       else
            {
             display_error(1,word[0],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
   }
/*---------------------------------------------------------------------*/
/* If the new setting for PREFIX is identical with the previous values */
/* don't do anything more.                                             */
/*---------------------------------------------------------------------*/
 if (previous_prefix == CURRENT_VIEW->prefix
 &&  previous_prefix_width == CURRENT_VIEW->prefix_width
 &&  previous_prefix_gap == CURRENT_VIEW->prefix_gap)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* To get here something has changed, so rebuild the windows and       */
/* display the screen.                                                 */
/*---------------------------------------------------------------------*/
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
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set printer - define printer spooler name

SYNTAX
     [SET] PRINTER spooler|[OPTION options]

DESCRIPTION
     The SET PRINTER command sets up the print spooler name to determine
     where output from the <PRINT> command goes.

     The 'options' can be one of the following:
     CPI n                           (characters per inch)
     LPI n                           (lines per inch)
     ORIENTation Portrait|Landscape  
     FONT fontname                   (name of fixed width font)

     No checking is done for printer options. ie. You may specify a
     font that THE doesn't know about, and the printing process may
     not work after that.

     The defaults for page layout for Win32 are:
     CPI 16
     LPI 8
     ORIENTation Portrait
     FONT LinePrinter BM

     'options' are only valid for Win32 platform.
     Printer output for the Win32 platform ALWAYS goes to the default
     printer. Therefore, the 'spooler' option is invalid on this 
     platform.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. THE adds more funtionality.

DEFAULT
     LPT1 - DOS/OS2, lpr - Unix, default - Win32

SEE ALSO
     <PRINT>

STATUS
     Complete. 
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEPrinter(CHARTYPE *params)
#else
short THEPrinter(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define PRI_PARAMS  3
 CHARTYPE *word[PRI_PARAMS+1];
 CHARTYPE strip[PRI_PARAMS];
 unsigned short num_params=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:THEPrinter");
#endif
/*---------------------------------------------------------------------*/
/* Parse the parameters...                                             */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,PRI_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (equal((CHARTYPE *)"option",word[0],6))
   {
#if defined(WIN32)
    if (equal((CHARTYPE *)"fontname",word[1],5))
      {
        setfontname((DEFCHAR*)word[2]);
      }
    else if (equal((CHARTYPE *)"cpi",word[1],3))
      {
       if (!valid_positive_integer(word[2]))
         {
          display_error(1,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
        setfontcpi(atoi(word[2]));
      }
    else if (equal((CHARTYPE *)"lpi",word[1],3))
      {
       if (!valid_positive_integer(word[2]))
         {
          display_error(1,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
        setfontlpi(atoi(word[2]));
      }
    else if (equal((CHARTYPE *)"orientation",word[1],6))
      {
        if (equal((CHARTYPE *)"portrait",word[2],1))
           setorient('P');
        else if (equal((CHARTYPE *)"landscape",word[2],1))
           setorient('L');
        else
          {
           display_error(1,word[2],FALSE);
#ifdef THE_TRACE
           trace_return();
#endif
           return(RC_INVALID_OPERAND);
          }
      }
    else if (equal((CHARTYPE *)"pagesize",word[1],5))
      {
       if (!valid_positive_integer(word[2]))
         {
          display_error(1,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
        setpagesize(atoi(word[2]));
      }
    else
      {
       display_error(1,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
#else
    display_error(78,(CHARTYPE*)"OPTIONS not supported on this platform",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
#endif
   }
 else
   {
    if (num_params == 0)
      {
       display_error(3,(CHARTYPE*)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (num_params > 1)
      {
       display_error(2,(CHARTYPE*)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
#ifdef WIN32

/*
 * This is not supported yet.
 */
# if 0
      if (setprintername( (DEFCHAR *)params ) != RC_OK )
      {
# ifdef THE_TRACE
       trace_return();
# endif
       return(RC_INVALID_OPERAND);
      }
# else
    display_error(78,(CHARTYPE*)"Changing printer name not supported on this platform",FALSE);
# ifdef THE_TRACE
    trace_return();
# endif
    return(RC_INVALID_OPERAND);
# endif
#else
      strcpy((DEFCHAR *)spooler_name,(DEFCHAR *)params);
#endif
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set pscreen - set physical size of screen

SYNTAX
     [SET] PSCReen height [width] [RESET|PRESET]

DESCRIPTION
     The SET PSCREEN command allows the user to adjust the size of the
     physical screen to the size specified by 'height' and 'width'.
     
     This command does not work on all platforms.

     The optional argument [RESET|PRESET] are ignored; they are there
     for Kedit compatibility.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. Ignores RESET|PRESET argument

DEFAULT
     System Dependent

STATUS
     Incomplete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Pscreen(CHARTYPE *params)
#else
short Pscreen(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define PSC_PARAMS  3
 CHARTYPE *word[PSC_PARAMS+1];
 CHARTYPE strip[PSC_PARAMS];
 unsigned short num_params=0;
 int current_cols=COLS,current_lines=LINES;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Pscreen");
#endif
/*---------------------------------------------------------------------*/
/* Validate parameters.                                                */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,PSC_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Validate height parameter...                                        */
/*---------------------------------------------------------------------*/
 if ((current_lines = atoi((DEFCHAR *)word[0])) == 0)
   {
    display_error(4,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Validate width parameter...                                         */
/*---------------------------------------------------------------------*/
 if (num_params > 1)
   {
    if ((current_cols = atoi((DEFCHAR *)word[1])) == 0)
      {
       display_error(4,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
   }
#if defined(OS2) || defined(WIN32)
 (void)THE_Resize(current_lines,current_cols);
 (void)THERefresh((CHARTYPE *)"");
 draw_cursor(TRUE);
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set readonly - allow/disallow changes to a file if it is readonly

SYNTAX
     [SET] READONLY ON|OFF

DESCRIPTION
     The SET READONLY command allows the user to disallow changes to
     files if they are readonly.  Normally, if a file is readonly, THE
     allows the user to make changes to the file contents while in the 
     editing session, but does not allow the file to be saved.

     With READONLY ON, THE disallows any changes to be made to the
     contents of the file in memory, in much the same way that THE
     disallows changes to be made to any files, if THE is started
     with the -r command line switch.

     While the -r command line switch disallows changes to be made to
     any files, SET READONLY ON, only disallows changes to be made to
     readonly files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEReadonly(CHARTYPE *params)
#else
short THEReadonly(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:THEReadonly");
#endif
 rc = execute_set_on_off(params,&READONLYx);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set reprofile - indicate if profile file to be executed for all files

SYNTAX
     [SET] REPROFile ON|OFF

DESCRIPTION
     The SET REPROFILE command allows the user to determine if the 
     <profile> file is to reexecuted for files subsequenlty edited.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     OFF

SEE ALSO
     <XEDIT>, <EDIT>, <THE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Reprofile(CHARTYPE *params)
#else
short Reprofile(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Reprofile");
#endif
 rc = execute_set_on_off(params,&REPROFILEx);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set reserved - display a reserved line

SYNTAX
     [SET] RESERved *|+|-n [colour] [text|OFF]

DESCRIPTION
     The SET RESERVED command reseves a line for the display of arbitrary
     text by the user. The position is determined by +|-n.
     This number, if positive, specifies the line relative from the
     top of the display. A negative number is relative from the
     bottom of the display.

     By specifying a line, say +3, then the third line from the top will
     be reserved, with the supplied text being displayed in that line.

     The <idline> of a file will always be displayed after any reserved
     lines.

     The <status line> is not considered part of the displayable area,
     so any positioning specifications ignore that line.

     A <reserved line> can only be turned off by identifying it in the
     same way that it was defined.  If a <reserved line> was added with
     the position specification of -1, it cannot be turned off with
     a position specification of 23, even though both position 
     specifiers result in the same display line.

     All reserved lines may be turned of by specifying * as the number
     of lines.

     The colour option specifies the colours to use to display the
     reserved line. The format of this colour specifier is the same 
     as for <SET COLOUR>. If no colour is specified, the colour of the
     reserved line will be the colour set by any <SET COLOUR> RESERVED
     command for the view or white on black by default.

     The text of reserved lines can also included embedded control
     characters to control the colour of portions of the text.
     Assume the following <SET CTLCHAR> commands have been issued:

          SET CTLCHAR ESCAPE !
          SET CTLCHAR @ PROTECT BOLD RED ON WHITE
          SET CTLCHAR % PROTECT GREEN ON BLACK

     Then to display a <reserved line> using the specified colours:

          SET RESERVED -1 normal!@bold red on white!%green on black

     It is an error to try to reserve a line which is the same line as
     <SET CURLINE>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SET COLOUR>, <SET CTLCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Reserved(CHARTYPE *params)
#else
short Reserved(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define RSR_PARAMS  2
 CHARTYPE *word[RSR_PARAMS+1];
 CHARTYPE strip[RSR_PARAMS];
 unsigned short num_params=0;
 short base=0,off=0;
 COLOUR_ATTR attr,save_attr;              /* initialisation done below */
 CHARTYPE *string=NULL;
 short rc=RC_OK;
 unsigned short x=0,y=0;
 LINETYPE new_focus_line=0L;
 bool any_colours=FALSE;
 RESERVED *curr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Reserved");
#endif
/*---------------------------------------------------------------------*/
/* Initialise attr and save_sttr...                                    */
/*---------------------------------------------------------------------*/
 memset((char *)&attr,0,sizeof(COLOUR_ATTR));
 memset((char *)&save_attr,0,sizeof(COLOUR_ATTR));
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_NONE;
 num_params = param_split(params,word,RSR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
 {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
 }
/*---------------------------------------------------------------------*/
/* First check for the special case of * OFF...                        */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)word[0],"*") == 0
 &&  equal((CHARTYPE *)"off",word[1],3))
    CURRENT_FILE->first_reserved = rll_free(CURRENT_FILE->first_reserved);
 else
 {
/*---------------------------------------------------------------------*/
/* Parse the position parameter...                                     */
/*---------------------------------------------------------------------*/
    rc = execute_set_row_position(word[0],&base,&off);
    if (rc != RC_OK)
    {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
    }
    if (equal((CHARTYPE *)"off",word[1],3))
       rc = delete_reserved_line(base,off);
    else
    {
/*---------------------------------------------------------------------*/
/* Parse the colour arguments (if any)...                              */
/*---------------------------------------------------------------------*/
       if ((rc = parse_colours(word[1],&attr,&string,TRUE,&any_colours)) != RC_OK)
       {
#ifdef THE_TRACE
          trace_return();
#endif
          return(rc);
       }
       if (!any_colours)
          memcpy(&attr,CURRENT_FILE->attr+ATTR_RESERVED,sizeof(COLOUR_ATTR));
/*---------------------------------------------------------------------*/
/* If the reserved row is the same row as CURLINE, return ERROR.       */
/*---------------------------------------------------------------------*/
       if (calculate_actual_row(CURRENT_VIEW->current_base,
                                CURRENT_VIEW->current_off,
                                CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) ==
           calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],FALSE))
       {
          display_error(64,(CHARTYPE *)"- same line as CURLINE",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_ENVIRON);
       }
/*---------------------------------------------------------------------*/
/* If no colours were specified, use the default colour as set by any  */
/* SET COLOUR RESERVED... command.                                     */
/*---------------------------------------------------------------------*/
       if (memcmp(&attr,&save_attr,sizeof(COLOUR_ATTR)) == 0)
          curr = add_reserved_line(word[0],string,base,off,CURRENT_FILE->attr+ATTR_RESERVED);
       else
          curr = add_reserved_line(word[0],string,base,off,&attr);
       if (curr == NULL)
          rc = RC_OUT_OF_MEMORY;
    }
 }
 build_screen(current_screen);
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
 {
    if (curses_started)
       getyx(CURRENT_WINDOW,y,x);
    new_focus_line = get_focus_line_in_view(current_screen,CURRENT_VIEW->focus_line,y);
    if (new_focus_line != CURRENT_VIEW->focus_line)
    {
       post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
       CURRENT_VIEW->focus_line = new_focus_line;
       y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,CURRENT_VIEW->current_row);
       if (curses_started)
          wmove(CURRENT_WINDOW,y,x);
       pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
    }
 }
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set rexxoutput - indicate where Rexx output is to go

SYNTAX
     [SET] REXXOUTput File|Display n

DESCRIPTION
     The SET REXXOUTPUT command indicates where output from the Rexx
     interpreter is to go; either captured to a file in the ring
     or displayed in a scrolling fashion on the screen.

     Also specified is the maximum number of lines from the Rexx
     interpreter that are to be displayed or captured. This is
     particularly useful when a Rexx <macro> gets into an infinite
     loop.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     Display 1000

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Rexxoutput(CHARTYPE *params)
#else
short Rexxoutput(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define REX_PARAMS  2
 CHARTYPE *word[REX_PARAMS+1];
 CHARTYPE strip[REX_PARAMS];
 unsigned short num_params=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Rexxoutput");
#endif
 if (rexx_output)
   {
    display_error(0,(CHARTYPE *)"Error: Unable to alter REXXOUTPUT settings",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,REX_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 2)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (equal((CHARTYPE *)"file",word[0],1))
    CAPREXXOUTx = TRUE;
 else
    if (equal((CHARTYPE *)"display",word[0],1))
       CAPREXXOUTx = FALSE;
    else
      {
       display_error(1,(CHARTYPE *)word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
 if (!valid_positive_integer(word[1]))
   {
    display_error(4,(CHARTYPE *)word[1],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 CAPREXXMAXx =  atol((DEFCHAR *)word[1]);
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set scale - set position and status of scale line on screen

SYNTAX
     [SET] SCALe ON|OFF [M[+n|-n]|[+|-]n]

DESCRIPTION
     The SET SCALE command sets the position and status of the scale line
     for the current view.

     The first form of parameters is:

     M[+n|-n] 
     this sets the <scale line> to be relative to the middle of
     the screen. A positive value adds to the middle line number, 
     a negative subtracts from it.
     eg. M+3 on a 24 line screen will be line 15
         M-5 on a 24 line screen will be line 7

     The second form of parameters is:

     [+|-]n
     this sets the <scale line> to be relative to the top of the
     screen (if positive or no sign) or relative to the bottom 
     of the screen if negative.
     eg. +3 or 3 will set current line to line 3
         -3 on a 24 line screen will be line 21

     If the resulting line is outside the bounds of the screen
     the position of the current line will become the middle line
     on the screen.

     It is an error to try to position the SCALE line on the same 
     line as <SET CURLINE>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     OFF M+1

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Scale(CHARTYPE *params)
#else
short Scale(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define SCA_PARAMS  2
 CHARTYPE *word[SCA_PARAMS+1];
 CHARTYPE strip[SCA_PARAMS];
 short num_params=0;
 short rc=RC_OK;
 short base=(short)CURRENT_VIEW->scale_base;
 short off=CURRENT_VIEW->scale_off;
 bool scalests=FALSE;
 unsigned short x=0,y=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Scale");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,SCA_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Parse the status parameter...                                       */
/*---------------------------------------------------------------------*/
 rc = execute_set_on_off(word[0],&scalests);
 if (rc != RC_OK)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Parse the position parameter...                                     */
/*---------------------------------------------------------------------*/
 if (num_params > 1)
   {
    rc = execute_set_row_position(word[1],&base,&off);
    if (rc != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
/*---------------------------------------------------------------------*/
/* If the SCALE row is the same row as CURLINE and it is being turned  */
/* on, return ERROR.                                                   */
/*---------------------------------------------------------------------*/
 if (calculate_actual_row(CURRENT_VIEW->current_base,
                          CURRENT_VIEW->current_off,
                          CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) ==
     calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE)
 && scalests)
   {
    display_error(64,(CHARTYPE *)"- same line as CURLINE",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_ENVIRON);
   }
 CURRENT_VIEW->scale_base = (CHARTYPE)base;
 CURRENT_VIEW->scale_off = off;
 CURRENT_VIEW->scale_on = scalests;
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 build_screen(current_screen);
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
    if (curses_started)
       getyx(CURRENT_WINDOW,y,x);
    CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen,CURRENT_VIEW->focus_line,y);
    y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,CURRENT_VIEW->current_row);
    if (curses_started)
       wmove(CURRENT_WINDOW,y,x);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   }
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set scope - sets which lines are to be excluded from commands

SYNTAX
     [SET] SCOPE All|Display

DESCRIPTION
     The SET SCOPE command indicates whether lines not displayed as
     the result of a <SET DISPLAY> or <ALL> command are included in
     the scope of lines to be acted upon by other THE commands.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     Display

SEE ALSO
     <SET DISPLAY>, <SET SELECT>, <ALL>

STATUS
     Completed.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Scope(CHARTYPE *params)
#else
short Scope(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Scope");
#endif
 if (equal((CHARTYPE *)"all",params,1))
    CURRENT_VIEW->scope_all = TRUE;
 else
    if (equal((CHARTYPE *)"display",params,1))
       CURRENT_VIEW->scope_all = FALSE;
    else
      {
       display_error(1,params,FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set screen - specify number of screens displayed

SYNTAX
     [SET] SCReen n [Horizontal|Vertical]
     [SET] SCReen Size l1|* [l2|*]

DESCRIPTION
     The SET SCREEN command specifies the number of views of file(s) to
     display on screen at once. If the number of views specified is 2
     and only one file is currently in the <ring>, two views of the
     same file are displayed.

     The second form of SET SCREEN allows the user to specify the
     number of lines that each screen occupies.  The sum of 'l1' and
     'l2' must equal to lscreen.5 or lscreen.5 - 1 if the <status line>
     is displayed.

     The value of 'l1' specifies the size of the topmost screen; 'l2'
     specifies the size of the bottommost screen.

     Either 'l1' or 'l2' can be set to *, but not both.  The * signifies
     that the screen size for the specified screen will be the remainder
     of the full display window after the size of the other screen
     has been subtracted.

     The THE display can only be split into 1 or 2 screens.

COMPATIBILITY
     XEDIT: Does not support Width or Define options.
     KEDIT: Does not support Split option.
     A maximum of 2 screens are supported.

DEFAULT
     1

SEE ALSO
     <SET STATUSLINE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEScreen(CHARTYPE *params)
#else
short THEScreen(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define SCR_PARAMS  3
#define SCR_MIN_LINES 3
 CHARTYPE *word[SCR_PARAMS+1];
 CHARTYPE strip[SCR_PARAMS];
 register short i=0;
 unsigned short num_params=0,num_views=0;
 CHARTYPE save_display_screens=0;
 bool save_horizontal=FALSE;
 int horiz=(-1);
 VIEW_DETAILS *save_current_view=NULL;
 short save_screen_rows[MAX_SCREENS];
 CHARTYPE save_current_screen=0;
 short rc=RC_OK;
 int size1=0,size2=0,offset=0;
 bool diff_sizes=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:THEScreen");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,SCR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 for (i=0;i<MAX_SCREENS;i++)
 {
    save_screen_rows[i] = screen_rows[i];
 }
 if (equal((CHARTYPE *)"size",word[0],1))
 {
    if (equal((CHARTYPE *)"*",word[1],1))
       size1 = -1;
    else
    {
       if (!valid_positive_integer(word[1]))
       {
          display_error(1,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       size1 = atoi((DEFCHAR *)word[1]);
    }
    num_views = 1;
    if (num_params == 3)
    {
       if (equal((CHARTYPE *)"*",word[2],1))
          size2 = -1;
       else
       {
          if (!valid_positive_integer(word[2]))
          {
             display_error(1,word[2],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
          }
          size2 = atoi((DEFCHAR *)word[2]);
       }
       num_views = 2;
    }
    if (size1 == -1
    &&  size2 == -1)
    {
       display_error(1,word[2],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if (STATUSLINEON())
       offset = 1;
    else
       offset = 0;
    if (size1 == -1)
    {
       size1 = (terminal_lines - offset) - size2;
       if (size1 < SCR_MIN_LINES)
       {
          display_error(6,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
    }
    if (size2 == -1)
    {
       size2 = (terminal_lines - offset) - size1;
       if (size2 < SCR_MIN_LINES)
       {
          display_error(6,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
    }
    if (size1 < SCR_MIN_LINES)
    {
       display_error(5,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if (size2 < SCR_MIN_LINES)
    {
       display_error(5,word[2],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if (size1 + size2 > terminal_lines - offset )
    {
       display_error(6,word[2],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if (size1 + size2 < terminal_lines - offset )
    {
       display_error(5,word[2],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
 }
 else
 {
    if (!valid_positive_integer(word[0]))
    {
       display_error(1,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if ((num_views = atoi((DEFCHAR *)word[0])) > MAX_SCREENS)
    {
       display_error(6,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if (num_views == 1)
       horiz = TRUE;
    else
    {
       if (equal((CHARTYPE *)"horizontal",word[1],1)
       || strcmp((DEFCHAR *)word[1],"") == 0)
          horiz = TRUE;
       if (equal((CHARTYPE *)"vertical",word[1],1))
          horiz = FALSE;
       if (horiz == (-1))
       {
          display_error(1,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
     }
 }
 screen_rows[0] = size1;
 screen_rows[1] = size2;
/*---------------------------------------------------------------------*/
/* Set the global variable display_screens to indicate the number of   */
/* screens currently displayed and the orientation of those screens    */
/* Save the old values first so we know how the screens were oriented. */
/*---------------------------------------------------------------------*/
 save_display_screens = display_screens;
 save_horizontal = horizontal;

 display_screens = (CHARTYPE)num_views;
 horizontal=(bool)horiz;
/*---------------------------------------------------------------------*/
/* If there is no change to the screens, exit.                         */
/*---------------------------------------------------------------------*/
 for (i=0;i<MAX_SCREENS;i++)
 {
    if (screen_rows[i] != save_screen_rows[i])
       diff_sizes = TRUE;
 }
 if (display_screens == save_display_screens
 &&  horizontal == save_horizontal
 &&  diff_sizes == FALSE)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*---------------------------------------------------------------------*/
/* Save the screen coordinates for later retrieval.                    */
/*---------------------------------------------------------------------*/
 if (curses_started)
   {
    getyx(CURRENT_WINDOW_FILEAREA,CURRENT_VIEW->y[WINDOW_FILEAREA],CURRENT_VIEW->x[WINDOW_FILEAREA]);
    if (CURRENT_WINDOW_PREFIX != NULL)
       getyx(CURRENT_WINDOW_PREFIX,CURRENT_VIEW->y[WINDOW_PREFIX],CURRENT_VIEW->x[WINDOW_PREFIX]);
   }
/*---------------------------------------------------------------------*/
/* Set up the screen views correctly so that when we create the new    */
/* window for each screen, the screens have a view to point to...      */
/*---------------------------------------------------------------------*/
 switch(display_screens)
   {
    case 1:                         /* now have only 1 screen (from 2) */
           save_current_view = CURRENT_VIEW;
           if (CURRENT_SCREEN.screen_view->file_for_view == OTHER_SCREEN.screen_view->file_for_view)
             {
              CURRENT_VIEW = OTHER_SCREEN.screen_view;
              CURRENT_FILE->file_views--;
              free_a_view();
              CURRENT_VIEW = save_current_view;
             }
           if (divider != (WINDOW *)NULL)
             {
              delwin(divider);
              divider = NULL;
             }
           current_screen = 0;
           CURRENT_SCREEN.screen_view = CURRENT_VIEW = save_current_view;
           OTHER_SCREEN.screen_view = NULL;
           break;
    case 2:                             /* now have 2 screens (from 1) */
           save_current_view = CURRENT_VIEW;
           save_current_screen = current_screen;
           current_screen = other_screen;     /* make other screen current */
           if (number_of_views == 1)
             {
              if ((rc = defaults_for_other_files(PREVIOUS_VIEW)) != RC_OK)
                {
#ifdef THE_TRACE
                 trace_return();
#endif
                 return(rc);
                }
              CURRENT_FILE = save_current_view->file_for_view;
              CURRENT_FILE->file_views++;
             }
           else
             {
              if (NEXT_VIEW == (VIEW_DETAILS *)NULL)
                 CURRENT_VIEW = vd_first;
              else
                 CURRENT_VIEW = NEXT_VIEW;
             }
           CURRENT_SCREEN.screen_view = CURRENT_VIEW;
           CURRENT_VIEW = save_current_view;
           current_screen = save_current_screen;
           break;
   }

 set_screen_defaults();
 if (curses_started)
   {
    for (i=0;i<display_screens;i++)
      {
       if ((rc = set_up_windows(i)) != RC_OK)
         {
#ifdef THE_TRACE
          trace_return();
#endif
          return(rc);
         }
       if (screen[i].screen_view->prefix)
          wmove(screen[i].win[WINDOW_PREFIX],
                screen[i].screen_view->y[WINDOW_PREFIX],screen[i].screen_view->x[WINDOW_PREFIX]);
       wmove(screen[i].win[WINDOW_FILEAREA],
             screen[i].screen_view->y[WINDOW_FILEAREA],screen[i].screen_view->x[WINDOW_FILEAREA]);
      }
   }

 if (!horizontal
 && display_screens > 1
 && curses_started)
   {
/*    redraw_window(divider);*/
    touchwin(divider);
    wnoutrefresh(divider);
   }

 if (display_screens > 1)
   {
    pre_process_line(OTHER_VIEW,OTHER_VIEW->focus_line,(LINE *)NULL);
    (void)prepare_view(other_screen);
    display_screen(other_screen);
    show_heading(other_screen);
   }
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 (void)prepare_view(current_screen);
 display_screen(current_screen);

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set select - sets the selection level for the specified lines

SYNTAX
     [SET] SELect [+|-]n [target]

DESCRIPTION
     The SET SELECT command sets the selection level for the indicated
     lines equal to 'n' (if no signs are specified) or adds or subtracts
     n from the selection level currently set for the lines in the
     target.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     0

SEE ALSO
     <SET SCOPE>, <SET DISPLAY>, <ALL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Select(CHARTYPE *params)
#else
short Select(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
#define SEL_PARAMS  2
 CHARTYPE *word[SEL_PARAMS+1];
 CHARTYPE strip[SEL_PARAMS];
 LINETYPE true_line=0L;
 short num_params=0;
 bool relative=FALSE;
 short off=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Select");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,SEL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Parse the first parameter...                                        */
/*---------------------------------------------------------------------*/
/* Determine if the selection level is relative to the existing value  */
/* or is an absolute value.                                            */
/*---------------------------------------------------------------------*/
 if (*(word[0]) == '-'
 ||  *(word[0]) == '+')
    relative = TRUE;
 else
    relative = FALSE;
/*---------------------------------------------------------------------*/
/* Get the value, positive or negative.                                */
/*---------------------------------------------------------------------*/
 if (!valid_integer(word[0]))
   {
    display_error(1,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 off = atoi((DEFCHAR *)word[0]);
/*---------------------------------------------------------------------*/
/* Parse the next parameter...                                         */
/*---------------------------------------------------------------------*/
 true_line = get_true_line(TRUE);
/*---------------------------------------------------------------------*/
/* If no target specified, just apply to the current line...           */
/*---------------------------------------------------------------------*/
 if (num_params == 1)
    word[1] = (CHARTYPE *)"+1";
#if 0
 initialise_target(&target);
 if ((rc = validate_target(word[1],&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
    free_target(&target);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
 num_lines = target.num_lines;
 true_line = target.true_line;
 direction = (target.num_lines<0) ? DIRECTION_BACKWARD : DIRECTION_FORWARD;
 free_target(&target);
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*---------------------------------------------------------------------*/
/* Get the current line from which to begin changing the select level. */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* For the number of lines affected, change the select level if the    */
/* line is in scope.                                                   */
/*---------------------------------------------------------------------*/
 for (i=0;i<num_lines;i++)
   {
    if (CURRENT_VIEW->scope_all
    ||  IN_SCOPE(CURRENT_VIEW,curr))
      {
       if (relative)
         {
          if (((short)curr->select + off) > 255)
             curr->select = 255;
          else
             if (((short)curr->select + off) < 0)
                curr->select = 0;
             else
                curr->select += off;
         }
       else
          curr->select = off;
      }
    if (direction == DIRECTION_FORWARD)
       curr = curr->next;
    else
       curr = curr->prev;
   }
#else
 rc = execute_select(word[1],relative,off);
#endif
/*---------------------------------------------------------------------*/
/* If we are on the command line and the result of this statement means*/
/* that the current line is no longer in scope, we need to make the    */
/* current line and possibly the focus line the next line in scope.    */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
   {
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,NULL,true_line,DIRECTION_FORWARD);
    build_screen(current_screen);
    if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
      {
       CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
       pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      }
   }
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
     set shadow - determines if shadow lines are displayed or not

SYNTAX
     [SET] SHADOW ON|OFF

DESCRIPTION
     The SET SHADOW command indicates whether <shadow line>s are to be
     displayed.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     ON

SEE ALSO
     <SET DISPLAY>, <SET SELECT>, <ALL>

STATUS
     Completed.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Shadow(CHARTYPE *params)
#else
short Shadow(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Shadow");
#endif
 rc = execute_set_on_off(params,&CURRENT_VIEW->shadow);
 if (rc == RC_OK)
   {
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
     set slk - set Soft Label Key definitions

SYNTAX
     [SET] SLK n|OFF [text]

DESCRIPTION
     The SET SLK command allows the user to specify a short text
     description to be displayed on the bottom of the screen, using
     the terminal's built-in Soft Label Keys, or the last line of
     the screen.

     The 'n' argument of the command represents the label
     number from left to right, with the first label numbered 1.

     'OFF' turns off display of the Soft Label Keys. This is the same as
     executing [SET] SLK n with no optional text for each label
     displayed.

     The main use for this command is to describe the function assigned
     to a function key, in place of a <reserved line>.

     On those platforms that support a pointing device, clicking the
     left mouse button on the Soft Label Key, is equivalent to pressing
     the associated function key.

     The number of Soft Label Keys displayed is dependent on which
     curses library THE is using.  PDCurses can display 10 keys with
     the length of the 'text' argument 7 characters on a screen that
     is 80 columns wide.  The number of characters that can be 
     displayed increases with the width of the screen.
     Other curses implementations, limit the number of Soft Label Keys
     to 8, with a text width of 8 characters.  Some curses 
     implementations do not support Soft Label Keys.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     OFF

SEE ALSO
     <SET COLOUR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Slk(CHARTYPE *params)
#else
short Slk(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define SLK_PARAMS  2
 CHARTYPE *word[SEL_PARAMS+1];
 CHARTYPE strip[SEL_PARAMS];
 short num_params=0;
 short rc=RC_OK;
 short key=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Slk");
#endif
#if defined(HAVE_SLK_INIT)
 if (SLKx)
   {
    strip[0]=STRIP_BOTH;
    strip[1]=STRIP_NONE;
    num_params = param_split(params,word,SLK_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
    if (num_params < 1)
      {
       display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (num_params == 1
    &&  equal((CHARTYPE *)"off",word[0],3))
      {
       slk_clear();
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OK);
      }
    key = atoi((DEFCHAR*)word[0]);
    if (key == 0)
      {
       display_error(1,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (key > max_slk_labels)
      {
       display_error(2,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    slk_set(key,(DEFCHAR*)word[1],1);
    slk_noutrefresh();
   }
 else
   {
    display_error(82,(CHARTYPE*)"- use -k command line switch",FALSE);
    rc = RC_INVALID_OPERAND;
   }
#else
 display_error(82,(CHARTYPE*)"SLK",FALSE);
 rc = RC_INVALID_OPERAND;
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set span - specify if a string target can span multiple lines (unavailable)

SYNTAX
     [SET] SPAN ON|OFF [Blank|Noblank [n|*]]

DESCRIPTION
     The SET SPAN set command determines if a character string that is
     the subject of a target search can span more than one line of the
     file.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: N/A

DEFAULT
     OFF Blank 2

STATUS
     Not started.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Span(CHARTYPE *params)
#else
short Span(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Span");
#endif
 display_error(0,(CHARTYPE *)"This command not yet implemented",FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set spill - specify if a string target can span multiple lines (unavailable)

SYNTAX
     [SET] SPILL ON|OFF|WORD

DESCRIPTION
     The SET SPILL set command determines how characters are spilt
     off the end of a line when the length of the line exceeds the
     truncation column.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: N/A

DEFAULT
     OFF

STATUS
     Not started.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Spill(CHARTYPE *params)
#else
short Spill(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Spill");
#endif
 display_error(0,(CHARTYPE *)"This command not yet implemented",FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}

/*man-start*********************************************************************
COMMAND
     set statopt - set display options on statusline

SYNTAX
     [SET] STATOPT ON option column [length [prompt] ]
     [SET] STATOPT OFF option|*

DESCRIPTION
     The SET STATOPT command allows the user to specify which internal
     settings of THE are to be displayed on the <status line>.

     The 'option' argument is any value returned by the <EXTRACT>
     command. eg NBFILE.1.

     The syntax of the ON option, displays the specified value, at the
     position in the <status line> specified by 'column'.  If supplied,
     'length' specifies the number of characters, beginning at the
     first character of the returned value, to display.  A value of 0
     indicates that the full value if to be displayed.  The optional
     'prompt' argument, allows the user to specify a string to display
     immediately before the returned value.

     OFF, removes the specified 'option' from displaying.  If '*'
     is specified, all displayed options will be removed.

     'column' is relative to the start of the <status line>.  The
     value of 'column' must be > 9, so that the version of THE is
     not obscured.

     Options will be displayed in the order in which they are set.

     If <SET CLOCK> or <SET HEX> are ON, these will take precedence
     over options specified with this command.

     The more values you display the longer it will take THE to display
     the <status line>.  Also, some values that are available via
     <EXTRACT> are not really suitable for use here. eg CURLINE.3.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     ON NBFILE.1 13 0 Files=
     ON WIDTH.1 23 0 Width=

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Statopt(CHARTYPE *params)
#else
short Statopt(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define OPT_PARAMS  5
 CHARTYPE *word[OPT_PARAMS+1];
 CHARTYPE strip[OPT_PARAMS];
 unsigned short num_params=0;
 short rc=RC_OK;
 LINE *curr=NULL;
 int tail=0,len=0,col=0,itemno=0;
 CHARTYPE item_type=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Statopt");
#endif
/*---------------------------------------------------------------------*/
/* Validate parameters.                                                */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 strip[3]=STRIP_BOTH;
 strip[4]=STRIP_LEADING;
 num_params = param_split(params,word,OPT_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
/*---------------------------------------------------------------------*/
/* If no arguments, error.                                             */
/*---------------------------------------------------------------------*/
 if (num_params == 0)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (num_params < 2)
 {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
 }
/*---------------------------------------------------------------------*/
/* Validate first parameter...                                         */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"off",word[0],3))
 {
    if (num_params > 2)
    {
       display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    if (strcmp((DEFCHAR *)word[1],"*") == 0)
       first_option = last_option = lll_free(first_option);
    else
    {
       curr = lll_locate(first_option,make_upper(word[1]));
       if (curr == NULL)
       {
          display_error(1,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       lll_del(&first_option,&last_option,curr,DIRECTION_FORWARD);
    }
 }
 else
 {
    if (equal((CHARTYPE *)"on",word[0],2))
    {
       if (num_params < 3)
       {
          display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       tail = split_function_name(word[1],&len);
       if (tail == (-1)
       ||  tail > MAX_VARIABLES_RETURNED)
       {
          display_error(1,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       if ((itemno = find_query_item(word[1],len,&item_type)) == (-1)
       || !(item_type & QUERY_EXTRACT))
       {
          display_error(1,word[1],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       if (!valid_positive_integer(word[2]))
       {
          display_error(4,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       len = 0;
       col = atoi((DEFCHAR *)word[2]);
       if (col > COLS)
       {
          display_error(6,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       if (col <= STATAREA_OFFSET)
       {
          display_error(5,word[2],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
       }
       if (num_params > 3)
       {
          if (!valid_positive_integer(word[3]))
          {
             display_error(4,word[3],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
          }
          len = atoi((DEFCHAR *)word[3]);
       }
       curr = lll_locate(first_option,make_upper(word[1]));
       if (curr != NULL)
          lll_del(&first_option,&last_option,curr,DIRECTION_FORWARD);
       curr = lll_add(first_option,last_option,sizeof(LINE));
       if (curr == NULL)
       {
          display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_OUT_OF_MEMORY);
       }
       curr->name = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)word[1])+1)*sizeof(CHARTYPE));
       if (curr->name == NULL)
       {
          display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_OUT_OF_MEMORY);
       }
       strcpy((DEFCHAR *)curr->name,(DEFCHAR *)make_upper(word[1]));
       curr->length = itemno;
       curr->select = (SELECTTYPE)col-STATAREA_OFFSET-1;
       curr->save_select = (SELECTTYPE)len;
       curr->pre = NULL;
       if (num_params > 4)
       {
          curr->line = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)word[4])+1)*sizeof(CHARTYPE));
          if (curr->line == NULL)
          {
             display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_OUT_OF_MEMORY);
          }
          strcpy((DEFCHAR *)curr->line,(DEFCHAR *)word[4]);
       }
       else
          curr->line = NULL;
       curr->flags.new_flag = tail;
       curr->flags.changed_flag = FALSE;
       curr->flags.tag_flag = FALSE;
       curr->flags.save_tag_flag = FALSE;
       if (first_option == NULL)
          first_option = curr;
       last_option = curr;
    }
    else
    {
       display_error(1,word[0],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
 }
 show_statarea();
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set statusline - set position of status line

SYNTAX
     [SET] STATUSLine Top|Bottom|Off|GUI

DESCRIPTION

     The SET STATUSLINE command determines the position of the 
     <status line> for the editing session. TOP will place the status 
     line on the first line of the screen; BOTTOM will place the status 
     line on the last line of the screen; OFF turns off the display of
     the status line.

     The GUI option is only meaningful for those platforms that support
     a separate status line window. If specified for non-GUI ports, the
     GUI option is equivalent to OFF.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.
            Added GUI option for THEdit port.

DEFAULT
     Bottom

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Statusline(CHARTYPE *params)
#else
short Statusline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE stat_place='?';
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Statusline");
#endif
 if (equal((CHARTYPE *)"top",params,1))
    stat_place='T';
 if (equal((CHARTYPE *)"bottom",params,1))
    stat_place='B';
 if (equal((CHARTYPE *)"off",params,2))
    stat_place='O';
 if (equal((CHARTYPE *)"gui",params,3))
   {
    stat_place='G';
#ifdef MSWIN
    SetStatusBar(1);
#endif
   }
#ifdef MSWIN
 else
   {
    SetStatusBar(0);
   }
#endif
 if (stat_place=='?')
   {
    display_error(1,(CHARTYPE *)params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* If the setting supplied is the same as the current setting, just    */
/* return without doing anything.                                      */
/*---------------------------------------------------------------------*/
 if (stat_place == STATUSLINEx)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* Now we need to move the windows around.                             */
/*---------------------------------------------------------------------*/
 STATUSLINEx = stat_place;
/*---------------------------------------------------------------------*/
/* Redefine the screen sizes and recreate statusline window...         */
/*---------------------------------------------------------------------*/
 set_screen_defaults();
 if (curses_started)
   {
    if (create_statusline_window() != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
/*---------------------------------------------------------------------*/
/* Recreate windows for other screen (if there is one)...              */
/*---------------------------------------------------------------------*/
 if (display_screens > 1)
   {
    if (curses_started)
      {
       if (set_up_windows((current_screen==0) ? 1 : 0) != RC_OK)
         {
#ifdef THE_TRACE
          trace_return();
#endif
          return(rc);
         }
       if (!horizontal)
         {
/*          redraw_window(divider); */
          touchwin(divider);
          wnoutrefresh(divider);
         }
      }
    build_screen(other_screen);
    display_screen(other_screen);
   }
/*---------------------------------------------------------------------*/
/* Recreate windows for the current screen...                          */
/*---------------------------------------------------------------------*/
 if (curses_started)
   {
    if (set_up_windows(current_screen) != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
 build_screen(current_screen);
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set stay - set condition of cursor position after CHANGE/LOCATE commands

SYNTAX
     [SET] STAY ON|OFF

DESCRIPTION
     The SET STAY set command determines what line is displayed as the 
     current line after an unsuccessful <LOCATE> or successful <CHANGE>
     command.

     With STAY ON, the <current line> remains where it currently is. 

     With STAY OFF, after an unsuccessful <LOCATE>, the <current line>
     becomes the <Bottom-of-File line> (or <Top-of-File line> if direction 
     is backwards).

     After a successful <CHANGE>, the <current line> is the last
     line affected by the <CHANGE> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     ON

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Stay(CHARTYPE *params)
#else
short Stay(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Stay");
#endif
 rc = execute_set_on_off(params,&CURRENT_VIEW->stay);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set synonym - define synonyms for commands (unavailable)

SYNTAX
     [SET] SYNonym ON|OFF
     [SET] SYNonym [LINEND char] newname [n] definition

DESCRIPTION
     The SET SYNONYM command allows the user to define synonyms for 
     commands or macros.

     The first format indicates if synonym processing is to be performed.

     The second format defines a command synonym.

     The synonym is 'newname', which effectively adds a new THE
     command with the definition specified by 'definition'.  The 'n'
     parameter defines the minimum length of the abbreviation for
     the new command
     An optional LINEND character can be specified prior to 'newname' 
     if the 'definition' contains multiple commands.

     'definition' can be of the form:
     [REXX] command [args] [#command [args] [...]]
     (where # represents the LINEND character specified prior to
     'newname')

     If the optional keyword; 'REXX', is supplied, the remainder of the
     command line is treated as a Rexx macro and is passed onto the
     Rexx interpreter (if you have one) for execution.

     Only 1 level of synonym processing is carried out; therefore
     a synonym cannot be specified in the 'definition'.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     OFF

STATUS
     Not started.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Synonym(CHARTYPE *params)
#else
short Synonym(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Synonym");
#endif
 display_error(0,(CHARTYPE *)"This command not yet implemented",FALSE);
/*---------------------------------------------------------------------*/
/* Do not allow the command COMMAND to be synonymed.                   */
/*---------------------------------------------------------------------*/
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set tabkey - set characteristics of the SOS TABF command

SYNTAX
     [SET] TABKey Tab|Character Tab|Character

DESCRIPTION
     The SET TABKEY sets the action to be taken when the <SOS TABF> 
     command is executed. Depending on the insert mode, the <SOS TABF>
     command will either display a raw tab character or will move to 
     the next tab column.

     The first operand refers to the behaviour of the <SOS TABF> command
     when <SET INSERTMODE> is OFF.

     The second operand specifies the behaviour when the <SOS TABF> 
     command is executed when <SET INSERTMODE> is ON.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     Tab Character

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabkey(CHARTYPE *params)
#else
short Tabkey(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define TKY_PARAMS  3
 CHARTYPE *word[TKY_PARAMS+1];
 CHARTYPE strip[TKY_PARAMS];
 unsigned short num_params=0;
 CHARTYPE tabo=tabkey_overwrite;
 CHARTYPE tabi=tabkey_insert;
 short rc=RC_INVALID_OPERAND;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Tabkey");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,TKY_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 switch(num_params)
   {
/*---------------------------------------------------------------------*/
/* Too few parameters, error.                                          */
/*---------------------------------------------------------------------*/
    case 0:
    case 1:
         display_error(3,(CHARTYPE *)"",FALSE);
         break;
/*---------------------------------------------------------------------*/
/* 2 parameters, validate them...                                      */
/*---------------------------------------------------------------------*/
    case 2:
/*---------------------------------------------------------------------*/
/* Validate first parameter; overwrite mode setting...                 */
/*---------------------------------------------------------------------*/
         if (equal((CHARTYPE *)"character",word[0],1))
           {
            tabo = 'C';
            rc = RC_OK;
           }
         if (equal((CHARTYPE *)"tab",word[0],1))
           {
            tabo = 'T';
            rc = RC_OK;
           }
/*---------------------------------------------------------------------*/
/* If not a valid first parameter, display an error and exit.          */
/*---------------------------------------------------------------------*/
         if (rc != RC_OK)
           {
            display_error(1,word[0],FALSE);
            break;
           }
         rc = RC_INVALID_OPERAND;
/*---------------------------------------------------------------------*/
/* Validate second parameter; insert mode setting...                   */
/*---------------------------------------------------------------------*/
         if (equal((CHARTYPE *)"character",word[1],1))
           {
            tabi = 'C';
            rc = RC_OK;
           }
/*---------------------------------------------------------------------*/
/* Validate second parameter; insert mode setting...                   */
/*---------------------------------------------------------------------*/
         if (equal((CHARTYPE *)"tab",word[1],1))
           {
            tabi = 'T';
            rc = RC_OK;
           }
/*---------------------------------------------------------------------*/
/* If not a valid second parameter, display an error and exit.         */
/*---------------------------------------------------------------------*/
         if (rc != RC_OK)
           {
            display_error(1,word[1],FALSE);
            break;
           }
         rc = RC_OK;
         break;
/*---------------------------------------------------------------------*/
/* Too many parameters...                                              */
/*---------------------------------------------------------------------*/
    default:
         display_error(2,(CHARTYPE *)"",FALSE);
         break;
   }
/*---------------------------------------------------------------------*/
/* If valid parameters, change the settings...                         */
/*---------------------------------------------------------------------*/
 if (rc == RC_OK)
   {
    tabkey_insert = tabi;
    tabkey_overwrite = tabo;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set tabline - set position and status of tab line on screen

SYNTAX
     [SET] TABLine ON|OFF [M[+n|-n]|[+|-]n]

DESCRIPTION
     The SET TABLINE command sets the position and status of the <tab line>
     for the current view.

     The first form of parameters is:

     M[+n|-n] 
     this sets the <tab line> to be relative to the middle of
     the screen. A positive value adds to the middle line number, 
     a negative subtracts from it.
     eg. M+3 on a 24 line screen will be line 15
         M-5 on a 24 line screen will be line 7

     The second form of parameters is:

     [+|-]n
     this sets the <tab line> to be relative to the top of the
     screen (if positive or no sign) or relative to the bottom 
     of the screen if negative.
     eg. +3 or 3 will set current line to line 3
         -3 on a 24 line screen will be line 21

     If the resulting line is outside the bounds of the screen
     the position of the current line will become the middle line
     on the screen.

     It is an error to try to position the TABL line on the same
     line as <SET CURLINE>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     OFF -3

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabline(CHARTYPE *params)
#else
short Tabline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define TBL_PARAMS  2
 CHARTYPE *word[TBL_PARAMS+1];
 CHARTYPE strip[TBL_PARAMS];
 short num_params=0;
 short rc=RC_OK;
 short base=(short)CURRENT_VIEW->tab_base;
 short off=CURRENT_VIEW->tab_off;
 bool tabsts=FALSE;
 unsigned short x=0,y=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Tabline");
#endif
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,TBL_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Parse the status parameter...                                       */
/*---------------------------------------------------------------------*/
 rc = execute_set_on_off(word[0],&tabsts);
 if (rc != RC_OK)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Parse the position parameter...                                     */
/*---------------------------------------------------------------------*/
 if (num_params > 1)
   {
    rc = execute_set_row_position(word[1],&base,&off);
    if (rc != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
/*---------------------------------------------------------------------*/
/* If the TABL  row is the same row as CURLINE and it is being turned  */
/* on, return ERROR.                                                   */
/*---------------------------------------------------------------------*/
 if (calculate_actual_row(CURRENT_VIEW->current_base,
                          CURRENT_VIEW->current_off,
                          CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE) ==
     calculate_actual_row(base,off,CURRENT_SCREEN.rows[WINDOW_FILEAREA],TRUE)
 && tabsts)
   {
    display_error(64,(CHARTYPE *)"- same line as CURLINE",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_ENVIRON);
   }
 CURRENT_VIEW->tab_base = (CHARTYPE)base;
 CURRENT_VIEW->tab_off = off;
 CURRENT_VIEW->tab_on = tabsts;
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 build_screen(current_screen);
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
    if (curses_started)
       getyx(CURRENT_WINDOW,y,x);
    CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen,CURRENT_VIEW->focus_line,y);
    y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,CURRENT_VIEW->current_row);
    if (curses_started)
       wmove(CURRENT_WINDOW,y,x);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   }
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set tabs - set tab columns or tab length

SYNTAX
     [SET] TABS n1 [n2 ... n32]
     [SET] TABS INCR n
     [SET] TABS OFF

DESCRIPTION
     The SET TABS command determines the position of tab columns in THE.

     The first format of SET TABS, specifies individual tab columns. Each
     column must be greater than the column to its left.

     The second format specifies the tab increment to use. ie each tab
     column will be set at each 'n' columns.

     The third format specifies that no tab columns are to be set.

     Tab columns are used by <SOS TABF>, <SOS TABB> and <SOS SETTAB> 
     commands to position the cursor and also by the <COMPRESS> and 
     <EXPAND> commands.

COMPATIBILITY
     XEDIT: Compatible. Does not support OFF option.
     KEDIT: Compatible. Does not support OFF option.

DEFAULT
     INCR 8

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabs(CHARTYPE *params)
#else
short Tabs(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define TABS_PARAMS  MAX_NUMTABS
 CHARTYPE *word[TABS_PARAMS+1];
 CHARTYPE strip[TABS_PARAMS];
 LENGTHTYPE stops[TABS_PARAMS];
 register short i=0;
 unsigned short num_params=0;
 LENGTHTYPE tabinc=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Tabs");
#endif
 for (i=0;i<TABS_PARAMS;i++)
    strip[i] = STRIP_BOTH;
 num_params = param_split(params,word,TABS_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
/*---------------------------------------------------------------------*/
/* If the INCR option is specified...                                  */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"incr",word[0],2))
   {
    if (num_params != 2)
      {
       display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (!valid_positive_integer(word[1]))
      {
       display_error(4,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    tabinc = (LENGTHTYPE)atoi((DEFCHAR *)word[1]);
    if (tabinc < 1)
      {
       display_error(5,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (tabinc > 32)
      {
       display_error(6,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    for (i=0;i<MAX_NUMTABS;i++)
       CURRENT_VIEW->tabs[i] = 1 + (tabinc*i);
    CURRENT_VIEW->numtabs = MAX_NUMTABS;
    CURRENT_VIEW->tabsinc = tabinc;
   }
 else if (equal((CHARTYPE *)"off",word[0],3))
/*---------------------------------------------------------------------*/
/* If the OFF option is specified...                                   */
/*---------------------------------------------------------------------*/
   {
    if (num_params != 1)
      {
       display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    for (i=0;i<MAX_NUMTABS;i++)
       CURRENT_VIEW->tabs[i] = 0;
    CURRENT_VIEW->numtabs = 0;
    CURRENT_VIEW->tabsinc = 0;
   }
 else
/*---------------------------------------------------------------------*/
/* ... individual TAB stop settings.                                   */
/*---------------------------------------------------------------------*/
   {
    if (num_params > MAX_NUMTABS)
      {
       display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if (num_params == 0)
      {
       display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    for (i=0;i<num_params;i++)
      {
       if (!valid_positive_integer(word[i]))
         {
          display_error(4,word[i],FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
       tabinc = (LENGTHTYPE)atoi((DEFCHAR *)word[i]);
       if (i > 0)
         {
          if (stops[i-1] >= tabinc)
            {
             display_error(5,word[i],FALSE);
#ifdef THE_TRACE
             trace_return();
#endif
             return(RC_INVALID_OPERAND);
            }
         }
       stops[i] = tabinc;
      }
    CURRENT_VIEW->numtabs = num_params;
    for (i=0;i<num_params;i++)
      {
       CURRENT_VIEW->tabs[i] = stops[i];
      }
    CURRENT_VIEW->tabsinc = 0;
   }
 build_screen(current_screen);
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set tabsin - set tab processing on file input

SYNTAX
     [SET] TABSIn ON|OFF [n]

DESCRIPTION
     The SET TABSIN command determines if tabs read from a file are to be
     expanded to spaces and if so how many spaces.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Does not support TABQUOTE option.

DEFAULT
     OFF 8

SEE ALSO
     <SET TABSOUT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabsin(CHARTYPE *params)
#else
short Tabsin(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*---------------------------------------------------------------------*/
/* The settings for TABSIN is a global value, despite it supposedly    */
/* being a file level value.                                           */
/*---------------------------------------------------------------------*/
/*--------------------------- local data ------------------------------*/
#define TABI_PARAMS  3
 CHARTYPE *word[TABI_PARAMS+1];
 CHARTYPE strip[TABI_PARAMS];
 unsigned short num_params=0;
 short rc=RC_INVALID_OPERAND;
 CHARTYPE tabsts=TABI_ONx;
 CHARTYPE tabn  =TABI_Nx;
 bool save_scope=FALSE;
 bool save_stay=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Tabsin");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,TABI_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 switch(num_params)
   {
/*---------------------------------------------------------------------*/
/* Too few parameters, error.                                          */
/*---------------------------------------------------------------------*/
    case 0:
         display_error(3,(CHARTYPE *)"",FALSE);
         break;
/*---------------------------------------------------------------------*/
/* 1 or 2 parameters, validate them...                                 */
/*---------------------------------------------------------------------*/
    case 1:
    case 2:
/*---------------------------------------------------------------------*/
/* Validate first parameter; on or off...                              */
/*---------------------------------------------------------------------*/
         if (equal((CHARTYPE *)"on",word[0],2))
           {
            tabsts = TRUE;
            rc = RC_OK;
           }
         if (equal((CHARTYPE *)"off",word[0],3))
           {
            tabsts = FALSE;
            rc = RC_OK;
           }
/*---------------------------------------------------------------------*/
/* If not a valid first parameter, display an error and exit.          */
/*---------------------------------------------------------------------*/
         if (rc != RC_OK)
           {
            display_error(1,word[0],FALSE);
            break;
           }
/*---------------------------------------------------------------------*/
/* For 1 parameter, don't check any more.                              */
/*---------------------------------------------------------------------*/
         if (num_params == 1)
            break;
/*---------------------------------------------------------------------*/
/* Validate second parameter; number of spaces for a TAB...            */
/*---------------------------------------------------------------------*/
         if (!valid_positive_integer(word[1]))
           {
            display_error(4,word[1],FALSE);
            break;
           }
         tabn = (CHARTYPE)atoi((DEFCHAR *)word[1]);
/*---------------------------------------------------------------------*/
/* tabn must be between 1 and 32...                                    */
/*---------------------------------------------------------------------*/
         if (tabn < 1)
           {
            display_error(5,word[1],FALSE);
            break;
           }
         if (tabn > 32)
           {
            display_error(6,word[1],FALSE);
            break;
           }
         rc = RC_OK;
         break;
/*---------------------------------------------------------------------*/
/* Too many parameters...                                              */
/*---------------------------------------------------------------------*/
    default:
         display_error(2,(CHARTYPE *)"",FALSE);
         break;
   }
/*---------------------------------------------------------------------*/
/* If valid parameters, change the settings...                         */
/*---------------------------------------------------------------------*/
 if (rc == RC_OK)
   {
    TABI_ONx = tabsts;
    TABI_Nx = tabn;
/*---------------------------------------------------------------------*/
/* If this command is issued from the profile file, we need to run     */
/* EXPAND ALL on it, as we have already read in the file.              */
/* We need to save the current setting of scope so that it can be      */
/* changed to ALL so that every line will be expanded.                 */
/* Of course if TABSIN OFF was set we DON'T run EXPAND ALL :-)         */
/*---------------------------------------------------------------------*/
    if (in_profile && tabsts)
      {
       save_stay = CURRENT_VIEW->stay;
       CURRENT_VIEW->stay = TRUE;
       save_scope = CURRENT_VIEW->scope_all;
       rc = execute_expand_compress((CHARTYPE *)"ALL",TRUE,FALSE,FALSE,FALSE);
       CURRENT_VIEW->scope_all = save_scope;
       CURRENT_VIEW->stay = save_stay;
      }
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set tabsout - set tab processing on file output

SYNTAX
     [SET] TABSOut ON|OFF [n]

DESCRIPTION
     The SET TABSOUT command determines if spaces written to a file are to
     be compressed to tabs and if so how many spaces.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     OFF 8

SEE ALSO
     <SET TABSIN>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabsout(CHARTYPE *params)
#else
short Tabsout(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define TABO_PARAMS  3
 CHARTYPE *word[TABO_PARAMS+1];
 CHARTYPE strip[TABO_PARAMS];
 unsigned short num_params=0;
 short rc=RC_INVALID_OPERAND;
 bool tabsts=CURRENT_FILE->tabsout_on;
 CHARTYPE tabn=CURRENT_FILE->tabsout_num;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Tabsout");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 strip[2]=STRIP_BOTH;
 num_params = param_split(params,word,TABO_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 switch(num_params)
   {
/*---------------------------------------------------------------------*/
/* Too few parameters, error.                                          */
/*---------------------------------------------------------------------*/
    case 0:
         display_error(3,(CHARTYPE *)"",FALSE);
         break;
/*---------------------------------------------------------------------*/
/* 1 or 2 parameters, validate them...                                 */
/*---------------------------------------------------------------------*/
    case 1:
    case 2:
/*---------------------------------------------------------------------*/
/* Validate first parameter; on or off...                              */
/*---------------------------------------------------------------------*/
         if (equal((CHARTYPE *)"on",word[0],2))
           {
            tabsts = TRUE;
            rc = RC_OK;
           }
         if (equal((CHARTYPE *)"off",word[0],3))
           {
            tabsts = FALSE;
            rc = RC_OK;
           }
/*---------------------------------------------------------------------*/
/* If not a valid first parameter, display an error and exit.          */
/*---------------------------------------------------------------------*/
         if (rc != RC_OK)
           {
            display_error(1,word[0],FALSE);
            break;
           }
/*---------------------------------------------------------------------*/
/* For 1 parameter, don't check any more.                              */
/*---------------------------------------------------------------------*/
         if (num_params == 1)
            break;
/*---------------------------------------------------------------------*/
/* Validate second parameter; number of spaces for a TAB...            */
/*---------------------------------------------------------------------*/
         if (!valid_positive_integer(word[1]))
           {
            display_error(4,word[1],FALSE);
            break;
           }
         tabn = (CHARTYPE)atoi((DEFCHAR *)word[1]);
/*---------------------------------------------------------------------*/
/* tabn must be between 1 and 32...                                    */
/*---------------------------------------------------------------------*/
         if (tabn < 1)
           {
            display_error(5,word[1],FALSE);
            break;
           }
         if (tabn > 32)
           {
            display_error(6,word[1],FALSE);
            break;
           }
         rc = RC_OK;
         break;
/*---------------------------------------------------------------------*/
/* Too many parameters...                                              */
/*---------------------------------------------------------------------*/
    default:
         display_error(2,(CHARTYPE *)"",FALSE);
         break;
   }
/*---------------------------------------------------------------------*/
/* If valid parameters, change the settings...                         */
/*---------------------------------------------------------------------*/
 if (rc == RC_OK)
   {
    CURRENT_FILE->tabsout_on = tabsts;
    CURRENT_FILE->tabsout_num = tabn;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set timecheck - specify if time stamp checking done

SYNTAX
     [SET] TIMECHECK ON|OFF

DESCRIPTION
     The SET TIMECHECK command allows the user to specify if a check
     is made of the modification time of the file being saved at the
     time of saving.  This is done to alert the user if a file has
     changed since they began editing the file.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Timecheck(CHARTYPE *params)
#else
short Timecheck(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Timecheck");
#endif
 rc = execute_set_on_off(params,&CURRENT_FILE->timecheck);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set tofeof - specify if TOF and BOF lines are displayed

SYNTAX
     [SET] TOFEOF ON|OFF

DESCRIPTION
     The SET TOFEOF command allows the user to specify if the
     <Top-of-File line> and the <Bottom-of-File line> are displayed.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     ON

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tofeof(CHARTYPE *params)
#else
short Tofeof(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Tofeof");
#endif
 rc = execute_set_on_off(params,&CURRENT_VIEW->tofeof);
 build_screen(current_screen);
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set trunc - specify the truncation column

SYNTAX
     [SET] TRunc n|*

DESCRIPTION
     The SET TRUNC set command determines the truncation column.  This
     is the rightmost column of text upon which THE commands are 
     effective.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     *

STATUS
     Incomplete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Trunc(CHARTYPE *params)
#else
short Trunc(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Trunc");
#endif
 display_error(0,(CHARTYPE *)"This command not yet implemented",FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set typeahead - set behaviour of screen redraw

SYNTAX
     [SET] TYPEAhead ON|OFF

DESCRIPTION
     The SET TYPEAHEAD set command determines whether or not THE uses the
     curses screen display optimization techniques.

     With TYPEAHEAD ON, curses will abort screen display if a keystroke
     is pending.

     With TYPEAHEAD OFF, curses will not abort screen display if a
     keystroke is pending.

     For BSD based curses, this function has no effect.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THETypeahead(CHARTYPE *params)
#else
short THETypeahead(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 bool setting=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:THETypeahead");
#endif

#ifdef HAVE_TYPEAHEAD
 if (curses_started)
   {
    rc = execute_set_on_off(params,&setting);
    if (rc == RC_OK)
      {
       if (setting)
         {
          typeahead(fileno(stdin));
          TYPEAHEADx = TRUE;
         }
       else
         {
          typeahead(-1);
          TYPEAHEADx = FALSE;
         }
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
     set undoing - turn on or off undo facility for the current file

SYNTAX
     [SET] UNDOING ON|OFF

DESCRIPTION
     The SET UNDOING command allows the user to turn on or off the
     undo facility for the current file.

     At this stage in the development of THE, setting UNDOING to OFF
     stops THE from saving changes made to lines in a file, and
     prevents those lines from being able to be RECOVERed.

     Setting UNDOING to OFF will increase the speed at which THE can
     execute CHANGE and DELETE commands.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Does not support optional arguments.

DEFAULT
     ON

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Undoing(CHARTYPE *params)
#else
short Undoing(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Undoing");
#endif
 rc = execute_set_on_off(params,&CURRENT_FILE->undoing);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set untaa - specifies if "Unsigned Numerical Targets Are Absolute"

SYNTAX
     [SET] UNTAA ON|OFF

DESCRIPTION
     The SET UNTAA command allows the user to turn on or off the
     behaviour of unsigned numerical targets.

     Numerical targets have the form [:|;|+|-]nn. By default, if the
     optional portion of the target is not supplied, then a '+' is
     assumed.  WIth SET UNTAA set to ON, if the optional portion of the
     target is not supplied, then a ':' is assumed.

     Caution:  This SET command affects all numerical targets, not
     just targets in the LOCATE command.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     OFF

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Untaa(CHARTYPE *params)
#else
short Untaa(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Untaa");
#endif
 rc = execute_set_on_off(params,&UNTAAx);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set verify - set column display limits

SYNTAX
     [SET] Verify first [last]

DESCRIPTION
     The SET VERIFY command sets the column limits for the display of the
     current file. 'first' specifies the first column to be displayed
     and 'last' specifies the last column to be displayed.

     If no 'last' option is specified '*' is assumed.

COMPATIBILITY
     XEDIT: Does not implement HEX display nor multiple column pairs.
     KEDIT: Does not implement HEX display nor multiple column pairs.

DEFAULT
     1 *

SEE ALSO
     <SET ZONE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Verify(CHARTYPE *params)
#else
short Verify(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define VER_PARAMS  2
 CHARTYPE *word[VER_PARAMS+1];
 CHARTYPE strip[VER_PARAMS];
 unsigned short num_params=0;
 LINETYPE col1=0L,col2=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Verify");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied. One or two         */
/* parameters can be supplied. The first parameter MUST be a positive  */
/* integer. The second can be a positive integer or '*'. If no second  */
/* parameter is supplied, '*' is assumed. The second parameter MUST be */
/* >= first parameter. '*' is regarded as the biggest number and is    */
/* literally max_line_length.                                          */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,VER_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params == 0)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (num_params > 2)
   {
    display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (!valid_positive_integer(word[0]))
   {
    display_error(4,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 col1 = atol((DEFCHAR *)word[0]);
 if (num_params == 1)
     col2 = max_line_length;
 else
     if (strcmp((DEFCHAR *)word[1],"*") == 0)
        col2 = max_line_length;
     else
        if (!valid_positive_integer(word[1]))
          {
           display_error(4,word[1],FALSE);
#ifdef THE_TRACE
           trace_return();
#endif
           return(RC_INVALID_OPERAND);
          }
        else
           col2 = atol((DEFCHAR *)word[1]);

 if (col2 > max_line_length)
    col2 = max_line_length;
 if (col1 > col2)
   {
    display_error(6,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 CURRENT_VIEW->verify_start = (LENGTHTYPE)col1;
 CURRENT_VIEW->verify_col = (LENGTHTYPE)col1;
 CURRENT_VIEW->verify_end = (LENGTHTYPE)col2;

#ifdef MSWIN
 Win31HScroll(CURRENT_VIEW->verify_col);
#endif

 build_screen(current_screen);
 display_screen(current_screen);

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set width - set width of maximum line that THE can edit

SYNTAX
     [SET] WIDTH n

DESCRIPTION
     The SET WIDTH command specifies the maximum length that a line
     can be within the edit session. This command is effectively the
     same as the -w command line switch.

     The value 'n' MUST be between 10 and 32700.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     512

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Width(CHARTYPE *params)
#else
short Width(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short width=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Width");
#endif
 width = (unsigned short)atoi((DEFCHAR*)params);
 if (width < 10)
   {
    display_error(5,(CHARTYPE *)"- width MUST be >= 10",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (width > 32700)
   {
    display_error(6,(CHARTYPE *)"- width MUST be <= 32700",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (display_length > 0
 &&  display_length > width)
   {
    display_error(6,(CHARTYPE *)"- width MUST be >= display length",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 max_line_length = width;
 if (allocate_working_memory() != 0)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 show_statarea();
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set word - controls what THE considers a word to be

SYNTAX
     [SET] WORD NONBlank|ALPHAnum

DESCRIPTION
     The SET WORD set command determines what sequence of characters THE
     considers a word to be. This is used in command such as <SOS DELWORD>,
     <SOS TABWORDF> and <MARK> WORD to specify the boundaries of the word.

     The default setting for SET WORD is 'NONBlank'. THE treats all
     sequences of characters seperated by a blank (ASCII 32) as words.

     With 'ALPHAnum' THE treats a group of consecutive alphanumeric
     characters as a word.  THE also includes the underscore character
     and characters with an ASCII value > 128 as alphanumeric.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     NONBlank

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Word(CHARTYPE *params)
#else
short Word(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE word=CURRENT_VIEW->word;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Word");
#endif
/*---------------------------------------------------------------------*/
/* Validate the the only parameter is 'NONBlank' or 'ALPHAnum'.        */
/*---------------------------------------------------------------------*/
 if (equal((CHARTYPE *)"nonblank",params,4))
    word = 'N';
 else
   {
    if (equal((CHARTYPE *)"alphanum",params,5))
       word = 'A';
    else
      {
       display_error(1,params,FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
   }
 CURRENT_VIEW->word = word;
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set wordwrap - set wordwrap feature on or off

SYNTAX
     [SET] WORDWrap ON|OFF

DESCRIPTION
     The SET WORDWRAP set command determines whether wordwrap occurs when
     the cursor moves past the right margin (as set by the <SET MARGINS>
     command).

     With WORDWRAP ON, the line, from the beginning of the word that
     exceeds the right margin, is wrapped onto the next line. The cursor
     position stays in the same position relative to the current word.

     With WORDWRAP OFF, no word wrap occurs.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     OFF

SEE ALSO
     <SET MARGINS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Wordwrap(CHARTYPE *params)
#else
short Wordwrap(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Wordwrap");
#endif
 rc = execute_set_on_off(params,&CURRENT_VIEW->wordwrap);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set wrap - enable/disable string locates around the end of the file

SYNTAX
     [SET] WRap ON|OFF

DESCRIPTION
     The SET WRAP set command determines whether THE will look for a
     string target off the ends of the file.

     With WRAP OFF, THE will attempt to locate a string target from the
     current line to the end of file (or top of file if the locate is
     a backwards search).

     With WRAP ON, THE will attempt to locate a string target from the
     current line to the end of file (or top of file if the locate is 
     a backwars search) and wrap around the end of the file and continue
     searching until the current line is reached.

     If the string target is located after wrapping around the end of
     the file, the message 'Wrapped...' is displayed.

     Commands affected by SET WRAP are; <LOCATE>, <FIND>, <NFIND>,
     <FINDUP> and <NFINDUP>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     OFF

SEE ALSO
     <LOCATE>, <FIND>, <NFIND>, <FINDUP>, <NFINDUP>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Wrap(CHARTYPE *params)
#else
short Wrap(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Wrap");
#endif
 rc = execute_set_on_off(params,&CURRENT_VIEW->wrap);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     set xterminal - set X terminal to execute under X

SYNTAX
     [SET] XTERMinal program

DESCRIPTION
     The SET XTERMINAL set command allows the user to specify the full
     quallified file name of the program to run when the <OS>, <DOS> or <!>
     command is entered without parameters when running the X version
     of THE.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

DEFAULT
     System dependent but usually one of:

          /usr/bin/X11/xterm
          /usr/openwin/bin/xterm

     The default value is set by the configure script.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Xterminal(CHARTYPE *params)
#else
short Xterminal(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Xterminal");
#endif
 if (strcmp((DEFCHAR *)params,"") == 0)
   {
    display_error(1,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (strlen((DEFCHAR *)params) > MAX_FILE_NAME)
   {
    display_error(37,params,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 strcpy((DEFCHAR *)xterm_program,(DEFCHAR *)params);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     set zone - set column limits for editing

SYNTAX
     [SET] Zone first [last]

DESCRIPTION
     The SET ZONE command sets the column limits for various other editor
     commands, such as <LOCATE> and <CHANGE>. It effectively restricts
     to the specified columns those parts of the file which can be
     acted upon.

     If no 'last' option is specified '*' is assumed.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1 *

SEE ALSO
     <SET VERIFY>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Zone(CHARTYPE *params)
#else
short Zone(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define ZON_PARAMS  2
 CHARTYPE *word[ZON_PARAMS+1];
 CHARTYPE strip[ZON_PARAMS];
 unsigned short num_params=0;
 LINETYPE col1=0L,col2=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("commset2.c:Zone");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied. One only           */
/* parameter MUST be supplied. The first parameter MUST be a positive  */
/* integer. The second can be a positive integer or '*'. If no second  */
/* parameter is supplied, ERROR.          The second parameter MUST be */
/* >= first parameter. '*' is regarded as the biggest number and is    */
/* literally max_line_length.                                          */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,ZON_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 2)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (num_params > 2)
   {
    display_error(2,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 if (!valid_positive_integer(word[0]))
   {
    display_error(4,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 col1 = atol((DEFCHAR *)word[0]);
 if (strcmp((DEFCHAR *)word[1],"*") == 0)
    col2 = max_line_length;
 else
    if (!valid_positive_integer(word[1]))
      {
       display_error(4,word[1],FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    else
       col2 = atol((DEFCHAR *)word[1]);

 if (col2 > max_line_length)
    col2 = max_line_length;
 if (col1 > col2)
   {
    display_error(6,word[0],FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 CURRENT_VIEW->zone_start = (LENGTHTYPE)col1;
 CURRENT_VIEW->zone_end   = (LENGTHTYPE)col2;
/*---------------------------------------------------------------------*/
/* Change the current column pointer if it is outside the new zone     */
/* settings...                                                         */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_column < CURRENT_VIEW->zone_start)
    CURRENT_VIEW->current_column = max(1,CURRENT_VIEW->zone_start-1);
 if (CURRENT_VIEW->current_column > CURRENT_VIEW->zone_end)
    CURRENT_VIEW->current_column = min(max_line_length,CURRENT_VIEW->zone_end+1);
/*---------------------------------------------------------------------*/
/* If the SCALE line is currently displayed, display the page so that  */
/* any changes are reflected in the SCALE line.                        */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->scale_on)
   {
    build_screen(current_screen);
    display_screen(current_screen);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
