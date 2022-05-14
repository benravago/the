/* Please, include the.h first. */

/* commset2.c */
extern bool           rexx_output;

/* commutil.c */
extern CHARTYPE       _THE_FAR last_command_for_reexecute[MAX_COMMAND_LENGTH],
                      _THE_FAR last_command_for_repeat[MAX_COMMAND_LENGTH],
                      _THE_FAR last_change_command[MAX_COMMAND_LENGTH],
                      _THE_FAR last_target[MAX_COMMAND_LENGTH],
                      *temp_cmd;
extern DEFINE         *first_define,
                      *last_define,
                      *first_mouse_define,
                      *last_mouse_define;
extern bool           clear_command;
extern LINE           *key_first_line,
                      *key_last_line;
extern LINETYPE       key_number_lines;
extern AREAS          valid_areas[ATTR_MAX];

/* default.c */
extern bool           BEEPx,
                      CAPREXXOUTx;
extern LINETYPE       CAPREXXMAXx;
extern bool           CLEARSCREENx,
                      CLOCKx;
extern CHARTYPE       CMDARROWSTABCMDx,
                      EOLx;
extern bool           HEXDISPLAYx,
                      INSERTMODEx,
                      LINEND_STATUSx;
extern CHARTYPE       LINEND_VALUEx,
                      NONDISPx,
                      PREFIXx;
extern bool           REPROFILEx;
extern CHARTYPE       TABI_ONx,
                      TABI_Nx;
extern ROWTYPE        STATUSLINEx;
extern bool           TYPEAHEADx,
                      scroll_cursor_stay,
                      MOUSEx,
                      SLKx,
                      SBx,
                      UNTAAx,
                      READONLYx;
extern int            DEFSORTx,
                      DIRORDERx,
                      CLEARERRORKEYx,
                      popup_escape_key;
extern PARSER_DETAILS *first_parser,
                      *last_parser;
extern PARSER_MAPPING *first_parser_mapping,
                      *last_parser_mapping;
extern bool CTLCHARx;
extern CHARTYPE ctlchar_escape;
extern COLOUR_ATTR ctlchar_attr[MAX_CTLCHARS];
extern CHARTYPE ctlchar_char[MAX_CTLCHARS];

/* edit.c */
extern bool           prefix_changed;

/* error.c */
extern CHARTYPE       *last_message;
extern int            last_message_length;

/* mygetopt.c */
extern char           *optarg;
extern int            opterr,
                      optind;

/* prefix.c */
extern LINE           *first_prefix_synonym;
extern LINETYPE       prefix_current_line;
extern bool           in_prefix_macro;

/* query.c */
extern VALUE          item_values[MAX_VARIABLES_RETURNED];

/* mouse.c */
#if defined(NCURSES_MOUSE_VERSION)
extern MEVENT         ncurses_mouse_event;
#endif

/* rexx.c */
extern LINE           *rexxout_first_line,
                      *rexxout_last_line,
                      *rexxout_curr;
extern LINETYPE       rexxout_number_lines;
extern QUERY_ITEM _THE_FAR query_item[];
extern QUERY_ITEM _THE_FAR function_item[];

/* the.c */
extern SCREEN_DETAILS screen[MAX_SCREENS];
extern short          screen_rows[MAX_SCREENS];
extern WINDOW         *statarea,
                      *error_window,
                      *divider;
extern VIEW_DETAILS   *vd_current,
                      *vd_first,
                      *vd_last,
                      *vd_mark;
extern CHARTYPE       number_of_views,
                      number_of_files,
                      display_screens,
                      current_screen;
extern SCREEN_DETAILS screen[MAX_SCREENS];
extern bool           rexx_support,
                      batch_only,
                      horizontal,
                      first_screen_display;
extern short          save_coord_x[VIEW_WINDOWS],
                      save_coord_y[VIEW_WINDOWS];
extern LINE           *next_line,
                      *curr_line,
                      *first_file_name,
                      *current_file_name,
                      *editv,
                      *first_option,
                      *last_option;
extern bool           error_on_screen,
                      colour_support,
                      initial,
                      been_interactive;
