/***********************************************************************/
/* EXECUTE.C -                                                         */
/* This file contains all functions that actually execute one or other */
/* commands.                                                           */
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

static char RCSid[] = "$Id: execute.c,v 1.1 1999/06/25 06:11:56 mark Exp mark $";

#include <the.h>
#include <proto.h>

/***********************************************************************/
#ifdef HAVE_PROTO
short execute_change_command(CHARTYPE *params,bool selective)
#else
short execute_change_command(params,selective)
CHARTYPE *params;
bool selective;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE num_lines=0L,long_n=0L,long_m=0L;
 LINE *curr=NULL;
 CHARTYPE *old_str=NULL,*new_str=NULL;
 short rc=0,selective_rc=RC_OK;
 short direction=DIRECTION_FORWARD;
 long number_changes=0L,number_of_changes=0L,number_of_occ=0L;
 short start_col=0,real_start=0,real_end=0,loc=0;
 LINETYPE true_line=0L,last_true_line=0L,number_lines=0L;
 LINETYPE num_actual_lines=0L,abs_num_lines=0L,i=0L;
 LINETYPE num_file_lines=0L;
 short len_old_str=0,len_new_str=0;
 TARGET target;
 CHARTYPE message[50];
 bool lines_based_on_scope=FALSE;
 CHARTYPE *save_params=NULL;
 short save_target_type=TARGET_RELATIVE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_change_command");
#endif
/*---------------------------------------------------------------------*/
/* Save the parameters for later...                                    */
/*---------------------------------------------------------------------*/
 if ((save_params = (CHARTYPE *)my_strdup(params)) == NULL)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied. Up to 4 parameters */
/* may be supplied. The first is the string to change and its new      */
/* value, the second is the target, the third is the number of times   */
/* to change the value on one line and lastly is which occurrence to   */
/* change first.                                                       */
/*---------------------------------------------------------------------*/
 initialise_target(&target);
 rc = split_change_params(params,&old_str,&new_str,&target,&long_n,&long_m);
 if (rc != RC_OK)
   {
    free_target(&target);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
 num_lines = target.num_lines;
 true_line = target.true_line;
 if (target.rt == NULL)
    lines_based_on_scope = TRUE;
 else
   {
    lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
    save_target_type = target.rt[0].target_type;
   }
 free_target(&target);
/*---------------------------------------------------------------------*/
/* Check for any hex strings in both old_str and new_str.              */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->hex)
   {
    if ((len_old_str = convert_hex_strings(old_str)) == (-1))
      {
       display_error(32,old_str,FALSE);
       (*the_free)(save_params);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    if ((len_new_str = convert_hex_strings(new_str)) == (-1))
      {
       display_error(32,new_str,FALSE);
       (*the_free)(save_params);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
   }
 else
   {
    len_old_str = strlen((DEFCHAR *)old_str);
    len_new_str = strlen((DEFCHAR *)new_str);
   }
/*---------------------------------------------------------------------*/
/* Save the last change command...                                     */
/*---------------------------------------------------------------------*/
 strcpy((DEFCHAR *)last_change_command,(DEFCHAR *)save_params);
 (*the_free)(save_params);
/*---------------------------------------------------------------------*/
/* If the number of lines is zero, don't make any  changes. Exit with  */
/* no rows changed.                                                    */
/*---------------------------------------------------------------------*/
 if (num_lines == 0L)
   {
    display_error(36,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_NO_LINES_CHANGED);
   }
 if (num_lines < 0)
   {
    direction = DIRECTION_BACKWARD;
    abs_num_lines = -num_lines;
   }
 else
   {
    direction = DIRECTION_FORWARD;
    abs_num_lines = num_lines;
   }

 if (true_line != CURRENT_VIEW->focus_line)
   {
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*  pre_process_line(CURRENT_VIEW,true_line,(LINE *)NULL);*/
   }
 number_lines = 0L;
 number_changes = 0L;
 number_of_changes = 0L;
 number_of_occ = 0L;
 start_col = 0;
 last_true_line = true_line;
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
 for (i=0L,num_actual_lines=0L;;i++)
   {
    if (lines_based_on_scope)
      {
       if (num_actual_lines == abs_num_lines)
          break;
      }
    else
      {
       if (abs_num_lines == i)
          break;
      }
    rc = processable_line(CURRENT_VIEW,true_line,curr);
    switch(rc)
      {
       case LINE_SHADOW:
            break;
/*       case LINE_TOF_EOF: MH12 */
       case LINE_TOF:
       case LINE_EOF:
            num_actual_lines++;
            break;
       default:
            pre_process_line(CURRENT_VIEW,true_line,curr);
            loc = 0;
            number_of_changes = number_of_occ = 0L;
            while(loc != (-1))
              {
               if (save_target_type == TARGET_BLOCK_CURRENT)
                 {
                  if (MARK_VIEW->mark_type == M_STREAM)
                    {
                     real_end = rec_len+len_old_str;
                     real_start = start_col;
                     if (true_line == MARK_VIEW->mark_start_line)
                        real_start = max(start_col,MARK_VIEW->mark_start_col-1);
                     if (true_line == MARK_VIEW->mark_end_line)
                        real_end = min(rec_len+len_old_str,MARK_VIEW->mark_end_col-1);
                    }
                  else
                    {
                     real_end = min(rec_len+len_old_str,MARK_VIEW->mark_end_col-1);
                     real_start = max(start_col,MARK_VIEW->mark_start_col-1);
                    }
                 }
               else
                 {
                  real_end = min(rec_len+len_old_str,CURRENT_VIEW->zone_end-1);
                  real_start = max(start_col,CURRENT_VIEW->zone_start-1);
                 }

               if (rec_len < real_start && blank_field(old_str))
                 {
                  loc = 0;
                  rec_len = real_start+1;
                 }
               else
                 {
                  loc = memfind(rec+real_start,old_str,(real_end-real_start+1),
                                len_old_str,
                                (CURRENT_VIEW->case_change == CASE_IGNORE) ? TRUE : FALSE,
                                CURRENT_VIEW->arbchar_status,
                                CURRENT_VIEW->arbchar_single,
                                CURRENT_VIEW->arbchar_multiple);
                 }
               if (loc != (-1))
                 {
                  start_col = loc+real_start;
                  if (number_of_changes <= long_n-1 && number_of_occ >= long_m-1)
                    {
                    /* the following block is done for change or confirm of sch */
                     if (!selective)
                       {
                        memdeln(rec,start_col,rec_len,len_old_str);
                        rec_len = (LENGTHTYPE)max((LINETYPE)start_col,(LINETYPE)rec_len - (LINETYPE)len_old_str);
                        meminsmem(rec,new_str,len_new_str,start_col,max_line_length,rec_len);
                        rec_len += len_new_str;
                        if (rec_len > max_line_length)
                          {
                           rec_len = max_line_length;
                           loc = (-1);
                          }
                        start_col += len_new_str;
                        number_changes++;
                        number_of_changes++;
                       }
                     else
                       {
                       /* selective */
                        selective_rc = selective_change(old_str,len_old_str,new_str,len_new_str,
                                                        true_line,last_true_line,start_col);
                        last_true_line = true_line;
                        switch(selective_rc)
                          {
                           case QUITOK:
                           case RC_OK:
                                start_col += len_new_str;
                                number_changes++;
                                number_of_changes++;
                                if (rec_len > max_line_length)
                                  {
                                   rec_len = max_line_length;
                                   loc = (-1);
                                  }
                                break;
                           case SKIP:
                                start_col += len_old_str;
                                break;
                           case QUIT:
                                break;
                          }
                        if (selective_rc == QUIT || selective_rc == QUITOK)
                           break;
                       }
                     number_of_occ++;
                    }
                  else
                    {
                     start_col += len_old_str;
                     number_of_occ++;
                    }
                  if (number_of_changes > long_n-1)
        /*          ||  number_of_occ > long_n-1)*/
                     loc = (-1);
                 }
              } /* end while */
            if (number_of_changes != 0L)       /* changes made */
              {
               post_process_line(CURRENT_VIEW,true_line,curr,FALSE);
               number_lines++;
              }
            num_actual_lines++;
            break;
      }
    if (selective_rc == QUIT || selective_rc == QUITOK)
       break;
    start_col = 0;
    if (direction == DIRECTION_FORWARD)
       curr = curr->next;
    else
       curr = curr->prev;
    true_line += (LINETYPE)(direction);
    num_file_lines += (LINETYPE)(direction);
    if (curr == NULL)
       break;
   }
/*---------------------------------------------------------------------*/
/* If no changes were made, display error message and return.          */
/*---------------------------------------------------------------------*/
 if (number_changes == 0L)
   {
    display_error(36,(CHARTYPE *)"",FALSE);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_NO_LINES_CHANGED);
   }
/*---------------------------------------------------------------------*/
/* Increment the alteration count here, once irrespective of the number*/
/* of lines changed.                                                   */
/*---------------------------------------------------------------------*/
 increment_alt(CURRENT_FILE);
/*---------------------------------------------------------------------*/
/* If STAY is OFF, change the current and focus lines by the number    */
/* of lines calculated from the target.                                */
/*---------------------------------------------------------------------*/
 if (selective)
   {
    if (!CURRENT_VIEW->stay)                            /* stay is off */
      {
       CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line;
      }
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
    build_screen(current_screen);
    display_screen(current_screen);
   }
 else
   {
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
    resolve_current_and_focus_lines(CURRENT_VIEW,last_true_line,num_file_lines,direction,TRUE,FALSE);
   }

 sprintf((DEFCHAR *)message,"%ld occurrence(s) changed on %ld line(s)",number_changes,number_lines);
 display_error(0,message,TRUE);
#ifdef THE_TRACE
 trace_return();
#endif
 if (CURRENT_TOF || CURRENT_BOF)
    return(RC_TOF_EOF_REACHED);
 else
    return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short selective_change(CHARTYPE *old_str,short len_old_str,CHARTYPE *new_str,
                       short len_new_str,LINETYPE true_line,LINETYPE last_true_line,short start_col)
#else
short selective_change(old_str,len_old_str,new_str,len_new_str,true_line,last_true_line,start_col)
CHARTYPE *old_str;
short len_old_str;
CHARTYPE *new_str;
short len_new_str;
LINETYPE true_line;
LINETYPE last_true_line;
short start_col;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short y=0,x=0,rc=RC_OK;
 int key=0;
 bool changed=FALSE;
 bool line_displayed=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: selective_change");
#endif

 getyx(CURRENT_WINDOW_FILEAREA,y,x);
                /* move cursor to old string a la cmatch */
                /* display message */
                /* accept key - C next, - N change, - Q to quit */

 CURRENT_VIEW->focus_line = true_line;
/*---------------------------------------------------------------------*/
/* Check if the true_line is in the currently displayed window.        */
/* If not, then change the current_line to the true_line.              */
/*---------------------------------------------------------------------*/
 line_displayed = FALSE;
 for (i=0;i<CURRENT_SCREEN.rows[WINDOW_FILEAREA];i++)
   {
    if (CURRENT_SCREEN.sl[i].line_number == true_line
    &&  CURRENT_SCREEN.sl[i].line_type == LINE_LINE)
      {
       line_displayed = TRUE;
       y = i;
       break;
      }
   }
 if (!line_displayed)
   {
    CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
    y = CURRENT_VIEW->current_row;
   }

 if (start_col >= CURRENT_VIEW->verify_col-1
 &&  start_col <= (CURRENT_SCREEN.cols[WINDOW_FILEAREA]+(CURRENT_VIEW->verify_col-1))-1)
    x = start_col-(CURRENT_VIEW->verify_col-1);
 else
   {
    x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
    CURRENT_VIEW->verify_col = max(1,start_col-(short)x);
    x = (start_col-(CURRENT_VIEW->verify_col-1));
   }

 key = 0;
 changed = FALSE;
 while(key == 0)
   {
    build_screen(current_screen);
    display_screen(current_screen);
    if (changed)
       display_prompt((CHARTYPE *)"Press 'N' for next,'C' to undo 'Q' to quit");
    else
       display_prompt((CHARTYPE *)"Press 'N' for next,'C' to change 'Q' to quit");
    wmove(CURRENT_WINDOW_FILEAREA,y,x);
    wrefresh(CURRENT_WINDOW_FILEAREA);

    key = my_getch(stdscr);
    clear_msgline(-1);
    switch(key)
      {
       case 'N':
       case 'n':
            if (changed)
               rc = RC_OK;
            else
               rc = SKIP;
            break;
       case 'C':
       case 'c':
            if (changed)
              {
               memdeln(rec,start_col,rec_len,len_new_str);
               rec_len -= len_new_str;
               meminsmem(rec,old_str,len_old_str,start_col,max_line_length,rec_len);
               rec_len += len_old_str;
              }
            else
              {
               memdeln(rec,start_col,rec_len,len_old_str);
               rec_len -= len_old_str;
               meminsmem(rec,new_str,len_new_str,start_col,max_line_length,rec_len);
               rec_len += len_new_str;
              }
            changed = (changed) ? FALSE : TRUE;
            key = 0;
            break;
       case 'Q':
       case 'q':
            if (changed)
               rc = QUITOK;
            else
               rc = QUIT;
            break;
       default:
            key = 0;
            break;
      }
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short insert_new_line(CHARTYPE *line,unsigned short len,LINETYPE num_lines,
                      LINETYPE true_line,bool start_left_col,bool make_current,
                      bool inc_alt,CHARTYPE select,bool move_cursor,
                      bool sos_command)
#else
short insert_new_line(line,len,num_lines,true_line,start_left_col,make_current,inc_alt,select,move_cursor,sos_command)
CHARTYPE *line;
unsigned short len;
LINETYPE num_lines;
LINETYPE true_line;
bool start_left_col;
bool make_current;
bool inc_alt;
CHARTYPE select;
bool move_cursor;
bool sos_command;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 LINE *curr=NULL,*save_curr=NULL;
 unsigned short x=0,y=0;
 short new_col=0;
 bool on_bottom_of_file=FALSE,on_bottom_of_screen=FALSE;
 short number_focus_rows=0;
 bool leave_cursor=FALSE;
 LINETYPE new_focus_line=0L,new_current_line=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: insert_new_line");
#endif
 if (!CURRENT_VIEW->scope_all)
    true_line = find_last_not_in_scope(CURRENT_VIEW,NULL,true_line,DIRECTION_FORWARD);
/*---------------------------------------------------------------------*/
/* If we are on the 'Bottom of File' line reduce the true_line by 1    */
/* so that the new line is added before the bottom line.               */
/*---------------------------------------------------------------------*/
 if (true_line == CURRENT_FILE->number_lines+1L)
    true_line--;
/*---------------------------------------------------------------------*/
/* Find the current LINE pointer for the true_line.                    */
/* This is the line after which the line(s) are to be added.           */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* Insert into the linked list the number of lines specified. All lines*/
/* will contain a blank line and a length of zero.                     */
/*---------------------------------------------------------------------*/
 save_curr = curr;
 for (i=0;i<num_lines;i++)
    {
     if ((curr = add_LINE(CURRENT_FILE->first_line,curr,line,len,select,TRUE)) == NULL)
       {
        display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
        trace_return();
#endif
        return(RC_OUT_OF_MEMORY);
       }
    }
/*---------------------------------------------------------------------*/
/* Fix the positioning of the marked block (if there is one and it is  */
/* in the current view) and any pending prefix commands.               */
/*---------------------------------------------------------------------*/
 adjust_marked_lines(TRUE,true_line,num_lines);
 adjust_pending_prefix(CURRENT_VIEW,TRUE,true_line,num_lines);
/*---------------------------------------------------------------------*/
/* Increment the number of lines counter for the current file and the  */
/* number of alterations, only if requested to do so.                  */
/*---------------------------------------------------------------------*/
 if (inc_alt)
    increment_alt(CURRENT_FILE);

 CURRENT_FILE->number_lines += num_lines;
/*---------------------------------------------------------------------*/
/* Sort out focus and current line.                                    */
/*---------------------------------------------------------------------*/
 if (move_cursor)
   {
    switch(CURRENT_VIEW->current_window)
      {
       case WINDOW_COMMAND:
            CURRENT_VIEW->focus_line = true_line + 1L;
            if (make_current)
               CURRENT_VIEW->current_line = true_line + 1L;
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
            break;
       case WINDOW_FILEAREA:
       case WINDOW_PREFIX:
            build_screen(current_screen);
            getyx(CURRENT_WINDOW,y,x);
            calculate_scroll_values(&number_focus_rows,&new_focus_line,
                                    &new_current_line,
                                    &on_bottom_of_screen,&on_bottom_of_file,
                                    &leave_cursor,DIRECTION_FORWARD);
            new_col = x;
            if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
              {
               if (!start_left_col)
                 {
                  if (CURRENT_VIEW->newline_aligned)
                    {
                     new_col = memne(save_curr->line,' ',save_curr->length);
                     if (new_col == (-1))
                        new_col = 0;
/*---------------------------------------------------------------------*/
/* Special case when right margin is > than screen width...            */
/*---------------------------------------------------------------------*/
                     if (CURRENT_VIEW->verify_start != CURRENT_VIEW->verify_col)
                       {
/*---------------------------------------------------------------------*/
/* If the new column position will be on the same page...              */
/*---------------------------------------------------------------------*/
                        if (CURRENT_VIEW->verify_col < new_col
                        &&  CURRENT_VIEW->verify_col + CURRENT_SCREEN.screen_cols > new_col)
                           new_col = (new_col - CURRENT_VIEW->verify_col) + 1;
                        else
                          {
                           x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
                           CURRENT_VIEW->verify_col = max(1,new_col - (short)x + 2);
                           new_col = (CURRENT_VIEW->verify_col == 1) ? new_col : x - 1;
                          }
                       }
                    }
                  else
                    {
                     new_col = 0;
                     CURRENT_VIEW->verify_col = 1;
                    }
                 }
              }
/*---------------------------------------------------------------------*/
/* Move the cursor to where it should be and display the page.         */
/*---------------------------------------------------------------------*/
            if (on_bottom_of_screen)
              {
               CURRENT_VIEW->current_line = new_current_line;
               CURRENT_VIEW->focus_line = new_focus_line;
               wmove(CURRENT_WINDOW,y-((leave_cursor) ? 0 : 1),new_col);
              }
            else
              {
/*---------------------------------------------------------------------*/
/* We are in the middle of the window, so just move the cursor down    */
/* 1 line.                                                             */
/*---------------------------------------------------------------------*/
              wmove(CURRENT_WINDOW,y+number_focus_rows,new_col);
              CURRENT_VIEW->focus_line = new_focus_line;
              if (compatible_feel == COMPAT_XEDIT
              && !sos_command)
                 CURRENT_VIEW->current_line = new_current_line;
             }
            break;
      }
   }
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 build_screen(current_screen);
 display_screen(current_screen);

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_os_command(CHARTYPE *cmd,bool quiet,bool pause)
#else
short execute_os_command(cmd,quiet,pause)
CHARTYPE *cmd;
bool quiet;
bool pause;
#endif
/***********************************************************************/
{
#ifdef XCURSES
#endif
/*--------------------------- local data ------------------------------*/
#if defined(DOS) || defined(OS2) || defined(WIN32)
#define SHELL "COMSPEC"
#else
#define SHELL "SHELL"
#endif
 short rc=0;
#ifdef XCURSES
 bool save_curses_started=curses_started;
#endif
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_os_command");
#endif

#ifdef MSWIN
 quiet = 1;
 pause = 0;
#endif

#ifdef XCURSES
 curses_started=FALSE;
#endif

 if (!quiet && curses_started)
   {
    attrset(A_NORMAL);
#if 0
    touchwin(stdscr);
    wmove(stdscr,0,0);
    addch(' ');
#else
    clear();
#endif
    wmove(stdscr,1,0);
    wrefresh(stdscr);   /* clear screen */
    suspend_curses();
   }
 if (allocate_temp_space(strlen((DEFCHAR *)cmd),TEMP_TEMP_CMD) != RC_OK)
    {
     display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
     trace_return();
#endif
     return(RC_OUT_OF_MEMORY);
    }
 if (strcmp((DEFCHAR *)cmd,"") == 0)
#ifdef XCURSES
   {
    strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)xterm_program);
    strcat((DEFCHAR *)temp_cmd," &");
   }
#else
    strcpy((DEFCHAR *)temp_cmd,getenv(SHELL));
#endif
 else
    strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)cmd);
