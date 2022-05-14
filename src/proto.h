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

/*
$Id: proto.h,v 1.2 1999/07/06 04:06:04 mark Exp mark $
*/

#ifdef HAVE_PROTO
                                                         /* commutil.c */
CHARTYPE *get_key_name(int,int *);
CHARTYPE *get_key_definition(int,bool,bool,bool);
short function_key(int,int,bool);
CHARTYPE *build_default_key_definition(int, CHARTYPE *);
short display_all_keys(void);
short command_line(CHARTYPE *,bool);
void cleanup_command_line(void);
void split_command(CHARTYPE *,CHARTYPE *,CHARTYPE *);
short param_split(CHARTYPE *,CHARTYPE *[],short,CHARTYPE *,CHARTYPE,CHARTYPE *,bool);
short command_split(CHARTYPE *,CHARTYPE *[],short,CHARTYPE *,CHARTYPE *);
LINETYPE get_true_line(bool);
CHARTYPE next_char(LINE *,long *,LENGTHTYPE);
short add_define(DEFINE **,DEFINE **,int,CHARTYPE *,bool);
short remove_define(DEFINE **,DEFINE **,int);
short append_define(DEFINE **,DEFINE **,int,short,CHARTYPE *,CHARTYPE *,int);
int find_key_value(CHARTYPE *);
short find_command(CHARTYPE *,bool);
void init_command(void);
void add_command(CHARTYPE *);
CHARTYPE *get_next_command( short,short );
bool valid_command_to_save(CHARTYPE *);
bool is_tab_col(LENGTHTYPE);
LENGTHTYPE find_next_tab_col(LENGTHTYPE);
LENGTHTYPE find_prev_tab_col(LENGTHTYPE);
short tabs_convert(LINE *,bool,bool,bool);
short convert_hex_strings(CHARTYPE *);
short marked_block(bool);
short suspend_curses(void);
short resume_curses(void);
short restore_THE(void);
short execute_set_sos_command(bool,CHARTYPE *);
short valid_command_type(bool,CHARTYPE *);
short allocate_temp_space(unsigned short,CHARTYPE);
void free_temp_space(CHARTYPE);
#ifdef MSWIN
void init_temp_space(void);
#endif
CHARTYPE calculate_actual_row(CHARTYPE,short,CHARTYPE,bool);
short get_valid_macro_file_name(CHARTYPE *,CHARTYPE *,CHARTYPE *,short *);
bool define_command(CHARTYPE *);
int find_key_name(CHARTYPE *);
int readv_cmdline(CHARTYPE *, WINDOW *, int);
short execute_mouse_commands(int);
short validate_n_m(CHARTYPE *,short *,short *);

                                                            /* print.c */
#ifdef WIN32
void StartTextPrnt(void);
void StopTextPrnt(void);
#endif
void print_line(bool ,LINETYPE,LINETYPE ,short ,CHARTYPE *,CHARTYPE *,short);
short setprintername(char*);
short setfontname(char*);
short setfontcpi(int);
short setfontlpi(int);
short setorient(char);
short setpagesize(int);
                                                           /* target.c */
short split_change_params(CHARTYPE *,CHARTYPE **,CHARTYPE **,TARGET *,LINETYPE *,LINETYPE *);
short parse_target(CHARTYPE *,LINETYPE,TARGET *,short,bool,bool,bool);
void initialise_target(TARGET *);
void free_target(TARGET *);
short find_target(TARGET *,LINETYPE,bool,bool);
short find_column_target(CHARTYPE *,LENGTHTYPE,TARGET *,LENGTHTYPE,bool,bool);
LINE *find_named_line(CHARTYPE *,LINETYPE *,bool);
short find_string_target(LINE *,RTARGET *);
short find_rtarget_target(LINE *,TARGET *,LINETYPE,LINETYPE,LINETYPE *);
bool find_rtarget_column_target(CHARTYPE *,LENGTHTYPE,TARGET *,LENGTHTYPE,LENGTHTYPE,LINETYPE *);
LINETYPE find_next_in_scope(VIEW_DETAILS *,LINE *,LINETYPE,short);
LINETYPE find_last_not_in_scope(VIEW_DETAILS *,LINE *,LINETYPE,short);
short validate_target(CHARTYPE *,TARGET *,short,LINETYPE,bool,bool);
void calculate_scroll_values(short *,LINETYPE *,LINETYPE *,bool *,bool *,bool *,short);
short find_first_focus_line(unsigned short *);
short find_last_focus_line(unsigned short *);
CHARTYPE find_unique_char(CHARTYPE *);
                                                         /* reserved.c */
RESERVED *add_reserved_line(CHARTYPE *,CHARTYPE *,short,short,COLOUR_ATTR *);
RESERVED *find_reserved_line(CHARTYPE,bool,ROWTYPE,short,short);
short delete_reserved_line(short,short);
#ifdef CTLCHAR
chtype *apply_ctlchar_to_reserved_line(RESERVED *);
#endif
                                                              /* box.c */
void box_operations(short ,CHARTYPE ,bool ,CHARTYPE );
                                                          /* execute.c */
short execute_os_command(CHARTYPE *,bool ,bool );
short execute_change_command(CHARTYPE *,bool );
short selective_change(CHARTYPE *,short,CHARTYPE *,short,LINETYPE ,LINETYPE ,short );
short insert_new_line(CHARTYPE *,unsigned short,LINETYPE,LINETYPE,bool,bool,bool,CHARTYPE,bool,bool);
short execute_makecurr(LINETYPE);
short execute_shift_command(short,short,LINETYPE,LINETYPE,bool,short,bool);
short execute_set_lineflag( unsigned int, unsigned int, unsigned int, LINETYPE, LINETYPE, bool, short );
short execute_change_case(CHARTYPE *,CHARTYPE);
short rearrange_line_blocks(CHARTYPE,CHARTYPE,LINETYPE,LINETYPE,LINETYPE,short,VIEW_DETAILS*,VIEW_DETAILS*,bool,LINETYPE *);
short execute_set_point(CHARTYPE *,LINETYPE ,bool);
short execute_wrap_word(unsigned short);
short execute_split_join(short,bool,bool);
short execute_put(CHARTYPE *,bool);
short execute_macro(CHARTYPE *,bool, short*);
short execute_set_on_off(CHARTYPE *,bool *);
short execute_set_row_position(CHARTYPE *,short *,short *);
short processable_line(VIEW_DETAILS *,LINETYPE,LINE *);
short execute_expand_compress(CHARTYPE *,bool,bool,bool,bool);
short execute_select(CHARTYPE *,bool,short);
short execute_move_cursor(LENGTHTYPE);
short execute_find_command(CHARTYPE *,short);
short execute_modify_command(CHARTYPE *);
LENGTHTYPE calculate_rec_len(short,LENGTHTYPE,LENGTHTYPE,LINETYPE);
short execute_editv(short,bool,CHARTYPE *);
short prepare_dialog(CHARTYPE *,bool,CHARTYPE *);
short execute_dialog(CHARTYPE *,CHARTYPE *,CHARTYPE *,bool,short,short,CHARTYPE *,short,bool);
short execute_popup(int , int , int , int , int , CHARTYPE **);
                                                          /* default.c */
