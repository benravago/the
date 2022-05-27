// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Additions of extended function key descriptions made by Mark Hessling.
 */

// TODO: validate these mappings

#define KEY_A2          0x198   /* Upper centre of keypad */
#define KEY_B1          0x19a   /* Middle left of keypad */
#define KEY_B3          0x19c   /* Middle right of keypad */
#define KEY_C2          0x19e   /* Lower centre of keypad */

/* special definition for UNIX systems */

#define KEY_RETURN     0xd
#define KEY_NUMENTER   KEY_ENTER
#define KEY_BKSP       0x8
#define KEY_DELETE     127

/* vt220 special keys */

#define CTL_LEFT       0x302
#define CTL_RIGHT      0x303
#define CTL_UP         0x304
#define CTL_DOWN       0x305
#define CTL_HOME       0x306
#define CTL_END        0x307
#define CTL_PGUP       0x308
#define CTL_PGDN       0x309

#define CSI          0233
#define KEY_ESC      0x1b

/* control alpha keys */
#define KEY_C_a      0x1
#define KEY_C_b      0x2
#define KEY_C_c      0x3
#define KEY_C_d      0x4
#define KEY_C_e      0x5
#define KEY_C_f      0x6
#define KEY_C_g      0x7
#define KEY_C_h      0x8
#define KEY_C_i      0x9
#define KEY_C_j      0xa
#define KEY_C_k      0xb
#define KEY_C_l      0xc
#define KEY_C_m      0xd
#define KEY_C_n      0xe
#define KEY_C_o      0xf
#define KEY_C_p      0x10
#define KEY_C_q      0x11
#define KEY_C_r      0x12
#define KEY_C_s      0x13
#define KEY_C_t      0x14
#define KEY_C_u      0x15
#define KEY_C_v      0x16
#define KEY_C_w      0x17
#define KEY_C_x      0x18
#define KEY_C_y      0x19
#define KEY_C_z      0x1a

/* unshifted function keys */
#define KEY_F1      KEY_F(1)
#define KEY_F2      KEY_F(2)
#define KEY_F3      KEY_F(3)
#define KEY_F4      KEY_F(4)
#define KEY_F5      KEY_F(5)
#define KEY_F6      KEY_F(6)
#define KEY_F7      KEY_F(7)
#define KEY_F8      KEY_F(8)
#define KEY_F9      KEY_F(9)
#define KEY_F10     KEY_F(10)
#define KEY_F11     KEY_F(11)
#define KEY_F12     KEY_F(12)

/* shifted function keys */
#define KEY_S_F1      KEY_F(13)
#define KEY_S_F2      KEY_F(14)
#define KEY_S_F3      KEY_F(15)
#define KEY_S_F4      KEY_F(16)
#define KEY_S_F5      KEY_F(17)
#define KEY_S_F6      KEY_F(18)
#define KEY_S_F7      KEY_F(19)
#define KEY_S_F8      KEY_F(20)
#define KEY_S_F9      KEY_F(21)
#define KEY_S_F10     KEY_F(22)
#define KEY_S_F11     KEY_F(23)
#define KEY_S_F12     KEY_F(24)

/* control function keys */
#define KEY_C_F1      KEY_F(25)
#define KEY_C_F2      KEY_F(26)
#define KEY_C_F3      KEY_F(27)
#define KEY_C_F4      KEY_F(28)
#define KEY_C_F5      KEY_F(29)
#define KEY_C_F6      KEY_F(30)
#define KEY_C_F7      KEY_F(31)
#define KEY_C_F8      KEY_F(32)
#define KEY_C_F9      KEY_F(33)
#define KEY_C_F10     KEY_F(34)
#define KEY_C_F11     KEY_F(35)
#define KEY_C_F12     KEY_F(36)

/* alt function keys */
#define KEY_A_F1      KEY_F(37)
#define KEY_A_F2      KEY_F(38)
#define KEY_A_F3      KEY_F(39)
#define KEY_A_F4      KEY_F(40)
#define KEY_A_F5      KEY_F(41)
#define KEY_A_F6      KEY_F(42)
#define KEY_A_F7      KEY_F(43)
#define KEY_A_F8      KEY_F(44)
#define KEY_A_F9      KEY_F(45)
#define KEY_A_F10     KEY_F(46)
#define KEY_A_F11     KEY_F(47)
#define KEY_A_F12     KEY_F(48)

