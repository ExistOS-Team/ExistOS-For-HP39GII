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
#define NAND_MAX_DBBT_PAGES_PER_NAND (1)

#define BOOTBLOCKSTRUCT_RESERVED1_SIZE_U32          (10)
#define BOOTBLOCKSTRUCT_RESERVED2_SIZE_U32          (19)
#define BOOTBLOCKSTRUCT_FIRMWAREBLOCKDATA_SIZE_U32  (128)

#define MAX_BBRC_REGIONS (32)




typedef struct NAND_Timing_t {
	unsigned char DataSetup;
	unsigned char DataHold;
	unsigned char AddressSetup;
	unsigned char SampleDelay;
}NAND_Timing_t;

typedef uint8_t NAND_Timing_State_t;

typedef struct NAND_Timing2_struct_t
{
    NAND_Timing_State_t eState;             //!< One of enum e_NAND_Timing_State.

    uint8_t             u8DataSetup;        //!< The data setup time (tDS), in nanoseconds.

    uint8_t             u8DataHold;         //!< The data hold time (tDH), in nanoseconds.

    uint8_t             u8AddressSetup;     //!< The address setup time (tSU), in nanoseconds.
                                            //! This value amalgamates the NAND parameters tCLS, tCS, and tALS.

    uint8_t             u8DSAMPLE_TIME;     //!< The data sample time, in nanoseconds.

    uint8_t             u8REA;              //!< From the NAND datasheet.

    uint8_t             u8RLOH;             //!< From the NAND datasheet.
                                            //! This is the amount of time that the last
                                            //! contents of the data lines will persist
                                            //! after the controller drives the -RE
                                            //! signal true.
                                            //! EDO Mode: This time is from the NAND spec, and the persistence of data
                                            //! is determined by (tRLOH + tDH).
                                            //! Non-EDO Mode: This time is ignored, because the persistence of data
                                            //! is determined by tRHOH.

    uint8_t             u8RHOH;             //!< From the NAND datasheet.
                                            //! This is the amount of time
                                            //! that the last contents of the data lines will persist after the
                                            //! controller drives the -RE signal false.
                                            //! EDO Mode: This time is ignored, because the persistence of data
                                            //! is determined by (tRLOH + tDH).
                                            //! Non-EDO Mode: This time is totally due to capacitive effects of the
                                            //! hardware.  For reliable behavior it should be set to zero, unless there is
                                            //! specific knowledge of the trace capacitance and the persistence of the
                                            //! data values.
} NAND_Timing2_struct_t;

typedef struct _DiscoveredBadBlockStruct_t
{
    union
    {
        struct 
        {
            uint32_t        m_u32NumberBB_NAND0;		//!< # Bad Blocks stored in this table for NAND0.
            uint32_t        m_u32NumberBB_NAND1;		//!< # Bad Blocks stored in this table for NAND1.
            uint32_t        m_u32NumberBB_NAND2;		//!< # Bad Blocks stored in this table for NAND2.
            uint32_t        m_u32NumberBB_NAND3;		//!< # Bad Blocks stored in this table for NAND3.  
        };
        uint32_t    m_u32NumberBB_NAND[4];          
    };
    union
    {
        struct 
        {
            uint32_t        m_u32Number2KPagesBB_NAND0; //!< Bad Blocks for NAND0 consume this # of 2K pages.   
            uint32_t        m_u32Number2KPagesBB_NAND1;	//!< Bad Blocks for NAND1 consume this # of 2K pages.  
            uint32_t        m_u32Number2KPagesBB_NAND2;	//!< Bad Blocks for NAND2 consume this # of 2K pages.
            uint32_t        m_u32Number2KPagesBB_NAND3;	//!< Bad Blocks for NAND3 consume this # of 2K pages.
        };
        uint32_t    m_u32Number2KPagesBB_NAND[4];
    };
} DiscoveredBadBlockStruct_t;


typedef struct BadBlocksPerRegionCounts_t   // i.e. the "BBRC"
{
    //! \brief Quantity of valid entries in the u32BadBlocks array.
    uint32_t    u32Entries;
    //! \brief An array of quantities of bad blocks, one quantity per region.
    uint32_t    u32NumBadBlksInRegion[MAX_BBRC_REGIONS];
} BadBlocksPerRegionCounts_t;


