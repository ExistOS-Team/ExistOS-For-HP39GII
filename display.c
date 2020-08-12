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
#include "regsapbh.h"
#include "display.h"
#include "utils.h"

hw_lcdif_DmaDesc screen_buffer_dma_desc;

unsigned char *screen_buffer = (unsigned char *)VRAM_BASE; //显存

unsigned int LCD_is_busy()
{
	return BF_RD(LCDIF_CTRL, RUN);
}

//向LCD发送数据
void LCD_writeDat(unsigned int dat, unsigned int dat_size)
{

	while(LCD_is_busy());
	BF_CS1(LCDIF_CTRL,DATA_SELECT,1);
	BF_CS1(LCDIF_CTRL,READ_WRITEB,0);
	BF_CS1 (LCDIF_CTRL, COUNT, dat_size);
	BF_CS1 (LCDIF_CTRL, RUN, 1);
	HW_LCDIF_DATA_WR(dat);
}

//向LCD发送命令
void LCD_writeCmd(unsigned int cmd, unsigned int cmd_size)
{

	while(LCD_is_busy());
	BF_CS1 (LCDIF_CTRL, DATA_SELECT,0);
	BF_CS1(LCDIF_CTRL,READ_WRITEB,0);
	BF_CS1 (LCDIF_CTRL, COUNT, cmd_size);
	BF_CS1 (LCDIF_CTRL, RUN, 1);
	HW_LCDIF_DATA_WR(cmd);
}

void LCD_readDat(unsigned char *buffer,unsigned int size_bytes, unsigned char first_read_dummy)
{
	while(LCD_is_busy());
	BF_CS1 (LCDIF_CTRL, DATA_SELECT,1);
	BF_CS1(LCDIF_CTRL,READ_WRITEB,1);
	BF_CS1 (LCDIF_CTRL, COUNT, size_bytes);
	BF_CS1 (LCDIF_CTRL1,READ_MODE_NUM_PACKED_SUBWORDS,1);	//一次读1字节
	BF_CS1 (LCDIF_CTRL1,FIRST_READ_DUMMY,first_read_dummy & 1);
	BF_CS1 (LCDIF_CTRL, RUN, 1);
	while(BF_RD(LCDIF_CTRL,RUN))
		{
			while(BF_RD(LCDIF_STAT,RXFIFO_EMPTY));
			*(buffer++) = BF_RD(LCDIF_DATA,DATA_ZERO);
		}
}