#ifdef UNIX
 if (strcmp((DEFCHAR *)temp_cmd,"") == 0)  /* no SHELL env variable set */
   {
    printf("No SHELL environment variable set - using /bin/sh\n");
    fflush(stdout);
    strcpy((DEFCHAR *)temp_cmd,"/bin/sh");
   }
#endif
 if (quiet)
   {
#ifdef UNIX
    strcat((DEFCHAR *)temp_cmd," > /dev/null");
#endif
#if defined(DOS) || defined(OS2) || defined(WIN32)
    strcat((DEFCHAR *)temp_cmd," > nul:");
#endif
   }
 rc = system((DEFCHAR *)temp_cmd);
#ifndef XCURSES
 if (pause)
   {
    printf("\n\n%s",HIT_ANY_KEY);
    fflush(stdout);
   }
#endif
 if (!quiet && curses_started)
   {
    resume_curses();
    if (pause)
       (void)my_getch(stdscr);
#if defined(HAVE_BROKEN_SYSVR4_CURSES)
    {
     short x=0,y=0;
     getyx(CURRENT_WINDOW,y,x);
     force_curses_background();
     wmove(CURRENT_WINDOW,y,x);
     refresh();
    }
#endif
    restore_THE();
   }
 if (curses_started)
    draw_cursor(TRUE);

#ifdef XCURSES
 curses_started = save_curses_started;
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_makecurr(LINETYPE line)
#else
short execute_makecurr(line)
LINETYPE line;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_makecurr");
#endif
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);

 CURRENT_VIEW->current_line = line;
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
    getyx(CURRENT_WINDOW,y,x);
 else
    getyx(CURRENT_WINDOW_FILEAREA,y,x);
 build_screen(current_screen);
 display_screen(current_screen);
 y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                            CURRENT_VIEW->current_row);
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
    wmove(CURRENT_WINDOW,y,x);
 else
    wmove(CURRENT_WINDOW_FILEAREA,y,x);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_shift_command(short shift_left,short num_cols,LINETYPE true_line,LINETYPE num_lines,bool lines_based_on_scope,short target_type,bool sos)
#else
short execute_shift_command(shift_left,num_cols,true_line,num_lines,lines_based_on_scope,target_type,sos)
short shift_left,num_cols;
LINETYPE true_line,num_lines;
bool lines_based_on_scope;
short target_type;
bool sos;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short y=0,x=0;
 LINE *curr=NULL;
 LINETYPE abs_num_lines=(num_lines < 0L ? -num_lines : num_lines);
 LINETYPE num_file_lines=0L,i=0L;
 LINETYPE num_actual_lines=0L;
 LENGTHTYPE left_col=0;
 register short j=0;
 short actual_cols=0;
 short rc=RC_OK;
 short direction=(num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
 bool adjust_alt=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_shift_command");
#endif
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
    getyx(CURRENT_WINDOW_FILEAREA,y,x);
 else
    getyx(CURRENT_WINDOW,y,x);
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
 for (i=0L,num_actual_lines=0L;;i++)
   {
    if (lines_based_on_scope)
      {
       if (num_actual_lines == abs_num_lines)
          break;
      }
    else
      {
       if (abs_num_lines == num_file_lines)
          break;
      }
    rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(i*direction),curr);
    switch(rc)
      {
       case LINE_SHADOW:
            break;
/*       case LINE_TOF_EOF: MH12 */
       case LINE_TOF:
       case LINE_EOF:
            num_actual_lines++;
            break;
       default:
            memset(trec,' ',max_line_length);
            memcpy(trec,curr->line,curr->length);
            trec_len = curr->length;
            if (target_type == TARGET_BLOCK_CURRENT)
              {
               if (MARK_VIEW->mark_type == M_LINE)
                  left_col = CURRENT_VIEW->zone_start-1;
               else
                  left_col = MARK_VIEW->mark_start_col-1;
              }
            else
               left_col = CURRENT_VIEW->zone_start-1;
            if (shift_left)
              {
               actual_cols = min(num_cols,max(0,trec_len-left_col));
               memdeln(trec,left_col,trec_len,actual_cols);
               trec_len -= actual_cols;
              }
            else
              {
               for (j=0;j<num_cols;j++)
                  meminschr(trec,' ',left_col,max_line_length,trec_len++);
               if (trec_len > max_line_length)
                 {
                  trec_len = max_line_length;
                  display_error(0,(CHARTYPE*)"Truncated",FALSE);
                 }
               actual_cols = num_cols;
              }
/*---------------------------------------------------------------------*/
/* Set a flag to cause alteration counts to be incremented.            */
/*---------------------------------------------------------------------*/
            if (actual_cols != 0)
              {
               adjust_alt = TRUE;
/*---------------------------------------------------------------------*/
/* Add the old line contents to the line recovery list.                */
/*---------------------------------------------------------------------*/
               add_to_recovery_list(curr->line,curr->length);
/*---------------------------------------------------------------------*/
/* Realloc the dynamic memory for the line if the line is now longer.  */
/*---------------------------------------------------------------------*/
               if (trec_len > curr->length)
                 {
                  curr->line = (CHARTYPE *)(*the_realloc)((void *)curr->line,(trec_len+1)*sizeof(CHARTYPE));
                  if (curr->line == NULL)
                    {
                     display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
                     trace_return();
#endif
                     return(RC_OUT_OF_MEMORY);
                    }
                 }
/*---------------------------------------------------------------------*/
/* Copy the contents of trec into the line.                            */
/*---------------------------------------------------------------------*/
               memcpy(curr->line,trec,trec_len);
               curr->length = trec_len;
               *(curr->line+trec_len) = '\0';
               curr->flags.changed_flag = TRUE;
              }
            num_actual_lines++;
            break;
      }
/*---------------------------------------------------------------------*/
/* Proceed to the next record, even if the current record not in scope.*/
/*---------------------------------------------------------------------*/
    if (direction == DIRECTION_BACKWARD)
       curr = curr->prev;
    else
       curr = curr->next;
    num_file_lines += (LINETYPE)direction;
    if (curr == NULL)
       break;
   }
/*---------------------------------------------------------------------*/
/* Increment the alteration counters once if any line has changed...   */
/*---------------------------------------------------------------------*/
 if (adjust_alt)
    increment_alt(CURRENT_FILE);
/*---------------------------------------------------------------------*/
/* Display the new screen...                                           */
/*---------------------------------------------------------------------*/
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 resolve_current_and_focus_lines(CURRENT_VIEW,true_line,num_file_lines,direction,TRUE,sos);
#if 0
 if (!CURRENT_VIEW->stay                                 /* stay is off */
 &&  num_lines != 0L)
    CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line+num_lines-(LINETYPE)direction;
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 build_screen(current_screen);
 display_screen(current_screen);
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND
 &&  curses_started)
   {
    getyx(CURRENT_WINDOW,y,x);
    y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                               CURRENT_VIEW->current_row);
    wmove(CURRENT_WINDOW,y,x);
   }
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 if (CURRENT_TOF || CURRENT_BOF)
    return(RC_TOF_EOF_REACHED);
 else
    return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_set_lineflag( unsigned int new_flag, unsigned int changed_flag, unsigned int tag_flag, LINETYPE true_line, LINETYPE num_lines, bool lines_based_on_scope, short target_type )
#else
short execute_set_lineflag( new_flag, changed_flag, tag_flag, true_line, num_lines, lines_based_on_scope, target_type )
unsigned int new_flag, changed_flag, tag_flag;
LINETYPE true_line, num_lines;
bool lines_based_on_scope;
short target_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   unsigned short y=0,x=0;
   LINE *curr=NULL;
   LINETYPE abs_num_lines=(num_lines < 0L ? -num_lines : num_lines);
   LINETYPE num_file_lines=0L,i=0L;
   LINETYPE num_actual_lines=0L;
   short rc=RC_OK;
   short direction=(num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
   bool adjust_alt=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("execute.c: execute_set_lineflag");
#endif
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      getyx(CURRENT_WINDOW_FILEAREA,y,x);
   else
      getyx(CURRENT_WINDOW,y,x);
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   for (i=0L,num_actual_lines=0L;;i++)
   {
      if (lines_based_on_scope)
      {
         if (num_actual_lines == abs_num_lines)
            break;
      }
      else
      {
         if (abs_num_lines == num_file_lines)
            break;
      }
      rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(i*direction),curr);
      switch(rc)
      {
         case LINE_SHADOW:
            break;
         case LINE_TOF:
         case LINE_EOF:
            num_actual_lines++;
            break;
         default:
            if ( new_flag < 2 )
               curr->flags.new_flag = new_flag;
            if ( changed_flag < 2 )
               curr->flags.changed_flag = changed_flag;
            if ( tag_flag < 2 )
               curr->flags.tag_flag = tag_flag;
            num_actual_lines++;
            break;
      }
/*---------------------------------------------------------------------*/
/* Proceed to the next record, even if the current record not in scope.*/
/*---------------------------------------------------------------------*/
      if (direction == DIRECTION_BACKWARD)
         curr = curr->prev;
      else
         curr = curr->next;
      num_file_lines += (LINETYPE)direction;
      if (curr == NULL)
         break;
   }
/*---------------------------------------------------------------------*/
/* Increment the alteration counters once if any line has changed...   */
/*---------------------------------------------------------------------*/
   if (adjust_alt)
      increment_alt(CURRENT_FILE);
/*---------------------------------------------------------------------*/
/* Display the new screen...                                           */
/*---------------------------------------------------------------------*/
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   resolve_current_and_focus_lines(CURRENT_VIEW,true_line,num_file_lines,direction,TRUE,FALSE);
#ifdef THE_TRACE
   trace_return();
#endif
   if (CURRENT_TOF || CURRENT_BOF)
      return(RC_TOF_EOF_REACHED);
   else
      return(RC_OK);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static bool change_case(CHARTYPE *str,short start,short end,CHARTYPE which_case)
#else
static bool change_case(str,start,end,which_case)
CHARTYPE *str;
short start,end;
CHARTYPE which_case;
#endif
/*---------------------------------------------------------------------*/
/* Returns TRUE if a lines was changed, FALSE otherwise.               */
/* This function MUST proceed execute_change_case().                   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
   bool altered=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("execute.c: change_case");
#endif
   for (i=start;i<end+1;i++)
   {
      switch(which_case)
      {
         case CASE_UPPER:
            if (islower(*(str+i)))
            {
               *(str+i) = toupper(*(str+i));
               altered = TRUE;
            }
            break;
         case CASE_LOWER:
            if (isupper(*(str+i)))
            {
               *(str+i) = tolower(*(str+i));
               altered = TRUE;
            }
            break;
      }
   }
#ifdef THE_TRACE
   trace_return();
#endif
   return(altered);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_change_case(CHARTYPE *params,CHARTYPE which_case)
#else
short execute_change_case(params,which_case)
CHARTYPE *params;
CHARTYPE which_case;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE num_lines=0L,true_line=0L,num_actual_lines=0L,i=0L,num_file_lines=0L;
 short  direction=0;
 LINE *curr=NULL;
 LENGTHTYPE start_col=0,end_col=0;
 short rc=RC_OK;
 TARGET target;
 short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
 bool lines_based_on_scope=TRUE;
 bool adjust_alt=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_change_case");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/* Valid values are: a target or "block".                              */
