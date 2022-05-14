/***********************************************************************/
/* NONANSI.C -                                                         */
/* This file contains all calls to non-ansi conforming routines.       */
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

static char RCSid[] = "$Id: nonansi.c,v 1.2 1999/08/07 06:27:43 mark Exp mark $";

#include <the.h>
#include <proto.h>

#if defined(UNIX) || defined(__EMX__)
# include <pwd.h>
#endif
#include <errno.h>

/*#define DEBUG 1*/
#ifdef __EMX__				/* prevent DOS- and OS2-include's    */
#elif defined(DOS)
#  include <dos.h>
#  if !defined(GO32)
#    include <direct.h>
#  endif
#elif defined(OS2)
#  include <direct.h>
#  include <io.h>
#  ifndef S_IFMT
#    define S_IFMT 0xF000
#  endif
#endif

#if defined(WIN32) && defined(_MSC_VER)
# include <direct.h>
#endif

/***********************************************************************/
#ifdef HAVE_PROTO
short file_readable(CHARTYPE *filename)
#else
short file_readable(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: file_readable");
#endif
 if (access((DEFCHAR *)filename,R_OK) == (-1))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(FALSE);
   }
#ifdef THE_TRACE
    trace_return();
#endif
 return(TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short file_writable(CHARTYPE *filename)
#else
short file_writable(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: file_writable");
#endif
 if (!file_exists(filename))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(TRUE);
   }
 if (access((DEFCHAR *)filename,W_OK) == (-1))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(FALSE);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short file_exists(CHARTYPE *filename)
#else
short file_exists(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: file_exists");
#endif
 if (access((DEFCHAR *)filename,F_OK) == (-1))
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(FALSE);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short remove_file(CHARTYPE *filename)
#else
short remove_file(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: remove_file");
#endif
if(filename == NULL)
     return(RC_OK);
#ifdef VMS
 if (delete(filename) == (-1))
#else
 if (unlink((DEFCHAR *)filename) == (-1))
#endif
   {
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_ACCESS_DENIED);
   }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
#if defined(DOS) || defined(OS2) || defined(WIN32) || defined(__EMX__)
/***********************************************************************/
#ifdef HAVE_PROTO
short splitpath(CHARTYPE *filename)
#else
short splitpath(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short len=0;
 CHARTYPE work_filename[MAX_FILE_NAME+1] ;
 CHARTYPE current_dir[MAX_FILE_NAME+1] ;
#ifdef __EMX__
 int new_dos_disk=0,current_dos_disk=0,temp_disk=0;
#elif defined(DOS)
 short new_dos_disk=0,current_dos_disk=0;        /* 1 - A,2 - B... */
 short temp_disk=0;
#elif defined (OS2)
 ULONG logical_os2_drives=0L;
#  if defined( __32BIT__) || defined(__386__)
 ULONG new_dos_disk=0L,current_dos_disk=0L,temp_disk=0L;
#  else
 USHORT new_dos_disk=0,current_dos_disk=0,temp_disk=0;
#  endif
#elif defined(WIN32)
 unsigned int new_dos_disk=0,current_dos_disk=0;        /* 1 - A,2 - B... */
 unsigned int temp_disk=0;
#endif
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: splitpath");
#endif
/*---------------------------------------------------------------------*/
/* Save the current directory.                                         */
/*---------------------------------------------------------------------*/
#if defined(EMX)
 _getcwd2(curr_path,MAX_FILE_NAME);
#else
 getcwd((DEFCHAR *)curr_path,MAX_FILE_NAME);
#endif
 strcpy(sp_path,"");
 strcpy(sp_fname,"");
 strcpy(work_filename,filename);
/*---------------------------------------------------------------------*/
/* If the supplied filename is empty, set the path = cwd and filename  */
/* equal to blank.                                                     */
/*---------------------------------------------------------------------*/
 if (strcmp(filename,"") == 0)
   {
#if defined(__EMX__)
    _getcwd2(sp_path,MAX_FILE_NAME);
#else
    getcwd(sp_path,MAX_FILE_NAME);
#endif
    strcpy(sp_fname,"");
   }
/*---------------------------------------------------------------------*/
/* For DOS and OS/2, get current drive.                                */
/*---------------------------------------------------------------------*/
#ifdef __EMX__
 current_dos_disk = (_getdrive() - 'A') + 1;
#elif defined(DOS)
# if defined(TC) || defined(GO32)
 current_dos_disk = (short)(getdisk() + 1);
# endif
# if defined(MSC)
 _dos_getdrive(&current_dos_disk);
# endif
#elif defined (OS2)
 DosQueryCurrentDisk(&current_dos_disk,&logical_os2_drives);
#else 				/* WIN32			     */
# if defined(__WATCOMC__)
 _dos_getdrive(&current_dos_disk);
# else					/* assume MSC			     */
 current_dos_disk = _getdrive();
# endif
#endif

 new_dos_disk = current_dos_disk;
/*---------------------------------------------------------------------*/
/* For DOS and OS/2, if a drive specified determine the drive number.  */
/*---------------------------------------------------------------------*/
 if (*(filename+1) == ':')/* we assume this means a drive secification */
    new_dos_disk = (toupper(*(filename)) - 'A') + 1;
/*---------------------------------------------------------------------*/
/* For DOS and OS/2, change to the specified disk (if supplied and     */
/* different). Validate the drive number.                              */
/*---------------------------------------------------------------------*/
 if (new_dos_disk != current_dos_disk)
   {
#if defined(__EMX__)
    _chdrive((char)((new_dos_disk-1)+'A'));
    temp_disk = (_getdrive() - 'A') + 1;
#elif defined(DOS)
# if defined(TC) || defined(GO32)
    setdisk((short)(new_dos_disk-1));
    temp_disk = getdisk()+1;
# else					/* assume MSC			     */
    _dos_setdrive(new_dos_disk,&temp_disk);
    _dos_getdrive(&temp_disk);
# endif
#elif defined(OS2)
    DosSetDefaultDisk(new_dos_disk);
    DosQueryCurrentDisk(&temp_disk,&logical_os2_drives);
#else 				/* assume WIN32 		     */
# if defined(__WATCOMC__)
    _dos_setdrive(new_dos_disk,&temp_disk);
    _dos_getdrive(&temp_disk);
# else					/* assume MSC			     */
    _chdrive(new_dos_disk);
    temp_disk = _getdrive();
# endif
#endif

    if (temp_disk != new_dos_disk)  /* invalid drive */
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return (RC_BAD_DRIVE);
      }
   }
/*---------------------------------------------------------------------*/
/* Save the current working directory on the specified drive, or the   */
/* current drive if not specified.                                     */
/*---------------------------------------------------------------------*/
#if defined(__EMX__)
 _getcwd2(current_dir,MAX_FILE_NAME);
#else
 getcwd(current_dir,MAX_FILE_NAME);
#endif
/*---------------------------------------------------------------------*/
/* If the work_filename contains a drive specifier, special handling is*/
/* needed.                                                             */
/*---------------------------------------------------------------------*/
 switch(strlen(filename))
   {
    case 1:
         break;
/*---------------------------------------------------------------------*/
/* If the filename consists only of a drive specifier, copy the current*/
/* directory for the now new drive into work_filename.                 */
/*---------------------------------------------------------------------*/
    case 2:
         if (*(filename+1) == ':')
            strcpy(work_filename,current_dir);
         break;
    default:
         if (*(filename+1) == ':'
         &&  *(filename+2) != ISLASH)
           {
            strcpy(work_filename,current_dir);
            if (current_dir[strlen(current_dir)-1] != ISLASH)
               strcat(work_filename,ISTR_SLASH);
            strcat(work_filename,filename+2);
           }
         break;
   }
/*---------------------------------------------------------------------*/
/* First determine if the supplied filename is a directory.            */
/*---------------------------------------------------------------------*/
 if (chdir(work_filename) == 0)  /* valid directory */
   {
#if defined(__EMX__)
    _getcwd2(sp_path,MAX_FILE_NAME);
#else
    getcwd(sp_path,MAX_FILE_NAME);
#endif
    strcpy(sp_fname,"");
   }
 else          /* here if the file is not a directory */
   {
    len = strzreveq(work_filename,ISLASH);
    switch(len)
      {
       case (-1):
#if defined(__EMX__)
            _getcwd2(sp_path,MAX_FILE_NAME);
#else
            getcwd(sp_path,MAX_FILE_NAME);
#endif
            strcpy(sp_fname,work_filename);
            break;
       case 0:
            strcpy(sp_path,work_filename);
            sp_path[1] = '\0';
            strcpy(sp_fname,work_filename+1+len);
            break;
      default:
            strcpy(sp_path,work_filename);
            sp_path[len] = '\0';
            strcpy(sp_fname,work_filename+1+len);
            break;
     }
   }
 if (strlen(sp_path) == 2
 && sp_path[1] == ':')
    strcat(sp_path,ISTR_SLASH);
/*---------------------------------------------------------------------*/
/* Change directory to the supplied path, if possible and store the    */
/* expanded path.                                                      */
/* If an error, restore the current path.                              */
/*---------------------------------------------------------------------*/
 if (chdir(sp_path) != 0)
   {
    if (new_dos_disk != current_dos_disk)
      {
       chdir(current_dir);
#if defined(__EMX__)
       _chdrive((char)((current_dos_disk-1)+'A'));
#elif defined(DOS)
# if defined(TC) || defined(GO32)
       setdisk((short)(current_dos_disk-1));
# else					/* assume MSC			     */
       _dos_setdrive(current_dos_disk,&temp_disk);
# endif
#elif defined OS2
       DosSetDefaultDisk(new_dos_disk);
#else 				/* assume WIN32 		     */
# if defined(__WATCOMC__)
       _dos_setdrive(current_dos_disk,&temp_disk);
# else					/* assume MSC			     */
       _chdrive(current_dos_disk);
# endif
#endif
      }
    chdir(curr_path);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_FILE_NOT_FOUND);
   }
/*---------------------------------------------------------------------*/
/* We are now in a valid directory, get the fully qualified directory  */
/* name.                                                               */
/*---------------------------------------------------------------------*/
#if defined(__EMX__)
 _getcwd2(sp_path,MAX_FILE_NAME);
#else
 getcwd(sp_path,MAX_FILE_NAME);
#endif
/*---------------------------------------------------------------------*/
/* For DOS or OS/2, change back to the current directory of the now    */
/* current disk and then change back to the original disk.             */
/*---------------------------------------------------------------------*/
 if (new_dos_disk != current_dos_disk)
   {
    if (chdir(current_dir) != 0)
      {
#ifdef THE_TRACE
       trace_return();
#endif
       return(RC_FILE_NOT_FOUND);
      }
#if defined(__EMX__)
    _chdrive((char)((current_dos_disk-1)+'A'));
#elif defined(DOS)
# if defined(TC) || defined(GO32)
    setdisk((short)(current_dos_disk-1));
# else					/* assume MSC			     */
    _dos_setdrive(current_dos_disk,&temp_disk);
# endif
#elif defined(OS2)
    DosSetDefaultDisk(current_dos_disk);
#else 				/* assume WIN32 		     */
# if defined(__WATCOMC__)
    _dos_setdrive(current_dos_disk,&temp_disk);
# else					/* assume MSC			     */
    _chdrive(current_dos_disk);
# endif
#endif
   }

 chdir(curr_path);
/*---------------------------------------------------------------------*/
/* Append the OS directory character to the path if it doesn't already */
/* end in the character.                                               */
/*---------------------------------------------------------------------*/
#ifndef VMS
 len = strlen(sp_path);
 if (len > 0)
    if (sp_path[len-1] != ISLASH)
       strcat(sp_path,(CHARTYPE *)ISTR_SLASH);
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
#elif defined(__QNX__)
/***********************************************************************/
#ifdef HAVE_PROTO
short splitpath(CHARTYPE *filename)
#else
short splitpath(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short len=0;
 CHARTYPE work_filename[MAX_FILE_NAME+1] ;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: splitpath");
#endif
/*---------------------------------------------------------------------*/
/* Copy the argument to a working area                                 */
/*---------------------------------------------------------------------*/
 getcwd((DEFCHAR *)curr_path,MAX_FILE_NAME);
 strcpy((DEFCHAR *)sp_path,"");
 strcpy((DEFCHAR *)sp_fname,"");
 strcpy((DEFCHAR *)work_filename,(DEFCHAR *)filename);
/*---------------------------------------------------------------------*/
/* If the supplied filename is empty, set the path = cwd and filename  */
/* equal to blank.                                                     */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)filename,"") == 0)
 {
    getcwd((DEFCHAR *)sp_path,MAX_FILE_NAME);
    strcpy((DEFCHAR *)sp_fname,"");
 }
