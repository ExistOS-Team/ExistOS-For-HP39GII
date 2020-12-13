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


#include "regspinctrl.h"
#include "regsclkctrl.h"
#include "regsapbh.h"
#include "nand.h"
#include "irq.h"
#include "hw_irq.h"
#include "utils.h"
#include "uart_debug.h"
#include "nandchips.h"

#include <stdio.h>

//0x9200 - 0xB200  BUFFER

unsigned int NAND_DMA_Channel = 4;
unsigned char chipSelect = 0;

unsigned int GPMI_clockFrequencyInHz = 0;    //GPMI控制器频率
unsigned int GPMI_clockPeriodIn_ns = 0;        //GPMI控制器周期

unsigned int DeviceTimeOut_s = 0;
unsigned short DeviceTimeOutCycles = 0;
volatile unsigned int dmaOperationCompleted = 0;
volatile unsigned int eccOperationCompleted = 0;

unsigned int mapping_sectorsPerblock = 64;
unsigned int mapping_firstBlockLBA = 64;

unsigned char address_page_data[5];

extern unsigned int __DMA_NAND_PALLOAD_BUFFER;
extern unsigned int __DMA_NAND_AUXILIARY_BUFFER;


//接口时序，单位ns
static struct NAND_Timing_t safe_timing =
{
	.DataSetup            = 80,
	.DataHold            = 60,
	.AddressSetup        = 25,
	.SampleDelay        = 1,
};


typedef struct
{
// DMA related fields
	unsigned int dma_nxtcmdar;
	unsigned int dma_cmd;
	unsigned int dma_bar;
// GPMI related fields
	unsigned int gpmi_ctrl0;        //PIO 0
	unsigned int gpmi_compare;        //PIO 1
	unsigned int gpmi_eccctrl;        //PIO 2
	unsigned int gpmi_ecccount;        //PIO 3
	unsigned int gpmi_data_ptr;        //PIO 4
	unsigned int gpmi_aux_ptr;        //PIO 5
} GPMI_DMA_GENERIC_DESCRIPTOR;



static struct NAND_Timing_t hw_timing;

hw_gpmi_DmaDesc gpmi_send_cmd_dma_desc;
hw_gpmi_DmaDesc gpmi_send_dat_dma_desc;
hw_gpmi_DmaDesc gpmi_read_dat_dma_desc;

unsigned char commandAddressBuffer[10];
unsigned char id[10];

struct nand_flash_dev foundChip;

volatile GPMI_DMA_GENERIC_DESCRIPTOR chains[10];





