/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "FTL_up.h"

#include "../debug.h"

/* Definitions of physical drive number for each drive */
#define DEV_NAND		0	

 
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_NAND :
		result = 0;
		stat = !FTL_inited();

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_NAND :
		result = 0;
		stat = !FTL_inited();

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;
	switch (pdrv) {
	case DEV_NAND :
		// translate the arguments here

		result = FTL_ReadSector(sector, count, buff);
		
		if(!result){
			res = RES_OK;
		}else{
			res = RES_NOTRDY;
		}

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_NAND :
		// translate the arguments here

		result = FTL_WriteSector(sector, count, ( uint8_t *)buff);
		if(!result){
			res = RES_OK;
		}else{
			res = RES_NOTRDY;
		}

		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	uint32_t startLBA, endLBA;

	switch (pdrv) {
	case DEV_NAND :

		switch (cmd)
		{
		case CTRL_SYNC:
			result = FTL_Sync();
			if(!result){
				res = RES_OK;
			}else{
				res = RES_NOTRDY;
			}
			break;
		case CTRL_TRIM:
			startLBA = ((uint32_t *)buff)[0];
			endLBA = ((uint32_t *)buff)[1];
			for(;startLBA <= endLBA; startLBA++){
				result = FTL_TrimSector(startLBA);
			}
			
			if(!result){
				res = RES_OK;
			}else{
				res = RES_NOTRDY;
			}
			break;
		case GET_BLOCK_SIZE:
			res = RES_OK;
			*((uint32_t *)buff) = FTL_GetSectorSize();
			break;

		case GET_SECTOR_COUNT:
			res = RES_OK;
			*((uint32_t *)buff) = FTL_GetSectorCount();
			break;

		default:
			break;
		}

		return res;
	}

	return RES_PARERR;
}

