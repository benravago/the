// SPDX-FileCopyrightText: 1989, 1990, 1991 by Patrick J. Wolfe.
// SPDX-License-Identifier: GPL-2.0
// SPDX-FileContributor: 2022 Ben Ravago

#include <errno.h>
#include <ncurses.h>

/*
 * Do special character handling if using ncurses, xcurses
 */
#include <getch.h>

#define NORMAL 100
#define ESCAPE 200
#define FKEY   300
#define BRACK  400

int my_getch(WINDOW *winptr) {
  int c = 0, tmp_c = (-1);
  short state = NORMAL;
  short fkeycount = 0;

  for (;;) {
    c = wgetch(winptr);
    if (c == ERR) {
      if (errno == EINTR) {
        c = KEY_RESIZE;
      } else {
        if (errno == 0) {
          c = wgetch(winptr);
        }
      }
    }
    switch (state) {

      case BRACK:
        switch (c) {
          /* Linux f1 thru f5 are <esc>[[A <esc>[[B ... <esc>[[E */
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
            return KEY_F1 + (c - 'A');
          default:
            state = NORMAL;
            break;
        }
        break;

      case FKEY:
        switch (c) {
          /* numeric function keys */
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            fkeycount = (fkeycount * 10) + (c - '0');
            break;

          case '~':
            switch (fkeycount) {
              /* Find, Insert Here, Remove, Select, Prev Screen, Next Screen */
              case 1:
                return KEY_Find;
              case 2:
                return KEY_InsertHere;
              case 3:
                return KEY_Remove;
              case 4:
                return KEY_Select;
              case 5:
                return KEY_PrevScreen;
              case 6:
                return KEY_NextScreen;

              /* vt220 function keys F1 to F10 */
              case 11:
              case 12:
              case 13:
              case 14:
              case 15:
                return KEY_F(1) + (fkeycount - 11);
              case 17:
              case 18:
              case 19:
              case 20:
              case 21:
                return KEY_F(6) + (fkeycount - 17);

              /* vt220 function keys F11 to F20 */
              case 23:
              case 24:
                return KEY_F(11) + (fkeycount - 23);
              case 25:
              case 26:
                return KEY_F(49) + (fkeycount - 25);
              case 28:
              case 29:
                return KEY_F(51) + (fkeycount - 28);
              case 31:
              case 32:
              case 33:
              case 34:
                return KEY_F(53) + (fkeycount - 31);

              /* vt220 function keys - non-standard */
              case 37:
              case 38:
              case 39:
              case 40:
              case 41:
              case 42:
              case 43:
              case 44:
              case 45:
              case 46:
              case 47:
              case 48:
              case 49:
              case 50:
              case 51:
              case 52:
              case 53:
              case 54:
              case 55:
              case 56:
              case 57:
              case 58:
              case 59:
              case 60:
                return KEY_F(13) + (fkeycount - 37);

              default:
                state = NORMAL;
            }
            break;

          case 0:
            return KEY_HOME;
          case 'A':
            return KEY_UP;
          case 'B':
            return KEY_DOWN;
          case 'C':
            return KEY_RIGHT;
          case 'D':
            return KEY_LEFT;
          case 'M':
            return KEY_NUMENTER;
          case 'Z':
            return KEY_BTAB;

          /* Xenix default key mappings */
          case 'H':
            return KEY_HOME;
          case 'F':
            return KEY_END;
          case 'L':
            return KEY_InsertHere;
          case 'G':
            return KEY_NextScreen;
          case 'I':
            return KEY_PrevScreen;

          case 'N':
            return KEY_F2;
          case 'O':
            return KEY_F3;
          case 'T':
            return KEY_F8;
          case 'U':
            return KEY_F9;
          case 'V':
            return KEY_F10;
          case 'W':
            return KEY_F11;
          case 'X':
            return KEY_F12;

          /* VT[12]00 PF keys */
          case 'P':
          case 'Q':
          case 'R':
          case 'S':
            return KEY_PF1 + (c - 'P');

          case 'j':
            return KEY_PadStar;
          case 'k':
            return KEY_PadPlus;
          case 'l':
            return KEY_PadComma;
          case 'm':
            return KEY_PadMinus;
          case 'n':
            return KEY_PadPeriod;
          case 'o':
            return KEY_PadSlash;
          case 'p':
            return KEY_Pad0;
          case 'r':
            return KEY_C2;
          case 's':
            return KEY_C3;
          case 't':
            return KEY_B1;
          case 'u':
            return KEY_B2;
          case 'v':
            return KEY_B3;
          case 'w':
            return KEY_A1;
          case 'x':
            return KEY_A2;
          case 'y':
            return KEY_A3;

          /* Sun Keyboard Function Keys */
          case 'z':
            switch (fkeycount) {
              /* SUN F1-F12 */
              case 224:
              case 225:
              case 226:
              case 227:
              case 228:
              case 229:
              case 230:
              case 231:
              case 232:
              case 233:
              case 234:
              case 235:
                return KEY_F(1) + (fkeycount - 224);
              /* SUN S-F1-S-F12 */
              case 324:
              case 325:
              case 326:
              case 327:
              case 328:
              case 329:
              case 330:
              case 331:
              case 332:
              case 333:
              case 334:
              case 335:
                return KEY_F(13) + (fkeycount - 324);
              /* SUN C-F1-C-F12 */
              case 424:
              case 425:
              case 426:
              case 427:
              case 428:
              case 429:
              case 430:
              case 431:
              case 432:
              case 433:
              case 434:
              case 435:
                return KEY_F(25) + (fkeycount - 424);
              case 192:
                return KEY_F(49);       /* L1 */
              case 193:
                return KEY_F(50);       /* L2 */
              case 195:
                return KEY_UNDO;
              case 292:
                return KEY_F(57);       /* S-L1 */
              case 293:
                return KEY_F(58);       /* S-L2 */
              case 214:
                return KEY_HOME;
              case 414:
                return CTL_HOME;
              case 215:
                return KEY_UP;
              case 415:
                return CTL_UP;
              case 216:
                return KEY_PrevScreen;
              case 416:
                return CTL_PGUP;
              case 217:
                return KEY_LEFT;
              case 417:
                return CTL_LEFT;
              case 219:
                return KEY_RIGHT;
              case 419:
                return CTL_RIGHT;
              case 220:
                return KEY_END;
              case 420:
                return CTL_END;
              case 221:
                return KEY_DOWN;
              case 421:
                return CTL_DOWN;
              case 222:
                return KEY_NextScreen;
              case 422:
                return CTL_PGDN;
              case 1:
                return KEY_BTAB;
              case 2:
                return KEY_InsertHere;
              case 3:
                return KEY_HOME;
              case 4:
                return KEY_END;
              case 5:
                return KEY_PrevScreen;
              case 6:
                return KEY_NextScreen;
              case 423:
                return KEY_PadComma;
              default:
                state = NORMAL;
            }
            break;

          /* IBM AIX ???????? */
          case 'q':
            switch (fkeycount) {
              case 0:          /* VT100/200 keypad */
                return KEY_C1;
              /* AIX F1-F12 */
              case 1:
              case 2:
              case 3:
              case 4:
              case 5:
              case 6:
              case 7:
              case 8:
              case 9:
              case 10:
              case 11:
              case 12:
                return KEY_F(1) + (fkeycount - 1);
              /* AIX SF1-F12 */
              case 13:
              case 14:
              case 15:
              case 16:
              case 17:
              case 18:
              case 19:
              case 20:
              case 21:
              case 22:
              case 23:
              case 24:
                return KEY_F(13) + (fkeycount - 13);
              /* AIX CF1-F12 */
              case 25:
              case 26:
              case 27:
              case 28:
              case 29:
              case 30:
              case 31:
              case 32:
              case 33:
              case 34:
              case 35:
              case 36:
                return KEY_F(25) + (fkeycount - 25);
              case 150:
                return KEY_PrevScreen;
              case 146:
                return KEY_END;
              case 154:
                return KEY_NextScreen;
              case 139:
                return KEY_InsertHere;
              default:
                state = NORMAL;
            }
            break;
          case '[':
            state = BRACK;
            break;

          default:
            state = NORMAL;
        }
        break;

      case ESCAPE:
        switch (c) {
          case 'O':            /* vt100 numeric keypad application codes */
          case '?':            /* vt52  numeric keypad application codes */
          case '[':
            state = FKEY;
            fkeycount = 0;
            break;
          /* VT52 PF keys */
          case 'P':
          case 'Q':
          case 'R':
          case 'S':
            return KEY_PF1 + (c - 'P');

          default:
            state = NORMAL;
        }
        break;

      default:
        switch (c) {
          case KEY_ESC:
            /* this code allows the user to use the ESC key */
            nodelay(winptr, TRUE);
            tmp_c = wgetch(winptr);
            nodelay(winptr, FALSE);
            if (tmp_c == ERR) {
              return (c);
            }
            ungetch(tmp_c);
            state = ESCAPE;
            break;

          case CSI:
            state = FKEY;
            fkeycount = 0;
            break;

          default:
            return (c);
        }
    }
  }
}