/*---------------------------------------------------------------------*/
/* Check if the first character is tilde; translate HOME env variable  */
/* if there is one. Obviously only applicable to UNIX.                 */
/*---------------------------------------------------------------------*/
 if (*(work_filename) == '~')
 {
    if (*(work_filename+1) == ISLASH
    ||  *(work_filename+1) == '\0')
    {
       strcpy((DEFCHAR *)work_filename,(DEFCHAR *)getenv("HOME"));
       strcat((DEFCHAR *)work_filename,(DEFCHAR *)(filename+1));
    }
    else
    {
       struct passwd *pwd;

       strcpy((DEFCHAR *)sp_path,(DEFCHAR *)filename+1);
       if ((len = strzeq(sp_path,ISLASH)) != (-1))
          sp_path[len] = '\0';
       if ((pwd = getpwnam((DEFCHAR *)sp_path)) == NULL)
          return(RC_BAD_FILEID);
       strcpy((DEFCHAR *)work_filename,pwd->pw_dir);
       if (len != (-1))
          strcat((DEFCHAR *)work_filename,(DEFCHAR *)(filename+1+len));
    }
 }
/*---------------------------------------------------------------------*/
/* First determine if the supplied filename is a directory.            */
/*---------------------------------------------------------------------*/
 if (chdir(work_filename) == 0)
 {
    chdir(curr_path);
    strcpy((DEFCHAR *)sp_path,(DEFCHAR *)work_filename);
    strcpy((DEFCHAR *)sp_fname,"");
 }
 else          /* here if the file doesn't exist or is not a directory */
 {
    len = strzreveq(work_filename,ISLASH);
    switch(len)
    {
       case (-1):
          getcwd((DEFCHAR *)sp_path,MAX_FILE_NAME);
          strcpy((DEFCHAR *)sp_fname,(DEFCHAR *)work_filename);
          break;
       case 0:
          strcpy((DEFCHAR *)sp_path,(DEFCHAR *)work_filename);
          sp_path[1] = '\0';
          strcpy((DEFCHAR *)sp_fname,(DEFCHAR *)work_filename+1+len);
          break;
      default:
          strcpy((DEFCHAR *)sp_path,(DEFCHAR *)work_filename);
          sp_path[len] = '\0';
          strcpy((DEFCHAR *)sp_fname,(DEFCHAR *)work_filename+1+len);
          break;
    }
 }
