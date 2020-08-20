/*
 * A simple user interface for this project
 *
 * Copyright 2020 Creep_er
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#ifdef __cplusplus 
extern "C" { 
#endif

// ┌─────┬───────┬───────┬────────┬──────────┬────────┐
// |     | 0     | 1     | 2      | 3        | 4      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 0   | f1    | f3    | f4     | f5       | up     |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 1   | symb  | f2    | num    | f6       | right  |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 2   | home  | plot  | views  | x,t,Φ,n  | left   |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 3   | vars  | apps  | a b/c  | ←        | down   |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 4   | sin   | math  | tan    | ln       | log    |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 5   | x^2   | cos   | (      | )        | ÷      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 6   | ,     | x^y   | 8      | 9        | x      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 7   | alpha | 7     | 5      | 6        | -      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 8   | shift | 4     | 2      | 3        | +      |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 9   | 0     | 1     | .      | (-)      | enter  |
// ├─────┼───────┼───────┼────────┼──────────┼────────┤
// | 10  | on/c  |       |        |          |        |
// └─────┴───────┴───────┴────────┴──────────┴────────┘
extern unsigned char key_matrix[5][11];

typedef enum keys {
  KEY_F1 = (0 << 3) + 0,
  KEY_F2 = (1 << 3) + 1,
  KEY_F3 = (0 << 3) + 1,
  KEY_F4 = (0 << 3) + 2,
  KEY_F5 = (0 << 3) + 3,
  KEY_F6 = (1 << 3) + 3,
  KEY_UP = (0 << 3) + 4,
  KEY_RIGHT = (1 << 3) + 4,
  KEY_LEFT = (2 << 3) + 4,
  KEY_DOWN = (3 << 3) + 4,
  KEY_SYMB = (1 << 3) + 0,
  KEY_NUM = (1 << 3) + 2,
  KEY_HOME = (2 << 3) + 0,
  KEY_PLOT = (2 << 3) + 1,
  KEY_VIEWS = (2 << 3) + 2,
  KEY_XTPHIN = (2 << 3) + 3,
  KEY_VARS = (3 << 3) + 0,
  KEY_APPS = (3 << 3) + 1,
  KEY_ABC = (3 << 3) + 2,
  KEY_BACKSPACE = (3 << 3) + 3,
  KEY_SIN = (4 << 3) + 0,
  KEY_MATH = (4 << 3) + 1,
  KEY_TAN = (4 << 3) + 2,
  KEY_LN = (4 << 3) + 3,
  KEY_LOG = (4 << 3) + 4,
  KEY_X2 = (5 << 3) + 0,
  KEY_COS = (5 << 3) + 1,
  KEY_LEFTBRACKET = (5 << 3) + 2,
  KEY_RIGHTBRACET = (5 << 3) + 3,
  KEY_DIVISION = (5 << 3) + 4,
  KEY_COMMA = (6 << 3) + 0,
  KEY_XY = (6 << 3) + 1,
  KEY_8 = (6 << 3) + 2,
  KEY_9 = (6 << 3) + 3,
  KEY_MULTIPLICATION = (6 << 3) + 4,
  KEY_ALPHA = (7 << 3) + 0,
  KEY_7 = (7 << 3) + 1,
  KEY_5 = (7 << 3) + 2,
  KEY_6 = (7 << 3) + 3,
  KEY_SUBTRACTION = (7 << 3) + 4,
  KEY_SHIFT = (8 << 3) + 0,
  KEY_4 = (8 << 3) + 1,
  KEY_2 = (8 << 3) + 2,
  KEY_3 = (8 << 3) + 3,
  KEY_PLUS = (8 << 3) + 4,
  KEY_0 = (9 << 3) + 0,
  KEY_1 = (9 << 3) + 1,
  KEY_DOT = (9 << 3) + 2,
  KEY_NEGATIVE = (9 << 3) + 3,
  KEY_ENTER = (9 << 3) + 4,
  KEY_ON = (10 << 3)
}keys;

void set_row_line(int row_line);
unsigned int read_col_line(int col_line);


void keyboard_init();
unsigned int is_key_ON_down();
void key_scan();
unsigned int is_key_down(keys key);

#ifdef __cplusplus 
} 
#endif
#endif