void LCD_setxy(unsigned int x, unsigned int y)
{
//列地址
	LCD_writeCmd(0x2A,1);
	LCD_writeDat(((x)&0xFF00)>>16,1);
	switch(pix_format)
		{
		case PIX_FORMAT_GRAY4:
			LCD_writeDat((x/3)&0xFF,1);
			break;
		case PIX_FORMAT_GRAY256:
			LCD_writeDat((x)&0xFF,1);
			break;
		}

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
void LCD_flush_buffer(void)
{
	LCD_setxy(0,0);
	switch(pix_format)
		{
		case PIX_FORMAT_GRAY4:
			for(int i=0; i<(255/3+1)*128; i+=4)LCD_writeDat(*((unsigned int *)(screen_buffer + i)),4);
			break;
		case PIX_FORMAT_GRAY256:
			for(int i=0; i<258*128; i+=4)LCD_writeDat(*((unsigned int *)(screen_buffer + i)),4);
			break;
		}
}

//清空显存
void LCD_clear_buffer(void)
{

	for(int i=0; i<256*128; i++)screen_buffer[i]=0;

}

//往显存写入一个像素点，颜色范围0~3
void LCD_write_pix(unsigned int x,unsigned int y,unsigned char color)
{
	unsigned char tmp;
	switch(pix_format)
		{
		case PIX_FORMAT_GRAY4:
			tmp = screen_buffer[ (x/3)+y*(255/3+1) ];
			switch(x%3)
				{
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
			screen_buffer[ (x/3)+y*(255/3+1) ] = tmp | 0x24;
			break;
		case PIX_FORMAT_GRAY256:
			screen_buffer[x+258*y]=color;
			break;
		}

}

void LCD_dma_channel_reset(void)
{
	BF_CS2(APBH_CTRL0, SFTRST, 0, CLKGATE, 0);
	BF_CS1(APBH_CTRL0, RESET_CHANNEL,(BF_RD(APBH_CTRL0,RESET_CHANNEL)|0x01));
	while((BF_RD(APBH_CTRL0,RESET_CHANNEL)&0x01));

	//此处应该开启IRQ

	screen_buffer_dma_desc.p_Next = 0;
	screen_buffer_dma_desc.Semaphore = 1;
	screen_buffer_dma_desc.Command = LCDIF_DMA_READ;
	screen_buffer_dma_desc.Chain = 0;
	screen_buffer_dma_desc.IRQOnCompletion = 0;
	screen_buffer_dma_desc.WaitForEndCommand = 0;
	screen_buffer_dma_desc.p_DMABuffer = screen_buffer;
	screen_buffer_dma_desc.PIOWords = 1;

	screen_buffer_dma_desc.PioWord.B.WORD_LENGTH=1;
	screen_buffer_dma_desc.PioWord.B.DATA_SELECT=1;
	screen_buffer_dma_desc.PioWord.B.RUN=1;
	screen_buffer_dma_desc.PioWord.B.BYPASS_COUNT=0;
	screen_buffer_dma_desc.PioWord.B.READ_WRITEB=0;

}

void LCD_dma_flush_buffer()
{
	LCD_setxy(0,0);

	switch(pix_format)
		{
		case PIX_FORMAT_GRAY4:
			screen_buffer_dma_desc.DMABytes = (255/3+1)*128;
			screen_buffer_dma_desc.PioWord.B.COUNT= (255/3+1)*128;
			break;
		case PIX_FORMAT_GRAY256:
			screen_buffer_dma_desc.DMABytes = 256*128;
			screen_buffer_dma_desc.PioWord.B.COUNT= 256*128;
			break;
		}


	while(LCD_is_busy());

	BF_WRn(APBH_CHn_NXTCMDAR, 0, CMD_ADDR, (reg32_t)&screen_buffer_dma_desc);
	BW_APBH_CHn_SEMA_INCREMENT_SEMA(0, 1);

}

//设置对比度 范围 0-127
void LCD_set_contrast(unsigned char contrast)
{
	LCD_writeCmd(0x25,1);
	LCD_writeDat(contrast & 0x7F,1);
}

void LCD_set_pix_format(hw_lcdif_DmaCommand pix_format)
{
	LCD_writeCmd(0x3A,1);
	LCD_writeDat(pix_format,1);
}

void LCD_init(void)
{
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

	BF_CS4 (LCDIF_TIMING,DATA_SETUP,10,DATA_HOLD,10,CMD_SETUP,10,CMD_HOLD,10);	//设置各个信号线的建立、保持时间

	delay_us(500);
	HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_LCD_CS_CTRL);	//CS拉低，选中屏幕
	HW_LCDIF_CTRL1_CLR(BM_LCDIF_CTRL1_RESET);		//RESET拉低10ms，硬件复位
	delay_us(100000);
	HW_LCDIF_CTRL1_SET(BM_LCDIF_CTRL1_RESET);
	delay_us(100000);

	pix_format = PIX_FORMAT_GRAY256;	//默认使用256级灰度

	//硬件复位完成，开始发送LCD初始化序列
	LCD_writeCmd(0x01,1);
	LCD_writeCmd(0x01,1);

	LCD_writeCmd(0xD7,1);	// Auto Load Set
	LCD_writeDat(0x9F,1);
	LCD_writeCmd(0xE0,1);	// EE Read/write mode
	LCD_writeDat(0x00,1);	// Set read mode
	delay_us(100000);
	LCD_writeCmd(0xE3,1);	// Read active
	delay_us(100000);
	LCD_writeCmd(0xE1,1);	// Cancel control

	LCD_writeCmd(0x11,1);	// sleep out

	delay_us(500000);

	LCD_writeCmd(0x28,1);	//Display off
	LCD_writeCmd(0xC0,1);	//Set Vop by initial Module
	LCD_writeDat(0x01,1);
	LCD_writeDat(0x01,1);	// base on Module

	LCD_writeCmd(0xF0,1);	// Set Frame Rate
	LCD_writeDat(0x0D,1);
	LCD_writeDat(0x0D,1);
	LCD_writeDat(0x0D,1);
	LCD_writeDat(0x0D,1);

	LCD_writeCmd(0xC3,1);	// Bias select
	LCD_writeDat(0x02,1);

	LCD_writeCmd(0xC4,1);	// Setting Booster times
	LCD_writeDat(0x07,1);

	LCD_writeCmd(0xD0,1);	// Analog circuit setting
	LCD_writeDat(0x1D,1);

	LCD_writeCmd(0xB5,1);	// N-Line
	LCD_writeDat(0x8C,1);
	LCD_writeDat(0x00,1);
	LCD_writeCmd(0x38,1);	// Idle mode off
	LCD_writeCmd(0x3A,1);	// pix format
	LCD_writeDat(pix_format,1);

	LCD_writeCmd(0x36,1);	// Memory Access Control
	LCD_writeDat(0x48,1);
	LCD_writeCmd(0xB0,1);	// Set Duty
	LCD_writeDat(0x87,1);
	LCD_writeCmd(0xB4,1);
	LCD_writeDat(0xA0,1);

	LCD_writeCmd(0x29,1);	//Display on

	LCD_dma_channel_reset();
}



