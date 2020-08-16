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
 
  
#include "console.h" 
#include "uart_debug.h"

unsigned char consoleBuffer[CONSOLE_BUFFER_SIZE];
console_position_info currentWritePosition;
console_position_info currentFlushPosition;


void console_flush_newline(){
	unsigned int offset = currentFlushPosition.y * CONSOlE_DEFAULT_MAX_WIDTH;
	LCD_clear_area(0,currentFlushPosition.y,255,currentFlushPosition.y + CONSOlE_DEFAULT_FONT_SIZE);
	/*
	for(int x=0; x<CONSOlE_DEFAULT_MAX_WIDTH ;x++){
		LCD_show_char(x*(CONSOlE_DEFAULT_FONT_SIZE/2),currentFlushPosition.y,consoleBuffer[offset]);
	}*/
	
	currentFlushPosition.y = ((currentFlushPosition.y + CONSOlE_DEFAULT_FONT_SIZE) % (CONSOlE_DEFAULT_FONT_SIZE)) % CONSOlE_DEFAULT_FONT_SIZE;
}

unsigned int i;

void console_puts(unsigned char s){
	
	switch(s){
		//case '\r':
		case '\n':
			currentWritePosition.x = 0;
			currentWritePosition.y++;
		LCD_scroll_up((LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT));
		LCD_clear_area(0, (currentWritePosition.y*(LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT)) ,255, (currentWritePosition.y*(LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT) + (LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT))+1);			
		break; 
		
		default:
			consoleBuffer[currentWritePosition.x + currentWritePosition.y * CONSOlE_DEFAULT_MAX_WIDTH] = s;
			LCD_show_char((currentWritePosition.x*(CONSOlE_DEFAULT_FONT_SIZE/2))%LCD_L , (currentWritePosition.y*((LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT)))%LCD_H,s,CONSOlE_DEFAULT_FONT_SIZE,255,1);	
			currentWritePosition.x++;
		break;
	}
	 
	
	if(currentWritePosition.x >= CONSOlE_DEFAULT_MAX_WIDTH) {
		currentWritePosition.x = 0;
		currentWritePosition.y++;
		LCD_scroll_up((LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT));
		LCD_clear_area(0, (currentWritePosition.y*(LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT)) ,255, (currentWritePosition.y*(LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT) + (LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT))+1);
		//console_flash_to_screen(currentFlushPosition);
	}
	
	if(currentWritePosition.y >= CONSOlE_DEFAULT_MAX_HEIGHT) {
		currentWritePosition.y  = 0;
		LCD_clear_area(0, (currentWritePosition.y*(LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT)) ,255, (currentWritePosition.y*(LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT) + (LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT))+1);
		//currentWritePosition.y = currentWritePosition.y % (CONSOlE_DEFAULT_MAX_HEIGHT);
		//currentWritePosition.x = 0;
		//LCD_scroll_reset();
		//LCD_scroll_up(CONSOlE_DEFAULT_FONT_SIZE);
		//LCD_clear_area(0,currentWritePosition.y*(CONSOlE_DEFAULT_FONT_SIZE),255,currentWritePosition.y*(CONSOlE_DEFAULT_FONT_SIZE) + CONSOlE_DEFAULT_FONT_SIZE);
	}

}
 


void console_init(){
	for(unsigned int i=0;i<CONSOLE_BUFFER_SIZE;i++){
		consoleBuffer[i] = ' ';
	}
	
	currentWritePosition.x = 0;
	currentWritePosition.y = 0;
	currentFlushPosition.x = 0;
	currentFlushPosition.y = 0;
	
	LCD_scroll_on();
	
	LCD_scroll_up((LCD_H/CONSOlE_DEFAULT_MAX_HEIGHT));
}