/* If no parameter is supplied, 1 is assumed.                          */
/*---------------------------------------------------------------------*/
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
 true_line = target.true_line;
/*---------------------------------------------------------------------*/
/* If the target is BLOCK set the left and right margins to be the     */
/* margins of the BOX BLOCK, otherwise use ZONE settings.              */
/*---------------------------------------------------------------------*/
 start_col = CURRENT_VIEW->zone_start-1;
 end_col = CURRENT_VIEW->zone_end-1;
 if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
   {
    num_lines = MARK_VIEW->mark_end_line-MARK_VIEW->mark_start_line+1L;
    true_line = MARK_VIEW->mark_start_line;
    direction = DIRECTION_FORWARD;
    lines_based_on_scope = FALSE;
    if (MARK_VIEW->mark_type != M_LINE)
      {
       start_col = MARK_VIEW->mark_start_col-1;
       end_col   = MARK_VIEW->mark_end_col-1;
      }
   }
/*---------------------------------------------------------------------*/
/* Find the current LINE pointer for the true_line.                    */
/* This is the first line to change.                                   */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* Change the case for the target lines and columns...                 */
/*---------------------------------------------------------------------*/
 for (i=0L,num_actual_lines=0L;;i++)
   {
    if (lines_based_on_scope)
      {
       if (num_actual_lines == num_lines)
          break;
      }
    else
      {
       if (num_lines == i)
          break;
      }
    rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(i*direction),curr);
    switch(rc)
      {
       case LINE_SHADOW:
            break;
/*       case LINE_TOF_EOF: MH12 */
       case LINE_TOF:
       case LINE_EOF:
            num_actual_lines++;
            break;
       default:
            add_to_recovery_list(curr->line,curr->length);
            if (MARK_VIEW
            &&  MARK_VIEW->mark_type == M_STREAM)
              {
               int mystart=0,myend=curr->length-1;
               if (true_line + i == MARK_VIEW->mark_start_line)
                  mystart = start_col;
               if (true_line + i == MARK_VIEW->mark_end_line)
                  myend = end_col;
               rc = change_case(curr->line,mystart,min(curr->length-1,myend),which_case);
              }
            else
               rc = change_case(curr->line,start_col,min(curr->length-1,end_col),which_case);
            if (rc)
              {
               adjust_alt = TRUE;
               curr->flags.changed_flag = TRUE;
              }
            num_actual_lines++;
            break;
       }
/*---------------------------------------------------------------------*/
/* Proceed to the next record, even if the current record not in scope.*/
/*---------------------------------------------------------------------*/
     if (direction == DIRECTION_FORWARD)
        curr = curr->next;
     else
        curr = curr->prev;
     num_file_lines += (LINETYPE)direction;
     if (curr == NULL)
        break;
    }
/*---------------------------------------------------------------------*/
/* Increment the alteration counts if any lines changed...             */
/*---------------------------------------------------------------------*/
 if (adjust_alt)
    increment_alt(CURRENT_FILE);
