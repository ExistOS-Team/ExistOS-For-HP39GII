
#include "stmp37xxNandConf.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "board_up.h"
#include "mtd_up.h"
#include "FTL_up.h"
#include "display_up.h"
#include "keyboard_up.h"

uint8_t LDLB2[144] = {
	0x03, 0x02, 0x01, 0x00, 0x0B, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x06, 0x00, 0x00, 0x00, 
	0x2A, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 
	0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x70, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xA9, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};



void mkNCB(uint32_t inBlock)
{
  mtdInfo_t *info = MTD_getDeviceInfo();
  
  uint8_t *pageBuff = pvPortMalloc(info->PageSize_B);
  
  uint8_t *metaBuff = pvPortMalloc(info->MetaSize_B);

	memset(pageBuff, 0xff, info->PageSize_B);
	memset(metaBuff, 0xff, info->MetaSize_B);

  NAND_Control_Block *NCB = (NAND_Control_Block *)pageBuff;

	NCB->m_u32Fingerprint1 = 0x504D5453;
	NCB->NAND_Timing.m_u8DataSetup = 0x10;
	NCB->NAND_Timing.m_u8DataHold = 0x0C;
	NCB->NAND_Timing.m_u8AddressSetup = 0x14;
	NCB->NAND_Timing.m_u8DSAMPLE_TIME = 0x06;
	NCB->m_u32DataPageSize = 0x800;
	NCB->m_u32TotalPageSize = 0x840;
	NCB->m_u32SectorsPerBlock = 0x40;
	NCB->m_u32SectorInPageMask = 0;
	NCB->m_u32SectorToPageShift = 0;
	NCB->m_u32NumberOfNANDs = 1;
	
	NCB->m_u32Fingerprint2 = 0x2042434E;
	
	NCB->m_u32NumRowBytes = 2;
	NCB->m_u32NumColumnBytes = 2;
	NCB->m_u32TotalInternalDie = 1;
	NCB->m_u32InternalPlanesPerDie = 1;
	NCB->m_u32CellType = 2;
	NCB->m_u32ECCType = 0;
	NCB->m_u32Read1stCode = 0x00;
	NCB->m_u32Read2ndCode = 0x30;
	NCB->m_u32Fingerprint3 = 0x4E494252;

	
	metaBuff[2] = 0x42;
	metaBuff[3] = 0x43;
	metaBuff[4] = 0x42;
	metaBuff[5] = 0x20;

  MTD_ErasePhyBlock(inBlock);

/*
  MTD_WritePhyPageMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, metaBuff);
  MTD_WritePhyPage(inBlock * info->PagesPerBlock, pageBuff);
*/

  MTD_WritePhyPageWithMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, pageBuff, metaBuff);

  vPortFree(pageBuff);
  vPortFree(metaBuff);

}

void mkDBBT(uint32_t inBlock)
{
  mtdInfo_t *info = MTD_getDeviceInfo();
  uint8_t *pageBuff = pvPortMalloc(info->PageSize_B);
  uint8_t *metaBuff = pvPortMalloc(info->MetaSize_B);

	memset(pageBuff, 0x00, info->PageSize_B);
	memset(metaBuff, 0xff, info->MetaSize_B);

  DiscoveredBadBlockTable_t *DBBT = (DiscoveredBadBlockTable_t *)pageBuff;

  DBBT->m_u32Fingerprint1 = 0x504D5453;
  DBBT->m_u32Number2KPagesBB_NAND0 = 1;
  DBBT->m_u32Number2KPagesBB_NAND1 = 1;
  DBBT->m_u32Number2KPagesBB_NAND2 = 1;
  DBBT->m_u32Number2KPagesBB_NAND3 = 1;

  DBBT->m_u32Fingerprint2 = 0x54424244;
  DBBT->m_u32Fingerprint3 = 0x44494252;

  memset(&metaBuff[2], 0, 6);

  MTD_ErasePhyBlock(inBlock);

  MTD_WritePhyPageWithMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, pageBuff, metaBuff);

  vPortFree(pageBuff);
  vPortFree(metaBuff);

}

