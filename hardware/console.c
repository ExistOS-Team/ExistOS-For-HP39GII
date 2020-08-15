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
unsigned char consoleBuffer2[CONSOLE_BUFFER_SIZE];
console_position_info cursorPosition;
console_position_info currentFlushPosition;

void console_flash_to_screen(console_position_info startFlushBufferPosition){
	unsigned int offset = startFlushBufferPosition.x + startFlushBufferPosition.y * CONSOlE_DEFAULT_MAX_WIDTH;
	for(unsigned int y=0; y <= CONSOlE_DEFAULT_MAX_HEIGHT; y++) {
		for(unsigned int x=0; x < CONSOlE_DEFAULT_MAX_WIDTH; x++) {
			if(offset>CONSOLE_BUFFER_SIZE){
				offset=0;
			}
			if(consoleBuffer2[offset] == consoleBuffer[offset]){
				continue;
			}
			consoleBuffer2[offset] = consoleBuffer[offset];
			LCD_show_char(x*(CONSOlE_DEFAULT_FONT_SIZE/2),y*CONSOlE_DEFAULT_FONT_SIZE,consoleBuffer[offset],CONSOlE_DEFAULT_FONT_SIZE,CONSOlE_DEFAULT_FONT_COLOR,0);
			offset++;
			
		}
	}	
}
unsigned int i;

void console_puts(unsigned char s){
	
	
	
	switch(s){
		case '\r':
		case '\n':
			cursorPosition.x = 0;
			cursorPosition.y++;
			//LCD_clear_buffer();
		break;
		
		default:
			consoleBuffer[cursorPosition.x + cursorPosition.y * CONSOlE_DEFAULT_MAX_WIDTH] = s;
			cursorPosition.x++;
		break;
	}
	
	if(cursorPosition.x >= CONSOlE_DEFAULT_MAX_WIDTH) {
		cursorPosition.x = 0;
		cursorPosition.y++;
		
		console_flash_to_screen(currentFlushPosition);
	}
	
	if(cursorPosition.y >= CONSOLE_BUFFER_SIZE / CONSOlE_DEFAULT_MAX_WIDTH) {
		cursorPosition.y = 0;
	}
	
	if((cursorPosition.y - CONSOlE_DEFAULT_MAX_HEIGHT) < 0){
		currentFlushPosition.y = (CONSOLE_BUFFER_SIZE / CONSOlE_DEFAULT_MAX_WIDTH) - CONSOlE_DEFAULT_MAX_HEIGHT;
		currentFlushPosition.x = 0;
	}else{
		currentFlushPosition.y = (cursorPosition.y - CONSOlE_DEFAULT_MAX_HEIGHT);
		currentFlushPosition.x = 0;
	}
	
	console_flash_to_screen(currentFlushPosition);
}

void console_flush(){
	LCD_clear_buffer();
	console_flash_to_screen(currentFlushPosition);
}

void console_init(){
	for(unsigned int i=0;i<CONSOLE_BUFFER_SIZE;i++){
		consoleBuffer[i] = ' ';
	}
	
	
	cursorPosition.x = 0;
	cursorPosition.y = 0;
	currentFlushPosition.x = 0;
	currentFlushPosition.y = 0;
}