/*---------------------------------------------------------------------*/
/* Change directory to the supplied path, if possible and store the    */
/* expanded path.                                                      */
/* If an error, restore the current path.                              */
/*---------------------------------------------------------------------*/
 if (qnx_fullpath((DEFCHAR *)work_filename,(DEFCHAR *)sp_path) == NULL)
 {
    chdir((DEFCHAR *)curr_path);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_FILE_NOT_FOUND);
 }
 strcpy(sp_path,work_filename);
 chdir((DEFCHAR *)curr_path);
/*---------------------------------------------------------------------*/
/* Append the OS directory character to the path if it doesn't already */
/* end in the character.                                               */
/*---------------------------------------------------------------------*/
 len = strlen((DEFCHAR *)sp_path);
 if (len > 0)
 {
    if (sp_path[len-1] != ISLASH)
       strcat((DEFCHAR *)sp_path,(DEFCHAR *)ISTR_SLASH);
 }
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
#else
/***********************************************************************/
#ifdef HAVE_PROTO
short splitpath(CHARTYPE *filename)
#else
short splitpath(filename)
CHARTYPE *filename;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short len=0;
 CHARTYPE work_filename[MAX_FILE_NAME+1] ;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: splitpath");
#endif
/*---------------------------------------------------------------------*/
/* Save the current directory.                                         */
/*---------------------------------------------------------------------*/
#if defined(EMX)
 _getcwd2(curr_path,MAX_FILE_NAME);
