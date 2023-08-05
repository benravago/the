// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* Directory routines                                     */

#include "the.h"
#include "proto.h"

#include <fnmatch.h>

char_t *make_full(char_t *path, char_t *file) {
  static char_t  filebuf[BUFSIZ];
  short pathlen = strlen((char *) path);

  if (pathlen + 1 + strlen((char *) file) + 1 > BUFSIZ) {
    return (NULL);
  }
  if (!strcmp((char *) path, "") || !strcmp((char *) path, ".")) {
    (void) strcpy((char *) filebuf, (char *) file);
    return (filebuf);
  }
  (void) strcpy((char *) filebuf, (char *) path);
  if (*(path + (pathlen - 1)) != ISLASH && *file != ISLASH) {
    (void) strcat((char *) filebuf, (char *) ISTR_SLASH);
  }
  (void) strcat((char *) filebuf, (char *) file);
  return (filebuf);
}

short getfiles(char_t *path, char_t *files, struct dirfile **dpfirst, struct dirfile **dplast) {
  DIR *dirp = NULL;
  struct stat sp;
  struct dirent *direntp = NULL;
  struct dirfile *dp = NULL;
  char_t *full_name = NULL;
  short entries = 10;
  struct tm *timp = NULL;

  dirp = opendir((char *) path);
  if (dirp == NULL) {
    return (10);
  }
  dp = *dpfirst = (struct dirfile *) malloc (entries * sizeof(struct dirfile));
  if (dp == NULL) {
    return (RC_OUT_OF_MEMORY);
  }
  memset(dp, 0, entries * sizeof(struct dirfile));
  memset(*dpfirst, 0, entries * sizeof(struct dirfile));
  *dplast = *dpfirst + entries;

  for (direntp = readdir(dirp); direntp != NULL; direntp = readdir(dirp)) {
    if (fnmatch((char *) files, (char *) direntp->d_name, 0) == 0) {
      if ((full_name = make_full(path, (char_t *) direntp->d_name)) == NULL) {
        return (RC_OUT_OF_MEMORY);
      }
      if (lstat((char *) full_name, &sp) != 0) {
        continue;
      }
      dp->fname_length = strlen(direntp->d_name) + 1;
      if ((dp->fname = (char_t *) malloc (dp->fname_length * sizeof(char_t))) == NULL) {
        return (RC_OUT_OF_MEMORY);
      }
      strcpy((char *) dp->fname, direntp->d_name);
      dp->fattr = sp.st_mode;
      dp->facl = 0;
      timp = localtime(&(sp.st_mtime));
      dp->f_hh = HH_MASK(timp);
      dp->f_mi = MI_MASK(timp);
      dp->f_ss = SS_MASK(timp);
      dp->f_dd = DD_MASK(timp);
      dp->f_mm = MM_MASK(timp);
      dp->f_yy = YY_MASK(timp);
      dp->fsize = sp.st_size;
      dp->lname = NULL;
      /*
       * If we have the lstat() function, and the current file
       * is a symbolic link, go and get the filename the symbolic
       * link points to...
       */
      if (S_ISLNK(dp->fattr)) {
        char buf[MAX_FILE_NAME + 1];
        int rc = 0;

        rc = readlink((char *) full_name, buf, sizeof(buf));
        if (rc != (-1)) {
          if ((dp->lname = (char_t *) malloc ((rc + 1) * sizeof(char_t))) == NULL) {
            return (RC_OUT_OF_MEMORY);
          }
          memcpy((char *) dp->lname, buf, rc);
          dp->lname[rc] = '\0';
        }
      }
      dp++;
      if (dp == *dplast) {
        *dpfirst = (struct dirfile *) realloc ((char_t *) * dpfirst, 2 * entries * sizeof(struct dirfile));
        if (*dpfirst == NULL) {
          return (RC_OUT_OF_MEMORY);
        }
        dp = *dpfirst + entries;
        *dplast = dp + entries;
        entries *= 2;
      }
    }
  }
  closedir(dirp);
  *dplast = dp;
  return (0);
}

int date_compare(struct dirfile *first, struct dirfile *next) {
  if (first->f_yy > next->f_yy) {
    return (1);
  } else if (first->f_yy < next->f_yy) {
    return (-1);
  }
  if (first->f_mm > next->f_mm) {
    return (1);
  } else if (first->f_mm < next->f_mm) {
    return (-1);
  }
  if (first->f_dd > next->f_dd) {
    return (1);
  } else if (first->f_dd < next->f_dd) {
    return (-1);
  }
  return (0);
}

int time_compare(struct dirfile *first, struct dirfile *next) {
  if (first->f_hh > next->f_hh) {
    return (1);
  } else if (first->f_hh < next->f_hh) {
    return (-1);
  }
  if (first->f_mi > next->f_mi) {
    return (1);
  } else if (first->f_mi < next->f_mi) {
    return (-1);
  }
  if (first->f_ss > next->f_ss) {
    return (1);
  } else if (first->f_ss < next->f_ss) {
    return (-1);
  }
  return (0);
}