/*---------------------------------------------------------------------*/
/* Display the new screen...                                           */
/*---------------------------------------------------------------------*/
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 resolve_current_and_focus_lines(CURRENT_VIEW,true_line,num_file_lines,direction,TRUE,FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 if (CURRENT_TOF || CURRENT_BOF)
    return(RC_TOF_EOF_REACHED);
 else
    return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short rearrange_line_blocks(CHARTYPE command,CHARTYPE source,
                            LINETYPE start_line,LINETYPE end_line,LINETYPE dest_line,
                            short num_occ,VIEW_DETAILS *src_view,VIEW_DETAILS *dst_view,
                            bool lines_based_on_scope,LINETYPE *lines_affected)
#else
short rearrange_line_blocks(command,source,start_line,end_line,dest_line,num_occ,
                            src_view,dst_view,lines_based_on_scope,lines_affected)
CHARTYPE command,source;
LINETYPE start_line,end_line,dest_line;
short num_occ;
VIEW_DETAILS *src_view,*dst_view;
bool lines_based_on_scope;
LINETYPE *lines_affected;
#endif
/***********************************************************************/
/* Parameters:                                                         */
/*    command: the command being executed; COPY,DELETE,DUPLICATE,MOVE  */
/*     source: where the command is executed; COMMAND, PREFIX, BLOCK   */
/* start_line: the first line (or only line) number to be acted on     */
/*   end_line: the last line number to be acted on                     */
/*  dest_line: the destination line for copy,move and duplicate. For   */
/*             delete this is not applicable.                          */
/*    num_occ: the number of times to execute the command; only for DUP*/
/* lines_affected: number of "real" lines affected by copy operation   */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short j=0,k=0;
 short rc=RC_OK;
 static unsigned short y=0,x=0;
 bool dst_inside_src=FALSE,lines_added=FALSE,reset_block=FALSE;
 bool dest_in_block=FALSE;
 short  direction=0;
 LINETYPE num_lines=0L,off=0L,adjust_line=dest_line,num_actual_lines=0L;
 LINETYPE i=0L,num_pseudo_lines=0L;
 LINE *curr_src=NULL,*curr_dst=NULL;
 LINE *save_curr_src=NULL,*save_curr_dst=NULL;
 FILE_DETAILS *src_file=NULL,*dst_file=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: rearrange_line_blocks");
#endif
 src_file = src_view->file_for_view;
 dst_file = dst_view->file_for_view;
 if (source == SOURCE_BLOCK)
    reset_block = FALSE;
 else
    reset_block = TRUE;
/*---------------------------------------------------------------------*/
/* This block of commands is for copying lines...                      */
/*---------------------------------------------------------------------*/
 switch(command)
   {
    case COMMAND_COPY:
    case COMMAND_OVERLAY_COPY:
    case COMMAND_MOVE_COPY_SAME:
    case COMMAND_MOVE_COPY_DIFF:
    case COMMAND_DUPLICATE:
         lines_added = TRUE;
         switch(source)
           {
            case SOURCE_BLOCK:
            case SOURCE_BLOCK_RESET:
                 if (src_view == dst_view
                 &&  dest_line >= start_line
                 &&  dest_line <  end_line)
                     dest_in_block = TRUE;
                 break;
            case SOURCE_PREFIX:
                 if (dest_line >= start_line
                 &&  dest_line <  end_line)
                     dest_in_block = TRUE;
                 break;
            default:
                 break;
           }
/*---------------------------------------------------------------------*/
/* If the destination line is within the marked block then we have to  */
/* handle the processing of the src_curr pointer differently.          */
/*---------------------------------------------------------------------*/
         if (dest_in_block)
           {
            dst_inside_src = TRUE;
            off = dest_line - start_line;
           }
         else
            dst_inside_src = FALSE;
         if (start_line > end_line)
           {
            direction = DIRECTION_BACKWARD;
            num_lines = start_line - end_line + 1L;
           }
         else
           {
            direction = DIRECTION_FORWARD;
            num_lines = end_line - start_line + 1L;
           }
         save_curr_src = lll_find(src_file->first_line,src_file->last_line,start_line,src_file->number_lines);
         save_curr_dst = lll_find(dst_file->first_line,dst_file->last_line,dest_line,dst_file->number_lines);
         for (k=0;k<num_occ;k++)
           {
            curr_src = save_curr_src;
            curr_dst = save_curr_dst;
            for (i=0L,num_actual_lines=0L;;i++)
               {
                if (lines_based_on_scope)
                  {
                   if (num_actual_lines == num_lines)
                      break;
                  }
                else
                  {
                   if (num_lines == i)
                      break;
                  }
                rc = processable_line(src_view,start_line+(i*direction),curr_src);
                switch(rc)
                  {
                   case LINE_SHADOW:
                        break;
/*                   case LINE_TOF_EOF: MH12 */
                   case LINE_TOF:
                   case LINE_EOF:
                        num_actual_lines++;
                        num_pseudo_lines++;
                        break;
                   default:
                        if ((curr_dst = add_LINE(dst_file->first_line,curr_dst,
                                        curr_src->line,curr_src->length,
                                        dst_view->display_low,TRUE)) == NULL)
/*                                      curr_src->select)) == NULL)*/
                          {
                           display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
                           trace_return();
#endif
                           return(RC_OUT_OF_MEMORY);
                          }
/*---------------------------------------------------------------------*/
/* If moving lines within the same file, move any line name with the   */
/* line also.                                                          */
/*---------------------------------------------------------------------*/
                          if (command == COMMAND_MOVE_COPY_SAME)
                            {
                             if (curr_src->name != (CHARTYPE *)NULL)
                               {
                                curr_dst->name = curr_src->name;
                                curr_src->name = (CHARTYPE *)NULL;
                               }
                            }
                          if (direction == DIRECTION_BACKWARD)
                          {
                             curr_dst = save_curr_dst;
                          }
                          num_actual_lines++;
                          break;
                  }
                   if (dst_inside_src && i == off)
                   {
                      for (j=0;j<off+1;j++)
                      {
                         curr_src = curr_src->next;
                      }
                   }
                   if (direction == DIRECTION_FORWARD)
                      curr_src = curr_src->next;
                   else
                      curr_src = curr_src->prev;
              }
            }
         dst_file->number_lines += (num_actual_lines-num_pseudo_lines)*num_occ;
         *lines_affected = (num_actual_lines-num_pseudo_lines)*num_occ;
         break;
    default:
         break;
   }
/*---------------------------------------------------------------------*/
/* This block of commands is for deleting lines...                     */
/*---------------------------------------------------------------------*/
 switch(command)
   {
    case COMMAND_DELETE:
    case COMMAND_OVERLAY_DELETE:
    case COMMAND_MOVE_DELETE_SAME:
    case COMMAND_MOVE_DELETE_DIFF:
         lines_added = FALSE;
         if (start_line > end_line)
           {
            direction = DIRECTION_BACKWARD;
            num_lines = start_line - end_line + 1L;
           }
         else
           {
            direction = DIRECTION_FORWARD;
            num_lines = end_line - start_line + 1L;
           }
         curr_dst = lll_find(dst_file->first_line,dst_file->last_line,start_line,dst_file->number_lines);
         for (i=0L,num_actual_lines=0L;;i++)
           {
            if (lines_based_on_scope)
              {
               if (num_actual_lines == num_lines)
                  break;
              }
            else
              {
               if (num_lines == i)
                  break;
              }
            rc = processable_line(dst_view,start_line+(i*direction),curr_dst);
            switch(rc)
              {
/*               case LINE_TOF_EOF: MH12 */
               case LINE_TOF:
               case LINE_EOF:
                    num_actual_lines++; /* this is meant to fall through */
                    num_pseudo_lines++;
               case LINE_SHADOW:
                    if (direction == DIRECTION_FORWARD)
                       curr_dst = curr_dst->next;
                    else
                       curr_dst = curr_dst->prev;
                    break;
               default:
                    if (command != COMMAND_MOVE_DELETE_SAME)
                       add_to_recovery_list(curr_dst->line,curr_dst->length);
                    curr_dst = delete_LINE(dst_file->first_line,dst_file->last_line,curr_dst,direction);
                    num_actual_lines++;
              }
            if (curr_dst == NULL)
               break;
           }
         dst_file->number_lines -= (num_actual_lines-num_pseudo_lines)*num_occ;
         break;
    default:
         break;
   }
/*---------------------------------------------------------------------*/
/* Increment alteration count for all but COMMAND_MOVE_COPY_SAME...    */
/*---------------------------------------------------------------------*/
 if (command != COMMAND_MOVE_COPY_SAME
 &&  command != COMMAND_OVERLAY_COPY
 &&  (num_actual_lines-num_pseudo_lines) != 0)
    increment_alt(dst_file);
/*---------------------------------------------------------------------*/
/* This block of commands is for sorting out cursor position...        */
/*---------------------------------------------------------------------*/
 if (curses_started)
    getyx(CURRENT_WINDOW,y,x);
 switch(command)
   {
    case COMMAND_COPY:
    case COMMAND_OVERLAY_COPY:
    case COMMAND_MOVE_COPY_SAME:
    case COMMAND_MOVE_COPY_DIFF:
    case COMMAND_DUPLICATE:
         if (source == SOURCE_COMMAND
         &&  CURRENT_VIEW->current_window == WINDOW_COMMAND
         &&  CURRENT_VIEW->stay)
             break;
#if PRE_23_BEHAVIOUR
         if (IN_VIEW(dst_view,dest_line))
           {
            dst_view->focus_line = dest_line+1L;
           }
         if (dst_view->current_window != WINDOW_COMMAND
         &&  dst_view == CURRENT_SCREEN.screen_view)
           {
            if (y == CURRENT_SCREEN.rows[WINDOW_FILEAREA]-1)/* on bottom line of window */
              {
               dst_view->current_line = dst_view->focus_line;
               y = dst_view->current_row;
              }
            else
               y = get_row_for_focus_line(current_screen,dst_view->focus_line,
                                          dst_view->current_row);
           }
#else
         if (command == COMMAND_DUPLICATE)
           {
            dst_view->focus_line = dest_line+1L;
            if (dst_view == CURRENT_SCREEN.screen_view)
              {
               unsigned short last_focus_row=0;
               find_last_focus_line(&last_focus_row);
               if (dest_line >= CURRENT_SCREEN.sl[last_focus_row].line_number)
                 {
                  dst_view->current_line = dst_view->focus_line;
                  y = dst_view->current_row;
                 }
               else
                 {
                  y = get_row_for_focus_line(current_screen,dst_view->focus_line,
                                             dst_view->current_row);
                 }
              }
           }
         else
           {
            if (IN_VIEW(dst_view,dest_line))
              {
               dst_view->focus_line = dest_line+1L;
              }
            if (dst_view->current_window != WINDOW_COMMAND
            &&  dst_view == CURRENT_SCREEN.screen_view)
              {
               unsigned short last_focus_row=0;
               find_last_focus_line(&last_focus_row);
               if (y == last_focus_row)
                 {
                  dst_view->current_line = dst_view->focus_line;
                  y = dst_view->current_row;
                 }
               else
                  y = get_row_for_focus_line(current_screen,dst_view->focus_line,
                                             dst_view->current_row);
              }
           }
#endif
         break;
    case COMMAND_DELETE:
    case COMMAND_OVERLAY_DELETE:
    case COMMAND_MOVE_DELETE_SAME:
    case COMMAND_MOVE_DELETE_DIFF:
         if (dst_view->focus_line >= start_line
         &&  dst_view->focus_line <= end_line)
           {
            if (IN_VIEW(dst_view,dest_line))
              {
               if (dst_view->current_line > dst_file->number_lines+1L)
                 {
#if 0
                  dst_view->current_line -= (num_actual_lines-num_pseudo_lines);
#else
                  /*
                   * This is better than before, but the cursor still ends up NOT on the
                   * focus line ?????
                   */
                  dst_view->current_line = dst_file->number_lines+1L;
/*                dst_view->focus_line = dst_view->current_line; */
                  dst_view->focus_line = dest_line;
#endif
                 }
               else
                  dst_view->focus_line = dest_line;
              }
            else
              {
               if (dest_line > dst_file->number_lines)
                  dst_view->focus_line = dst_view->current_line = dst_file->number_lines;
               else
                  dst_view->focus_line = dst_view->current_line = dest_line;
              }
           }
         else
           {
            dest_line = (dst_view->focus_line < start_line ? dst_view->focus_line : dst_view->focus_line - num_lines);
            if (IN_VIEW(dst_view,dest_line))
              {
               if (dst_view->current_line > dst_file->number_lines+1L)
                  dst_view->current_line -= (num_actual_lines-num_pseudo_lines);
               dst_view->focus_line = dest_line;
              }
            else
              {
               if (dest_line > dst_file->number_lines)
                  dst_view->focus_line = dst_view->current_line = dst_file->number_lines;
               else
                  dst_view->focus_line = dst_view->current_line = dest_line;
              }
           }
         if (dst_file->number_lines == 0L)
               dst_view->focus_line = dst_view->current_line = 0L;
         if (dst_view->current_window != WINDOW_COMMAND
         &&  dst_view == CURRENT_SCREEN.screen_view)
           {
            build_screen(current_screen); /* MH */
            y = get_row_for_focus_line(current_screen,dst_view->focus_line,
                                       dst_view->current_row);
           }
/*---------------------------------------------------------------------*/
/* This is set here so that the adjust_pending_prefix command will work*/
/*---------------------------------------------------------------------*/
         if (direction == DIRECTION_BACKWARD)
            adjust_line = end_line;
         else
            adjust_line = start_line;
         dst_view->current_line = find_next_in_scope(dst_view,NULL,dst_view->current_line,DIRECTION_FORWARD);
         break;
    default:
         break;
   }
/*---------------------------------------------------------------------*/
/* This block of commands is for adjusting prefix and block lines...   */
/*---------------------------------------------------------------------*/
 switch(source)
   {
    case SOURCE_BLOCK:
    case SOURCE_BLOCK_RESET:
         adjust_pending_prefix(dst_view,lines_added,adjust_line,(num_actual_lines-num_pseudo_lines)*num_occ);
         if (command == COMMAND_MOVE_DELETE_SAME)
            adjust_marked_lines(lines_added,adjust_line,(num_actual_lines-num_pseudo_lines)*num_occ);
         else
            switch(command)
              {
               case COMMAND_MOVE_DELETE_DIFF:
                    src_view->marked_line = src_view->marked_col = FALSE;
                    break;
               case COMMAND_OVERLAY_DELETE:
                    break;
               default:
                    if (command == COMMAND_COPY
                    &&  src_view != dst_view)
                       src_view->marked_line = src_view->marked_col = FALSE;
/*---------------------------------------------------------------------*/
/* The following does a 'reset block' in the current view.             */
/*---------------------------------------------------------------------*/
                    if (reset_block)
                      {
                       dst_view->marked_line = dst_view->marked_col = FALSE;
                       MARK_VIEW = (VIEW_DETAILS *)NULL;
                      }
                    else
                      {
                       if (command == COMMAND_OVERLAY_DELETE)
                          dst_view->mark_start_line = dest_line;
                       else
                          dst_view->mark_start_line = dest_line + 1L;
                       dst_view->mark_end_line = dest_line + (num_actual_lines-num_pseudo_lines);
                       dst_view->marked_col = FALSE;
                       dst_view->mark_start_col = src_view->mark_start_col;
                       dst_view->mark_end_col = src_view->mark_end_col;
                       dst_view->mark_type = M_LINE;
                       dst_view->focus_line = dst_view->mark_start_line;
                       MARK_VIEW = dst_view;
                      }
                    break;
              }
         break;
    case SOURCE_PREFIX:
    case SOURCE_COMMAND:
         adjust_marked_lines(lines_added,adjust_line,(num_actual_lines-num_pseudo_lines)*num_occ);
         adjust_pending_prefix(dst_view,lines_added,adjust_line,(num_actual_lines-num_pseudo_lines)*num_occ);
         break;
   }
 if (command != COMMAND_MOVE_DELETE_DIFF)
    pre_process_line(CURRENT_VIEW,dst_view->focus_line,(LINE *)NULL);
 if ((source == SOURCE_BLOCK
 ||   source == SOURCE_BLOCK_RESET)
 && display_screens > 1)
   {
    build_screen(other_screen);
    display_screen(other_screen);
   }
 if (command != COMMAND_MOVE_DELETE_DIFF
 &&  command != COMMAND_MOVE_COPY_SAME
 &&  command != COMMAND_OVERLAY_COPY
 &&  command != COMMAND_OVERLAY_DELETE)
   {
    build_screen(current_screen);
    display_screen(current_screen);
   }
 if (dst_view->current_window != WINDOW_COMMAND
 &&  dst_view == CURRENT_SCREEN.screen_view)
   {
    if (curses_started)
       wmove(CURRENT_WINDOW,y,x);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_set_point(CHARTYPE *name,LINETYPE true_line,bool point_on)
#else
short execute_set_point(name,true_line,point_on)
CHARTYPE *name;
LINETYPE true_line;
bool point_on;
#endif
/***********************************************************************/
/* Parameters:                                                         */
/*       name: the name of the line to be processed                    */
/*  true_line: the line number of the line                             */
/*   point_on: indicates if the line name is to be turned on or off    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=NULL;
 LINETYPE dummy=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_set_point");
#endif
 if (point_on)
   {
/*---------------------------------------------------------------------*/
/* Find a line that already has the same name. If one exists, remove   */
/* the name.                                                           */
/*---------------------------------------------------------------------*/
    if ((curr = find_named_line(name,&dummy,FALSE)) != (LINE *)NULL)
      {
       (*the_free)(curr->name);
       curr->name = (CHARTYPE *)NULL;
      }
/*---------------------------------------------------------------------*/
/* Allocate space for the name and attach it to the true_line.         */
/*---------------------------------------------------------------------*/
    curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
    if ((curr->name=(CHARTYPE *)(*the_malloc)(strlen((DEFCHAR *)name)+1)) == NULL)
      {
       display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OUT_OF_MEMORY);
      }
    strcpy((DEFCHAR *)curr->name,(DEFCHAR *)name);
   }
 else
   {
/*---------------------------------------------------------------------*/
/* Find a line that already has the same name. If one exists, remove   */
/* the name otherwise display an error.                                */
/*---------------------------------------------------------------------*/
    if ((curr = find_named_line(name,&dummy,FALSE)) != (LINE *)NULL)
      {
       (*the_free)(curr->name);
       curr->name = (CHARTYPE *)NULL;
      }
    else
      {
       display_error(60,name,FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
   }

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_wrap_word(unsigned short col)
#else
short execute_wrap_word(col)
unsigned short col;
#endif
/***********************************************************************/
/* Parameters: col   - current column position within rec              */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short col_break=0,cursor_offset=0;
 LINE *curr=NULL,*next_line=NULL;
 bool bnewline=FALSE,cursor_wrap=FALSE;
 CHARTYPE *buf=NULL,*word_to_wrap=NULL;
 short next_line_start=0,length_word=0,last_col=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_wrap_word");
#endif
/*---------------------------------------------------------------------*/
/* This function is called when the length of the focus line exceeds   */
/* the right margin. If the cursor is positioned in the last word of   */
/* the line, the cursor moves with that word to the next line.         */
/* If the combined length of the word to be wrapped and a space and the*/
/* line following the focus line exceeds the right margin, a new line  */
/* is inserted with the word being wrapped, otherwise the word to be   */
/* wrapped is prepended to the following line.                         */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/* Find the current LINE pointer for the focus_line.                   */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->focus_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* Determine where to start splitting the line in relation to end of   */
/* line...                                                             */
/*---------------------------------------------------------------------*/
 col_break = memreveq(rec,' ',rec_len);
/*---------------------------------------------------------------------*/
/* If there is no word break, don't attempt any wrap.                  */
/*---------------------------------------------------------------------*/
 if (col_break == (-1))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* Actual column to break on is 1 character to right of last space.    */
/*---------------------------------------------------------------------*/
 col_break++;
/*---------------------------------------------------------------------*/
/* Make a null terminated string out of current line so we can grab the*/
/* word to be wrapped.                                                 */
/*---------------------------------------------------------------------*/
 rec[rec_len] = '\0';
/*---------------------------------------------------------------------*/
/* Point to word to wrap and determine its length.                     */
/*---------------------------------------------------------------------*/
 length_word = rec_len - col_break;
 word_to_wrap = (CHARTYPE *)rec+col_break;
/*---------------------------------------------------------------------*/
/* If the position of the cursor is before the word to wrap leave the  */
/* cursor where it is.                                                 */
/*---------------------------------------------------------------------*/
 if (col >= col_break)
   {
    cursor_wrap = TRUE;
    cursor_offset = col - col_break - 1;
   }
 else
    cursor_wrap = FALSE;
/*---------------------------------------------------------------------*/
/* Now we have to work out if a new line is to added or we prepend to  */
/* the following line...                                               */
/*---------------------------------------------------------------------*/
 if (curr->next->next == NULL)             /* next line bottom of file */
    bnewline = TRUE;
 else
   {
    next_line = curr->next;
    if (!IN_SCOPE(CURRENT_VIEW,next_line))
       bnewline = TRUE;
    else
      {
       next_line_start = memne(next_line->line,' ',next_line->length);
       if (next_line_start != CURRENT_VIEW->margin_left-1) /* next line doesn't start in left margin */
          bnewline = TRUE;
       else
         {
          if (next_line->length + length_word + 1 > CURRENT_VIEW->margin_right)
             bnewline = TRUE;
         }
      }
   }
/*---------------------------------------------------------------------*/
/* Save the word to be wrapped...                                      */
/*---------------------------------------------------------------------*/
 buf = (CHARTYPE *)(*the_malloc)(length_word+CURRENT_VIEW->margin_left);
 if (buf == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 memcpy(buf,word_to_wrap,length_word);
/*---------------------------------------------------------------------*/
/* Remove the word to be wrapped from the focus line buffer; rec...    */
/*---------------------------------------------------------------------*/
 for (i=col_break;i<rec_len+1;i++)
    rec[i] = ' ';
 last_col = memrevne(rec,' ',max_line_length);
 rec_len = (last_col == (-1)) ? 0 : last_col + 1;
/*---------------------------------------------------------------------*/
/* We now should know if a new line is to added or not.                */
/*---------------------------------------------------------------------*/
 if (bnewline)
   {
    for (i=0;i<CURRENT_VIEW->margin_left-1;i++)
       (void)meminschr(buf,' ',0,max_line_length,i+length_word);
    curr = add_LINE(CURRENT_FILE->first_line,curr,buf,length_word+CURRENT_VIEW->margin_left-1,curr->select,TRUE);
    CURRENT_FILE->number_lines++;
   }
 else
   {
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line+1L,(LINE *)NULL);
    (void)meminschr(rec,' ',CURRENT_VIEW->margin_left-1,max_line_length,rec_len++);
    (void)meminsmem(rec,buf,length_word,CURRENT_VIEW->margin_left-1,
                    max_line_length,rec_len);
    rec_len += length_word;
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line+1L,(LINE *)NULL,TRUE);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   }
 (*the_free)(buf);
/*---------------------------------------------------------------------*/
/* We now should know if the cursor is to wrap or stay where it is.    */
/*---------------------------------------------------------------------*/
 if (cursor_wrap)
   {
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
    build_screen(current_screen);
    THEcursor_down(TRUE);
    rc = Sos_firstchar((CHARTYPE *)"");
    for (i=0;i<cursor_offset+1;i++)
       rc = THEcursor_right(TRUE,FALSE);
   }
 else
   {
    if (INSERTMODEx)
       rc = THEcursor_right(TRUE,FALSE);
   }
 build_screen(current_screen);
 display_screen(current_screen);

#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_split_join(short action,bool aligned,bool cursorarg)
#else
short execute_split_join(action,aligned,cursorarg)
short action;
bool aligned;
bool cursorarg;
#endif
/***********************************************************************/
/* Parameters: action  - split, join or spltjoin                       */
/*             aligned - whether to align text or not                  */
/*             cursor  - whether to split focus line at cursor or      */
/*                       command line at current column                */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short num_cols=0,num_blanks_focus=0,num_blanks_next=0;
 unsigned short x=0,y=0;
 LINE *curr=NULL;
 LINETYPE true_line=0L;
 LENGTHTYPE col=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_split_join");
#endif
/*---------------------------------------------------------------------*/
/* Determine line and column to use.                                   */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND
 ||  cursorarg == FALSE)
 {
    col = CURRENT_VIEW->current_column-1;
    true_line = CURRENT_VIEW->current_line;
 }
 else
 {
    if (curses_started)
    {
       getyx(CURRENT_WINDOW_FILEAREA,y,x);
       col = (x+CURRENT_VIEW->verify_col-1);
       true_line = CURRENT_VIEW->focus_line;
    }
    else
    {
       col = 0;
       true_line = CURRENT_VIEW->current_line;
    }
 }
/*---------------------------------------------------------------------*/
/* Reject the command if true_line is top or bottom of file.           */
/*---------------------------------------------------------------------*/
 if (VIEW_TOF(CURRENT_VIEW,true_line) || VIEW_BOF(CURRENT_VIEW,true_line))
 {
    display_error(38,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_ENVIRON);
 }
/*---------------------------------------------------------------------*/
/* If this function called from SPLTJOIN(), then determine if SPLIT    */
/* or JOIN is required.                                                */
/*---------------------------------------------------------------------*/
 if (action == SPLTJOIN_SPLTJOIN)
    action = (col >= rec_len) ? SPLTJOIN_JOIN : SPLTJOIN_SPLIT;
/*---------------------------------------------------------------------*/
/* Copy any changes in the focus line to the linked list.              */
/*---------------------------------------------------------------------*/
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*---------------------------------------------------------------------*/
/* Find the current LINE pointer for the true line.                    */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);

 getyx(CURRENT_WINDOW,y,x);
 switch(action)
 {
    case SPLTJOIN_SPLIT:
         memset(rec,' ',max_line_length);
         if (col < curr->length)
         {
            memcpy(rec,curr->line+col,curr->length-col);
            rec_len = curr->length-col;
         }
         else
            rec_len = 0;
         curr->flags.changed_flag = TRUE;
/*---------------------------------------------------------------------*/
/* Calculate the number of leading blanks on the current line so that  */
/* the new line can have this many blanks prepended to align properly. */
/*---------------------------------------------------------------------*/
         if (aligned)
         {
            num_cols = memne(curr->line,' ',curr->length);
            if (num_cols == (-1))
               num_cols = 0;
            for (i=0;i<num_cols;i++)
               meminschr(rec,' ',0,max_line_length,rec_len++);
            rec_len = min(rec_len,max_line_length);
         }
         add_LINE(CURRENT_FILE->first_line,curr,(rec),rec_len,curr->select,TRUE);
         CURRENT_FILE->number_lines++;
         if (CURRENT_VIEW->current_window == WINDOW_COMMAND
         ||  cursorarg == FALSE)
         {
            if (curr->length > col)
            {
               curr->length = col;
               *(curr->line+(col)) = '\0';
               increment_alt(CURRENT_FILE);
            }
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         }
         else
         {
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
            Sos_delend((CHARTYPE *)"");
            post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
         }
         break;
    case SPLTJOIN_JOIN:
         if (curr->next->next == NULL)
         {
/*---------------------------------------------------------------------*/
/* Trying to join with the bottom of file line.                        */
/*---------------------------------------------------------------------*/
#ifdef THE_TRACE
            trace_return();
#endif
            return(RC_INVALID_ENVIRON);
         }
/*---------------------------------------------------------------------*/
/* Calculate the number of leading blanks for the focus line and also  */
/* for the line to be joined. To align the join properly, we have to   */
/* remove up the number of leading blanks in the focus line from the   */
/* beginning of the line to be joined.                                 */
/*---------------------------------------------------------------------*/
         if (aligned)
         {
            num_blanks_focus = memne(curr->line,' ',curr->length);
            if (num_blanks_focus == (-1))
               num_blanks_focus = 0;
            num_blanks_next = memne(curr->next->line,' ',curr->length);
            if (num_blanks_next == (-1))
               num_blanks_next = 0;
            num_cols = min(num_blanks_focus,num_blanks_next);
         }
         else
            num_cols = 0;
         if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->current_line,(LINE *)NULL);
         meminsmem(rec,curr->next->line+num_cols,curr->next->length-num_cols,
                      col,max_line_length,col);
         if (col+curr->next->length-num_cols > max_line_length)
         {
            display_error(0,(CHARTYPE*)"Truncated",FALSE);
         }
         rec_len = min(max_line_length,col+curr->next->length-num_cols);
         post_process_line(CURRENT_VIEW,true_line,(LINE *)NULL,TRUE);
         curr = delete_LINE(CURRENT_FILE->first_line,CURRENT_FILE->last_line,curr->next,DIRECTION_BACKWARD);
         if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