#else
 getcwd((DEFCHAR *)curr_path,MAX_FILE_NAME);
#endif
 strcpy((DEFCHAR *)sp_path,"");
 strcpy((DEFCHAR *)sp_fname,"");
 strcpy((DEFCHAR *)work_filename,(DEFCHAR *)filename);
/*---------------------------------------------------------------------*/
/* If the supplied filename is empty, set the path = cwd and filename  */
/* equal to blank.                                                     */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)filename,"") == 0)
   {
    getcwd((DEFCHAR *)sp_path,MAX_FILE_NAME);
    strcpy((DEFCHAR *)sp_fname,"");
   }
/*---------------------------------------------------------------------*/
/* Check if the first character is tilde; translate HOME env variable  */
/* if there is one. Obviously only applicable to UNIX.                 */
/*---------------------------------------------------------------------*/
 if (*(work_filename) == '~')
   {
    if (*(work_filename+1) == ISLASH
    ||  *(work_filename+1) == '\0')
      {
       strcpy((DEFCHAR *)work_filename,(DEFCHAR *)getenv("HOME"));
       strcat((DEFCHAR *)work_filename,(DEFCHAR *)(filename+1));
      }
    else
      {
       struct passwd *pwd;

       strcpy((DEFCHAR *)sp_path,(DEFCHAR *)filename+1);
       if ((len = strzeq(sp_path,ISLASH)) != (-1))
          sp_path[len] = '\0';
       if ((pwd = getpwnam((DEFCHAR *)sp_path)) == NULL)
          return(RC_BAD_FILEID);
       strcpy((DEFCHAR *)work_filename,pwd->pw_dir);
       if (len != (-1))
          strcat((DEFCHAR *)work_filename,(DEFCHAR *)(filename+1+len));
      }
   }