void GPMI_send_cmd(unsigned int command, unsigned int address, unsigned int address_size_bytes, unsigned int readback_size_bytes, unsigned char *readback_buffer_address)
{
	dmaOperationCompleted = 0;

	commandAddressBuffer[0]=command;
	commandAddressBuffer[1]=address;

	/***********************************
	 *     DMA描述符链 0    等待FLASH就绪，超时产生中断
	 ***********************************/

	chains[0].dma_cmd = BF_APBH_CHn_CMD_CMDWORDS(1)                        |    // 发送1个PIO命令到GPMI控制器
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD(1)                  |    // 完成当前命令之后再继续执行
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(1)               |    // 等待NAND就绪后开始执行
	                    BF_APBH_CHn_CMD_NANDLOCK(0)                     |    // 锁住NAND防止被其它DMA通道占用
	                    BF_APBH_CHn_CMD_CHAIN(1)                        |    // 还有剩下的描述符链
	                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);            // 仅读写GPMI控制器，不发送任何数据

	chains[0].dma_bar = (unsigned int)NULL;

	chains[0].gpmi_ctrl0 =  BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)                    |     // 当前模式：等待NAND就绪
	                        BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT)         |    // 8bit总线模式
	                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)                              |    // 数据模式
	                        BF_GPMI_CTRL0_CS(chipSelect);                                            // 片选

	chains[0].dma_nxtcmdar = (reg32_t)&chains[1];

	/***********************************
	 *     DMA描述符链 1    发送命令和地址
	 ***********************************/
	chains[1].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (1 + address_size_bytes) | // 1字节命令 和剩下的地址数据
	                    BF_APBH_CHn_CMD_CMDWORDS (3)                        | // 发送3个PIO命令到GPMI控制器
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                        | // 等待NAND就绪后开始执行
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                         |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)                     |
	                    BF_APBH_CHn_CMD_NANDLOCK (1)                         |
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                         |
	                    BF_APBH_CHn_CMD_CHAIN (1)                             |
	                    BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);              // 从内存读取，发送到NAND

	chains[1].dma_bar = (unsigned int)&commandAddressBuffer;

	chains[1].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)             | // 写入NAND
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)             |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)             |
	                       BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(0)                    |
	                       BF_GPMI_CTRL0_CS (chipSelect)                    |
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)             |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (1)                 | // 发送一字节命令后立即切换地址模式
	                       BF_GPMI_CTRL0_XFER_COUNT (1 + address_size_bytes);

	chains[1].gpmi_compare = (unsigned int)NULL;
	chains[1].gpmi_eccctrl = (unsigned int)NULL;

	if(address_size_bytes)
		{
			chains[1].dma_nxtcmdar = (reg32_t)&chains[2];
		}
	else
		{
			chains[1].dma_nxtcmdar = (reg32_t)&chains[3];
		}

	/***********************************
	 *     DMA描述符链 2    回读数据
	 ***********************************/


	chains[2].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (readback_size_bytes)    |
	                    BF_APBH_CHn_CMD_CMDWORDS (3)                        |
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (0)                        |
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                         |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)                     |
	                    BF_APBH_CHn_CMD_NANDLOCK (0)                         |
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                         |
	                    BF_APBH_CHn_CMD_CHAIN (1)                             |
	                    BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE);

	chains[2].dma_bar = (unsigned int)readback_buffer_address;

	chains[2].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ)             |
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)             |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)             |
	                       BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(0)                    |
	                       BF_GPMI_CTRL0_CS (chipSelect)                    |
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)             |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (0)                 |
	                       BF_GPMI_CTRL0_XFER_COUNT (readback_size_bytes);

	chains[2].gpmi_compare = (unsigned int)NULL;
	chains[2].gpmi_eccctrl = (unsigned int)NULL;

	chains[2].dma_nxtcmdar = (reg32_t)&chains[3];

	/***********************************
	 *     DMA描述符链 3    终止
	 ***********************************/


	chains[3].dma_cmd =     BF_APBH_CHn_CMD_IRQONCMPLT(1)                    |
	                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)                    |
	                        BF_APBH_CHn_CMD_SEMAPHORE(1)                      |    // 发送完成当前描述符后DMA计数器减一
	                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

	chains[3].dma_bar = (unsigned int)NULL;
	chains[3].dma_nxtcmdar = (unsigned int)NULL;


	BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains[0]);    // 填写DMA寄存器下个描述符地址
	BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1);                        // DMA计数器加一，开始工作

}



