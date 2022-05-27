/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
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
 */

/*
$Id: proto.h,v 1.64 2019/09/13 02:30:08 mark Exp $
*/

                                                         /* commutil.c */
char_t *get_key_name Args((int, int *));
char_t *get_key_definition Args((int, int, bool, bool));
short function_key Args((int, int, bool));
bool is_modifier_key Args((int));
char_t *build_default_key_definition Args((int, char_t *));
char_t *build_synonym_definition Args((DEFINE *, char_t *, char_t *, bool));
short display_all_keys Args((void));
int set_rexx_variables_for_all_keys Args((int, int *));
short command_line Args((char_t *, bool));
void cleanup_command_line Args((void));
void split_command Args((char_t *, char_t *, char_t *));
short param_split Args((char_t *, char_t *[], int, char_t *, char_t, char_t *, bool));
short quoted_param_split Args((char_t *, char_t *[], int, char_t *, char_t, char_t *, bool, char_t *));
short command_split Args((char_t *, char_t *[], int, char_t *, char_t *));
line_t get_true_line Args((bool));
length_t get_true_column Args((bool));
char_t next_char Args((LINE *, long *, length_t));
short add_define Args((DEFINE **, DEFINE **, int, char_t *, bool, char_t *, char_t));
short remove_define Args((DEFINE **, DEFINE **, int, char_t *));
short append_define Args((DEFINE **, DEFINE **, int, short, char_t *, char_t *, int, char_t *, char_t));
short find_command Args((char_t *, bool));
void init_command Args((void));
void add_command Args((char_t *));
char_t *get_next_command Args((short, int));
bool valid_command_to_save Args((char_t *));
bool is_tab_col Args((length_t));
length_t find_next_tab_col Args((length_t));
length_t find_prev_tab_col Args((length_t));
short tabs_convert Args((LINE *, bool, bool, bool));
short convert_hex_strings Args((char_t *));
short marked_block Args((bool));
short suspend_curses Args((void));
short resume_curses Args((void));
short restore_THE Args((void));
short execute_set_sos_command Args((bool, char_t *));
short valid_command_type Args((bool, char_t *));
short allocate_temp_space Args((length_t, char_t));
void free_temp_space Args((char_t));
char_t calculate_actual_row Args((short, short, row_t, bool));
short get_valid_macro_file_name Args((char_t *, char_t *, char_t *, short *));
bool define_command Args((char_t *));
int find_key_name Args((char_t *));
int readv_cmdline Args((char_t *, WINDOW *, int));
short execute_mouse_commands Args((int));
short validate_n_m Args((char_t *, short *, short *));
void ResetOrDeleteCUABlock Args((int));
short execute_locate Args((char_t *, bool, bool, bool *));
void adjust_other_screen_shadow_lines Args((void));
int is_file_in_ring Args((char_t * fpath, char_t * fname));
int save_lastop Args((int idx, char_t * lastop));
char_t *get_command_name Args((int idx, bool *set_command, bool *sos_command));

                                                            /* print.c */
void print_line Args((bool, line_t, line_t, short, char_t *, char_t *, short));
short setprintername Args((char *));
short setfontname Args((char *));
short setfontcpi Args((int));
short setfontlpi Args((int));
short setorient Args((char));
short setpagesize Args((int));

                                                           /* target.c */
short split_change_params Args((char_t *, char_t **, char_t **, TARGET *, line_t *, line_t *));
short parse_target Args((char_t *, line_t, TARGET *, long, bool, bool, bool));
void initialise_target Args((TARGET *));
void free_target Args((TARGET *));
short find_target Args((TARGET *, line_t, bool, bool));
short find_column_target Args((char_t *, length_t, TARGET *, length_t, bool, bool));
THELIST *find_line_name Args((LINE * curr, char_t * name));
LINE *find_named_line Args((char_t *, line_t *, bool));
short find_string_target Args((LINE *, RTARGET *, length_t, int));
short find_rtarget_target Args((LINE *, TARGET *, line_t, line_t, line_t *));
bool find_rtarget_column_target Args((char_t *, length_t, TARGET *, length_t, length_t, line_t *));
line_t find_next_in_scope Args((VIEW_DETAILS *, LINE *, line_t, short));
line_t find_last_not_in_scope Args((VIEW_DETAILS *, LINE *, line_t, short));
short validate_target Args((char_t *, TARGET *, long, line_t, bool, bool));
void calculate_scroll_values Args((char_t, VIEW_DETAILS *, short *, line_t *, line_t *, bool *, bool *, bool *, short));
short find_first_focus_line Args((char_t, unsigned short *));
short find_last_focus_line Args((char_t, unsigned short *));
char_t find_unique_char Args((char_t *));

                                                         /* reserved.c */
