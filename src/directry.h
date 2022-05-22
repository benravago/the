// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include <dirent.h>
#include <sys/stat.h>

#include <time.h>

#define F_RO 0
#define F_HI 0
#define F_SY 0
#define F_DI 0
#define F_AR 0

#define ATTR_TYPE mode_t
#define SIZE_TYPE long
#define TIME_TYPE time_t
#define DATE_TYPE CHARTYPE
#define D_TYPE    struct tm *
#define T_TYPE    struct tm *
#define DONE_TYPE short

#define HOUR_MASK (time->tm_hour)
#define MINU_MASK (time->tm_min)
#define DAYS_MASK (date->tm_mday)
#define MONT_MASK (mon[date->tm_mon])
#define YEAR_MASK (date->tm_year)

#define HH_MASK(a) (a->tm_hour)
#define MI_MASK(a) (a->tm_min)
#define SS_MASK(a) (a->tm_sec)
#define DD_MASK(a) (a->tm_mday)
#define MM_MASK(a) (a->tm_mon)
#define YY_MASK(a) ((a->tm_year)+1900)

struct dirfile {
  CHARTYPE *fname;              /* file name */
  CHARTYPE *lname;              /* link name */
  ATTR_TYPE fattr;              /* file attributes */
  SIZE_TYPE fsize;              /* size of file */
  CHARTYPE f_hh;                /* hour */
  CHARTYPE f_mi;                /* minute */
  CHARTYPE f_ss;                /* second */
  CHARTYPE f_dd;                /* day */
  CHARTYPE f_mm;                /* month */
  int f_yy;                     /* year */
  int facl;                     /* acl */
  int fname_length;             /* length of filename */
};

int date_comp();
int time_comp();
int size_comp();
int name_comp();
int dir_comp(); 

CHARTYPE *make_full(CHARTYPE *, CHARTYPE *);

short getfiles(CHARTYPE *, CHARTYPE *, struct dirfile **, struct dirfile **);

CHARTYPE *file_attrs(ATTR_TYPE, CHARTYPE *, int);
CHARTYPE *file_date(struct dirfile *, CHARTYPE *);
CHARTYPE *file_time(struct dirfile *, CHARTYPE *);

short set_dirtype(CHARTYPE *);