/*---------------------------------------------------------------------*/
/* If on the bottom line, use the previous line.                       */
/*---------------------------------------------------------------------*/
         if (CURRENT_BOF)
         {
            CURRENT_VIEW->current_line--;
            y++;
         }
/*---------------------------------------------------------------------*/
/* Decrement the number of lines counter for the current file and move */
/* the cursor to the appropriate line.                                 */
/*---------------------------------------------------------------------*/
         CURRENT_FILE->number_lines--;
         if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
            wmove(CURRENT_WINDOW,y,x);
         break;
 }
 /*
  * Determine new current line
  */
 if (action == SPLTJOIN_SPLIT
 &&  compatible_feel == COMPAT_XEDIT
 &&  CURRENT_VIEW->current_window == WINDOW_COMMAND)
 {
    CURRENT_VIEW->current_line++;
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
    if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
    {
       build_screen(current_screen);
       if (!line_in_view(current_screen,CURRENT_VIEW->focus_line)
       &&  compatible_feel == COMPAT_XEDIT)
       {
          THEcursor_cmdline(1);
       }
       else
       {
          y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                                  CURRENT_VIEW->current_row);
          wmove(CURRENT_WINDOW,y,x);
       }
    }
 }
/*---------------------------------------------------------------------*/
/* Fix the positioning of the marked block (if there is one and it is  */
/* in the current view) and any pending prefix commands.               */
/*---------------------------------------------------------------------*/
 adjust_marked_lines((action==SPLTJOIN_SPLIT)?TRUE:FALSE,true_line,1L);
 adjust_pending_prefix(CURRENT_VIEW,(action==SPLTJOIN_SPLIT)?TRUE:FALSE,true_line,1L);
 build_screen(current_screen);
 display_screen(current_screen);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_put(CHARTYPE *params,bool putdel)
#else
short execute_put(params,putdel)
CHARTYPE *params;
bool putdel;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE num_lines=0L,true_line=0L,num_file_lines=0L;
 bool append=FALSE;
 CHARTYPE *filename=NULL;
 short rc=RC_OK;
 LENGTHTYPE start_col=0,end_col=max_line_length;
 bool lines_based_on_scope = TRUE;
 TARGET target;
 short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL|TARGET_SPARE;
 short direction=DIRECTION_FORWARD;
 bool clip=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_put");
#endif
/*---------------------------------------------------------------------*/
/* If there are no arguments, default to "1"...                        */
/*---------------------------------------------------------------------*/
 if (strcmp("",(DEFCHAR *)params) == 0)
    params = (CHARTYPE *)"1";
/*---------------------------------------------------------------------*/
/* Validate first argument as a target...                              */
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
/*---------------------------------------------------------------------*/
/* If there is no second argument, no filename supplied...             */
/*---------------------------------------------------------------------*/
 if (target.spare == (-1))
 {
    append = FALSE;
    filename = tempfilename;
 }
 else
 {
    if (equal((CHARTYPE *)"clip:",MyStrip(target.rt[target.spare].string,STRIP_BOTH,' '),5))
    {
       clip = TRUE;
    }
    else
    {
       if ((rc = splitpath(strtrunc(target.rt[target.spare].string))) != RC_OK)
       {
          display_error(10,strtrunc(target.rt[target.spare].string),FALSE);
          free_target(&target);
#ifdef THE_TRACE
          trace_return();
#endif
          return(rc);
       }
       strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)sp_path);
       strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)sp_fname);
       filename = temp_cmd;
       append = TRUE;
    }
 }
 true_line = target.true_line;
 num_lines = target.num_lines;
/*---------------------------------------------------------------------*/
/* Determine in which direction we are working.                        */
/*---------------------------------------------------------------------*/
 direction = (target.num_lines < 0L) ? DIRECTION_BACKWARD : DIRECTION_FORWARD;
/*---------------------------------------------------------------------*/
/* If the marked block is a BOX block, set up the left and right column*/
/* values.                                                             */
/*---------------------------------------------------------------------*/
 if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
   {
    lines_based_on_scope = FALSE;
    if (MARK_VIEW->mark_type == M_BOX)
      {
       start_col = MARK_VIEW->mark_start_col-1;
       end_col = MARK_VIEW->mark_end_col-1;
      }
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if (clip)
 {
    if ((rc = setclipboard(CURRENT_FILE,
                     filename,
                     TRUE,
                     num_lines,
                     true_line,
                     &num_file_lines,
                     append,
                     start_col,
                     end_col,
                     FALSE,
                     lines_based_on_scope)) != RC_OK)
    {
       free_target(&target);
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
    }
 }
 else
 {
    if ((rc = save_file(CURRENT_FILE,
                     filename,
                     TRUE,
                     num_lines,
                     true_line,
                     &num_file_lines,
                     append,
                     start_col,
                     end_col,
                     FALSE,
                     lines_based_on_scope,
                     FALSE)) != RC_OK)
    {
       free_target(&target);
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
    }
 }
/*---------------------------------------------------------------------*/
/* If we are executing a putd command, delete the target...            */
/*---------------------------------------------------------------------*/
 if (putdel)
    rc = DeleteLine(target.string);
 else
   {
/*---------------------------------------------------------------------*/
/* If STAY is OFF, change the current and focus lines by the number    */
/* of lines calculated from the target. This is only applicable for    */
/* PUT and NOT for PUTDEL.                                             */
/*---------------------------------------------------------------------*/
   resolve_current_and_focus_lines(CURRENT_VIEW,true_line,num_file_lines,direction,FALSE,FALSE);
  }
 free_target(&target);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_macro(CHARTYPE *params,bool error_on_not_found,short *macrorc)
#else
short execute_macro(params,error_on_not_found,macrorc)
CHARTYPE *params;
bool error_on_not_found;
short *macrorc;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 short errnum=0;
 FILE *fp=NULL;
#define MAC_PARAMS  3
 CHARTYPE *word[MAC_PARAMS+1];
 CHARTYPE strip[MAC_PARAMS];
 unsigned short num_params=0;
 CHARTYPE *macroname=NULL;
 bool save_in_macro=in_macro;
 bool allow_interactive=FALSE;
 CHARTYPE *tmpfilename=NULL;
 CHARTYPE *tmpargs=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_macro");
#endif
/*---------------------------------------------------------------------*/
/* Validate the parameters. At least 1 must be present, the filename.  */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_NONE;
 num_params = param_split(params,word,MAC_PARAMS-1,WORD_DELIMS,TEMP_PARAM,strip,TRUE);
 if (num_params == 0)
   {
    display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
/*---------------------------------------------------------------------*/
/* Check if first parameter is ?...                                    */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)word[0],"?") == 0)
 {
    strip[0]=STRIP_BOTH;
    strip[1]=STRIP_BOTH;
    strip[2]=STRIP_NONE;
    num_params = param_split(params,word,MAC_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,TRUE);
    if (num_params == 1)
    {
       display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
    }
    tmpfilename = word[1];
    tmpargs = word[2];
    allow_interactive = TRUE;
 }
 else
 {
    tmpfilename = word[0];
    tmpargs = word[1];
    allow_interactive = FALSE;
 }
/*---------------------------------------------------------------------*/
/* Allocate some space for macroname...                                */
/*---------------------------------------------------------------------*/
 if ((macroname = (CHARTYPE *)(*the_malloc)((MAX_FILE_NAME+1)*sizeof(CHARTYPE))) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
/*---------------------------------------------------------------------*/
/* Find the fully qualified file name for the supplied macro name.     */
/*---------------------------------------------------------------------*/
 rc = get_valid_macro_file_name(tmpfilename,macroname,macro_suffix,&errnum);
/*---------------------------------------------------------------------*/
/* Validate the return code...                                         */
/*---------------------------------------------------------------------*/
 switch(rc)
   {
/*---------------------------------------------------------------------*/
/* If RC_OK, continue to process the macro...                          */
/*---------------------------------------------------------------------*/
    case RC_OK:
         break;
/*---------------------------------------------------------------------*/
/* If RC_FILE_NOT_FOUND and IMPOS is not on, display an error and exit.*/
/* If IMPOS is on, just return without displaying an error.            */
/*---------------------------------------------------------------------*/
    case RC_FILE_NOT_FOUND:
         if (error_on_not_found)
            display_error(errnum,tmpfilename,FALSE);
         (*the_free)(macroname);
#ifdef THE_TRACE
         trace_return();
#endif
         return(rc);
         break;
/*---------------------------------------------------------------------*/
/* All other cases, display error and return.                          */
/*---------------------------------------------------------------------*/
    default:
         display_error(errnum,tmpfilename,FALSE);
         (*the_free)(macroname);
#ifdef THE_TRACE
         trace_return();
#endif
         return(rc);
   }
/*---------------------------------------------------------------------*/
/* Set in_macro = TRUE to stop multiple show_page()s being performed.  */
/*---------------------------------------------------------------------*/
 in_macro = TRUE;
/*---------------------------------------------------------------------*/
/* Save the values of the cursor position...                           */
/*---------------------------------------------------------------------*/
 get_cursor_position(&original_screen_line,&original_screen_column,
                     &original_file_line,&original_file_column);
/*---------------------------------------------------------------------*/
/* If REXX is supported, process the macro as a REXX macro...          */
/*---------------------------------------------------------------------*/
 if (rexx_support)
   {
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
    rc = execute_macro_file(macroname,tmpargs,macrorc,allow_interactive);
    if (rc != RC_OK)
      {
       display_error(54,(CHARTYPE *)"",FALSE);
       rc = RC_SYSTEM_ERROR;
      }
    (*the_free)(macroname);
   }
 else
   {
/*---------------------------------------------------------------------*/
/* ...otherwise, process the file as a non-REXX macro file...          */
/*---------------------------------------------------------------------*/
    if ((fp = fopen((DEFCHAR *)macroname,"r")) == NULL)
      {
       rc = RC_ACCESS_DENIED;
       display_error(8,macroname,FALSE);
      }
    post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
    if (rc == RC_OK)
      {
       rc = execute_command_file(fp);
       fclose(fp);
      }
    (*the_free)(macroname);
    if (rc == RC_SYSTEM_ERROR)
       display_error(53,(CHARTYPE *)"",FALSE);
    if (rc == RC_NOREXX_ERROR)
       display_error(52,(CHARTYPE *)"",FALSE);
   }
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
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_set_on_off(CHARTYPE *inparams,bool *flag)
#else
short execute_set_on_off(inparams,flag)
CHARTYPE *inparams;
bool *flag;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 int len=0;
 CHARTYPE *params=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_set_on_off");
#endif
/*---------------------------------------------------------------------*/
/* Strip the leading and trailing spaces from parameters...            */
/*---------------------------------------------------------------------*/
 if ((params = (CHARTYPE *)my_strdup(inparams)) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 params = MyStrip(params,STRIP_BOTH,' ');
/*---------------------------------------------------------------------*/
/* Validate the parameter. It must be ON or OFF.                       */
/*---------------------------------------------------------------------*/
 len = strlen((DEFCHAR *)params);

 if ((len < 2) 
 || (toupper(params[0]) != 'O'))
   {
    display_error(1,(CHARTYPE *)inparams,FALSE);
    rc = RC_INVALID_OPERAND;
   }
 else
   {
    if ((len == 3)
    && (toupper(params[1]) == 'F')
    && (toupper(params[2]) == 'F'))
       *flag = FALSE;
    else
      {
       if ((len == 2)
       && (toupper(params[1]) == 'N'))
          *flag = TRUE;
       else
         {
          display_error(1,(CHARTYPE *)inparams,FALSE);
          rc = RC_INVALID_OPERAND;
         }
      }
   }
 (*the_free)(params);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_set_row_position(CHARTYPE *inparams,short *base,short *off)
#else
short execute_set_row_position(inparams,base,off)
CHARTYPE *inparams;
short *base,*off;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 CHARTYPE *params=NULL,*save_param_ptr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_set_row_position");
#endif
/*---------------------------------------------------------------------*/
/* Strip the leading and trailing spaces from parameters...            */
/*---------------------------------------------------------------------*/
 if ((params = save_param_ptr = (CHARTYPE *)my_strdup(inparams)) == NULL)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 params = MyStrip(params,STRIP_BOTH,' ');
/*---------------------------------------------------------------------*/
/* Parse the position parameter...                                     */
/*---------------------------------------------------------------------*/
 if (*params == 'M'
 ||  *params == 'm')
   {
    *base = POSITION_MIDDLE;
    params++;
    if (blank_field(params))
       *off = 0;
    else
      {
       if ((*params != '-'
       &&  *params != '+')
       || ((*off = atoi((DEFCHAR *)params)) == 0))
         {
          display_error(1,inparams,FALSE);
          (*the_free)(save_param_ptr);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_INVALID_OPERAND);
         }
      }
   }
 else
   {
    if ((*off = atoi((DEFCHAR *)params)) == 0)
      {
       display_error(1,inparams,FALSE);
          (*the_free)(save_param_ptr);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_INVALID_OPERAND);
      }
    *base = (*off > 0) ? POSITION_TOP : POSITION_BOTTOM;
   }
 (*the_free)(save_param_ptr);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short processable_line(VIEW_DETAILS *view,LINETYPE true_line,LINE *curr)
#else
short processable_line(view,true_line,curr)
VIEW_DETAILS *view;
LINETYPE true_line;
LINE *curr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: processable_line");
#endif

#if 0
/* MH12 */
 if (VIEW_TOF(view,true_line)
 ||  VIEW_BOF(view,true_line))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(LINE_TOF_EOF);
   }
#endif
 if (VIEW_TOF(view,true_line))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(LINE_TOF);
   }
 if (VIEW_BOF(view,true_line))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(LINE_EOF);
   }

 if (view->scope_all
 ||  IN_SCOPE(view,curr))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(LINE_LINE);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(LINE_SHADOW);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_expand_compress(CHARTYPE *params,bool expand,bool inc_alt,bool use_tabs,bool add_to_recovery)
#else
short execute_expand_compress(params,expand,inc_alt,use_tabs,add_to_recovery)
CHARTYPE *params;
bool expand,inc_alt,use_tabs,add_to_recovery;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE i=0L,num_actual_lines=0L;
 LINETYPE num_lines=0L,true_line=0L,num_file_lines=0L;
 short direction=0,rc=RC_OK;
 LINE *curr=NULL;
 TARGET target;
 short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
 bool lines_based_on_scope=FALSE;
 bool adjust_alt=FALSE;
