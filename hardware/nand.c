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

#include "regsgpmi.h"
#include "regspinctrl.h"
#include "nand.h"

static struct gpmi_timing safe_timing = {
	.DataSetup		= 80,
	.DataHold		= 60,
	.AddressSetup		= 25,
	.HalfPeriods		= 0,
	.SampleDelay		= 6,
	.NandTimingState	= 0,
	.tREA		= 0xFF,
	.tRLOH		= 0xFF,
	.tRHOH		= 0xFF,
};

void NAND_init(){
	BF_CS8 (
		PINCTRL_MUXSEL0, 
	    BANK0_PIN07, 0, //D7
	    BANK0_PIN06, 0, //D6
	    BANK0_PIN05, 0, //D5
	    BANK0_PIN04, 0, //D4
	    BANK0_PIN03, 0, //D3
	    BANK0_PIN02, 0, //D2
	    BANK0_PIN01, 0, //D1
	    BANK0_PIN00, 0	//D0
	);
	/*BF_CS1 (
		PINCTRL_MUXSEL0,
	    BANK0_PIN13, 0, //CE0N
	);*/
	BF_CS1 (
		PINCTRL_MUXSEL4,
	    BANK2_PIN15, 1 	//GPMI_CE0
	);		
	BF_CS5 (
		PINCTRL_MUXSEL1,
	    BANK0_PIN25, 0, //RDn
	    BANK0_PIN24, 0, //WRn
	    BANK0_PIN23, 3, //IRQ
	    BANK0_PIN22, 0, //RSTn
	    BANK0_PIN19, 0 //RB0
	);
	//设置引脚复用寄存器，连接至SoC内部的NAND控制器
						
	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
	while(BF_RD(GPMI_CTRL0,SFTRST));
	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);	
	while(BF_RD(GPMI_CTRL0,CLKGATE));
	
	HW_GPMI_CTRL1_CLR(BM_GPMI_CTRL1_GPMI_MODE);				//设置GPMI控制器为NAND模式
	HW_GPMI_CTRL1_SET(BM_GPMI_CTRL1_ATA_IRQRDY_POLARITY);	//设置中断触发条件（NAND就绪时触发）
	HW_GPMI_CTRL1_SET(BM_GPMI_CTRL1_DEV_RESET);
	
	
	
	
	
}