/* remaining curses function keys */
#define KEY_F13       KEY_F(49)
#define KEY_F14       KEY_F(50)
#define KEY_F15       KEY_F(51)
#define KEY_F16       KEY_F(52)
#define KEY_F17       KEY_F(53)
#define KEY_F18       KEY_F(54)
#define KEY_F19       KEY_F(55)
#define KEY_F20       KEY_F(56)
#define KEY_S_F13     KEY_F(57)
#define KEY_S_F14     KEY_F(58)
#define KEY_S_F15     KEY_F(59)
#define KEY_S_F16     KEY_F(60)
#define KEY_S_F17     KEY_F(61)
#define KEY_S_F18     KEY_F(62)
#define KEY_S_F19     KEY_F(63)

#define KEY_Find       KEY_FIND
#define KEY_InsertHere KEY_IC
#define KEY_Remove     KEY_REPLACE
#define KEY_Select     KEY_SELECT
#define KEY_PrevScreen KEY_PPAGE
#define KEY_NextScreen KEY_NPAGE
#define KEY_PF1        0x350
#define KEY_PF2        0x351
#define KEY_PF3        0x352
#define KEY_PF4        0x353
#define KEY_PadComma   0x354
#define KEY_PadMinus   0x355
#define KEY_PadPeriod  0x356
#define KEY_PadPlus    0x357
#define KEY_PadStar    0x358
#define KEY_PadSlash   0x359
#define KEY_Pad0       0x360


struct key_values {
  char *mnemonic;
  int key_value;
  int shift;
};
typedef struct key_values KEYS;

