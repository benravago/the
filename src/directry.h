// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

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
  uchar *fname;                 /* file name */
  uchar *lname;                 /* link name */
  mode_t fattr;                 /* file attributes */
  long  fsize;                  /* size of file */
  uchar f_hh;                   /* hour */
  uchar f_mi;                   /* minute */
  uchar f_ss;                   /* second */
  uchar f_dd;                   /* day */
  uchar f_mm;                   /* month */
  int f_yy;                     /* year */
  int facl;                     /* acl */
  int fname_length;             /* length of filename */
};

int date_comp(const void*, const void*);
int time_comp(const void*, const void*);
int size_comp(const void*, const void*);
int name_comp(const void*, const void*);
int dir_comp(const void*, const void*);

uchar *make_full(uchar *, uchar *);

short getfiles(uchar *, uchar *, struct dirfile **, struct dirfile **);

uchar *file_attrs(mode_t, uchar *, int);
uchar *file_date(struct dirfile *, uchar *);
uchar *file_time(struct dirfile *, uchar *);