RESERVED *add_reserved_line Args((char_t *, char_t *, short, short, COLOUR_ATTR *, bool));
RESERVED *find_reserved_line Args((char_t, bool, row_t, short, short));
short delete_reserved_line Args((short, short));

                                                              /* box.c */
void box_operations Args((short, char_t, bool, char_t));
void box_paste_from_clipboard Args((LINE *, line_t, line_t));

                                                          /* execute.c */
short execute_os_command Args((char_t *, bool, bool));
short execute_change_command Args((char_t *, bool));
short insert_new_line Args((char_t, VIEW_DETAILS *, char_t *, length_t, line_t, line_t, bool, bool, bool, char_t, bool, bool));
short execute_makecurr Args((char_t, VIEW_DETAILS *, line_t));
short execute_shift_command Args((char_t, VIEW_DETAILS *, bool, length_t, line_t, line_t, bool, long, bool, bool));
short execute_set_lineflag Args((unsigned int, unsigned int, unsigned int, line_t, line_t, bool, long));
short do_actual_change_case Args((line_t, line_t, char_t, bool, short, length_t, length_t));
short execute_change_case Args((char_t *, char_t));
short rearrange_line_blocks Args((char_t, char_t, line_t, line_t, line_t, long, VIEW_DETAILS *, VIEW_DETAILS *, bool, line_t *));
short execute_set_point Args((char_t, VIEW_DETAILS *, char_t *, line_t, bool));
short execute_wrap_word Args((length_t));
short execute_split_join Args((short, bool, bool));
short execute_put Args((char_t *, bool));
short execute_macro Args((char_t *, bool, short *));
short write_macro Args((char_t *));
short execute_set_on_off Args((char_t *, bool *, bool));
short execute_set_row_position Args((char_t *, short *, short *));
short processable_line Args((VIEW_DETAILS *, line_t, LINE *));
short execute_expand_compress Args((char_t *, bool, bool, bool, bool));
short execute_select Args((char_t *, bool, short));
short execute_move_cursor Args((char_t, VIEW_DETAILS *, length_t));
short execute_find_command Args((char_t *, long));
short execute_modify_command Args((char_t *));
length_t calculate_rec_len Args((short, char_t *, length_t, length_t, line_t, short));
short execute_editv Args((short, bool, char_t *));
short prepare_dialog Args((char_t *, bool, char_t *));
short execute_dialog Args((char_t *, char_t *, char_t *, bool, short, short, char_t *, short, bool));
short prepare_popup Args((char_t *));
short execute_popup Args((int, int, int, int, int, int, int, int, char_t **, int));
short execute_preserve Args((VIEW_DETAILS *, PRESERVED_VIEW_DETAILS **, FILE_DETAILS *, PRESERVED_FILE_DETAILS **));
short execute_restore Args((VIEW_DETAILS *, PRESERVED_VIEW_DETAILS **, FILE_DETAILS *, PRESERVED_FILE_DETAILS **, bool));

                                                          /* default.c */
void set_global_defaults Args((void));
void set_global_look_defaults Args((void));
void set_global_feel_defaults Args((void));
void set_file_defaults Args((FILE_DETAILS *));
void set_view_defaults Args((VIEW_DETAILS *));
short get_profile Args((char_t *, char_t *));
short defaults_for_first_file Args((void));
short defaults_for_other_files Args((VIEW_DETAILS *));
short default_file_attributes Args((FILE_DETAILS *));
void set_screen_defaults Args((void));
short set_THE_key_defaults Args((int, int));
short set_XEDIT_key_defaults Args((int, int));
short set_ISPF_key_defaults Args((int, int));
short set_KEDIT_key_defaults Args((int, int));
short construct_default_parsers Args((void));
short destroy_all_parsers Args((void));
short construct_default_parser_mapping Args((void));
char_t *find_default_parser Args((char_t *));

                                                             /* edit.c */
void editor Args((void));
int process_key Args((int, bool));
short EditFile Args((char_t *, bool));

                                                            /* error.c */
int display_error Args((unsigned short, char_t *, bool));
void clear_msgline Args((int));
void display_prompt Args((char_t *));
int expose_msgline Args((void));

                                                             /* file.c */
