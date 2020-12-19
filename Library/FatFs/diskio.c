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
#include "map.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_FLASH	0	/* Example: Map Ramdisk to physical drive 0 */


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
	case DEV_FLASH :
		result = RES_OK;
		

		return result;

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
	//printf("init driver.\n");
	switch (pdrv) {
	case DEV_FLASH :
		result = RES_OK;
		//printf("init DEV_FLASH.\n");
		return result;

	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

extern struct dhara_map map;

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{

	DRESULT result;
	dhara_error_t err = 0;
	result = RES_OK;
	//printf("read disk %d \n",pdrv);
	switch (pdrv) {
		case DEV_FLASH :
			//printf("read:%d, count:%d \n",sector,count);
			
			
			for(int i=0; i<count; i++){
				dhara_map_read(&map, sector + i, buff, &err);
				//printf("err:%d\n",err);
				if(err != DHARA_E_NONE){
					result = RES_ERROR;
					break;
				}
			}



	}

	return result;
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

	DRESULT result;
	dhara_error_t err = 0;
	result = RES_OK;
	
	switch (pdrv) {
		case DEV_FLASH :
			//printf("write:%d, count:%d \n",sector,count);
			for(int i=0; i<count; i++){
				dhara_map_write(&map, sector + i, buff, &err);
				if(err != DHARA_E_NONE){
					result = RES_ERROR;
					break;
				}
			}
			
	}

	return result;
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
	DRESULT res = RES_OK;
	dhara_error_t err = 0;
	
	switch (pdrv) {
		case DEV_FLASH :
		
			//printf("cmd:%d\n",cmd);
			switch ( cmd ) {
				case CTRL_SYNC:
					dhara_map_sync(&map, &err);
					break;
				case GET_SECTOR_COUNT:
					//res = dhara_map_capacity(&map);
					*((unsigned int *)buff) = dhara_map_capacity(&map);
					return RES_OK;
				case GET_SECTOR_SIZE:
					*((unsigned int *)buff)  = 2048;
					return RES_OK;
				case GET_BLOCK_SIZE:
					*((unsigned int *)buff)  = 64;
					return RES_OK;
				case CTRL_TRIM:
					dhara_map_trim(&map, *((unsigned int *)buff), &err);
					break;
			}
		if(err != DHARA_E_NONE){
					res = RES_ERROR;
				}	
		return res;
	}
}