void set_global_defaults(void);
void set_global_look_defaults(void);
void set_global_feel_defaults(void);
void set_file_defaults(FILE_DETAILS *);
void set_view_defaults(VIEW_DETAILS *);
short get_profile(CHARTYPE *,CHARTYPE *);
short defaults_for_first_file(void);
short defaults_for_other_files(VIEW_DETAILS *);
short default_file_attributes(FILE_DETAILS *);
void set_screen_defaults(void);
void set_defaults(void);
short set_THE_key_defaults(int,int);
short set_XEDIT_key_defaults(int,int);
short set_KEDIT_key_defaults(int,int);
short construct_default_parsers(void);
short destroy_all_parsers(void);
short construct_default_parser_mapping(void);
CHARTYPE *find_default_parser(CHARTYPE *,CHARTYPE *);
                                                             /* edit.c */
void editor(void);
int process_key(int,bool);
short EditFile(CHARTYPE *,bool);
                                                            /* error.c */
void display_error(unsigned short ,CHARTYPE *,bool);
void clear_msgline(int);
void display_prompt(CHARTYPE *);
void expose_msgline(void);
                                                             /* file.c */
short get_file(CHARTYPE *);
LINE *read_file(FILE *,LINE *,CHARTYPE *,LINETYPE,LINETYPE,bool);
LINE *read_fixed_file(FILE *,LINE *,CHARTYPE *,LINETYPE,LINETYPE);
short save_file(FILE_DETAILS *,CHARTYPE *,bool,LINETYPE,LINETYPE,LINETYPE *,bool,LENGTHTYPE,LENGTHTYPE,bool,bool,bool);
void increment_alt(FILE_DETAILS *);
CHARTYPE *new_filename(CHARTYPE *,CHARTYPE *,CHARTYPE *,CHARTYPE *);
short remove_aus_file(FILE_DETAILS *);
short free_view_memory(bool,bool);
void free_a_view(void);
short free_file_memory(bool);
short read_directory(void);
VIEW_DETAILS *find_file(CHARTYPE *,CHARTYPE *);
short execute_command_file(FILE *);
short process_command_line(CHARTYPE *,short);
CHARTYPE *read_file_into_memory(CHARTYPE *,int *);
                                                            /* getch.c */
#if !defined(DOS) && !defined(OS2)
int my_getch (WINDOW *);
#endif
                                                          /* nonansi.c */
short file_readable(CHARTYPE *);
short file_writable(CHARTYPE *);
short file_exists(CHARTYPE *);
short remove_file(CHARTYPE *);
short splitpath(CHARTYPE *);
#ifndef HAVE_RENAME
short rename(CHARTYPE *,CHARTYPE *);
#endif
#ifdef OS2
bool LongFileNames(CHARTYPE *);
bool IsPathAndFilenameValid(CHARTYPE *);
#endif
LINE *getclipboard(LINE *);
short setclipboard(FILE_DETAILS *,CHARTYPE *,bool,LINETYPE,LINETYPE,LINETYPE *,bool,LENGTHTYPE,LENGTHTYPE,bool,bool);
                                                           /* parser.c */
short parse_line(CHARTYPE,FILE_DETAILS *,SHOW_LINE *,short);
short parse_paired_comments(CHARTYPE,FILE_DETAILS *);
short construct_parser(CHARTYPE *, int, PARSER_DETAILS **,CHARTYPE *,CHARTYPE *);
short destroy_parser(PARSER_DETAILS *);
bool find_parser_mapping(FILE_DETAILS *, PARSER_MAPPING *);
PARSER_DETAILS *find_auto_parser(FILE_DETAILS *);
short parse_reserved_line(RESERVED *);
                                                           /* prefix.c */
short execute_prefix_commands(void);
void clear_pending_prefix_command(THE_PPC *,LINE *);
THE_PPC *delete_pending_prefix_command(THE_PPC *,FILE_DETAILS *,LINE *);
void add_prefix_command(LINE *,LINETYPE,bool);
short add_prefix_synonym(CHARTYPE *,CHARTYPE *);
CHARTYPE *find_prefix_synonym(CHARTYPE *);
CHARTYPE *find_prefix_oldname(CHARTYPE *);
CHARTYPE *get_prefix_command(LINETYPE);
                                                             /* show.c */
void show_heading(CHARTYPE);
void show_statarea(void);
void clear_statarea(void);
void build_screen(CHARTYPE);
void display_screen(CHARTYPE);
void show_marked_block(void);
void redraw_window(WINDOW *);
void repaint_screen(void);
void touch_screen(CHARTYPE);
void refresh_screen(CHARTYPE);
void redraw_screen(CHARTYPE);
bool line_in_view(CHARTYPE,LINETYPE);
bool column_in_view(CHARTYPE,LENGTHTYPE);
LINETYPE find_next_current_line(LINETYPE,short);
short get_row_for_focus_line(CHARTYPE,LINETYPE,short);
LINETYPE get_focus_line_in_view(CHARTYPE,LINETYPE,unsigned short);
LINETYPE calculate_focus_line(LINETYPE,LINETYPE);
void get_current_position(CHARTYPE,LINETYPE *,LENGTHTYPE *);
void calculate_new_column(COLTYPE,LENGTHTYPE,LENGTHTYPE,COLTYPE *,LENGTHTYPE *);
short prepare_view(CHARTYPE);
short advance_view(VIEW_DETAILS *,short);
short force_curses_background(void);
short THE_Resize(int,int);
                                                           /* scroll.c */
