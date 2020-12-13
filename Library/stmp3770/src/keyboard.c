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
#include "keyboard.h"

//SSP1_SCK		BANK1_PIN23
//SSP1_DATA1	BANK1_PIN25
//SSP1_DATA3	BANK1_PIN27
//SSP1_DATA2	BANK1_PIN26
//SSP1_CMD		BANK1_PIN22

unsigned char key_matrix[5][11] = {0};

void set_row_line(int row_line)
{
	unsigned int tmp_DOUT,tmp_DOE;

	tmp_DOUT = BF_RD(PINCTRL_DOUT2,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE2,DOE);
	tmp_DOUT |= ((1<<14)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2));
	tmp_DOE |= ((1<<14)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2));
	BF_CS1(PINCTRL_DOUT2,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE2,DOE,tmp_DOE);
	BF_CS1(PINCTRL_DOUT2,DOUT,tmp_DOUT);

	tmp_DOUT = BF_RD(PINCTRL_DOUT1,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE1,DOE);
	tmp_DOUT |= ((1<<24));
	tmp_DOE |= ((1<<24));
	BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE1,DOE,tmp_DOE);
	BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);

	tmp_DOUT = BF_RD(PINCTRL_DOUT0,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE0,DOE);
	tmp_DOUT |= ((1<<20));
	tmp_DOE |= ((1<<20));
	BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE0,DOE,tmp_DOE);
	BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);

	switch(row_line)
		{
		case 5:
			tmp_DOUT = BF_RD(PINCTRL_DOUT1,DOUT);
			tmp_DOE = BF_RD(PINCTRL_DOE1,DOE);
			tmp_DOUT &= ~((1<<24));
			tmp_DOE |= ((1<<24));
			BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);
			BF_CS1(PINCTRL_DOE1,DOE,tmp_DOE);
			BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);
			break;
		case 8:
			tmp_DOUT = BF_RD(PINCTRL_DOUT0,DOUT);
			tmp_DOE = BF_RD(PINCTRL_DOE0,DOE);
			tmp_DOUT &= ~((1<<20));
			tmp_DOE |= ((1<<20));
			BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);
			BF_CS1(PINCTRL_DOE0,DOE,tmp_DOE);
			BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);
			break;
		default:
			tmp_DOUT = BF_RD(PINCTRL_DOUT2,DOUT);
			tmp_DOE = BF_RD(PINCTRL_DOE2,DOE);
			switch(row_line)
				{
				case 0:
					tmp_DOUT &= ~((1<<6));
					break;
				case 1:
					tmp_DOUT &= ~((1<<5));
					break;
				case 2:
					tmp_DOUT &= ~((1<<4));
					break;
				case 3:
					tmp_DOUT &= ~((1<<2));
					break;
				case 4:
					tmp_DOUT &= ~((1<<3));
					break;
				case 6:
					tmp_DOUT &= ~((1<<8));
					break;
				case 7:
					tmp_DOUT &= ~((1<<7));
					break;
				case 9:
					tmp_DOUT &= ~((1<<14));
					break;
				}
			tmp_DOE |= ((1<<14)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2));
			BF_CS1(PINCTRL_DOUT2,DOUT,tmp_DOUT);
			BF_CS1(PINCTRL_DOE2,DOE,tmp_DOE);
			BF_CS1(PINCTRL_DOUT2,DOUT,tmp_DOUT);
			break;
		}
}

unsigned int read_col_line(int col_line)
{
	switch(col_line)
		{
		case 0:
			return (BF_RD(PINCTRL_DIN1,DIN)>>23)&1;
		case 1:
			return (BF_RD(PINCTRL_DIN1,DIN)>>25)&1;
		case 2:
			return (BF_RD(PINCTRL_DIN1,DIN)>>27)&1;
		case 3:
			return (BF_RD(PINCTRL_DIN1,DIN)>>26)&1;
		case 4:
			return (BF_RD(PINCTRL_DIN1,DIN)>>22)&1;
		default:
			break;
		}
	return 0;
}


void keyboard_init()
{
	unsigned int tmp_DOUT,tmp_DOE;
	BF_CS6 (
	    PINCTRL_MUXSEL3,
	    BANK1_PIN22,3,
	    BANK1_PIN23,3,
	    BANK1_PIN24,3,
	    BANK1_PIN25,3,
	    BANK1_PIN26,3,
	    BANK1_PIN27,3
	);
	BF_CS6 (
	    PINCTRL_MUXSEL4,
	    BANK2_PIN02,3,
	    BANK2_PIN03,3,
	    BANK2_PIN04,3,
	    BANK2_PIN05,3,
	    BANK2_PIN06,3,
	    BANK2_PIN07,3
	);
	BF_CS1 (
	    PINCTRL_MUXSEL4,
	    BANK2_PIN08,3
	);
	BF_CS1 (
	    PINCTRL_MUXSEL5,
	    BANK2_PIN14,3
	);

	BF_CS1 (
	    PINCTRL_MUXSEL1,
	    BANK0_PIN14,3
	);

	BF_CS1 (
	    PINCTRL_MUXSEL1,
	    BANK0_PIN20,3
	);

	//设置引脚复用寄存器，作为默认IO口使用

	//列线设置
	tmp_DOUT = BF_RD(PINCTRL_DOUT1,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE1,DOE);
	tmp_DOUT |= ((1<<22)|(1<<23)|(1<<25)|(1<<26)|(1<<27));
	tmp_DOE &= ~((1<<22)|(1<<23)|(1<<25)|(1<<26)|(1<<27));
	BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE1,DOE,tmp_DOE);
	//BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);


	//行线设置
	tmp_DOUT = BF_RD(PINCTRL_DOUT2,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE2,DOE);
	tmp_DOUT &= ~((1<<14)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2));
	tmp_DOE |= ((1<<14)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2));
	BF_CS1(PINCTRL_DOUT2,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE2,DOE,tmp_DOE);
	BF_CS1(PINCTRL_DOUT2,DOUT,tmp_DOUT);

	tmp_DOUT = BF_RD(PINCTRL_DOUT1,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE1,DOE);
	tmp_DOUT &= ~((1<<24));
	tmp_DOE |= ((1<<24));
	BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE1,DOE,tmp_DOE);
	BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);

	tmp_DOUT = BF_RD(PINCTRL_DOUT0,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE0,DOE);
	tmp_DOUT &= ~((1<<20));
	tmp_DOE |= ((1<<20));
	BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE0,DOE,tmp_DOE);
	BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);

	//ON键
	tmp_DOUT = BF_RD(PINCTRL_DOUT0,DOUT);
	tmp_DOE = BF_RD(PINCTRL_DOE0,DOE);
	tmp_DOUT &= ~((1<<14));
	tmp_DOE &= ~((1<<14));
	BF_CS1(PINCTRL_DOUT0,DOUT,tmp_DOUT);
	BF_CS1(PINCTRL_DOE0,DOE,tmp_DOE);

	set_row_line(0);

}

unsigned int is_key_ON_down()
{
	return ((BF_RD(PINCTRL_DIN0,DIN)>>14)&1);
}

void key_scan()
{

	for(int y=0; y<10; y++)
	{
		set_row_line(y);
		for(int x=0; x<5; x++)
		{
			key_matrix[x][y] = !read_col_line(x);
		}
	}
	
	key_matrix[0][10] = ((BF_RD(PINCTRL_DIN0,DIN)>>14)&1);	
};

unsigned int is_key_down(keys key) { return key_matrix[key % 8][key >> 3]; };