short get_file Args((char_t *));
LINE *read_file Args((FILE *, LINE *, char_t *, line_t, line_t, bool));
LINE *read_fixed_file Args((FILE *, LINE *, char_t *, line_t, line_t));
short save_file Args((FILE_DETAILS *, char_t *, bool, line_t, line_t, line_t *, bool, length_t, length_t, bool, bool, bool));
void increment_alt Args((FILE_DETAILS *));
char_t *new_filename Args((char_t *, char_t *, char_t *, char_t *));
short remove_aus_file Args((FILE_DETAILS *));
short free_view_memory Args((bool, bool));
void free_a_view Args((void));
short free_file_memory Args((bool));
short read_directory Args((void));
VIEW_DETAILS *find_file Args((char_t *, char_t *));
VIEW_DETAILS *find_pseudo_file Args((char_t));
short execute_command_file Args((FILE *));
char_t *read_file_into_memory Args((char_t *, int *));

                                                            /* getch.c */
int my_getch Args((WINDOW *));

                                                          /* nonansi.c */
short file_readable Args((char_t *));
short file_writable Args((char_t *));
short file_exists Args((char_t *));
short remove_file Args((char_t *));
short splitpath Args((char_t *));
LINE *getclipboard Args((LINE *, int));
short setclipboard Args((FILE_DETAILS *, char_t *, bool, line_t, line_t, line_t, line_t *, bool, length_t, length_t, bool, bool, int));
void draw_cursor Args((bool));

                                                           /* parser.c */
short parse_line Args((char_t, FILE_DETAILS *, SHOW_LINE *, short));
short parse_paired_comments Args((char_t, FILE_DETAILS *));
short construct_parser Args((char_t *, int, PARSER_DETAILS **, char_t *, char_t *));
short destroy_parser Args((PARSER_DETAILS *));
bool find_parser_mapping Args((FILE_DETAILS *, PARSER_MAPPING *));
PARSER_DETAILS *find_auto_parser Args((FILE_DETAILS *));
short parse_reserved_line Args((RESERVED *));

                                                           /* prefix.c */
short execute_prefix_commands Args((void));
void clear_pending_prefix_command Args((THE_PPC *, FILE_DETAILS *, LINE *));
THE_PPC *delete_pending_prefix_command Args((THE_PPC *, FILE_DETAILS *, LINE *));
void add_prefix_command Args((char_t, VIEW_DETAILS *, LINE *, line_t, bool, bool));
short add_prefix_synonym Args((char_t *, char_t *));
char_t *find_prefix_synonym Args((char_t *));
char_t *find_prefix_oldname Args((char_t *));
char_t *get_prefix_command Args((line_t));
char_t get_syntax_element Args((char_t, int, int));

                                                             /* show.c */
void prepare_idline Args((char_t));
void show_heading Args((char_t));
void show_statarea Args((void));
void clear_statarea Args((void));
void display_filetabs Args((VIEW_DETAILS *));
void build_screen Args((char_t));
void display_screen Args((char_t));
void display_cmdline Args((char_t, VIEW_DETAILS *));
void show_marked_block Args((void));
void redraw_window Args((WINDOW *));
void repaint_screen Args((void));
void touch_screen Args((char_t));
void refresh_screen Args((char_t));
void redraw_screen Args((char_t));
bool line_in_view Args((char_t, line_t));
bool column_in_view Args((char_t, length_t));
line_t find_next_current_line Args((line_t, short));
short get_row_for_focus_line Args((char_t, line_t, short));
line_t get_focus_line_in_view Args((char_t, line_t, row_t));
line_t calculate_focus_line Args((line_t, line_t));
char *get_current_position Args((char_t, line_t *, length_t *));
void calculate_new_column Args((char_t, VIEW_DETAILS *, col_t, length_t, length_t, col_t *, length_t *));
short prepare_view Args((char_t));
short advance_view Args((VIEW_DETAILS *, short));
short force_curses_background Args((void));
short THE_Resize Args((int, int));

                                                           /* scroll.c */
short scroll_page Args((short, line_t, bool));
short scroll_line Args((char_t, VIEW_DETAILS *, short, line_t, bool, short));

                                                              /* the.c */
void init_colour_pairs Args((void));
int setup_profile_files Args((char_t *));
void cleanup Args((void));
int allocate_working_memory Args((void));
char **StringToArgv Args((int *, char *));

                                                             /* util.c */