short scroll_page(short,LINETYPE,bool);
short scroll_line(short,LINETYPE,bool,bool);
                                                              /* the.c */
void init_colour_pairs(void);
int setup_profile_files(CHARTYPE *);
void cleanup(void);
int allocate_working_memory(void);
char **StringToArgv( int *, char* );
                                                             /* util.c */
CHARTYPE *ebc2asc(CHARTYPE *, int, int, int);
CHARTYPE *asc2ebc(CHARTYPE *, int, int, int);
short memreveq(CHARTYPE *,CHARTYPE ,short );
short memrevne(CHARTYPE *,CHARTYPE ,short );
CHARTYPE *meminschr(CHARTYPE *,CHARTYPE ,short ,short ,short );
CHARTYPE *meminsmem(CHARTYPE *,CHARTYPE *,short ,short ,short ,short );
CHARTYPE *memdeln(CHARTYPE *,short ,short,short );
CHARTYPE *strdelchr(CHARTYPE *,CHARTYPE);
CHARTYPE *memrmdup(CHARTYPE *,short *,CHARTYPE);
CHARTYPE *strrmdup(CHARTYPE *,CHARTYPE,bool);
short strzne(CHARTYPE *,CHARTYPE );
CHARTYPE *my_strdup(CHARTYPE *);
short memne(CHARTYPE *,CHARTYPE ,short );
short strzrevne(CHARTYPE *,CHARTYPE );
short strzreveq(CHARTYPE *,CHARTYPE );
CHARTYPE *strtrunc(CHARTYPE *);
CHARTYPE *MyStrip(CHARTYPE *,char,char);
short memfind(CHARTYPE *,CHARTYPE *,short ,short ,bool ,bool ,CHARTYPE ,CHARTYPE);
short memcmpi(CHARTYPE *,CHARTYPE *,short );
short my_strcmpi(char *,char * );
CHARTYPE *make_upper(CHARTYPE *);
unsigned short equal(CHARTYPE *,CHARTYPE *,short );
short valid_integer(CHARTYPE *);
short valid_positive_integer(CHARTYPE *);
short strzeq(CHARTYPE *,CHARTYPE );
CHARTYPE *strtrans(CHARTYPE *,CHARTYPE,CHARTYPE);
LINE *add_LINE(LINE *,LINE *,CHARTYPE *,LENGTHTYPE,SELECTTYPE,bool);
LINE *append_LINE(LINE *,CHARTYPE *,LENGTHTYPE);
LINE *delete_LINE(LINE *,LINE *,LINE *,short);
void put_string(WINDOW *,short ,short ,CHARTYPE *,short);
void put_char(WINDOW *,chtype ,CHARTYPE );
short set_up_windows(short);
short draw_divider(void);
short create_statusline_window(void);
void pre_process_line(VIEW_DETAILS *,LINETYPE,LINE *);
short post_process_line(VIEW_DETAILS *,LINETYPE,LINE *,bool);
bool blank_field(CHARTYPE *);
void adjust_marked_lines(bool ,LINETYPE ,LINETYPE );
void adjust_pending_prefix(VIEW_DETAILS *,bool ,LINETYPE ,LINETYPE );
CHARTYPE case_translate(CHARTYPE );
void add_to_recovery_list(CHARTYPE *,LENGTHTYPE);
void get_from_recovery_list(short );
void free_recovery_list(void);
short my_wmove(WINDOW *,short ,short ,short ,short );
short my_isalphanum(CHARTYPE);
short get_row_for_tof_eof(short,CHARTYPE);
int search_query_item_array(void *, size_t, size_t, const char *, int );
int split_function_name(CHARTYPE *, int *);
char *thetmpnam(char *);

#if THIS_APPEARS_TO_NOT_BE_USED
WINDOW *adjust_window(WINDOW *,short ,short ,short ,short );
#endif

void draw_cursor(bool);
short my_wclrtoeol(WINDOW *);
short my_wdelch(WINDOW *);
short get_word(CHARTYPE *,LENGTHTYPE,LENGTHTYPE,LENGTHTYPE *,LENGTHTYPE *);
                                                           /* linked.c */
