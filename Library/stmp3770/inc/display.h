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

#ifndef _DISPLAY_H
#define _DISPLAY_H
#include <stdint.h>
#include "regslcdif.h"

#ifdef __cplusplus 
extern "C" { 
#endif

//#define 	VRAM_BASE						0x00001000		//显存基址
#define 	LCD_TIMOUT_US					100000		//数据超时时间

#define 	LCD_H	128
#define 	LCD_L	256

#define		LCD_COMMANDE_BUFFER_SIZE		4			//命令缓冲区大小
#define		LCD_PARAMETER_BUFFER_SIZE		32			//命令参数缓冲区大小

static unsigned int pix_format;



typedef enum lcd_pix_format	//LCD灰度模式
{
    PIX_FORMAT_GRAY4	= 2,
    PIX_FORMAT_GRAY256 	= 7
} lcd_pix_format;



typedef enum lcdSendStatus
{
    NORMAL_S1 		= 0,
    NORMAL_S2 	,
    COMMAND	,	 
    PARAMETER  
} lcdSendStatus;

typedef enum hw_lcdif_DmaCommand
{
    LCDIF_NO_DMA_XFER = 0,
    LCDIF_DMA_WRITE = 1,		//从设备写到内存
    LCDIF_DMA_READ = 2			//从内存读到设备
} hw_lcdif_DmaCommand;


typedef struct _hw_lcdif_DmaDesc	//用于LCD屏幕控制器的DMA描述符
{
    struct _hw_lcdif_DmaDesc *p_Next;
    union 
    {
        struct 
        {
            union 
            {
                struct 
                {
                    hw_lcdif_DmaCommand Command : 2;
                    unsigned char Chain : 1;
                    unsigned char IRQOnCompletion : 1;
                    unsigned char NANDLock : 1;
                    unsigned char NANDWaitForReady : 1;
                    unsigned char Semaphore : 1;
                    unsigned char WaitForEndCommand : 1;
                } ;
                unsigned char Bits;
            };
            unsigned int Reserved : 4;
            unsigned char PIOWords : 4;
            unsigned short DMABytes: 16;
        };
        unsigned int CommandBits;
    };
    void *p_DMABuffer;
    hw_lcdif_ctrl_t PioWord;

} hw_lcdif_DmaDesc;


void LCD_dma_flush_auto_buffer_stop(void);
void LCD_dma_flush_auto_buffer_start(void);
void LCD_scroll_reset();
void LCD_scroll_on();
void LCD_scroll_up(unsigned int pixs);
void LCD_scroll_down(unsigned int pixs);

void LCD_clear_area(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

/*===========================================================================
\brief       从LCD中读取数据

\param[in]	 *buffer  				指向数据回读的地址         
\param[in]	 size_bytes  			读取字节数     
\param[in]	 first_read_dummy  		是否跳过回读的第一个字节  
===========================================================================*/
void LCD_readDat(unsigned char *buffer,unsigned int size_bytes, unsigned char first_read_dummy);


/*===========================================================================
\brief       往显存写入一个像素点

\param[in]	 x  			横坐标        
\param[in]	 y  			纵坐标   
\param[in]	 color  		颜色值
===========================================================================*/
void LCD_write_pix(unsigned int x,unsigned int y,unsigned char color);


/*===========================================================================
\brief       清空显存
===========================================================================*/
void LCD_clear_buffer(void);

/*===========================================================================
\brief       将显存的数据刷新到LCD，由CPU控制刷入
===========================================================================*/
void LCD_flush_buffer(void);

/*===========================================================================
\brief       设置LCD对比度

\param[in]	 contrast  	对比度 范围 0-0x7F
===========================================================================*/
void LCD_set_contrast(unsigned char contrast);

/*===========================================================================
\brief       设置写入LCD内部存储器的开始坐标

\param[in]	 x  	横坐标        
\param[in]	 y  	纵坐标 
===========================================================================*/
void LCD_setxy(unsigned int x, unsigned int y);

/*===========================================================================
\brief       向LCD发送命令，一次发送最大大小为4字节

\param[in]	 cmd  		命令值    
\param[in]	 cmd_size 	命令长度（字节，范围0~4）
===========================================================================*/
void LCD_write_cmd(unsigned int cmd, unsigned int cmd_size);	

/*===========================================================================
\brief       向LCD发送数据，一次发送最大大小为4字节

\param[in]	 cmd  		数据    
\param[in]	 dat_size 	数据长度（字节，范围0~4）
===========================================================================*/
void LCD_write_dat(unsigned int dat, unsigned int dat_size);	

/*===========================================================================
\brief       LCDIF初始化以及屏幕初始化
===========================================================================*/
void LCD_init(void);

/*===========================================================================
\brief       重置LCD的DMA控制器
===========================================================================*/
void LCD_dma_channel_reset(void);


/*===========================================================================
\brief       通知DMA将显存刷入屏幕
===========================================================================*/
void LCD_dma_flush_buffer(void);

/*===========================================================================
\brief       设置LCD的灰度模式

\param[in]	 pix_format  PIX_FORMAT_GRAYn
===========================================================================*/
void LCD_set_pix_format(lcd_pix_format pix_format);

/*===========================================================================
\brief		显示字符串

\param[in]		x,y:起点坐标
\param[in]		width,height:区域大小  
\param[in]		size:字体大小
\param[in]		color:字体颜色
\param[in]		*p:字符串起始地址		
===========================================================================*/  
void LCD_show_string(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t color,uint8_t *p);


/*===========================================================================
\brief		在指定位置显示一个字符

\param[in]		x,y:起始坐标
\param[in]		num:要显示的字符:" "--->"~"
\param[in]		size:字体大小 12/16/24
\param[in]		size:字体大小
===========================================================================*/  
void LCD_show_char(uint16_t x, uint16_t y, uint8_t num, uint8_t size, uint8_t color, uint8_t mode);

/*===========================================================================
\brief       往LCD上输出文字信息
===========================================================================*/
void LCD_print(uint8_t *p);


#ifdef __cplusplus 
} 
#endif
#endif

