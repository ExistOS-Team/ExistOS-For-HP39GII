

#ifndef SERVICE_STMP_PARTITION_H
#define SERVICE_STMP_PARTITION_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdbool.h"


#define NAND_CONFIG_BLOCK_MAGIC_COOKIE  0x00010203
#define NAND_CONFIG_BLOCK_VERSION       0x0000000b


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
    NandConfigBlockRegionInfo_t Regions[10]; 
} NandConfigBlockInfo_t;



bool isRawFlash();
void vSTMPPartition( void *pvParameters );
unsigned int getDataRegonTotalBlocks();
unsigned int getDataRegonStartBlock();
void lockFlash(bool lock);

void modifyRegion(unsigned int regionNum, unsigned int newStartBlock, unsigned int newBlockSize, unsigned int tag);
void saveRegionTable();
void installBootimgInPage(unsigned int page, void *buffer);
void resetFlashRegionInfo();
void eraseBootimgRegion();

unsigned int isSTMPDiskInited();

#endif

