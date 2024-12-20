// SPDX-FileCopyrightText: 2013 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include "the.h"
#include "proto.h"

bool prefix_changed = FALSE;

void editor(void) {
  short y = 0, x = 0;

  /*
   * Reset any command line positioning parameters
   * so only those files edited from the command line
   * take on the line/col, command line position.
   */
  startup_line = startup_column = 0;
  if (display_screens > 1) {
    display_screen((uchar) (other_screen));
  }
  getyx(CURRENT_WINDOW, y, x);
  wmove(CURRENT_WINDOW, y, x);
  wrefresh(CURRENT_WINDOW);
  if (error_on_screen) {
    if (error_window != NULL) {
      touchwin(error_window);
      wrefresh(error_window);
      wmove(CURRENT_WINDOW, y, x);
      wrefresh(CURRENT_WINDOW);
    }
  }
  for (;;) {
    if (process_key(-1, FALSE) != RC_OK) {
      break;
    }
  }
  return;
}

int process_key(int key, bool mouse_details_present) {
  unsigned short x = 0, y = 0;
  short rc = RC_OK;
  uchar string_key[2];

  string_key[1] = '\0';
  if (is_termresized()) {
    (void) THE_Resize(0, 0);
    (void) THERefresh((uchar *) "");
  }
  if (single_instance_server) {
    key = process_fifo_input(key);
  }
  if (key == (-1)) {
    key = my_getch(CURRENT_WINDOW);
  }
  if (key != KEY_MOUSE) {
    if (!mouse_details_present) {
      reset_saved_mouse_pos();
    }
  }
  if (is_termresized()) {
    return (RC_OK);
  }
  if (key == KEY_RESIZE) {
    (void) THE_Resize(0, 0);
    (void) THERefresh((uchar *) "");
    return (RC_OK);
  }
  if (key == -1) {
    return (RC_OK);
  }
  initial = FALSE;              /* set first time a key is requested */
  if (error_on_screen) {
    clear_msgline(key);
  }
  if (current_key == -1) {
    current_key = 0;
  } else {
    if (current_key == 7) {
      current_key = 0;
    } else {
      current_key++;
    }
  }
  /*
   * Save details about the last key pressed
   */
  lastkeys[current_key] = key;
  if (key == KEY_MOUSE) {
    lastkeys_is_mouse[current_key] = 1;
  } else {
    lastkeys_is_mouse[current_key] = 0;
  }
  /*
   * If we are recording a macro, check if the key hit is the end-of-record
   * key.
   */
  if (record_fp) {
    if (key == record_key) {
      char ctime_buf[26];
      time_t now;

      /*
       * Write a comment at the bottom
       */
      now = time(NULL);
      strcpy(ctime_buf, ctime(&now));
      ctime_buf[24] = '\0';
      fprintf(record_fp, "/* Recording of macro ended %s */\n", ctime_buf);
      fclose(record_fp);
      free(record_status);
      record_fp = NULL;
      record_status = NULL;
      /*
       * Refresh the status area to reflect we are no longer recording
       */
      show_statarea();
      getyx(CURRENT_WINDOW, y, x);
      wmove(CURRENT_WINDOW, y, x);
      wrefresh(CURRENT_WINDOW);
      return (RC_OK);
    }
    /*
     * If we are recording a macro, write the key definintion here
     */
    write_macro(get_key_definition(key, THE_KEY_DEFINE_RAW, TRUE, (bool) ((key == KEY_MOUSE) ? TRUE : FALSE)));
  }
  save_for_repeat = 0;
  rc = function_key(key, OPTION_NORMAL, mouse_details_present);
  save_for_repeat = 1;
  if (number_of_files == 0) {
    return (RC_INVALID_ENVIRON);
  }
  if (rc >= RAW_KEY) {
    if (rc > RAW_KEY) {
      key = rc - (RAW_KEY * 2);
    }
    if (key < 256 && key >= 0) {
      string_key[0] = (uchar) key;
      /*
       * If operating in CUA mode, and a CUA block exists,
       * check if the block should be reset or deleted before executing the command.
       */
      if (INTERFACEx == INTERFACE_CUA && CURRENT_VIEW->mark_type == M_CUA) {
        ResetOrDeleteCUABlock(CUA_DELETE_BLOCK);
      }
      (void) Text(string_key);
    }
  }
  show_statarea();
  if (display_screens > 1 && SCREEN_FILE(0) == SCREEN_FILE(1)) {
    build_screen((uchar) (other_screen));
    display_screen((uchar) (other_screen));
    /*    refresh_screen(other_screen);*/
    show_heading((uchar) (other_screen));
  }
  refresh_screen(current_screen);
  if (error_on_screen) {
    getyx(CURRENT_WINDOW, y, x);
    if (error_window != NULL) {
      touchwin(error_window);
      wnoutrefresh(error_window);
    }
    wmove(CURRENT_WINDOW, y, x);
    wnoutrefresh(CURRENT_WINDOW);
  }
  doupdate();
  return (RC_OK);
}

