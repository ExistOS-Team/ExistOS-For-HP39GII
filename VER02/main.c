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

#include <stdarg.h>
#include "regsdigctl.h"
#include "regsuartdbg.h"
#include "uart_debug.h"
#include "utils.h"
#include "display.h"

extern const unsigned char test_picture[8192];	//128*256 4阶灰度图片

void _boot(){
	unsigned int time = 0;
	unsigned char tmp;
	
	LCD_init();	//屏幕初始化
	LCD_clear_buffer();	//清屏
	LCD_flush_buffer();	//刷新显示
	
	//显示图片 128x256
	for(int y=0;y<127;y++){
		for(int x=0;x<255;x++){
			tmp = test_picture [(x/4)+y*(256/4)]; //从图片中取出对应x,y的像素数据
			switch(x%4){
				case 3:
					tmp = (tmp&0x3);
					break;
				case 2:
					tmp = ((tmp>>2)&0x3);
					break;
				case 1:
					tmp = ((tmp>>4)&0x3);
					break;
				case 0:
					tmp = ((tmp>>6)&0x3);
					break;
				default:
					break;
			}
			LCD_write_pix(x,y,tmp);	//将图片像素写入到显存中
		}
	}
	
	LCD_flush_buffer();	//刷新显示

	while(1){
		uartdbg_printf("micro seconds:%d\n",HW_DIGCTL_MICROSECONDS_RD());	//每隔1s串口打印出当前经过的微秒数
		delay_us(1000000);
	}
}

