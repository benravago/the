// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* NOTE:
 *
 * Most of the time of the program is spend for displaying the screen.
 * The improvement of screen operations rely on the curses functionality and the used routines below.
 *
 * Therefore we use some ugly tricks to make the whole stuff as fast as possible.
 * The fastest routine may be waddchnstr and friends.
 *
 * This routine is only found in newer curses variants.
 *
 * To hold the source readable and to improve the speed under this conditions,
 * we use the global line buffers linebuf (char or unsigned char) and linebufch (chtype).
 *
 * Each buffer has max(COLS,THE_MAX_SCREEN_WIDTH)+1 elements.
 *
 * We use a module global loop variable which is reset by the INIT_LINE_OUTPUT macro.
 * Each line part may be added by the ADD_LINE_OUTPUT or FILE_LINE_OUTPUT macros.
 * These macros should be called with so many characters that a complete window line is EXACTLY filled in the sum.
 * Then we call END_LINE_OUTPUT to do the displaying.                                   =======
 *
 * The calling sequence for each line is:
 *   1.   INIT_LINE_OUTPUT
 *   2.   ADD_LINE_OUTPUT
 *  [3.   ADD_LINE_OUTPUT or FILL_LINE_OUTPUT
 *    ...
 *  [n.   ADD_LINE_OUTPUT or FILL_LINE_OUTPUT ]
 *   n+1. END_LINE_OUTPUT
 *
 * Be careful, no overflow checking of the buffers are performed.
 *
 * Pseudo prototypes of the macros:
 *   void INIT_LINE_OUTPUT(WINDOW *window,int line);
 *   void ADD_LINE_OUTPUT(uchar *string,int stringlength,chtype colour);
 *   void FILL_LINE_OUTPUT(uchar c,int filllength,chtype colour);
 *   void END_LINE_OUTPUT(void);
 */

#include "the.h"
#include "proto.h"

#include <time.h>

/*------------------------ function definitions -----------------------*/

static void build_lines(uchar, short, LINE *, short, short);
static void build_lines_for_display(uchar, short, short, short);
static void show_lines(uchar);
static void show_a_line(uchar, short, SHOW_LINE *);
static void set_prefix_contents(uchar, LINE *, short, long, bool);
static void show_hex_line(uchar, short);
static long displayed_max_line_length = 0;      /* max length of displayed line */
static LINE *hexshow_curr = NULL;       /* module global for historical reasons? */

/* Make a chtype value from a character and a colour.
 * This may be wrong if the format of the chtype is incompatible.
 * Please check this first if you get strange results with your curses implementation.
 */
#define make_chtype(ch,col) (etmode_flag[ch])?((chtype) etmode_table[ch]):(((chtype) etmode_table[ch]) | ((chtype) col))

#define mysetchar(dest, ch, colour ) { setcchar( dest, &ch, colour, 0, NULL ); }

/* Use fast line output routines.
 * We are faster with one simple function call in opposite of many wmove/wattrset/waddch calls.
 */
static int _fast_col;           /* Used elements of linebufch */
static WINDOW *_fast_win;       /* buffered for waddchnstr */

#define INIT_LINE_OUTPUT(win,line) {                  \
                        _fast_win = win;              \
                        _fast_col = 0;                \
                        wmove(_fast_win,line,0);      \
                        }

#define ADD_LINE_OUTPUT(line,length,colour) {                  \
                       long l = length;                        \
                       uchar *src;                             \
                       chtype color, /* beware of slow arg! */ \
                              *dest;                           \
                       dest = linebufch + _fast_col;           \
                       _fast_col += l;                         \
                       src = line;                             \
                       color = colour;                         \
                       while (l--) {                           \
                          *dest++ = make_chtype(*src,color);   \
                          src++;                               \
                       } }

#define ADD_SYNTAX_LINE_OUTPUT(line,length,highlight) {        \
                       long l = length;                        \
                       uchar *src;                             \
                       chtype *dest,*highl;                    \
                       dest = linebufch + _fast_col;           \
                       _fast_col += l;                         \
                       src = line;                             \
                       highl = highlight;                      \
                       while (l--) {                           \
                          *dest++ = make_chtype(*src,*highl);  \
                          src++;                               \
                          highl++;                             \
                       } }

#define FILL_LINE_OUTPUT(c,length,colour) {                      \
                        chtype *dest,C = make_chtype(c,colour);  \
                        long l = length;                         \
                        dest = linebufch + _fast_col;            \
                        _fast_col += l;                          \
                        while (l--)                              \
                           *dest++ = C;                          \
                        }

#define END_LINE_OUTPUT() { waddchnstr(_fast_win,                \
                                     linebufch,                  \
                                     _fast_col);                 \
                          }

#define REXX_INT_CHAR 'I'

/* small helper routines */

static void display_line_left(WINDOW *win, chtype colour, uchar *str, int lenstr, int line, int width) {
  int linelength;

  if ((linelength = lenstr) > width) {
    linelength = width;
  }
  INIT_LINE_OUTPUT(win, line);
  if (linelength) {
    ADD_LINE_OUTPUT(str, linelength, colour);
  }
  if ((linelength = width - linelength) != 0) {
    FILL_LINE_OUTPUT(' ', linelength, colour);
  }
  END_LINE_OUTPUT();
}

static void display_syntax_line_left(WINDOW *win, chtype colour, uchar *str, chtype *high, int line, int width) {
  int linelength;

  if ((linelength = strlen((char *) str)) > width) {
    linelength = width;
  }
  INIT_LINE_OUTPUT(win, line);
  if (linelength) {
    ADD_SYNTAX_LINE_OUTPUT(str, linelength, high);
  }
  if ((linelength = width - linelength) != 0) {
    FILL_LINE_OUTPUT(' ', linelength, colour);
  }
  END_LINE_OUTPUT();
}

static void display_line_center(WINDOW *win, chtype colour, uchar *str, int line, int width, int fillchar) {
  int linelength, first;

  if ((linelength = strlen((char *) str)) > width) {
    linelength = width;
  }
  first = (width - linelength) >> 1;
  INIT_LINE_OUTPUT(win, line);
  if (first) {
    FILL_LINE_OUTPUT(fillchar, first, colour);
  }
  if (linelength) {
    ADD_LINE_OUTPUT(str, linelength, colour);
  }
  if ((linelength = width - linelength - first) != 0) {
    FILL_LINE_OUTPUT(fillchar, linelength, colour);
  }
  END_LINE_OUTPUT();
}

/* real stuff */

void prepare_idline(uchar scrno) {
  short fpath_len = 0, max_name = 0;
  long x = 0;
  long line_number = 0L;
  uchar buffer[120];            /* should be large enough for very long values */
  uchar display_path[MAX_FILE_NAME + 1];
  uchar *fpath = display_path;
  short num_to_delete = 0, num_to_start = 0;
  VIEW_DETAILS *screen_view = SCREEN_VIEW(scrno);
  FILE_DETAILS *screen_file = SCREEN_FILE(scrno);
  int buflen;
  char *pos_string = NULL;

  /*
   * Determine content of window title. This can be display whether IDLINE is ON or OFF
   */
  if (strlen((char *) screen_file->display_name) > 0) {
    strcpy((char *) display_path, (char *) screen_file->display_name);
  } else {
    switch (screen_file->pseudo_file) {
      case PSEUDO_DIR:
        strcpy((char *) display_path, "DIR: ");
        strcat((char *) display_path, (char *) dir_path);
        strcat((char *) display_path, (char *) dir_files);
        break;
      case PSEUDO_REXX:
        strcpy((char *) display_path, "Output from: ");
        strcat((char *) display_path, (char *) rexx_macro_name);
        break;
      case PSEUDO_KEY:
        strcpy((char *) display_path, "Key definitions:");
        break;
      default:
        if (screen_file->display_actual_filename) {
          strcpy((char *) display_path, (char *) screen_file->fpath);
          strcat((char *) display_path, (char *) screen_file->fname);
        } else {
          strcpy((char *) display_path, (char *) screen_file->fname);
        }
        break;
    }
  }
  /*
   * Get line,col values only if POSITION is ON...
   */
  if (screen_view->position_status) {
    pos_string = get_current_position(scrno, &line_number, &x);
  }
  /*
   * Set up buffer for line,col,size and alt values for vertical screens.
   */
  if (display_screens != 1 && !horizontal) {
    if (screen_view->position_status) {
      switch (compatible_look) {
        case COMPAT_XEDIT:
          sprintf((char *) buffer, "S=%lu L=%lu C=%lu A=%u,%u", screen_file->number_lines, line_number, x, screen_file->autosave_alt, screen_file->save_alt);
          break;
        case COMPAT_ISPF:
          if (pos_string == NULL) {
            sprintf((char *) buffer, "S=%lu L=%lu C=%lu A=%u,%u", screen_file->number_lines, line_number, x, screen_file->autosave_alt, screen_file->save_alt);
          } else {
            sprintf((char *) buffer, "S=%lu L=%s C=%lu A=%u,%u", screen_file->number_lines, pos_string, x, screen_file->autosave_alt, screen_file->save_alt);
          }
          break;
        default:
          sprintf((char *) buffer, "L=%lu C=%lu S=%lu A=%u,%u", line_number, x, screen_file->number_lines, screen_file->autosave_alt, screen_file->save_alt);
          break;
      }
    } else {
      sprintf((char *) buffer, "S=%lu A=%u,%u", screen_file->number_lines, screen_file->autosave_alt, screen_file->save_alt);
    }
    max_name = max(0, (screen[scrno].screen_cols - 1) - strlen((char *) buffer));
  } else {
    if (screen_view->position_status) {
      switch (compatible_look) {
        case COMPAT_XEDIT:
          sprintf((char *) buffer, "Size=%-6lu Line=%-6lu Col=%-3lu Alt=%u,%u", screen_file->number_lines, line_number, x, screen_file->autosave_alt, screen_file->save_alt);
          break;
        case COMPAT_ISPF:
          if (pos_string == NULL) {
            sprintf((char *) buffer, "Size=%-6lu Line=%-6lu Col=%-3lu Alt=%u,%u", screen_file->number_lines, line_number, x, screen_file->autosave_alt, screen_file->save_alt);
          } else {
            sprintf((char *) buffer, "Size=%-6lu Line=%s Col=%-3lu Alt=%u,%u", screen_file->number_lines, pos_string, x, screen_file->autosave_alt, screen_file->save_alt);
          }
          break;
        default:
          sprintf((char *) buffer, "Line=%-6lu Col=%-4lu Size=%-5lu Alt=%u,%u", line_number, x, screen_file->number_lines, screen_file->autosave_alt, screen_file->save_alt);
          break;
      }
      max_name = max(0, (screen[scrno].screen_cols - 47));
    } else {
      if (compatible_look == COMPAT_XEDIT) { /* speed up */
        sprintf((char *) buffer, "Size=%-9lu%sAlt=%u,%u", screen_file->number_lines, "                  ", screen_file->autosave_alt, screen_file->save_alt);
        max_name = max(0, (screen[scrno].screen_cols - 47));
      } else {
        sprintf((char *) buffer, "Size=%-5lu Alt=%u,%u", screen_file->number_lines, screen_file->autosave_alt, screen_file->save_alt);
        max_name = max(0, (screen[scrno].screen_cols - 26));
      }
    }
  }
  /*
   * Determine which portion of filename can be displayed.
   */
  fpath = strtrans(display_path, ISLASH, ESLASH);
  fpath_len = strlen((char *) fpath);
  if (fpath_len > max_name) {
    num_to_delete = fpath_len - max_name + 2;
    num_to_start = max(0, (long) ((strlen((char *) fpath) / 2) - (num_to_delete / 2)));
    memcpy(linebuf, fpath, num_to_start);
    strcpy((char *) linebuf + num_to_start, "<>");
    strcat((char *) linebuf + num_to_start + 2, (char *) fpath + num_to_start + num_to_delete);
  } else {
    strcpy((char *) linebuf, (char *) fpath);
    memset(linebuf + fpath_len, ' ', max_name - fpath_len);
  }
  buflen = screen[scrno].screen_cols - max_name - 1;
  sprintf((char *) linebuf + max_name, " %-*.*s", buflen, buflen, buffer);
  return;
}