void GPMI_read_block_with_ecc8(unsigned char set_read_command,unsigned char start_read_command,
                               unsigned char *address_data, unsigned int *buffer, unsigned int address_data_size_bytes )
{
	unsigned int i;
	eccOperationCompleted = 0;
	// 设置读取模式命令（1byte） + 地址数据(一般5bytes) + 开始读取命令(1byte)
	commandAddressBuffer[0] = set_read_command;
	for(i=1; i <= address_data_size_bytes; i++ )
		{
			commandAddressBuffer[i] = address_data[i - 1];
		}
	commandAddressBuffer[i] = start_read_command;

	//for(unsigned int k = 0; k<10; k++)printf("%x ",commandAddressBuffer[k]);
	//printf("\n");

	//Reference STMP3770 datasheet P.439

	/***********************************
	 *     Descriptor 0: issue NAND read setup command (CLE/ALE)
	 ***********************************/

	chains[0].dma_nxtcmdar = (unsigned int)&chains[1];
	chains[0].dma_cmd =         BF_APBH_CHn_CMD_XFER_COUNT (1 + address_data_size_bytes)    | // point to the next descriptor
	                            BF_APBH_CHn_CMD_CMDWORDS (3)                                | // send 3 words to the GPMI
	                            BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                                | // wait for command to finish before continuing
	                            BF_APBH_CHn_CMD_SEMAPHORE (0)                                |
	                            BF_APBH_CHn_CMD_NANDWAIT4READY(0)                            |
	                            BF_APBH_CHn_CMD_NANDLOCK (1)                                |
	                            BF_APBH_CHn_CMD_IRQONCMPLT (0)                                |
	                            BF_APBH_CHn_CMD_CHAIN (1)                                    | // follow chain to next command
	                            BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);                       // read data from DMA, write to NAND

	chains[0].dma_bar = (unsigned int)&commandAddressBuffer;

	chains[0].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)                    |
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)                    |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)                        |
	                       BF_GPMI_CTRL0_CS (chipSelect)                             |
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)                     |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (1)                        |
	                       BF_GPMI_CTRL0_XFER_COUNT (1 + address_data_size_bytes);
	chains[0].gpmi_compare = (unsigned int)NULL;
	chains[0].gpmi_eccctrl = BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);                  // disable the ECC

	/***********************************
	 *     Descriptor 1: issue NAND read execute command (CLE)
	 ***********************************/
	chains[1].dma_nxtcmdar = (unsigned int)&chains[2];                                 // point to the next descriptor
	chains[1].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (1)                             |     // 1 byte read command
	                    BF_APBH_CHn_CMD_CMDWORDS (1)                             | // send 1 word to GPMI
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                         | // wait for command to finish before continuing
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                             |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)                        |
	                    BF_APBH_CHn_CMD_NANDLOCK (1)                            | // prevent other DMA channels from taking over
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                            |
	                    BF_APBH_CHn_CMD_CHAIN (1)                                | // follow chain to next command
	                    BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);                   // read data from DMA, write to NAND

	chains[1].dma_bar = (unsigned int)&commandAddressBuffer[i];                   // point to byte 6, read execute command

	chains[1].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)                | // write to the NAND
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)                |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)                |
	                       BF_GPMI_CTRL0_CS (chipSelect)                        | // must correspond to NAND CS used
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)                |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (0)                    |
	                       BF_GPMI_CTRL0_XFER_COUNT (1);                           // 1 byte command
	/***********************************
	 *  Descriptor 3: wait for ready (DATA)
	 ***********************************/
	chains[2].dma_nxtcmdar = (unsigned int)&chains[3];                       // point to the next descriptor
	chains[2].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (0)                     | // no dma transfer
	                    BF_APBH_CHn_CMD_CMDWORDS (1)                     | // send 1 word to GPMI
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                 | // wait for command to finish before continuing
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                     |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(1)                 | // wait for nand to be ready
	                    BF_APBH_CHn_CMD_NANDLOCK (1)                     | // relinquish nand lock
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                     |
	                    BF_APBH_CHn_CMD_CHAIN (1)                         | // follow chain to next command
	                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);       // no dma transfer

	chains[2].dma_bar = (unsigned int)NULL;                               // field not used
	// 1 word sent to the GPMI
	chains[2].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY) | // wait for NAND ready
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)             |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)             |
	                       BF_GPMI_CTRL0_CS (chipSelect)                     | // must correspond to NAND CS used
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)             |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (0)                 |
	                       BF_GPMI_CTRL0_XFER_COUNT (0);

	/***********************************
	 *    Descriptor 4: psense compare (time out check)
	 ***********************************/
	chains[3].dma_nxtcmdar = (unsigned int)&chains[4];                           // point to the next descriptor
	chains[3].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (0)                         | // no dma transfer
	                    BF_APBH_CHn_CMD_CMDWORDS (0)                         | // no words sent to GPMI
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (0)                     | // do not wait to continue
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                         |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)                     |
	                    BF_APBH_CHn_CMD_NANDLOCK (1)                         |
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                         |
	                    BF_APBH_CHn_CMD_CHAIN (1)                             | // follow chain to next command
	                    BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE);             // perform a sense check


	//chains[3].dma_bar = dma_error_handler; // if sense check fails, branch to error handler
	chains[3].dma_bar = (unsigned int)&chains[7]; // if sense check fails, branch to error handler


	/***********************************
	* Descriptor 5: read 4K page plus 65 byte meta-data Nand data
	* and send it to ECC block (DATA)
	***********************************/
	chains[4].dma_nxtcmdar = (unsigned int)&chains[5];                               // point to the next descriptor
	chains[4].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (0)                           | // no dma transfer
	                    BF_APBH_CHn_CMD_CMDWORDS (6)                             | // send 6 words to GPMI
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                          | // wait for command to finish beforecontinuing
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                            |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)                        |
	                    BF_APBH_CHn_CMD_NANDLOCK (1)                             | // prevent other DMA channels from taking over
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                           | // ECC block generates ecc8 interrupt on completion
	                    BF_APBH_CHn_CMD_CHAIN (1)                                | // follow chain to next command
	                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);               // no DMA transfer, ECC block handlestransfer

	chains[4].dma_bar = (unsigned int)NULL; // field not used
	// 6 words sent to the GPMI
	chains[4].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ)                 | // read from the NAND
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)                 |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)                  |
	                       BF_GPMI_CTRL0_CS (chipSelect)                          | // must correspond to NAND CS used
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)                 |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (0)                    |
	                       BF_GPMI_CTRL0_XFER_COUNT (4*(512+9)+(19+9));               // 2K PAGE SIZE four 512 byte data blocks (plusparity, t = 4) \
	and one 19 byte aux block (plusparity, t = 4)

	//BF_GPMI_CTRL0_XFER_COUNT (8*(512+18)+(65+9));           // 4K PAGE SIZE eight 512 byte data blocks (plusparity, t = 8) \
	and one 65 byte aux block (plusparity, t = 4)


	chains[4].gpmi_compare = (unsigned int)NULL;                                   // field not used but necessary to seteccctrl