void *ll_add(void *,void *,unsigned short );
void *ll_del(void *,void *,short );
void ll_free(void *);
void *ll_find(void *,LINETYPE );
LINE *lll_add(LINE *,LINE *,unsigned short );
LINE *lll_del(LINE **,LINE **,LINE *,short );
LINE *lll_free(LINE *);
LINE *lll_find(LINE *,LINE *,LINETYPE,LINETYPE);
LINE *lll_locate(LINE *,CHARTYPE *);
VIEW_DETAILS *vll_add(VIEW_DETAILS *,VIEW_DETAILS *,unsigned short );
VIEW_DETAILS *vll_del(VIEW_DETAILS **,VIEW_DETAILS **,VIEW_DETAILS *,short );
DEFINE *dll_add(DEFINE *,DEFINE *,unsigned short );
DEFINE *dll_del(DEFINE **,DEFINE **,DEFINE *,short );
DEFINE *dll_free(DEFINE *);
THE_PPC *pll_add(THE_PPC *,THE_PPC *,unsigned short );
THE_PPC *pll_del(THE_PPC **,THE_PPC **,THE_PPC *,short );
THE_PPC *pll_free(THE_PPC *);
THE_PPC *pll_find(THE_PPC *,LINETYPE);
RESERVED *rll_add(RESERVED *,RESERVED *,unsigned short );
RESERVED *rll_del(RESERVED **,RESERVED **,RESERVED *,short );
RESERVED *rll_free(RESERVED *);
RESERVED *rll_find(RESERVED *,short);
PARSER_DETAILS *parserll_add(PARSER_DETAILS *,PARSER_DETAILS *,unsigned short );
PARSER_DETAILS *parserll_del(PARSER_DETAILS **,PARSER_DETAILS **,PARSER_DETAILS *,short );
PARSER_DETAILS *parserll_free(PARSER_DETAILS *);
PARSER_DETAILS *parserll_find(PARSER_DETAILS *,CHARTYPE *);
PARSE_KEYWORDS *parse_keywordll_add(PARSE_KEYWORDS *,PARSE_KEYWORDS *,unsigned short );
PARSE_KEYWORDS *parse_keywordll_del(PARSE_KEYWORDS **,PARSE_KEYWORDS **,PARSE_KEYWORDS *,short );
PARSE_KEYWORDS *parse_keywordll_free(PARSE_KEYWORDS *);
PARSE_FUNCTIONS *parse_functionll_add(PARSE_FUNCTIONS *,PARSE_FUNCTIONS *,unsigned short );
PARSE_FUNCTIONS *parse_functionll_del(PARSE_FUNCTIONS **,PARSE_FUNCTIONS **,PARSE_FUNCTIONS *,short );
PARSE_FUNCTIONS *parse_functionll_free(PARSE_FUNCTIONS *);
PARSE_HEADERS *parse_headerll_add(PARSE_HEADERS *,PARSE_HEADERS *,unsigned short );
PARSE_HEADERS *parse_headerll_free(PARSE_HEADERS *);
PARSER_MAPPING *mappingll_add(PARSER_MAPPING *,PARSER_MAPPING *,unsigned short );
PARSER_MAPPING *mappingll_del(PARSER_MAPPING **,PARSER_MAPPING **,PARSER_MAPPING *,short );
PARSER_MAPPING *mappingll_free(PARSER_MAPPING *);
PARSER_MAPPING *mappingll_find(PARSER_MAPPING *,CHARTYPE *,CHARTYPE *);
PARSE_COMMENTS *parse_commentsll_add(PARSE_COMMENTS *,PARSE_COMMENTS *,unsigned short );
PARSE_COMMENTS *parse_commentsll_del(PARSE_COMMENTS **,PARSE_COMMENTS **,PARSE_COMMENTS *,short );
PARSE_COMMENTS *parse_commentsll_free(PARSE_COMMENTS *);
PARSE_COMMENTS *parse_commentsll_find(PARSE_COMMENTS *,CHARTYPE *);
                                                             /* rexx.c */
unsigned long MyRexxRegisterFunctionExe(CHARTYPE *);
unsigned long MyRexxDeregisterFunction(CHARTYPE *);
short initialise_rexx(void);
short finalise_rexx(void);
short execute_macro_file(CHARTYPE *,CHARTYPE *,short *,bool);
short execute_macro_instore(CHARTYPE *,short *,CHARTYPE **,int *,int *,int);
short get_rexx_variable(CHARTYPE *,CHARTYPE **,int *);
short set_rexx_variable(CHARTYPE *,CHARTYPE *,short,short);
CHARTYPE *get_rexx_interpreter_version(CHARTYPE *);
                                                           /* os2eas.c */
#if defined(OS2)
bool ReadEAs(CHARTYPE *);
bool WriteEAs(CHARTYPE *);
#endif
                                                            /* query.c */
short find_query_item(CHARTYPE *,int,CHARTYPE *);
short show_status(void);
short save_status(CHARTYPE *);
short set_extract_variables(short);
short get_item_values(int,short,CHARTYPE *,CHARTYPE,LINETYPE,CHARTYPE *,LINETYPE);
int number_query_item( void );
int number_function_item( void );
void format_options( CHARTYPE * );
                                                         /* directry.c */
short set_dirtype(CHARTYPE *);
CHARTYPE *get_dirtype(CHARTYPE *);
                                                          /* thematch.c */
int thematch(CHARTYPE *,CHARTYPE *,int);
                                                             /* sort.c */
short execute_sort(CHARTYPE *);
                                                           /* cursor.c */
short THEcursor_cmdline(short);
short THEcursor_column(void);
short THEcursor_down(bool);
short THEcursor_file(LINETYPE,LENGTHTYPE);
short THEcursor_home(bool);
short THEcursor_left(bool,bool);
short THEcursor_right(bool,bool);
short THEcursor_up(bool);
short THEcursor_move(bool,short,short);
short THEcursor_mouse(void);
long where_now(void);
long what_current_now(void);
long what_other_now(void);
long where_next(long,long,long);
long where_before(long,long,long);
bool enterable_field(long);
short go_to_new_field(long,long);
void get_cursor_position(long*,long*,long*,long*);
short advance_focus_line(LINETYPE);
short advance_current_line(LINETYPE);
short advance_current_or_focus_line(LINETYPE);
void resolve_current_and_focus_lines(VIEW_DETAILS *, LINETYPE, LINETYPE , short, bool , bool);
                                                           /* colour.c */
short parse_colours(CHARTYPE *,COLOUR_ATTR *,CHARTYPE **,bool,bool*);
chtype merge_curline_colour(COLOUR_ATTR *, COLOUR_ATTR *);
void set_up_default_colours(FILE_DETAILS *,COLOUR_ATTR *,int);
void set_up_default_ecolours(FILE_DETAILS *);
CHARTYPE *get_colour_strings(COLOUR_ATTR *);
                                                           /* column.c */
short column_command(CHARTYPE *,int);
                                                            /* mouse.c */
#if defined(NCURSES_MOUSE_VERSION)
void wmouse_position(WINDOW *, int *, int *);
#endif
short THEMouse(CHARTYPE *);
short get_mouse_info(int *,int *,int *);
void which_window_is_mouse_in(CHARTYPE *,int *);
void reset_saved_mouse_pos(void);
void get_saved_mouse_pos(int *, int *);
void initialise_mouse_commands(void);
int mouse_info_to_key(int,int,int,int);
CHARTYPE *mouse_key_number_to_name(int,CHARTYPE *);
int find_mouse_key_value(CHARTYPE *,CHARTYPE *);
short ScrollbarHorz(CHARTYPE *);
short ScrollbarVert(CHARTYPE *);
                                                           /* memory.c */
void init_memory_table( void );
void free_memory_flists( void );
void *get_a_block( int );
void give_a_block( void * );
void *resize_a_block( void *, int );
void the_free_flists ( void );
                                                            /* comm*.c */
