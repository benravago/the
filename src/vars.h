// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* commset1.c */
extern the_header_mapping thm[];

/* commset2.c */
extern bool rexx_output;

/* commutil.c */
extern char_t *last_command_for_reexecute, * last_command_for_repeat, * last_command_for_repeat_in_macro, *temp_cmd;
extern DEFINE *first_define, *last_define, *first_mouse_define, *last_mouse_define, *first_synonym, *last_synonym;
extern LINE *key_first_line, *key_last_line;
extern line_t key_number_lines;
extern AREAS valid_areas[ATTR_MAX];

/* default.c */
extern bool BEEPx, CAPREXXOUTx, ERROROUTPUTx, CLEARSCREENx, CLOCKx, HEXDISPLAYx, INSERTMODEx, LINEND_STATUSx, REPROFILEx, DONT_CALL_DEFSORTx, TYPEAHEADx, scroll_cursor_stay, MOUSEx, SLKx, SBx, UNTAAx, PAGEWRAPx, FILETABSx, CTLCHARx, save_for_repeat, inDIALOG;
extern char_t CMDARROWSTABCMDx, EOLx, INTERFACEx, LINEND_VALUEx, NONDISPx, PREFIXx, TABI_ONx, TABI_Nx, EQUIVCHARx, EQUIVCHARstr[2], BACKUP_SUFFIXx[101], ERRORFORMATx;
extern line_t CAPREXXMAXx;
extern row_t STATUSLINEx;
extern int DEFSORTx, DIRORDERx, CLEARERRORKEYx, TARGETSAVEx, REGEXPx, READONLYx, COMMANDCALLSx, FUNCTIONCALLSx, popup_escape_key, popup_escape_keys[MAXIMUM_POPUP_KEYS], last_command_index;
extern PARSER_DETAILS *first_parser, *last_parser;
extern PARSER_MAPPING *first_parser_mapping, *last_parser_mapping;
extern char_t ctlchar_escape;
extern COLOUR_ATTR ctlchar_attr[MAX_CTLCHARS];
extern char_t ctlchar_char[MAX_CTLCHARS];
extern bool ctlchar_protect[MAX_CTLCHARS];
extern struct regexp_syntax regexp_syntaxes[];

/* edit.c */
extern bool prefix_changed;

/* error.c */
extern char_t *last_message;
extern int last_message_length;

/* mygetopt.c */
extern char *optarg;
extern int opterr, optind;

/* prefix.c */
extern LINE *first_prefix_synonym;
extern line_t prefix_current_line;
extern bool in_prefix_macro;

/* query.c */
extern VALUE item_values[MAX_VARIABLES_RETURNED];

/* mouse.c */
extern MEVENT ncurses_mouse_event;

/* rexx.c */
extern LINE *rexxout_first_line, *rexxout_last_line, *rexxout_curr;
extern line_t rexxout_number_lines;
extern QUERY_ITEM query_item[];
extern QUERY_ITEM function_item[];

/* the.c */
extern SCREEN_DETAILS screen[MAX_SCREENS];
extern short screen_rows[MAX_SCREENS];
extern short screen_cols[MAX_SCREENS];
extern WINDOW *statarea, *error_window, *divider, *filetabs;
extern VIEW_DETAILS *vd_current, *vd_first, *vd_last, *vd_mark, *filetabs_start_view;
extern line_t number_of_files;
extern char_t number_of_views, display_screens, current_screen;
extern SCREEN_DETAILS screen[MAX_SCREENS];
extern bool rexx_support, batch_only, horizontal, first_screen_display;
extern short save_coord_x[VIEW_WINDOWS], save_coord_y[VIEW_WINDOWS];
extern LINE *next_line, *curr_line, *first_file_name, *current_file_name, *editv, *first_option, *last_option;
extern bool error_on_screen, colour_support, initial, been_interactive;
extern char_t *rec;
extern length_t rec_len;
extern char_t *trec;
extern length_t trec_len;
extern length_t max_trec_len;
extern char_t *brec;
extern length_t brec_len;
extern char_t *cmd_rec;
extern length_t cmd_rec_len;
extern length_t cmd_verify_col;
extern char_t *pre_rec;
extern length_t pre_rec_len;
extern char_t *profile_command_line, *target_buffer;
extern length_t target_buffer_len;
extern bool focus_changed, current_changed, in_profile, in_nomsg, in_reprofile;
extern int profile_file_executions;
extern bool execute_profile, in_macro, in_readv, file_read, curses_started, the_readonly, interactive_in_macro, be_quiet;
extern char_t *the_version, *the_release, *the_copyright, term_name[20];
extern char_t *tempfilename;
extern short colour_offset_bits;
extern char_t user_home_dir[MAX_FILE_NAME + 1];
extern char_t *rexxoutname, *keyfilename, rexx_pathname[MAX_FILE_NAME + 1], rexx_filename[10], *dirfilename;
extern char_t macro_suffix[12];
extern char_t dir_pathname[MAX_FILE_NAME + 1], dir_filename[10], key_pathname[MAX_FILE_NAME + 1], key_filename[15];
extern char_t curr_path[MAX_FILE_NAME + 1], sp_path[MAX_FILE_NAME + 1], sp_fname[MAX_FILE_NAME + 1], dir_path[MAX_FILE_NAME + 1], dir_files[MAX_FILE_NAME + 1], rexx_macro_name[MAX_FILE_NAME + 1], rexx_macro_parameters[MAX_FILE_NAME + 1], the_home_dir[MAX_FILE_NAME + 1], the_help_file[MAX_FILE_NAME + 1], the_macro_path[MAX_FILE_NAME + 1], the_macro_path_buf[MAX_FILE_NAME + 1], **the_macro_dir;
extern int max_macro_dirs, total_macro_dirs;
extern char_t *prf_arg, *local_prf, *specified_prf;
extern char_t tabkey_insert, tabkey_overwrite;
extern char_t spooler_name[MAX_FILE_NAME + 1];
extern struct stat stat_buf;
extern length_t display_length;
extern short lastrc, compatible_look, compatible_feel, compatible_keys, prefix_width, prefix_gap;
extern chtype etmode_table[256];
extern bool etmode_flag[256];
extern short terminal_lines, terminal_cols;
extern line_t original_screen_line, original_screen_column, original_file_line, original_file_column, startup_line;
extern length_t startup_column;
extern char_t *linebuf;
extern chtype *linebufch;
extern length_t linebuf_size;
extern int lastkeys[8], lastkeys_is_mouse[8], current_key;

extern bool ncurses_screen_resized;
extern int max_slk_labels, slk_format_switch;
extern bool single_instance_server;
extern char_t fifo_name[MAX_FILE_NAME + 1];
extern char_t pid_name[MAX_FILE_NAME + 1];
extern LASTOP lastop[LASTOP_MAX];
extern FILE *record_fp;
extern int record_key;
extern char_t *record_status;

