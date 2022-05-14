/***********************************************************************/
/* PREFIX.C - Prefix commands.                                         */
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

static char RCSid[] = "$Id: prefix.c,v 1.2 1999/07/31 06:05:05 mark Exp mark $";

#include <the.h>
#include <proto.h>

/*-------------------------- declarations -----------------------------*/
#ifdef HAVE_PROTO
static short parse_prefix_command(THE_PPC *,CHARTYPE *,CHARTYPE *);
static short invalidate_prefix(THE_PPC *);
static short prefix_makecurr(THE_PPC *,short,LINETYPE);
static short prefix_add(THE_PPC *,short,LINETYPE);
static short prefix_duplicate(THE_PPC *,short,LINETYPE);
static short prefix_copy(THE_PPC *,short,LINETYPE);
static short prefix_move(THE_PPC *,short,LINETYPE);
static short prefix_delete(THE_PPC *,short,LINETYPE);
static short prefix_shift_left(THE_PPC *,short,LINETYPE);
static short prefix_shift_right(THE_PPC *,short,LINETYPE);
static short prefix_tabline(THE_PPC *,short,LINETYPE);
static short prefix_scale(THE_PPC *,short,LINETYPE);
static short prefix_show(THE_PPC *,short,LINETYPE);
static short prefix_exclude(THE_PPC *,short,LINETYPE);
static short prefix_block_duplicate(THE_PPC *,short,LINETYPE);
static short prefix_block_copy(THE_PPC *,short,LINETYPE);
static short prefix_block_move(THE_PPC *,short,LINETYPE);
static short prefix_block_delete(THE_PPC *,short,LINETYPE);
static short prefix_block_shift_left(THE_PPC *,short,LINETYPE);
static short prefix_block_shift_right(THE_PPC *,short,LINETYPE);
static short prefix_block_exclude(THE_PPC *,short,LINETYPE);
static THE_PPC *find_bottom_ppc(THE_PPC *,short);
static THE_PPC *find_target_ppc(void);
static THE_PPC *calculate_target_line(void);
static short try_rexx_prefix_macro(THE_PPC *);
static CHARTYPE *substr(CHARTYPE *, CHARTYPE *, short, short);
static short post_prefix_add(THE_PPC *,short,LINETYPE);
#else
static short parse_prefix_command();
static short invalidate_prefix();
static short prefix_makecurr();
static short prefix_add();
static short prefix_duplicate();
static short prefix_copy();
static short prefix_move();
static short prefix_delete();
static short prefix_shift_left();
static short prefix_shift_right();
static short prefix_tabline();
static short prefix_scale();
static short prefix_show();
static short prefix_exclude();
static short prefix_block_duplicate();
static short prefix_block_copy();
static short prefix_block_move();
static short prefix_block_delete();
static short prefix_block_shift_left();
static short prefix_block_shift_right();
static short prefix_block_exclude();
static THE_PPC *find_bottom_ppc();
static THE_PPC *find_target_ppc();
static THE_PPC *calculate_target_line();
static short try_rexx_prefix_macro();
static CHARTYPE *substr();
static short post_prefix_add();
#endif
/*---------------------------------------------------------------------*/
/* The following two are to specify the first and last items in the    */
/* linked list for prefix synonyms.                                    */
/*---------------------------------------------------------------------*/
LINE *first_prefix_synonym=NULL;
LINE *last_prefix_synonym=NULL;
/*---------------------------------------------------------------------*/

#define THE_PPC_NO_TARGET        (-1)
#define THE_PPC_NO_COMMAND       (-2)
#define THE_PPC_TARGET_PREVIOUS  0
#define THE_PPC_TARGET_FOLLOWING 1
/* the above two defines correspond to the position in the pc[] array  */
/* and should be changed if the position in pc[] array changes.        */

#define NUMBER_PREFIX_COMMANDS 22
 static PREFIX_COMMAND pc[NUMBER_PREFIX_COMMANDS] =
  {
   {(CHARTYPE *)"p",1,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,TRUE,FALSE,NULL,1L,FALSE,FALSE,0,NULL},
   {(CHARTYPE *)"f",1,FALSE,FALSE,FALSE,FALSE,FALSE,TRUE,FALSE,FALSE,NULL,1L,FALSE,FALSE,0,NULL},
   {(CHARTYPE *)"tabl",4,TRUE,FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,prefix_tabline,1L,FALSE,FALSE,0,NULL},
   {(CHARTYPE *)"scale",5,TRUE,FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,prefix_scale,1L,FALSE,FALSE,0,NULL},
   {(CHARTYPE *)"\"\"",2,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,prefix_block_duplicate,1L,FALSE,FALSE,40,NULL},
   {(CHARTYPE *)"cc",2,TRUE,TRUE,FALSE,TRUE,TRUE,FALSE,FALSE,FALSE,prefix_block_copy,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"mm",2,TRUE,TRUE,FALSE,TRUE,TRUE,FALSE,FALSE,FALSE,prefix_block_move,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"dd",2,TRUE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,FALSE,prefix_block_delete,1L,FALSE,FALSE,10,NULL},
   {(CHARTYPE *)"xx",2,TRUE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,TRUE,prefix_block_exclude,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"<<",2,TRUE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,FALSE,prefix_block_shift_left,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)">>",2,TRUE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,FALSE,prefix_block_shift_right,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"\"",1,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,prefix_duplicate,1L,FALSE,FALSE,40,NULL},
   {(CHARTYPE *)"c",1,TRUE,TRUE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,prefix_copy,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"m",1,TRUE,TRUE,TRUE,FALSE,TRUE,FALSE,FALSE,FALSE,prefix_move,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"d",1,TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,prefix_delete,1L,FALSE,FALSE,10,NULL},
   {(CHARTYPE *)"s",1,TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,TRUE,prefix_show,MAX_LONG,TRUE,TRUE,30,NULL},
   {(CHARTYPE *)"x",1,TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,TRUE,prefix_exclude,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"<",1,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,prefix_shift_left,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)">",1,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,FALSE,FALSE,prefix_shift_right,1L,FALSE,FALSE,30,NULL},
   {(CHARTYPE *)"i",1,TRUE,TRUE,FALSE,FALSE,FALSE,TRUE,FALSE,FALSE,prefix_add,1L,FALSE,FALSE,60,post_prefix_add},
   {(CHARTYPE *)"a",1,TRUE,TRUE,FALSE,FALSE,FALSE,TRUE,FALSE,FALSE,prefix_add,1L,FALSE,FALSE,60,post_prefix_add},
   {(CHARTYPE *)"/",1,TRUE,TRUE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,prefix_makecurr,1L,FALSE,FALSE,50,NULL},
  };