typedef struct _BootBlockStruct_t
{
    uint32_t    m_u32FingerPrint1;      //!< First fingerprint in first byte.
    union
    {
        struct
        {            
            NAND_Timing_t   m_NANDTiming;           //!< Optimum timing parameters for Tas, Tds, Tdh in nsec.
            uint32_t        m_u32DataPageSize;      //!< 2048 for 2K pages, 4096 for 4K pages.
            uint32_t        m_u32TotalPageSize;     //!< 2112 for 2K pages, 4314 for 4K pages.
            uint32_t        m_u32SectorsPerBlock;   //!< Number of 2K sections per block.
            uint32_t        m_u32SectorInPageMask;  //!< Mask for handling pages > 2K.
            uint32_t        m_u32SectorToPageShift; //!< Address shift for handling pages > 2K.
            uint32_t        m_u32NumberOfNANDs;     //!< Total Number of NANDs - not used by ROM.
        } NCB_Block1;
        struct
        {
            struct  
            {
                uint16_t    m_u16Major;             
                uint16_t    m_u16Minor;
                uint16_t    m_u16Sub;
                uint16_t    m_u16Reserved;
            } LDLB_Version;                     //!< LDLB version - not used by ROM.
            uint32_t    m_u32NANDBitmap;        //!< bit 0 == NAND 0, bit 1 == NAND 1, bit 2 = NAND 2, bit 3 = NAND3
        } LDLB_Block1;
        DiscoveredBadBlockStruct_t zDBBT1;
        // This one just forces the spacing.
        uint32_t    m_Reserved1[BOOTBLOCKSTRUCT_RESERVED1_SIZE_U32];
    };
    uint32_t    m_u32FingerPrint2;      //!< 2nd fingerprint at word 10.
    union
    {
        struct
        {
            uint32_t        m_u32NumRowBytes;   //!< Number of row bytes in read/write transactions.
            uint32_t        m_u32NumColumnBytes;//!< Number of row bytes in read/write transactions.
            uint32_t        m_u32TotalInternalDie;  //!< Number of separate chips in this NAND.
            uint32_t        m_u32InternalPlanesPerDie;  //!< Number of internal planes - treat like separate chips.
            uint32_t        m_u32CellType;      //!< MLC or SLC.
            uint32_t        m_u32ECCType;       //!< 4 symbol or 8 symbol ECC?
            uint32_t        m_u32Read1stCode;   //!< First value sent to initiate a NAND Read sequence.
            uint32_t        m_u32Read2ndCode;   //!< Second value sent to initiate a NAND Read sequence.


        } NCB_Block2;
        struct
        {
            uint32_t    m_u32Firmware_startingNAND;     //!< Firmware image starts on this NAND.
            uint32_t    m_u32Firmware_startingSector;   //!< Firmware image starts on this sector.
            uint32_t    m_u32Firmware_sectorStride;     //!< Amount to jump between sectors - unused in ROM.
            uint32_t    m_uSectorsInFirmware;           //!< Number of sectors in firmware image.
            uint32_t    m_u32Firmware_startingNAND2;    //!< Secondary FW Image starting NAND.
            uint32_t    m_u32Firmware_startingSector2;  //!< Secondary FW Image starting Sector.
            uint32_t    m_u32Firmware_sectorStride2;    //!< Secondary FW Image stride - unused in ROM.
            uint32_t    m_uSectorsInFirmware2;          //!< Number of sector in secondary FW image.
            struct  
            {
                uint16_t    m_u16Major;
                uint16_t    m_u16Minor;
                uint16_t    m_u16Sub;
                uint16_t    m_u16Reserved;
            } FirmwareVersion;
            uint32_t    m_u32DiscoveredBBTableSector;   //!< Location of Discovered Bad Block Table (DBBT).
            uint32_t    m_u32DiscoveredBBTableSector2;  //!< Location of backup DBBT 
        } LDLB_Block2;
        // This one just forces the spacing.
        uint32_t    m_Reserved2[BOOTBLOCKSTRUCT_RESERVED2_SIZE_U32];    
    };

    uint16_t    m_u16Major;         //!< Major version of BootBlockStruct_t
    uint16_t    m_u16Minor;         //!< Minor version of BootBlockStruct_t

    uint32_t    m_u32FingerPrint3;    //!< 3rd fingerprint at word 30.

    //! \brief Contains values used by firmware, not by ROM.
    struct {
        uint16_t                m_u16Major;             //!< Major version of BootBlockStruct_t.FirmwareBlock
        uint16_t                m_u16Minor;             //!< Minor version of BootBlockStruct_t.FirmwareBlock

        union
        {
            uint32_t                    m_u32FirmwareBlockData[BOOTBLOCKSTRUCT_FIRMWAREBLOCKDATA_SIZE_U32]; 
                                                            //!< Minimum size of BootBlockStruct_t.FirmwareBlock.
                                                            //!< Also provides a place for miscellaneous data storage.
            NAND_Timing2_struct_t       NAND_Timing2_struct;//!< Timing values for the GPMI interface to the NAND.

            BadBlocksPerRegionCounts_t  BadBlocksPerRegionCounts;
                                                            //!< Contains counts of bad-blocks in all regions.
        };
    
    } FirmwareBlock;

} BootBlockStruct_t;




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

