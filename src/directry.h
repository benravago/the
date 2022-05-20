/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
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
 */

/*
$Id: directry.h,v 1.8 2013/01/23 04:13:42 mark Exp $
*/









# if defined(M_XENIX)
#   include <sys/dirent.h>
#   include <sys/ndir.h>
# else
#     include <dirent.h>
#     include <sys/stat.h>
# endif
#  include <time.h>

# define F_RO 0
# define F_HI 0
# define F_SY 0
# define F_DI 0
# define F_AR 0

# if defined(ATT) || defined(M_XENIX)
#  define mode_t ushort
# endif

# define ATTR_TYPE mode_t
# define SIZE_TYPE long
# define TIME_TYPE time_t
# define DATE_TYPE CHARTYPE
# define D_TYPE    struct tm *
# define T_TYPE    struct tm *
# define DONE_TYPE short

# define HOUR_MASK (time->tm_hour)
# define MINU_MASK (time->tm_min)
# define DAYS_MASK (date->tm_mday)
# define MONT_MASK (mon[date->tm_mon])
# define YEAR_MASK (date->tm_year)

# define HH_MASK(a) (a->tm_hour)
# define MI_MASK(a) (a->tm_min)
# define SS_MASK(a) (a->tm_sec)
# define DD_MASK(a) (a->tm_mday)
# define MM_MASK(a) (a->tm_mon)
# define YY_MASK(a) ((a->tm_year)+1900)


struct dirfile {
   CHARTYPE *fname;     /* file name */
   CHARTYPE *lname;     /* link name */
   ATTR_TYPE   fattr;      /* file attributes */
   SIZE_TYPE   fsize;      /* size of file */
   CHARTYPE f_hh;    /* hour */
   CHARTYPE f_mi;    /* minute */
   CHARTYPE f_ss;    /* second */
   CHARTYPE f_dd;    /* day */
   CHARTYPE f_mm;    /* month */
   int   f_yy;    /* year */
   int   facl;    /* acl */
   int   fname_length;     /* length of filename */
};

int date_comp();           /* this has been deliberatly left undefined */
int time_comp();           /* this has been deliberatly left undefined */
int size_comp();           /* this has been deliberatly left undefined */
int name_comp();           /* this has been deliberatly left undefined */
int dir_comp();            /* this has been deliberatly left undefined */
CHARTYPE *make_full(CHARTYPE *,CHARTYPE *);
short getfiles(CHARTYPE *,CHARTYPE *,struct dirfile **,struct dirfile **);
CHARTYPE *file_attrs(ATTR_TYPE,CHARTYPE *,int);
CHARTYPE *file_date(struct dirfile *,CHARTYPE *);
CHARTYPE *file_time(struct dirfile *,CHARTYPE *);
short set_dirtype(CHARTYPE *);