void show_heading(uchar scrno) {
  FILE_DETAILS *screen_file = SCREEN_FILE(scrno);
  WINDOW *screen_window_idline = SCREEN_WINDOW_IDLINE(scrno);

  prepare_idline(scrno);
  /* display the stuff */
  INIT_LINE_OUTPUT(screen_window_idline, 0);
  ADD_LINE_OUTPUT(linebuf, strlen((char *) linebuf), set_colour(screen_file->attr + ATTR_IDLINE));
  END_LINE_OUTPUT();

  wnoutrefresh(screen_window_idline);
  return;
}

void show_statarea(void) {
  short x = 0;
  int key = 0;
  time_t timer;
  struct tm *tblock = NULL;
  int length;
  char buffer[THE_MAX_SCREEN_WIDTH + 10];

  /*
   * If the status line is off, just exit...
   */
  if (STATUSLINEx == 'O' || !curses_started || CURRENT_VIEW == NULL) {
    return;
  }
  /*
   * If GUI option set for status line...
   */
  if (STATUSLINEx == 'G') {
    return;
  }
  /*
   * Display THE version.
   */
  sprintf((char *) linebuf, "THE.%-9s", the_version);
  memset((char *) linebuf + 10, ' ', max(0, COLS - 9));
  /*
   * Display number of files or copyright on startup.
   */
  if (initial) {
    strcpy((char *) buffer, "   ");
    strcat((char *) buffer, (char *) the_copyright);
  } else {
    /*
     * Display any pending prefix command warning
     */
    if (CURRENT_FILE->first_ppc != NULL && CURRENT_FILE->first_ppc->ppc_cmd_idx != (-1) && CURRENT_FILE->first_ppc->ppc_cmd_idx != (-2)) {
      sprintf(buffer, "'%s' pending...", get_prefix_command(CURRENT_FILE->first_ppc->ppc_cmd_idx));
    } else if (record_fp) {
      strcpy(buffer, (char *) record_status);
    } else {
      memset(buffer, ' ', min(COLS, sizeof(buffer)));
      format_options((uchar *) buffer);
    }
  }
  length = strlen(buffer);
  if (STATAREA_OFFSET + length < max(0, COLS - 27)) {
    memset((char *) linebuf + STATAREA_OFFSET + length, ' ', max(0, (COLS - 27) - STATAREA_OFFSET + length));
  }
  memcpy((char *) linebuf + STATAREA_OFFSET, buffer, length);
  /*
   * Display CLOCK.
   */
  if (CLOCKx) {
    timer = time(NULL);
    tblock = localtime(&timer);
    sprintf((char *) linebuf + max(0, (COLS - 27)), "%2d:%02d%s ", (tblock->tm_hour > 12) ? (tblock->tm_hour - 12) : (tblock->tm_hour), tblock->tm_min, (tblock->tm_hour >= 12) ? ("pm") : ("am"));
  } else {
    strcpy((char *) linebuf + max(0, (COLS - 27)), "        ");
  }
  /*
   * Display HEXDISPLAY.
   */
  if (HEXDISPLAYx) {
    x = getcurx(CURRENT_WINDOW);
    switch (CURRENT_VIEW->current_window) {
      case WINDOW_FILEAREA:
        key = (short) (*(rec + CURRENT_VIEW->verify_col - 1 + x) & A_CHARTEXT);
        break;
      case WINDOW_COMMAND:
        key = (short) (*(cmd_rec + x + cmd_verify_col - 1) & A_CHARTEXT);
        break;
      case WINDOW_PREFIX:
        key = (short) (*(pre_rec + x) & A_CHARTEXT);
        break;
    }
    {
      sprintf((char *) linebuf + max(0, (COLS - 19)), "'%c'=%02X/%03d  ", (unsigned char) ((key == 0) ? ' ' : key), key, key);
    }
  } else {
    strcpy((char *) linebuf + max(0, (COLS - 19)), "            ");
  }
  /*
   * Display colour setting.
   */
  linebuf[max(0, (COLS - 7))] = (colour_support) ? 'C' : 'c';
  /*
   * Display REXX support character.
   */
  linebuf[max(0, (COLS - 6))] = (rexx_support) ? REXX_INT_CHAR : ' ';
  /*
   * Display INSERTMODE toggle.
   */
  strcpy((char *) linebuf + max(0, (COLS - 5)), (INSERTMODEx) ? " Ins" : "    ");
  /*
   * Refresh the STATUS LINE.
   */
  INIT_LINE_OUTPUT(statarea, 0);
  ADD_LINE_OUTPUT(linebuf, strlen((char *) linebuf), set_colour(CURRENT_FILE->attr + ATTR_STATAREA));
  END_LINE_OUTPUT();
  wnoutrefresh(statarea);
  return;
}

void clear_statarea(void) {
  /*
   * If the status line is not displayed, don't do anything.
   */
  switch (STATUSLINEx) {
    case 'T':
    case 'B':
      INIT_LINE_OUTPUT(statarea, 0);
      FILL_LINE_OUTPUT(' ', COLS, (CURRENT_VIEW == NULL || CURRENT_FILE == NULL) ? A_NORMAL : set_colour(CURRENT_FILE->attr + ATTR_STATAREA));
      END_LINE_OUTPUT();
      break;
    default:
      break;
  }
  return;
}

void display_filetabs(VIEW_DETAILS *start) {
  VIEW_DETAILS *curr;
  FILE_DETAILS *first_view_file = NULL;
  bool process_view = FALSE;
  int j = 0;
  chtype normal, high;
  int fname_len, fill_len = COLS - 2, extras;
  bool first = TRUE, more = FALSE;

  /*
   * If filetabs is not displayed, don't do anything.
   * Also, if scale line is not on, we don't display filetabs
   */
  if (FILETABSx) {
    normal = set_colour(CURRENT_FILE->attr + ATTR_FILETABS);
    high = set_colour(CURRENT_FILE->attr + ATTR_FILETABSDIV);
    INIT_LINE_OUTPUT(filetabs, 0);
    if (start) {
      curr = start;
    } else {
      if (filetabs_start_view == NULL) {
        curr = vd_current;
      } else {
        curr = filetabs_start_view;
      }
    }
    if (number_of_files > 1) {
      for (j = 0; j < number_of_files;) {
        process_view = TRUE;
        if (curr && curr->file_for_view && curr->file_for_view->file_views > 1) {
          if (first_view_file == curr->file_for_view) {
            process_view = FALSE;
          } else {
            first_view_file = curr->file_for_view;
          }
        }
        if (process_view) {
          j++;
          if (curr != CURRENT_VIEW && curr->file_for_view) {
            fname_len = strlen((char *) curr->file_for_view->fname);
            if (fname_len + 2 + DEFAULT_FILETABS_GAP_WIDTH > fill_len) {
              more = TRUE;
              break;
            }
            if (first) {
              first = FALSE;
              extras = 0;
              filetabs_start_view = curr;
            } else {
              char buf[DEFAULT_FILETABS_GAP_WIDTH + 1];

              buf[0] = '|';
              buf[1] = '\0';
              ADD_LINE_OUTPUT((uchar *) buf, DEFAULT_FILETABS_GAP_WIDTH, high);
              extras = DEFAULT_FILETABS_GAP_WIDTH;
            }

            ADD_LINE_OUTPUT(curr->file_for_view->fname, fname_len, normal);
            fill_len = fill_len - fname_len - extras;
          }
        }
        curr = curr->next;
        if (curr == NULL) {
          curr = vd_first;
        }
      }
    }
    FILL_LINE_OUTPUT(' ', fill_len, normal);
    if (more) {
      ADD_LINE_OUTPUT((uchar *) "<>", 2, normal);
    } else {
      ADD_LINE_OUTPUT((uchar *) "  ", 2, normal);
    }
    END_LINE_OUTPUT();
    wnoutrefresh(filetabs);
  }
  return;
}

void redraw_window(WINDOW *win) {
  short i = 0, j = 0;
  chtype ch = 0;
  short y = 0, x = 0;

  getyx(win, y, x);
  for (i = 0; i < getmaxx(win); i++) {
    for (j = 0; j < getmaxy(win); j++) {
      wmove(win, j, i);
      ch = (chtype) (winch(win) & A_CHARTEXT);
      put_char(win, ch, ADDCHAR);
    }
  }
  wmove(win, y, x);
  return;
}

void build_screen(uchar scrno) {
  LINE *curr = NULL;
  LINE *save_curr = NULL;
  short crow = SCREEN_VIEW(scrno)->current_row;
  long cline = SCREEN_VIEW(scrno)->current_line;

  hexshow_curr = save_curr = curr = lll_find(SCREEN_FILE(scrno)->first_line, SCREEN_FILE(scrno)->last_line, cline, SCREEN_FILE(scrno)->number_lines);
  displayed_max_line_length = 0;
  /*
   * Build the file contents from the current line to the bottom of the window.
   */
  build_lines(scrno, DIRECTION_FORWARD, curr, (short) (screen[scrno].rows[WINDOW_FILEAREA] - crow), crow);
  /*
   * Build the file contents from the current line to the top of the window.
   */
  curr = save_curr->prev;
  build_lines(scrno, DIRECTION_BACKWARD, curr, crow, (short) (crow - 1));
  return;
}

void display_screen(uchar scrno) {
  unsigned short x = 0, y = 0;
  unsigned short savex = 0, savey = 0;
  short crow;

  /*
   * We don't display the screen if we are in a macro, running in batch, running REPEAT command, or curses hasn't started yet...
   */
  if (!interactive_in_macro) {
    if (batch_only || in_macro || !curses_started) {
      return;
    }
  }
  first_screen_display = TRUE;
  /*
   * Turn off the cursor.
   */
  draw_cursor(FALSE);
  /*
   * Display the IDLINE window...
   */
  show_heading(scrno);
  /*
   * Display the ARROW and CMDLINE if on...
   */
  if (SCREEN_WINDOW_ARROW(scrno) != NULL) {
    wattrset(SCREEN_WINDOW_ARROW(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_ARROW));
    redraw_window(SCREEN_WINDOW_ARROW(scrno));
    touchwin(SCREEN_WINDOW_ARROW(scrno));
    wnoutrefresh(SCREEN_WINDOW_ARROW(scrno));
  }
  if (SCREEN_WINDOW_COMMAND(scrno) != NULL) {
    wattrset(SCREEN_WINDOW_COMMAND(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_CMDLINE));
    redraw_window(SCREEN_WINDOW_COMMAND(scrno));
    touchwin(SCREEN_WINDOW_COMMAND(scrno));
    wnoutrefresh(SCREEN_WINDOW_COMMAND(scrno));
  }
  /*
   * Save the position of previous window if on command line.
   */
  if (SCREEN_VIEW(scrno)->current_window == WINDOW_COMMAND) {
    getyx(SCREEN_PREV_WINDOW(scrno), savey, savex);
  }
  getyx(SCREEN_WINDOW(scrno), y, x);
  /*
   * Display the built lines...
   */
  crow = SCREEN_VIEW(scrno)->current_row;
  build_lines_for_display(scrno, DIRECTION_FORWARD, (short) (screen[scrno].rows[WINDOW_FILEAREA] - crow), crow);
  build_lines_for_display(scrno, DIRECTION_BACKWARD, crow, (short) (crow - 1));
  /*
   * Check for nested comments if using a parser
   */
  if (SCREEN_FILE(scrno)->parser && SCREEN_FILE(scrno)->parser->have_paired_comments && SCREEN_VIEW(scrno)->syntax_headers & HEADER_COMMENT) {
    parse_paired_comments(scrno, SCREEN_FILE(scrno));
  }
  show_lines(scrno);
  /*
   * If we have the file tabs window show it
   */
  if (FILETABSx) {
    display_filetabs(NULL);
  }
  /*
   * Refresh the windows.
   */
  if (SCREEN_WINDOW_PREFIX(scrno) != NULL) {
    wnoutrefresh(SCREEN_WINDOW_PREFIX(scrno));
  }
  if (SCREEN_WINDOW_GAP(scrno) != NULL) {
    wnoutrefresh(SCREEN_WINDOW_GAP(scrno));
  }
  wnoutrefresh(SCREEN_WINDOW_FILEAREA(scrno));
  /*
   * Lastly, turn the cursor back on again.
   */
  draw_cursor(TRUE);
  /*
   * Restore the position of previous window if on command line.
   */
  if (SCREEN_VIEW(scrno)->current_window == WINDOW_COMMAND) {
    wmove(SCREEN_PREV_WINDOW(scrno), savey, savex);
  }
  wmove(SCREEN_WINDOW(scrno), y, x);
  return;
}