// GPMI ECCCTRL PIO This launches the 4K byte transfer through ECC8’s
// bus master. Setting the ECC_ENABLE bit redirects the data flow
// within the GPMI so that read data flows to the ECC8 engine instead
// of flowing to the GPMI’s DMA channel.
	chains[4].gpmi_eccctrl =
	    //BV_FLD(GPMI_ECCCTRL, ECC_CMD, DECODE_8_BIT)             | // specify t = 8 mode
	    BV_FLD(GPMI_ECCCTRL, ECC_CMD, DECODE_4_BIT)             | // specify t = 4 mode
	    BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, ENABLE)                | // enable ECC module
	    BF_GPMI_ECCCTRL_BUFFER_MASK (0X10F);                      // read all 4 data blocks and 1 aux block

	//BF_GPMI_ECCCTRL_BUFFER_MASK (0X1FF);                    // read all 8 data blocks and 1 aux block

	//chains[4].gpmi_ecccount = BF_GPMI_ECCCOUNT_COUNT(8*(512+18)+(65+9));            // 4K PAGE SIZE specify number of bytes read fromNAND
	chains[4].gpmi_ecccount = BF_GPMI_ECCCOUNT_COUNT(4*(512+9)+(19+9));               // 2K PAGE SIZE specify number of bytes read fromNAND
	chains[4].gpmi_data_ptr = (unsigned int)buffer;               // pointer for the 4K byte data area
	chains[4].gpmi_aux_ptr = (unsigned int)&__DMA_NAND_AUXILIARY_BUFFER;              // pointer for the 65 byte aux area + parity and syndrome bytes \
	for both data and aux blocks.

	/***********************************
	* Descriptor 6: disable ECC block
	***********************************/
	chains[5].dma_nxtcmdar = (unsigned int)&chains[6];                           // point to the next descriptor
	chains[5].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (0)                      | // no dma transfer
	                    BF_APBH_CHn_CMD_CMDWORDS (3)                          | // send 3 words to GPMI
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                       | // wait for command to finish before continuing
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                         |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(1)                     | // wait for nand to be ready
	                    BF_APBH_CHn_CMD_NANDLOCK (1)                          | // need nand lock to be thread safe while turnoff ECC8
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                        |
	                    BF_APBH_CHn_CMD_CHAIN (1)                             | // follow chain to next command
	                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);               // no dma transfer
	chains[5].dma_bar = (unsigned int)NULL;                                   // field not used
	// 3 words sent to the GPMI
	chains[5].gpmi_ctrl0 = BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)           |
	                       BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)             |
	                       BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)              |
	                       BF_GPMI_CTRL0_CS (chipSelect)                      | // must correspond to NAND CS used
	                       BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)             |
	                       BF_GPMI_CTRL0_ADDRESS_INCREMENT (0)                |
	                       BF_GPMI_CTRL0_XFER_COUNT (0);
	chains[5].gpmi_compare = (unsigned int)NULL;                               // field not used but necessary to set eccctrl
	chains[5].gpmi_eccctrl = BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);          // disable the ECC block
	/***********************************
	* Descriptor 7: deassert nand lock
	***********************************/
	chains[6].dma_nxtcmdar = (unsigned int)&chains[7];                                  // not used since this is last descriptor
	chains[6].dma_cmd = BF_APBH_CHn_CMD_XFER_COUNT (0)                      | // no dma transfer
	                    BF_APBH_CHn_CMD_CMDWORDS (0)                          | // no words sent to GPMI
	                    BF_APBH_CHn_CMD_WAIT4ENDCMD (1)                       | // wait for command to finish before continuing
	                    BF_APBH_CHn_CMD_SEMAPHORE (0)                         |
	                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)                     |
	                    BF_APBH_CHn_CMD_NANDLOCK (0)                          | // relinquish nand lock
	                    BF_APBH_CHn_CMD_IRQONCMPLT (0)                        | // ECC8 engine generates interrupt
	                    BF_APBH_CHn_CMD_CHAIN (1)                             | // terminate DMA chain processing
	                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);               // no dma transfer
	chains[6].dma_bar = (unsigned int)NULL;                                      // field not used



	/***********************************
	     *     DMA描述符链 3    终止
	     ***********************************/


	chains[7].dma_cmd =     BF_APBH_CHn_CMD_IRQONCMPLT(1)                    |
	                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)                   |
	                        BF_APBH_CHn_CMD_SEMAPHORE(1)                     |    // 发送完成当前描述符后DMA计数器减一
	                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

	chains[7].dma_bar = (unsigned int)NULL;
	chains[7].dma_nxtcmdar = (unsigned int)NULL;


	BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains[0]);    // 填写DMA寄存器下个描述符地址
	BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1);                        // DMA计数器加一，开始工作

}


