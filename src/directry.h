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

#define fsize_t long

struct dirfile {
  char* fname;                /* file name */
  char* lname;                /* link name */
  mode_t fattr;               /* file attributes */
  fsize_t fsize;              /* size of file */
  char f_hh;                  /* hour */
  char f_mi;                  /* minute */
  char f_ss;                  /* second */
  char f_dd;                  /* day */
  char f_mm;                  /* month */
  int f_yy;                   /* year */
  int facl;                   /* acl */
  int fname_length;           /* length of filename */
};

int date_comp();
int time_comp();
int size_comp();
int name_comp();
int dir_comp();

char* make_full(char*, char*);

short getfiles(char*, char*, struct dirfile**, struct dirfile **);

char* file_attrs(mode_t, char*, int);
char* file_date(struct dirfile*, char*);
char* file_time(struct dirfile*, char*);

