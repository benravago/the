// SPDX-FileCopyrightText: 2001 Mark Hessling <mark@rexx.org>
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

/*
 * Additions of DOS keys and extended function key descriptions made by Mark Hessling.
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
  char_t *mnemonic;
  int key_value;
  int shift;
};
typedef struct key_values KEYS;

KEYS key_table[] = {
  { (char_t *) "A", (int) 'a', 0 },
  { (char_t *) "B", (int) 'b', 0 },
  { (char_t *) "C", (int) 'c', 0 },
  { (char_t *) "D", (int) 'd', 0 },
  { (char_t *) "E", (int) 'e', 0 },
  { (char_t *) "F", (int) 'f', 0 },
  { (char_t *) "G", (int) 'g', 0 },
  { (char_t *) "H", (int) 'h', 0 },
  { (char_t *) "I", (int) 'i', 0 },
  { (char_t *) "J", (int) 'j', 0 },
  { (char_t *) "K", (int) 'k', 0 },
  { (char_t *) "L", (int) 'l', 0 },
  { (char_t *) "M", (int) 'm', 0 },
  { (char_t *) "N", (int) 'n', 0 },
  { (char_t *) "O", (int) 'o', 0 },
  { (char_t *) "P", (int) 'p', 0 },
  { (char_t *) "Q", (int) 'q', 0 },
  { (char_t *) "R", (int) 'r', 0 },
  { (char_t *) "S", (int) 's', 0 },
  { (char_t *) "T", (int) 't', 0 },
  { (char_t *) "U", (int) 'u', 0 },
  { (char_t *) "V", (int) 'v', 0 },
  { (char_t *) "W", (int) 'w', 0 },
  { (char_t *) "X", (int) 'x', 0 },
  { (char_t *) "Y", (int) 'y', 0 },
  { (char_t *) "Z", (int) 'z', 0 },
  { (char_t *) "S-A", (int) 'A', SHIFT_SHIFT },
  { (char_t *) "S-B", (int) 'B', SHIFT_SHIFT },
  { (char_t *) "S-C", (int) 'C', SHIFT_SHIFT },
  { (char_t *) "S-D", (int) 'D', SHIFT_SHIFT },
  { (char_t *) "S-E", (int) 'E', SHIFT_SHIFT },
  { (char_t *) "S-F", (int) 'F', SHIFT_SHIFT },
  { (char_t *) "S-G", (int) 'G', SHIFT_SHIFT },
  { (char_t *) "S-H", (int) 'H', SHIFT_SHIFT },
  { (char_t *) "S-I", (int) 'I', SHIFT_SHIFT },
  { (char_t *) "S-J", (int) 'J', SHIFT_SHIFT },
  { (char_t *) "S-K", (int) 'K', SHIFT_SHIFT },
  { (char_t *) "S-L", (int) 'L', SHIFT_SHIFT },
  { (char_t *) "S-M", (int) 'M', SHIFT_SHIFT },
  { (char_t *) "S-N", (int) 'N', SHIFT_SHIFT },
  { (char_t *) "S-O", (int) 'O', SHIFT_SHIFT },
  { (char_t *) "S-P", (int) 'P', SHIFT_SHIFT },
  { (char_t *) "S-Q", (int) 'Q', SHIFT_SHIFT },
  { (char_t *) "S-R", (int) 'R', SHIFT_SHIFT },
  { (char_t *) "S-S", (int) 'S', SHIFT_SHIFT },
  { (char_t *) "S-T", (int) 'T', SHIFT_SHIFT },
  { (char_t *) "S-U", (int) 'U', SHIFT_SHIFT },
  { (char_t *) "S-V", (int) 'V', SHIFT_SHIFT },
  { (char_t *) "S-W", (int) 'W', SHIFT_SHIFT },
  { (char_t *) "S-X", (int) 'X', SHIFT_SHIFT },
  { (char_t *) "S-Y", (int) 'Y', SHIFT_SHIFT },
  { (char_t *) "S-Z", (int) 'Z', SHIFT_SHIFT },
  { (char_t *) "0", (int) '0', 0 },
  { (char_t *) "1", (int) '1', 0 },
  { (char_t *) "2", (int) '2', 0 },
  { (char_t *) "3", (int) '3', 0 },
  { (char_t *) "4", (int) '4', 0 },
  { (char_t *) "5", (int) '5', 0 },
  { (char_t *) "6", (int) '6', 0 },
  { (char_t *) "7", (int) '7', 0 },
  { (char_t *) "8", (int) '8', 0 },
  { (char_t *) "9", (int) '9', 0 },
  { (char_t *) "`", (int) '`', 0 },
  { (char_t *) "-", (int) '-', 0 },
  { (char_t *) "=", (int) '=', 0 },
  { (char_t *) "[", (int) '[', 0 },
  { (char_t *) "]", (int) ']', 0 },
  { (char_t *) "\\", (int) '\\', 0 },
  { (char_t *) ";", (int) ';', 0 },
  { (char_t *) "'", (int) '\'', 0 },
  { (char_t *) ",", (int) ',', 0 },
  { (char_t *) ".", (int) '.', 0 },
  { (char_t *) "/", (int) '/', 0 },
  { (char_t *) ")", (int) ')', 0 },
  { (char_t *) "!", (int) '!', 0 },
  { (char_t *) "@", (int) '@', 0 },
  { (char_t *) "#", (int) '#', 0 },
  { (char_t *) "$", (int) '$', 0 },
  { (char_t *) "%", (int) '%', 0 },
  { (char_t *) "^", (int) '^', 0 },
  { (char_t *) "&", (int) '&', 0 },
  { (char_t *) "*", (int) '*', 0 },
  { (char_t *) "(", (int) '(', 0 },
  { (char_t *) "~", (int) '~', 0 },
  { (char_t *) "_", (int) '_', 0 },
  { (char_t *) "+", (int) '+', 0 },
  { (char_t *) "{", (int) '{', 0 },
  { (char_t *) "}", (int) '}', 0 },
  { (char_t *) "|", (int) '|', 0 },
  { (char_t *) ":", (int) ':', 0 },
  { (char_t *) "\"", (int) '\"', 0 },
  { (char_t *) "<", (int) '<', 0 },
  { (char_t *) ">", (int) '>', 0 },
  { (char_t *) "?", (int) '?', 0 },
  { (char_t *) "SPACE", (int) ' ', 0 },
  { (char_t *) "ESC", KEY_ESC, 0 },
  { (char_t *) "F0", KEY_F0, 0 },
  { (char_t *) "F1", KEY_F1, 0 },
  { (char_t *) "F2", KEY_F2, 0 },
  { (char_t *) "F3", KEY_F3, 0 },
  { (char_t *) "F4", KEY_F4, 0 },
  { (char_t *) "F5", KEY_F5, 0 },
  { (char_t *) "F6", KEY_F6, 0 },
  { (char_t *) "F7", KEY_F7, 0 },
  { (char_t *) "F8", KEY_F8, 0 },
  { (char_t *) "F9", KEY_F9, 0 },
  { (char_t *) "F10", KEY_F10, 0 },
  { (char_t *) "F11", KEY_F11, 0 },
  { (char_t *) "F12", KEY_F12, 0 },
  { (char_t *) "S-F1", KEY_S_F1, SHIFT_SHIFT },
  { (char_t *) "S-F2", KEY_S_F2, SHIFT_SHIFT },
  { (char_t *) "S-F3", KEY_S_F3, SHIFT_SHIFT },
  { (char_t *) "S-F4", KEY_S_F4, SHIFT_SHIFT },
  { (char_t *) "S-F5", KEY_S_F5, SHIFT_SHIFT },
  { (char_t *) "S-F6", KEY_S_F6, SHIFT_SHIFT },
  { (char_t *) "S-F7", KEY_S_F7, SHIFT_SHIFT },
  { (char_t *) "S-F8", KEY_S_F8, SHIFT_SHIFT },
  { (char_t *) "S-F9", KEY_S_F9, SHIFT_SHIFT },
  { (char_t *) "S-F10", KEY_S_F10, SHIFT_SHIFT },
  { (char_t *) "S-F11", KEY_S_F11, SHIFT_SHIFT },
  { (char_t *) "S-F12", KEY_S_F12, SHIFT_SHIFT },
  { (char_t *) "C-F1", KEY_C_F1, SHIFT_CTRL },
  { (char_t *) "C-F2", KEY_C_F2, SHIFT_CTRL },
  { (char_t *) "C-F3", KEY_C_F3, SHIFT_CTRL },
  { (char_t *) "C-F4", KEY_C_F4, SHIFT_CTRL },
  { (char_t *) "C-F5", KEY_C_F5, SHIFT_CTRL },
  { (char_t *) "C-F6", KEY_C_F6, SHIFT_CTRL },
  { (char_t *) "C-F7", KEY_C_F7, SHIFT_CTRL },
  { (char_t *) "C-F8", KEY_C_F8, SHIFT_CTRL },
  { (char_t *) "C-F9", KEY_C_F9, SHIFT_CTRL },
  { (char_t *) "C-F10", KEY_C_F10, SHIFT_CTRL },
  { (char_t *) "C-F11", KEY_C_F11, SHIFT_CTRL },
  { (char_t *) "C-F12", KEY_C_F12, SHIFT_CTRL },
  { (char_t *) "A-F1", KEY_A_F1, SHIFT_ALT },
  { (char_t *) "A-F2", KEY_A_F2, SHIFT_ALT },
  { (char_t *) "A-F3", KEY_A_F3, SHIFT_ALT },
  { (char_t *) "A-F4", KEY_A_F4, SHIFT_ALT },
  { (char_t *) "A-F5", KEY_A_F5, SHIFT_ALT },
  { (char_t *) "A-F6", KEY_A_F6, SHIFT_ALT },
  { (char_t *) "A-F7", KEY_A_F7, SHIFT_ALT },
  { (char_t *) "A-F8", KEY_A_F8, SHIFT_ALT },
  { (char_t *) "A-F9", KEY_A_F9, SHIFT_ALT },
  { (char_t *) "A-F10", KEY_A_F10, SHIFT_ALT },
  { (char_t *) "A-F11", KEY_A_F11, SHIFT_ALT },
  { (char_t *) "A-F12", KEY_A_F12, SHIFT_ALT },
  { (char_t *) "F13", KEY_F13, 0 },
  { (char_t *) "F14", KEY_F14, 0 },
  { (char_t *) "F15", KEY_F15, 0 },
  { (char_t *) "F16", KEY_F16, 0 },
  { (char_t *) "F17", KEY_F17, 0 },
  { (char_t *) "F18", KEY_F18, 0 },
  { (char_t *) "F19", KEY_F19, 0 },
  { (char_t *) "F20", KEY_F20, 0 },
  { (char_t *) "S-F13", KEY_S_F13, SHIFT_SHIFT },
  { (char_t *) "S-F14", KEY_S_F14, SHIFT_SHIFT },
  { (char_t *) "S-F15", KEY_S_F15, SHIFT_SHIFT },
  { (char_t *) "S-F16", KEY_S_F16, SHIFT_SHIFT },
  { (char_t *) "S-F17", KEY_S_F17, SHIFT_SHIFT },
  { (char_t *) "S-F18", KEY_S_F18, SHIFT_SHIFT },
  { (char_t *) "S-F19", KEY_S_F19, SHIFT_SHIFT },
  { (char_t *) "NUM0", KEY_Pad0, 0 },
  { (char_t *) "NUM1", KEY_C1, 0 },
  { (char_t *) "NUM2", KEY_C2, 0 },
  { (char_t *) "NUM3", KEY_C3, 0 },
  { (char_t *) "NUM4", KEY_B1, 0 },
  { (char_t *) "CENTER", KEY_B2, 0 },
  { (char_t *) "NUM6", KEY_B3, 0 },
  { (char_t *) "NUM7", KEY_A1, 0 },
  { (char_t *) "NUM8", KEY_A2, 0 },
  { (char_t *) "NUM9", KEY_A3, 0 },

  { (char_t *) "BKSP", KEY_BKSP, 0 },
  { (char_t *) "ENTER", KEY_RETURN, 0 },
  { (char_t *) "ENTER", KEY_ENTER, 0 },

  { (char_t *) "CURD", KEY_DOWN, 0 },
  { (char_t *) "CURU", KEY_UP, 0 },
  { (char_t *) "CURL", KEY_LEFT, 0 },
  { (char_t *) "CURR", KEY_RIGHT, 0 },
  { (char_t *) "BACKSPACE", KEY_BACKSPACE, 0 },
  { (char_t *) "HOME", KEY_HOME, 0 },
  { (char_t *) "PF1", KEY_PF1, 0 },
  { (char_t *) "PF2", KEY_PF2, 0 },
  { (char_t *) "PF3", KEY_PF3, 0 },
  { (char_t *) "PF4", KEY_PF4, 0 },
  { (char_t *) "NUMENTER", KEY_NUMENTER, 0 },
  { (char_t *) "MINUS", KEY_PadMinus, 0 },
  { (char_t *) "NUMSTOP", KEY_PadPeriod, 0 },
  { (char_t *) "COMMA", KEY_PadComma, 0 },
  { (char_t *) "STAR", KEY_PadStar, 0 },
  { (char_t *) "PLUS", KEY_PadPlus, 0 },
  { (char_t *) "SLASH", KEY_PadSlash, 0 },
  { (char_t *) "S-TAB", KEY_BTAB, SHIFT_SHIFT },
  { (char_t *) "FIND", KEY_Find, 0 },
  { (char_t *) "INS", KEY_InsertHere, 0 },
  { (char_t *) "REMOVE", KEY_Remove, 0 },
  { (char_t *) "DEL", KEY_DELETE, 0 },
  { (char_t *) "SELECT", KEY_Select, 0 },
  { (char_t *) "PGUP", KEY_PrevScreen, 0 },
  { (char_t *) "PGDN", KEY_NextScreen, 0 },
  { (char_t *) "TAB", KEY_C_i, 0 },
  { (char_t *) "RETURN", KEY_RETURN, 0 },
  { (char_t *) "CSI", CSI, 0 },
  { (char_t *) "BREAK", KEY_BREAK, 0 },
  { (char_t *) "DL", KEY_DL, 0 },
  { (char_t *) "IL", KEY_IL, 0 },
  { (char_t *) "DC", KEY_DC, 0 },
  { (char_t *) "INS", KEY_IC, 0 },
  { (char_t *) "EIC", KEY_EIC, 0 },
  { (char_t *) "CLEAR", KEY_CLEAR, 0 },
  { (char_t *) "EOS", KEY_EOS, 0 },
  { (char_t *) "EOL", KEY_EOL, 0 },
  { (char_t *) "SF", KEY_SF, 0 },
  { (char_t *) "SR", KEY_SR, 0 },
  { (char_t *) "PGDN", KEY_NPAGE, 0 },
  { (char_t *) "PGUP", KEY_PPAGE, 0 },
  { (char_t *) "S-TAB", KEY_STAB, SHIFT_SHIFT },
  { (char_t *) "C-TAB", KEY_CTAB, SHIFT_CTRL },
  { (char_t *) "CATAB", KEY_CATAB, 0 },
  { (char_t *) "ENTER", KEY_ENTER, 0 },
  { (char_t *) "S-RESET", KEY_SRESET, SHIFT_SHIFT },
  { (char_t *) "RESET", KEY_RESET, 0 },
  { (char_t *) "PRINT", KEY_PRINT, 0 },
  { (char_t *) "LL", KEY_LL, 0 },
  { (char_t *) "A1", KEY_A1, 0 },
  { (char_t *) "A3", KEY_A3, 0 },
  { (char_t *) "B2", KEY_B2, 0 },
  { (char_t *) "C1", KEY_C1, 0 },
  { (char_t *) "C3", KEY_C3, 0 },
  { (char_t *) "S-TAB", KEY_BTAB, SHIFT_SHIFT },
  { (char_t *) "BEG", KEY_BEG, 0 },
  { (char_t *) "CANCEL", KEY_CANCEL, 0 },
  { (char_t *) "CLOSE", KEY_CLOSE, 0 },
  { (char_t *) "COMMAND", KEY_COMMAND, 0 },
  { (char_t *) "COPY", KEY_COPY, 0 },
  { (char_t *) "CREATE", KEY_CREATE, 0 },
  { (char_t *) "END", KEY_END, 0 },
  { (char_t *) "EXIT", KEY_EXIT, 0 },
  { (char_t *) "FIND", KEY_FIND, 0 },
  { (char_t *) "HELP", KEY_HELP, 0 },
  { (char_t *) "MARK", KEY_MARK, 0 },
  { (char_t *) "MESSAGE", KEY_MESSAGE, 0 },
  { (char_t *) "MOVE", KEY_MOVE, 0 },
  { (char_t *) "NEXT", KEY_NEXT, 0 },
  { (char_t *) "OPEN", KEY_OPEN, 0 },
  { (char_t *) "OPTIONS", KEY_OPTIONS, 0 },
  { (char_t *) "PREVIOUS", KEY_PREVIOUS, 0 },
  { (char_t *) "REDO", KEY_REDO, 0 },
  { (char_t *) "REFERENCE", KEY_REFERENCE, 0 },
  { (char_t *) "REFRESH", KEY_REFRESH, 0 },
  { (char_t *) "REPLACE", KEY_REPLACE, 0 },
  { (char_t *) "RESTART", KEY_RESTART, 0 },
  { (char_t *) "RESUME", KEY_RESUME, 0 },
  { (char_t *) "SAVE", KEY_SAVE, 0 },
  { (char_t *) "S-BEG", KEY_SBEG, SHIFT_SHIFT },
  { (char_t *) "S-CANCEL", KEY_SCANCEL, SHIFT_SHIFT },
  { (char_t *) "S-COMMAND", KEY_SCOMMAND, SHIFT_SHIFT },
  { (char_t *) "S-COPY", KEY_SCOPY, SHIFT_SHIFT },
  { (char_t *) "S-CREATE", KEY_SCREATE, SHIFT_SHIFT },
  { (char_t *) "S-DC", KEY_SDC, SHIFT_SHIFT },
  { (char_t *) "S-DL", KEY_SDL, SHIFT_SHIFT },
  { (char_t *) "SELECT", KEY_SELECT, 0 },
  { (char_t *) "S-END", KEY_SEND, SHIFT_SHIFT },
  { (char_t *) "S-EOL", KEY_SEOL, SHIFT_SHIFT },
  { (char_t *) "S-EXIT", KEY_SEXIT, SHIFT_SHIFT },
  { (char_t *) "S-FIND", KEY_SFIND, SHIFT_SHIFT },
  { (char_t *) "S-HELP", KEY_SHELP, SHIFT_SHIFT },
  { (char_t *) "S-HOME", KEY_SHOME, SHIFT_SHIFT },
  { (char_t *) "S-INS", KEY_SIC, SHIFT_SHIFT },
  { (char_t *) "S-CURL", KEY_SLEFT, SHIFT_SHIFT },
  { (char_t *) "S-MESSAGE", KEY_SMESSAGE, SHIFT_SHIFT },
  { (char_t *) "S-MOVE", KEY_SMOVE, SHIFT_SHIFT },
  { (char_t *) "S-NEXT", KEY_SNEXT, SHIFT_SHIFT },
  { (char_t *) "S-OPTIONS", KEY_SOPTIONS, SHIFT_SHIFT },
  { (char_t *) "S-PREVIOUS", KEY_SPREVIOUS, SHIFT_SHIFT },
  { (char_t *) "S-PRINT", KEY_SPRINT, SHIFT_SHIFT },
  { (char_t *) "S-REDO", KEY_SREDO, SHIFT_SHIFT },
  { (char_t *) "S-REPLACE", KEY_SREPLACE, SHIFT_SHIFT },
  { (char_t *) "S-CURR", KEY_SRIGHT, SHIFT_SHIFT },
  { (char_t *) "S-RSUME", KEY_SRSUME, SHIFT_SHIFT },
  { (char_t *) "S-SAVE", KEY_SSAVE, SHIFT_SHIFT },
  { (char_t *) "S-SUSPEND", KEY_SSUSPEND, SHIFT_SHIFT },
  { (char_t *) "S-UNDO", KEY_SUNDO, SHIFT_SHIFT },
  { (char_t *) "SUSPEND", KEY_SUSPEND, 0 },
  { (char_t *) "UNDO", KEY_UNDO, 0 },
  { (char_t *) "C-CURL", CTL_LEFT, SHIFT_CTRL },
  { (char_t *) "C-CURR", CTL_RIGHT, SHIFT_CTRL },
  { (char_t *) "C-CURU", CTL_UP, SHIFT_CTRL },
  { (char_t *) "C-CURD", CTL_DOWN, SHIFT_CTRL },
  { (char_t *) "C-HOME", CTL_HOME, SHIFT_CTRL },
  { (char_t *) "C-END", CTL_END, SHIFT_CTRL },
  { (char_t *) "C-PGUP", CTL_PGUP, SHIFT_CTRL },
  { (char_t *) "C-PGDN", CTL_PGDN, SHIFT_CTRL },
  { (char_t *) "C-A", (int) KEY_C_a, SHIFT_CTRL },
  { (char_t *) "C-B", (int) KEY_C_b, SHIFT_CTRL },
  { (char_t *) "C-C", (int) KEY_C_c, SHIFT_CTRL },
  { (char_t *) "C-D", (int) KEY_C_d, SHIFT_CTRL },
  { (char_t *) "C-E", (int) KEY_C_e, SHIFT_CTRL },
  { (char_t *) "C-F", (int) KEY_C_f, SHIFT_CTRL },
  { (char_t *) "C-G", (int) KEY_C_g, SHIFT_CTRL },
  { (char_t *) "C-H", (int) KEY_C_h, SHIFT_CTRL },
  { (char_t *) "C-I", (int) KEY_C_i, SHIFT_CTRL },
  { (char_t *) "C-J", (int) KEY_C_j, SHIFT_CTRL },
  { (char_t *) "C-K", (int) KEY_C_k, SHIFT_CTRL },
  { (char_t *) "C-L", (int) KEY_C_l, SHIFT_CTRL },
  { (char_t *) "C-M", (int) KEY_C_m, SHIFT_CTRL },
  { (char_t *) "C-N", (int) KEY_C_n, SHIFT_CTRL },
  { (char_t *) "C-O", (int) KEY_C_o, SHIFT_CTRL },
  { (char_t *) "C-P", (int) KEY_C_p, SHIFT_CTRL },
  { (char_t *) "C-Q", (int) KEY_C_q, SHIFT_CTRL },
  { (char_t *) "C-R", (int) KEY_C_r, SHIFT_CTRL },
  { (char_t *) "C-S", (int) KEY_C_s, SHIFT_CTRL },
  { (char_t *) "C-T", (int) KEY_C_t, SHIFT_CTRL },
  { (char_t *) "C-U", (int) KEY_C_u, SHIFT_CTRL },
  { (char_t *) "C-V", (int) KEY_C_v, SHIFT_CTRL },
  { (char_t *) "C-W", (int) KEY_C_w, SHIFT_CTRL },
  { (char_t *) "C-X", (int) KEY_C_x, SHIFT_CTRL },
  { (char_t *) "C-Y", (int) KEY_C_y, SHIFT_CTRL },
  { (char_t *) "C-Z", (int) KEY_C_z, SHIFT_CTRL },
  { (char_t *) NULL, 0, 0 },
};