LINETYPE prefix_current_line;
bool in_prefix_macro=FALSE;     /* indicate if processing prefix macro */
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_prefix_commands(void)
#else
short execute_prefix_commands()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short cmd_idx=0,rc=RC_OK;
 CHARTYPE cmd[MAX_PREFIX_WIDTH+1];
 CHARTYPE mult[MAX_PREFIX_WIDTH+1];
 LINETYPE long_mult=0L;
 short top_ppc=0,bot_ppc=0,target_ppc=0;
 THE_PPC *curr_ppc=NULL;
 TARGET target;
 short target_type=TARGET_NORMAL;
 unsigned short y=0,x=0;
 int top_priority_idx=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  execute_prefix_commands");
#endif
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
/*---------------------------------------------------------------------*/
/* Validate all pending prefix commands...                             */
/*---------------------------------------------------------------------*/
 curr_ppc = CURRENT_FILE->first_ppc;
 while(1)
   {
    if (curr_ppc == NULL)
       break;
/*---------------------------------------------------------------------*/
/* If a set point command, validate the name.                          */
/*---------------------------------------------------------------------*/
    if (*(curr_ppc->ppc_command) == '.')
      {
       if (isalpha(curr_ppc->ppc_command[1]))
         {
          if (execute_set_point(curr_ppc->ppc_command,curr_ppc->ppc_line_number,TRUE) != RC_OK)
            {
             invalidate_prefix(curr_ppc);
             curr_ppc = curr_ppc->next;
             continue;
            }
          clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
          curr_ppc = curr_ppc->next;
          continue;
         }
       else
         {
          invalidate_prefix(curr_ppc);
          curr_ppc = curr_ppc->next;
          continue;
         }
      }
/*---------------------------------------------------------------------*/
/* If an invalid prefix command from last time, clear it.              */
/*---------------------------------------------------------------------*/
    if (*(curr_ppc->ppc_command) == '?')
      {
       clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
       curr_ppc = curr_ppc->next;
       continue;
      }
/*---------------------------------------------------------------------*/
/* To get here we have 'normal' prefix command.                        */
/*---------------------------------------------------------------------*/
    if (rexx_support)              /* if REXX support is in effect ... */
      {
/*---------------------------------------------------------------------*/
/* first determine if the command is a prefix macro BEFORE looking for */
/* standard prefix commands.                                           */
/*---------------------------------------------------------------------*/
       rc = try_rexx_prefix_macro(curr_ppc);
/*---------------------------------------------------------------------*/
/* If at this point there are no more files in the ring; we assume that*/
/* this was caused by exitting the last file in the ring from a prefix */
/* macro, exit and ignore any more prefix commands. This is messy !!!  */
/*---------------------------------------------------------------------*/
       if (number_of_files == 0)
         {
#ifdef THE_TRACE
          trace_return();
#endif
          return(RC_COMMAND_NO_FILES);
         }
       if (rc != RC_NOT_COMMAND)
         {
          curr_ppc = curr_ppc->next;
          continue;
         }
      }
/*---------------------------------------------------------------------*/
/* if no prefix macro found for the prefix command, check to see if it */
/* is a standard prefix command.                                       */
/*---------------------------------------------------------------------*/
    if ((cmd_idx = parse_prefix_command(curr_ppc,cmd,mult)) == THE_PPC_NO_TARGET)
      {
       invalidate_prefix(curr_ppc);
       curr_ppc = curr_ppc->next;
       continue;
      }
    if (cmd_idx == THE_PPC_NO_COMMAND)
      {
       curr_ppc = curr_ppc->next;
       continue;
      }
/*---------------------------------------------------------------------*/
/* If running in read-only mode and the function selected is not valid */
/* display an error.                                                   */
/*---------------------------------------------------------------------*/
    if (ISREADONLY(CURRENT_FILE)
    && !pc[cmd_idx].valid_in_readonly)
      {
       display_error(56,(CHARTYPE *)"",FALSE);
       invalidate_prefix(curr_ppc);
       continue;
      }
/*---------------------------------------------------------------------*/
/* We now have a recognised command. We have to validate its parameters*/
/* and find any associated pending commands.                           */
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/* Set the block_command flag for the current prefix command to the    */
/* appropriate value for the prefix command.                           */
/*---------------------------------------------------------------------*/
    curr_ppc->ppc_block_command = pc[cmd_idx].block_prefix_command;
/*---------------------------------------------------------------------*/
/* If the command does not allow parameters and there are parameters,  */
/* error.                                                              */
/*---------------------------------------------------------------------*/
    if (!pc[cmd_idx].multiples_allowed
    && strcmp((DEFCHAR *)mult,"") != 0)
      {
       invalidate_prefix(curr_ppc);
       curr_ppc = curr_ppc->next;
       continue;
      }
/*---------------------------------------------------------------------*/
/* If the command does allow parameters and there are no parameters,   */
/* set to default...                                                   */
/*---------------------------------------------------------------------*/
    rc = RC_OK;
    if (strcmp((DEFCHAR *)mult,"") == 0)
       long_mult = pc[cmd_idx].default_target;
    else
/*---------------------------------------------------------------------*/
/* ...otherwise validate the target supplied...                        */
/*---------------------------------------------------------------------*/
      {
       if (pc[cmd_idx].multiples_allowed)
         {
          if (pc[cmd_idx].full_target_allowed)
            {
             initialise_target(&target);
             target.ignore_scope = pc[cmd_idx].ignore_scope;
             if (pc[cmd_idx].use_last_not_in_scope        /* S command */
             && mult[0] == '-')
                rc = validate_target(mult,&target,target_type,find_last_not_in_scope(CURRENT_VIEW,NULL,curr_ppc->ppc_line_number,DIRECTION_FORWARD),TRUE,TRUE);
             else
                rc = validate_target(mult,&target,target_type,curr_ppc->ppc_line_number,TRUE,TRUE);
             if (target.num_lines == 0L)
                rc = RC_INVALID_OPERAND;
             long_mult = target.num_lines;
             free_target(&target);
            }
          else
             if (!valid_positive_integer(mult))
                rc = RC_INVALID_OPERAND;
             else
                long_mult = atol((DEFCHAR *)mult);
         }
      }
/*---------------------------------------------------------------------*/
/* If the target is invalid or not found, invalidate the command.      */
/*---------------------------------------------------------------------*/
    if (rc != RC_OK)
      {
       invalidate_prefix(curr_ppc);
       curr_ppc = curr_ppc->next;
       continue;
      }
    curr_ppc->ppc_cmd_param = long_mult;
    curr_ppc = curr_ppc->next;
   }
