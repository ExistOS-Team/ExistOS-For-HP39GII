#ifndef __STMP_NAND_CONTRL_BLOCK__
#define __STMP_NAND_CONTRL_BLOCK__

#include <stdint.h>


typedef struct _NAND_Control_Block  
{
	uint32_t m_u32Fingerprint1;
	struct
	{
		uint8_t m_u8DataSetup;
		uint8_t m_u8DataHold;
		uint8_t m_u8AddressSetup;
		uint8_t m_u8DSAMPLE_TIME;
	} NAND_Timing;
		uint32_t m_u32DataPageSize; //!< 2048 for 2K pages, 4096 for 4K pages.
		uint32_t m_u32TotalPageSize; //!< 2112 for 2K pages, 4314 for 4K pages.
		uint32_t m_u32SectorsPerBlock; //!< Number of 2K sections per block.
		uint32_t m_u32SectorInPageMask; //!< Mask for handling pages > 2K.
		uint32_t m_u32SectorToPageShift; //!< Address shift for handling pages > 2K.
		uint32_t m_u32NumberOfNANDs; //!< Total Number of NANDs - not used by ROM.
		
		uint32_t Reserve1[3];
		
		uint32_t m_u32Fingerprint2; // @ word offset 10
		
		uint32_t m_u32NumRowBytes; //!< Number of row bytes in read/write transactions.
		uint32_t m_u32NumColumnBytes;//!< Number of row bytes in read/write transactions.
		uint32_t m_u32TotalInternalDie; //!< Number of separate chips in this NAND.
		uint32_t m_u32InternalPlanesPerDie; //!< Number of internal planes -
		// !<treat like separate chips.
		uint32_t m_u32CellType; //!< MLC or SLC.
		uint32_t m_u32ECCType; //!< 4 symbol or 8 symbol ECC?
		uint32_t m_u32Read1stCode; //!< First value sent to initiate a NAND
		//!< Read sequence.
		uint32_t m_u32Read2ndCode; //!< Second value sent to initiate a NAND
		//!< Read sequence.
		
		uint32_t Reserve[12];
		
		uint32_t m_u32Fingerprint3; // @ word offset 20
} NAND_Control_Block ;

typedef struct _LogicalDriveLayoutBlock
{
	uint32_t m_u32Fingerprint1;
	struct
		{
			uint16_t m_u16Major;
			uint16_t m_u16Minor;
			uint16_t m_u16Sub;
		} LDLB_Version;
		
	uint16_t Reserve1;
	uint32_t Reserve2[8];
		
	uint32_t m_u32Fingerprint2;
	
	uint32_t m_u32Firmware_startingNAND;
	uint32_t m_u32Firmware_startingSector;
	uint32_t m_u32Firmware_sectorStride;
	uint32_t m_u32SectorsInFirmware;
	uint32_t m_u32Firmware_StartingNAND2;
	uint32_t m_u32Firmware_StartingSector2;
	uint32_t m_u32Firmware_SectorStride2;
	uint32_t m_u32SectorsInFirmware2;
	struct
		{
			uint16_t m_u16Major;
			uint16_t m_u16Minor;
			uint16_t m_u16Sub;
		} FirmwareVersion;

	uint32_t m_u32DiscoveredBBTableSector;
	uint32_t m_u32DiscoveredBBTableSector2;
	uint32_t Rsvd[8];
	
	uint32_t m_u32Fingerprint3;
	//uint32_t RSVD[100]; // Region configuration used by SDK only.
	
} LogicalDriveLayoutBlock_t ;



typedef struct _DiscoveredBadBlockTable
{
    uint32_t m_u32Fingerprint1;
    uint32_t m_u32NumberBB_NAND0;
    uint32_t m_u32NumberBB_NAND1;
    uint32_t m_u32NumberBB_NAND2;
    uint32_t m_u32NumberBB_NAND3;
    uint32_t m_u32Number2KPagesBB_NAND0;
    uint32_t m_u32Number2KPagesBB_NAND1;
    uint32_t m_u32Number2KPagesBB_NAND2;
    uint32_t m_u32Number2KPagesBB_NAND3;
    uint32_t RSVD[2];
    uint32_t m_u32Fingerprint2;
    uint32_t RSVD2[20];
    uint32_t m_u32Fingerprint3;
}DiscoveredBadBlockTable_t;



#endif