short Add(CHARTYPE *);
short Alert(CHARTYPE *);
short All(CHARTYPE *);
short Alt(CHARTYPE *);
short Arbchar(CHARTYPE *);
short Autocolour(CHARTYPE *);
short Autosave(CHARTYPE *);
short Autoscroll(CHARTYPE *);
short Backup(CHARTYPE *);
short Backward(CHARTYPE *);
short BeepSound(CHARTYPE *);
short Bottom(CHARTYPE *);
short Cappend(CHARTYPE *);
short Cancel(CHARTYPE *);
short Case(CHARTYPE *);
short Ccancel(CHARTYPE *);
short Cdelete(CHARTYPE *);
short Cfirst(CHARTYPE *);
short Change(CHARTYPE *);
short Cinsert(CHARTYPE *);
short Clast(CHARTYPE *);
short Clearerrorkey(CHARTYPE *);
short Clearscreen(CHARTYPE *);
short Clocate(CHARTYPE *);
short Clock(CHARTYPE *);
short Cmatch(CHARTYPE *);
short Cmdarrows(CHARTYPE *);
short Cmdline(CHARTYPE *);
short Cmsg(CHARTYPE *);
short Colour(CHARTYPE *);
short Colouring(CHARTYPE *);
short Compat(CHARTYPE *);
short Compress(CHARTYPE *);
short THECommand(CHARTYPE *);
short ControlChar(CHARTYPE *);
short Copy(CHARTYPE *);
short Coverlay(CHARTYPE *);
short Creplace(CHARTYPE *);
short Ctlchar(CHARTYPE *);
short Curline(CHARTYPE *);
short Cursor(CHARTYPE *);
short CursorStay(CHARTYPE *);
short Define(CHARTYPE *);
short Defsort(CHARTYPE *);
short DeleteLine(CHARTYPE *);
short Dialog(CHARTYPE *);
short Directory(CHARTYPE *);
short Dirinclude(CHARTYPE *);
short Display(CHARTYPE *);
short Duplicate(CHARTYPE *);
short Ecolour(CHARTYPE *);
short Emsg(CHARTYPE *);
short THEEditv(CHARTYPE *);
short Enter(CHARTYPE *);
short Eolout(CHARTYPE *);
short Etmode(CHARTYPE *);
short Expand(CHARTYPE *);
short Extract(CHARTYPE *);
short Ffile(CHARTYPE *);
short File(CHARTYPE *);
short Fillbox(CHARTYPE *);
short Find(CHARTYPE *);
short Findup(CHARTYPE *);
short Fext(CHARTYPE *);
short Filename(CHARTYPE *);
short Fmode(CHARTYPE *);
short Fname(CHARTYPE *);
short Forward(CHARTYPE *);
short Fpath(CHARTYPE *);
short Fullfname(CHARTYPE *);
short Get(CHARTYPE *);
short Help(CHARTYPE *);
short Hex(CHARTYPE *);
short Hexdisplay(CHARTYPE *);
short Hexshow(CHARTYPE *);
short Highlight(CHARTYPE *);
short Hit(CHARTYPE *);
short Idline(CHARTYPE *);
short Impmacro(CHARTYPE *);
short Impos(CHARTYPE *);
short Input(CHARTYPE *);
short Inputmode(CHARTYPE *);
short Insertmode(CHARTYPE *);
short Join(CHARTYPE *);
short Left(CHARTYPE *);
short Lineflag(CHARTYPE *);
short Linend(CHARTYPE *);
short Locate(CHARTYPE *);
short Lowercase(CHARTYPE *);
short Macro(CHARTYPE *);
short SetMacro(CHARTYPE *);
short Macroext(CHARTYPE *);
short Macropath(CHARTYPE *);
short Margins(CHARTYPE *);
short Mark(CHARTYPE *);
short Modify(CHARTYPE *);
short Mouse(CHARTYPE *);
short THEMove(CHARTYPE *);
short Msg(CHARTYPE *);
short Msgline(CHARTYPE *);
short Msgmode(CHARTYPE *);
short Newlines(CHARTYPE *);
short THENext(CHARTYPE *);
short Nextwindow(CHARTYPE *);
short Nfind(CHARTYPE *);
short Nfindup(CHARTYPE *);
short Nomsg(CHARTYPE *);
short Nondisp(CHARTYPE *);
short Nop(CHARTYPE *);
short Number(CHARTYPE *);
short Overlaybox(CHARTYPE *);
short Os(CHARTYPE *);
short Osnowait(CHARTYPE *);
short Osquiet(CHARTYPE *);
short Osredir(CHARTYPE *);
short Parser(CHARTYPE *);
short Pending(CHARTYPE *);
short Point(CHARTYPE *);
short Popup(CHARTYPE *);
short Position(CHARTYPE *);
short Prefix(CHARTYPE *);
short Preserve(CHARTYPE *);
short Prevwindow(CHARTYPE *);
short Print(CHARTYPE *);
short Pscreen(CHARTYPE *);
short THEPrinter(CHARTYPE *);
short Put(CHARTYPE *);
short Putd(CHARTYPE *);
short Qquit(CHARTYPE *);
short Quit(CHARTYPE *);
short Query(CHARTYPE *);
short THEReadonly(CHARTYPE *);
short Readv(CHARTYPE *);
short Recover(CHARTYPE *);
short Reexecute(CHARTYPE *);
short Redraw(CHARTYPE *);
short THERefresh(CHARTYPE *);
short Repeat(CHARTYPE *);
short Replace(CHARTYPE *);
short Reprofile(CHARTYPE *);
short Reserved(CHARTYPE *);
short Reset(CHARTYPE *);
short Restore(CHARTYPE *);
short Retrieve(CHARTYPE *);
short Rexxoutput(CHARTYPE *);
short THERexx(CHARTYPE *);
short Rgtleft(CHARTYPE *);
short Right(CHARTYPE *);
short Save(CHARTYPE *);
short Scope(CHARTYPE *);
short Scale(CHARTYPE *);
short Select(CHARTYPE *);
short Set(CHARTYPE *);
short Schange(CHARTYPE *);
short Slk(CHARTYPE *);
short THEScreen(CHARTYPE *);
short Shadow(CHARTYPE *);
short Shift(CHARTYPE *);
short ShowKey(CHARTYPE *);
short Sort(CHARTYPE *);
short Sos(CHARTYPE *);
short Sos_addline(CHARTYPE *);
short Sos_blockend(CHARTYPE *);
short Sos_blockstart(CHARTYPE *);
short Sos_bottomedge(CHARTYPE *);
short Sos_current(CHARTYPE *);
short Sos_cursoradj(CHARTYPE *);
short Sos_delback(CHARTYPE *);
short Sos_delchar(CHARTYPE *);
short Sos_delend(CHARTYPE *);
short Sos_delline(CHARTYPE *);
short Sos_delword(CHARTYPE *);
short Sos_doprefix(CHARTYPE *);
short Sos_edit(CHARTYPE *);
short Sos_endchar(CHARTYPE *);
short Sos_execute(CHARTYPE *);
short Sos_firstchar(CHARTYPE *);
short Sos_firstcol(CHARTYPE *);
short Sos_instab(CHARTYPE *);
short Sos_lastcol(CHARTYPE *);
short Sos_leftedge(CHARTYPE *);
short Sos_makecurr(CHARTYPE *);
short Sos_marginl(CHARTYPE *);
short Sos_marginr(CHARTYPE *);
short Sos_pastecmdline(CHARTYPE *);
short Sos_parindent(CHARTYPE *);
short Sos_prefix(CHARTYPE *);
short Sos_qcmnd(CHARTYPE *);
short Sos_rightedge(CHARTYPE *);
short Sos_settab(CHARTYPE *);
short Sos_startendchar(CHARTYPE *);
short Sos_tabb(CHARTYPE *);
short Sos_tabf(CHARTYPE *);
short Sos_tabfieldb(CHARTYPE *);
short Sos_tabfieldf(CHARTYPE *);
short Sos_tabwordb(CHARTYPE *);
short Sos_tabwordf(CHARTYPE *);
short Sos_topedge(CHARTYPE *);
short Sos_undo(CHARTYPE *);
short Span(CHARTYPE *);
short Spill(CHARTYPE *);
short Split(CHARTYPE *);
short Spltjoin(CHARTYPE *);
short Ssave(CHARTYPE *);
short Statopt(CHARTYPE *);
short Status(CHARTYPE *);
short Statusline(CHARTYPE *);
short Stay(CHARTYPE *);
short Suspend(CHARTYPE *);
short Synonym(CHARTYPE *);
short Tabkey(CHARTYPE *);
short Tabline(CHARTYPE *);
short Tabpre(CHARTYPE *);
short Tabs(CHARTYPE *);
short Tabsin(CHARTYPE *);
short Tabsout(CHARTYPE *);
short Tag(CHARTYPE *);
short Text(CHARTYPE *);
short Timecheck(CHARTYPE *);
short Toascii(CHARTYPE *);
short Tofeof(CHARTYPE *);
short Top(CHARTYPE *);
short Trunc(CHARTYPE *);
short THETypeahead(CHARTYPE *);
short Undoing(CHARTYPE *);
short Untaa(CHARTYPE *);
short Up(CHARTYPE *);
short Uppercase(CHARTYPE *);
short Verify(CHARTYPE *);
short Width(CHARTYPE *);
short Word(CHARTYPE *);
short Wordwrap(CHARTYPE *);
short Wrap(CHARTYPE *);
short Xedit(CHARTYPE *);
short Xterminal(CHARTYPE *);
short Zone(CHARTYPE *);