/*---------------------------------------------------------------------*/
/* For each valid pending  command for the current view, execute  the  */
/* command.                                                            */
/*---------------------------------------------------------------------*/
 curr_ppc = CURRENT_FILE->first_ppc;
 top_priority_idx=(-1);
 while(curr_ppc != NULL)
   {
    top_ppc = bot_ppc = target_ppc = (-1);
/*---------------------------------------------------------------------*/
/* Execute the function associated with the prefix command...          */
/*---------------------------------------------------------------------*/
    cmd_idx = curr_ppc->ppc_cmd_idx;
    long_mult = curr_ppc->ppc_cmd_param;
    if (cmd_idx != (-1)
    &&  cmd_idx != (-2)
    &&  pc[cmd_idx].function != NULL)
       rc = (*pc[cmd_idx].function)(curr_ppc,cmd_idx,long_mult);
    curr_ppc = curr_ppc->next;
/*---------------------------------------------------------------------*/
/* Determine the prefix command with the highest priority and save it. */
/*---------------------------------------------------------------------*/
    if (cmd_idx >= 0)
      {
       if (top_priority_idx == (-1))
          top_priority_idx = cmd_idx;
       else
         {
          if (pc[cmd_idx].priority > pc[top_priority_idx].priority)
             top_priority_idx = cmd_idx;
         }
      }
   }
/*---------------------------------------------------------------------*/
/* The "cleared" pending prefix commands now need to be deleted from   */
/* the linked list...                                                  */
/*---------------------------------------------------------------------*/
 curr_ppc = CURRENT_FILE->first_ppc;
 while(curr_ppc != NULL)
   {
    if (curr_ppc->ppc_cmd_idx == (-1))
       curr_ppc = delete_pending_prefix_command(curr_ppc,CURRENT_FILE,NULL);
    else
       curr_ppc = curr_ppc->next;
   }
/*---------------------------------------------------------------------*/
/* Now that we are here, display the new version of the screen.        */
/*---------------------------------------------------------------------*/
 if (top_priority_idx != (-1))
   {
    if (pc[top_priority_idx].post_function != NULL)
       rc = (*pc[top_priority_idx].post_function)(curr_ppc,cmd_idx,long_mult);
   }
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 build_screen(current_screen); 
 display_screen(current_screen);
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
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short parse_prefix_command(THE_PPC *curr_ppc,CHARTYPE *cmd,CHARTYPE *mult)
#else
static short parse_prefix_command(curr_ppc,cmd,mult)
THE_PPC *curr_ppc;
CHARTYPE *cmd,*mult;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0,j=0;
 short len_pcmd=0;
 CHARTYPE pcmd[MAX_PREFIX_WIDTH+1];
 short pos=0,rc=RC_OK;
 LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  parse_prefix_command");
#endif
/*---------------------------------------------------------------------*/
/* For each pending prefix command for the current view, execute the   */
/* appropriate command.                                                */
/*---------------------------------------------------------------------*/
 rc = THE_PPC_NO_TARGET;
 strcpy((DEFCHAR *)pcmd,(DEFCHAR *)curr_ppc->ppc_command); /* get our own copy to pull apart */
 if (blank_field(pcmd))          /* if prefix command is blank, return */
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(THE_PPC_NO_COMMAND);
   }
 len_pcmd = strlen((DEFCHAR *)pcmd);