char_t *ebc2asc Args((char_t *, int, int, int));
char_t *asc2ebc Args((char_t *, int, int, int));
length_t memreveq Args((char_t *, char_t, length_t));
length_t memrevne Args((char_t *, char_t, length_t));
char_t *meminschr Args((char_t *, char_t, length_t, length_t, length_t));
char_t *meminsmem Args((char_t *, char_t *, length_t, length_t, length_t, length_t));
char_t *memdeln Args((char_t *, length_t, length_t, length_t));
char_t *strdelchr Args((char_t *, char_t));
char_t *memrmdup Args((char_t *, length_t *, char_t));
char_t *strrmdup Args((char_t *, char_t, bool));
length_t strzne Args((char_t *, char_t));
char_t *my_strdup Args((char_t *));
length_t memne Args((char_t *, char_t, length_t));
length_t strzrevne Args((char_t *, char_t));
length_t strzreveq Args((char_t *, char_t));
char_t *strtrunc Args((char_t *));
char_t *MyStrip Args((char_t *, char, char));
length_t memfind Args((char_t *, char_t *, length_t, length_t, bool, bool, char_t, char_t, length_t *));
void memrev Args((char_t *, char_t *, length_t));
length_t memcmpi Args((char_t *, char_t *, length_t));
char_t *make_upper Args((char_t *));
bool equal Args((char_t *, char_t *, length_t));
bool valid_integer Args((char_t *));
bool valid_positive_integer Args((char_t *));
short valid_positive_integer_against_maximum Args((char_t *, length_t));
length_t strzeq Args((char_t *, char_t));
char_t *strtrans Args((char_t *, char_t, char_t));
LINE *add_LINE Args((LINE *, LINE *, char_t *, length_t, select_t, bool));
LINE *append_LINE Args((LINE *, char_t *, length_t));
LINE *delete_LINE Args((LINE **, LINE **, LINE *, short, bool));
void put_string Args((WINDOW *, row_t, col_t, char_t *, length_t));
void put_char Args((WINDOW *, chtype, char_t));
short set_up_windows Args((short));
short draw_divider Args((void));
short create_statusline_window Args((void));
short create_filetabs_window Args((void));
void pre_process_line Args((VIEW_DETAILS *, line_t, LINE *));
short post_process_line Args((VIEW_DETAILS *, line_t, LINE *, bool));
bool blank_field Args((char_t *));
void adjust_marked_lines Args((bool, line_t, line_t));
void adjust_pending_prefix Args((VIEW_DETAILS *, bool, line_t, line_t));
char_t case_translate Args((char_t));
void add_to_recovery_list Args((char_t *, length_t));
void get_from_recovery_list Args((short));
void free_recovery_list Args((void));
short my_wmove Args((WINDOW *, short, short, short, short));
short my_isalphanum Args((char_t));
short get_row_for_tof_eof Args((short, char_t));
void set_compare_exact Args((bool));
int search_query_item_array Args((void *, size_t, size_t, const char *, int));
int split_function_name Args((char_t *, int *));
VIEW_DETAILS *find_filetab Args((int));
VIEW_DETAILS *find_next_file Args((VIEW_DETAILS *, short));

short my_wclrtoeol Args((WINDOW *));
short my_wdelch Args((WINDOW *));
short get_word Args((char_t *, length_t, length_t, length_t *, length_t *));
short get_fieldword Args((char_t *, length_t, length_t, length_t *, length_t *));

                                                           /* linked.c */