unsigned int dma_cmplt;
unsigned int dma_error;

void GPMI_dma_irq_handle()
{
	dma_cmplt = BF_RD(APBH_CTRL1, CH4_CMDCMPLT_IRQ);
	dma_error = BF_RD(APBH_CTRL1, CH4_AHB_ERROR_IRQ);
	//printf("GPMI DMA IRQ CMPLT:%1d ERROR:%1d\n",dma_cmplt,dma_error);

	if(dma_cmplt)
		{
			if(!dmaOperationCompleted)
				{
					dmaOperationCompleted = 1;
				}
		}


	HW_APBH_CTRL1_SET(BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ);
	HW_APBH_CTRL1_CLR(BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ);
	HW_APBH_CTRL1_TOG(BM_APBH_CTRL1_CH4_CMDCMPLT_IRQ);

	HW_APBH_CTRL1_SET(BM_APBH_CTRL1_CH4_AHB_ERROR_IRQ);
	HW_APBH_CTRL1_CLR(BM_APBH_CTRL1_CH4_AHB_ERROR_IRQ);
	HW_APBH_CTRL1_TOG(BM_APBH_CTRL1_CH4_AHB_ERROR_IRQ);



	irq_set_enable(VECTOR_IRQ_GPMI_DMA, 1);
}

void GPMI_irq_handle()
{
	//printf("GPMI IRQ DevIRQ:%1d Time out:%1d\n",BF_RD(GPMI_CTRL1,DEV_IRQ),BF_RD(GPMI_CTRL1,TIMEOUT_IRQ));

	BF_CS1(GPMI_CTRL1,DEV_IRQ,0);
	BF_CS1(GPMI_CTRL1,TIMEOUT_IRQ,0);

	irq_set_enable(VECTOR_IRQ_GPMI, 1);

}

void ecc8_completion_irq_handle()
{

	eccOperationCompleted = 1;

	/*
		printf("ECC8:finish. Error Level:%1x %1x %1x %1x\n",
		       BF_RD(ECC8_STATUS1,STATUS_PAYLOAD0),
		       BF_RD(ECC8_STATUS1,STATUS_PAYLOAD1),
		       BF_RD(ECC8_STATUS1,STATUS_PAYLOAD2),
		       BF_RD(ECC8_STATUS1,STATUS_PAYLOAD3)
		      );*/

	//HW_ECC8_STATUS0_RD();
	//HW_ECC8_STATUS1_RD();
	if((BF_RD(ECC8_STATUS1,STATUS_PAYLOAD0) == 0xE)||
	   (BF_RD(ECC8_STATUS1,STATUS_PAYLOAD1) == 0xE)||
	   (BF_RD(ECC8_STATUS1,STATUS_PAYLOAD2) == 0xE)||
	   (BF_RD(ECC8_STATUS1,STATUS_PAYLOAD3) == 0xE)
		){
		printf("uncorrectable ECC error while reading.\n");
	}

	HW_ECC8_CTRL_SET(BM_ECC8_CTRL_COMPLETE_IRQ);
	HW_ECC8_CTRL_CLR(BM_ECC8_CTRL_COMPLETE_IRQ);
	HW_ECC8_CTRL_TOG(BM_ECC8_CTRL_COMPLETE_IRQ);



	//BF_CLR (ECC8_CTRL,BM_ERROR_IRQ);

	//printf("ECC8_CTRL:%x \n",(*(unsigned int *)0x80008000));

	//printf("ECCCOUNT:%x \n",(*(unsigned int *)0x8000C030));

	irq_set_enable(VECTOR_IRQ_ECC8, 1);                                            //ECC8


}

void gpmi_dma_init()
{

	BF_CS1(APBH_CTRL0, FREEZE_CHANNEL, BF_RD(APBH_CTRL0, FREEZE_CHANNEL)&(1<<NAND_DMA_Channel));
	BF_CS1(APBH_CTRL0, CLKGATE_CHANNEL, BF_RD(APBH_CTRL0, CLKGATE_CHANNEL)&(1<<NAND_DMA_Channel));
	BF_CS1(APBH_CTRL0, RESET_CHANNEL, (BF_RD(APBH_CTRL0, RESET_CHANNEL)|(1<<NAND_DMA_Channel)));
	while((BF_RD(APBH_CTRL0, RESET_CHANNEL)&(1<<NAND_DMA_Channel)))
		{
			;        //等待DMA通道重置完成
		}



	irq_install_service(VECTOR_IRQ_GPMI_DMA, (void *)GPMI_dma_irq_handle);        //注册DMA操作完成中断的处理函数
	irq_install_service(VECTOR_IRQ_GPMI, (void *)GPMI_irq_handle);                //注册DMA操作完成中断的处理函数
	irq_set_enable(VECTOR_IRQ_GPMI_DMA, 1);                                        //打开中断控制器中关于GPMI的中断请求
	irq_set_enable(VECTOR_IRQ_GPMI, 1);                                            //打开中断控制器中关于GPMI的中断请求

	BF_CS1 (APBH_CTRL1, CH4_CMDCMPLT_IRQ_EN, 1);                                    //打开GPMI DMA控制器完成中断请求

}