void display_cmdline(uchar curr_screen, VIEW_DETAILS *curr_view) {
  unsigned short x = 0, y = 0;

  if (batch_only || !curses_started) {
    return;
  }
  if (SCREEN_WINDOW_COMMAND(curr_screen) != NULL) {
    /*
     * Clear the cmdline from the beginning to the end
     * Display the contents of the cmdline from the cmd_verify_col
     */
    getyx(SCREEN_WINDOW_COMMAND(curr_screen), y, x);
    if (inDIALOG) {
      display_line_left(SCREEN_WINDOW_COMMAND(curr_screen), set_colour(curr_view->file_for_view->attr + ATTR_DIA_EDITFIELD), cmd_rec + cmd_verify_col - 1, cmd_rec_len, 0, screen[curr_screen].cols[WINDOW_COMMAND]);
    } else {
      display_line_left(SCREEN_WINDOW_COMMAND(curr_screen), set_colour(curr_view->file_for_view->attr + ATTR_CMDLINE), cmd_rec + cmd_verify_col - 1, cmd_rec_len, 0, screen[curr_screen].cols[WINDOW_COMMAND]);
    }
    wnoutrefresh(SCREEN_WINDOW_COMMAND(curr_screen));
    wmove(SCREEN_WINDOW_COMMAND(curr_screen), y, x);
  }
  return;
}

/* BE CAREFUL!
 * This function and his friend build_lines_for_display below should always be changed in conjunction. EVER!
 *
 * This function is been called by build_screen, an often called function.
 * Therefore needless things for RUNNING should be moved to build_screen_for_display which is called before a true display.
 * Put things there for DISPLAYING. ...for_display relies on informations computed here.
 */
