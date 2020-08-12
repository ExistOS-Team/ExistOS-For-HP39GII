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
#include "regsclkctrl.h"
#include "regspinctrl.h"
#include "regslcdif.h"
#include "utils.h"

unsigned char screen_buffer[(255/3+1)*128];	//显存
unsigned int  sent_delay = 0;	//每发送一条数据或指令后延时多少us

//向LCD发送数据
void LCD_writeDat(unsigned int dat, unsigned int dat_size){
	
	if(BF_RD(LCDIF_CTRL,RUN)==1){
		delay_us(1000);
		if(BF_RD(LCDIF_CTRL,RUN)==1){
			delay_us(100000);
		}
	}
	BF_CS1(LCDIF_CTRL,DATA_SELECT,1);	
	BF_CS1 (LCDIF_CTRL, COUNT, dat_size);
	BF_CS1 (LCDIF_CTRL, RUN, 1);
	HW_LCDIF_DATA_WR(dat);
	delay_us(sent_delay);
}

//向LCD发送命令
void LCD_writeCmd(unsigned int cmd, unsigned int cmd_size){

	if(BF_RD(LCDIF_CTRL,RUN)==1){
		delay_us(1000);
		if(BF_RD(LCDIF_CTRL,RUN)==1){
			delay_us(10000);
		}
	}
	BF_CS1 (LCDIF_CTRL, DATA_SELECT,0);
	BF_CS1 (LCDIF_CTRL, COUNT, cmd_size);
	BF_CS1 (LCDIF_CTRL, RUN, 1);
	HW_LCDIF_DATA_WR(cmd);
	delay_us(sent_delay);
}

void LCD_setxy(unsigned int x, unsigned int y){
//列地址
LCD_writeCmd(0x2A,1);
LCD_writeDat(((x)&0xFF00)>>16,1);
LCD_writeDat((x/3)&0xFF,1);
LCD_writeDat(0x00,1);
LCD_writeDat(0x55,1);

//行地址
LCD_writeCmd(0x2B,1);
LCD_writeDat((y&0xFF00)>>16,1);
LCD_writeDat((y&0xFF)+8,1);
LCD_writeDat(0x00,1);
LCD_writeDat(0x87,1);

//开始写LCD
LCD_writeCmd(0x2C,1);	
}

//将显存的数据刷新到LCD
void LCD_flush_buffer(void){
	LCD_setxy(0,0);
	for(int i=0;i<(255/3+1)*128;i+=4)LCD_writeDat(*((unsigned int *)(screen_buffer + i)),4);
}

//清空显存
void LCD_clear_buffer(void){
	for(int i=0;i<(255/3+1)*128;i++)screen_buffer[i]=0;
}

//往显存写入一个像素点，颜色范围0~3
void LCD_write_pix(unsigned int x,unsigned int y,unsigned char color){
	unsigned char tmp;
	tmp = screen_buffer[ (x/3)+y*(255/3+1) ];
	switch(x%3){
		case 2:
			tmp &= 0xFC;
			tmp |= ((color & 0x3));
			break;
		case 1:
			tmp &= 0xE7;
			tmp |= ((color & 0x3) << 3);
			break;		
		case 0:
			tmp &= 0x3F;
			tmp |= ((color & 0x3) << 6);
			break;				
		default:
			break;
	}
	screen_buffer[ (x/3)+y*(255/3+1) ] = tmp;
}