#else
                                                         /* commutil.c */
CHARTYPE *get_key_name();
CHARTYPE *get_key_definition();
short function_key();
CHARTYPE *build_default_key_definition();
short display_all_keys();
short command_line();
void cleanup_command_line();
void split_command();
short param_split();
short command_split();
LINETYPE get_true_line();
CHARTYPE next_char();
short add_define();
short remove_define();
short append_define();
int find_key_value();
short find_command();
void init_command();
void add_command();
CHARTYPE *get_next_command();
bool valid_command_to_save();
bool is_tab_col();
LENGTHTYPE find_next_tab_col();
LENGTHTYPE find_prev_tab_col();
short tabs_convert();
short convert_hex_strings();
short marked_block();
short suspend_curses();
short resume_curses();
short restore_THE();
short execute_set_sos_command();
short valid_command_type();
short allocate_temp_space();
void free_temp_space();
CHARTYPE calculate_actual_row();
short get_valid_macro_file_name();
bool define_command();
int find_key_name();
int readv_cmdline();
short execute_mouse_commands();
short validate_n_m();

                                                            /* print.c */
#ifdef WIN32
void StartTextPrnt();
void StopTextPrnt();
#endif
void print_line();
short setprintername();
short setfontname();
short setfontcpi();
short setfontlpi();
short setorient();
short setpagesize();
                                                           /* target.c */
short split_change_params();
short parse_target();
void initialise_target();
void free_target();
short find_target();
short find_column_target();
LINE *find_named_line();
short find_string_target();
short find_rtarget_target();
bool find_rtarget_column_target();
LINETYPE find_next_in_scope();
LINETYPE find_last_not_in_scope();
short validate_target();
void calculate_scroll_values();
short find_first_focus_line();
short find_last_focus_line();
CHARTYPE find_unique_char();
                                                         /* reserved.c */
RESERVED *add_reserved_line();
RESERVED *find_reserved_line();
short delete_reserved_line();
#ifdef CTLCHAR
chtype *apply_ctlchar_to_reserved_line();
#endif
                                                              /* box.c */
void box_operations();
                                                          /* execute.c */
short execute_os_command();
short execute_change_command();
short selective_change();
short insert_new_line();
short execute_makecurr();
short execute_shift_command();
short execute_set_lineflag();
short execute_change_case();
short rearrange_line_blocks();
short execute_set_point();
short execute_wrap_word();
short execute_split_join();
short execute_put();
short execute_macro();
short execute_set_on_off();
short execute_set_row_position();
short processable_line();
short execute_expand_compress();
short execute_select();
short execute_move_cursor();
short execute_find_command();
short execute_modify_command();
LENGTHTYPE calculate_rec_len();
short execute_editv();
short prepare_dialog();
short execute_dialog();
short execute_popup();
                                                          /* default.c */