THELIST *ll_add Args((THELIST * first, THELIST * curr, unsigned short size));
THELIST *ll_del Args((THELIST ** first, THELIST ** last, THELIST * curr, short direction, THELIST_DEL delfunc));
THELIST *ll_free Args((THELIST * first, THELIST_DEL delfunc));
LINE *lll_add Args((LINE *, LINE *, unsigned short));
LINE *lll_del Args((LINE **, LINE **, LINE *, short));
LINE *lll_free Args((LINE *));
LINE *lll_find Args((LINE *, LINE *, line_t, line_t));
LINE *lll_locate Args((LINE *, char_t *));
VIEW_DETAILS *vll_add Args((VIEW_DETAILS *, VIEW_DETAILS *, unsigned short));
VIEW_DETAILS *vll_del Args((VIEW_DETAILS **, VIEW_DETAILS **, VIEW_DETAILS *, short));
DEFINE *dll_add Args((DEFINE *, DEFINE *, unsigned short));
DEFINE *dll_del Args((DEFINE **, DEFINE **, DEFINE *, short));
DEFINE *dll_free Args((DEFINE *));
THE_PPC *pll_add Args((THE_PPC **, unsigned short, line_t));
THE_PPC *pll_del Args((THE_PPC **, THE_PPC **, THE_PPC *, short));
THE_PPC *pll_free Args((THE_PPC *));
THE_PPC *pll_find Args((THE_PPC *, line_t));
RESERVED *rll_add Args((RESERVED *, RESERVED *, unsigned short));
RESERVED *rll_del Args((RESERVED **, RESERVED **, RESERVED *, short));
RESERVED *rll_free Args((RESERVED *));
RESERVED *rll_find Args((RESERVED *, short));
PARSER_DETAILS *parserll_add Args((PARSER_DETAILS *, PARSER_DETAILS *, unsigned short));
PARSER_DETAILS *parserll_del Args((PARSER_DETAILS **, PARSER_DETAILS **, PARSER_DETAILS *, short));
PARSER_DETAILS *parserll_free Args((PARSER_DETAILS *));
PARSER_DETAILS *parserll_find Args((PARSER_DETAILS *, char_t *));
PARSE_KEYWORDS *parse_keywordll_add Args((PARSE_KEYWORDS *, PARSE_KEYWORDS *, unsigned short));
PARSE_KEYWORDS *parse_keywordll_del Args((PARSE_KEYWORDS **, PARSE_KEYWORDS **, PARSE_KEYWORDS *, short));
PARSE_KEYWORDS *parse_keywordll_free Args((PARSE_KEYWORDS *));
PARSE_FUNCTIONS *parse_functionll_add Args((PARSE_FUNCTIONS *, PARSE_FUNCTIONS *, unsigned short));
PARSE_FUNCTIONS *parse_functionll_del Args((PARSE_FUNCTIONS **, PARSE_FUNCTIONS **, PARSE_FUNCTIONS *, short));
PARSE_FUNCTIONS *parse_functionll_free Args((PARSE_FUNCTIONS *));
PARSE_HEADERS *parse_headerll_add Args((PARSE_HEADERS *, PARSE_HEADERS *, unsigned short));
PARSE_HEADERS *parse_headerll_free Args((PARSE_HEADERS *));
PARSER_MAPPING *mappingll_add Args((PARSER_MAPPING *, PARSER_MAPPING *, unsigned short));
PARSER_MAPPING *mappingll_del Args((PARSER_MAPPING **, PARSER_MAPPING **, PARSER_MAPPING *, short));
PARSER_MAPPING *mappingll_free Args((PARSER_MAPPING *));
PARSER_MAPPING *mappingll_find Args((PARSER_MAPPING *, char_t *, char_t *));
PARSE_COMMENTS *parse_commentsll_add Args((PARSE_COMMENTS *, PARSE_COMMENTS *, unsigned short));
PARSE_COMMENTS *parse_commentsll_del Args((PARSE_COMMENTS **, PARSE_COMMENTS **, PARSE_COMMENTS *, short));
PARSE_COMMENTS *parse_commentsll_free Args((PARSE_COMMENTS *));
PARSE_COMMENTS *parse_commentsll_find Args((PARSE_COMMENTS *, char_t *));
PARSE_POSTCOMPARE *parse_postcomparell_add Args((PARSE_POSTCOMPARE *, PARSE_POSTCOMPARE *, unsigned short));
PARSE_POSTCOMPARE *parse_postcomparell_del Args((PARSE_POSTCOMPARE **, PARSE_POSTCOMPARE **, PARSE_POSTCOMPARE *, short));
PARSE_POSTCOMPARE *parse_postcomparell_free Args((PARSE_POSTCOMPARE *));
PARSE_EXTENSION *parse_extensionll_add Args((PARSE_EXTENSION *, PARSE_EXTENSION *, unsigned short));
PARSE_EXTENSION *parse_extensionll_del Args((PARSE_EXTENSION **, PARSE_EXTENSION **, PARSE_EXTENSION *, short));
PARSE_EXTENSION *parse_extensionll_free Args((PARSE_EXTENSION *));

                                                             /* rexx.c */