KEYS key_table[] = {
  { "A", (int) 'a', 0 },
  { "B", (int) 'b', 0 },
  { "C", (int) 'c', 0 },
  { "D", (int) 'd', 0 },
  { "E", (int) 'e', 0 },
  { "F", (int) 'f', 0 },
  { "G", (int) 'g', 0 },
  { "H", (int) 'h', 0 },
  { "I", (int) 'i', 0 },
  { "J", (int) 'j', 0 },
  { "K", (int) 'k', 0 },
  { "L", (int) 'l', 0 },
  { "M", (int) 'm', 0 },
  { "N", (int) 'n', 0 },
  { "O", (int) 'o', 0 },
  { "P", (int) 'p', 0 },
  { "Q", (int) 'q', 0 },
  { "R", (int) 'r', 0 },
  { "S", (int) 's', 0 },
  { "T", (int) 't', 0 },
  { "U", (int) 'u', 0 },
  { "V", (int) 'v', 0 },
  { "W", (int) 'w', 0 },
  { "X", (int) 'x', 0 },
  { "Y", (int) 'y', 0 },
  { "Z", (int) 'z', 0 },
  { "S-A", (int) 'A', SHIFT_SHIFT },
  { "S-B", (int) 'B', SHIFT_SHIFT },
  { "S-C", (int) 'C', SHIFT_SHIFT },
  { "S-D", (int) 'D', SHIFT_SHIFT },
  { "S-E", (int) 'E', SHIFT_SHIFT },
  { "S-F", (int) 'F', SHIFT_SHIFT },
  { "S-G", (int) 'G', SHIFT_SHIFT },
  { "S-H", (int) 'H', SHIFT_SHIFT },
  { "S-I", (int) 'I', SHIFT_SHIFT },
  { "S-J", (int) 'J', SHIFT_SHIFT },
  { "S-K", (int) 'K', SHIFT_SHIFT },
  { "S-L", (int) 'L', SHIFT_SHIFT },
  { "S-M", (int) 'M', SHIFT_SHIFT },
  { "S-N", (int) 'N', SHIFT_SHIFT },
  { "S-O", (int) 'O', SHIFT_SHIFT },
  { "S-P", (int) 'P', SHIFT_SHIFT },
  { "S-Q", (int) 'Q', SHIFT_SHIFT },
  { "S-R", (int) 'R', SHIFT_SHIFT },
  { "S-S", (int) 'S', SHIFT_SHIFT },
  { "S-T", (int) 'T', SHIFT_SHIFT },
  { "S-U", (int) 'U', SHIFT_SHIFT },
  { "S-V", (int) 'V', SHIFT_SHIFT },
  { "S-W", (int) 'W', SHIFT_SHIFT },
  { "S-X", (int) 'X', SHIFT_SHIFT },
  { "S-Y", (int) 'Y', SHIFT_SHIFT },
  { "S-Z", (int) 'Z', SHIFT_SHIFT },
  { "0", (int) '0', 0 },
  { "1", (int) '1', 0 },
  { "2", (int) '2', 0 },
  { "3", (int) '3', 0 },
  { "4", (int) '4', 0 },
  { "5", (int) '5', 0 },
  { "6", (int) '6', 0 },
  { "7", (int) '7', 0 },
  { "8", (int) '8', 0 },
  { "9", (int) '9', 0 },
  { "`", (int) '`', 0 },
  { "-", (int) '-', 0 },
  { "=", (int) '=', 0 },
  { "[", (int) '[', 0 },
  { "]", (int) ']', 0 },
  { "\\", (int) '\\', 0 },
  { ";", (int) ';', 0 },
  { "'", (int) '\'', 0 },
  { ",", (int) ',', 0 },
  { ".", (int) '.', 0 },
  { "/", (int) '/', 0 },
  { ")", (int) ')', 0 },
  { "!", (int) '!', 0 },
  { "@", (int) '@', 0 },
  { "#", (int) '#', 0 },
  { "$", (int) '$', 0 },
  { "%", (int) '%', 0 },
  { "^", (int) '^', 0 },
  { "&", (int) '&', 0 },
  { "*", (int) '*', 0 },
  { "(", (int) '(', 0 },
  { "~", (int) '~', 0 },
  { "_", (int) '_', 0 },
  { "+", (int) '+', 0 },
  { "{", (int) '{', 0 },
  { "}", (int) '}', 0 },
  { "|", (int) '|', 0 },
  { ":", (int) ':', 0 },
  { "\"", (int) '\"', 0 },
  { "<", (int) '<', 0 },
  { ">", (int) '>', 0 },
  { "?", (int) '?', 0 },
  { "SPACE", (int) ' ', 0 },
  { "ESC", KEY_ESC, 0 },
  { "F0", KEY_F0, 0 },
  { "F1", KEY_F1, 0 },
  { "F2", KEY_F2, 0 },
  { "F3", KEY_F3, 0 },
  { "F4", KEY_F4, 0 },
  { "F5", KEY_F5, 0 },
  { "F6", KEY_F6, 0 },
  { "F7", KEY_F7, 0 },
  { "F8", KEY_F8, 0 },
  { "F9", KEY_F9, 0 },
  { "F10", KEY_F10, 0 },
  { "F11", KEY_F11, 0 },
  { "F12", KEY_F12, 0 },
  { "S-F1", KEY_S_F1, SHIFT_SHIFT },
  { "S-F2", KEY_S_F2, SHIFT_SHIFT },
  { "S-F3", KEY_S_F3, SHIFT_SHIFT },
  { "S-F4", KEY_S_F4, SHIFT_SHIFT },
  { "S-F5", KEY_S_F5, SHIFT_SHIFT },
  { "S-F6", KEY_S_F6, SHIFT_SHIFT },
  { "S-F7", KEY_S_F7, SHIFT_SHIFT },
  { "S-F8", KEY_S_F8, SHIFT_SHIFT },
  { "S-F9", KEY_S_F9, SHIFT_SHIFT },
  { "S-F10", KEY_S_F10, SHIFT_SHIFT },
  { "S-F11", KEY_S_F11, SHIFT_SHIFT },
  { "S-F12", KEY_S_F12, SHIFT_SHIFT },
  { "C-F1", KEY_C_F1, SHIFT_CTRL },
  { "C-F2", KEY_C_F2, SHIFT_CTRL },
  { "C-F3", KEY_C_F3, SHIFT_CTRL },
  { "C-F4", KEY_C_F4, SHIFT_CTRL },
  { "C-F5", KEY_C_F5, SHIFT_CTRL },
  { "C-F6", KEY_C_F6, SHIFT_CTRL },
  { "C-F7", KEY_C_F7, SHIFT_CTRL },
  { "C-F8", KEY_C_F8, SHIFT_CTRL },
  { "C-F9", KEY_C_F9, SHIFT_CTRL },
  { "C-F10", KEY_C_F10, SHIFT_CTRL },
  { "C-F11", KEY_C_F11, SHIFT_CTRL },
  { "C-F12", KEY_C_F12, SHIFT_CTRL },
  { "A-F1", KEY_A_F1, SHIFT_ALT },
  { "A-F2", KEY_A_F2, SHIFT_ALT },
  { "A-F3", KEY_A_F3, SHIFT_ALT },
  { "A-F4", KEY_A_F4, SHIFT_ALT },
  { "A-F5", KEY_A_F5, SHIFT_ALT },
  { "A-F6", KEY_A_F6, SHIFT_ALT },
  { "A-F7", KEY_A_F7, SHIFT_ALT },
  { "A-F8", KEY_A_F8, SHIFT_ALT },
  { "A-F9", KEY_A_F9, SHIFT_ALT },
  { "A-F10", KEY_A_F10, SHIFT_ALT },
  { "A-F11", KEY_A_F11, SHIFT_ALT },
  { "A-F12", KEY_A_F12, SHIFT_ALT },
  { "F13", KEY_F13, 0 },
  { "F14", KEY_F14, 0 },
  { "F15", KEY_F15, 0 },
  { "F16", KEY_F16, 0 },
  { "F17", KEY_F17, 0 },
  { "F18", KEY_F18, 0 },
  { "F19", KEY_F19, 0 },
  { "F20", KEY_F20, 0 },
  { "S-F13", KEY_S_F13, SHIFT_SHIFT },
  { "S-F14", KEY_S_F14, SHIFT_SHIFT },
  { "S-F15", KEY_S_F15, SHIFT_SHIFT },
  { "S-F16", KEY_S_F16, SHIFT_SHIFT },
  { "S-F17", KEY_S_F17, SHIFT_SHIFT },
  { "S-F18", KEY_S_F18, SHIFT_SHIFT },
  { "S-F19", KEY_S_F19, SHIFT_SHIFT },
  { "NUM0", KEY_Pad0, 0 },
  { "NUM1", KEY_C1, 0 },
  { "NUM2", KEY_C2, 0 },
  { "NUM3", KEY_C3, 0 },
  { "NUM4", KEY_B1, 0 },
  { "CENTER", KEY_B2, 0 },
  { "NUM6", KEY_B3, 0 },
  { "NUM7", KEY_A1, 0 },
  { "NUM8", KEY_A2, 0 },
  { "NUM9", KEY_A3, 0 },

  { "BKSP", KEY_BKSP, 0 },
  { "ENTER", KEY_RETURN, 0 },
  { "ENTER", KEY_ENTER, 0 },

  { "CURD", KEY_DOWN, 0 },
  { "CURU", KEY_UP, 0 },
  { "CURL", KEY_LEFT, 0 },
  { "CURR", KEY_RIGHT, 0 },
  { "BACKSPACE", KEY_BACKSPACE, 0 },
  { "HOME", KEY_HOME, 0 },
  { "PF1", KEY_PF1, 0 },
  { "PF2", KEY_PF2, 0 },
  { "PF3", KEY_PF3, 0 },
  { "PF4", KEY_PF4, 0 },
  { "NUMENTER", KEY_NUMENTER, 0 },
  { "MINUS", KEY_PadMinus, 0 },
  { "NUMSTOP", KEY_PadPeriod, 0 },
  { "COMMA", KEY_PadComma, 0 },
  { "STAR", KEY_PadStar, 0 },
  { "PLUS", KEY_PadPlus, 0 },
  { "SLASH", KEY_PadSlash, 0 },
  { "S-TAB", KEY_BTAB, SHIFT_SHIFT },
  { "FIND", KEY_Find, 0 },
  { "INS", KEY_InsertHere, 0 },
  { "REMOVE", KEY_Remove, 0 },
  { "DEL", KEY_DELETE, 0 },
  { "SELECT", KEY_Select, 0 },
  { "PGUP", KEY_PrevScreen, 0 },
  { "PGDN", KEY_NextScreen, 0 },
  { "TAB", KEY_C_i, 0 },
  { "RETURN", KEY_RETURN, 0 },
  { "CSI", CSI, 0 },
  { "BREAK", KEY_BREAK, 0 },
  { "DL", KEY_DL, 0 },
  { "IL", KEY_IL, 0 },
  { "DC", KEY_DC, 0 },
  { "INS", KEY_IC, 0 },
  { "EIC", KEY_EIC, 0 },
  { "CLEAR", KEY_CLEAR, 0 },
  { "EOS", KEY_EOS, 0 },
  { "EOL", KEY_EOL, 0 },
  { "SF", KEY_SF, 0 },
  { "SR", KEY_SR, 0 },
  { "PGDN", KEY_NPAGE, 0 },
  { "PGUP", KEY_PPAGE, 0 },
  { "S-TAB", KEY_STAB, SHIFT_SHIFT },
  { "C-TAB", KEY_CTAB, SHIFT_CTRL },
  { "CATAB", KEY_CATAB, 0 },
  { "ENTER", KEY_ENTER, 0 },
  { "S-RESET", KEY_SRESET, SHIFT_SHIFT },
  { "RESET", KEY_RESET, 0 },
  { "PRINT", KEY_PRINT, 0 },
  { "LL", KEY_LL, 0 },
  { "A1", KEY_A1, 0 },
  { "A3", KEY_A3, 0 },
  { "B2", KEY_B2, 0 },
  { "C1", KEY_C1, 0 },
  { "C3", KEY_C3, 0 },
  { "S-TAB", KEY_BTAB, SHIFT_SHIFT },
  { "BEG", KEY_BEG, 0 },
  { "CANCEL", KEY_CANCEL, 0 },
  { "CLOSE", KEY_CLOSE, 0 },
  { "COMMAND", KEY_COMMAND, 0 },
  { "COPY", KEY_COPY, 0 },
  { "CREATE", KEY_CREATE, 0 },
  { "END", KEY_END, 0 },
  { "EXIT", KEY_EXIT, 0 },
  { "FIND", KEY_FIND, 0 },
  { "HELP", KEY_HELP, 0 },
  { "MARK", KEY_MARK, 0 },
  { "MESSAGE", KEY_MESSAGE, 0 },
  { "MOVE", KEY_MOVE, 0 },
  { "NEXT", KEY_NEXT, 0 },
  { "OPEN", KEY_OPEN, 0 },
  { "OPTIONS", KEY_OPTIONS, 0 },
  { "PREVIOUS", KEY_PREVIOUS, 0 },
  { "REDO", KEY_REDO, 0 },
  { "REFERENCE", KEY_REFERENCE, 0 },
  { "REFRESH", KEY_REFRESH, 0 },
  { "REPLACE", KEY_REPLACE, 0 },
  { "RESTART", KEY_RESTART, 0 },
  { "RESUME", KEY_RESUME, 0 },
  { "SAVE", KEY_SAVE, 0 },
  { "S-BEG", KEY_SBEG, SHIFT_SHIFT },
  { "S-CANCEL", KEY_SCANCEL, SHIFT_SHIFT },
  { "S-COMMAND", KEY_SCOMMAND, SHIFT_SHIFT },
  { "S-COPY", KEY_SCOPY, SHIFT_SHIFT },
  { "S-CREATE", KEY_SCREATE, SHIFT_SHIFT },
  { "S-DC", KEY_SDC, SHIFT_SHIFT },
  { "S-DL", KEY_SDL, SHIFT_SHIFT },
  { "SELECT", KEY_SELECT, 0 },
  { "S-END", KEY_SEND, SHIFT_SHIFT },
  { "S-EOL", KEY_SEOL, SHIFT_SHIFT },
  { "S-EXIT", KEY_SEXIT, SHIFT_SHIFT },
  { "S-FIND", KEY_SFIND, SHIFT_SHIFT },
  { "S-HELP", KEY_SHELP, SHIFT_SHIFT },
  { "S-HOME", KEY_SHOME, SHIFT_SHIFT },
  { "S-INS", KEY_SIC, SHIFT_SHIFT },
  { "S-CURL", KEY_SLEFT, SHIFT_SHIFT },
  { "S-MESSAGE", KEY_SMESSAGE, SHIFT_SHIFT },
  { "S-MOVE", KEY_SMOVE, SHIFT_SHIFT },
  { "S-NEXT", KEY_SNEXT, SHIFT_SHIFT },
  { "S-OPTIONS", KEY_SOPTIONS, SHIFT_SHIFT },
  { "S-PREVIOUS", KEY_SPREVIOUS, SHIFT_SHIFT },
  { "S-PRINT", KEY_SPRINT, SHIFT_SHIFT },
  { "S-REDO", KEY_SREDO, SHIFT_SHIFT },
  { "S-REPLACE", KEY_SREPLACE, SHIFT_SHIFT },
  { "S-CURR", KEY_SRIGHT, SHIFT_SHIFT },
  { "S-RSUME", KEY_SRSUME, SHIFT_SHIFT },
  { "S-SAVE", KEY_SSAVE, SHIFT_SHIFT },
  { "S-SUSPEND", KEY_SSUSPEND, SHIFT_SHIFT },
  { "S-UNDO", KEY_SUNDO, SHIFT_SHIFT },
  { "SUSPEND", KEY_SUSPEND, 0 },
  { "UNDO", KEY_UNDO, 0 },
  { "C-CURL", CTL_LEFT, SHIFT_CTRL },
  { "C-CURR", CTL_RIGHT, SHIFT_CTRL },
  { "C-CURU", CTL_UP, SHIFT_CTRL },
  { "C-CURD", CTL_DOWN, SHIFT_CTRL },
  { "C-HOME", CTL_HOME, SHIFT_CTRL },
  { "C-END", CTL_END, SHIFT_CTRL },
  { "C-PGUP", CTL_PGUP, SHIFT_CTRL },
  { "C-PGDN", CTL_PGDN, SHIFT_CTRL },
  { "C-A", (int) KEY_C_a, SHIFT_CTRL },
  { "C-B", (int) KEY_C_b, SHIFT_CTRL },
  { "C-C", (int) KEY_C_c, SHIFT_CTRL },
  { "C-D", (int) KEY_C_d, SHIFT_CTRL },
  { "C-E", (int) KEY_C_e, SHIFT_CTRL },
  { "C-F", (int) KEY_C_f, SHIFT_CTRL },
  { "C-G", (int) KEY_C_g, SHIFT_CTRL },
  { "C-H", (int) KEY_C_h, SHIFT_CTRL },
  { "C-I", (int) KEY_C_i, SHIFT_CTRL },
  { "C-J", (int) KEY_C_j, SHIFT_CTRL },
  { "C-K", (int) KEY_C_k, SHIFT_CTRL },
  { "C-L", (int) KEY_C_l, SHIFT_CTRL },
  { "C-M", (int) KEY_C_m, SHIFT_CTRL },
  { "C-N", (int) KEY_C_n, SHIFT_CTRL },
  { "C-O", (int) KEY_C_o, SHIFT_CTRL },
  { "C-P", (int) KEY_C_p, SHIFT_CTRL },
  { "C-Q", (int) KEY_C_q, SHIFT_CTRL },
  { "C-R", (int) KEY_C_r, SHIFT_CTRL },
  { "C-S", (int) KEY_C_s, SHIFT_CTRL },
  { "C-T", (int) KEY_C_t, SHIFT_CTRL },
  { "C-U", (int) KEY_C_u, SHIFT_CTRL },
  { "C-V", (int) KEY_C_v, SHIFT_CTRL },
  { "C-W", (int) KEY_C_w, SHIFT_CTRL },
  { "C-X", (int) KEY_C_x, SHIFT_CTRL },
  { "C-Y", (int) KEY_C_y, SHIFT_CTRL },
  { "C-Z", (int) KEY_C_z, SHIFT_CTRL },
  { NULL, 0, 0 },
};
