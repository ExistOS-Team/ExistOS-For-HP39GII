/*
 * A simple user interface for this project
 *
 * Copyright 2020 LiberCalc Team
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
 
#ifndef __CONSOLE_H
#define __CONSOLE_H

typedef struct {
	unsigned int x;
	unsigned int y;
} console_position_info;



#define CONSOlE_DEFAULT_FONT_SIZE	12
#define CONSOlE_DEFAULT_FONT_COLOR	255

#define CONSOlE_DEFAULT_MAX_WIDTH		41
#define CONSOlE_DEFAULT_MAX_HEIGHT		9

#define CONSOLE_BUFFER_SIZE	(CONSOlE_DEFAULT_MAX_HEIGHT * CONSOlE_DEFAULT_MAX_WIDTH * 11)

void console_puts(unsigned char s);
void console_init();
void console_flush();


#endif