/*---------------------------------------------------------------------*/
/* First determine if the supplied filename is a directory.            */
/*---------------------------------------------------------------------*/
 if ((stat((DEFCHAR *)work_filename,&stat_buf) == 0)
 &&  (stat_buf.st_mode & S_IFMT) == S_IFDIR)
   {
    strcpy((DEFCHAR *)sp_path,(DEFCHAR *)work_filename);
    strcpy((DEFCHAR *)sp_fname,"");
   }
 else          /* here if the file doesn't exist or is not a directory */
   {
    len = strzreveq(work_filename,ISLASH);
    switch(len)
      {
       case (-1):
            getcwd((DEFCHAR *)sp_path,MAX_FILE_NAME);
            strcpy((DEFCHAR *)sp_fname,(DEFCHAR *)work_filename);
            break;
       case 0:
            strcpy((DEFCHAR *)sp_path,(DEFCHAR *)work_filename);
            sp_path[1] = '\0';
            strcpy((DEFCHAR *)sp_fname,(DEFCHAR *)work_filename+1+len);
            break;
      default:
            strcpy((DEFCHAR *)sp_path,(DEFCHAR *)work_filename);
            sp_path[len] = '\0';
            strcpy((DEFCHAR *)sp_fname,(DEFCHAR *)work_filename+1+len);
            break;
     }
   }
/*---------------------------------------------------------------------*/
/* Change directory to the supplied path, if possible and store the    */
/* expanded path.                                                      */
/* If an error, restore the current path.                              */
/*---------------------------------------------------------------------*/
 if (chdir((DEFCHAR *)sp_path) != 0)
   {
    chdir((DEFCHAR *)curr_path);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_FILE_NOT_FOUND);
   }
 getcwd((DEFCHAR *)sp_path,MAX_FILE_NAME);
 chdir((DEFCHAR *)curr_path);
/*---------------------------------------------------------------------*/
/* Append the OS directory character to the path if it doesn't already */
/* end in the character.                                               */
/*---------------------------------------------------------------------*/
#ifndef VMS
 len = strlen((DEFCHAR *)sp_path);
 if (len > 0)
    if (sp_path[len-1] != ISLASH)
       strcat((DEFCHAR *)sp_path,(DEFCHAR *)ISTR_SLASH);
#endif
#ifdef THE_TRACE
 trace_return();
#endif
 return(RC_OK);
}
#endif

#ifndef HAVE_RENAME
/***********************************************************************/
#ifdef HAVE_PROTO
short rename(CHARTYPE *path1,CHARTYPE *path2)
#else
short rename(path1,path2)
CHARTYPE *path1;
CHARTYPE *path2;
#endif
/***********************************************************************/
/* Function  : Emulate missing rename() function missing from SystemV  */
/* Parameters: path1    - old filename                                 */
/*             path2    - new_filename                                 */
/* Return    : 0 on success, -1 on error                               */
/***********************************************************************/
{
#ifdef THE_TRACE
 trace_function("nonansi.c: rename");
#endif
  if (link(path1,path2) != 0)
    {
#ifdef THE_TRACE
     trace_return();
#endif
     return(-1);
    }
  if (unlink(path1) != 0)
    {
#ifdef THE_TRACE
     trace_return();
#endif
     return(-1);
    }
#ifdef THE_TRACE
 trace_return();
#endif
 return(0);
}
#endif