short EditFile(uchar *fn, bool external_command_line) {
  short rc = RC_OK, y = 0, x = 0;
  VIEW_DETAILS *save_current_view = NULL;
  VIEW_DETAILS *previous_current_view = NULL;
  uchar save_prefix = 0;
  short save_gap = 0;
  ushort save_cmd_line = 0;
  bool save_id_line = 0;

  /*
   * With no arguments, edit the next file in the ring...
   */
  if (strcmp((char *) fn, "") == 0) {
    rc = advance_view(NULL, DIRECTION_FORWARD);
    return (rc);
  }
  /*
   * With "-" as argument, edit the previous file in the ring...
   */
  if (strcmp((char *) fn, "-") == 0) {
    rc = advance_view(NULL, DIRECTION_BACKWARD);
    return (rc);
  }
  /*
   * If there are still file(s) in the ring,
   * clear the command line and save any changes to the focus line.
   */
  if (number_of_files > 0) {
    post_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL, TRUE);
    memset(cmd_rec, ' ', max_line_length);
    cmd_rec_len = 0;
  }
  previous_current_view = CURRENT_VIEW;
  /*
   * Save the position of the cursor for the current view
   * before getting the contents of the new file...
   */
  if (curses_started && number_of_files > 0) {
    if (CURRENT_WINDOW_COMMAND != NULL) {
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
      my_wclrtoeol(CURRENT_WINDOW_COMMAND);
    }
    getyx(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->y[WINDOW_FILEAREA], CURRENT_VIEW->x[WINDOW_FILEAREA]);
    if (CURRENT_WINDOW_PREFIX != NULL) {
      getyx(CURRENT_WINDOW_PREFIX, CURRENT_VIEW->y[WINDOW_PREFIX], CURRENT_VIEW->x[WINDOW_PREFIX]);
    }
  }
  if (number_of_files > 0) {
    save_prefix = CURRENT_VIEW->prefix;
    save_gap = CURRENT_VIEW->prefix_gap;
    save_cmd_line = CURRENT_VIEW->cmd_line;
    save_id_line = CURRENT_VIEW->id_line;
  }
  /*
   * Read the contents of the new file into memory...
   */
  if ((rc = get_file(strrmdup(strtrans(fn, OSLASH, ISLASH), ISLASH, TRUE))) != RC_OK) {
    return (rc);
  }
  /*
   * If more than one screen is displayed, sort out which view is to be displayed...
   */
  if (display_screens > 1) {
    save_current_view = CURRENT_VIEW;
    CURRENT_SCREEN.screen_view = CURRENT_VIEW = previous_current_view;
    advance_view(save_current_view, DIRECTION_FORWARD);
  } else {
    if (number_of_files > 0) {
      /*
       * If the position of the prefix or
       * command line for the new view is different from the previous view,
       * rebuild the windows...
       */
      if ((save_prefix & PREFIX_LOCATION_MASK) != (CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) || save_gap != CURRENT_VIEW->prefix_gap || save_cmd_line != CURRENT_VIEW->cmd_line || save_id_line != CURRENT_VIEW->id_line) {
        set_screen_defaults();
        if (curses_started) {
          if (set_up_windows(current_screen) != RC_OK) {
            return (RC_OK);
          }
        }
      }
    }
    /*
     * Re-calculate CURLINE for the new view in case the CURLINE is no longer in the display area.
     */
    prepare_view(current_screen);
  }
  pre_process_line(CURRENT_VIEW, CURRENT_VIEW->focus_line, (LINE *) NULL);
  build_screen(current_screen);
  /*
   * Position the cursor in the main window depending on the type of file
   */
  if (curses_started) {
    if (CURRENT_VIEW->in_ring) {
      wmove(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->y[WINDOW_FILEAREA], CURRENT_VIEW->x[WINDOW_FILEAREA]);
      if (CURRENT_WINDOW_PREFIX != NULL) {
        wmove(CURRENT_WINDOW_PREFIX, CURRENT_VIEW->y[WINDOW_PREFIX], CURRENT_VIEW->x[WINDOW_PREFIX]);
      }
      getyx(CURRENT_WINDOW, y, x);
      wmove(CURRENT_WINDOW, y, x);
    } else {
      if (CURRENT_FILE->pseudo_file == PSEUDO_DIR) {
        wmove(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->current_row, FILE_START - 1);
      } else {
        wmove(CURRENT_WINDOW_FILEAREA, CURRENT_VIEW->current_row, 0);
      }
    }
  }
  /*
   * Execute any profile file...
   */
  if ((REPROFILEx && CURRENT_VIEW->in_ring == FALSE) || (in_profile && external_command_line)) {
    profile_file_executions++;
    in_reprofile = TRUE;
    if (execute_profile) {
      if (local_prf != (uchar *) NULL) {
        rc = get_profile(local_prf, prf_arg);
      }
    }
    in_reprofile = FALSE;
  }
  /*
   * If the result of processing the profile file results in no files in the ring, we need to get out NOW.
   */
  if (number_of_files == 0) {
    return (rc);
  }
  /* pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);*/
  build_screen(current_screen);
  /*
   * If startup values were specified on the command, line, move cursor there...
   */
  if (startup_line != 0 || startup_column != 0) {
    THEcursor_goto(startup_line, startup_column);
  }
  filetabs_start_view = NULL;
  /*
   * If curses hasn't started, don't try to use curses functions...
   */
  if (curses_started) {
    display_screen(current_screen);
    if (CURRENT_WINDOW_COMMAND != NULL) {
      wmove(CURRENT_WINDOW_COMMAND, 0, 0);
    }
    if (CURRENT_WINDOW_PREFIX != NULL) {
      touchwin(CURRENT_WINDOW_PREFIX);
    }
    if (CURRENT_WINDOW_GAP != NULL) {
      touchwin(CURRENT_WINDOW_GAP);
    }
    if (CURRENT_WINDOW_COMMAND != NULL) {
      touchwin(CURRENT_WINDOW_COMMAND);
    }
    if (CURRENT_WINDOW_IDLINE != NULL) {
      touchwin(CURRENT_WINDOW_IDLINE);
    }
    touchwin(CURRENT_WINDOW_FILEAREA);
    show_statarea();
  }
  /*
   * If we have a Rexx interpreter,
   * register a handler for ring.x where x is the number of files in the ring.
   */
  if (rexx_support) {
    uchar tmp[20];

    sprintf((char *) tmp, "ring.%ld", number_of_files + ((compatible_feel == COMPAT_XEDIT) ? 1 : 0));
    MyRexxRegisterFunctionExe(tmp);
  }
  return (rc);
}