void ecc8_init()
{



	HW_ECC8_CTRL_CLR(BM_ECC8_CTRL_SFTRST);
	while(BF_RD(ECC8_CTRL,SFTRST));
	HW_ECC8_CTRL_CLR(BM_ECC8_CTRL_CLKGATE);
	while(BF_RD(ECC8_CTRL,CLKGATE));

	HW_ECC8_CTRL_SET(BM_ECC8_CTRL_SFTRST);
	while(!BF_RD(ECC8_CTRL,CLKGATE))
		{
			;    //等待重置完成
		}
	HW_ECC8_CTRL_CLR(BM_ECC8_CTRL_SFTRST);
	while(BF_RD(ECC8_CTRL,SFTRST));
	HW_ECC8_CTRL_CLR(BM_ECC8_CTRL_CLKGATE);
	while(BF_RD(ECC8_CTRL,CLKGATE));
	HW_ECC8_CTRL_CLR(BM_ECC8_CTRL_AHBM_SFTRST);


	BF_CS1 (ECC8_CTRL, COMPLETE_IRQ_EN, 1);
	//BF_CS1 (ECC8_CTRL, DEBUG_STALL_IRQ_EN, 1);
	//BF_CS1 (ECC8_CTRL, DEBUG_WRITE_IRQ_EN, 1);

	irq_install_service(VECTOR_IRQ_ECC8, (void *)ecc8_completion_irq_handle);    //ECC8

	irq_set_enable(VECTOR_IRQ_ECC8, 1);                                            //ECC8


}

void set_page_address_data(unsigned int pageNumber)
{
	address_page_data[0] = 0;
	address_page_data[1] = 0;    //col addr
	address_page_data[2] = pageNumber & 0xFF;    //page addr
	address_page_data[3] = (pageNumber >> 8)&0xFF;
	address_page_data[4] = (pageNumber >> 16)&0xFF;
}



unsigned int gpmi_is_busy()
{
	return(!dmaOperationCompleted || !eccOperationCompleted);
}

/*
 *	return:
 *	0 done
 *  1 device busy
 *  2 read timeout
 */
unsigned int read_nand_pages(unsigned int start_page, unsigned int pages, unsigned int *buffer, unsigned int timeout_ms)
{
	unsigned int cnt1, cnt2, pgcnt = 0;
	if(timeout_ms)
		{
			cnt1 = timeout_ms * 100;
		}
	else
		{
			cnt1 = 10000 * 100;
		}

	cnt2 = cnt1;

	if(!dmaOperationCompleted || !eccOperationCompleted)
		{
			return deviceBusy;
		}

	while(pgcnt < pages)
		{
			set_page_address_data(start_page);
			GPMI_read_block_with_ecc8(NAND_CMD_READ0,NAND_CMD_READSTART,address_page_data,buffer,4);

			while((!dmaOperationCompleted || !eccOperationCompleted))
				{
					delay_us(10);
					cnt2--;
					if(cnt2 == 0)
						{
							return deviceTimeout;
						}
				}

			cnt2 = cnt1;
			pgcnt++;
			start_page++;
			buffer += foundChip.pagesize;

		}


}

/**
 * ns_to_cycles - 转换纳秒时间到对应循环周期
 *
 * @nstime:   时间，单位纳秒
 * @period:   参考周期
 * @min:      允许最小周期
 */
unsigned int ns_to_cycles(unsigned int nstime, unsigned int period, unsigned int min)
{
	unsigned int k;

	k = (nstime + period - 1) / period;

	return (k > min) ? k : min;
}


