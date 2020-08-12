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
#include "regspinctrl.h"
#include "regslcdif.h"
#include "regsdigctl.h"
#include "regsuartdbg.h"
#include "regsapbh.h"
#include "mmu.h"
#include "uart_debug.h"
#include "utils.h"
#include "display.h"
#include "keyboard.h"
#include "exception.h"
#include "irq.h"

extern const unsigned char test_picture[32768];	//128*256 4阶灰度图片

void _boot()
{
	unsigned int time = 0;
	unsigned char tmp;

	switch_mode(SVC_MODE);	//切换到系统管理模式
	disable_interrupts();	//关闭所有中断
	exception_init();
	irq_init();
	DFLTP_init();			//初始化页表
	enable_mmu();			//开启内存映射


	LCD_init();				//屏幕初始化
	keyboard_init();		//键盘初始化
	LCD_set_contrast(0x38);	//设置对比度
	
	LCD_clear_buffer();		//清屏
	//显示图片 256x128
	for(int y=0; y<127; y++)
		{
			for(int x=0; x<255; x++)
				{
					//LCD_write_pix(x,y,255-test_picture[x+y*256]);	//将图片像素写入到显存中
				}
		}
		
		
	LCD_ShowString(1,1,24*16,24,24,255,"Hello World!");
	LCD_ShowString(1,25,24*12,24,16,255,"Hello World!");
	LCD_ShowString(1,40,24*12,24,12,255,"Hello World!");

	LCD_dma_flush_buffer();	//刷新显示
	

	while(1)
		{
			uartdbg_printf("time: %d CPUID:%x\n",time,read_cpuid());//每间隔一秒串口输出启动时间、CPU和寄存器信息
			uartdbg_print_regs();
			time++;
			delay_us(1000000);

		}
}

