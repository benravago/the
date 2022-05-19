/*
 * This software is Copyright (c) 1989, 1990, 1991 by Patrick J. Wolfe.
 *
 * Permission is hereby granted to copy, distribute or otherwise
 * use any part of this package as long as you do not try to make
 * money from it or pretend that you wrote it.  This copyright
 * notice must be maintained in any copy made.
 *
 * Use of this software constitutes acceptance for use in an AS IS
 * condition. There are NO warranties with regard to this software.
 * In no event shall the author be liable for any damages whatsoever
 * arising out of or in connection with the use or performance of this
 * software.  Any use of this software is at the user's own risk.
 *
 * If you make modifications to this software that you feel
 * increases it usefulness for the rest of the community, please
 * email the changes, enhancements, bug fixes as well as any and
 * all ideas to me. This software is going to be maintained and
 * enhanced as deemed necessary by the community.
 *
 *              Patrick J. Wolfe
 *              uunet!uiucuxc!kailand!pwolfe
 *              pwolfe@kailand.kai.com
 *
 * Additions of DOS keys and extended function key descriptions
 * made by Mark Hessling.
 *
 */

/* System V curses key names and codes returned by getch */

/*
$Id: getch.h,v 1.6 2019/09/08 04:08:22 mark Exp $
*/

// TODO: validate these mappings
#define KEY_A2          0x198             /* Upper centre of keypad */
#define KEY_B1          0x19a             /* Middle left of keypad */
#define KEY_B3          0x19c             /* Middle right of keypad */
#define KEY_C2          0x19e             /* Lower centre of keypad */

/* special definition for UNIX systems */

#define KEY_RETURN     0xd
#define KEY_NUMENTER   KEY_ENTER
#define KEY_BKSP      0x8
#define KEY_DELETE  127

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

#ifdef PAD0
# define KEY_Pad0      PAD0
#else
# define KEY_Pad0      0x360
#endif

/* Mouse button definitions */

#define KEY_PB1        0x400
#define KEY_PB2        0x401
#define KEY_PB3        0x402
#define KEY_S_PB1      0x403
#define KEY_S_PB2      0x404
#define KEY_S_PB3      0x405
#define KEY_C_PB1      0x406
#define KEY_C_PB2      0x407
#define KEY_C_PB3      0x408
#define KEY_A_PB1      0x409
#define KEY_A_PB2      0x40a
#define KEY_A_PB3      0x40b

#define KEY_RB1        0x410
#define KEY_RB2        0x411
#define KEY_RB3        0x412
#define KEY_S_RB1      0x413
#define KEY_S_RB2      0x414
#define KEY_S_RB3      0x415
#define KEY_C_RB1      0x416
#define KEY_C_RB2      0x417
#define KEY_C_RB3      0x418
#define KEY_A_RB1      0x419
#define KEY_A_RB2      0x41a
#define KEY_A_RB3      0x41b

#define KEY_DB1        0x420
#define KEY_DB2        0x421
#define KEY_DB3        0x422
#define KEY_S_DB1      0x423
#define KEY_S_DB2      0x424
#define KEY_S_DB3      0x425
#define KEY_C_DB1      0x426
#define KEY_C_DB2      0x427
#define KEY_C_DB3      0x428
#define KEY_A_DB1      0x429
#define KEY_A_DB2      0x42a
#define KEY_A_DB3      0x42b