void mkLDLB(uint32_t inBlock, uint32_t fwPageOffset, uint32_t fwPageTotal, uint32_t DBBT1_page, uint32_t DBBT2_page)
{
  mtdInfo_t *info = MTD_getDeviceInfo();
  uint8_t *pageBuff = pvPortMalloc(info->PageSize_B);
  uint8_t *metaBuff = pvPortMalloc(info->MetaSize_B);

	memset(pageBuff, 0xff, info->PageSize_B);
	memset(metaBuff, 0xff, info->MetaSize_B);

	LogicalDriveLayoutBlock_t *LDLB = (LogicalDriveLayoutBlock_t *)pageBuff;
			
	LDLB->m_u32Fingerprint1	= 0x504D5453;
	LDLB->LDLB_Version.m_u16Major = 1;
	LDLB->LDLB_Version.m_u16Minor = 0;
	LDLB->LDLB_Version.m_u16Sub = 0;
	
	LDLB->m_u32Fingerprint2 = 0x424C444C;
	LDLB->m_u32Firmware_startingNAND = 0;
	LDLB->m_u32Firmware_startingSector = fwPageOffset;
	LDLB->m_u32Firmware_sectorStride = 0;
	LDLB->m_u32SectorsInFirmware = fwPageTotal;

	LDLB->m_u32Firmware_StartingNAND2 = 0;
	LDLB->m_u32Firmware_StartingSector2 = fwPageOffset;
	LDLB->m_u32Firmware_SectorStride2 = 0;
	LDLB->m_u32SectorsInFirmware2 = fwPageTotal;
	
	LDLB->FirmwareVersion.m_u16Major = 1;
	LDLB->FirmwareVersion.m_u16Minor = 0;
	LDLB->FirmwareVersion.m_u16Sub = 0;
	
	LDLB->m_u32DiscoveredBBTableSector = DBBT1_page;
	LDLB->m_u32DiscoveredBBTableSector2 = DBBT2_page;

	LDLB->m_u32Fingerprint3 = 0x4C494252;

	metaBuff[2] = 0x42; //B
	metaBuff[3] = 0x43; //C
	metaBuff[4] = 0x42; //B
	metaBuff[5] = 0x20; //' '

  MTD_ErasePhyBlock(inBlock);

  MTD_WritePhyPageWithMeta(inBlock * info->PagesPerBlock, info->MetaSize_B, pageBuff, metaBuff);

  vPortFree(pageBuff);
  vPortFree(metaBuff);

}


void RestoreLDLB2()
{
    mtdInfo_t *nand_info = MTD_getDeviceInfo();
    uint8_t *pgbuff = pvPortMalloc(nand_info->PageSize_B);
    uint8_t *mtbuff = pvPortMalloc(nand_info->MetaSize_B);


    memset(pgbuff, 0xFF, nand_info->PageSize_B);
    memcpy(pgbuff, LDLB2, sizeof(LDLB2));
    memset(mtbuff, 0xFF, nand_info->MetaSize_B);
    mtbuff[2] = 0x42; //B
	mtbuff[3] = 0x43; //C
	mtbuff[4] = 0x42; //B
	mtbuff[5] = 0x20; //' ' 
    MTD_WritePhyPageWithMeta(8*64+1, nand_info->MetaSize_B, pgbuff, mtbuff);
    MTD_WritePhyPageWithMeta(12*64+1, nand_info->MetaSize_B, pgbuff, mtbuff);

    vPortFree(pgbuff);
    vPortFree(mtbuff);

}


void mkSTMPNandStructure(uint32_t OLStartBlock, uint32_t OLPages)
{
	mkNCB(0);
	mkNCB(4);

	mkDBBT(16);
	mkDBBT(19);

	mkLDLB(8,  OLStartBlock * 64, OLPages, 16 * 64, 19 * 64);
	mkLDLB(12, OLStartBlock * 64, OLPages, 16 * 64, 19 * 64);

	RestoreLDLB2();
	
}