void LCD_init(void)
{
	sent_delay = 5000;
	BF_CS8 (PINCTRL_MUXSEL2,
	        BANK1_PIN07, 0,
	        BANK1_PIN06, 0,
	        BANK1_PIN05, 0,
	        BANK1_PIN04, 0,
	        BANK1_PIN03, 0,
	        BANK1_PIN02, 0,
	        BANK1_PIN01, 0,
	        BANK1_PIN00, 0);
	//设置引脚复用寄存器，连接至SoC内部的LCD控制器
	BF_CS5 (
	    PINCTRL_MUXSEL3,
	    BANK1_PIN20,0,
	    BANK1_PIN19,0,
	    BANK1_PIN18,0,
	    BANK1_PIN17,0,
	    BANK1_PIN16,0
	);
	//设置引脚复用寄存器，连接至SoC内部的LCD控制器

	BF_CS2(CLKCTRL_PIX,CLKGATE,0,DIV,0x1);		//设置主时钟分频
	HW_LCDIF_CTRL_CLR(BM_LCDIF_CTRL_CLKGATE);
	HW_LCDIF_CTRL_SET(BM_LCDIF_CTRL_SFTRST);	//重置LCD控制器
	delay_us(100);
	HW_LCDIF_CTRL_CLR(BM_LCDIF_CTRL_SFTRST | BM_LCDIF_CTRL_CLKGATE);	//将LCD控制器唤醒

	HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_MODE86);
	HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_LCD_CS_CTRL);		//设置LCD控制器使用8080总线模式进行通信

	HW_LCDIF_CTRL1_SET(BM_LCDIF_CTRL1_RESET);			//拉高RESET线

	HW_LCDIF_VDCTRL0_SET(BM_LCDIF_VDCTRL0_VSYNC_OEB);	//VSYNC引脚作为输入（实际上不使用）

	BF_CS2 (LCDIF_CTRL, VSYNC_MODE, 0, WORD_LENGTH, 1);	//不使用VSYNC模式，设置总线带宽为8bit
	BF_CS1 (LCDIF_CTRL, BYPASS_COUNT, 0);

	BF_CS4 (LCDIF_TIMING,DATA_SETUP,0x8,DATA_HOLD,0x8,CMD_SETUP,0x8,CMD_HOLD,0x8);	//设置各个信号线的建立、保持时间
	delay_us(500);
	HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_LCD_CS_CTRL);	//CS拉低，选中屏幕
	HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_RESET);		//RESET拉低10ms，硬件复位
	delay_us(100000);
	HW_LCDIF_CTRL1_SET(BM_LCDIF_CTRL1_RESET);
	delay_us(100000);
	//硬件复位完成，开始发送LCD初始化序列
	LCD_writeCmd(0x01,1);
	LCD_writeCmd(0x01,1);
	
	LCD_writeCmd(0xD7,1);
	LCD_writeDat(0x9F,1);
	LCD_writeCmd(0xE0,1);
	LCD_writeDat(0x00,1);

	LCD_writeCmd(0xE3,1);

	LCD_writeCmd(0xE1,1);

	LCD_writeCmd(0x11,1);	//sleep out

	LCD_writeCmd(0x28,1);	//Display off
	LCD_writeCmd(0xC0,1);
	LCD_writeDat(0x01,1);
	LCD_writeDat(0x01,1);
	LCD_writeCmd(0xF0,1);
	LCD_writeDat(0x0D,1);
	LCD_writeDat(0x0D,1);
	LCD_writeDat(0x0D,1);
	LCD_writeDat(0x0D,1);
	LCD_writeCmd(0xC3,1);
	LCD_writeDat(0x02,1);

	LCD_writeCmd(0xC4,1);
	LCD_writeDat(0x07,1);

	LCD_writeCmd(0xD0,1);
	LCD_writeDat(0x1D,1);

	LCD_writeCmd(0xB5,1);
	LCD_writeDat(0x8C,1);
	LCD_writeDat(0x00,1);
	LCD_writeCmd(0x38,1);
	LCD_writeCmd(0x3A,1);	//pix format
	LCD_writeDat(0x02,1);

	LCD_writeCmd(0x36,1);
	LCD_writeDat(0x48,1);
	LCD_writeCmd(0xB0,1);
	LCD_writeDat(0x87,1);
	LCD_writeCmd(0xB4,1);
	LCD_writeDat(0xA0,1);

	LCD_writeCmd(0x29,1);	//Display on
	sent_delay = 70;
}