void NAND_init()
{
	BF_CS8 (
	    PINCTRL_MUXSEL0,
	    BANK0_PIN07, 0, //D7
	    BANK0_PIN06, 0, //D6
	    BANK0_PIN05, 0, //D5
	    BANK0_PIN04, 0, //D4
	    BANK0_PIN03, 0, //D3
	    BANK0_PIN02, 0, //D2
	    BANK0_PIN01, 0, //D1
	    BANK0_PIN00, 0    //D0
	);

	BF_CS1 (
	    PINCTRL_MUXSEL4,
	    BANK2_PIN15, 1     //GPMI_CE0N
	);


	BF_CS7 (
	    PINCTRL_MUXSEL1,
	    BANK0_PIN25, 0, //RDn
	    BANK0_PIN24, 0, //WRn
	    BANK0_PIN23, 3, //IRQ
	    BANK0_PIN22, 0, //RSTn
	    BANK0_PIN19, 0, //RB0

	    BANK0_PIN17, 0, //A1
	    BANK0_PIN16, 0  //A0
	);


	*((unsigned int *)0x80018200 + 0x08) |= 0x33333333;
	*((unsigned int *)0x80018200 + 0x04) |= 0x22222222;     //BANK0 0-7     8mA

	*((unsigned int *)0x80018220 + 0x08) |= 0x33003033;
	*((unsigned int *)0x80018220 + 0x04) |= 0x22002022;     //BANK0 16 17 19 22 23     8mA

	*((unsigned int *)0x80018290 + 0x08) |= 0x30000000;
	*((unsigned int *)0x80018290 + 0x04) |= 0x20000000;     //BANK2 15         8mA

	//设置引脚复用寄存器，连接至SoC内部的NAND控制器



	BF_CS1(CLKCTRL_GPMI, DIV, 1);                //设置主时钟分频
	BF_CLR(CLKCTRL_GPMI,CLKGATE);
	//HW_CLKCTRL_GPMI(BM_CLKCTRL_GPMI_CLKGATE);
	while(BF_RD(CLKCTRL_GPMI,CLKGATE));
	GPMI_clockFrequencyInHz  = 24 * 1000000;    //24MHz    todo:从时钟控制器里取出分频系数并计算
	GPMI_clockPeriodIn_ns    = (1000000000 / GPMI_clockFrequencyInHz) ;
	DeviceTimeOut_s = 1;
	DeviceTimeOutCycles = GPMI_clockFrequencyInHz / 4096;

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
	while(BF_RD(GPMI_CTRL0,SFTRST));
	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);
	while(BF_RD(GPMI_CTRL0,CLKGATE));

	HW_GPMI_CTRL0_SET(BM_GPMI_CTRL0_SFTRST);
	while(!BF_RD(GPMI_CTRL0,CLKGATE))
		{
			;    //等待重置完成
		}

	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_SFTRST);
	while(BF_RD(GPMI_CTRL0,SFTRST));
	HW_GPMI_CTRL0_CLR(BM_GPMI_CTRL0_CLKGATE);
	while(BF_RD(GPMI_CTRL0,CLKGATE));

	ecc8_init();
	eccOperationCompleted = 1;

	HW_GPMI_CTRL1_WR(
	    BF_GPMI_CTRL1_DEV_RESET(BV_GPMI_CTRL1_DEV_RESET__DISABLED) |
	    BF_GPMI_CTRL1_ATA_IRQRDY_POLARITY(BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVEHIGH) |
	    BW_GPMI_CTRL1_GPMI_MODE(BV_GPMI_CTRL1_GPMI_MODE__NAND));

	hw_timing.DataSetup = ns_to_cycles(safe_timing.DataSetup,GPMI_clockPeriodIn_ns,1);
	hw_timing.DataHold = ns_to_cycles(safe_timing.DataHold,GPMI_clockPeriodIn_ns,1);
	hw_timing.AddressSetup = ns_to_cycles(safe_timing.AddressSetup,GPMI_clockPeriodIn_ns,0);

	//printf("NAND DataSetup:%x, DataHold:%x, AddressSetup:%x ",hw_timing.DataSetup,hw_timing.DataHold,hw_timing.DataSetup);

	BF_CS3(
	    GPMI_TIMING0,
	    DATA_SETUP, hw_timing.DataSetup,
	    DATA_HOLD, hw_timing.DataHold,
	    ADDRESS_SETUP, hw_timing.DataSetup
	);

	BF_CS1 (
	    GPMI_TIMING1,
	    DEVICE_BUSY_TIMEOUT, DeviceTimeOutCycles
	);
	BF_CS2 (
	    GPMI_CTRL1,
	    DSAMPLE_TIME, safe_timing.SampleDelay,
	    BURST_EN, 0
	);

	gpmi_dma_init();

	for(int i = 0; i<6; i++)
		{
			id[i] = 0x00;
		}

	GPMI_send_cmd(NAND_CMD_RESET,0,0,0,NULL);
	while(!dmaOperationCompleted);
	GPMI_send_cmd(NAND_CMD_READID,0,1,6,id);
	while(!dmaOperationCompleted);




	foundChip.mfr_id = id[0];
	foundChip.dev_id = id[1];

	foundChip.pagesize = 2048;		//todo 解析pagesize



	unsigned int i = 0;
	while(nand_manuf_ids[i].name != NULL)
		{
			if(foundChip.mfr_id == nand_manuf_ids[i].id)
				{
					printf("found FLASH:%s",nand_manuf_ids[i].name);
				}
			i++;
		}
	i=0;

	while(nand_flash_ids[i].name != NULL)
		{
			if(foundChip.dev_id == nand_flash_ids[i].dev_id)
				{
					printf(" %s\n",nand_flash_ids[i].name);
				}
			i++;
		}

	//printf("GPMI STAT:%04x \n",*((unsigned int *)0x8000C0B0));
	//printf("GPMI DEBUG:%04x \n",*((unsigned int *)0x8000C0C0));
	//printf("HW_APBH_CH4_SEMA:%04x \n",*((unsigned int *)0x80004240));
	//printf("DMACTRL1:%04x \n",*((unsigned int *)0x8000C060));
	//printf("PIO 0:%04x \n",*((unsigned int *)0x8000C000));



	//read_nand_pages(0,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
	//bootBlockInfo = ((BootBlockStruct_t *)(&__DMA_NAND_PALLOAD_BUFFER));
	//printf("Finger Print1 %x\n",bootBlockInfo->m_u32FingerPrint1);
	//printf("Nand page size: %d Bytes\n",bootBlockInfo->NCB_Block1.m_u32DataPageSize);/*
	//printf("1st FW Image: %d\n",bootBlockInfo->LDLB_Block2.m_u32Firmware_startingSector);
	//printf("2st FW Image: %d\n",bootBlockInfo->LDLB_Block2.m_u32Firmware_startingSector2);
	//printf("1st FW Size: %d\n",bootBlockInfo->LDLB_Block2.m_uSectorsInFirmware);
	//printf("2st FW Size: %d\n",bootBlockInfo->LDLB_Block2.m_uSectorsInFirmware2);*/
	//read_nand_pages(0x300,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);

	//userDataStartBlock = *((unsigned int *)((unsigned int)&__DMA_NAND_PALLOAD_BUFFER) + 13 + 4) +
	//                     *((unsigned int *)((unsigned int)&__DMA_NAND_PALLOAD_BUFFER) + 17 + 2) ;//+ 0x300;

	/*
		*((unsigned char *)((unsigned int)&firstPartitionBlock) + 3) = *((unsigned char *)((unsigned int)&__DMA_NAND_PALLOAD_BUFFER) + 0x1C6 + 3);
		*((unsigned char *)((unsigned int)&firstPartitionBlock) + 2) = *((unsigned char *)((unsigned int)&__DMA_NAND_PALLOAD_BUFFER) + 0x1C6 + 2);
		*((unsigned char *)((unsigned int)&firstPartitionBlock) + 1) = *((unsigned char *)((unsigned int)&__DMA_NAND_PALLOAD_BUFFER) + 0x1C6 + 1);
		*((unsigned char *)((unsigned int)&firstPartitionBlock) + 0) = *((unsigned char *)((unsigned int)&__DMA_NAND_PALLOAD_BUFFER) + 0x1C6 + 0);
		firstPartitionBlock += userDataStartBlock;
		printf("1st Partition blk addr: %x\n",firstPartitionBlock);
	*/
	//read_nand_pages(userDataStartBlock,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
	/*
		for(unsigned int pageStart = firstPartitionBlock; pageStart < userDataStartBlock+100 ; pageStart++)
			{

				read_nand_pages(pageStart,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
				for(unsigned int i = &__DMA_NAND_PALLOAD_BUFFER; i < (&__DMA_NAND_PALLOAD_BUFFER + 0x800/4); i++)
					{
						uartdbg_printf("%x ",*((unsigned char *)i));
					}

			}*/
/*
	volatile unsigned int cdcd=0;

	for(int pageStart = 0; pageStart < 0x8000 ; pageStart++)
		{
			read_nand_pages(pageStart,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);

			if( (*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x786D6170) ||
				(*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x00010203) ||
				(*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x211DEBFA) ||
				(*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x45584D41)
				)
				{
					cdcd=4;
				}	

			if(cdcd)
				{
					
					uartdbg_printf("PAGE %x :\n",pageStart);
					for(unsigned int i = &__DMA_NAND_PALLOAD_BUFFER; i < (&__DMA_NAND_PALLOAD_BUFFER + (0x800)/4); i+=4)
							{
								uartdbg_printf("%x ",*((unsigned int *)i));
							}
					
					uartdbg_putc('\n');
					cdcd--;
					
					
				}
		}
	printf("User Disk blk addr: %x\n",userDataStartBlock);
	read_nand_pages(userDataStartBlock,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
 
*/
}