unsigned long MyRexxRegisterFunctionExe Args((char_t *));
unsigned long MyRexxDeregisterFunction Args((char_t *));
short initialise_rexx Args((void));
short finalise_rexx Args((void));
short execute_macro_file Args((char_t *, char_t *, short *, bool));
short execute_macro_instore Args((char_t *, short *, char_t **, int *, int *, int));
short get_rexx_variable Args((char_t *, char_t **, int *));
short set_rexx_variable Args((char_t *, char_t *, length_t, int));
char_t *get_rexx_interpreter_version Args((char_t *));

                                                           /* os2eas.c */
                                                            /* query.c */
short find_query_item Args((char_t *, int, char_t *));
short show_status Args((void));
short save_status Args((char_t *));
short set_extract_variables Args((short));
short get_number_dynamic_items Args((int));
short get_item_values Args((int, short, char_t *, char_t, line_t, char_t *, line_t));
int number_query_item Args((void));
int number_function_item Args((void));
void format_options Args((char_t *));

                                                             /* sort.c */
short execute_sort Args((char_t *));

                                                           /* cursor.c */
short THEcursor_cmdline Args((char_t, VIEW_DETAILS *, short));
short THEcursor_column Args((void));
short THEcursor_down Args((char_t, VIEW_DETAILS *, short));
short THEcursor_file Args((bool, line_t, length_t));
short THEcursor_home Args((char_t, VIEW_DETAILS *, bool));
short THEcursor_left Args((short, bool));
short THEcursor_right Args((short, bool));
short THEcursor_up Args((short));
short THEcursor_move Args((char_t, VIEW_DETAILS *, bool, bool, short, short));
short THEcursor_goto Args((line_t, length_t));
short THEcursor_mouse Args((void));
long where_now Args((void));
long what_current_now Args((void));
long what_other_now Args((void));
long where_next Args((long, long, long));
long where_before Args((long, long, long));
bool enterable_field Args((long));
short go_to_new_field Args((long, long));
void get_cursor_position Args((line_t *, length_t *, line_t *, length_t *));
short advance_focus_line Args((line_t));
short advance_current_line Args((line_t));
short advance_current_or_focus_line Args((line_t));
void resolve_current_and_focus_lines Args((char_t, VIEW_DETAILS *, line_t, line_t, short, bool, bool));

                                                           /* colour.c */
short parse_colours Args((char_t *, COLOUR_ATTR *, char_t **, bool, bool *));
short parse_modifiers Args((char_t *, COLOUR_ATTR *));
chtype merge_curline_colour Args((COLOUR_ATTR *, COLOUR_ATTR *));
void set_up_default_colours Args((FILE_DETAILS *, COLOUR_ATTR *, int));
void set_up_default_ecolours Args((FILE_DETAILS *));
char_t *get_colour_strings Args((COLOUR_ATTR *));
int is_valid_colour Args((char_t * colour));

                                                           /* column.c */
short column_command Args((char_t *, int));

                                                            /* mouse.c */
void wmouse_position Args((WINDOW *, int *, int *));
short THEMouse Args((char_t *));
short get_mouse_info Args((int *, int *, int *));
void which_window_is_mouse_in Args((char_t *, int *));
void reset_saved_mouse_pos Args((void));
void get_saved_mouse_pos Args((int *, int *));
void initialise_mouse_commands Args((void));
int mouse_info_to_key Args((int, int, int, int));
char_t *mouse_key_number_to_name Args((int, char_t *, int *));
int find_mouse_key_value Args((char_t *));
int find_mouse_key_value_in_window Args((char_t *, char_t *));
short ScrollbarHorz Args((char_t *));
short ScrollbarVert Args((char_t *));

                                                           /* single.c */
int initialise_fifo Args((LINE * first_file_name, line_t startup_line, length_t startup_column, bool ro));
int process_fifo_input Args((int key));
void close_fifo Args((void));

                                                            /* comm*.c */
