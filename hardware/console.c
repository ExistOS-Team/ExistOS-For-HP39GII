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
 
#include "display.h"
#include "console.h" 
#include "uart_debug.h"

unsigned char consoleBuffer[CONSOLE_BUFFER_SIZE];
console_position_info cursorPosition;
console_position_info currentFlushPosition;


void console_flush_newline(){
	
	
	
}

unsigned int i;

void console_puts(unsigned char s){
	
	
	
	switch(s){
		case '\r':
		case '\n':
			cursorPosition.x = 0;
			cursorPosition.y++;
		break;
		
		default:
			consoleBuffer[cursorPosition.x + cursorPosition.y * CONSOlE_DEFAULT_MAX_WIDTH] = s;
			cursorPosition.x++;
		break;
	}
	
	if(cursorPosition.x >= CONSOlE_DEFAULT_MAX_WIDTH) {
		cursorPosition.x = 0;
		cursorPosition.y++;
		
		//console_flash_to_screen(currentFlushPosition);
	}
	
	if(cursorPosition.y >= CONSOLE_BUFFER_SIZE / CONSOlE_DEFAULT_MAX_WIDTH) {
		cursorPosition.y = 0;
	}
	

	
	//console_flash_to_screen(currentFlushPosition);
}



void console_init(){
	for(unsigned int i=0;i<CONSOLE_BUFFER_SIZE;i++){
		consoleBuffer[i] = ' ';
	}
	
	
	cursorPosition.x = 0;
	cursorPosition.y = 0;
	currentFlushPosition.x = 0;
	currentFlushPosition.y = 0;
	
	LCD_scroll_on();
	
}