/*---------------------------------------------------------------------*/
/* For each prefix synonym, determine if it exists somewhere in the    */
/* entered prefix command.                                             */
/*---------------------------------------------------------------------*/
 curr = first_prefix_synonym;
 while(1)
   {
    if (curr == NULL)
       break;
    pos = memfind(pcmd,curr->name,len_pcmd,strlen((DEFCHAR *)curr->name),TRUE,FALSE,' ',' ');
    if (pos == (-1))
      {
       curr = curr->next;
       continue;
      }
/*---------------------------------------------------------------------*/
/* Now that a match on synonym is made, determine the original prefix  */
/* command associated with that synonym...                             */
/*---------------------------------------------------------------------*/
    for (i=0;i<NUMBER_PREFIX_COMMANDS;i++)
      {
       if (strcmp((DEFCHAR *)pc[i].cmd,(DEFCHAR *)curr->line) == 0)
         {
          strcpy((DEFCHAR *)cmd,(DEFCHAR *)pc[i].cmd);
          for (j=0;j<strlen((DEFCHAR *)curr->name);j++)
             *(pcmd+pos+j) = ' ';
          strtrunc(pcmd);
          strcpy((DEFCHAR *)mult,(DEFCHAR *)pcmd);
          curr_ppc->ppc_cmd_idx = i;
#ifdef THE_TRACE
          trace_return();
#endif
          return(i);
         }
      }
/*---------------------------------------------------------------------*/
/* To get here we found a prefix synonym, but no matching original     */
/* command, so return an error.                                        */
/*---------------------------------------------------------------------*/
    curr_ppc->ppc_cmd_idx = (-1);
#ifdef THE_TRACE
    trace_return();
#endif
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* For each valid prefix command,  check to see if a valid command     */
/* exists somewhere in the entered prefix command.                     */
/*---------------------------------------------------------------------*/
 for (i=0;i<NUMBER_PREFIX_COMMANDS;i++)
   {
    pos = memfind(pcmd,pc[i].cmd,len_pcmd,pc[i].cmd_len,TRUE,FALSE,' ',' ');
    if (pos == (-1))
       continue;
    strcpy((DEFCHAR *)cmd,(DEFCHAR *)pc[i].cmd);
    for (j=0;j<pc[i].cmd_len;j++)
       *(pcmd+pos+j) = ' ';
    strtrunc(pcmd);
    strcpy((DEFCHAR *)mult,(DEFCHAR *)pcmd);
    rc = i;
/*---------------------------------------------------------------------*/
/* Set a flag in ppc[] array to indicate which command is present.     */
/*---------------------------------------------------------------------*/
    curr_ppc->ppc_cmd_idx = i;
    break;
   }
/*---------------------------------------------------------------------*/
/* If command not found, set a flag in ppc[] array to indicate command */
/* is invalid.                                                         */
/*---------------------------------------------------------------------*/
 if (rc == THE_PPC_NO_TARGET)
    curr_ppc->ppc_cmd_idx = (-1);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_makecurr(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_makecurr(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE top_line = curr_ppc->ppc_line_number;
/*--------------------------- processing ------------------------------*/
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 execute_makecurr(top_line);
 if (number_lines != 0L)
    CURRENT_VIEW->current_column = (LENGTHTYPE)number_lines;
 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_tabline(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_tabline(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE top_line = curr_ppc->ppc_line_number;
 short tab_row = 0;
/*--------------------------- processing ------------------------------*/
 if ((tab_row = get_row_for_focus_line(current_screen,top_line,(-1))) != (-1))
   {
    CURRENT_VIEW->tab_base = POSITION_TOP;
    CURRENT_VIEW->tab_off = tab_row+1;
    CURRENT_VIEW->tab_on = TRUE;
   }
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_scale(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_scale(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE top_line = curr_ppc->ppc_line_number;
 short scale_row = 0;
/*--------------------------- processing ------------------------------*/
 if ((scale_row = get_row_for_focus_line(current_screen,top_line,(-1))) != (-1))
   {
    CURRENT_VIEW->scale_base = POSITION_TOP;
    CURRENT_VIEW->scale_off = scale_row+1;
    CURRENT_VIEW->scale_on = TRUE;
   }
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_show(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_show(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register int i=0;
 short rc=RC_OK;
 LINETYPE top_line = curr_ppc->ppc_line_number;
 LINETYPE target_line=0L;
 LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (TOF(top_line)
 ||  BOF(top_line))
    return(-1);
/*---------------------------------------------------------------------*/
/* Find the current line from where we start showing...                */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,top_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* If the line that the show command was entered is in scope, then     */
/* return with an error.                                               */
/*---------------------------------------------------------------------*/
 if (IN_SCOPE(CURRENT_VIEW,curr))
   {
    invalidate_prefix(curr_ppc);
    return(-1);
   }
/*---------------------------------------------------------------------*/
/* For a negative target, show the lines from the end of the excluded  */
/* block.                                                              */
/*---------------------------------------------------------------------*/
 if (number_lines < 0)
   {
    target_line = find_last_not_in_scope(CURRENT_VIEW,curr,top_line,DIRECTION_FORWARD);
    curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,target_line,CURRENT_FILE->number_lines);
    for(i=0;i>number_lines;i--)
      {
       curr->select = CURRENT_VIEW->display_high;
       curr = curr->prev;
       if (curr->prev == NULL
       ||  IN_SCOPE(CURRENT_VIEW,curr))
          break;
      }
   }
 else
/*---------------------------------------------------------------------*/
/* For a positive target, show the lines from the start of the excluded*/
/* block.                                                              */
/*---------------------------------------------------------------------*/
   {
    for(i=0;i<number_lines;i++)
      {
       curr->select = CURRENT_VIEW->display_high;
       curr = curr->next;
       if (curr->next == NULL
       ||  IN_SCOPE(CURRENT_VIEW,curr))
          break;
      }
   }
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_exclude(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_exclude(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LINETYPE top_line = curr_ppc->ppc_line_number;
 LINE *curr=NULL;
 short direction=DIRECTION_FORWARD;
/*--------------------------- processing ------------------------------*/
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (TOF(top_line)
 ||  BOF(top_line))
    return(-1);
/*---------------------------------------------------------------------*/
/* If the high value of SET DISPLAY is 255, we can't exclude any lines */
/* so exit.                                                            */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->display_high == 255)
    return(rc);
 if (number_lines < 0)
    direction = DIRECTION_BACKWARD;
 else
    direction = DIRECTION_FORWARD;
/*---------------------------------------------------------------------*/
/* Find the current line from where we start excluding...              */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,top_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* For the number of lines affected, change the select level if the    */
/* line is in scope.                                                   */
/*---------------------------------------------------------------------*/
 while(number_lines != 0)
   {
    if (IN_SCOPE(CURRENT_VIEW,curr))
       curr->select = (short)CURRENT_VIEW->display_high + 1;
    if (direction == DIRECTION_FORWARD)
       curr = curr->next;
    else
       curr = curr->prev;
    number_lines -= direction;
   }
/*---------------------------------------------------------------------*/
/* Determine if current line is now not in scope...                    */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->current_line,CURRENT_FILE->number_lines);
 if (!IN_SCOPE(CURRENT_VIEW,curr))
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,curr,CURRENT_VIEW->current_line,DIRECTION_FORWARD);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_add(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_add(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line = curr_ppc->ppc_line_number;
/*--------------------------- processing ------------------------------*/
 if (top_line == CURRENT_FILE->number_lines+1)
    top_line--;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 rc = insert_new_line((CHARTYPE *)"",0,number_lines,
                     top_line,FALSE,FALSE,TRUE,CURRENT_VIEW->display_low,FALSE,FALSE);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_duplicate(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_occ)
#else
static short prefix_duplicate(curr_ppc,cmd_idx,number_occ)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_occ;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line = curr_ppc->ppc_line_number;
 LINETYPE lines_affected=0L;
/*--------------------------- processing ------------------------------*/
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (top_line != 0L
 &&  top_line != CURRENT_FILE->number_lines+1)
    rc = rearrange_line_blocks(COMMAND_DUPLICATE,SOURCE_PREFIX,top_line,
                              top_line,top_line,(short)number_occ,
                              CURRENT_VIEW,CURRENT_VIEW,TRUE,&lines_affected);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_copy(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_copy(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE bottom_line=0L,target_line=0L,lines_affected=0L;
 LINETYPE top_line = curr_ppc->ppc_line_number;
 THE_PPC *target_ppc=NULL;
 short rc=(-1);
/*--------------------------- processing ------------------------------*/
 if ((target_ppc = calculate_target_line()) == NULL)
    return(rc);
 target_line = target_ppc->ppc_line_number+target_ppc->ppc_cmd_param;
 bottom_line = top_line + number_lines - ((number_lines < 0L) ? (-1L) : 1L);
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (top_line != 0L
 &&  top_line != CURRENT_FILE->number_lines+1)
    rc = rearrange_line_blocks(COMMAND_COPY,SOURCE_PREFIX,top_line,bottom_line,
                              target_line,1,CURRENT_VIEW,CURRENT_VIEW,TRUE,
                              &lines_affected);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_move(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_move(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE bottom_line=0L,target_line=0L,lines_affected=0L;
 LINETYPE top_line = curr_ppc->ppc_line_number;
 THE_PPC *target_ppc=NULL;
 short rc=(-1);
/*--------------------------- processing ------------------------------*/
 if ((target_ppc = calculate_target_line()) == NULL)
    return(rc);
 target_line = target_ppc->ppc_line_number+target_ppc->ppc_cmd_param;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 bottom_line = top_line + number_lines - ((number_lines < 0L) ? (-1L) : 1L);

 if (top_line != 0L
 &&  top_line != CURRENT_FILE->number_lines+1)
   {
    if ((rc = rearrange_line_blocks(COMMAND_MOVE_COPY_SAME,SOURCE_PREFIX,
         top_line,bottom_line,target_line,1,CURRENT_VIEW,CURRENT_VIEW,TRUE,&lines_affected)) != RC_OK)
       return(rc);
    if (target_line < top_line)
      {
       top_line += number_lines;
       target_line += number_lines;
       bottom_line += number_lines;
      }
    rc = rearrange_line_blocks(COMMAND_MOVE_DELETE_SAME,SOURCE_PREFIX,top_line,
                              bottom_line,target_line,1,CURRENT_VIEW,CURRENT_VIEW,TRUE,
                              &lines_affected);
   }
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_delete(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_delete(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line = curr_ppc->ppc_line_number;
 LINETYPE bottom_line=0L,target_line=0L,lines_affected=0L;
/*--------------------------- processing ------------------------------*/

 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (!TOF(top_line)
 &&  !BOF(top_line))
   {
    bottom_line = top_line + number_lines - ((number_lines < 0L) ? (-1L) : 1L);
    target_line = (number_lines < 0L) ? (bottom_line) : (top_line);
    rc = rearrange_line_blocks(COMMAND_DELETE,SOURCE_PREFIX,top_line,bottom_line,
                              target_line,1,CURRENT_VIEW,CURRENT_VIEW,TRUE,
                              &lines_affected);
   }
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_shift_left(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_cols)
#else
static short prefix_shift_left(curr_ppc,cmd_idx,number_cols)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_cols;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE top_line = curr_ppc->ppc_line_number;
/*--------------------------- processing ------------------------------*/
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (top_line != 0L
 &&  top_line != CURRENT_FILE->number_lines+1)
   {
    execute_shift_command(TRUE,(short)number_cols,top_line,1L,TRUE,TARGET_UNFOUND,FALSE);
   }
 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_shift_right(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_cols)
#else
static short prefix_shift_right(curr_ppc,cmd_idx,number_cols)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_cols;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE start_line = curr_ppc->ppc_line_number;
/*--------------------------- processing ------------------------------*/
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 if (start_line != 0L
 &&  start_line != CURRENT_FILE->number_lines+1)
   {
    execute_shift_command(FALSE,(short)number_cols,start_line,1L,TRUE,TARGET_UNFOUND,FALSE);
   }
 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_duplicate(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_occ)
#else
static short prefix_block_duplicate(curr_ppc,cmd_idx,number_occ)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_occ;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line=0L,bottom_line=0L,lines_affected=0L;
 THE_PPC *bottom_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(rc);
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);
 rc = rearrange_line_blocks(COMMAND_DUPLICATE,SOURCE_PREFIX,top_line,bottom_line,
                           bottom_line,(short)number_occ,CURRENT_VIEW,CURRENT_VIEW,FALSE,
                           &lines_affected);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_copy(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_block_copy(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line=0L,bottom_line=0L,target_line=0L,lines_affected=0L;
 THE_PPC *bottom_ppc=NULL;
 THE_PPC *target_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(rc);
 if ((target_ppc = calculate_target_line()) == NULL)
    return(rc);
 target_line = target_ppc->ppc_line_number+target_ppc->ppc_cmd_param;
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);
 rc = rearrange_line_blocks(COMMAND_COPY,SOURCE_PREFIX,top_line,bottom_line,
                           target_line,1,CURRENT_VIEW,CURRENT_VIEW,FALSE,
                           &lines_affected);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_move(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_block_move(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line=0L,bottom_line=0L,target_line=0L,num_lines=0L,lines_affected=0L;
 THE_PPC *bottom_ppc=NULL;
 THE_PPC *target_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(rc);
 if ((target_ppc = calculate_target_line()) == NULL)
    return(rc);
 target_line = target_ppc->ppc_line_number+target_ppc->ppc_cmd_param;
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);
 if ((rc = rearrange_line_blocks(COMMAND_MOVE_COPY_SAME,SOURCE_PREFIX,
           top_line,bottom_line,target_line,1,CURRENT_VIEW,CURRENT_VIEW,FALSE,&lines_affected)) != RC_OK)
    return(rc);
 if (target_line < top_line)
   {
    num_lines = bottom_line - top_line + 1L;
    top_line += num_lines;
    bottom_line += num_lines;
    target_line += num_lines;
   }
 rc = rearrange_line_blocks(COMMAND_MOVE_DELETE_SAME,SOURCE_PREFIX,top_line,bottom_line,
                           target_line,1,CURRENT_VIEW,CURRENT_VIEW,FALSE,
                           &lines_affected);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_delete(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_block_delete(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=(-1);
 LINETYPE top_line=0L,bottom_line=0L,lines_affected=0L;
 THE_PPC *bottom_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(rc);
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);
 rc = rearrange_line_blocks(COMMAND_DELETE,SOURCE_PREFIX,top_line,bottom_line,
                           bottom_line,1,CURRENT_VIEW,CURRENT_VIEW,FALSE,
                           &lines_affected);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_shift_left(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_cols)
#else
static short prefix_block_shift_left(curr_ppc,cmd_idx,number_cols)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_cols;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE top_line=0L,bottom_line=0L;
 THE_PPC *bottom_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(-1);
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);

 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);

 execute_shift_command(TRUE,(short)number_cols,top_line,bottom_line-top_line+1L,FALSE,TARGET_UNFOUND,FALSE);

 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_shift_right(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_cols)
#else
static short prefix_block_shift_right(curr_ppc,cmd_idx,number_cols)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_cols;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE top_line=0L,bottom_line=0L;
 THE_PPC *bottom_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(-1);
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);

 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);

 execute_shift_command(FALSE,(short)number_cols,top_line,bottom_line-top_line+1L,FALSE,TARGET_UNFOUND,FALSE);

 return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short prefix_block_exclude(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short prefix_block_exclude(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LINETYPE top_line=0L,bottom_line=0L,num_lines=0L,i=0L;
 LINE *curr=NULL;
 THE_PPC *bottom_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((bottom_ppc = find_bottom_ppc(curr_ppc,cmd_idx)) == NULL)
    return(-1);
 top_line = min(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 bottom_line = max(curr_ppc->ppc_line_number,bottom_ppc->ppc_line_number);
 top_line = (top_line == 0L) ? 1L : top_line;
 bottom_line = (bottom_line == CURRENT_FILE->number_lines+1L) ? bottom_line-1L : bottom_line;
 num_lines = bottom_line-top_line+1L;
 clear_pending_prefix_command(curr_ppc,(LINE *)NULL);
 clear_pending_prefix_command(bottom_ppc,(LINE *)NULL);
/*---------------------------------------------------------------------*/
/* If the high value of SET DISPLAY is 255, we can't exclude any lines */
/* so exit.                                                            */
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->display_high == 255)
    return(rc);
/*---------------------------------------------------------------------*/
/* Find the current line from where we start excluding...              */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,top_line,CURRENT_FILE->number_lines);
/*---------------------------------------------------------------------*/
/* For the number of lines affected, change the select level if the    */
/* line is in scope.                                                   */
/*---------------------------------------------------------------------*/
 for (i=0;i<num_lines;i++)
   {
    if (IN_SCOPE(CURRENT_VIEW,curr))
       curr->select = (short)CURRENT_VIEW->display_high + 1;
    curr = curr->next;
   }
/*---------------------------------------------------------------------*/
/* Determine if current line is now not in scope...                    */
/*---------------------------------------------------------------------*/
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->current_line,CURRENT_FILE->number_lines);
 if (!IN_SCOPE(CURRENT_VIEW,curr))
    CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,curr,CURRENT_VIEW->current_line,DIRECTION_FORWARD);
 return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short invalidate_prefix(THE_PPC *curr_ppc)
#else
static short invalidate_prefix(curr_ppc)
THE_PPC *curr_ppc;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short len=0;
/*--------------------------- processing ------------------------------*/
 if (*(curr_ppc->ppc_command) != '?')
   {
    len = strlen((DEFCHAR *)curr_ppc->ppc_command);
    meminschr(curr_ppc->ppc_command,'?',0,CURRENT_VIEW->prefix_width,len);
    *(curr_ppc->ppc_command+len+1) = '\0';
   }
/*---------------------------------------------------------------------*/
/* ensure that there is no chance that a 'real' prefix command can be  */
/* executed.                                                           */
/*---------------------------------------------------------------------*/
 curr_ppc->ppc_cmd_idx = (-2);
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void clear_pending_prefix_command(THE_PPC *curr_ppc,LINE *curr_line)
#else
void clear_pending_prefix_command(curr_ppc,curr_line)
THE_PPC *curr_ppc;
LINE *curr_line;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=curr_line;
/*--------------------------- processing ------------------------------*/
/*---------------------------------------------------------------------*/
/* If curr_ppc == NULL, then do nothing.                               */
/*---------------------------------------------------------------------*/
 if (curr_ppc == NULL)
    return;
 if (curr == (LINE *)NULL)
    curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,curr_ppc->ppc_line_number,CURRENT_FILE->number_lines);
 curr->pre = NULL;
 curr_ppc->ppc_cmd_idx = (-1);
 curr_ppc->ppc_block_command = FALSE;
 curr_ppc->ppc_shadow_line = FALSE;
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
THE_PPC *delete_pending_prefix_command(THE_PPC *curr_ppc,FILE_DETAILS *curr_file,LINE *curr_line)
#else
THE_PPC *delete_pending_prefix_command(curr_ppc,curr_file,curr_line)
THE_PPC *curr_ppc;
FILE_DETAILS *curr_file;
LINE *curr_line;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=curr_line;
 THE_PPC *return_ppc=NULL;
/*--------------------------- processing ------------------------------*/
/*---------------------------------------------------------------------*/
/* If curr_ppc == NULL, then do nothing.                               */
/*---------------------------------------------------------------------*/
 if (curr_ppc == NULL)
    return(NULL);
 if (curr == (LINE *)NULL)
    curr = lll_find(curr_file->first_line,curr_file->last_line,curr_ppc->ppc_line_number,curr_file->number_lines);
 curr->pre = NULL;
 return_ppc = pll_del(&(curr_file->first_ppc),&(curr_file->last_ppc),curr_ppc,DIRECTION_FORWARD);
 return(return_ppc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static THE_PPC *find_bottom_ppc(THE_PPC *curr_ppc,short top_cmd_idx)
#else
static THE_PPC *find_bottom_ppc(curr_ppc,top_cmd_idx)
THE_PPC *curr_ppc;
short top_cmd_idx;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *bottom_ppc=curr_ppc;
/*--------------------------- processing ------------------------------*/
 while(bottom_ppc != NULL)
   {
    if (bottom_ppc != curr_ppc)
      {
       if (top_cmd_idx == bottom_ppc->ppc_cmd_idx)
          return(bottom_ppc);
      }
    bottom_ppc = bottom_ppc->next;
   }
 return(NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static THE_PPC *find_target_ppc(void)
#else
static THE_PPC *find_target_ppc()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *target_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 target_ppc = CURRENT_FILE->first_ppc;
 while(target_ppc != NULL)
   {
    if (target_ppc->ppc_cmd_idx == THE_PPC_TARGET_PREVIOUS
    ||  target_ppc->ppc_cmd_idx == THE_PPC_TARGET_FOLLOWING)
       return(target_ppc);
    target_ppc = target_ppc->next;
   }
 return(NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static THE_PPC *calculate_target_line(void)
#else
static THE_PPC *calculate_target_line()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 THE_PPC *target_ppc=NULL;
/*--------------------------- processing ------------------------------*/
 if ((target_ppc = find_target_ppc()) == NULL)
    return(NULL);
 target_ppc->ppc_cmd_param = 0L;
/*---------------------------------------------------------------------*/
/* If the target line is NOT top of file line and the target type is   */
/* PREVIOUS, subtract 1 from the target line.                          */
/*---------------------------------------------------------------------*/
 if (target_ppc->ppc_cmd_idx == THE_PPC_TARGET_PREVIOUS
 && !TOF(target_ppc->ppc_line_number))
    target_ppc->ppc_cmd_param = (-1);
/*---------------------------------------------------------------------*/
/* If the target line is the bottom of file and the target type is     */
/* FOLLOWING, sutract 1 from the target line.                          */
/*---------------------------------------------------------------------*/
 if (target_ppc->ppc_cmd_idx == THE_PPC_TARGET_FOLLOWING
 && BOF(target_ppc->ppc_line_number))
    target_ppc->ppc_cmd_param = (-1);
 clear_pending_prefix_command(target_ppc,(LINE *)NULL);
 return(target_ppc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short try_rexx_prefix_macro(THE_PPC *curr_ppc)
#else
static short try_rexx_prefix_macro(curr_ppc)
THE_PPC *curr_ppc;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short ctr=0, parm_length=0, pmacro_rc=0, errnum = 0 ;
 bool found_left=FALSE, found_right=FALSE;
 LINETYPE line_number=0L;
 CHARTYPE pm_parms[(MAX_PREFIX_WIDTH*4)+1], t_area[MAX_PREFIX_WIDTH+1],
      parms[MAX_PREFIX_WIDTH+1], tmpstr[MAX_PREFIX_WIDTH+1],
      orig_cmd[MAX_PREFIX_WIDTH+1];
 short macrorc=0;
/*--------------------------- processing ------------------------------*/
 strcpy((DEFCHAR *)orig_cmd,(DEFCHAR *)curr_ppc->ppc_command);

 found_right = FALSE;
 found_left = FALSE;
/*---------------------------------------------------------------------*/
/* Scan 'orig_cmd' from right to left looking for an embedded macro    */
/* file name.                                                          */
/*---------------------------------------------------------------------*/
 for (ctr = strlen((DEFCHAR *)orig_cmd); ctr >= 0 && !found_right; ctr--)
   {
    substr(&tmpstr[0], orig_cmd, 1, ctr);
    strcpy((DEFCHAR *)t_area,(DEFCHAR *)tmpstr);
/*---------------------------------------------------------------------*/
/* First check if the prefix command is a synonym.                     */
/*---------------------------------------------------------------------*/
/* check for prefix synonym, if so use the macro name not the synonym */
    get_valid_macro_file_name(find_prefix_synonym(t_area),temp_cmd,macro_suffix,&errnum);

    if (errnum == 0)
       found_right = TRUE;
   }

/*---------------------------------------------------------------------*/
/* No valid macro file name found ?                                    */
/*                                                                     */
/* Scan 'temp_cmd' from left to right looking for an embedded macro    */
/* file name.                                                          */
/*---------------------------------------------------------------------*/
 if (!found_right)
   for ( ctr = 0; ctr <= strlen((DEFCHAR *)orig_cmd) && !found_left; ctr++)
     {
      substr(&tmpstr[0], orig_cmd, ctr + 1, strlen((DEFCHAR *)temp_cmd) - ctr);
      strcpy((DEFCHAR *)t_area,(DEFCHAR *)tmpstr);
/*---------------------------------------------------------------------*/
/* First check if the prefix command is a synonym.                     */
/*---------------------------------------------------------------------*/
/* check for prefix synonym, if so use the macro name not the synonym */
      get_valid_macro_file_name(find_prefix_synonym(t_area),temp_cmd,macro_suffix,&errnum);

      if (errnum == 0)
         found_left = TRUE;
     }
/*---------------------------------------------------------------------*/
/* We have a prefix macro...                                           */
/*---------------------------------------------------------------------*/
 if (found_right || found_left)
   {
    parm_length = strlen((DEFCHAR *)orig_cmd) - strlen((DEFCHAR *)t_area);

    if (found_right)
       substr(&tmpstr[0],orig_cmd,strlen((DEFCHAR *)t_area) + 1,parm_length);
    else
       substr(&tmpstr[0],orig_cmd,1,parm_length);

    strcpy((DEFCHAR *)parms,(DEFCHAR *)tmpstr);
    line_number = curr_ppc->ppc_line_number;
/*---------------------------------------------------------------------*/
/* If the prefix command was entered on a shadow line, pass this to    */
/* the prefix macro...                                                 */
/*---------------------------------------------------------------------*/
    if (curr_ppc->ppc_shadow_line)
       sprintf((DEFCHAR *)pm_parms," PREFIX %s SHADOW %ld %s",t_area,line_number,parms);
    else
       sprintf((DEFCHAR *)pm_parms," PREFIX %s SET %ld %s",t_area,line_number,parms);
    strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)pm_parms);     /* add on the parameter list */
    prefix_current_line = line_number;
    in_prefix_macro = TRUE;
/*---------------------------------------------------------------------*/
/* Go and execute the prefix macro.                                    */
/*---------------------------------------------------------------------*/
    pmacro_rc = execute_macro(temp_cmd,TRUE,&macrorc);
    in_prefix_macro = FALSE;
   }
 else
    pmacro_rc = RC_NOT_COMMAND;
 return(pmacro_rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static CHARTYPE *substr(CHARTYPE *temp_string,CHARTYPE *string,short start,short length)
#else
static CHARTYPE *substr(temp_string, string, start, length)
CHARTYPE *temp_string, *string;
short start, length;
#endif
/***********************************************************************/
/* This function is like the REXX SUBSTR function, it returns a        */
/* pointer to a string containing the result.                          */
/* Note: The address of the beginning of a blank string is passed to be*/
/* modified by address: substr(&tmpstr[0],<string>,<start>,<length>);  */
/* ( In this case, the result is contained in 'tmpstr'  :-)            */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register int i=0, target_idx = 0;
/*--------------------------- processing ------------------------------*/
 for (i = start - 1; target_idx <= length - 1; i++)
    temp_string[target_idx++] = string[i];
 temp_string[target_idx] = '\0';   /*MH*/
 return(temp_string);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void add_prefix_command(LINE *curr,LINETYPE line_number,bool block_command)
#else
void add_prefix_command(curr,line_number,block_command)
LINE *curr;
LINETYPE line_number;
bool block_command;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 CHARTYPE temp_prefix_array[MAX_PREFIX_WIDTH+1];
 THE_PPC *curr_ppc=NULL;
 bool redisplay_screen=FALSE;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  add_prefix_command");
#endif
 prefix_changed = FALSE;
/*---------------------------------------------------------------------*/
/* Copy the contexts of the prefix record area into a temporary area.  */
/*---------------------------------------------------------------------*/
 for (i=0;i<pre_rec_len;i++)
     temp_prefix_array[i] = pre_rec[i];
 temp_prefix_array[pre_rec_len] = '\0';
 strtrunc(temp_prefix_array);
/*---------------------------------------------------------------------*/
/* If the prefix record area is blank, clear the pending prefix area.  */
/*---------------------------------------------------------------------*/
 if (blank_field(temp_prefix_array))
   {
    (void)delete_pending_prefix_command(curr->pre,CURRENT_FILE,curr);
    redisplay_screen = TRUE;
   }
 else
   {
/*---------------------------------------------------------------------*/
/* If the input line already points to an entry in the array, use the  */
/* existing entry in the array, otherwise add to the next entry.       */
/*---------------------------------------------------------------------*/
    curr_ppc = pll_find(CURRENT_FILE->first_ppc,line_number);
    if (curr_ppc == NULL)  /* not found */
      {
       CURRENT_FILE->last_ppc = pll_add(CURRENT_FILE->first_ppc,CURRENT_FILE->last_ppc,sizeof(THE_PPC));
       if (CURRENT_FILE->last_ppc == NULL)
         {
          display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
          trace_return();
#endif
          return;
         }
       if (CURRENT_FILE->first_ppc == NULL)
          CURRENT_FILE->first_ppc = CURRENT_FILE->last_ppc;
       curr_ppc = CURRENT_FILE->last_ppc;
      }
    curr->pre = curr_ppc;
    strcpy((DEFCHAR *)curr_ppc->ppc_command,(DEFCHAR *)temp_prefix_array);
    curr_ppc->ppc_line_number = line_number;
    curr_ppc->ppc_block_command = block_command;
    curr_ppc->ppc_cmd_idx = (-1);
    if (IN_SCOPE(CURRENT_VIEW,curr))
       curr_ppc->ppc_shadow_line = FALSE;
    else
       curr_ppc->ppc_shadow_line = TRUE;
   }
/*---------------------------------------------------------------------*/
/* Clear the pending prefix command line.                              */
/* This removed by MH 20/01/96 to ensure end() in PREFIX works         */
/*---------------------------------------------------------------------*/
#if 0
 memset(pre_rec,' ',MAX_PREFIX_WIDTH);
 pre_rec_len = 0;
 pre_rec[MAX_PREFIX_WIDTH] = '\0';
#endif

 if (redisplay_screen)
   {
    build_screen(current_screen);
    display_screen(current_screen);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short add_prefix_synonym(CHARTYPE *synonym,CHARTYPE *macroname)
#else
short add_prefix_synonym(synonym,macroname)
CHARTYPE *synonym,*macroname;
#endif
/***********************************************************************/
/* Parameters:                                                         */
/*    synonym: synonym for prefix macro                                */
/*  macroname: name of REXX macro file                                 */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  add_prefix_synonym");
#endif
/*---------------------------------------------------------------------*/
/* First thing is to delete any definitions that may exist for the     */
/* supplied synonym.                                                   */
/*---------------------------------------------------------------------*/
 curr = first_prefix_synonym;
 while(curr != NULL)
   {
    if (strcmp((DEFCHAR *)curr->name,(DEFCHAR *)synonym) == 0)
      {
       if (curr->name != NULL)
          (*the_free)(curr->name);
       if (curr->line != NULL)
          (*the_free)(curr->line);
       curr = lll_del(&first_prefix_synonym,&last_prefix_synonym,curr,DIRECTION_FORWARD);
      }
    else
       curr = curr->next;
   }
/*---------------------------------------------------------------------*/
/* Lastly append the synonym at the end of the linked list.            */
/*---------------------------------------------------------------------*/
 curr = lll_add(first_prefix_synonym,last_prefix_synonym,sizeof(LINE));
 if (curr == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 curr->line = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)macroname)+1)*sizeof(CHARTYPE));
 if (curr->line == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 strcpy((DEFCHAR *)curr->line,(DEFCHAR *)macroname);
 curr->name = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)synonym)+1)*sizeof(CHARTYPE));
 if (curr->name == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
   }
 strcpy((DEFCHAR *)curr->name,(DEFCHAR *)synonym);
 last_prefix_synonym = curr;
 if (first_prefix_synonym == NULL)
    first_prefix_synonym = last_prefix_synonym;
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
/*man***************************************************************************
NAME
     find_prefix_synonym

SYNOPSIS
     CHARTYPE *find_prefix_synonym(synonym)
     CHARTYPE *synonym;

DESCRIPTION
     The find_prefix_synonym function finds a synonym for 'synonym'
     and returns that value. If no synonym exists, the 'synonym' is
     returned unchanged.

     This function is only available if REXX support is available.
     
RETURN VALUE
     Either the macroname associated with 'synonym' or 'synonym'.
*******************************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *find_prefix_synonym(CHARTYPE *synonym)
#else
CHARTYPE *find_prefix_synonym(synonym)
CHARTYPE *synonym;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  find_prefix_synonym");
#endif
 curr = first_prefix_synonym;
 while(curr != NULL)
   {
    if (strcmp((DEFCHAR *)synonym,(DEFCHAR *)curr->name) == 0)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return((CHARTYPE *)curr->line);
      }
    curr = curr->next;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(synonym);
}
/*man***************************************************************************
NAME
     find_prefix_oldname

SYNOPSIS
     CHARTYPE *find_prefix_oldname(oldname)
     CHARTYPE *oldname;

DESCRIPTION
     The find_prefix_oldname function finds the synonym for
     'oldname' and returns that value. If no synonym exists, the 
     'oldname' is returned unchanged.

     This function is only available if REXX support is available.
     
RETURN VALUE
     Either the synonym associated with 'oldname' or 'oldname'.
*******************************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *find_prefix_oldname(CHARTYPE *oldname)
#else
CHARTYPE *find_prefix_oldname(oldname)
CHARTYPE *oldname;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  find_prefix_oldname");
#endif
 curr = first_prefix_synonym;
 while(curr != NULL)
   {
    if (strcmp((DEFCHAR *)oldname,(DEFCHAR *)curr->line) == 0)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(curr->name);
      }
    curr = curr->next;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(oldname);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *get_prefix_command(LINETYPE prefix_index)
#else
CHARTYPE *get_prefix_command(prefix_index)
LINETYPE prefix_index;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("prefix.c:  get_prefix_command");
#endif
/*---------------------------------------------------------------------*/
/* Just return a pointer to the prefix command associated with the     */
/* supplied index.                                                     */
/*---------------------------------------------------------------------*/
#ifdef THE_TRACE
 trace_return();
#endif
 return(pc[prefix_index].cmd);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static short post_prefix_add(THE_PPC *curr_ppc,short cmd_idx,LINETYPE number_lines)
#else
static short post_prefix_add(curr_ppc,cmd_idx,number_lines)
THE_PPC *curr_ppc;
short cmd_idx;
LINETYPE number_lines;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 rc = Sos_leftedge((CHARTYPE *)"");
 if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    if (!BOF((CURRENT_VIEW->focus_line)+1L))
       rc = THEcursor_down(TRUE);
   }
 return(rc);
}
