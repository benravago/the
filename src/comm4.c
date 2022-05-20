/* COMM4.C - Commands P-S                                              */
/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */
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

short Popup(CHARTYPE *params)
{
   short rc=RC_OK;

   rc = prepare_popup( params );
   return(rc);
}

short Preserve(CHARTYPE *params)
{
   short rc=RC_OK;

   /*
    * Don't allow any parameters
    */
   if ( !blank_field(params) )
   {
      display_error( 1, params, FALSE );
      return( RC_INVALID_OPERAND );
   }
   rc = execute_preserve( CURRENT_VIEW, &CURRENT_VIEW->preserved_view_details, CURRENT_FILE, &CURRENT_FILE->preserved_file_details );

   return(rc);
}
short Prevwindow(CHARTYPE *params)
{
   short rc=RC_OK;
   /*
    * must use local variable, NOT a constant as we MyStrip() this later on
    * and some compilers give seg fault trying to change a constant
    */
   CHARTYPE tmp[2] = { '-', '\0' };

   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,params,FALSE);
      return(RC_INVALID_OPERAND);
   }
   if (display_screens == 1)
   {
      rc = Xedit((CHARTYPE *)tmp);
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

   return(RC_OK);
}
short Print(CHARTYPE *params)
{
#define PRT_PARAMS  2
   CHARTYPE *word[PRT_PARAMS+1];
   CHARTYPE strip[PRT_PARAMS];
   unsigned short num_params=0;
   short page_break=0;
   short rc=RC_OK;
   long target_type=TARGET_NORMAL|TARGET_ALL|TARGET_BLOCK_CURRENT|TARGET_SPARE;
   TARGET target;
   CHARTYPE *line_term = (CHARTYPE *)"\n";

   /*
    * Under Win32, startup the printing system, the first time this
    * command is called. This makes startup of THE faster.
    */
   /*
    * Split parameters up...
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_NONE;
   num_params = param_split(params,word,PRT_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      num_params = 1;
      word[0] = (CHARTYPE *)"1";
   }
   /*
    * If first argument is LINE...
    */
   if (equal((CHARTYPE *)"line",word[0],4))
   {
      print_line(FALSE,0L,0L,0,(CHARTYPE *)word[1],line_term,0);
      return(RC_OK);
   }
   /*
    * If first argument is STRING...
    */
   if (equal((CHARTYPE *)"string",word[0],5))
   {
      print_line(FALSE,0L,0L,0,(CHARTYPE *)word[1],(CHARTYPE *)"",0);
      return(RC_OK);
   }
   /*
    * If first argument is FORMFEED...
    */
   if (equal((CHARTYPE *)"formfeed",word[0],4))
   {
      if (num_params > 1)
      {
         display_error(1,word[1],FALSE);
         return(RC_INVALID_OPERAND);
      }
      print_line(FALSE,0L,0L,0,(CHARTYPE *)"",(CHARTYPE *)"\f",0);
      return(RC_OK);
   }
   /*
    * If first argument is CLOSE...
    */
   if (equal((CHARTYPE *)"close",word[0],5))
   {
      if (num_params > 1)
      {
         display_error(1,word[1],FALSE);
         return(RC_INVALID_OPERAND);
      }
      print_line(TRUE,0L,0L,0,(CHARTYPE *)"",(CHARTYPE *)"",0);
      return(RC_OK);
   }
   /*
    * ...treat all other options as targets...
    */
   initialise_target(&target);
   if ((rc = validate_target(params,&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
      free_target(&target);
      return(rc);
   }
   if (target.spare == (-1))
      page_break = 0;
   else
   {
      if (!valid_positive_integer(strtrunc(target.rt[target.spare].string)))
      {
         display_error(4,word[0],FALSE);
         return(RC_INVALID_OPERAND);
      }
      page_break = atoi((DEFCHAR *)strtrunc(target.rt[target.spare].string));
   }
   print_line(FALSE,target.true_line,target.num_lines,page_break,
              (CHARTYPE *)"",line_term,target.rt[0].target_type);
   free_target(&target);
   return(RC_OK);
}
short Put(CHARTYPE *params)
{
   short rc=RC_OK;

   rc = execute_put( params, FALSE );
   return(rc);
}
short Putd(CHARTYPE *params)
{
   short rc=RC_OK;

   rc = execute_put( params, TRUE );
   return(rc);
}
short Qquit(CHARTYPE *params)
{
   /*
    * No arguments are allowed; error if any are present.
    */
   if ( strcmp( (DEFCHAR *)params, "" ) != 0 )
   {
      display_error( 1, (CHARTYPE *)params, FALSE );
      return(RC_INVALID_OPERAND);
   }
   post_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *)NULL, TRUE );
   free_view_memory( TRUE, TRUE );
   return(RC_OK);
}
short Query(CHARTYPE *params)
{
#define QUE_PARAMS  2
   CHARTYPE *word[QUE_PARAMS+1];
   CHARTYPE strip[QUE_PARAMS];
   unsigned short num_params=0;
   register short i=0;
   short itemno=0;
   bool save_msgmode_status = CURRENT_VIEW->msgmode_status;
   CHARTYPE item_type=0;

   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_NONE;
   num_params = param_split( params, word, QUE_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );
   if ( num_params == 0 )
   {
      display_error( 3, (CHARTYPE *)"", FALSE );
      return(RC_INVALID_OPERAND);
   }
   if ( ( itemno = find_query_item( word[0], strlen( (DEFCHAR *)word[0] ), &item_type ) ) == (-1)
   ||  !( item_type & QUERY_QUERY ) )
   {
      display_error( 1, params, FALSE );
      return(RC_INVALID_OPERAND);
   }
   /*
    * Note that multi-line outputs like PARSER or COLOR will be displayed as part
    * of get_item_values()
    */
   itemno = get_item_values( 1, itemno, (CHARTYPE *)word[1], QUERY_QUERY, 0L, NULL, 0L );
   /*
    * Save the current position and size of the message line so we can
    * restore it. Do it after we have queried the status, otherwise
    * the status of msgline will be stuffed!
    */
   CURRENT_VIEW->msgmode_status = TRUE;
   /*
    * We display output from QUERY here only for single line queries.
    * Multi-line displays will have set itemno = EXTRACT_VARIABLES_SET
    */
   if ( itemno != EXTRACT_ARG_ERROR
   &&   itemno != EXTRACT_VARIABLES_SET )
   {
      strcpy( (DEFCHAR *)temp_cmd, "" );
      for ( i = 0; i < itemno + 1; i++ )
      {
         strcat( (DEFCHAR *)temp_cmd, (DEFCHAR *)item_values[i].value );
         strcat( (DEFCHAR *)temp_cmd, " " );
      }
      display_error( 0, temp_cmd, TRUE );
   }
   CURRENT_VIEW->msgmode_status = save_msgmode_status;
   return(RC_OK);
}
short Quit(CHARTYPE *params)
{
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      return(RC_INVALID_OPERAND);
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   if (CURRENT_FILE->save_alt > 0)
   {
      display_error(22,(CHARTYPE *)"",FALSE);
      return(RC_FILE_CHANGED);
   }
   free_view_memory(TRUE,TRUE);
   return(RC_OK);
}
short Readv(CHARTYPE *params)
{
#define REA_PARAMS  2
   CHARTYPE *word[REA_PARAMS+1];
   CHARTYPE strip[REA_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK,itemno=0,num_values=0;
   unsigned short y=0,x=0;
   CHARTYPE item_type=0;
   bool cursor_on_cmdline=FALSE;

   if ( !in_macro
   ||   !rexx_support )
   {
      display_error( 53, (CHARTYPE *)"", FALSE );
      return( RC_INVALID_ENVIRON );
   }
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_NONE;
   num_params = param_split( params, word, REA_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );
   if ( num_params == 0 )
   {
      display_error( 1, params, FALSE );
      return( RC_INVALID_OPERAND );
   }

   getyx( CURRENT_WINDOW, y, x );
   (void)THERefresh( (CHARTYPE *)"" );
   if ( equal( (CHARTYPE *)"key", word[0], 3) )
   {
      /*
       * Move the cursor to the current location - Bug #3370863.
       */
      wmove( CURRENT_WINDOW, y, x );
      /*
       * Find the item in the list of valid extract options...
       */
      if ( ( itemno = find_query_item( (CHARTYPE *)"READV", 5, &item_type ) ) == (-1) )
      {
         display_error( 1, params, FALSE );
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
            THEcursor_home( current_screen, CURRENT_VIEW, TRUE );
      }
      else
      {
         display_error( 1, word[0], FALSE );
         return( RC_INVALID_OPERAND );
      }
   }
   initial = FALSE;
   return( rc );
}
short THERecord(CHARTYPE *params)
{
#define RECD_PARAMS  2
   CHARTYPE strip[RECD_PARAMS];
   CHARTYPE quoted[RECD_PARAMS];
   CHARTYPE *word[RECD_PARAMS+1];
   unsigned short num_params=0;
   short rc=RC_OK;
   char ctime_buf[26];
   time_t now;

   /*
    * If we are already recording, display error...
    */
   if ( record_fp )
   {
      display_error( 148, (CHARTYPE *)"", FALSE );
      return( RC_INVALID_OPERAND );
   }
   /*
    * Validate the parameters that have been supplied.
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   quoted[0]='\0';
   quoted[1]='"';
   num_params = quoted_param_split( params, word, RECD_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE, quoted );
   if ( num_params < 2 )
   {
      display_error( 3, (CHARTYPE *)"", FALSE );
      return( RC_INVALID_OPERAND );
   }
   if ( num_params > 2 )
   {
      display_error( 2, (CHARTYPE *)"", FALSE );
      return( RC_INVALID_OPERAND );
   }
   /*
    * The first parameter is the key name mnemonic , the next is one or
    * more commands and/or parameters.
    * First check the mnemonic for decimal string value. ie begins with \
    */
   if ( word[0][0] == '\\' )
   {
       if ( ( record_key = atoi( (DEFCHAR *)word[0]+1 ) ) == 0 )
          rc = RC_INVALID_OPERAND;
   }
   else
   {
       if ( ( record_key = find_key_name( word[0] ) ) == (-1) )
          rc = RC_INVALID_OPERAND;
   }
   if ( rc == RC_OK )
   {
      /*
       * Now validate that the specified filename is writeable,
       * and if so, open it for append and write a comment at the
       * start.
       */
      if ( file_writable( word[1] ) )
      {
         record_fp = fopen( (DEFCHAR *)word[1], "w" );
         if ( record_fp == NULL )
         {
            display_error( 8, (CHARTYPE *)"", FALSE );
            rc = RC_INVALID_OPERAND;
         }
         else
         {
            /*
             * Write a comment at the top
             */
            now = time( NULL );
            strcpy( ctime_buf, ctime( &now ) );
            ctime_buf[24] = '\0';
            fprintf( record_fp, "/* Recording of macro started %s */\n", ctime_buf );
            /*
             * Generate the string to display in the status area
             */
            if ( ( record_status = (CHARTYPE *)(*the_malloc)( 27+strlen( (DEFCHAR *)word[0] ) ) ) == NULL )
            {
               display_error( 30, (CHARTYPE *)"", FALSE );
               rc = RC_OUT_OF_MEMORY;
               fclose( record_fp );
               record_fp = NULL;
            }
            sprintf( (DEFCHAR *)record_status, "Recording macro. %s to stop.", (DEFCHAR *)word[0] );
         }
      }
      else
      {
         display_error( 2, (CHARTYPE *)"", FALSE );
         rc = RC_INVALID_OPERAND;
      }
   }
   return( rc );
}
short Recover(CHARTYPE *params)
{
#define REC_PARAMS  2
   CHARTYPE strip[REC_PARAMS];
   CHARTYPE *word[REC_PARAMS+1];
   unsigned short num_params=0;
   short num=0;

   /*
    * Validate the parameters that have been supplied. The one and only
    * parameter should be a positive integer greater than zero or '*'.
    * If no parameter is supplied, 1 is assumed.
    */
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
               return(RC_INVALID_OPERAND);
            }
            num = atoi((DEFCHAR *)word[0]);
         }
         break;
      default:
         display_error(1,word[1],FALSE);
         return(RC_INVALID_OPERAND);
   }
   get_from_recovery_list(num);
   return(RC_OK);
}
short Redit(CHARTYPE *params)
{
   short rc;
   PRESERVED_VIEW_DETAILS *preserved_view_details=NULL;
   PRESERVED_FILE_DETAILS *preserved_file_details=NULL;
   LINETYPE current_line, focus_line;
   CHARTYPE *edit_fname;

   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,params,FALSE);
      return(RC_INVALID_OPERAND);
   }
   /*
    * Ignore the command if a pseudo file...
    */
   if (CURRENT_FILE->pseudo_file)
   {
      return(RC_OK);
   }
   /*
    * Save the filename...
    */
   if ( ( edit_fname = (CHARTYPE *)(*the_malloc)( 1+strlen( (DEFCHAR *)CURRENT_FILE->fpath )+strlen( (DEFCHAR *)CURRENT_FILE->fname ) ) ) == NULL )
   {
      display_error( 30, (CHARTYPE *)"", FALSE );
      rc = RC_OUT_OF_MEMORY;
   }
   else
   {
      strcpy( (DEFCHAR *)edit_fname, (DEFCHAR *)CURRENT_FILE->fpath );
      strcat( (DEFCHAR *)edit_fname, (DEFCHAR *)CURRENT_FILE->fname );
      rc = execute_preserve( CURRENT_VIEW, &preserved_view_details, CURRENT_FILE, &preserved_file_details );
      current_line = CURRENT_VIEW->current_line;
      focus_line = CURRENT_VIEW->focus_line;
      if ( rc == RC_OK )
      {
         free_view_memory(TRUE,TRUE);
         rc = EditFile( edit_fname, FALSE );
         execute_restore( CURRENT_VIEW, &preserved_view_details, CURRENT_FILE, &preserved_file_details, FALSE );
         if ( current_line < CURRENT_FILE->number_lines )
            CURRENT_VIEW->current_line = current_line;
         else
            CURRENT_VIEW->current_line = CURRENT_FILE->number_lines;
         if ( focus_line < CURRENT_FILE->number_lines )
            CURRENT_VIEW->focus_line = focus_line;
         else
            CURRENT_VIEW->focus_line = CURRENT_FILE->number_lines;
         /*
          * Reset DISPLAY as we don't keep the settings
          */
         CURRENT_VIEW->display_low = 0;
         CURRENT_VIEW->display_high = 0;
         CURRENT_VIEW->scope_all = TRUE;
         pre_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *)NULL );
         build_screen( current_screen );
         display_screen( current_screen );
      }
      (*the_free)( edit_fname );
   }
   return(rc);
}
short Redraw(CHARTYPE *params)
{
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,params,FALSE);
      return(RC_INVALID_OPERAND);
   }
   erase();
   refresh();
   restore_THE();
   THERefresh( (CHARTYPE *)"" );
   refresh();
   return(RC_OK);
}
short THERefresh(CHARTYPE *params)
{
   bool save_in_macro=in_macro;
   unsigned short y=0,x=0;
   LINETYPE new_focus_line=0L;

   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,params,FALSE);
      return(RC_INVALID_OPERAND);
   }
   if (!curses_started)
   {
      return(RC_INVALID_ENVIRON);
   }
   interactive_in_macro = TRUE; /* enable contents to be changed inside a macro */
   in_macro = FALSE;
   getyx(CURRENT_WINDOW,y,x);
   if (display_screens > 1)
   {
      prepare_view( (CHARTYPE)(other_screen) );
      display_screen( (CHARTYPE)(other_screen) );
      if (!horizontal)
      {
         touchwin(divider);
         wnoutrefresh(divider);
      }
   }
   show_statarea();
   if ( FILETABSx )
   {
      display_filetabs( NULL );
   }
   if ( max_slk_labels )
   {
      slk_touch();
      slk_noutrefresh();
   }

   CURRENT_VIEW->current_row = calculate_actual_row(CURRENT_VIEW->current_base, CURRENT_VIEW->current_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);
   build_screen( (CHARTYPE)(current_screen) );
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
   touchwin(CURRENT_WINDOW);
   wnoutrefresh(CURRENT_WINDOW);

   touchwin(curscr);
   doupdate();
   interactive_in_macro = FALSE;
   in_macro = save_in_macro;
   return(RC_OK);
}
short Repeat(CHARTYPE *params)
{
   LINETYPE num_lines=0L;
   short rc=RC_OK;
   short direction=0;
   TARGET target;
   long target_type=TARGET_NORMAL;

   if (strcmp("",(DEFCHAR *)params) == 0)
      params = (CHARTYPE *)"+1";
   initialise_target(&target);
   if ((rc = validate_target(params,&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
      free_target(&target);
      return(rc);
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   /*
    * Determine in which direction we are working.
    */
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
   /*
    * Repeat the last command until the number of lines has been reached
    * or the last command returns non-zero.
    */

   while(num_lines-- > 0)
   {
      rc = advance_current_or_focus_line((LINETYPE)direction);
      if (rc != RC_OK)
         break;
      if ( in_macro )
      {
         rc = command_line(last_command_for_repeat_in_macro,COMMAND_ONLY_FALSE);
      }
      else
      {
         rc = command_line(last_command_for_repeat,COMMAND_ONLY_FALSE);
      }
      if (rc != RC_OK)
         break;
   }
   display_screen(current_screen);
   return(rc);
}
short Replace(CHARTYPE *params)
{
   LENGTHTYPE len_params=0;
   short rc=RC_OK;
   LINETYPE true_line=0L;
   LINE *curr=NULL;
   SELECTTYPE current_select=0;
   THELIST *name=NULL;


   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   if (CURRENT_VIEW->hex)
   {
      len_params = convert_hex_strings( params );
      switch( len_params )
      {
         case -1: /* invalid hex value */
            display_error( 32, params, FALSE );
            return(RC_INVALID_OPERAND);
            break;
         case -2: /* memory exhausted */
            display_error( 30, (CHARTYPE *)"", FALSE );
            return(RC_OUT_OF_MEMORY);
            break;
         default:
            break;
      }
   }
   else
     len_params = strlen((DEFCHAR *)params);
   true_line = get_true_line(TRUE);
   if (TOF(true_line)
   ||  BOF(true_line))
   {
      display_error(38,(CHARTYPE *)"",FALSE);
      return(RC_OUT_OF_MEMORY); /* ?? */
   }
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   current_select = curr->select;
   add_to_recovery_list(curr->line,curr->length);
   /*
    * If the line has at least one name, save it to reinstate later...
    */
   if ( curr->first_name )
      name = curr->first_name;
   /*
    * Delete the line, but don't delete the line names...
    */
   curr = delete_LINE( &CURRENT_FILE->first_line, &CURRENT_FILE->last_line, curr, DIRECTION_FORWARD, FALSE );
   curr = curr->prev;
   if ( ( curr = add_LINE( CURRENT_FILE->first_line, curr, params, len_params, current_select, TRUE ) ) == NULL )
   {
      display_error( 30, (CHARTYPE *)"", FALSE );
      return(RC_OUT_OF_MEMORY);
   }
   increment_alt( CURRENT_FILE );

   /*
    * Put the line's name back if we had one...
    */
   if ( name )
      curr->first_name = name;

   pre_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *)NULL );

   build_screen( current_screen );
   display_screen( current_screen );

   return(rc);
}
short Reset(CHARTYPE *params)
{
#define RES_PARAMS  1
   CHARTYPE *word[RES_PARAMS+1];
   CHARTYPE strip[RES_PARAMS];
   unsigned short num_params=0;
   THE_PPC *curr_ppc=NULL;

   strip[0]=STRIP_BOTH;
   num_params = param_split(params,word,RES_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params > 1)
   {
      display_error(1,word[1],FALSE);
      return(RC_INVALID_OPERAND);
   }
   /*
    * Reset the marked block, if any.
    */
   if (equal((CHARTYPE *)"block",word[0],1)
   ||  equal((CHARTYPE *)"all",word[0],3)
   ||  num_params == 0)
   {
      if (MARK_VIEW != (VIEW_DETAILS *)NULL)
      {
         MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
         MARK_VIEW->mark_type = M_NONE;
         MARK_VIEW = (VIEW_DETAILS *)NULL;
      }
   }
   /*
    * Reset the pending prefix commands, if any.
    */
   if (equal((CHARTYPE *)"prefix",word[0],1)
   ||  equal((CHARTYPE *)"all",word[0],3)
   ||  num_params == 0)
   {
      curr_ppc = CURRENT_FILE->first_ppc;
      while(curr_ppc != NULL)
         curr_ppc = delete_pending_prefix_command(curr_ppc,CURRENT_FILE,(LINE *)NULL);
      memset(pre_rec,' ',MAX_PREFIX_WIDTH+1);
      pre_rec_len = 0;
   }
   /*
    * Reset the THIGHTLIGHT area
    */
   if (equal((CHARTYPE *)"thighlight",word[0],5)
   ||  equal((CHARTYPE *)"all",word[0],3)
   ||  num_params == 0)
   {
      CURRENT_VIEW->thighlight_active = FALSE;
   }

   build_screen(current_screen);
   display_screen(current_screen);

   return(RC_OK);
}
short Restore(CHARTYPE *params)
{
   short rc=RC_OK;

   /*
    * Don't allow any parameters
    */
   if ( !blank_field( params ) )
   {
      display_error( 1, params, FALSE );
      return( RC_INVALID_OPERAND );
   }
   rc = execute_restore( CURRENT_VIEW, &CURRENT_VIEW->preserved_view_details, CURRENT_FILE, &CURRENT_FILE->preserved_file_details, TRUE );
   return(rc);
}
short THERexx(CHARTYPE *params)
{
   short rc=RC_OK;

   if (rexx_support)
   {
      short macrorc=0;
      bool save_in_macro=in_macro;
      /*
       * Set in_macro = TRUE to stop multiple show_page()s being performed.
       */
      in_macro = TRUE;
      /*
       * Save the values of the cursor position for EXTRACT command..
       */
      get_cursor_position(&original_screen_line,&original_screen_column,
                     &original_file_line,&original_file_column);
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
      rc = execute_macro_instore(params, &macrorc, NULL, NULL, NULL, 0);
      /*
       * Set in_macro = FALSE to indicate we are out of the macro and do a
       * show_page() now as long as there are still file(s) in the ring.
       */
      in_macro = save_in_macro;
      if (number_of_files > 0)
      {
         if (display_screens > 1)
         {
            build_screen( (CHARTYPE)(other_screen) );
            display_screen( (CHARTYPE)(other_screen) );
         }
         build_screen(current_screen);
         display_screen(current_screen);
      }
   }
   return(rc);
}
short Rgtleft(CHARTYPE *params)
{
   short rc=RC_OK;
   LINETYPE shift_val=0L;
   CHARTYPE _THE_FAR buffer[100];

   /*
    * Validate only parameter, a positive integer. 3/4 if no argument.
    */
   if (blank_field(params))
   {
      shift_val = min(CURRENT_SCREEN.cols[WINDOW_FILEAREA],
                  1 + CURRENT_VIEW->verify_end - CURRENT_VIEW->verify_start)*3/4;
   }
   else
   {
      if ( ( rc = valid_positive_integer_against_maximum( params, MAX_WIDTH_NUM ) ) != 0 )
      {
         if ( rc == 4 )
            sprintf( (DEFCHAR *)buffer, "%s", params );
         else
            sprintf( (DEFCHAR *)buffer, "- MUST be <= %ld", MAX_WIDTH_NUM );
         display_error( rc, buffer, FALSE );
         return(RC_INVALID_OPERAND);
      }
      shift_val = atol((DEFCHAR *)params);
   }
   if ((LINETYPE)CURRENT_VIEW->verify_col - (LINETYPE)CURRENT_VIEW->verify_start > 0)
      shift_val = -shift_val;
   CURRENT_VIEW->verify_col = max(1,CURRENT_VIEW->verify_col+shift_val);
   build_screen(current_screen);
   display_screen(current_screen);
   return(rc);
}
short Right(CHARTYPE *params)
{
   short rc=RC_OK;
   LINETYPE shift_val;
   CHARTYPE _THE_FAR buffer[100];

   /*
    * Validate only parameter, HALF or positive integer. 1 if no argument.
    */
   if ( equal( (CHARTYPE *)"half", params, 4 ) )
      shift_val = CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2;
   else if ( equal( (CHARTYPE *)"full", params, 4 ) )
      shift_val = CURRENT_SCREEN.cols[WINDOW_FILEAREA];
   else if ( blank_field( params ) )
      shift_val = 1L;
   else
   {
      if ( ( rc = valid_positive_integer_against_maximum( params, MAX_WIDTH_NUM ) ) != 0 )
      {
         if ( rc == 4 )
            sprintf( (DEFCHAR *)buffer, "%s", params );
         else
            sprintf( (DEFCHAR *)buffer, "- MUST be <= %ld", MAX_WIDTH_NUM );
         display_error( rc, buffer, FALSE );
         return(RC_INVALID_OPERAND);
      }
      shift_val = atol( (DEFCHAR *)params );
   }
   /*
    * If the argument is 0, set verify column to 1
    */
   if ( shift_val == 0L )
   {
      CURRENT_VIEW->verify_col = 1;
   }
   else
   {
      CURRENT_VIEW->verify_col = max(1,CURRENT_VIEW->verify_col+shift_val);
   }
   build_screen( current_screen );
   display_screen( current_screen );
   return(rc);
}
short Save(CHARTYPE *params)
{
   short rc=RC_OK;

   /*
    * The filename can be quoted; so strip leading and trailing
    * double quotes
    */
   params = MyStrip( params, STRIP_BOTH, '"' );
   post_process_line( CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   if ( ( rc = save_file( CURRENT_FILE, params, FALSE, CURRENT_FILE->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, FALSE ) ) != RC_OK )
   {
      return(rc);
   }
   /*
    * Only set the alteration count to zero if save was successful.
    */
   CURRENT_FILE->autosave_alt = CURRENT_FILE->save_alt = 0;
   /*
    * If autosave is on at the time of Saving, remove the .aus file...
    */
   if ( CURRENT_FILE->autosave > 0 )
      rc = remove_aus_file( CURRENT_FILE );
   return(rc);
}
short Schange(CHARTYPE *params)
{
   short rc=RC_OK;

   interactive_in_macro = TRUE;
   rc = execute_change_command( params, TRUE );
   interactive_in_macro = FALSE;
   return(rc);
}
short THESearch(CHARTYPE *params)
{
   short rc=RC_OK;

   /*
    * If no parameter is specified, use the last_search. If that doesn't
    * exist, error.
    */
   if ( blank_field( params ) )
   {
      if ( blank_field( lastop[LASTOP_SEARCH].value ) )
      {
         display_error( 39, (CHARTYPE *)"", FALSE );
         return( RC_INVALID_OPERAND );
      }
      rc = execute_locate( lastop[LASTOP_SEARCH].value, TRUE, THE_SEARCH_SEMANTICS, NULL );
      return( rc );
   }
   /*
    * Here we have some parameters.
    */
   rc = execute_locate( params, TRUE, THE_SEARCH_SEMANTICS, NULL );
   return( rc );
}
short Set(CHARTYPE *params)
{
   short rc=RC_OK;

   rc = execute_set_sos_command(TRUE,params);
   return(rc);
}
short Shift(CHARTYPE *params)
{
#define SHI_PARAMS  3
   CHARTYPE *word[SHI_PARAMS+1];
   CHARTYPE strip[SHI_PARAMS];
   bool shift_left;
   LINETYPE num_lines=0L,true_line=0L;
   LENGTHTYPE num_cols=0L;
   short num_params=0;
   short rc=RC_OK;
   long target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
   long save_target_type=TARGET_UNFOUND;
   TARGET target;
   bool num_lines_based_on_scope=FALSE;
   CHARTYPE _THE_FAR buffer[100];

   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_NONE;
   num_params = param_split(params,word,SHI_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)                                     /* no params */
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      return(RC_INVALID_OPERAND);
   }
   /*
    * Validate first parameter:
    *    must be Left or Right
    */
   if (equal((CHARTYPE *)"left",word[0],1))
      shift_left = TRUE;
   else if (equal((CHARTYPE *)"right",word[0],1))
      shift_left = FALSE;
   else
   {
      display_error(1,word[0],FALSE);
      return(RC_INVALID_OPERAND);
   }
   /*
    * Validate second parameter (if there is one)
    *    If present, must be valid positive integer.
    *    If not present, default to 1.
    */
   if (num_params < 2)
      num_cols = 1;
   else
   {
      if ( ( rc = valid_positive_integer_against_maximum( word[1], MAX_WIDTH_NUM ) ) != 0 )
      {
         if ( rc == 4 )
            sprintf( (DEFCHAR *)buffer, "%s", word[1] );
         else
            sprintf( (DEFCHAR *)buffer, "- MUST be <= %ld", MAX_WIDTH_NUM );
         display_error( rc, buffer, FALSE );
         return(RC_INVALID_OPERAND);
      }
      num_cols = atol((DEFCHAR *)word[1]);
   }
   /*
    * Validate third  parameter (if there is one)
    *    If present, must be valid target.
    *    If not present, default to 1.
    */
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
         return(rc);
      }
      if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
      {
         if ( ( MARK_VIEW->mark_type == M_STREAM || MARK_VIEW->mark_type == M_CUA )
         &&  target.num_lines > 1 )
         {
            display_error(62,(CHARTYPE*)"",FALSE);
            return(RC_INVALID_OPERAND);
         }
      }
      num_lines = target.num_lines;
      true_line = target.true_line;
      save_target_type = target.rt[0].target_type;
      num_lines_based_on_scope = (save_target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
      free_target(&target);
   }
   /*
    * Now we are here, everything's OK, do the actual shift...
    */
   rc = execute_shift_command( current_screen, CURRENT_VIEW, shift_left, num_cols, true_line, num_lines, num_lines_based_on_scope, save_target_type, FALSE, FALSE );
   return(rc);
}
short ShowKey(CHARTYPE *params)
{
   int key=0;
   short rc=RC_OK;
   bool mouse_key=FALSE;

   /*
    * If no arguments, show key definitions as prompted.
    */
   if (strcmp((DEFCHAR *)params,"") == 0)
   {
      /*
       * Turn off the cursor.
       */
      draw_cursor(FALSE);
      wrefresh(CURRENT_WINDOW);
      display_prompt((CHARTYPE *)"Press the key to be translated...spacebar to exit");
      key = 0;
      while(key != ' ')
      {
         while(1)
         {
            if (is_termresized())
            {
               (void)THE_Resize(0,0);
               (void)THERefresh((CHARTYPE *)"");
            }
            key = my_getch( CURRENT_WINDOW );
            if (is_termresized())
               continue;
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
            break;
         }
         clear_msgline(-1);
         display_prompt( get_key_definition( key, THE_KEY_DEFINE_SHOW, TRUE, mouse_key ) );
      }
      /*
       * Turn on the cursor.
       */
      draw_cursor(TRUE);
      clear_msgline(-1);
   }
   else
   {
      /*
       * If an argument, it must be ALL.
       */
      if (equal((CHARTYPE *)"all",params,3))
         rc = display_all_keys();
      else
      {
         display_error(1,(CHARTYPE *)params,FALSE);
         rc = RC_INVALID_OPERAND;
      }
   }
   return(rc);
}
short Sort(CHARTYPE *params)
{
   short rc=RC_OK;

   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   rc = execute_sort(params);
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   build_screen(current_screen);
   display_screen(current_screen);
   return(rc);
}
short Sos(CHARTYPE *params)
{
#define SOS_PARAMS  10
   register short i=0;
   CHARTYPE strip[SOS_PARAMS];
   CHARTYPE *word[SOS_PARAMS+1];
   short num_params=0;
   short rc=RC_OK;

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
      return(RC_INVALID_OPERAND);
   }
   /*
    * For each "command" go an execute it.
    */
   for (i=0;i<num_params;i++)
   {
      if ((rc = execute_set_sos_command(FALSE,word[i])) != RC_OK)
         break;
   }
   return(rc);
}
short Split(CHARTYPE *params)
{
#define SPT_PARAMS  2
   CHARTYPE *word[SPT_PARAMS+1];
   CHARTYPE strip[SPT_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK;
   bool aligned=FALSE;
   bool cursorarg=FALSE;

   /*
    * Split parameters up...
    */
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
               return(RC_INVALID_ENVIRON);
            }
         }
      }
   }
   rc = execute_split_join(SPLTJOIN_SPLIT,aligned,cursorarg);
   return(rc);
}
short Spltjoin(CHARTYPE *params)
{
   short rc=RC_OK;

   rc = execute_split_join(SPLTJOIN_SPLTJOIN,TRUE,TRUE);
   return(rc);
}
short Ssave(CHARTYPE *params)
{
   short rc=RC_OK;

   /*
    * The filename can be quoted; so strip leading and trailing
    * double quotes
    */
   params = MyStrip( params, STRIP_BOTH, '"' );
   post_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *)NULL, TRUE );
   if ( ( rc = save_file( CURRENT_FILE, params, TRUE, CURRENT_FILE->number_lines, 1L, NULL, FALSE, 0, max_line_length, TRUE, FALSE, FALSE ) ) != RC_OK )
   {
      return(rc);
   }
   /*
    * Only set the alteration count to zero if save was successful.
    */
   CURRENT_FILE->autosave_alt = CURRENT_FILE->save_alt = 0;
   /*
    * If autosave is on at the time of SSaving, remove the .aus file...
    */
   if ( CURRENT_FILE->autosave > 0 )
      rc = remove_aus_file( CURRENT_FILE );
   return(rc);
}
short Status(CHARTYPE *params)
{
   short rc=RC_OK;
   int key=0;

   if ( strcmp( (DEFCHAR *)params,"" ) == 0 )
   {
      if ( batch_only )
      {
         display_error( 24, (CHARTYPE *)"status", FALSE );
         rc = RC_INVALID_ENVIRON;
      }
      else
      {
         rc = show_status();
         while( 1 )
         {
            if ( is_termresized() )
            {
               (void)THE_Resize( 0, 0 );
               (void)show_status();
            }
            key = my_getch( stdscr );
            if ( key == KEY_MOUSE )
               continue;
            if ( is_termresized() )
               continue;
            break;
         }
         Redraw( (CHARTYPE *)"" );
      }
   }
   else
   {
      /*
       * The filename can be quoted; so strip leading and trailing
       * double quotes
       */
      params = MyStrip( params, STRIP_BOTH, '"' );
      rc = save_status( params );
   }
   return(rc);
}
short Suspend(CHARTYPE *params)
{
   short rc=RC_OK;
   void (*func)(int);

   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(2,params,FALSE);
      return(RC_INVALID_OPERAND);
   }
   if (strcmp("/bin/sh",getenv("SHELL")) == 0)
   {
      display_error(40,(CHARTYPE *)"",FALSE);
      return(RC_INVALID_OPERAND);
   }
   suspend_curses();
   func = signal(SIGTSTP,SIG_DFL);
   kill(0,SIGTSTP);
   signal(SIGTSTP,func);
   resume_curses();
   Redraw((CHARTYPE *)"");
   return(rc);
}