/*--------------------------- processing ------------------------------*/
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/* If no parameter is supplied, 1 is assumed.                          */
/*---------------------------------------------------------------------*/
 true_line = get_true_line(TRUE);
 if (strcmp("",(DEFCHAR *)params) == 0)
    params = (CHARTYPE *)"1";
 initialise_target(&target);
 if ((rc = validate_target(params,&target,target_type,true_line,TRUE,TRUE)) != RC_OK)
   {
    free_target(&target);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* If a BOX BLOCK target requested, return an error...                 */
/*---------------------------------------------------------------------*/
 if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
   {
    switch(MARK_VIEW->mark_type)
      {
       case M_BOX:
       case M_COLUMN:
       case M_WORD:
            display_error(48,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_OPERAND;
            break;
       case M_STREAM:
            display_error(49,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_OPERAND;
            break;
       default:
            break;
      }
    if (MARK_VIEW->mark_type != M_LINE)
      {
       free_target(&target);
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*---------------------------------------------------------------------*/
/* Determine in which direction we are working.                        */
/*---------------------------------------------------------------------*/
 if (target.num_lines < 0L)
   {
    direction = DIRECTION_BACKWARD;
    num_lines = -target.num_lines;
   }
 else
   {
    direction = DIRECTION_FORWARD;
    num_lines = target.num_lines;
   }
 true_line = target.true_line;
 lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
 free_target(&target);
/*---------------------------------------------------------------------*/
/* Find the current LINE pointer for the true_line.                    */
/* This is the first line to change.                                   */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* Convert all tabs in the current line to spaces.                     */
/*---------------------------------------------------------------------*/
 for (i=0L,num_actual_lines=0L;;i++)
   {
    if (lines_based_on_scope)
      {
       if (num_actual_lines == num_lines)
          break;
      }
    else
      {
       if (num_lines == i)
          break;
      }
    rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(i*direction),curr);
    switch(rc)
      {
       case LINE_SHADOW:
            break;
/*       case LINE_TOF_EOF: MH12 */
       case LINE_TOF:
       case LINE_EOF:
            num_actual_lines++;
            break;
       default:
            rc = tabs_convert(curr,expand,use_tabs,add_to_recovery);
            if (rc == RC_FILE_CHANGED)
               adjust_alt = TRUE;
            else
               if (rc != RC_OK)
                 {
#ifdef THE_TRACE
                  trace_return();
#endif
                  return(rc);
                 }
            num_actual_lines++;
      }
    if (direction == DIRECTION_FORWARD)
       curr = curr->next;
    else
       curr = curr->prev;
    num_file_lines += (LINETYPE)direction;
    if (curr == NULL)
       break;
   }
/*---------------------------------------------------------------------*/
/* Increment the number of alterations count if required...            */
/*---------------------------------------------------------------------*/
 if (inc_alt
 &&  adjust_alt)
    increment_alt(CURRENT_FILE);
/*---------------------------------------------------------------------*/
/* If STAY is OFF, change the current and focus lines by the number    */
/* of lines calculated from the target.                                */
/*---------------------------------------------------------------------*/
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 resolve_current_and_focus_lines(CURRENT_VIEW,true_line,num_file_lines,direction,TRUE,FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 if (CURRENT_TOF || CURRENT_BOF)
    return(RC_TOF_EOF_REACHED);
 else
    return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_select(CHARTYPE *params,bool relative,short off)
#else
short execute_select(params,relative,off)
CHARTYPE *params;
bool relative;
short off;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE i=0L,num_actual_lines=0L;
 LINETYPE num_lines=0L,true_line=0L;
 short direction=0,rc=RC_OK;
 LINE *curr=NULL;
 TARGET target;
 short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
 bool lines_based_on_scope=FALSE;
/*--------------------------- processing ------------------------------*/
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied.                    */
/* If no parameter is supplied, 1 is assumed.                          */
/*---------------------------------------------------------------------*/
 true_line = get_true_line(TRUE);
 initialise_target(&target);
 if ((rc = validate_target(params,&target,target_type,true_line,TRUE,TRUE)) != RC_OK)
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
    num_lines = -target.num_lines;
   }
 else
   {
    direction = DIRECTION_FORWARD;
    num_lines = target.num_lines;
   }
 true_line = target.true_line;
 lines_based_on_scope = (target.rt[0].target_type == TARGET_BLOCK_CURRENT) ? FALSE : TRUE;
/*---------------------------------------------------------------------*/
/* Find the current LINE pointer for the true_line.                    */
/* This is the first line to change.                                   */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* Convert all tabs in the current line to spaces.                     */
/*---------------------------------------------------------------------*/
 for (i=0L,num_actual_lines=0L;;i++)
   {
    if (lines_based_on_scope)
      {
       if (num_actual_lines == num_lines)
          break;
      }
    else
      {
       if (num_lines == i)
          break;
      }
    rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(i*direction),curr);
    switch(rc)
      {
       case LINE_SHADOW:
            break;
       case LINE_TOF:
       case LINE_EOF:
            num_actual_lines++;
            break;
       default:
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
            num_actual_lines++;
      }
    if (direction == DIRECTION_FORWARD)
       curr = curr->next;
    else
       curr = curr->prev;
    if (curr == NULL)
       break;
   }
 free_target(&target);
#ifdef THE_TRACE
 trace_return();
#endif
 if (CURRENT_TOF || CURRENT_BOF)
    return(RC_TOF_EOF_REACHED);
 else
    return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_move_cursor(LENGTHTYPE col)
#else
short execute_move_cursor(col)
LENGTHTYPE col;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short y=0,x=0;
 COLTYPE new_screen_col=0;
 LENGTHTYPE new_verify_col=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_move_cursor");
#endif
/*---------------------------------------------------------------------*/
/* Don't do anything for PREFIX window...                              */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OK);
   }

 getyx(CURRENT_WINDOW,y,x);
 calculate_new_column(x,CURRENT_VIEW->verify_col,col,&new_screen_col,&new_verify_col);
 if (CURRENT_VIEW->verify_col != new_verify_col
 &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    CURRENT_VIEW->verify_col = new_verify_col;
    build_screen(current_screen);
    display_screen(current_screen);
   }
 wmove(CURRENT_WINDOW,y,new_screen_col);

#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_find_command(CHARTYPE *str,short target_type)
#else
short execute_find_command(str,target_type)
CHARTYPE *str;
short target_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LENGTHTYPE save_zone_start=CURRENT_VIEW->zone_start;
 LENGTHTYPE save_zone_end=CURRENT_VIEW->zone_end;
 bool save_arbchar_status=CURRENT_VIEW->arbchar_status;
 bool save_hex=CURRENT_VIEW->hex;
 bool negative=FALSE,wrapped=FALSE;
 CHARTYPE save_arbchar_single=CURRENT_VIEW->arbchar_single;
 TARGET target;
 LINETYPE true_line=0L;
 LINETYPE save_focus_line=CURRENT_VIEW->focus_line;
 LINETYPE save_current_line=CURRENT_VIEW->current_line;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_find_command");
#endif
 if (strcmp((DEFCHAR *)str,"") == 0) /* argument supplied */
   {
    display_error(1,str,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 initialise_target(&target);
/*---------------------------------------------------------------------*/
/* Force the ZONE settings to start at 1 and end at parameter length.  */
/* Ensure ARBCHAR and HEX will be ignored in the search...             */
/*---------------------------------------------------------------------*/
 CURRENT_VIEW->zone_start = 1;
 CURRENT_VIEW->zone_end = strlen((DEFCHAR *)str);
 CURRENT_VIEW->arbchar_status = TRUE;
 CURRENT_VIEW->arbchar_single = find_unique_char(str);
 CURRENT_VIEW->hex = FALSE;
 rc = validate_target(str,&target,target_type,get_true_line(TRUE),TRUE,FALSE);
 if (rc == RC_TARGET_NOT_FOUND
 &&  CURRENT_VIEW->wrap)
   {
    wrapped = TRUE;
    negative = target.rt[0].negative;
    free_target(&target);
    initialise_target(&target);
    true_line = (negative ? CURRENT_FILE->number_lines+1 : 0L);
    CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line;
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
    rc = validate_target(str,&target,target_type,true_line,TRUE,FALSE);
   }
/*---------------------------------------------------------------------*/
/* Put ZONE, ARBCHAR and HEX back the way they were...                 */
/*---------------------------------------------------------------------*/
 CURRENT_VIEW->zone_start = save_zone_start;
 CURRENT_VIEW->zone_end = save_zone_end;
 CURRENT_VIEW->arbchar_status = save_arbchar_status;
 CURRENT_VIEW->arbchar_single = save_arbchar_single;
 CURRENT_VIEW->hex = save_hex;
/*---------------------------------------------------------------------*/
/* Check for target not found...                                       */
/*---------------------------------------------------------------------*/
 if (rc == RC_TARGET_NOT_FOUND)
   {
    if (wrapped)
      {
       CURRENT_VIEW->focus_line = save_focus_line;
       CURRENT_VIEW->current_line = save_current_line;
      }
    display_error(34,(CHARTYPE *)"",FALSE);
    free_target(&target);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* Target found, so advance the cursor...                              */
/*---------------------------------------------------------------------*/
/* post_process_line(CURRENT_VIEW,save_focus_line,(LINE *)NULL,TRUE);*/
 if (wrapped)
   {
    CURRENT_VIEW->focus_line = save_focus_line;
    CURRENT_VIEW->current_line = save_current_line;
    build_screen(current_screen);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND
    ||  compatible_feel == COMPAT_XEDIT)
       CURRENT_VIEW->current_line = true_line;
    else
       CURRENT_VIEW->focus_line = true_line;
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   }
 rc = advance_current_or_focus_line(target.num_lines);
 free_target(&target);
 if (wrapped)
    display_error(0,(CHARTYPE*)"Wrapped...",FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_modify_command(CHARTYPE *str)
#else
short execute_modify_command(str)
CHARTYPE *str;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short itemno=0;
 CHARTYPE item_type=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_modify_command");
#endif
 if ((itemno = find_query_item(str,strlen((DEFCHAR *)str),&item_type)) == (-1)
 || !(item_type & QUERY_MODIFY))
 {
    display_error(1,str,FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_INVALID_OPERAND);
 }

 itemno = get_item_values(1,itemno,(CHARTYPE *)"",QUERY_MODIFY,0L,NULL,0L);
 strcpy((DEFCHAR *)temp_cmd,"set");
 for (i=0;i<itemno+1;i++)
   {
    strcat((DEFCHAR *)temp_cmd," ");
    strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)item_values[i].value);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LENGTHTYPE calculate_rec_len(short action,LENGTHTYPE current_rec_len,LENGTHTYPE start_col,LINETYPE num_cols)
#else
LENGTHTYPE calculate_rec_len(action,current_rec_len,start_col,num_cols)
short action;
LENGTHTYPE current_rec_len,start_col;
LINETYPE num_cols;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LENGTHTYPE new_rec_len=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: calcualte_rec_len");
#endif
 switch(action)
   {
    case ADJUST_DELETE:
         if (num_cols > 0)
           {
            if (start_col < current_rec_len)
               new_rec_len = (LENGTHTYPE)(LINETYPE)current_rec_len -
                              (min((LINETYPE)current_rec_len - (LINETYPE)start_col,num_cols));
           }
         else
           {
            new_rec_len = current_rec_len;
           }
         break;
    case ADJUST_INSERT:
         new_rec_len = current_rec_len;
         break;
    case ADJUST_OVERWRITE:
         new_rec_len = current_rec_len;
         break;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(new_rec_len);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short set_editv(CHARTYPE *var,CHARTYPE *val,bool editv_file,bool rexx_var)
#else
static short set_editv(var,val,editv_file,rexx_var)
CHARTYPE *var;
CHARTYPE *val;
bool editv_file;
bool rexx_var;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LINE *curr=NULL;
 int len_var=0,len_val=0;
 CHARTYPE *value=NULL;
 LINE *first=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: set_editv");
#endif
 first = (editv_file)?CURRENT_FILE->editv:editv;
 if (rexx_var)
   {
    rc = get_rexx_variable(var,&value,&len_val);
    if (rc != RC_OK)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(rc);
      }
   }
 else
   {
    if (val == NULL)
       value = (CHARTYPE*)"";
    else
       value = val;
    len_val = strlen((DEFCHAR*)value);
   }
 var = make_upper(var);
 len_var = strlen((DEFCHAR*)var);
 curr = lll_locate(first,var);
 if (curr) /* found an existing variable */
   {
    if (len_val > curr->length)
      {
       curr->line = (CHARTYPE *)(*the_realloc)(curr->line,(len_val+1)*sizeof(CHARTYPE));
       if (curr->line == NULL)
         {
          if (rexx_var && value)
             (*the_free)(value);
          display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_OUT_OF_MEMORY);
         }
      }
    if (len_val == 0)  /* need to delete the entry */
      {
       if (editv_file)
          lll_del(&(CURRENT_FILE->editv),NULL,curr,DIRECTION_FORWARD);
       else
          lll_del(&editv,NULL,curr,DIRECTION_FORWARD);
      }
    else
       strcpy((DEFCHAR*)curr->line,(DEFCHAR*)value);
   }
 else
   {
    curr = lll_add(first,NULL,sizeof(LINE));
    if (curr == NULL)
      {
       if (rexx_var && value)
          (*the_free)(value);
       display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OUT_OF_MEMORY);
      }
/*---------------------------------------------------------------------*/
/* As the current variable is always inserted as the start of the LL   */
/* we must set the first pointer to the current variable each time.    */
/*---------------------------------------------------------------------*/
    if (editv_file)
       CURRENT_FILE->editv = curr;
    else
       editv = curr;
    curr->line = (CHARTYPE *)(*the_malloc)((len_val+1)*sizeof(CHARTYPE));
    if (curr->line == NULL)
      {
       if (rexx_var && value)
          (*the_free)(value);
       display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OUT_OF_MEMORY);
      }
    strcpy((DEFCHAR*)curr->line,(DEFCHAR*)value);
    curr->length = len_val;
    curr->name = (CHARTYPE *)(*the_malloc)((len_var+1)*sizeof(CHARTYPE));
    if (curr->name == NULL)
      {
       if (rexx_var && value)
          (*the_free)(value);
       display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_OUT_OF_MEMORY);
      }
    strcpy((DEFCHAR*)curr->name,(DEFCHAR*)var);
   }
 if (rexx_var && value)
    (*the_free)(value);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short execute_editv(short editv_type,bool editv_file,CHARTYPE *params)
#else
short execute_editv(editv_type,editv_file,params)
short editv_type;
bool editv_file;
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define EDITV_PARAMS  2
 CHARTYPE *word[EDITV_PARAMS+1];
 CHARTYPE strip[EDITV_PARAMS];
 CHARTYPE *p=NULL,*str=NULL;
 unsigned short num_params=0;
 LINE *curr=NULL,*first=NULL;
 int key=0,lineno=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("execute.c: execute_editv");
