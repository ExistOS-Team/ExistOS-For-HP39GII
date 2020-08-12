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


void LCD_write_pix(unsigned int x,unsigned int y,unsigned char color);	//往显存写入一个像素点，颜色范围0~3
void LCD_clear_buffer(void);	//清空显存
void LCD_flush_buffer(void);	//将显存的数据刷新到LCD

void LCD_setxy(unsigned int x, unsigned int y); //设置写入LCD内部存储器的行列地址
void LCD_writeCmd(unsigned int cmd, unsigned int cmd_size);	//向LCD发送命令
void LCD_writeDat(unsigned int dat, unsigned int dat_size);	//向LCD发送数据
void LCD_init(void); //LCDIF初始化以及屏幕初始化

#endif

