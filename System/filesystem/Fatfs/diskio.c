/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */

#include "sys_llapi.h"
#include <stdio.h>
/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2 /* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    LBA_t sector, /* Start sector in LBA */
    UINT count    /* Number of sectors to read */
) {
    int result;

    result = ll_flash_page_read(sector, count, buff);
    if (result == 0) {
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    LBA_t sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
) {
    int result;

    result = ll_flash_page_write(sector, count, (uint8_t *)buff);
    if (result == 0) {
        return RES_OK;
    }

    return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
) {
    DRESULT res;
    DWORD val;
    int result;

    switch (cmd) {
    case CTRL_SYNC:
        ll_flash_sync();
        return RES_OK;
    case CTRL_TRIM: {
        LBA_t *lba = (LBA_t *)buff;
        uint32_t startLBA, endLBA;
        startLBA = lba[0];
        endLBA = lba[1];
        for (; startLBA <= endLBA; startLBA++) {
            ll_flash_page_trim(startLBA);
        }
        return RES_OK;
    }
    case GET_BLOCK_SIZE:
        *((DWORD *)buff) = 1;
        return RES_OK;
    case GET_SECTOR_COUNT: 
        val = ll_flash_get_pages();
        printf("sc:%d\n", val);
        *((LBA_t *)buff) = val;
        return RES_OK;
    case GET_SECTOR_SIZE:
        val = ll_flash_get_page_size();
        printf("ss:%d\n", val);
        //((BYTE *)buff)[0] = val & 0xFF;
        //((BYTE *)buff)[1] = (val>>8) & 0xFF;
        *((WORD *)buff) = val;
        return RES_OK;
    default:
        break;
    }

	//printf("unknown opa:%d\n", cmd);

    return RES_PARERR;
}
