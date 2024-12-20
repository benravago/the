// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/* Please, include the.h first. */

/* commset1.c */
extern the_header_mapping thm[];

/* commset2.c */
extern bool rexx_output;

/* commutil.c */
extern uchar *last_command_for_reexecute, *last_command_for_repeat, *last_command_for_repeat_in_macro, *temp_cmd;
extern DEFINE *first_define, *last_define, *first_mouse_define, *last_mouse_define, *first_synonym, *last_synonym;
extern LINE *key_first_line, *key_last_line;
extern long key_number_lines;
extern AREAS valid_areas[ATTR_MAX];

/* default.c */
extern bool BEEPx, CAPREXXOUTx, ERROROUTPUTx, CLEARSCREENx, CLOCKx, HEXDISPLAYx, INSERTMODEx, LINEND_STATUSx, REPROFILEx, DONT_CALL_DEFSORTx, TYPEAHEADx, scroll_cursor_stay, MOUSEx, SLKx, SBx, UNTAAx,
       PAGEWRAPx, FILETABSx, CTLCHARx, save_for_repeat,
       inDIALOG;
extern uchar CMDARROWSTABCMDx, EOLx, INTERFACEx, LINEND_VALUEx, NONDISPx, PREFIXx, TABI_ONx, TABI_Nx, EQUIVCHARx, EQUIVCHARstr[2], BACKUP_SUFFIXx[101], ERRORFORMATx;
extern long CAPREXXMAXx;
extern ushort STATUSLINEx;
extern int DEFSORTx, DIRORDERx, CLEARERRORKEYx, TARGETSAVEx, REGEXPx, READONLYx, COMMANDCALLSx, FUNCTIONCALLSx, popup_escape_key, popup_escape_keys[MAXIMUM_POPUP_KEYS], last_command_index;
extern PARSER_DETAILS *first_parser, *last_parser;
extern PARSER_MAPPING *first_parser_mapping, *last_parser_mapping;
extern uchar ctlchar_escape;
extern COLOUR_ATTR ctlchar_attr[MAX_CTLCHARS];
extern uchar ctlchar_char[MAX_CTLCHARS];
extern bool ctlchar_protect[MAX_CTLCHARS];
extern struct regexp_syntax regexp_syntaxes[];

/* edit.c */
extern bool prefix_changed;

/* error.c */
extern uchar *last_message;
extern int last_message_length;

/* prefix.c */
extern LINE *first_prefix_synonym;
extern long prefix_current_line;
extern bool in_prefix_macro;

/* query.c */
extern VALUE item_values[MAX_VARIABLES_RETURNED];

/* mouse.c */
extern MEVENT ncurses_mouse_event;

/* rexx.c */
extern LINE *rexxout_first_line, *rexxout_last_line, *rexxout_curr;
extern long rexxout_number_lines;
extern QUERY_ITEM query_item[];
extern QUERY_ITEM function_item[];

/* the.c */
extern SCREEN_DETAILS screen[MAX_SCREENS];
extern short screen_rows[MAX_SCREENS];
extern short screen_cols[MAX_SCREENS];
extern WINDOW *statarea, *error_window, *divider, *filetabs;
extern VIEW_DETAILS *vd_current, *vd_first, *vd_last, *vd_mark, *filetabs_start_view;
extern long number_of_files;
extern uchar number_of_views, display_screens, current_screen;
extern SCREEN_DETAILS screen[MAX_SCREENS];
extern bool rexx_support, batch_only, horizontal, first_screen_display;
extern short save_coord_x[VIEW_WINDOWS], save_coord_y[VIEW_WINDOWS];
extern LINE *next_line, *curr_line, *first_file_name, *current_file_name, *editv, *first_option, *last_option;
extern bool error_on_screen, colour_support, initial, been_interactive;
extern uchar *rec;
extern long rec_len;
extern uchar *trec;
extern long trec_len;
extern long max_trec_len;
extern uchar *brec;
extern long brec_len;
extern uchar *cmd_rec;
extern long cmd_rec_len;
extern long cmd_verify_col;
extern uchar *pre_rec;
extern long pre_rec_len;
extern uchar *profile_command_line, *target_buffer;
extern long target_buffer_len;
extern bool focus_changed, current_changed, in_profile, in_nomsg, in_reprofile;
extern int profile_file_executions;
extern bool execute_profile, in_macro, in_readv, file_read, curses_started, the_readonly, interactive_in_macro, be_quiet;
extern uchar *the_version, *the_release, *the_copyright, *term_name;
extern uchar *tempfilename;
extern short colour_offset_bits;
extern uchar user_home_dir[MAX_FILE_NAME + 1];
extern uchar *rexxoutname, *keyfilename, rexx_pathname[MAX_FILE_NAME + 1], rexx_filename[10], *dirfilename;
extern uchar xterm_program[MAX_FILE_NAME + 1], macro_suffix[12];
extern uchar dir_pathname[MAX_FILE_NAME + 1], dir_filename[10], key_pathname[MAX_FILE_NAME + 1], key_filename[15];
extern uchar curr_path[MAX_FILE_NAME + 1], sp_path[MAX_FILE_NAME + 1], sp_fname[MAX_FILE_NAME + 1], dir_path[MAX_FILE_NAME + 1], dir_files[MAX_FILE_NAME + 1], rexx_macro_name[MAX_FILE_NAME + 1],
       rexx_macro_parameters[MAX_FILE_NAME + 1],
       the_home_dir[MAX_FILE_NAME + 1], the_help_file[MAX_FILE_NAME + 1], the_macro_path[MAX_FILE_NAME + 1], the_macro_path_buf[MAX_FILE_NAME + 1], **the_macro_dir;
extern int max_macro_dirs, total_macro_dirs;
extern uchar *prf_arg, *local_prf, *specified_prf;
extern uchar tabkey_insert, tabkey_overwrite;
extern uchar spooler_name[MAX_FILE_NAME + 1];
extern struct stat stat_buf;
extern long display_length;
extern short lastrc, compatible_look, compatible_feel, compatible_keys, prefix_width, prefix_gap;
extern chtype etmode_table[256];
extern bool etmode_flag[256];
extern short terminal_lines, terminal_cols;
extern long original_screen_line, original_screen_column, original_file_line, original_file_column, startup_line;
extern long startup_column;
extern uchar *linebuf;
extern chtype *linebufch;
extern long linebuf_size;
extern int lastkeys[8], lastkeys_is_mouse[8], current_key;

extern bool ncurses_screen_resized;
extern int max_slk_labels, slk_format_switch;
extern bool single_instance_server;
extern uchar fifo_name[MAX_FILE_NAME + 1];
extern uchar pid_name[MAX_FILE_NAME + 1];
extern LASTOP lastop[LASTOP_MAX];
extern FILE *record_fp;
extern int record_key;
extern uchar *record_status;