void set_global_defaults();
void set_global_look_defaults();
void set_global_feel_defaults();
void set_file_defaults();
void set_view_defaults();
short get_profile();
short defaults_for_first_file();
short defaults_for_other_files();
short default_file_attributes();
void set_screen_defaults();
void set_defaults();
short set_THE_key_defaults();
short set_XEDIT_key_defaults();
short set_KEDIT_key_defaults();
short construct_default_parsers();
short destroy_all_parsers();
short construct_default_parser_mapping();
CHARTYPE *find_default_parser();
                                                             /* edit.c */
void editor();
int process_key();
short EditFile();
                                                            /* error.c */
void display_error();
void clear_msgline();
void display_prompt();
void expose_msgline();
                                                             /* file.c */
short get_file();
LINE *read_file();
LINE *read_fixed_file();
short save_file();
void increment_alt();
CHARTYPE *new_filename();
short remove_aus_file();
short free_view_memory();
void free_a_view();
short free_file_memory();
short read_directory();
VIEW_DETAILS *find_file();
short execute_command_file();
short process_command_line();
CHARTYPE *read_file_into_memory();
                                                            /* getch.c */
#if !defined(DOS) && !defined(OS2)
int my_getch ();
#endif
                                                          /* nonansi.c */
short file_readable();
short file_writable();
short file_exists();
short remove_file();
short splitpath();
#ifndef HAVE_RENAME
short rename();
#endif
#ifdef OS2
bool LongFileNames();
bool IsPathAndFilenameValid();
#endif
LINE *getclipboard();
short setclipboard();
                                                           /* parser.c */
short parse_line();
short parse_paired_comments();
short construct_parser();
short destroy_parser();
bool find_parser_mapping();
PARSER_DETAILS *find_auto_parser();
short parse_reserved_line();
                                                           /* prefix.c */
short execute_prefix_commands();
void clear_pending_prefix_command();
THE_PPC *delete_pending_prefix_command();
void add_prefix_command();
short add_prefix_synonym();
CHARTYPE *find_prefix_synonym();
CHARTYPE *find_prefix_oldname();
CHARTYPE *get_prefix_command();
                                                             /* show.c */
void show_heading();
void show_statarea();
void clear_statarea();
void build_screen();
void display_screen();
void show_marked_block();
void redraw_window();
void repaint_screen();
void touch_screen();
void refresh_screen();
void redraw_screen();
bool line_in_view();
bool column_in_view();
LINETYPE find_next_current_line();
short get_row_for_focus_line();
LINETYPE get_focus_line_in_view();
LINETYPE calculate_focus_line();
void get_current_position();
void calculate_new_column();
short prepare_view();
short advance_view();
short force_curses_background();
short THE_Resize();
                                                           /* scroll.c */
short scroll_page();
short scroll_line();
                                                              /* the.c */
void init_colour_pairs();
void display_info();
void cleanup();
int allocate_working_memory();
char **StringToArgv();
                                                             /* util.c */
CHARTYPE *ebc2asc();
CHARTYPE *asc2ebc();
short memreveq();
short memrevne();
CHARTYPE *meminschr();
CHARTYPE *meminsmem();
CHARTYPE *memdeln();
CHARTYPE *strdelchr();
CHARTYPE *memrmdup();
CHARTYPE *strrmdup();
short strzne();
CHARTYPE *my_strdup();
short memne();
short strzrevne();
short strzreveq();
CHARTYPE *strtrunc();
CHARTYPE *MyStrip();
short memfind();
short memcmpi();
short my_strcmpi();
CHARTYPE *make_upper();
unsigned short equal();
short valid_integer();
short valid_positive_integer();
short strzeq();
CHARTYPE *strtrans();
LINE *add_LINE();
LINE *append_LINE();
LINE *delete_LINE();
void put_string();
void put_char();
short set_up_windows();
short draw_divider();
short create_statusline_window();
void pre_process_line();
short post_process_line();
bool blank_field();
void adjust_marked_lines();
void adjust_pending_prefix();
CHARTYPE case_translate();
void add_to_recovery_list();
void get_from_recovery_list();
void free_recovery_list();
short my_isalphanum();
short my_wmove();
short get_row_for_tof_eof();
int search_query_item_array();
int split_function_name();
char *thetmpnam();

#if THIS_APPEARS_TO_NOT_BE_USED
WINDOW *adjust_window();
#endif

void draw_cursor();
short my_wclrtoeol();
short my_wdelch();
short get_word();
                                                           /* linked.c */
void *ll_add();
void *ll_del();
void ll_free();
void *ll_find();
LINE *lll_add();
LINE *lll_del();
LINE *lll_free();
LINE *lll_find();
LINE *lll_locate();
VIEW_DETAILS *vll_add();
VIEW_DETAILS *vll_del();
DEFINE *dll_add();
DEFINE *dll_del();
DEFINE *dll_free();
THE_PPC *pll_add();
THE_PPC *pll_del();
THE_PPC *pll_free();
THE_PPC *pll_find();
RESERVED *rll_add();
RESERVED *rll_del();
RESERVED *rll_free();
RESERVED *rll_find();
PARSER_DETAILS *parserll_add();
PARSER_DETAILS *parserll_del();
PARSER_DETAILS *parserll_free();
PARSER_DETAILS *parserll_find();
PARSE_KEYWORDS *parse_keywordll_add();
PARSE_KEYWORDS *parse_keywordll_del();
PARSE_KEYWORDS *parse_keywordll_free();
PARSE_FUNCTIONS *parse_functionll_add();
PARSE_FUNCTIONS *parse_functionll_del();
PARSE_FUNCTIONS *parse_functionll_free();
PARSER_MAPPING *mappingll_add();
PARSER_MAPPING *mappingll_del();
PARSER_MAPPING *mappingll_free();
PARSER_MAPPING *mappingll_find();
PARSE_COMMENTS *parse_commentsll_add();
PARSE_COMMENTS *parse_commentsll_del();
PARSE_COMMENTS *parse_commentsll_free();
PARSE_COMMENTS *parse_commentsll_find();
                                                             /* rexx.c */
unsigned long MyRexxRegisterFunctionExe();
unsigned long MyRexxDeregisterFunctionExe();
short initialise_rexx();
short finalise_rexx();
short execute_macro_file();
short execute_macro_instore();
short get_rexx_variable();
short set_rexx_variable();
CHARTYPE *get_rexx_interpreter_version();
                                                            /* query.c */