#if defined(OS2) || (defined(__EMX__) && !defined(MSDOS))
/***********************************************************************/
/* Function  : Determine if file system allows the supplied filename.  */
/* Parameters: PathFn - directory path and filename without ext.     */
/* Note      : replaces function LongFileNames			       */
/* Return    : 1 if file system allows this filename		       */
/***********************************************************************/
bool IsPathAndFilenameValid(CHARTYPE *PathFn)
{
 CHARTYPE FullFn[CCHMAXPATH];
 CHARTYPE *buf;
 ULONG rc;
#ifdef THE_TRACE
 trace_function("nonansi.c: IsPathAndFilenameValid");
#endif

#ifdef __EMX__
 if (_osmode == DOS_MODE)
    return(0);
#endif
 if ((buf = malloc(strlen(PathFn) + 5)) == NULL)
    return(0);				/* Fake an error		     */
 strcpy(buf,PathFn);
 strcat(buf,".xxx");                    /* any extension                     */
#if defined(__32BIT__) || defined(__386__)
 rc = DosQueryPathInfo(buf,FIL_QUERYFULLNAME,FullFn,sizeof(FullFn));
#else
 rc = DosQPathInfo(buf,FIL_QUERYFULLNAME,FullFn,sizeof(FullFn),0ul);
#endif
 free(buf);
 if (rc == 0)
    return(1);
 return(0);
}
#endif

#ifdef USE_OLD_LONGFILENAMES		/* FGC: previous ifdef OS2	     */
#  if defined(__32BIT__) || defined(__386__)
#  define FSQBUFFERSIZE 64
/***********************************************************************/
bool LongFileNames(CHARTYPE *path)
/***********************************************************************/
/* Function  : Determine if file system allows long file names. (HPFS) */
/*             This is the 32-bit version.                             */
/* Parameters: path     - directory path                               */
/* Return    : 1 if file system is HPFS                                */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
        ULONG nDrive=0L;
        ULONG lMap=0L;
        char buffer[FSQBUFFERSIZE];
        FSQBUFFER2 *bData = (FSQBUFFER2 *) buffer;
        char bName[3];
        ULONG bDataLen=0L;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: LongFileNames");
#endif
 if ((strlen (path) > 0) && path [1] == ':')
    bName[0] = path[0];
 else
   {
    DosQueryCurrentDisk(&nDrive, &lMap);
    bName[0] = (char) (nDrive + 'A' - 1);
   }
 bName[1] = ':';
 bName[2] = 0;
 bDataLen = FSQBUFFERSIZE;
 DosQueryFSAttach(bName, 0, FSAIL_QUERYNAME, bData, &bDataLen);
 return(strcmp(bData->szFSDName + bData->cbName, "HPFS") == 0);
}
#  else

/***********************************************************************/
bool LongFileNames(CHARTYPE *path)
/***********************************************************************/
/* Function  : Determine if file system allows long filenames. (HPFS)  */
/*             This is the 16-bit version.                             */
/* Parameters: path     - directory path                               */
/* Return    : 1 if file system is HPFS                                */
/***********************************************************************/
{
typedef struct _FSNAME {
        USHORT cbName;
        UCHAR  szName[1];
} FSNAME;
typedef struct _FSQINFO {
        USHORT iType;
        FSNAME Name;
        UCHAR  rgFSAData[59];
} FSQINFO;
typedef FSQINFO FAR *PFSQINFO;
/*--------------------------- local data ------------------------------*/
 USHORT nDrive=0,cbData=0;
 ULONG lMap=0L;
 FSQINFO bData;
 BYTE bName[3];
 FSNAME *pFSName=NULL;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: LongFileNames");
#endif
 if ((strlen(path) > 0) && path[1] == ':')
    bName[0] = path[0];
 else
   {
    DosQueryCurrentDisk(&nDrive, &lMap);
    bName[0] = (char)(nDrive + '@');
   }
 bName[1] = ':';
 bName[2] = 0;
 cbData = sizeof(bData);
 DosQFSAttach((PSZ)bName,0,1,(PBYTE)&bData,&cbData,0L);
 pFSName = &bData.Name;
 (CHARTYPE *)pFSName += pFSName->cbName + sizeof(pFSName->cbName)+1;
 return(strcmp((CHARTYPE *)&(pFSName->szName[0]),"HPFS") == 0);
}
#  endif
#endif

#ifdef PRE_111_GO32
/***********************************************************************/
#ifdef HAVE_PROTO
short getdisk(void)
#else
short getdisk()
#endif
/***********************************************************************/
/* Function  : Determine the drive letter for the current disk.        */
/* Parameters: nil                                                     */
/* Return    : 0 for drive A:, 1 for drive B:, etc...                  */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 regs.h.ah = 0x19;
 int86(0x21, &regs, &regs);
 return ((short) regs.h.al);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short setdisk(short drive)
