#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "tusb.h"

/* System serive includes. */
#include "ServiceUSBDevice.h"
#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "display.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

NandConfigBlockInfo_t *NandConfigBlock;
NandConfigBlockRegionInfo_t Regions[10];
unsigned int nRegion = 0;
unsigned char _isRawFlash = true;

void vScanAndBuildRegionInfo(){
	unsigned int block;
	unsigned int *buffer;
	
	nRegion = 0 ;
	buffer = pvPortMalloc(2048);
	
	for(block = 0; block < 64; block++){
		xReadFlashPages( block * 64 + 1, 1 ,buffer, 5000);
		
		NandConfigBlock = (NandConfigBlockInfo_t *)buffer;
		
		if((NandConfigBlock->iMagicCookie == NAND_CONFIG_BLOCK_MAGIC_COOKIE) && 
		
		   (NandConfigBlock->iVersionNum  == NAND_CONFIG_BLOCK_VERSION)){
			   _isRawFlash = false;
			   printf("Found %d STMP Region(s).\n", NandConfigBlock->iNumRegions);
				
			   for(int i=0 ; i<NandConfigBlock->iNumRegions; i++ ){
				   Regions[i].eDriveType = NandConfigBlock->Regions[i].eDriveType;
				   Regions[i].wTag = NandConfigBlock->Regions[i].wTag;
				   Regions[i].iNumBlks = NandConfigBlock->Regions[i].iNumBlks;
				   Regions[i].iChip = NandConfigBlock->Regions[i].iChip;
				   Regions[i].iStartBlock = NandConfigBlock->Regions[i].iStartBlock;
				   nRegion++;
				   printf("%d: DriveType: %d, Block Size: %d, Start Block: %d, Tag: %08X\n",i, 
																NandConfigBlock->Regions[i].eDriveType,
																NandConfigBlock->Regions[i].iNumBlks,
																NandConfigBlock->Regions[i].iStartBlock,
																NandConfigBlock->Regions[i].wTag
																
																);
			   }
			   
			   break;
		   }
	}
	
	vPortFree(buffer);
	
}


bool isRawFlash()
{
	return _isRawFlash;
}

unsigned int getDataRegonTotalBlocks(){
	if(_isRawFlash){
		return 0;
	}
	for(int i = 0; i < nRegion; i++){
		if(Regions[i].eDriveType == kDriveTypeData){
			return Regions[i].iNumBlks;
		}
	}
	return 0;
}

unsigned int getDataRegonStartBlock(){
	if(_isRawFlash){
		return 0;
	}
	for(int i = 0; i < nRegion; i++){
		if(Regions[i].eDriveType == kDriveTypeData){
			return Regions[i].iStartBlock;
		}
	}
	return 0;
}

void vSTMPPartition( void *pvParameters )
{
	vTaskDelay(1);
	
	vScanAndBuildRegionInfo();
	
	for(;;) {
		vTaskSuspend(NULL);
	}
	
	
}