extern CHARTYPE       *rec;
extern LENGTHTYPE     rec_len;
extern CHARTYPE       *trec;
extern LENGTHTYPE     trec_len;
extern LENGTHTYPE     max_trec_len;
extern CHARTYPE       *brec;
extern LENGTHTYPE     brec_len;
extern CHARTYPE       *cmd_rec;
extern unsigned short cmd_rec_len;
extern CHARTYPE       *pre_rec;
extern unsigned short pre_rec_len;
extern CHARTYPE       *profile_command_line,
                      *target_buffer;
extern unsigned short target_buffer_len;
extern bool           focus_changed,
                      current_changed,
                      in_profile,
                      in_nomsg,
                      in_reprofile;
extern int            profile_file_executions;
extern bool           execute_profile,
                      in_macro,
                      in_repeat,
                      in_readv,
                      file_read,
                      curses_started,
                      readonly,
                      be_quiet;
extern CHARTYPE       *the_version,
                      *the_release,
                      *the_copyright,
                      term_name[20],
                      *tempfilename;
#if defined(UNIX)
extern CHARTYPE       user_home_dir[MAX_FILE_NAME+1];
#endif
#if !defined(MULTIPLE_PSEUDO_FILES)
extern CHARTYPE       *rexxoutname,
                      *keyfilename,
                      _THE_FAR rexx_pathname[MAX_FILE_NAME+1],
                      rexx_filename[10],
                      *dirfilename;
#endif
extern CHARTYPE       _THE_FAR xterm_program[MAX_FILE_NAME+1],
                      macro_suffix[12];
#if !defined(MULTIPLE_PSEUDO_FILES)
extern CHARTYPE       _THE_FAR dir_pathname[MAX_FILE_NAME+1],
                      dir_filename[10],
                      _THE_FAR key_pathname[MAX_FILE_NAME+1],
                      key_filename[15];
#endif
extern CHARTYPE       _THE_FAR curr_path[MAX_FILE_NAME+1],
                      _THE_FAR sp_path[MAX_FILE_NAME+1],
                      _THE_FAR sp_fname[MAX_FILE_NAME+1],
                      _THE_FAR dir_path[MAX_FILE_NAME+1],
                      _THE_FAR dir_files[MAX_FILE_NAME+1],
                      _THE_FAR rexx_macro_name[MAX_FILE_NAME+1],
                      _THE_FAR rexx_macro_parameters[MAX_FILE_NAME+1],
                      _THE_FAR the_home_dir[MAX_FILE_NAME+1],
                      _THE_FAR the_help_file[MAX_FILE_NAME+1],
                      _THE_FAR the_macro_path[MAX_FILE_NAME+1],
                      _THE_FAR the_macro_path_buf[MAX_FILE_NAME+1],
                      *the_macro_dir[MAX_MACRO_DIRS];
extern int            max_macro_dirs;
extern CHARTYPE       *prf_arg,
                      *local_prf,
                      *specified_prf,
                      tabkey_insert,
                      tabkey_overwrite;
extern unsigned short file_start;
extern CHARTYPE       _THE_FAR spooler_name[MAX_FILE_NAME+1];
extern struct stat    stat_buf;
extern LENGTHTYPE     display_length;
extern short          lastrc,
                      compatible_look,
                      compatible_feel,
                      compatible_keys,
                      prefix_width,
                      prefix_gap;
extern chtype         etmode_table[256];
extern bool           etmode_flag[256];
extern short          terminal_lines,
                      terminal_cols;
extern LINETYPE       original_screen_line,
                      original_screen_column,
                      original_file_line,
                      original_file_column,
                      startup_line;
extern LENGTHTYPE     startup_column;
#ifdef XCURSES
extern char           *XCursesProgramName;
#endif
extern CHARTYPE       *linebuf;
extern chtype         *linebufch;
extern int            lastkeys[8],
                      current_key;
#ifdef WIN32
extern bool           StartedPrinter;
#endif

#if defined(SIGWINCH) && defined(USE_NCURSES)
extern bool ncurses_screen_resized;
#endif
extern int            max_slk_labels;