short find_query_item();
short show_status();
short save_status();
short set_extract_variables();
short get_item_values();
int number_query_item( );
int number_function_item( );
void format_options( );
                                                         /* directry.c */
short set_dirtype();
CHARTYPE *get_dirtype();
                                                          /* thematch.c */
int thematch();
                                                             /* sort.c */
short execute_sort();
                                                           /* cursor.c */
short THEcursor_cmdline();
short THEcursor_column();
short THEcursor_down();
short THEcursor_file();
short THEcursor_home();
short THEcursor_left();
short THEcursor_right();
short THEcursor_up();
short THEcursor_move();
short THEcursor_mouse();
long where_now();
long what_current_now();
long what_other_now();
long where_next();
long where_before();
bool enterable_field();
short go_to_new_field();
void get_cursor_position();
short advance_focus_line();
short advance_current_line();
short advance_current_or_focus_line();
void resolve_current_and_focus_lines();
                                                           /* colour.c */
short parse_colours();
chtype merge_curline_colour();
void set_up_default_colours();
void set_up_default_ecolours();
CHARTYPE *get_colour_strings();
                                                           /* column.c */
short column_command();
                                                            /* mouse.c */
#if defined(NCURSES_MOUSE_VERSION)
void wmouse_position();
#endif
short THEMouse();
short get_mouse_info();
void which_window_is_mouse_in();
void reset_saved_mouse_pos();
void get_saved_mouse_pos();
void initialise_mouse_commands();
int mouse_info_to_key();
CHARTYPE *mouse_key_number_to_name();
int find_mouse_key_value();
short ScrollbarHorz();
short ScrollbarVert();
                                                           /* memory.c */
void init_memory_table();
void free_memory_flists();
void *get_a_block();
void give_a_block();
void *resize_a_block();
void the_free_flists ();
                                                            /* comm*.c */
short Add();
short Alert();
short All();
short Alt();
short Arbchar();
short Autocolour();
short Autosave();
short Autoscroll();
short Backup();
short Backward();
short BeepSound();
short Bottom();
short Cappend();
short Cancel();
short Case();
short Ccancel();
short Cdelete();
short Cfirst();
short Change();
short Cinsert();
short Clast();
short Clearerrorkey();
short Clearscreen();
short Clocate();
short Clock();
short Cmatch();
short Cmdarrows();
short Cmdline();
short Cmsg();
short Colour();
short Colouring();
short Compat();
short Compress();
short THECommand();
short ControlChar();
short Copy();
short Coverlay();
short Creplace();
short Ctlchar();
short Curline();
short Cursor();
short CursorStay();
short Define();
short Defsort();
short DeleteLine();
short Dialog();
short Directory();
short Dirinclude();
short Display();
short Duplicate();
short THEEditv();
short Ecolour();
short Emsg();
short Enter();
short Eolout();
short Etmode();
short Expand();
short Extract();
short Ffile();
short File();
short Fillbox();
short Find();
short Findup();
short Fext();
short Filename();
short Fmode();
short Fname();
short Forward();
short Fpath();
short Fullfname();
short Get();
short Help();
short Hex();
short Hexdisplay();
short Hexshow();
short Highlight();
short Hit();
short Idline();
short Impmacro();
short Impos();
short Input();
short Inputmode();
short Insertmode();
short Join();
short Left();
short Lineflag();
short Linend();
short Locate();
short Lowercase();
short Macro();
short SetMacro();
short Macroext();
short Macropath();
short Margins();
short Mark();
short Modify();
short Mouse();
short THEMove();
short Msg();
short Msgline();
short Msgmode();
short Newlines();
short THENext();
short Nextwindow();
short Nfind();
short Nfindup();
short Nomsg();
short Nondisp();
short Nop();
short Number();
short THEOptions();
short Overlaybox();
short Os();
short Osnowait();
short Osquiet();
short Osredir();
short Parser();
short Pending();
short Point();
short Popup();
short Position();
short Prefix();
short Preserve();
short Prevwindow();
short Print();
short Pscreen();
short THEPrinter();
short Put();
short Putd();
short Qquit();
short Quit();
short Query();
short THEReadonly();
short Readv();
short Recover();
short Reexecute();
short Redraw();
short THERefresh();
short Repeat();
short Replace();
short Reprofile();
short Reserved();
short Reset();
short Restore();
short Retrieve();
short Rexxoutput();
short THERexx();
short Rgtleft();
short Right();
short Save();
short Scale();
short Scope();
short Select();
short Schange();
short Slk();
short THEScreen();
short Set();
short Shadow();
short Shift();
short ShowKey();
short Sort();
short Sos();
short Sos_addline();
short Sos_blockend();
short Sos_blockstart();
short Sos_bottomedge();
short Sos_current();
short Sos_cursoradj();
short Sos_delback();
short Sos_delchar();
short Sos_delend();
short Sos_delline();
short Sos_delword();
short Sos_doprefix();
short Sos_edit();
short Sos_endchar();
short Sos_execute();
short Sos_firstchar();
short Sos_firstcol();
short Sos_instab();
short Sos_lastcol();
short Sos_leftedge();
short Sos_makecurr();
short Sos_marginl();
short Sos_marginr();
short Sos_pastecmdline();
short Sos_parindent();
short Sos_prefix();
short Sos_qcmnd();
short Sos_rightedge();
short Sos_settab();
short Sos_startendchar();
short Sos_tabb();
short Sos_tabf();
short Sos_tabfieldb();
short Sos_tabfieldf();
short Sos_tabwordb();
short Sos_tabwordf();
short Sos_topedge();
short Sos_undo();
short Span();
short Spill();
short Split();
short Spltjoin();
short Ssave();
short Statopt();
short Status();
short Statusline();
short Stay();
short Suspend();
short Synonym();
short Tabkey();
short Tabline();
short Tabpre();
short Tabs();
short Tabsin();
short Tabsout();
short Tag();
short Text();
short Timecheck();
short Toascii();
short Tofeof();
short Top();
short Trunc();
short THETypeahead();
short Undoing();
short Untaa();
short Up();
short Uppercase();
short Verify();
short Width();
short Word();
short Wordwrap();
short Wrap();
short Xedit();
short Xterminal();
short Zone();

#endif
