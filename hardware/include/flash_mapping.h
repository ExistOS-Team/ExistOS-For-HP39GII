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


 
#ifndef _FLASH_MAPPING_H
#define _FLASH_MAPPING_H

#include "nand.h"
#ifdef __cplusplus 
extern "C" { 
#endif



/* Header for zone and phy maps when stored on the NAND. */
struct NandMapSectionHeader
{
    /* Common signature for all map types. */
    uint32_t signature;  
    
    /* 'zone' or 'phys' */
    uint32_t mapType;  
    
    /* Version of this header structure, see #kNandMapSectionHeaderVersion. */
    uint32_t version;    
    
    /* Size in bytes of each entry. */
    uint32_t entrySize;  
    
    /* Total number of entries in this section. */
    uint32_t entryCount; 
    
    /* LBA for the first entry in this section. */
    uint32_t startLba;   
};

/* Type for the map section header. */
typedef struct NandMapSectionHeader NandMapSectionHeader_t;


#define NAND_MAX_DBBT_PAGES_PER_NAND (1)
#define BOOTBLOCKSTRUCT_RESERVED1_SIZE_U32          (10)
#define BOOTBLOCKSTRUCT_RESERVED2_SIZE_U32          (19)
#define BOOTBLOCKSTRUCT_FIRMWAREBLOCKDATA_SIZE_U32  (128)
#define MAX_BBRC_REGIONS (32)

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

/* Possible types of a logical drive. */
typedef enum {
    /* 
    ** Public data drive.
    **
    ** Read/write with random access. Typically accessible through the filesystem. Accessible via
    ** USB mass storage and MTP. 
    */
    kDriveTypeData = 0,
    
    /* 
    ** System drive.
    **
    ** Designed to hold firmware or system resources that are accessed frequently but written very
    ** rarely. Must be erased before it can be written, and sectors must be written in sequential
    ** order with none skipped. Also, each sector can only be written to once. 
    */
    kDriveTypeSystem,
    
    /* 
    ** Hidden data drive.
    ** 
    ** Similar to #kDriveTypeData, except that hidden data drives are not accessible through
    ** USB mass storage or MTP. 
    */
    kDriveTypeHidden,
    
    /* 
    ** Unknown drive type.
    **
    ** Drives should never be this type. 
    */
    kDriveTypeUnknown
} LogicalDriveType_t;

typedef struct _NandConfigBlockRegionInfo {

    /* 
    ** Constants for region tag values.
    ** 
    ** These constants define special values for the \a wTag field of a config block region
    ** info structure. In addition to these values, the normal drive tag values are valid. 
    */
	/*
    enum _region_tag_constants
    {
        // Tag value for a boot region in the config block. 
        kBootRegionTag = 0x7fffffff
    };*/

    /* Some System Drive, or Data Drive */
    LogicalDriveType_t eDriveType;       
    
    /* Drive Tag */
    uint32_t wTag;              
    
    /* Size, in blocks, of whole Region. Size includes embedded Bad Blocks */
    int iNumBlks;         
    
    /* Chip number that region is located on. */
    int iChip;            
    
    /* Region's start block relative to chip. */
    int iStartBlock;      
} NandConfigBlockRegionInfo_t;

/* Configuration block info sector */
typedef struct _NandConfigBlockInfo {
    /* #NAND_CONFIG_BLOCK_MAGIC_COOKIE */
    int iMagicCookie;       
    
    /* #NAND_CONFIG_BLOCK_VERSION */
    int iVersionNum;        
    
    /* Number Bad Blocks on this Chip */
    int iNumBadBlks;        
    
    /* Number of regions in the region array. */
    int iNumRegions;        
    
    /* Total number of reserved blocks on this chip enable. */
    int iNumReservedBlocks; 
    
    /* Information about the regions on this chip enable. */
    //NandConfigBlockRegionInfo_t Regions[1]; 
} NandConfigBlockInfo_t;

typedef struct MetadataFields
{
    uint8_t blockStatus;    //!< Non-0xff value means the block is bad.
    uint8_t blockNumber;    //!< Logical block number used for system drives.
    union {
        struct {
            uint16_t lba0;   //!< Halfword 0 of the logical block address.
            uint16_t lsi;   //!< The logical sector index.
        };
        struct {
            uint8_t tag0;   //!< Byte 0 of the tag, MSB of the tag word.
            uint8_t tag1;   //!< Byte 1 of the tag.
            uint8_t tag2;   //!< Byte 2 of the tag.
            uint8_t tag3;   //!< Byte 3 of the tag, LSB of the tag word.
        };
    };
    uint16_t lba1;   //!< Halfword 1 of the logical block address.
    uint8_t flags;  //!< Flags fields.
    uint8_t reserved;   //!< Current unused.
}MetadataFields;


int Flash_mapping_init();
void mapping_read_lba_page(unsigned int lba_start_page, unsigned int *buffer, unsigned int pages);


#ifdef __cplusplus 
};
#endif

#endif