short Add Args((char_t *));
short Alert Args((char_t *));
short All Args((char_t *));
short Alt Args((char_t *));
short Arbchar Args((char_t *));
short Autocolour Args((char_t *));
short Autosave Args((char_t *));
short Autoscroll Args((char_t *));
short Backup Args((char_t *));
short Backward Args((char_t *));
short BeepSound Args((char_t *));
short Bottom Args((char_t *));
short Boundmark Args((char_t *));
short Cappend Args((char_t *));
short Cancel Args((char_t *));
short Case Args((char_t *));
short Ccancel Args((char_t *));
short Cdelete Args((char_t *));
short Cfirst Args((char_t *));
short Change Args((char_t *));
short Cinsert Args((char_t *));
short Clast Args((char_t *));
short THEClipboard Args((char_t *));
short Clearerrorkey Args((char_t *));
short Clearscreen Args((char_t *));
short Clocate Args((char_t *));
short Clock Args((char_t *));
short Cmatch Args((char_t *));
short Cmdarrows Args((char_t *));
short Cmdline Args((char_t *));
short Cmsg Args((char_t *));
short Colour Args((char_t *));
short Colouring Args((char_t *));
short Compat Args((char_t *));
short Compress Args((char_t *));
short THECommand Args((char_t *));
short ControlChar Args((char_t *));
short Copy Args((char_t *));
short Coverlay Args((char_t *));
short Creplace Args((char_t *));
short Ctlchar Args((char_t *));
short Curline Args((char_t *));
short Cursor Args((char_t *));
short CursorStay Args((char_t *));
short Define Args((char_t *));
short Defsort Args((char_t *));
short DeleteLine Args((char_t *));
short Dialog Args((char_t *));
short Directory Args((char_t *));
short Display Args((char_t *));
short Duplicate Args((char_t *));
short Ecolour Args((char_t *));
short Emsg Args((char_t *));
short THEEditv Args((char_t *));
short Enter Args((char_t *));
short Eolout Args((char_t *));
short Equivchar Args((char_t *));
short Errorformat Args((char_t *));
short Erroroutput Args((char_t *));
short Etmode Args((char_t *));
short Expand Args((char_t *));
short Extract Args((char_t *));
short Ffile Args((char_t *));
short File Args((char_t *));
short THEFiletabs Args((char_t *));
short Fillbox Args((char_t *));
short Find Args((char_t *));
short Findup Args((char_t *));
short Fext Args((char_t *));
short Filename Args((char_t *));
short Fmode Args((char_t *));
short Fname Args((char_t *));
short Forward Args((char_t *));
short Fpath Args((char_t *));
short Fullfname Args((char_t *));
short Get Args((char_t *));
short THEHeader Args((char_t *));
short Help Args((char_t *));
short Hex Args((char_t *));
short Hexdisplay Args((char_t *));
short Hexshow Args((char_t *));
short Highlight Args((char_t *));
short Hit Args((char_t *));
short Idline Args((char_t *));
short Impmacro Args((char_t *));
short Impos Args((char_t *));
short Input Args((char_t *));
short Inputmode Args((char_t *));
short Insertmode Args((char_t *));
short THEInterface Args((char_t *));
short Join Args((char_t *));
short Lastop Args((char_t *));
short Left Args((char_t *));
short Lineflag Args((char_t *));
short Linend Args((char_t *));
short Locate Args((char_t *));
short Lowercase Args((char_t *));
short Macro Args((char_t *));
short SetMacro Args((char_t *));
short Macroext Args((char_t *));
short Macropath Args((char_t *));
short Margins Args((char_t *));
short Mark Args((char_t *));
short Modify Args((char_t *));
short Mouse Args((char_t *));
short THEMove Args((char_t *));
short Msg Args((char_t *));
short Msgline Args((char_t *));
short Msgmode Args((char_t *));
short Newlines Args((char_t *));
short THENext Args((char_t *));
short Nextwindow Args((char_t *));
short Nfind Args((char_t *));
short Nfindup Args((char_t *));
short Nomsg Args((char_t *));
short Nondisp Args((char_t *));
short Nop Args((char_t *));
short Number Args((char_t *));
short Overlaybox Args((char_t *));
short Os Args((char_t *));
short Osnowait Args((char_t *));
short Osquiet Args((char_t *));
short Osredir Args((char_t *));
short Pagewrap Args((char_t *));
short Parser Args((char_t *));
short Pending Args((char_t *));
short Point Args((char_t *));
short Popup Args((char_t *));
short Position Args((char_t *));
short Prefix Args((char_t *));
short Preserve Args((char_t *));
short Prevwindow Args((char_t *));
short Print Args((char_t *));
short Pscreen Args((char_t *));
short THEPrinter Args((char_t *));
short Put Args((char_t *));
short Putd Args((char_t *));
short Qquit Args((char_t *));
short Quit Args((char_t *));
short Query Args((char_t *));
short THEReadonly Args((char_t *));
short Readv Args((char_t *));
short THERecord Args((char_t *));
short Recover Args((char_t *));
short Reexecute Args((char_t *));
short Redit Args((char_t *));
short Redraw Args((char_t *));
short THERefresh Args((char_t *));
short Repeat Args((char_t *));
short Replace Args((char_t *));
short Reprofile Args((char_t *));
short Reserved Args((char_t *));
short Reset Args((char_t *));
short Restore Args((char_t *));
short Retrieve Args((char_t *));
short Rexxhalt Args((char_t *));
short Rexxoutput Args((char_t *));
short THERexx Args((char_t *));
short Rgtleft Args((char_t *));
short Right Args((char_t *));
short Save Args((char_t *));
short Scope Args((char_t *));
short Scale Args((char_t *));
short THESearch Args((char_t *));
short Select Args((char_t *));
short Set Args((char_t *));
short Schange Args((char_t *));
short Slk Args((char_t *));
short THEScreen Args((char_t *));
short Shadow Args((char_t *));
short Shift Args((char_t *));
short ShowKey Args((char_t *));
short Sort Args((char_t *));
short Sos Args((char_t *));
short Sos_addline Args((char_t *));
short Sos_blockend Args((char_t *));
short Sos_blockstart Args((char_t *));
short Sos_bottomedge Args((char_t *));
short Sos_cuadelback Args((char_t *));
short Sos_cuadelchar Args((char_t *));
short Sos_current Args((char_t *));
short do_Sos_current Args((char_t *, char_t, VIEW_DETAILS *));
short Sos_cursoradj Args((char_t *));
short Sos_cursorshift Args((char_t *));
short Sos_delback Args((char_t *));
short Sos_delchar Args((char_t *));
short Sos_delend Args((char_t *));
short Sos_delline Args((char_t *));
short Sos_delword Args((char_t *));
short Sos_doprefix Args((char_t *));
short Sos_edit Args((char_t *));
short Sos_endchar Args((char_t *));
short Sos_execute Args((char_t *));
short Sos_firstchar Args((char_t *));
short Sos_firstcol Args((char_t *));
short Sos_instab Args((char_t *));
short Sos_lastcol Args((char_t *));
short Sos_leftedge Args((char_t *));
short Sos_makecurr Args((char_t *));
short Sos_marginl Args((char_t *));
short Sos_marginr Args((char_t *));
short Sos_pastecmdline Args((char_t *));
short Sos_parindent Args((char_t *));
short Sos_prefix Args((char_t *));
short do_Sos_prefix Args((char_t *, char_t, VIEW_DETAILS *));
short Sos_qcmnd Args((char_t *));
short Sos_rightedge Args((char_t *));
short Sos_settab Args((char_t *));
short Sos_startendchar Args((char_t *));
short Sos_tabb Args((char_t *));
short Sos_tabf Args((char_t *));
short Sos_tabfieldb Args((char_t *));
short Sos_tabfieldf Args((char_t *));
short Sos_tabwordb Args((char_t *));
short Sos_tabwordf Args((char_t *));
short Sos_topedge Args((char_t *));
short Sos_undo Args((char_t *));
short Span Args((char_t *));
short Spill Args((char_t *));
short Split Args((char_t *));
short Spltjoin Args((char_t *));
short Ssave Args((char_t *));
short Statopt Args((char_t *));
short Status Args((char_t *));
short Statusline Args((char_t *));
short Stay Args((char_t *));
short Suspend Args((char_t *));
short Synonym Args((char_t *));
short Tabfile Args((char_t *));
short Tabkey Args((char_t *));
short Tabline Args((char_t *));
short Tabpre Args((char_t *));
short Tabs Args((char_t *));
short Tabsin Args((char_t *));
short Tabsout Args((char_t *));
short Tag Args((char_t *));
short Targetsave Args((char_t *));
short Text Args((char_t *));
short THighlight Args((char_t *));
short Timecheck Args((char_t *));
short Toascii Args((char_t *));
short Tofeof Args((char_t *));
short Top Args((char_t *));
short Trailing Args((char_t *));
short Trunc Args((char_t *));
short THETypeahead Args((char_t *));
short Undoing Args((char_t *));
short Untaa Args((char_t *));
short Up Args((char_t *));
short Uppercase Args((char_t *));
short Verify Args((char_t *));
short Width Args((char_t *));
short Word Args((char_t *));
short Wordwrap Args((char_t *));
short Wrap Args((char_t *));
short Xedit Args((char_t *));
short Zone Args((char_t *));