int date_comp(const void *in_first, const void *in_next) {
  int rc = 0;
  struct dirfile *first = (struct dirfile *) in_first, *next = (struct dirfile *) in_next;

  rc = date_compare(first, next);
  if (rc == 0) {
    rc = time_compare(first, next);
  }
  if (rc == 0) {
    rc = strcmp((char *) first->fname, (char *) next->fname);
  }
  if (rc == 0) {
    return (0);
  }
  if (DIRORDERx == DIRSORT_DESC) {
    rc = (rc > 0) ? (-1) : 1;
  }
  return (rc);
}

int time_comp(const void *in_first, const void *in_next) {
  int rc = 0;
  struct dirfile *first = (struct dirfile *) in_first, *next = (struct dirfile *) in_next;

  rc = time_compare(first, next);
  if (rc == 0) {
    rc = strcmp((char *) first->fname, (char *) next->fname);
  }
  if (rc == 0) {
    return (0);
  }
  if (DIRORDERx == DIRSORT_DESC) {
    rc = (rc > 0) ? (-1) : 1;
  }
  return (rc);
}

int dir_comp(const void *in_first, const void *in_next) {
  int first_dir = 0;
  int next_dir = 0;
  int rc = 0;
  struct dirfile *first = (struct dirfile *) in_first, *next = (struct dirfile *) in_next;

  first_dir = S_ISDIR(first->fattr);
  next_dir = S_ISDIR(next->fattr);
  if (first_dir && !next_dir) {
    rc = (-1);
  }
  if (!first_dir && next_dir) {
    rc = 1;
  }
  if (rc == 0) {
    rc = strcmp((char *) first->fname, (char *) next->fname);
  }
  if (rc == 0) {
    return (0);
  }
  if (DIRORDERx == DIRSORT_DESC) {
    rc = (rc > 0) ? (-1) : 1;
  }
  return (rc);
}

int size_comp(const void *in_first, const void *in_next) {
  int rc = 0;
  struct dirfile *first = (struct dirfile *) in_first, *next = (struct dirfile *) in_next;

  if (first->fsize > next->fsize) {
    rc = 1;
  } else {
    if (first->fsize < next->fsize) {
      rc = -1;
    }
  }
  if (rc == 0) {
    rc = strcmp((char *) first->fname, (char *) next->fname);
  }
  if (rc == 0) {
    return (0);
  }
  if (DIRORDERx == DIRSORT_DESC) {
    rc = (rc > 0) ? (-1) : 1;
  }
  return (rc);
}

int name_comp(const void *in_first, const void *in_next) {
  int rc = 0;
  struct dirfile *first = (struct dirfile *) in_first, *next = (struct dirfile *) in_next;

  rc = strcmp((char *) first->fname, (char *) next->fname);
  if (rc == 0) {
    return (0);
  }
  if (DIRORDERx == DIRSORT_DESC) {
    rc = (rc > 0) ? (-1) : 1;
  }
  return (rc);
}

char_t *file_date(struct dirfile *date, char_t *str_date) {
  static char_t  *mon[12] = { (char_t *) "Jan", (char_t *) "Feb", (char_t *) "Mar", (char_t *) "Apr", (char_t *) "May", (char_t *) "Jun", (char_t *) "Jul", (char_t *) "Aug", (char_t *) "Sep", (char_t *) "Oct", (char_t *) "Nov", (char_t *) "Dec" };
  sprintf((char *) str_date, "%2d-%3.3s-%4.4d", date->f_dd, mon[date->f_mm], date->f_yy);
  return (str_date);
}

char_t *file_time(struct dirfile *time, char_t *str_time) {
  sprintf((char *) str_time, "%2d:%2.2d", time->f_hh, time->f_mi);
  return (str_time);
}

char_t *file_attrs(mode_t attrs, char_t *str_attr, int facl) {
  mode_t ftype = attrs;

  str_attr[11] = '\0';
  str_attr[10] = ' ';
  if (facl) {
    str_attr[10] = '+';
  }
  str_attr[0] = '-';
  if (S_ISDIR(ftype))  str_attr[0] = 'd';
  if (S_ISCHR(ftype))  str_attr[0] = 'c';
  if (S_ISBLK(ftype))  str_attr[0] = 'b';
  if (S_ISFIFO(ftype)) str_attr[0] = 'p';
  if (S_ISLNK(ftype))  str_attr[0] = 'l';
  if (S_ISSOCK(ftype)) str_attr[0] = 's';

  str_attr[1] = (attrs & S_IRUSR) ? 'r' : '-';
  str_attr[2] = (attrs & S_IWUSR) ? 'w' : '-';
  str_attr[3] = (attrs & S_IXUSR) ? 'x' : '-';
  str_attr[3] = (attrs & S_ISUID) ? 's' : str_attr[3];
  str_attr[4] = (attrs & S_IRGRP) ? 'r' : '-';
  str_attr[5] = (attrs & S_IWGRP) ? 'w' : '-';
  str_attr[6] = (attrs & S_IXGRP) ? 'x' : '-';
  str_attr[6] = (attrs & S_ISGID) ? 's' : str_attr[6];
  str_attr[7] = (attrs & S_IROTH) ? 'r' : '-';
  str_attr[8] = (attrs & S_IWOTH) ? 'w' : '-';
  str_attr[9] = (attrs & S_IXOTH) ? 'x' : '-';
  return (str_attr);
}