#else
short setdisk(drive)
short drive;
#endif
/***********************************************************************/
/* Function  : Change the current drive to that specified.             */
/* Parameters: 0 for drive A:, 2 for drive B:, etc...                  */
/* Return    : number of drives present                                */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 regs.h.ah = 0x0e;
 regs.h.dl = (char)drive;
 int86(0x21, &regs, &regs);
 return ((short) regs.h.al);
}
#endif

/***********************************************************************/
#ifdef HAVE_PROTO
LINE *getclipboard(LINE *now)
#else
LINE *getclipboard(now)
LINE *now;
#endif
/***********************************************************************/
/* Function  : Reads the contents of the clipboard into the file.      */
/* Parameters: pointer to line after which lines are to be added       */
/* Return    : 0 on success.                                           */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE *ptr=NULL;
 LINE *curr=now;
 LINETYPE i,line_start,numlines=0;
 LENGTHTYPE maxlen=0;
 long length=0;
 short rc=RC_OK;
#if defined(UNIX) || defined(MAC)
 int offset = 0;
#else
 int offset = 1;
#endif
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("nonansi.c: getclipboard");
#endif
#if defined(PDC_CLIP_SUCCESS) && !defined(DOS)
 rc = PDC_getclipboard( (DEFCHAR **)&ptr, &length );

 switch( rc )
 {
    case PDC_CLIP_MEMORY_ERROR:
       display_error(30,(CHARTYPE *)"",FALSE);
       rc = RC_OUT_OF_MEMORY;
       break;
    case PDC_CLIP_ACCESS_ERROR:
       display_error(186,(CHARTYPE *)"",FALSE);
       rc = RC_INVALID_ENVIRON;
       break;
    case PDC_CLIP_EMPTY:
       display_error(187,(CHARTYPE *)"",FALSE);
       rc = RC_INVALID_ENVIRON;
       break;
    default:
       rc = RC_OK;
       break;
 }
#else
 display_error(82,(CHARTYPE *)"CLIP:",FALSE);
#ifdef THE_TRACE
 trace_return();
#endif
 return NULL;
#endif
 if (rc != RC_OK)
 {
#ifdef THE_TRACE
    trace_return();
#endif
    return NULL;
 }

 line_start=0;

 for (i=0; i<length; i++)
 {
    if (*(ptr+i) == 0x0A) /* CR was prior character */
    {
       *(ptr+i-offset) = '\0';
       if ((curr = add_LINE(CURRENT_FILE->first_line,curr,ptr+line_start,i-line_start-offset,0,TRUE)) == NULL)
       {
          curr = NULL;
          break;
       }
       line_start = i+1;
       numlines++;
       if (i-line_start-offset > maxlen)
          maxlen = i-line_start-offset;
    }
 }
 if (line_start != i
 &&  curr)
 {
    curr = add_LINE(CURRENT_FILE->first_line,curr,ptr+line_start,i-line_start,0,TRUE);
    numlines++;
    if (i-line_start > maxlen)
       maxlen = i-line_start;
 }

 if (curr)
 {
    CURRENT_FILE->max_line_length = maxlen;
    CURRENT_FILE->number_lines += numlines;
 }
 free(ptr);
 return curr;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short setclipboard(FILE_DETAILS *cf,CHARTYPE *new_fname,bool force,LINETYPE in_lines,
                LINETYPE start_line,LINETYPE *num_file_lines,bool append,LENGTHTYPE start_col, LENGTHTYPE end_col,bool ignore_scope,bool lines_based_on_scope)