static void build_lines(uchar scrno, short direction, LINE *curr, short rows, short start_row) {
  RESERVED *curr_rsrvd;
  long num_shadow_lines = 0;
  short tab_actual_row;
  short scale_actual_row;
  short hexshow_actual_start_row = 0;
  SHOW_LINE *scurr;
  VIEW_DETAILS *screen_view;
  long cline;
  FILE_DETAILS *screen_file;
  int display_rec, isTOForEOF, is_hexshow_on, has_reserveds, is_shadow, is_tab_on, is_scale_on;

  /*
   * These only need to be calculated once.
   */
  screen_view = SCREEN_VIEW(scrno);
  cline = screen_view->current_line;
  screen_file = SCREEN_FILE(scrno);
  is_hexshow_on = (screen_view->hexshow_on != FALSE);
  has_reserveds = (screen_file->first_reserved != NULL);
  is_shadow = (screen_view->shadow != FALSE);
  is_tab_on = (screen_view->tab_on != FALSE);
  is_scale_on = (screen_view->scale_on != FALSE);
  tab_actual_row = calculate_actual_row(screen_view->tab_base, screen_view->tab_off, screen[scrno].rows[WINDOW_FILEAREA], TRUE);
  scale_actual_row = calculate_actual_row(screen_view->scale_base, screen_view->scale_off, screen[scrno].rows[WINDOW_FILEAREA], TRUE);
  if (is_hexshow_on) {
    hexshow_actual_start_row = calculate_actual_row(screen_view->hexshow_base, screen_view->hexshow_off, screen[scrno].rows[WINDOW_FILEAREA], TRUE);
  }
  /*
   * Determine if the contents of "rec" should be used to display the focus line.
   */
  if (display_screens > 1) {
    if (scrno == current_screen || SCREEN_FILE(current_screen) == SCREEN_FILE(other_screen)) {
      display_rec = 1;
    } else {
      display_rec = 0;
    }
  } else {
    display_rec = 1;
  }
  /*
   * Determine the row that is the focus line.
   */
  if (direction == DIRECTION_BACKWARD) {
    cline--;
  }
  num_shadow_lines = 0;
  scurr = screen[scrno].sl + start_row;
  /*
   * Now, for each row to be displayed...
   */
  while (rows) {
    scurr->number_lines_excluded = 0;
    /*
     * If HEXSHOW is ON...
     */
    if (is_hexshow_on) {
      if (hexshow_actual_start_row == start_row || hexshow_actual_start_row + 1 == start_row) {
        scurr->line_type = LINE_HEXSHOW;
        scurr->line_number = (-1L);
        scurr->main_enterable = FALSE;
        scurr->prefix_enterable = FALSE;
        scurr->highlight = FALSE;
        if (scrno == current_screen) {
          if (screen_view->current_line == screen_view->focus_line) {
            scurr->contents = rec;
            scurr->length = rec_len;
          } else {
            scurr->contents = hexshow_curr->line;
            scurr->length = hexshow_curr->length;
          }
        } else {
          if (screen_view->current_line == SCREEN_VIEW(current_screen)->focus_line && display_rec) {
            scurr->contents = rec;
            scurr->length = rec_len;
          } else {
            scurr->contents = hexshow_curr->line;
            scurr->length = hexshow_curr->length;
          }
        }
        /* other_start_col is used to determine if upper or lower line.
         * Doing this here allows ignoring hexshow_actual_start_row later.
         */
        if (hexshow_actual_start_row == start_row) {
          scurr->other_start_col = 0;
        } else {
          scurr->other_start_col = 1;
        }
        start_row += direction;
        scurr += direction;
        rows--;
        continue;
      }
    }
    /*
     * If the current line is a reserved line...
     */
    if (has_reserveds) {        /* at least one reserved line */
      if ((curr_rsrvd = find_reserved_line(scrno, TRUE, start_row, 0, 0)) != NULL) {
        scurr->other_start_col = scurr->other_end_col = (long) -1;
        scurr->line_type = LINE_RESERVED;
        scurr->line_number = (-1L);
        scurr->current = (LINE *) NULL;
        scurr->main_enterable = FALSE;
        scurr->prefix_enterable = FALSE;
        scurr->highlight = FALSE;
        /* Save for later use, already correct if no prefix */
        scurr->contents = curr_rsrvd->disp;
        scurr->rsrvd = curr_rsrvd;
        scurr->length = curr_rsrvd->disp_length;
        scurr->normal_colour = set_colour(curr_rsrvd->attr);
        start_row += direction;
        scurr += direction;
        rows--;
        continue;
      }
    }
    /*
     * If the current line is the scale or tab line...
     */
    if ((is_scale_on && scale_actual_row == start_row) || (is_tab_on && tab_actual_row == start_row)) {
      scurr->contents = NULL;
      scurr->line_number = (-1L);
      scurr->current = (LINE *) NULL;
      if (compatible_feel == COMPAT_ISPF) {
        scurr->main_enterable = TRUE;
        scurr->prefix_enterable = TRUE;
      } else {
        scurr->main_enterable = FALSE;
        scurr->prefix_enterable = FALSE;
      }
      scurr->highlight = FALSE;
      scurr->line_type = LINE_LINE;
      if (is_tab_on && tab_actual_row == start_row) {
        scurr->line_type |= LINE_TABLINE;
      }
      if (is_scale_on && scale_actual_row == start_row) {
        scurr->line_type |= LINE_SCALE;
      }
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * If the current line is above or below TOF or EOF, set all to blank.
     */
    if (curr == NULL) {
      scurr->contents = NULL;
      scurr->line_type = (direction == DIRECTION_BACKWARD) ? LINE_OUT_OF_BOUNDS_ABOVE : LINE_OUT_OF_BOUNDS_BELOW;
      scurr->line_number = (-1L);
      scurr->current = (LINE *) NULL;
      scurr->main_enterable = FALSE;
      scurr->prefix_enterable = FALSE;
      scurr->highlight = FALSE;
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * If the current line is excluded, increment a running total.
     * Ignore the line if on TOF or BOF.
     */
    if (curr->next != NULL && curr->prev != NULL) { /* Bottom of file & Top of file */
      if (IN_SCOPE(screen_view, curr) || cline == screen_view->current_line) {
        isTOForEOF = 0;
      } else {
        if (num_shadow_lines == 0 && direction == DIRECTION_FORWARD) {
          scurr->line_number = cline;
          scurr->current = curr;
        }
        num_shadow_lines++;
        cline += (long) direction;
        isTOForEOF = 0;
        /* At this point we may reduce the runtime to about 60% if we use a fast loop.
         * This will prevent MUCH if any of the above ifs will produce more overhead, e.g. has_reserveds.
         * Keep in mind that none of the above ifs will ever do something useful at this point.
         * We do the necessary stuff only and for each direction.
         * This produces more code but it's REALLY worth.
         */
        if (direction == DIRECTION_FORWARD) {
          curr = curr->next;    /* belonging to above shadow */
          for (;;) {            /* like above useful checks */
            if (curr->next == NULL) {
              isTOForEOF = 1;
              break;
            }
            if (IN_SCOPE(screen_view, curr) || cline == screen_view->current_line) {
              break;
            }
            num_shadow_lines++;
            cline++;
            curr = curr->next;
          }
        } else {
          curr = curr->prev;    /* belonging to above shadow */
          for (;;) {            /* like above useful checks */
            if (curr->prev == NULL) {
              isTOForEOF = 1;
              break;
            }
            if (IN_SCOPE(screen_view, curr) || cline == screen_view->current_line) {
              break;
            }
            num_shadow_lines++;
            cline--;
            curr = curr->prev;
          }
        }
      }
    } else {
      isTOForEOF = 1;
    }
    /*
     * If we get here, we have to determine if a shadow line is to be displayed or not.
     */
    if (is_shadow && num_shadow_lines > 0) {
      scurr->length = 0;
      if (direction != DIRECTION_FORWARD) {
        scurr->line_number = cline + 1;
        scurr->current = curr;
      }
      scurr->main_enterable = TRUE;
      scurr->prefix_enterable = TRUE;
      scurr->highlight = FALSE;
      scurr->number_lines_excluded = num_shadow_lines;
      scurr->line_type = LINE_SHADOW;
      if (compatible_feel == COMPAT_XEDIT) {
        scurr->main_enterable = FALSE;
      }
      num_shadow_lines = 0;
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * The remainder is for lines that are to be displayed.
     */
    scurr->line_number = cline;
    scurr->current = curr;
    /*
     * If the current row to be displayed is the focus line,
     * display the working area, rec and rec_len instead of the entry in the LL.
     */
    if (scrno == current_screen) {
      if (cline == screen_view->focus_line && display_rec) {
        scurr->contents = rec;
        scurr->length = rec_len;
      } else {
        scurr->contents = curr->line;
        scurr->length = curr->length;
      }
    } else {
      if (cline == SCREEN_VIEW(current_screen)->focus_line && display_rec) {
        scurr->contents = rec;
        scurr->length = rec_len;
      } else {
        scurr->contents = curr->line;
        scurr->length = curr->length;
      }
    }
    /*
     * Determine if the length of this row is longer than our last saved longest line...
     */
    if (scurr->length > displayed_max_line_length) {
      displayed_max_line_length = scurr->length;
    }
    scurr->main_enterable = TRUE;
    scurr->prefix_enterable = TRUE;
    scurr->highlight = FALSE;
    /*
     * Set up TOF and EOF lines...
     */
    if (isTOForEOF) {
      if (compatible_feel == COMPAT_XEDIT) {
        scurr->main_enterable = FALSE;
      }
      scurr->line_type = (curr->next == NULL) ? LINE_EOF : LINE_TOF;    /* MH12 */
    } else {
      scurr->line_type = LINE_LINE;
      if (screen_view->highlight) {
        switch (screen_view->highlight) {
          case HIGHLIGHT_TAG:
            if (curr->flags.tag_flag) {
              scurr->highlight = TRUE;
            }
            break;
          case HIGHLIGHT_ALT:
            if (curr->flags.new_flag || curr->flags.changed_flag) {
              scurr->highlight = TRUE;
            }
            break;
          case HIGHLIGHT_SELECT:
            if (curr->select >= screen_view->highlight_low && curr->select <= screen_view->highlight_high) {
              scurr->highlight = TRUE;
            }
            break;
          default:
            break;
        }
      }
    }
    start_row += direction;
    scurr += direction;
    rows--;
    cline += (long) direction;
    if (direction == DIRECTION_FORWARD) {
      curr = curr->next;
    } else {
      curr = curr->prev;
    }
  }
  return;
}

/*
 * should always be changed in conjunction. EVER!
 *
 * This function is been called by display_screen only before a true screen update.
 * Therefore needless things for DISPLAYING should be placed here.
 * Put things needed for RUNNING must be placed in build_lines. This function
 * relies on informations computed in build_lines.
 */
static void build_lines_for_display(uchar scrno, short direction, short rows, short start_row) {
  int marked = 0, is_cursor_line, is_cursor_line_filearea_different;
  bool current;
  int widthnogap, gap, h, len;
  SHOW_LINE *scurr;
  VIEW_DETAILS *screen_view;
  FILE_DETAILS *screen_file;
  int is_prefix_on;
  long cline, off = 0;
  chtype attr_block, attr_cblock, attr_filearea, attr_gap, attr_highlight, attr_chighlight, attr_prefix, attr_shadow, attr_curline, attr_cursor;
  bool line_parseable = FALSE;
  long mark_start_line = 0L;
  long mark_end_line = 0L;
  long mark_start_col = 0;
  long mark_end_col = 0;

  /*
   * Determine the row that is the focus line.
   */
  scurr = screen[scrno].sl + start_row;
  /*
   * These only need to be calculated once.
   */
  screen_view = SCREEN_VIEW(scrno);
  screen_file = SCREEN_FILE(scrno);
  is_prefix_on = (screen_view->prefix != 0);
  /* commonly used attrs */
  attr_block = set_colour(screen_file->attr + ATTR_BLOCK);
  attr_cblock = set_colour(screen_file->attr + ATTR_CBLOCK);
  attr_filearea = set_colour(screen_file->attr + ATTR_FILEAREA);
  attr_gap = set_colour(screen_file->attr + ATTR_GAP);
  attr_highlight = set_colour(screen_file->attr + ATTR_HIGHLIGHT);
  attr_chighlight = set_colour(screen_file->attr + ATTR_CHIGHLIGHT);
  attr_prefix = set_colour(screen_file->attr + ATTR_PREFIX);
  attr_shadow = set_colour(screen_file->attr + ATTR_SHADOW);
  attr_curline = set_colour(screen_file->attr + ATTR_CURLINE);
  attr_cursor = set_colour(screen_file->attr + ATTR_CURSORLINE);
  gap = screen_view->prefix_gap;
  widthnogap = screen_view->prefix_width - gap;
  /*
   * Now, for each row to be displayed...
   */
  while (rows) {
    scurr->is_highlighting = FALSE;
    line_parseable = FALSE;
    scurr->is_current_line = FALSE;
    /*
     * Remove the highlight_type memory
     */
    if (scurr->highlight_type) {
      free(scurr->highlight_type);
      scurr->highlight_type = NULL;
    }
    /*
     * If this line is a hexshow line...
     */
    if (scurr->line_type == LINE_HEXSHOW) {
      scurr->normal_colour = attr_shadow;
      if (is_prefix_on) {
        scurr->prefix[0] = '\0';
        scurr->prefix_colour = attr_prefix;
        if (gap) {
          scurr->gap_colour = attr_gap;
          scurr->gap[0] = '\0';
        }
      }
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * If the current line is a reserved line...
     */
    if (scurr->line_type == LINE_RESERVED) {
      if (CTLCHARx) {
        scurr->is_highlighting = TRUE;
      }
      /*
       * If the reserved line is to scroll with the filearea contents
       */
      if (scurr->rsrvd->autoscroll) {
        if (is_prefix_on) {
          scurr->prefix_colour = scurr->gap_colour = scurr->normal_colour;
          scurr->prefix[0] = '\0';
          scurr->prefix_colour = attr_prefix;
          scurr->gap_colour = attr_gap;
          scurr->gap[0] = '\0';
        }
        /*
         * For autoscroll reserved lines, we use the reserved line "highlighting" structure member directly as it can be > THE_MAX_SCREEN_WIDTH character.
         * So we don't copy this data into the SHOW_LINE "highlighting" structure member
         */
      } else {
        if (is_prefix_on) {
          scurr->prefix_colour = scurr->gap_colour = scurr->normal_colour;
          len = scurr->length;
          if ((screen_view->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT) {
            /* fill prefix with reserved line contents */
            h = min(len, widthnogap);
            memcpy(scurr->prefix, scurr->contents, h);
            memcpy(scurr->prefix_highlighting, scurr->rsrvd->highlighting, h * sizeof(chtype));
            off = h;            /* off now points to highlighting for gap */
            scurr->prefix[h] = '\0';
            scurr->contents += h;
            len -= h;
            /* fill gap with reserved line contents */
            h = min(len, gap);
            memcpy(scurr->gap, scurr->contents, h);
            memcpy(scurr->gap_highlighting, scurr->rsrvd->highlighting + off, h * sizeof(chtype));
            off += h;           /* off now points to highlighting for filearea */
            scurr->gap[h] = '\0';
            /* remainer of line goes in filearea */
            if ((len -= h) == 0) {
              scurr->contents = NULL;
            } else {
              scurr->contents += h;
              memcpy(scurr->highlighting, scurr->rsrvd->highlighting + off, len * sizeof(chtype));
              scurr->length = len;
            }
          } else {              /* prefix on right */
            scurr->length = min(len, screen[scrno].cols[WINDOW_FILEAREA]);
            len -= scurr->length;
            if (gap) {
              h = min(len, gap);
              memcpy(scurr->gap, scurr->contents + scurr->length, h);
              memcpy(scurr->gap_highlighting, scurr->rsrvd->highlighting + scurr->length, h * sizeof(chtype));
              scurr->gap[h] = '\0';
            } else {
              h = 0;
            }
            /* now copy the rest to prefix if any */
            len = min(len, widthnogap);
            memcpy(scurr->prefix, scurr->contents + scurr->length + h, len);
            memcpy(scurr->prefix_highlighting, scurr->rsrvd->highlighting + scurr->length + h, len * sizeof(chtype));
            scurr->prefix[len] = '\0';
            memcpy(scurr->highlighting, scurr->rsrvd->highlighting, scurr->length * sizeof(chtype));
          }
        } else {
          memcpy(scurr->highlighting, scurr->rsrvd->highlighting, scurr->length * sizeof(chtype));
        }
      }
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * If the current line is the scale line...
     */
    if (scurr->line_type & LINE_SCALE) {
      if (is_prefix_on) {
        if (compatible_look == COMPAT_ISPF) {
          strcpy((char *) scurr->prefix, "=COLS>");
        } else {
          scurr->prefix[0] = '\0';
        }
        scurr->prefix_colour = attr_prefix;
        scurr->gap_colour = attr_gap;
        scurr->gap[0] = '\0';
      }
      scurr->normal_colour = set_colour(screen_file->attr + ATTR_SCALE);
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * If the current line is the tab line...
     */
    if (scurr->line_type & LINE_TABLINE) {
      if (is_prefix_on) {
        if (compatible_look == COMPAT_ISPF) {
          strcpy((char *) scurr->prefix, "=TABS>");
        } else {
          scurr->prefix[0] = '\0';
        }
        scurr->prefix_colour = attr_prefix;
        scurr->gap_colour = attr_gap;
        scurr->gap[0] = '\0';
      }
      scurr->normal_colour = set_colour(screen_file->attr + ATTR_TABLINE);
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * If this line is above TOF or below EOF...
     */
    if ((scurr->line_type == LINE_OUT_OF_BOUNDS_ABOVE) || (scurr->line_type == LINE_OUT_OF_BOUNDS_BELOW)) {
      scurr->length = 0;
      scurr->normal_colour = attr_filearea;
      if (is_prefix_on) {
        scurr->prefix[0] = '\0';
        scurr->prefix_colour = attr_prefix;
        scurr->gap_colour = attr_gap;
        scurr->gap[0] = '\0';
      }
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    cline = scurr->line_number;
    /*
     * If this line is a shadow line...
     */
    if (scurr->line_type == LINE_SHADOW) {
      if (direction == DIRECTION_FORWARD) {
        set_prefix_contents(scrno, scurr->current, start_row, cline, FALSE);
      } else {
        set_prefix_contents(scrno, scurr->current->next, start_row, cline, FALSE);
      }
      scurr->normal_colour = attr_shadow;
      start_row += direction;
      scurr += direction;
      rows--;
      continue;
    }
    /*
     * Determine if line being processed is the focus line.
     * Focus line determination out-ranks current line processing later.
     */
    if (cline == screen_view->focus_line) {
      is_cursor_line = 1;
      scurr->is_cursor_line = 1;
    } else {
      is_cursor_line = 0;
      scurr->is_cursor_line = 0;
    }
    if (attr_filearea != attr_cursor) {
      is_cursor_line_filearea_different = 1;
      scurr->is_cursor_line_filearea_different = 1;
    } else {
      is_cursor_line_filearea_different = 0;
      scurr->is_cursor_line_filearea_different = 0;
    }
    /*
     * Determine if line being processed is the current line.
     */
    if (cline == screen_view->current_line) {
      scurr->is_current_line = current = TRUE;
    } else {
      scurr->is_current_line = current = FALSE;
    }
    /*
     * Determine if line being processed is in a marked block.
     */
    if (MARK_VIEW != (VIEW_DETAILS *) NULL && MARK_VIEW == screen_view) {
      if (MARK_VIEW->mark_type == M_CUA) {
        if ((MARK_VIEW->mark_start_line * max_line_length) + MARK_VIEW->mark_start_col < (MARK_VIEW->mark_end_line * max_line_length) + MARK_VIEW->mark_end_col) {
          mark_start_line = MARK_VIEW->mark_start_line;
          mark_end_line = MARK_VIEW->mark_end_line;
          mark_start_col = MARK_VIEW->mark_start_col;
          mark_end_col = MARK_VIEW->mark_end_col;
        } else {
          mark_start_line = MARK_VIEW->mark_end_line;
          mark_end_line = MARK_VIEW->mark_start_line;
          mark_start_col = MARK_VIEW->mark_end_col;
          mark_end_col = MARK_VIEW->mark_start_col;
        }
        if (cline >= mark_start_line && cline <= mark_end_line) {
          marked = 1;
        } else {
          marked = 0;
        }
      } else {
        if (cline >= MARK_VIEW->mark_start_line && cline <= MARK_VIEW->mark_end_line) {
          marked = 1;
        } else {
          marked = 0;
        }
      }
    }
    set_prefix_contents(scrno, scurr->current, start_row, cline, current);
    /*
     * If this line is TOF or EOF...
     */
    if ((scurr->line_type == LINE_TOF) || (scurr->line_type == LINE_EOF)) {
      if (is_cursor_line && is_cursor_line_filearea_different) {
        scurr->normal_colour = attr_cursor;
      } else {
        scurr->normal_colour = (current) ? set_colour(screen_file->attr + ATTR_CTOFEOF) : set_colour(screen_file->attr + ATTR_TOFEOF);
      }
    } else {
      /*
       * We have a LINE_LINE, so allocate space for our highlight_type
       */
      scurr->highlight_type = (unsigned char *) malloc(scurr->length);
      if (scurr->highlight_type) {
        memset(scurr->highlight_type, THE_SYNTAX_NONE, scurr->length);
      }
      if (marked) {
        switch (MARK_VIEW->mark_type) {
          case M_LINE:
            scurr->normal_colour = (!current) ? attr_block : attr_cblock;
            break;
          case M_BOX:
          case M_COLUMN:
          case M_WORD:
            scurr->other_start_col = MARK_VIEW->mark_start_col - 1;
            scurr->other_end_col = MARK_VIEW->mark_end_col - 1;
            if (scurr->highlight) {
              scurr->normal_colour = (!current) ? attr_highlight : attr_chighlight;
            } else {
              if (is_cursor_line && is_cursor_line_filearea_different) {
                scurr->normal_colour = attr_cursor;
              } else {
                scurr->normal_colour = (!current) ? attr_filearea : attr_curline;
              }
            }
            scurr->other_colour = (!current) ? attr_block : attr_cblock;
            line_parseable = TRUE;
            break;
          case M_STREAM:
            if (scurr->highlight) {
              scurr->normal_colour = (!current) ? attr_highlight : attr_chighlight;
            } else {
              if (is_cursor_line && is_cursor_line_filearea_different) {
                scurr->normal_colour = attr_cursor;
              } else {
                scurr->normal_colour = (!current) ? attr_filearea : attr_curline;
              }
            }
            scurr->other_colour = (!current) ? attr_block : attr_cblock;
            scurr->other_end_col = MAX_INT;
            scurr->other_start_col = 0;
            if (cline == MARK_VIEW->mark_start_line) {
              scurr->other_start_col = MARK_VIEW->mark_start_col - 1;
            }
            if (cline == MARK_VIEW->mark_end_line) {
              scurr->other_end_col = MARK_VIEW->mark_end_col - 1;
            }
            if (cline > MARK_VIEW->mark_start_line && cline < MARK_VIEW->mark_end_line) {
              scurr->normal_colour = (!current) ? attr_block : attr_cblock;
            }
            /*
             * This can be more accurate. Only set true when the line
             * is fully marked.
             */
            line_parseable = TRUE;
            break;
          case M_CUA:
            if (scurr->highlight) {
              scurr->normal_colour = (!current) ? attr_highlight : attr_chighlight;
            } else {
              if (is_cursor_line && is_cursor_line_filearea_different) {
                scurr->normal_colour = attr_cursor;
              } else {
                scurr->normal_colour = (!current) ? attr_filearea : attr_curline;
              }
            }
            scurr->other_colour = (!current) ? attr_block : attr_cblock;
            scurr->other_end_col = MAX_INT;
            scurr->other_start_col = 0;

            if (cline == mark_start_line) {
              scurr->other_start_col = mark_start_col - 1;
            }
            if (cline == mark_end_line) {
              scurr->other_end_col = mark_end_col - 1;
            }
            if (cline > mark_start_line && cline < mark_end_line) {
              scurr->normal_colour = (!current) ? attr_block : attr_cblock;
            }
            /*
             * This can be more accurate. Only set true when the line
             * is fully marked.
             */
            line_parseable = TRUE;
            break;
        }
      } else {
        scurr->other_start_col = scurr->other_end_col = (long) -1;
        if (scurr->highlight) {
          scurr->normal_colour = (!current) ? attr_highlight : attr_chighlight;
          scurr->other_colour = scurr->normal_colour;
        } else {
          if (is_cursor_line && is_cursor_line_filearea_different) {
            scurr->normal_colour = attr_cursor;
          } else {
            scurr->normal_colour = (!current) ? attr_filearea : attr_curline;
          }
          scurr->other_colour = scurr->normal_colour;
        }
        line_parseable = TRUE;
      }
    }
    /*
     * If we are using colouring and we are not using the NULL parser and the line has been determined as parseable,
     * build the colours in the highlighting array based on the line's contents.
     */
    if (line_parseable && SCREEN_FILE(scrno)->colouring && SCREEN_FILE(scrno)->parser && scurr->length > 0) {
      parse_line(scrno, SCREEN_FILE(scrno), scurr, start_row);  /* test for error return */
      scurr->is_highlighting = TRUE;
    }
    start_row += direction;
    scurr += direction;
    rows--;
  }
  return;
}

static void show_lines(uchar scrno) {
  short i = 0;
  long j = 0;
  long true_col = 0;
  long off = 0, num_tens = 0;
  uchar tens[30];
  int gap = SCREEN_VIEW(scrno)->prefix_gap;
  int width = SCREEN_VIEW(scrno)->prefix_width - gap;
  long filearea_cols = screen[scrno].cols[WINDOW_FILEAREA];
  long max_cols = min(filearea_cols, SCREEN_VIEW(scrno)->verify_end - SCREEN_VIEW(scrno)->verify_start + 1);
  WINDOW *screen_window_filearea = SCREEN_WINDOW_FILEAREA(scrno);
  char buffer[60];
  uchar *ptr;
  SHOW_LINE *scurr = screen[scrno].sl;

  for (i = 0, scurr = screen[scrno].sl; i < screen[scrno].rows[WINDOW_FILEAREA]; i++, scurr++) {
    /*
     * Display the contents of the prefix area (if on).
     */
    if (SCREEN_VIEW(scrno)->prefix) {
      if (scurr->line_type == LINE_RESERVED) {
        display_syntax_line_left(SCREEN_WINDOW_PREFIX(scrno), scurr->prefix_colour, scurr->prefix, scurr->prefix_highlighting, i, width);
        if (SCREEN_VIEW(scrno)->prefix_gap) {
          display_syntax_line_left(SCREEN_WINDOW_GAP(scrno), scurr->gap_colour, scurr->gap, scurr->gap_highlighting, i, gap);
        }
      } else {
        /* not a reserved line */
        display_line_left(SCREEN_WINDOW_PREFIX(scrno), scurr->prefix_colour, scurr->prefix, strlen((char *) scurr->prefix), i, width);
        if (SCREEN_VIEW(scrno)->prefix_gap) {
          char tmp_gap[21];

          /* as this is NOT a reserved line, nothing is displayed in the gap */
          /* except for a LINE if required */
          /* no need to use UTF-8 length here */
          memset(tmp_gap, ' ', SCREEN_VIEW(scrno)->prefix_gap);
          tmp_gap[SCREEN_VIEW(scrno)->prefix_gap] = '\0';
          display_line_left(SCREEN_WINDOW_GAP(scrno), (SCREEN_VIEW(scrno)->prefix_gap_line ? scurr->gap_colour | A_ALTCHARSET | ACS_VLINE : scurr->gap_colour), (uchar *) tmp_gap, SCREEN_VIEW(scrno)->prefix_gap, i, gap);
        }
      }
    }
    /*
     * Display any shadow line.
     * No need to test to see if SHADOW is ON as number_excluded_lines would not be > 0 if SHADOW OFF.
     */
    if (scurr->number_lines_excluded > 0) {
      sprintf(buffer, " %lu", (unsigned long) scurr->number_lines_excluded);
      strcat(buffer + 2 /* at least */, " line(s) not displayed ");
      /* no need to use UTF-8 length here */
      display_line_center(screen_window_filearea, scurr->normal_colour, (uchar *) buffer, i, filearea_cols, '-');
      continue;
    }
    /*
     * Display SCALE and/or TABLINE...
     */
    if (scurr->line_type & LINE_SCALE || scurr->line_type & LINE_TABLINE) {
      true_col = SCREEN_VIEW(scrno)->verify_col - 1;
      for (j = 0, ptr = linebuf; j < max_cols; j++, ptr++, true_col++) {
        /*
         * Display '|' in current column position if this is the scale line.
         */
        if (scurr->line_type & LINE_SCALE && SCREEN_VIEW(scrno)->current_column == true_col + 1) {
          *ptr = '|';
          continue;
        }
        /*
         * Display 'T' in each tab column. This overrides all other characters except column position.
         */
        if (scurr->line_type & LINE_TABLINE) {
          if (is_tab_col(true_col + 1)) {
            *ptr = '|';
            continue;
          }
        }
        /*
         * Only display the following if it is a scale line...
         */
        if (scurr->line_type & LINE_SCALE) {
          if (SCREEN_VIEW(scrno)->margin_left - 1 == true_col) {
            *ptr = '[';
            continue;
          }
          if (SCREEN_VIEW(scrno)->margin_right - 1 == true_col) {
            *ptr = ']';
            continue;
          }
          if (SCREEN_VIEW(scrno)->margin_indent_offset_status && SCREEN_VIEW(scrno)->margin_left + SCREEN_VIEW(scrno)->margin_indent - 1 == true_col) {
            *ptr = 'p';
            continue;
          }
          if (!SCREEN_VIEW(scrno)->margin_indent_offset_status && SCREEN_VIEW(scrno)->margin_indent - 1 == true_col) {
            *ptr = 'p';
            continue;
          }
          if (SCREEN_VIEW(scrno)->zone_start - 1 == true_col) {
            *ptr = '<';
            continue;
          }
          if (SCREEN_VIEW(scrno)->zone_end - 1 == true_col) {
            *ptr = '>';
            continue;
          }
          if (true_col % 10 == 9) {
            /* this is still not perfect; see "marg 1 100" with "verify 40 110".
             * We should build the scale first.
             */
            sprintf((char *) tens, "%ld", (true_col / 10) + 1);
            num_tens = strlen((char *) tens);
            for (off = num_tens - 1; off >= 0; off--) {
              if (ptr - off < linebuf) {
                continue;
              }
              if ((off == 0) || (ptr[-off] == ' ') || (ptr[-off] == '.')) {
                ptr[-off] = tens[num_tens - off - 1];
              }
            }
            continue;
          }
          if (true_col % 5 == 4) {
            *ptr = '+';
            continue;
          }
          *ptr = '.';
        } else {                /* only TABLINE */
          *ptr = ' ';
        }
      }
      if (max_cols < filearea_cols) {
        memset(ptr, ' ', filearea_cols - max_cols);
      }
      INIT_LINE_OUTPUT(screen_window_filearea, i);
      /* no need to use UTF-8 length here */
      ADD_LINE_OUTPUT(linebuf, filearea_cols, scurr->normal_colour);
      END_LINE_OUTPUT();
      continue;
    }
    /*
     * Display HEXSHOW line...
     */
    if (scurr->line_type & LINE_HEXSHOW) {
      wmove(screen_window_filearea, i, 0);
      wattrset(screen_window_filearea, scurr->normal_colour);
      my_wclrtoeol(screen_window_filearea);
      show_hex_line(scrno, i);
      continue;
    }
    /*
     * Display TOF or EOF line.
     */
    if (scurr->line_type == LINE_TOF) {
      /* no need to use UTF-8 length here */
      display_line_left(screen_window_filearea, scurr->normal_colour, (SCREEN_VIEW(scrno)->tofeof) ? TOP_OF_FILE : (uchar *) "", (SCREEN_VIEW(scrno)->tofeof) ? strlen((char *) TOP_OF_FILE) : 0, i, filearea_cols);
      continue;
    }
    if (scurr->line_type == LINE_EOF) {
      /* no need to use UTF-8 length here */
      display_line_left(screen_window_filearea, scurr->normal_colour, (SCREEN_VIEW(scrno)->tofeof) ? BOTTOM_OF_FILE : (uchar *) "", (SCREEN_VIEW(scrno)->tofeof) ? strlen((char *) BOTTOM_OF_FILE) : 0, i, filearea_cols);
      continue;
    }
    /*
     * Display marked LINE block line(s).
     */
    show_a_line(scrno, i, scurr);
  }
  if (SCREEN_WINDOW_PREFIX(scrno) != NULL) {
    wattrset(SCREEN_WINDOW_PREFIX(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_PENDING));
  }
  if (SCREEN_WINDOW_GAP(scrno) != NULL) {
    wattrset(SCREEN_WINDOW_GAP(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_GAP));
  }
  wattrset(screen_window_filearea, set_colour(SCREEN_FILE(scrno)->attr + ATTR_FILEAREA));
  return;
}

#define TMP_EXTRA 1

static void show_a_line(uchar scrno, short row, SHOW_LINE *scurr) {
  long vend, vlen, blanks_after_length;
  long blength, clength;
  long bvcol, cvcol;
  ushort bcols, ccols;
  long bother_start_col, bother_end_col;
  long cother_start_col, cother_end_col;
  uchar *line;
  SHOW_LINE *current;
  int fillverify;
  chtype normal, other, *high;

  /*
   * If the line to be displayed is a reserved line, set the columns to be displayed so that the full line is displayed.
   */
  current = &(screen[scrno].sl[row]);
  line = current->contents;
  ccols = screen[scrno].cols[WINDOW_FILEAREA];  /* number of character columns displayed */
  normal = current->normal_colour;
  /*
   * If the RESERVED line is set to autoscroll, use the highlighting from the reserved line structure, not the highlighting in the SHOW_LINE structure.
   * The reserved line "highlighting" has the complete line, the SHOW_LINE "highlighting" only has up to THE_MAX_SCREEN_WIDTH characters
   */
  if (current->line_type == LINE_RESERVED && current->rsrvd->autoscroll) {
    high = current->rsrvd->highlighting + SCREEN_VIEW(scrno)->verify_col - 1;
  } else {
    high = current->highlighting;
  }
  blanks_after_length = 0;
  /*
   * If the contents are NULL then clear to eol in normal colour.
   */
  if (line == NULL) {
    INIT_LINE_OUTPUT(SCREEN_WINDOW_FILEAREA(scrno), row);
    FILL_LINE_OUTPUT(' ', ccols, normal);
    END_LINE_OUTPUT();
    return;
  }
  if (current->line_type == LINE_RESERVED && !current->rsrvd->autoscroll) {
    cvcol = 0;
    vend = ccols;
    vlen = ccols;
  } else {
    cvcol = SCREEN_VIEW(scrno)->verify_col - 1;
    vend = SCREEN_VIEW(scrno)->verify_end - 1;
    vlen = SCREEN_VIEW(scrno)->verify_end - SCREEN_VIEW(scrno)->verify_start + 1;
  }
  blength = current->length;    /* length of line in bytes; ie actual space used */
  clength = blength;
  bvcol = cvcol;
  other = current->other_colour;
  INIT_LINE_OUTPUT(SCREEN_WINDOW_FILEAREA(scrno), row);
  /*
   * If there is something past the VERIFY END column, fill it up with blanks in normal colour first and adjust cols.
   */
  if (ccols > vlen) {
    fillverify = ccols - vlen;
    ccols = vlen;
  } else {
    fillverify = 0;
  }
  cother_start_col = current->other_start_col;
  cother_end_col = current->other_end_col;
  line += bvcol;                /* line now points to the first character of the verify column */
  /* This optimization will only work if memset and memcpy are inline functions with fast assembler variants.
   * This is true in most cases with modern machines and compilers.
   */
  if (cvcol >= clength) {       /* line empty after cvcol? */
    memset(linebuf, ' ', ccols);
  } else if (cvcol + ccols <= clength) { /* line fills at least filearea? */
    bcols = ccols;
    /*
     * Copy to our working buffer from the first byte of the verify column; line points to that first byte
     */
    memcpy(linebuf, line, bcols + TMP_EXTRA);
  } else {                      /* line must be padded with blanks */
    blength -= bvcol;
    clength -= cvcol;
    memcpy(linebuf, line, blength);
    memset(linebuf + blength, ' ', ccols - clength + 1);
    blanks_after_length = ccols - clength;
  }
  if ((cvcol > cother_end_col) || (cvcol + ccols - 1 < cother_start_col)) {
    /*
     * To get here we are only displaying in one colour for the whole line. ie no box block, but it could be a line block.
     */
    if (current->is_highlighting && (!current->highlight)) {
      ADD_SYNTAX_LINE_OUTPUT(linebuf, ccols - blanks_after_length, high);
      FILL_LINE_OUTPUT(' ', blanks_after_length, normal);
    } else {
      ADD_LINE_OUTPUT(linebuf, ccols, normal);
    }
  } else {
    bother_start_col = cother_start_col;
    bother_end_col = cother_end_col;
    /* something must be displayed in another colour.
     * 1. display normal chars up to the start of other
     * 2. display another chars up to the end of other
     * 3. display normal chars until the end
     * first, setup stuff
     */
    cother_end_col -= cvcol;
    cother_end_col++;
    if (cother_end_col > ccols) {
      cother_end_col = ccols;
    }
    /* other_end_col normalized to cols */
    line = linebuf;         /* var line is unused now, will be incremented */
    /* other_start_col -= vcol; NOT allowed: vcol may be > other_start_col! */
    if (cvcol < cother_start_col) {
      /*
       * Display the columns BEFORE the marked block
       * We can do syntax highlighting here
       */
      cother_start_col -= cvcol;        /* don't do this above:
                                         * vcol may be > other_start_col! This leads
                                         * to an undetected underflow on unsigned's
                                         * which leads to a crash.
                                         */
      if (current->is_highlighting) {
        ADD_SYNTAX_LINE_OUTPUT(line, cother_start_col, high);
        high += cother_start_col;
      } else {
        ADD_LINE_OUTPUT(line, cother_start_col, normal);
      }
      ccols -= cother_start_col;
      cother_end_col -= cother_start_col;
      bother_start_col = cother_start_col;
      line += bother_start_col;
    }
    bother_end_col = cother_end_col;
    /*
     * Display the columns IN the the marked block
     * We DON'T do syntax highlighting here other_end_col always exists
     */
    ADD_LINE_OUTPUT(line, cother_end_col, other);
    ccols -= cother_end_col;
    line += bother_end_col;
    if (current->is_highlighting) {
      high += cother_end_col;
    }
    if (ccols) {
      /*
       * Display the columns AFTER the marked block
       * We can do syntax highlighting here
       */
      if (current->is_highlighting) {
        ADD_SYNTAX_LINE_OUTPUT(line, ccols, high);
      } else {
        ADD_LINE_OUTPUT(line, ccols, normal);
      }
    }
  }
  if (fillverify) {
    FILL_LINE_OUTPUT(' ', fillverify, normal);
  }
  /*
   * If THIGHLIGHT is on and active and the line contains the target, display it over the top of everything else.
   */
  if (SCREEN_VIEW(scrno)->thighlight_on && SCREEN_VIEW(scrno)->thighlight_active && SCREEN_VIEW(scrno)->thighlight_target.true_line == scurr->line_number) {
    int i, j, idx;

    other = set_colour(SCREEN_FILE(scrno)->attr + ATTR_THIGHLIGHT);
    for (i = 0; i < SCREEN_VIEW(scrno)->thighlight_target.num_targets; i++) {
      /*
       * Change the colour of the text if the rtarget was found and it wasn't a NOT boolean.
       */
      if (SCREEN_VIEW(scrno)->thighlight_target.rt[i].found & !SCREEN_VIEW(scrno)->thighlight_target.rt[i].not_target) {
        if (SCREEN_VIEW(scrno)->thighlight_target.rt[i].start < (bvcol + linebuf_size) && (SCREEN_VIEW(scrno)->thighlight_target.rt[i].start + SCREEN_VIEW(scrno)->thighlight_target.rt[i].found_length) > bvcol) {
          line = linebuf + SCREEN_VIEW(scrno)->thighlight_target.rt[i].start - bvcol;
          for (j = 0; j < SCREEN_VIEW(scrno)->thighlight_target.rt[i].found_length; j++, line++) {
            idx = SCREEN_VIEW(scrno)->thighlight_target.rt[i].start - cvcol + j;
            if (idx <= (vend - cvcol) && idx >= 0) {
              linebufch[idx] = make_chtype(*line, other);
            }
          }
        }
      }
    }
  }
  END_LINE_OUTPUT();
  return;
}

static void set_prefix_contents(uchar scrno, LINE *curr, short start_row, long cline, bool is_current) {
  uchar *ptr = NULL;
  VIEW_DETAILS *screen_view = SCREEN_VIEW(scrno);
  FILE_DETAILS *screen_file;
  SHOW_LINE *scurr;
  int width;

  if (screen_view->prefix) {
    screen_file = SCREEN_FILE(scrno);
    scurr = screen[scrno].sl + start_row;
    ptr = scurr->prefix;
    width = screen_view->prefix_width - screen_view->prefix_gap;
    if (curr->pre != NULL) {    /* prefix command pending... */
      // && !blank_field(curr->pre->ppc_command)) /* ... and not blank */
      strcpy((char *) ptr, (char *) curr->pre->ppc_orig_command);
      scurr->prefix_colour = set_colour(screen_file->attr + ATTR_PENDING);
    } else {                    /* no prefix command on this line */
      scurr->prefix_colour = (is_current) ? set_colour(screen_file->attr + ATTR_CPREFIX) : set_colour(screen_file->attr + ATTR_PREFIX);
      if (screen_view->number) {
        if ((screen_view->prefix & PREFIX_STATUS_MASK) == PREFIX_ON) {
          sprintf((char *) ptr, "%*.*ld", width, width, cline);
        } else {
          sprintf((char *) ptr, "%*ld", width, cline);
        }
      } else if ((screen_view->prefix & PREFIX_STATUS_MASK) == PREFIX_ON) {
        memset(ptr, '=', width);
        scurr->prefix[width] = '\0';
      } else {
        scurr->prefix[0] = '\0';
      }
    }
    /*
     * clear the gap
     */
    scurr->gap[0] = '\0';
    scurr->gap_colour = (is_current) ? set_colour(screen_file->attr + ATTR_CGAP) : set_colour(screen_file->attr + ATTR_GAP);
  }
  return;
}

static void show_hex_line(uchar scrno, short row) {
  short i = 0;
  long vcol = 0, vlen = 0;
  long length = 0;
  uchar *line = NULL, *lptr;
  int upper_nibble = screen[scrno].sl[row].other_start_col == 0;
  ushort cols;
  chtype normal;
  unsigned char c;
  SHOW_LINE *current;
  static char hexchars[] = "0123456789ABCDEF";

  /*
   * Set up columns to display...
   */
  vcol = SCREEN_VIEW(scrno)->verify_col - 1;
  vlen = SCREEN_VIEW(scrno)->verify_end - SCREEN_VIEW(scrno)->verify_start + 1;
  current = &(screen[scrno].sl[row]);
  length = current->length;
  line = current->contents;
  normal = current->normal_colour;
  cols = screen[scrno].cols[WINDOW_FILEAREA];
  /* adjust line and length to vcol */
  if (length < vcol) {
    length = 0;
  } else {
    length -= vcol;
    line += vcol;
  }
  /* don't display characters after VERIFY END or end of filearea */
  if (length > vlen) {
    length = vlen;
  }
  if (length > cols) {
    length = cols;
  }
  INIT_LINE_OUTPUT(SCREEN_WINDOW_FILEAREA(scrno), row);
  if (upper_nibble) {
    for (i = 0, lptr = linebuf; i < length; i++, line++) {
      *lptr++ = hexchars[(((unsigned) (*line)) >> 4) & 0x0F];
    }
  } else {
    for (i = 0, lptr = linebuf; i < length; i++) {
      *lptr++ = hexchars[*line++ & 0x0F];
    }
  }
  ADD_LINE_OUTPUT(linebuf, length, normal);
  if (length < cols) {
    c = ' ';
    if (upper_nibble) {
      c >>= 4;
    }
    FILL_LINE_OUTPUT((int) hexchars[(int)(c & 0x0f)], cols - length, normal);
  }
  END_LINE_OUTPUT();
  return;
}

void touch_screen(uchar scrno) {
  int i = 0;
  WINDOW *win;

  for (i = 0; i < VIEW_WINDOWS; i++) {
    win = screen[scrno].win[i];
    if (win != (WINDOW *) NULL) {
      touchwin(win);
    }
  }
  return;
}

void refresh_screen(uchar scrno) {
  /*
   * Turn off the cursor.
   */
  show_heading(scrno);
  if (SCREEN_WINDOW_FILEAREA(scrno) != SCREEN_WINDOW(scrno)) {
    wnoutrefresh(SCREEN_WINDOW_FILEAREA(scrno));
  }
  if (SCREEN_WINDOW_PREFIX(scrno) != (WINDOW *) NULL && SCREEN_WINDOW_PREFIX(scrno) != SCREEN_WINDOW(scrno)) {
    wnoutrefresh(SCREEN_WINDOW_PREFIX(scrno));
  }
  if (SCREEN_WINDOW_GAP(scrno) != (WINDOW *) NULL) {
    wnoutrefresh(SCREEN_WINDOW_GAP(scrno));
  }
  if (SCREEN_WINDOW_ARROW(scrno) != (WINDOW *) NULL) {
    touchwin(SCREEN_WINDOW_ARROW(scrno));
    wnoutrefresh(SCREEN_WINDOW_ARROW(scrno));
  }
  if (SCREEN_WINDOW_COMMAND(scrno) != (WINDOW *) NULL && SCREEN_WINDOW_COMMAND(scrno) != SCREEN_WINDOW(scrno)) {
    wnoutrefresh(SCREEN_WINDOW_COMMAND(scrno));
  }
  wnoutrefresh(SCREEN_WINDOW(scrno));
  /*
   * Turn on the cursor.
   */
  return;
}

void redraw_screen(uchar scrno) {
  if (curses_started) {
    if (SCREEN_WINDOW_COMMAND(scrno) != NULL) {
      wattrset(SCREEN_WINDOW_COMMAND(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_CMDLINE));
      touchwin(SCREEN_WINDOW_COMMAND(scrno));
      wnoutrefresh(SCREEN_WINDOW_COMMAND(scrno));
    }
    if (SCREEN_WINDOW_ARROW(scrno) != NULL) {
      wattrset(SCREEN_WINDOW_ARROW(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_ARROW));
      touchwin(SCREEN_WINDOW_ARROW(scrno));
      wnoutrefresh(SCREEN_WINDOW_ARROW(scrno));
    }
    if (SCREEN_WINDOW_IDLINE(scrno) != NULL) {
      wattrset(SCREEN_WINDOW_IDLINE(scrno), set_colour(SCREEN_FILE(scrno)->attr + ATTR_IDLINE));
      touchwin(SCREEN_WINDOW_IDLINE(scrno));
      wnoutrefresh(SCREEN_WINDOW_IDLINE(scrno));
    }
    if (SCREEN_WINDOW_PREFIX(scrno) != NULL) {
      touchwin(SCREEN_WINDOW_PREFIX(scrno));
      wnoutrefresh(SCREEN_WINDOW_PREFIX(scrno));
    }
    if (SCREEN_WINDOW_GAP(scrno) != NULL) {
      touchwin(SCREEN_WINDOW_GAP(scrno));
      wnoutrefresh(SCREEN_WINDOW_GAP(scrno));
    }
    touchwin(SCREEN_WINDOW_FILEAREA(scrno));
    wnoutrefresh(SCREEN_WINDOW_FILEAREA(scrno));
  }
  return;
}

bool line_in_view(uchar scrno, long line_number) {
  short i, max = screen[scrno].rows[WINDOW_FILEAREA];
  bool result = FALSE;
  SHOW_LINE *scurr;

  scurr = screen[scrno].sl;
  for (i = 0; i < max; i++, scurr++) {
    if (scurr->line_number == line_number) {
      result = TRUE;
      break;
    }
  }
  return (result);
}

bool column_in_view(uchar scrno, long column_number) {
  bool result = FALSE;
  long min_file_col = 0, max_file_col = 0;

  /*
   * This function is only valid in FILEAREA or CMDLINE
   */
  if (screen[scrno].screen_view->current_window == WINDOW_COMMAND) {
    min_file_col = cmd_verify_col - 1;
    max_file_col = min_file_col + screen[scrno].cols[WINDOW_COMMAND] - 1;
  } else {
    min_file_col = screen[scrno].screen_view->verify_col - 1;
    max_file_col = min_file_col + screen[scrno].cols[WINDOW_FILEAREA] - 1;
  }
  if (column_number >= min_file_col && column_number <= max_file_col) { /* new column in display */
    result = TRUE;
  }
  return (result);
}

long find_next_current_line(long num_pages, short direction) {
  short i = 0;
  long cline = CURRENT_VIEW->current_line;
  short rows = 0, num_display_lines = 0, num_shadow_lines = 0;
  LINE *curr = NULL;
  RESERVED *curr_reserved = CURRENT_FILE->first_reserved;
  short tab_actual_row = calculate_actual_row(CURRENT_VIEW->tab_base, CURRENT_VIEW->tab_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);
  short scale_actual_row = calculate_actual_row(CURRENT_VIEW->scale_base, CURRENT_VIEW->scale_off, CURRENT_SCREEN.rows[WINDOW_FILEAREA], TRUE);

  /*
   * Determine the number of file lines displayed...
   */
  num_display_lines = (CURRENT_SCREEN.rows[WINDOW_FILEAREA]) - 1;
  for (i = 0; curr_reserved != NULL; i++) {
    curr_reserved = curr_reserved->next;
  }
  num_display_lines -= i;
  if (CURRENT_VIEW->scale_on) {
    num_display_lines--;
  }
  if (CURRENT_VIEW->tab_on) {
    num_display_lines--;
  }
  if (CURRENT_VIEW->hexshow_on) {
    num_display_lines = num_display_lines - 2;
  }
  if (CURRENT_VIEW->scale_on && CURRENT_VIEW->tab_on && tab_actual_row == scale_actual_row) {
    num_display_lines++;
  }
  curr = lll_find(CURRENT_FILE->first_line, CURRENT_FILE->last_line, cline, CURRENT_FILE->number_lines);
  while (num_pages) {
    rows = num_display_lines;
    while (rows) {
      /*
       * If the current line is above or below TOF or EOF, set all to blank.
       */
      if (curr == NULL) {
        cline = (direction == DIRECTION_FORWARD) ? CURRENT_FILE->number_lines + 1L : 0L;
        num_pages = 1L;
        break;
      }
      /*
       * If the current line is excluded, increment a running total.
       * Ignore the line if on TOF or BOF.
       */
      if (curr->next != NULL && curr->prev != NULL) { /* Bottom of file & Top of file */
        if (!IN_SCOPE(CURRENT_VIEW, curr)) {
          num_shadow_lines++;
          cline += (long) direction;
          if (direction == DIRECTION_FORWARD) {
            curr = curr->next;
          } else {
            curr = curr->prev;
          }
          continue;
        }
      }
      /*
       * If we get here, we have to determine if a shadow line is to be displayed or not.
       */
      if (CURRENT_VIEW->shadow && num_shadow_lines > 0) {
        num_shadow_lines = 0;
        rows--;
        continue;
      }
      rows--;
      cline += (long) direction;
      if (direction == DIRECTION_FORWARD) {
        curr = curr->next;
      } else {
        curr = curr->prev;
      }
    }
    num_pages--;
  }
  if (direction == DIRECTION_FORWARD && cline > CURRENT_FILE->number_lines + 1L) {
    cline = CURRENT_FILE->number_lines + 1L;
  }
  if (direction == DIRECTION_BACKWARD && cline < 0L) {
    cline = 0L;
  }
  cline = find_next_in_scope(CURRENT_VIEW, (LINE *) NULL, cline, direction);
  return (cline);
}

short get_row_for_focus_line(uchar scrno, long fl, short cr) {
  short i = 0, max = screen[scrno].rows[WINDOW_FILEAREA];
  SHOW_LINE *scurr;

  /*
   * Return the row within the main window where the focus line is placed.
   * If the focus line is off the screen, or out of bounds of the current size of the file;
   * <0 or >number_lines, this returns the current row.
   */
  scurr = screen[scrno].sl;
  for (i = 0; i < max; i++, scurr++) {
    if (scurr->line_number == fl) {
      return (i);
    }
  }
  return (cr);
}

long get_focus_line_in_view(uchar scrno, long fl, ushort row) {
  ushort i, max = screen[scrno].rows[WINDOW_FILEAREA];
  SHOW_LINE *scurr;

  /*
   * Return a new focus line if the specified focus line is no longer in view,
   * or the same line number if that line is still in view.
   */
  scurr = screen[scrno].sl + row;
  for (i = row; i < max; i++, scurr++) {
    if (scurr->line_number != (-1L)) {
      return (scurr->line_number);
    }
  }
  scurr = screen[scrno].sl + row;
  for (i = row; i > 0; i--, scurr--) {
    if (scurr->line_number != (-1L)) {
      return (scurr->line_number);
    }
  }
  /*
   * We should never get here as there would be no editable lines in view
   */
  return (fl);
}

long calculate_focus_line(long fl, long cl) {
  long new_fl = (-1L);
  ushort i, max = CURRENT_SCREEN.rows[WINDOW_FILEAREA];
  SHOW_LINE *scurr;

  /*
   * Return the new focus line.
   * If the focus line is still in the window, it stays as is.
   * If not,the focus   line becomes the current line.
   */
  scurr = CURRENT_SCREEN.sl;
  for (i = 0; i < max; i++, scurr++) {
    if (scurr->line_number == fl && (scurr->line_type == LINE_LINE || scurr->line_type == LINE_TOF || scurr->line_type == LINE_EOF)) {
      new_fl = fl;
      break;
    }
  }
  if (new_fl == (-1L)) {
    new_fl = cl;
  }
  return (new_fl);
}

char *get_current_position(uchar scrno, long *line, long *col) {
  short y = 0, x = 0;
  char *ret = NULL;
  SHOW_LINE *scurr;

  if (curses_started) {
    getyx(SCREEN_WINDOW(scrno), y, x);
  }
  scurr = screen[scrno].sl + y;
  switch (SCREEN_VIEW(scrno)->current_window) {
    case WINDOW_COMMAND:
      *line = SCREEN_VIEW(scrno)->current_line;
      *col = (long) (x + 1 + cmd_verify_col - 1);
      break;
    case WINDOW_FILEAREA:
      *line = SCREEN_VIEW(scrno)->focus_line;
      *col = (long) x + SCREEN_VIEW(scrno)->verify_col;
      if (compatible_look == COMPAT_ISPF) {
        if (scurr->line_type & LINE_TABLINE) {
          ret = "TABS";
        } else if (scurr->line_type & LINE_SCALE) {
          ret = "COLS";
        } else if (scurr->line_type & LINE_BOUNDS) {
          ret = "BNDS";
        }
      }
      break;
    case WINDOW_PREFIX:
      *line = SCREEN_VIEW(scrno)->focus_line;
      *col = (long) (x + 1);
      if (compatible_look == COMPAT_ISPF) {
        if (scurr->line_type & LINE_TABLINE) {
          ret = "TABS";
        } else if (scurr->line_type & LINE_SCALE) {
          ret = "COLS";
        } else if (scurr->line_type & LINE_BOUNDS) {
          ret = "BNDS";
        }
      }
      break;
  }
  return ret;
}

void calculate_new_column(uchar curr_screen, VIEW_DETAILS *curr_view, ushort current_screen_col, long current_verify_col, long new_file_col, ushort *new_screen_col, long *new_verify_col) {
  long x = 0;

  if (column_in_view(curr_screen, new_file_col)) {
    *new_screen_col = (long) (new_file_col - (current_verify_col - 1));
    *new_verify_col = current_verify_col;
    return;
  }
  /*
   * To get here, we have new verify column...
   */
  x = screen[curr_screen].cols[curr_view->current_window] / 2;
  *new_verify_col = (long) max(1L, (long) new_file_col - x + 2L);
  *new_screen_col = (long) ((*new_verify_col == 1) ? new_file_col : x - 1);
  return;
}

short prepare_view(uchar scrn) {
  int y = 0, x = 0;
  VIEW_DETAILS *screen_view = SCREEN_VIEW(scrn);

  screen_view->current_row = calculate_actual_row(screen_view->current_base, screen_view->current_off, screen[scrn].rows[WINDOW_FILEAREA], TRUE);
  build_screen(scrn);
  if (!line_in_view(scrn, screen_view->focus_line)) {
    screen_view->focus_line = screen_view->current_line;
    pre_process_line(screen_view, screen_view->focus_line, (LINE *) NULL);
    build_screen(scrn);
  }
  if (curses_started) {
    getyx(SCREEN_WINDOW_FILEAREA(scrn), y, x);
    y = get_row_for_focus_line(scrn, screen_view->focus_line, screen_view->current_row);
    /* ensure column from WINDOW is in view */
    wmove(SCREEN_WINDOW_FILEAREA(scrn), y, x);
  }

  return (RC_OK);
}

short advance_view(VIEW_DETAILS *next_view, short direction) {
  VIEW_DETAILS *save_current_view = next_view;  /* point to passed view */
  uchar save_prefix = 0;
  ushort save_cmd_line = 0;
  short save_gap = 0, save_prefix_width = 0;
  bool save_id_line = 0;
  int y = 0, x = 0;
  short rc = RC_OK;

  /*
   * If this is the only file, ignore the command...
   */
  if (number_of_files < 2) {
    return (RC_OK);
  }
  /*
   * Reset the filetabs view
   */
  filetabs_start_view = NULL;
  /*
   * If we already have a current view, save some details of it...
   */
  if (CURRENT_VIEW) {
    save_prefix = CURRENT_VIEW->prefix;
    save_prefix_width = CURRENT_VIEW->prefix_width;
    save_gap = CURRENT_VIEW->prefix_gap;
    save_cmd_line = CURRENT_VIEW->cmd_line;
    save_id_line = CURRENT_VIEW->id_line;
  }
  memset(cmd_rec, ' ', max_line_length);
  cmd_rec_len = 0;
  /*
   * If we have not passed a "next" view determine what the next view will be...
   */
  if (!save_current_view) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    /*
     * Get a temporary pointer to the "next" view in the linked list.
     */
    if (direction == DIRECTION_FORWARD) {
      if (CURRENT_VIEW->next == (VIEW_DETAILS *) NULL) {
        save_current_view = vd_first;
      } else {
        save_current_view = CURRENT_VIEW->next;
      }
    } else {
      if (CURRENT_VIEW->prev == (VIEW_DETAILS *) NULL) {
        save_current_view = vd_last;
      } else {
        save_current_view = CURRENT_VIEW->prev;
      }
    }
  }
  /*
   * Save the position of the cursor for the current view before getting the contents of the new file...
   */
  if (curses_started) {
    if (CURRENT_WINDOW_COMMAND != NULL) {
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
      my_wclrtoeol(CURRENT_WINDOW_COMMAND);
    }
    getyx(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->y[WINDOW_FILEAREA], CURRENT_VIEW->x[WINDOW_FILEAREA]);
    if (CURRENT_WINDOW_PREFIX != NULL) {
      getyx(CURRENT_WINDOW_PREFIX, CURRENT_VIEW->y[WINDOW_PREFIX], CURRENT_VIEW->x[WINDOW_PREFIX]);
    }
  }
  /*
   * If more than one screen is displayed and the file displayed in each screen is the same,
   * remove the 'current' view from the linked list, making the next view the current one.
   * Only do this is the "next" view is not the view in the other screen.
   */
  if (display_screens > 1) {
    if (CURRENT_SCREEN.screen_view->file_for_view == OTHER_SCREEN.screen_view->file_for_view) {
      if (CURRENT_VIEW->file_for_view == save_current_view->file_for_view) {
        if (direction == DIRECTION_FORWARD) {
          if (save_current_view->next == (VIEW_DETAILS *) NULL) {
            save_current_view = vd_first;
          } else {
            save_current_view = save_current_view->next;
          }
        } else {
          if (save_current_view->prev == (VIEW_DETAILS *) NULL) {
            save_current_view = vd_last;
          } else {
            save_current_view = save_current_view->prev;
          }
        }
      }
      free_a_view();
      CURRENT_VIEW = CURRENT_SCREEN.screen_view = save_current_view;
      OTHER_FILE->file_views--;
    } else {
      /*
       * First check if the file in the next view is the same as the file being displayed in the other screen...
       */
      if (save_current_view->file_for_view == OTHER_FILE) {
        CURRENT_VIEW = CURRENT_SCREEN.screen_view = save_current_view;
        if ((rc = defaults_for_other_files(OTHER_VIEW)) != RC_OK) {
          return (rc);
        }
        CURRENT_SCREEN.screen_view = CURRENT_VIEW;
        CURRENT_FILE = CURRENT_SCREEN.screen_view->file_for_view = OTHER_FILE;
        CURRENT_FILE->file_views++;
      } else {
        CURRENT_VIEW = CURRENT_SCREEN.screen_view = save_current_view;
      }
    }
  } else {                      /* only one screen being displayed...less hassle */
    /*
     * Make the current view the "next" one determined above.
     */
    CURRENT_VIEW = CURRENT_SCREEN.screen_view = save_current_view;
  }
  /*
   * If the position of the prefix or command line for the new view is different from the previous view, rebuild the windows...
   */
  if ((save_prefix & PREFIX_LOCATION_MASK) != (CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) || save_gap != CURRENT_VIEW->prefix_gap || save_prefix_width != CURRENT_VIEW->prefix_width || save_cmd_line != CURRENT_VIEW->cmd_line || save_id_line != CURRENT_VIEW->id_line) {
    set_screen_defaults();
    if (curses_started) {
      if (set_up_windows(current_screen) != RC_OK) {
        return (RC_OK);
      }
    }
  }
  /*
   * Re-calculate CURLINE for the new view in case the CURLINE is no longer in the display area.
   */
  prepare_view(current_screen);
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  display_screen(current_screen);
  if (curses_started) {
    if (statarea != NULL) {
      wattrset(statarea, set_colour(CURRENT_FILE->attr + ATTR_STATAREA));
      redraw_window(statarea);
      touchwin(statarea);
    }
    if (divider != NULL) {
      if (display_screens > 1 && !horizontal) {
        wattrset(divider, set_colour(CURRENT_FILE->attr + ATTR_DIVIDER));
      }
      touchwin(divider);
      wnoutrefresh(divider);
    }
    wmove(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->y[WINDOW_FILEAREA], CURRENT_VIEW->x[WINDOW_FILEAREA]);
    if (CURRENT_WINDOW_PREFIX != NULL) {
      wmove(CURRENT_WINDOW_PREFIX, CURRENT_VIEW->y[WINDOW_PREFIX], CURRENT_VIEW->x[WINDOW_PREFIX]);
    }
    getyx(CURRENT_WINDOW, y, x);
    wmove(CURRENT_WINDOW, y, x);
  }
  return (RC_OK);
}

short THE_Resize(int rows, int cols) {
  short i = 0;
  int length;
  int rc = RC_OK;

  /*
   * This function is called as the result of a screen resize.
   */
  if (rows && cols) {
    resizeterm(rows, cols);
  }
  endwin();
  doupdate();                   /* make ncurses set LINES and COLS properly */
  wnoutrefresh(stdscr);
  ncurses_screen_resized = FALSE;
  terminal_lines = LINES;
  terminal_cols = COLS;
  length = COLS + 1;
  if (length > linebuf_size) {
    /* only resize linebuf and linebufch if the new terminal width is > the current size */
    linebuf_size = length;
    if ((linebuf = (uchar *) realloc(linebuf, linebuf_size)) == NULL) {
      cleanup();
      return (30);
    }
    if ((linebufch = (chtype *) realloc(linebufch, (linebuf_size * sizeof(chtype)))) == NULL) {
      cleanup();
      return (30);
    }
  }
  if (screen_rows[0] != 0) {
    int offset = (STATUSLINEON())? 1 : 0;

    /*
     * 2 screens are displayed with different sizes.
     * Attempt to maintain the same ratio between the two.
     */
    screen_rows[0] = ((terminal_lines - offset) * screen_rows[0]) / (screen_rows[0] + screen_rows[1]);
    screen_rows[1] = (terminal_lines - offset) - screen_rows[0];
  }
  if (screen_cols[0] != 0) {
    /*
     * 2 screens are displayed with different sizes.
     * Attempt to maintain the same ratio between the two.
     */
    screen_cols[0] = (terminal_cols * screen_cols[0]) / (screen_cols[0] + screen_cols[1]);
    screen_cols[1] = terminal_cols - screen_cols[0];
  }
  set_screen_defaults();
  if (curses_started) {
    for (i = 0; i < display_screens; i++) {
      if ((rc = set_up_windows(i)) != RC_OK) {
        return (rc);
      }
    }
  }
  create_statusline_window();
  create_filetabs_window();
  return (RC_OK);
}

