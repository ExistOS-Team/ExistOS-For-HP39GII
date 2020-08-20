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
#include "regsclkctrl.h"
#include "regsicoll.h"
#include "mmu.h"
#include "uart_debug.h"
#include "utils.h"
#include "display.h"
#include "keyboard.h"
#include "exception.h"
#include "irq.h"
#include "hw_irq.h"
#include "memory.h"
#include "console.h"
#include "nand.h"
#include "timer.h"
#include "flash_mapping.h"
#include "./filesystem/fatfs/ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


extern unsigned char key_matrix[5][11];

extern int main();
extern void fs_test_main();

// lv_disp_t * disp;
// void lv_disp_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
// {
//     int32_t x, y;
//     for(y = area->y1; y <= area->y2; y++) {
//         for(x = area->x1; x <= area->x2; x++) {
// 			printf("get pix:%d,%d,%d",x,y, color_p->full);
//             LCD_write_pix(x, y, color_p->full);
//             color_p++;
//         }
//     }
//     /* IMPORTANT!!!
//      * Inform the graphics library that you are ready with the flushing*/
//     lv_disp_flush_ready(disp);
// }
//  static lv_color_t buf_1[10];
 
void _boot()
{
	disable_interrupts();					//关闭所有中断
	switch_mode(SVC_MODE);					//切换到系统管理模式
	asm volatile ("ldr sp,=#0x0007F000");	//设置系统管理模式下的栈地址
	exception_init();						//初始化异常向量
	irq_init();								//初始化中断
	DFLTP_init();							//初始化页表
	enable_mmu();							//开启内存映射
	stack_init();							//栈初始化（设定异常、系统、中断等模式下的堆栈
	enable_interrupts();					//打开中断

	LCD_init();								//屏幕初始化
	keyboard_init();						//键盘初始化
	LCD_set_contrast(0x40);					//设置对比度
	LCD_clear_buffer();						//清屏缓存
	LCD_dma_flush_auto_buffer_start();		//开启自动刷屏
	//malloc_init();
	console_init();
	NAND_init();
	
	
	BF_CS1(UARTDBGIBRD,BAUD_DIVINT,1);
	BF_CS1(UARTDBGFBRD,BAUD_DIVFRAC,36);
	BF_CS1(UARTDBGLCR_H,FEN,1);	
	
	

	
	// lv_init();

    // static lv_disp_buf_t disp_buf;
   	// printf("Step");
	// fflush(stdout);
    // lv_disp_buf_init(&disp_buf, buf_1,NULL,10);
	// printf("Step");
	// fflush(stdout);
	// lv_disp_drv_t disp_drv;                 /*A variable to hold the drivers. Can be local variable*/
    // lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	// printf("Step");
	// fflush(stdout);
    // disp_drv.buffer = &disp_buf;            /*Set an initialized buffer*/
    // disp_drv.flush_cb = lv_disp_flush_cb;        /*Set a flush callback to draw to the display*/
    // disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	// printf("Step");
	// fflush(stdout);
	// lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    // lv_obj_set_pos(btn, 0, 0);                            /*Set its position*/
    // lv_obj_set_size(btn, 120, 50);                          /*Set its size*/              
    // lv_obj_t * label = lv_label_create(btn, NULL);          /*Add a label to the button*/
    // lv_label_set_text(label, "Button");                     /*Set the labels text*/
	// lv_obj_t * label1 = lv_label_create(lv_scr_act(), NULL);
	// lv_label_set_text(label1, "Hello world!"); 
	// lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
	// lv_obj_align(btn, label1, LV_ALIGN_OUT_TOP_MID, 0, -10);
	// printf("Step");
	// fflush(stdout);
	// while(1)
	// {
	// 	lv_task_handler();
	// }
	
	//main();
	
	
	
	//Flash_mapping_init();
	
	//fs_test_main();
	
	
	main();
	printf("System halt.");
	fflush(stdout);
	
	while(1)
		{

  /*
			fflush(stdout);
			//LCD_clear_buffer();
			key_scan();
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 11; j++)
				{
					if(key_matrix[i][j]==1){
						//char buf[30];
						//snprintf(buf, 30, "pressed key:%d,%d",i,j);
						//LCD_show_string(0,0,24*16,24,24,255,buf);
						printf("pressed key:%d,%d",i,j);
					};
				};
				
			};
			delay_us(100000);
*/
			//LCD_dma_flush_buffer();
			
			/*
			uartdbg_printf("time: %d CPUID:%x\n",HW_DIGCTL_MICROSECONDS_RD(),read_cpuid());//串口输出启动时间、CPU和寄存器信息
			uartdbg_print_regs();

			LCD_clear_buffer();		//清显存
			LCD_show_string(1,1,24*16,24,24,255,"Hello World!");
			LCD_show_string(1,25,24*12,24,16,255,"Hello World!");
			LCD_show_string(1,40,24*12,24,12,255,"Hello World!");

			delay_us(1000000);
			LCD_clear_buffer();
			//显示图片 256x128
			for(int y=0; y<127; y++) 
				{
					for(int x=0; x<255; x++)
						{
							LCD_write_pix(x,y,255-test_picture[x+y*256]);	//将屏幕测试图片像素写入到显存中
						}
				}
				while(1)
				{	
					LCD_scroll_up(1);
				delay_us(20000);
				}
			
			//delay_us(20000);
			*/

		}
}