#endif
 first = (editv_file)?CURRENT_FILE->editv:editv;
 switch(editv_type)
   {
    case EDITV_SETL:
         strip[0] = STRIP_BOTH;
         strip[1] = STRIP_NONE;
         num_params = param_split(params,word,EDITV_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
         if (num_params == 0)
           {
            display_error(3,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
            trace_return();
#endif
            return(RC_INVALID_OPERAND);
           }
         rc = set_editv(word[0],word[1],editv_file,FALSE);
         break;
    case EDITV_SET:
         p = (CHARTYPE *)strtok((DEFCHAR *)params," ");
         while(p != NULL)
           {
/*            p = make_upper(p);*/
            str = (CHARTYPE *)strtok(NULL," ");
            rc = set_editv(p,str,editv_file,FALSE);
            if (str == NULL)
               break;
            p = (CHARTYPE *)strtok(NULL," ");
           }
         break;
    case EDITV_PUT:
         p = (CHARTYPE *)strtok((DEFCHAR *)params," ");
         while(p != NULL)
           {
/*            p = make_upper(p);*/
            rc = set_editv(p,NULL,editv_file,TRUE);
            p = (CHARTYPE *)strtok(NULL," ");
           }
         break;
    case EDITV_GET:
         p = (CHARTYPE *)strtok((DEFCHAR *)params," ");
         while(p != NULL)
           {
            p = make_upper(p);
            curr = lll_locate(first,p);
            if (curr
            &&  curr->line)
               str = curr->line;
            else
               str = (CHARTYPE *)"";
            if (set_rexx_variable(p,str,strlen((DEFCHAR*)str),-1) != RC_OK)
               break;
            p = (CHARTYPE *)strtok(NULL," ");
           }
         break;
    case EDITV_LIST:
         wclear(stdscr);
         if (blank_field(params))
           {
            curr = first;
            while (curr != NULL)
              {
               if (curr->line)
                  str = curr->line;
               else
                  str = (CHARTYPE *)"";
               attrset(A_BOLD);
               mvaddstr(lineno,0,(DEFCHAR *)curr->name);
               attrset(A_NORMAL);
               mvaddstr(lineno,1+strlen((DEFCHAR *)curr->name),(DEFCHAR *)str);
               lineno++;
               curr = curr->next;
              }
           }
         else
           {
            p = (CHARTYPE *)strtok((DEFCHAR *)params," ");
            while(p != NULL)
              {
/*               p = make_upper(p);*/
               curr = lll_locate(first,p);
               if (curr
               &&  curr->line)
                  str = curr->line;
               else
                  str = (CHARTYPE *)"";
               attrset(A_BOLD);
               mvaddstr(lineno,0,(DEFCHAR *)p);
               attrset(A_NORMAL);
               mvaddstr(lineno,1+strlen((DEFCHAR *)curr->name),(DEFCHAR *)str);
               lineno++;
               p = (CHARTYPE *)strtok(NULL," ");
              }
           }
         mvaddstr(terminal_lines-2,0,HIT_ANY_KEY);
         refresh();
         while(1)
           {
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
         break;
   }

#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short prepare_dialog( CHARTYPE *params, bool alert, CHARTYPE *stemname )
#else
short prepare_dialog( params, alert, stemname )
CHARTYPE *params;
bool alert;
CHARTYPE *stemname,
#endif
/***********************************************************************/
{
#define STATE_START        0
#define STATE_EDITFIELD    1
#define STATE_TITLE        2
#define STATE_BUTTON       3
#define STATE_DEFBUTTON    4
#define STATE_ICON         5
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   int len_params=strlen((DEFCHAR *)params);
   CHARTYPE delimiter;
   int len_prompt,len_title,len_initial,i,j,k,state=STATE_START;
   short button=BUTTON_OK;
   short default_button=1;
   CHARTYPE *prompt=NULL,*initial=NULL,*title=NULL;
   bool editfield=FALSE;
   bool found;
   int button_len[4];
   int button_num[4];
   DEFCHAR *button_text[4];
   int icon_len[4];
   DEFCHAR *icon_text[4];
   DEFCHAR *strdefbutton="defbutton";
   DEFCHAR *streditfield="editfield";
   DEFCHAR num[20];
   short icon;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("execute.c: prepare_dialog");
#endif
  /*
   * Must run from a Rexx macro...
   */
   if (!in_macro
   ||  !rexx_support)
   {
      display_error(53,(CHARTYPE *)"",FALSE);
  #ifdef THE_TRACE
      trace_return();
  #endif
      return(RC_INVALID_ENVIRON);
   }
   /*
    * Defining these constants this way is done because non-ansi compilers
    * can't handle defintions like int a[2] = {0,1};
    */
   button_len[0] = 2; button_text[0] = "ok";button_num[0] = 1;
   button_len[1] = 8; button_text[1] = "okcancel";button_num[1] = 2;
   button_len[2] = 5; button_text[2] = "yesno";button_num[2] = 2;
   button_len[3] = 11; button_text[3] = "yesnocancel";button_num[3] = 3;

   icon_len[0] = 15; icon_text[0] = "iconexclamation";
   icon_len[1] = 15; icon_text[1] = "iconinformation";
   icon_len[2] = 12; icon_text[2] = "iconquestion";
   icon_len[3] = 8;  icon_text[3] = "iconstop";
   /*
    * got to be something to display...
    */
   if (len_params < 2)
   {
      display_error(1,params,FALSE);
  #ifdef THE_TRACE
      trace_return();
  #endif
      return(RC_INVALID_OPERAND);
   }
   /*
    * Set the default icon and buttons in case these are not supplied
    */
   if ( alert )
   {
      icon = ICON_STOP;
      button = BUTTON_OK;
   }
   else
   {
      icon = ICON_NONE;
      button = BUTTON_OKCANCEL;
   }
   /*
    * The first character of the string is the delimiter for the prompt
    */
   delimiter = *(params);
   params++;                               /* throw away first delimiter */
   for (i=0;i<len_params;i++)
   {
      if (*(params+i) == delimiter)
         break;
   }
   /*
    * Did we get a delimiter ?
    */
   if (i == len_params) /* no, then use defaults */
   {
      prompt = (CHARTYPE *)(*the_malloc)(len_params*sizeof(CHARTYPE));
      if (prompt == NULL)
      {
         display_error(30,params,FALSE);
  #ifdef THE_TRACE
         trace_return();
  #endif
         return(RC_INVALID_OPERAND);
      }
      strcpy((DEFCHAR *)prompt,(DEFCHAR *)params);
   }
   else /* yes, we may have other options... */
   {
      len_prompt = i;
      prompt = (CHARTYPE *)(*the_malloc)(1+(len_prompt*sizeof(CHARTYPE)));
      if (prompt == NULL)
      {
         display_error(30,params,FALSE);
  #ifdef THE_TRACE
         trace_return();
  #endif
         return(RC_INVALID_OPERAND);
      }
      memcpy(prompt,params,len_prompt);
      prompt[len_prompt] = '\0';
      params += len_prompt+1;
      strtrunc(params);
      len_params = strlen((DEFCHAR *)params);
      for (i=0;i<len_params;)
      {
         if (len_params == 0)
            break;
         switch(state)
         {
            case STATE_START:
               if (params[i] == 'E' || params[i] == 'e')
               {
                  state = STATE_EDITFIELD;
                  break;
               }
               if (params[i] == 'T' || params[i] == 't')
               {
                  state = STATE_TITLE;
                  break;
               }
               if (params[i] == 'O' || params[i] == 'o' || params[i] == 'Y' || params[i] == 'y')
               {
                  state = STATE_BUTTON;
                  break;
               }
               if (params[i] == 'D' || params[i] == 'd')
               {
                  state = STATE_DEFBUTTON;
                  break;
               }
               if (params[i] == 'I' || params[i] == 'i')
               {
                  state = STATE_ICON;
                  break;
               }
               if (params[i] == ' ')
               {
                  i++;
                  break;
               }
               display_error(1,params,FALSE);
               rc = RC_INVALID_OPERAND;
               break;
            case STATE_EDITFIELD:
               if (len_params < 4)
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               for (i=0;i<len_params;i++)
               {
                  if (*(params+i) == ' ')
                     break;
                  if (isupper(*(params+i)))
                     *(params+i) = tolower(*(params+i));
                  if (*(params+i) != streditfield[i])
                  {
                     rc = RC_INVALID_OPERAND;
                     break;
                  }
               }
               if (rc == RC_INVALID_OPERAND)
               {
                  display_error(1,params,FALSE);
                  break;
               }
               params += i;
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
               editfield = TRUE;
               delimiter = *(params);
               if (delimiter == 'T' 
               ||  delimiter == 't'
               ||  delimiter == 'Y'
               ||  delimiter == 'y'
               ||  delimiter == 'O'
               ||  delimiter == 'o'
               ||  delimiter == 'D'
               ||  delimiter == 'd'
               ||  delimiter == 'I'
               ||  delimiter == 'i'
               ||  delimiter == '\0')
               {
                  state = STATE_START;
                  i = 0;
                  break;
               }
               params++;
               for (i=0;i<len_params;i++)
               {
                  if (*(params+i) == delimiter)
                     break;
               }
               if (i == len_params) /* no, then error */
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               len_initial = i;
               initial = (CHARTYPE *)(*the_malloc)(1+(len_initial*sizeof(CHARTYPE)));
               if (initial == NULL)
               {
                  display_error(30,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               memcpy(initial,params,len_initial);
               initial[len_initial] = '\0';
               params += (len_initial+1); /* add 1 for delimiter */
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
               state = STATE_START;
               i = 0;
               break;
            case STATE_TITLE:
               if (len_params < 6
               ||  memcmpi(params,(CHARTYPE *)"title ",6) != 0)
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               params += 6;
               strtrunc(params);
               delimiter = *(params);
               params++;
               len_params = strlen((DEFCHAR *)params);
               for (i=0;i<len_params;i++)
               {
                  if (*(params+i) == delimiter)
                     break;
               }
               if (i == len_params) /* no, then error */
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               len_title = i+2;  /* allow for leading and trailing space */
               title = (CHARTYPE *)(*the_malloc)(1+(len_title*sizeof(CHARTYPE)));
               if (title == NULL)
               {
                  display_error(30,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               memset(title,' ',len_title);
               memcpy(title+1,params,len_title-2);
               title[len_title] = '\0';
               params += (len_title-1); /* remove 2 spaces and add 1 for delimiter */
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
               i = 0;
               state = STATE_START;
               break;
            case STATE_ICON:
               if ( len_params >= 5
               &&  memcmpi( params, "icon", 4 ) == 0 )
               {
                  /*
                   * Find the first space, or end of string; set
                   * the space to nul, and see if the "string" matches
                   * one of the valid icon strings.
                   */
                  for ( j = 0; j < len_params; j++ )
                  {
                     if ( *params+j == ' ' )
                     {
                        *(params+j) = '\0';
                        break;
                     }
                  }
                  found = FALSE;
                  for (k=0;k<4;k++)
                  {
                     if ( equal( icon_text[k], params, 5 ) )
                     {
                        icon = k;
                        found = TRUE;
                        break;
                     }
                  }
                  /*
                   * Reset the space we changed
                   */
                  *(params+j) = ' ';
                  if ( found == FALSE )
                  {
                     display_error(1,params,FALSE);
                     rc = RC_INVALID_OPERAND;
                     break;
                  }
               }
               else
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               params += icon_len[icon];
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
               state = STATE_START;
               i = 0;
               break;
            case STATE_BUTTON:
               button = -1;
               for (i=0;i<4;i++)
               {
                  if (len_params >= button_len[i]
                  &&  memcmpi(params,(CHARTYPE *)button_text[i],button_len[i]) == 0
                  && (len_params == button_len[i] || *(params+button_len[i]) == ' '))
                  {
                     button = i;
                     break;
                  }
               }
               if (button == (-1))
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               params += button_len[i];
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
               state = STATE_START;
               i = 0;
               break;
            case STATE_DEFBUTTON:
               if (len_params < 6)
               {
                  display_error(1,params,FALSE);
                  rc = RC_INVALID_OPERAND;
                  break;
               }
               for (i=0;i<len_params;i++)
               {
                  if (*(params+i) == ' ')
                     break;
                  if (isupper(*(params+i)))
                     *(params+i) = tolower(*(params+i));
                  if (*(params+i) != strdefbutton[i])
                  {
                     rc = RC_INVALID_OPERAND;
                     break;
                  }
               }
               if (rc == RC_INVALID_OPERAND)
               {
                  display_error(1,params,FALSE);
                  break;
               }
               params += i;
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
               for (i=0;i<len_params;i++)
               {
                  if (*(params+i) == ' ')
                     break;
                  if (!isdigit(*(params+i)))
                  {
                     rc = RC_INVALID_OPERAND;
                     break;
                  }
                  num[i] = *(params+i);
               }
               if (rc == RC_INVALID_OPERAND)
               {
                  display_error(1,params,FALSE);
                  break;
               }
               num[i] = '\0';
               default_button = atoi(num);
               params += i;
               strtrunc(params);
               len_params = strlen((DEFCHAR *)params);
  
               state = STATE_START;
               i = 0;
               break;
            default:
               display_error(1,params,FALSE);
               rc = RC_INVALID_OPERAND;
               break;
         }
         if (rc != RC_OK)
            break;
      }
   }
   if (default_button < 1)
   {
      display_error(5,(CHARTYPE *)num,FALSE);
      rc = RC_INVALID_OPERAND;
   }
   if (default_button > button_num[button])
   {
      display_error(6,(CHARTYPE *)num,FALSE);
      rc = RC_INVALID_OPERAND;
   }
   if (rc == RC_OK)
   {
      rc = execute_dialog(prompt,title,initial,editfield,button,default_button-1,stemname,icon,alert);
   }
   if (prompt)
      (*the_free)(prompt);
   if (initial)
      (*the_free)(initial);
   if (title)
      (*the_free)(title);
#ifdef THE_TRACE
   trace_return();
#endif
   return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short execute_dialog(CHARTYPE *prompt, CHARTYPE *title, CHARTYPE *initial, bool editfield,short button, short default_button,CHARTYPE *stemname, short icon, bool alert)
#else
short execute_dialog(prompt, title, initial, editfield,button, default_button,stemname,icon,alert)
CHARTYPE *prompt;
CHARTYPE *title;
CHARTYPE *initial;
bool editfield;
short button;
short default_button;
CHARTYPE *stemname;
short icon;
bool alert;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   int button_length[4],key,num_buttons=0,i;
   short title_length=0,initial_length=0,max_width,cursor_pos=0;
   short prompt_length=0,prompt_max_length=0,prompt_lines=0;
   DEFCHAR *prompt_line[12];
   WINDOW *dw=NULL;
   WINDOW *save_command_window=NULL;
   CHARTYPE *save_cmd_rec=NULL;
   CHARTYPE *editfield_buf=NULL;
   unsigned short save_cmd_rec_len;
   short dw_lines,dw_cols,dw_y,dw_x;
   short editfield_col=-1;
   DEFCHAR *button_text[3];
   short button_len[3];
   short button_col[3];
   short max_button_len=0;
   short item_selected=-1;
   CHARTYPE button_buf[15];
   bool in_editfield;
   LINETYPE save_max_line_length=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("execute.c: execute_dialog");
#endif
   button_length[0] = 4;
   button_length[1] = 11;
   button_length[2] = 8;
   button_length[3] = 15;
   /*
    * Split the prompt up into multiple lines if applicable
    */
   if (prompt)
   {
      prompt_line[prompt_lines] = prompt;
      prompt_length = strlen((DEFCHAR *)prompt);
      for ( i = 0; i < prompt_length && prompt_lines < 9; i++ )
      {
         if ( prompt[i] == 10)
         {
            prompt_lines++;
            prompt[i] = '\0';
            if ( strlen( prompt_line[prompt_lines-1] ) > prompt_max_length )
               prompt_max_length = strlen( prompt_line[prompt_lines-1] );
            prompt_line[prompt_lines] = prompt+i+1;
         }
      }
      prompt_lines++;
      if ( strlen( prompt_line[prompt_lines-1] ) > prompt_max_length )
         prompt_max_length = strlen( prompt_line[prompt_lines-1] );
   }
   /*
    * work out dimensions of dialog box based on length of prompt, title,
    * buttons, initial value and width of screen
    */
   max_width = terminal_cols - 2;
   if (title) 
      title_length = strlen((DEFCHAR *)title);
   if (initial)
      initial_length = strlen((DEFCHAR *)initial);
   if (button == BUTTON_OK)
      max_button_len = 8;
   else if (button == BUTTON_YESNO)
      max_button_len = 16;
   else if (button == BUTTON_YESNOCANCEL)
      max_button_len = 25;
   else if (button == BUTTON_OKCANCEL)
      max_button_len = 18;
   dw_cols = max_button_len;
   if (prompt_max_length+2 > dw_cols)
      dw_cols = prompt_max_length+2;
   if (title_length+3 > dw_cols)
      dw_cols = title_length+2;
   if (initial_length+2 > dw_cols)
      dw_cols = initial_length+2;
   dw_cols += 3;
   if (dw_cols > max_width)
   {
      dw_cols = max_width;
   }
   dw_lines = 6 + prompt_lines + ((editfield) ? 2 : 0);
   dw_x = (terminal_cols - dw_cols) / 2;
   dw_y = (terminal_lines - dw_lines) / 2;
   switch(button)
   {
      case BUTTON_OK:
         num_buttons = 1;
         button_text[0] = " OK ";
         button_len[0] = 4;
         button_col[0] = (dw_cols - 4) / 2;
         break;
      case BUTTON_YESNO:
         num_buttons = 2;
         button_text[0] = " YES ";
         button_len[0] = 5;
         button_text[1] = " NO ";
         button_len[1] = 4;
         button_col[0] = (dw_cols / 4) - (button_len[0] / 2);
         button_col[1] = (dw_cols / 2) + ((dw_cols / 4) - (button_len[1] / 2));
         break;
      case BUTTON_OKCANCEL:
         num_buttons = 2;
         button_text[0] = " OK ";
         button_len[0] = 4;
         button_text[1] = " CANCEL ";
         button_len[1] = 8;
         button_col[0] = (dw_cols / 4) - (button_len[0] / 2);
         button_col[1] = (dw_cols / 2) + ((dw_cols / 4) - (button_len[1] / 2));
         break;
      case BUTTON_YESNOCANCEL:
         num_buttons = 3;
         button_text[0] = " YES ";
         button_len[0] = 5;
         button_text[1] = " NO ";
         button_len[1] = 4;
         button_text[2] = " CANCEL ";
         button_len[2] = 8;
         button_col[0] = (dw_cols / 6) - (button_len[0] / 2);
         button_col[1] = (dw_cols / 3) + ((dw_cols / 6) - (button_len[1] / 2));
         button_col[2] = (2 *(dw_cols / 3)) + ((dw_cols / 6) - (button_len[2] / 2));
         break;
   }
   /*
    * Create the dialog window
    */
   dw = newwin(dw_lines,dw_cols,dw_y,dw_x);
   if (dw == NULL)
   {
      THERefresh((CHARTYPE *)"");
      restore_THE();
      display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
      trace_return();
#endif
      return(RC_OUT_OF_MEMORY);
   }
   if (editfield)
   {
      editfield_buf = (CHARTYPE *)(*the_malloc)(dw_cols + 1);
      if ( editfield_buf == NULL )
      {
         delwin( dw );
         display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_OUT_OF_MEMORY);
      }
      if ( initial )
         strcpy( (DEFCHAR *)editfield_buf, (DEFCHAR *)initial );
      else
         strcpy( (DEFCHAR *)editfield_buf, "" );
      /*
       * Save the CMDLINE contents
       */
      save_cmd_rec_len = cmd_rec_len;
      save_cmd_rec = (CHARTYPE *)(*the_malloc)(save_cmd_rec_len+1);
      if ( save_cmd_rec == NULL )
      {
         delwin( dw );
         (*the_free)(editfield_buf);
         display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_OUT_OF_MEMORY);
      }
      memcpy( (DEFCHAR *)save_cmd_rec, (DEFCHAR *)cmd_rec, cmd_rec_len );
      save_cmd_rec[cmd_rec_len] = '\0';
      /*
       * Save the CMDLINE window and create a new one in our dialog window
       */
      save_command_window = CURRENT_WINDOW_COMMAND;
      CURRENT_WINDOW_COMMAND = derwin( dw, 1, dw_cols-4, 3+prompt_lines, 2 );
      if ( CURRENT_WINDOW_COMMAND == (WINDOW *)NULL)
      {
         delwin( CURRENT_WINDOW_COMMAND );
         CURRENT_WINDOW_COMMAND = save_command_window;
         delwin( dw );
         (*the_free)( save_cmd_rec );
         (*the_free)(editfield_buf);
         display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
         trace_return();
#endif
         return(RC_OUT_OF_MEMORY);
      }
      wattrset( CURRENT_WINDOW_COMMAND, set_colour( CURRENT_FILE->attr+ATTR_CMDLINE ) );
   }
#ifdef HAVE_WBKGD
   wbkgd( dw, set_colour( CURRENT_FILE->attr+( ( alert ) ? ATTR_ALERT : ATTR_DIALOG ) ) );
#else
   wattrset( dw, set_colour( CURRENT_FILE->attr+( ( alert ) ? ATTR_ALERT : ATTR_DIALOG ) ) );
   wmove(dw,0,0);
   wclrtobot(dw);
#endif
#if defined(HAVE_BOX)
   wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_DIVIDER));
   box(dw,0,0);
#endif
   wattrset( dw, set_colour( CURRENT_FILE->attr+( ( alert ) ? ATTR_ALERT : ATTR_DIALOG ) ) );
   /*
    * Add the prompt line(s) to the window
    */
   for ( i = 0; i < prompt_lines; i++)
   {
      wmove(dw,2+i,2);
      waddstr(dw,(DEFCHAR *)prompt_line[i]);
   }
   /*
    * Add the title to the window
    */
   if (title)
   {
      wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_DIVIDER));
      wmove(dw,0,1);
      waddstr(dw,(DEFCHAR *)title);
   }
   /*
    * Prepare the editfield if we have one
    */
   if (editfield)
   {
      /*
       * Now we have passed any checks that result in a premature return
       * change the contents of the cmd_rec
       */
      in_editfield = TRUE;
      draw_cursor(TRUE);
      save_max_line_length = max_line_length;
      max_line_length = dw_cols - 2;
      default_button = -1;
   }
   else
   {
      wmove(dw,dw_lines-3,cursor_pos);
      in_editfield = FALSE;
      draw_cursor(FALSE);
   }
   while(1)
   {
      /*
       * Draw the buttons...
       */
      for (i=0;i<num_buttons;i++)
      {
         if (default_button == i)
         {
            wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_CBLOCK));
            cursor_pos = button_col[i];
         }
         else
            wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_BLOCK));
         wmove(dw,dw_lines-3,button_col[i]);
         waddstr(dw,button_text[i]);
      }
      if (in_editfield)
      {
         /*
          * Go into the pseudo command line and process it until
          * an exit key is pressed. On exit make the first button active.
          */
         draw_cursor(TRUE);
         rc = readv_cmdline( editfield_buf, dw, editfield_col );
         memcpy( (DEFCHAR *)editfield_buf, (DEFCHAR *)cmd_rec, cmd_rec_len );
         editfield_buf[cmd_rec_len] = '\0';
         in_editfield = FALSE;
         draw_cursor(FALSE);
         default_button = 0;
         editfield_col = -1;
         if ( rc == RC_READV_TERM_MOUSE )
         {
            /*
             * we terminated the readv_cmdline() with the press of a
             * mouse button outside the CMDLINE. make it look like the
             * mouse was just clicked so we can check if the click happened
             * on a button
             */
            key = KEY_MOUSE;
         }
         else
         {
            continue;
         }
      }
      else
      {
         /*
          * Display the dialog window and pseudo command line
          * and get a key.
          */
         wrefresh(dw);
         if ( editfield )
         {
            wrefresh( CURRENT_WINDOW_COMMAND );
         }
         key = my_getch(stdscr);
      }
