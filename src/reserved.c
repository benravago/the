/***********************************************************************/
/* RESERVED.C -                                                        */
/* This file contains funtions related to reserved lines.              */
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

static char RCSid[] = "$Id: reserved.c,v 1.1 1999/06/25 06:11:56 mark Exp mark $";

#include <the.h>
#include <proto.h>

/***********************************************************************/
#ifdef HAVE_PROTO
RESERVED *add_reserved_line(CHARTYPE *spec,CHARTYPE *line,short base,short off,COLOUR_ATTR *attr)
#else
RESERVED *add_reserved_line(spec,line,base,off,attr)
CHARTYPE *spec,*line;
short base;
short off;
COLOUR_ATTR *attr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 RESERVED *curr=NULL;
 CHARTYPE *templine=line;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("reserved.c:add_reserved_line");
#endif
/*---------------------------------------------------------------------*/
/* First check if the row already has a reserved line on it...         */
/*---------------------------------------------------------------------*/
 if ((curr = find_reserved_line(current_screen,FALSE,0,base,off)) != NULL)
    delete_reserved_line(base,off);
 curr = rll_add(CURRENT_FILE->first_reserved,CURRENT_FILE->first_reserved,sizeof(RESERVED));
 if (CURRENT_FILE->first_reserved == NULL)
    CURRENT_FILE->first_reserved = curr;
 if (templine == NULL)
    templine = (CHARTYPE *)"";
 if ((curr->line = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)templine)+1)*sizeof(CHARTYPE))) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(NULL);
   }
 if ((curr->disp = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)templine)+1)*sizeof(CHARTYPE))) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(NULL);
   }
 if ((curr->spec = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)spec)+1)*sizeof(CHARTYPE))) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(NULL);
   }
 if ((curr->attr = (COLOUR_ATTR *)(*the_malloc)(sizeof(COLOUR_ATTR))) == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(NULL);
   }
 strcpy((DEFCHAR *)curr->line,(DEFCHAR *)templine);
 strcpy((DEFCHAR *)curr->spec,(DEFCHAR *)spec);
 curr->length = strlen((DEFCHAR *)templine);
 curr->base = base;
 curr->off = off;
 memcpy(curr->attr,attr,sizeof(COLOUR_ATTR));
 parse_reserved_line(curr);
#ifdef THE_TRACE
 trace_return();
#endif
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
RESERVED *find_reserved_line(CHARTYPE scrno,bool find_by_row,ROWTYPE row,short base,short off)
#else
RESERVED *find_reserved_line(scrno,find_by_row,row,base,off)
CHARTYPE scrno;
bool find_by_row;
ROWTYPE row;
short base,off;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 RESERVED *curr=SCREEN_FILE(scrno)->first_reserved;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("reserved.c:find_reserved_line");
#endif
 while(curr != NULL)
   {
    if (find_by_row)
      {
       if (curr->base == POSITION_TOP
       &&  row == curr->off-1)
          break;
       if (curr->base == POSITION_BOTTOM
       &&  row == (curr->off+screen[scrno].rows[WINDOW_FILEAREA]))
          break;
       if (curr->base == POSITION_MIDDLE
       &&  row == (curr->off+(screen[scrno].rows[WINDOW_FILEAREA]/2))-1)
          break;
      }
    else
      {
       if (curr->base == base
       &&  curr->off == off)
          break;
      }
    curr = curr->next;
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(curr);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short delete_reserved_line(short base,short off)
#else
short delete_reserved_line(base,off)
short base,off;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 RESERVED *curr=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("reserved.c:delete_reserved_line");
#endif
 if ((curr = find_reserved_line(current_screen,FALSE,0,base,off)) == NULL)
   {
    display_error(64,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_NO_LINES_CHANGED);
   }
 if (curr->line != NULL)
    (*the_free)(curr->line);
 if (curr->disp != NULL)
    (*the_free)(curr->disp);
 if (curr->spec != NULL)
    (*the_free)(curr->spec);
 if (curr->attr != NULL)
    (*the_free)(curr->attr);
 rll_del(&CURRENT_FILE->first_reserved,NULL,curr,DIRECTION_FORWARD);
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