#else
short setclipboard(cf,new_fname,force,in_lines,start_line,num_file_lines,append,start_col,end_col,ignore_scope,lines_based_on_scope)
FILE_DETAILS *cf;
CHARTYPE *new_fname;
bool force,append;
LINETYPE in_lines,start_line;
LINETYPE *num_file_lines;
LENGTHTYPE start_col,end_col;
bool ignore_scope;
bool lines_based_on_scope;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 LINETYPE i=0L;
 LINETYPE abs_num_lines=(in_lines < 0L ? -in_lines : in_lines);
 LINETYPE num_actual_lines=0L;
 LINETYPE my_num_file_lines=0L;
 short direction=(in_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
 LINE *curr=NULL;
 short rc=RC_OK;
 bool save_scope_all=CURRENT_VIEW->scope_all;
#ifdef UNIX
 CHARTYPE *eol = (CHARTYPE *)"\n";
 int eol_len=1;
#elif defined(MAC)
 CHARTYPE *eol = (CHARTYPE *)"\r";
 int eol_len=1;
#else
 CHARTYPE *eol = (CHARTYPE *)"\r\n";
 int eol_len=2;
#endif
 long clip_size=1024;
 CHARTYPE *ptr=NULL;
 long len=0,pos=0;
/*--------------------------- processing ------------------------------*/
#ifdef THE_TRACE
 trace_function("file.c:    setclipboard");
#endif
/*---------------------------------------------------------------------*/
/* Determine where to start writing from in the linked list.           */
/*---------------------------------------------------------------------*/
 curr = lll_find(cf->first_line,cf->last_line,start_line,cf->number_lines);
/*---------------------------------------------------------------------*/
/* Save the setting of scope_all if we are ignoring scope. ie full file*/
/* is being written...                                                 */
/*---------------------------------------------------------------------*/
 if (ignore_scope)
    CURRENT_VIEW->scope_all = TRUE;
/*---------------------------------------------------------------------*/
/* Now write out the contents of the file array to the new filename.   */
/*---------------------------------------------------------------------*/
 ptr = (CHARTYPE *)malloc(clip_size);
 if (!ptr)
 {
    display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
    trace_return();
#endif
    return(RC_OUT_OF_MEMORY);
 }

 rc = RC_OK;
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
    rc = processable_line(CURRENT_VIEW,start_line+(LINETYPE)(i*direction),curr);
    switch(rc)
    {
       case LINE_SHADOW:
            rc = 0;
            break;
       case LINE_TOF:
       case LINE_EOF:
            rc = 0;
            num_actual_lines++;
            break;
       default:
            rc = 0;
            if (start_col < curr->length)
            {
               len = min(curr->length-start_col,(end_col - start_col) + 1);
               if ( pos + len > clip_size)
               {
                  ptr = (CHARTYPE *)realloc((DEFCHAR *)ptr, clip_size * 2);
                  clip_size *= 2;
               }
               memcpy((DEFCHAR *)ptr+pos,(DEFCHAR *)curr->line+start_col,len);
               /*
               if ((rc = write_line(curr->line+start_col,min(curr->length-start_col,(end_col - start_col) + 1),fp)) == RC_DISK_FULL)
                  break;
               */
               pos += len;
            }
            if ( pos + 1 + eol_len > clip_size)
            {
               ptr = (CHARTYPE *)realloc((DEFCHAR *)ptr, clip_size * 2);
               if (!ptr)
               {
                  display_error(30,(CHARTYPE *)"",FALSE);
#ifdef THE_TRACE
                  trace_return();
#endif
                  return(RC_OUT_OF_MEMORY);
               }
               clip_size *= 2;
            }
            memcpy((DEFCHAR *)ptr+pos,(DEFCHAR *)eol,eol_len);
            pos += eol_len;
            num_actual_lines++;
            break;
    }
/*---------------------------------------------------------------------*/
/* Proceed to the next record, even if the current record not in scope.*/
/*---------------------------------------------------------------------*/
    if (rc) 
       break;
    if (direction == DIRECTION_BACKWARD)
       curr = curr->prev;
    else
       curr = curr->next;
    my_num_file_lines += (LINETYPE)direction;
    if (curr == NULL)
       break;
 }
/*---------------------------------------------------------------------*/
/* Nul terminate the string.                                           */
/*---------------------------------------------------------------------*/
 *(ptr+pos) = '\0';
/*---------------------------------------------------------------------*/
/* Restore the setting of scope_all if we changed it before...         */
/*---------------------------------------------------------------------*/
 if (ignore_scope)
    CURRENT_VIEW->scope_all = save_scope_all;

#if defined(PDC_CLIP_SUCCESS) && !defined(DOS)
 rc = PDC_setclipboard( (DEFCHAR *)ptr, pos );

 switch( rc )
 {
    case PDC_CLIP_MEMORY_ERROR:
       display_error(30,(CHARTYPE *)"",FALSE);
       return(RC_OUT_OF_MEMORY);
       break;
    case PDC_CLIP_ACCESS_ERROR:
       display_error(186,(CHARTYPE *)"",FALSE);
       rc = RC_INVALID_ENVIRON;
       break;
    default:
       rc = RC_OK;
       break;
 }
#else
 display_error(82,(CHARTYPE *)"CLIP:",FALSE);
 rc = RC_INVALID_ENVIRON;
#endif

 free(ptr);
#ifdef THE_TRACE
 trace_return();
#endif
 return(rc);
}