#if defined(XCURSES)
      /*
       * Ignore scrollbar "keys"
       */
      if (key == KEY_SF || key == KEY_SR)
         continue;
#endif
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
      if (key == KEY_MOUSE)
      {
         int b,ba,bm,y,x;
         if (get_mouse_info(&b,&ba,&bm) != RC_OK)
            continue;
         if (b != 1
         ||  ba == BUTTON_PRESSED)
            continue;
         wmouse_position(dw, &y, &x);
         if (y == -1
         &&  x == -1)
         {
            /*
             * Button 1 clicked or released outside of window; ignore it
             */
            continue;
         }
         /*
          * Check that the mouse is clicked on a button
          */
         if ( y == dw_lines-3 )
         {
            bool found=FALSE;
            for (i=0;i<num_buttons;i++)
            {
               if ( x >= button_col[i] 
               &&   x <= (button_col[i] + button_len[i]) )
               {
                  found = TRUE;
                  break;
               }
            }
            if (!found)
               continue;
         }
         else if ( y == dw_lines-5
         &&   editfield
         &&   x > 1
         &&   x < dw_cols-2 )
         {
            /*
             * Clicked somewhere on the editfield
             */
            in_editfield = TRUE;
            default_button = -1;
            editfield_col = x - 2;
            continue;
         }
         else
         {
            /*
             * Clicked somewhere other than the button line
             */
            continue;
         }
         /*
          * Got a valid button. Check if its a click or press
          */
         if (ba == BUTTON_CLICKED 
         ||  ba == BUTTON_RELEASED)
         {
            /*
             * Got a valid line. Redisplay it in highlighted mode.
             */
            item_selected = i;
            touchwin(dw);
            wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_CBLOCK));
            wmove(dw,dw_lines-3,button_col[i]);
            waddstr(dw,button_text[i]);
            wrefresh(dw);
            break;
         }
         continue;
      }
      else
#endif
      {
         if (key == 9)
         {
           if (++default_button == num_buttons)
           {
              if (editfield)
              {
                 in_editfield = TRUE;
              }
              else
                 default_button = 0;
           }
         }
         else if (key == 'q')
         {
            item_selected = default_button;
            break;
         }
         else if (key == 10 || key == 13)
         {
            item_selected = default_button;
            break;
         }
      }
   }
   delwin(dw);
   draw_cursor(TRUE);
   /*
    * Set DIALOG.2 to the button pressed
    */
   strcpy( (DEFCHAR *)button_buf, (DEFCHAR *)button_text[item_selected] );
   strtrunc(button_buf);
   set_rexx_variable( stemname, button_buf, strlen((DEFCHAR *)button_buf), 2 );
   /*
    * Set DIALOG.1 to value of editfield2
    */
   if (editfield)
   {
      set_rexx_variable( stemname, editfield_buf, strlen( (DEFCHAR *)editfield_buf ), 1 );
   }
   else
   {
      set_rexx_variable( stemname, "", 0, 1 );
   }
   /*
    * Set DIALOG.0 to 2
    */
   set_rexx_variable( stemname, (CHARTYPE *)"2", 1, 0 );
   /*
    * If we had an editfield, restore the CMDLINE window and contents
    */
   if ( editfield )
   {
      delwin( CURRENT_WINDOW_COMMAND );
      CURRENT_WINDOW_COMMAND = save_command_window;
      max_line_length = save_max_line_length;
      wmove(CURRENT_WINDOW_COMMAND,0,0);
      my_wclrtoeol(CURRENT_WINDOW_COMMAND);
      memset(cmd_rec,' ',max_line_length);
      cmd_rec_len = 0;
      wmove(CURRENT_WINDOW_COMMAND,0,0);
      CURRENT_VIEW->cmdline_col = -1;
      if ( save_cmd_rec[0] == '&' )
         Cmsg( save_cmd_rec );
      (*the_free)( save_cmd_rec );
      (*the_free)(editfield_buf);
   }
   THERefresh( (CHARTYPE *)"" );
   restore_THE();
#ifdef THE_TRACE
   trace_return();
#endif
   return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short execute_popup(int y, int x, int height, int width, int num_args, CHARTYPE **args)
#else
short execute_popup(y, x, height, width, num_args, args)
int y,x,height,width,num_args;
CHARTYPE **args;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   int key,i;
   WINDOW *dw=NULL;
   short item_selected=-1,highlighted_line=-1;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
   trace_function("execute.c: execute_popup");
#endif
   /*
    * Create the popup menu window
    */
   dw = newwin(height,width,y,x);
   if (dw == NULL)
   {
   }
#ifdef HAVE_WBKGD
   wbkgd(dw,set_colour(CURRENT_FILE->attr+ATTR_FILEAREA));
#else
   wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_FILEAREA));
   wmove(dw,0,0);
   wclrtobot(dw);
#endif
   draw_cursor(FALSE);
   while(1)
   {
#if defined(HAVE_BOX)
      wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_DIVIDER));
      box(dw,0,0);
#endif
      for (i=0;i<num_args;i++)
      {
         if ((args[i][0]) == '-')
         {
            wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_STATAREA));
            wmove(dw,i+1,1);
#ifdef HAVE_WHLINE
            whline(dw,0,width-2);
#else
            {
               int i;
               for (i=0;i<width-2;i++)
               {
                  waddch(dw,'-');
               }
            }
#endif
         }
         else
         {
            if (i == highlighted_line)
               wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_BLOCK));
            else
               wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_FILEAREA));
            wmove(dw,i+1,2);
            waddstr(dw,(DEFCHAR *)args[i]);
         }
      }
      touchwin(dw);
      wrefresh(dw);
      key = wgetch(stdscr);
#if defined(XCURSES)
      /*
       * Ignore scrollbar "keys"
       */
      if (key == KEY_SF || key == KEY_SR)
         continue;
#endif
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
      if (key == KEY_MOUSE)
      {
         int b,ba,bm,y,x;
         if (get_mouse_info(&b,&ba,&bm) != RC_OK)
         {
#ifdef THE_TRACE
           trace_return();
#endif
           return(RC_OK);
         }
         if (b != 1
         ||  ba == BUTTON_PRESSED)
            continue;
         wmouse_position(dw, &y, &x);
         if (y == -1
         &&  x == -1)
         {
            /*
             * Button 1 clicked or released outside of window; exit
             */
            break;
         }
         /*
          * Check that the mouse is clicked on a valid item
          */
         if (y > 0 && y < height && x > 0 && x < width)
         {
            i = y-1;
         }
         else
         {
            /*
             * Clicked on border
             */
            continue;
         }
         if ((args[i][0]) == '-')
         {
            /*
             * Clicked on a line
             */
            continue;
         }
         /*
          * Got a valid line. Check if its a click or press
          */
         if (ba == BUTTON_CLICKED 
         ||  ba == BUTTON_RELEASED)
         {
            /*
             * Got a valid line. Redisplay it in highlighted mode.
             */
            item_selected = i;
            wattrset(dw,set_colour(CURRENT_FILE->attr+ATTR_BLOCK));
            wmove(dw,i+1,2);
            waddstr(dw,(DEFCHAR *)args[i]);
            touchwin(dw);
            wrefresh(dw);
            break;
         }
         continue;
      }
      else
#endif
      {
         if (key == 9 || key == KEY_DOWN)
         {
            if (++highlighted_line == num_args)
               highlighted_line = 0;
            if (args[highlighted_line][0] == '-')
               highlighted_line++;
         }
         if (key == KEY_UP)
         {
            if (highlighted_line == -1)
               highlighted_line = num_args;
            if (--highlighted_line == -1)
               highlighted_line = num_args-1;
            if (args[highlighted_line][0] == '-')
               highlighted_line--;
         }
         else if (key == popup_escape_key || key == 'q')
            break;
         else if (key == 10 || key == 13)
         {
            item_selected = highlighted_line;
            break;
         }
      }
   }
   delwin(dw);
   draw_cursor(TRUE);
   if (item_selected == -1)
   {
      set_rexx_variable((CHARTYPE *)"POPUP","",0,1);
   }
   else
   {
      set_rexx_variable((CHARTYPE *)"POPUP",args[item_selected],strlen(args[item_selected]),1);
   }
   set_rexx_variable((CHARTYPE *)"POPUP",(CHARTYPE *)"1",1,0);

   THERefresh((CHARTYPE *)"");
   restore_THE();

#ifdef THE_TRACE
   trace_return();
#endif
   return(rc);
}
