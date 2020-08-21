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


#ifndef __NAND_H
#define __NAND_H
#ifdef __cplusplus 
extern "C" { 
#endif

#include "regsgpmi.h"
#include "regsecc8.h"

#include <stdint.h>
#include <stddef.h>

#define NAND_MAX_ID_LEN 8

typedef enum OperationStatus{
	operationDone = 0,
	deviceBusy,
	deviceTimeout
}OperationStatus;


unsigned int userDataStartBlock;
unsigned int firstPartitionBlock;



	  
#define NAND_CMD_READ0			0
#define NAND_CMD_READ1			1
#define NAND_CMD_RNDOUT			5
#define NAND_CMD_PAGEPROG		0x10
#define NAND_CMD_READOOB		0x50
#define NAND_CMD_ERASE1			0x60
#define NAND_CMD_STATUS			0x70
#define NAND_CMD_SEQIN			0x80
#define NAND_CMD_RNDIN			0x85
#define NAND_CMD_READID			0x90
#define NAND_CMD_ERASE2			0xd0
#define NAND_CMD_PARAM			0xec
#define NAND_CMD_GET_FEATURES	0xee
#define NAND_CMD_SET_FEATURES	0xef
#define NAND_CMD_RESET			0xff

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_RNDOUTSTART	0xE0
#define NAND_CMD_CACHEDPROG	0x15
#define NAND_CMD_NONE		-1
#define NAND_CMD_LOCK		0x2a
#define NAND_CMD_UNLOCK1	0x23
#define NAND_CMD_UNLOCK2	0x24


typedef struct NAND_Timing_t {
	unsigned char DataSetup;
	unsigned char DataHold;
	unsigned char AddressSetup;
	unsigned char SampleDelay;
}NAND_Timing_t;




typedef enum hw_DmaCommand
{
    NO_DMA_XFER = 0,
    DMA_WRITE = 1,		//从设备写到内存
    DMA_READ = 2			//从内存读到设备
} hw_DmaCommand;



typedef struct hw_gpmi_DmaDesc	//用于GPMI控制器的DMA描述符
{
    struct _hw_gpmi_DmaDesc *p_Next;
    union 
    {
        struct 
        {
            union 
            {
                struct 
                {
                    hw_DmaCommand Command : 2;
                    unsigned char Chain : 1;
                    unsigned char IRQOnCompletion : 1;
                    unsigned char NANDLock : 1;
                    unsigned char NANDWaitForReady : 1;
                    unsigned char Semaphore : 1;
                    unsigned char WaitForEndCommand : 1;
                    unsigned char HALTONTERMINATE : 1;
					
                } ;
                unsigned char Bits;
            };
            unsigned char Reserved : 3;
            unsigned char CMDWORDS : 4;
            unsigned short XFER_COUNT: 16;
        };
        unsigned int CommandBits;
    };
    void *p_DMABuffer;
    hw_gpmi_ctrl0_t PioWord0;
    unsigned int PioWord1;
    unsigned int PioWord2;
    unsigned int PioWord3;

} hw_gpmi_DmaDesc;




void NAND_init();

unsigned int GPMI_dma_is_busy();
void GPMI_read_dat(unsigned char *buffer_address, unsigned int dat_size);
void GPMI_send_dat(unsigned int *dat_address, unsigned int data_size_bytes);
void GPMI_send_cmd(unsigned int command, unsigned int address, unsigned int address_size_bytes, unsigned int readback_size_bytes, unsigned char *readback_buffer_address);


unsigned int gpmi_is_busy();
unsigned int read_nand_pages(unsigned int start_page, unsigned int pages, unsigned int *buffer, unsigned int timeout_ms);




#endif

