/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"

#include "FTL_up.h"
#include "mtd_up.h"

#if CFG_TUD_MSC

//#define RAW_FLASH_ACCESS

// Some MCU doesn't have enough 8KB SRAM to store the whole disk
// We will use Flash as read-only disk with board that has
// CFG_EXAMPLE_MSC_READONLY defined

uint8_t msc_rec_disk_pbr[] =
    {
        0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x40, 0x04, 0x00,
        0x02, 0x00, 0x02, /*sectors*/ 0x00, 0x01, /**/ 0xF8, 0x02, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x3F, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0x60, 0x4C, 0xE9, 0x1C, 0x45, 0x4F, 0x53, 0x5F, 0x52,
        0x45, 0x43, 0x44, 0x53, 0x4B, 0x20, 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x00, 0x00,
        /*... 0x55, 0xAA*/
};

/*
sector    item
0         Boot
1-3       Resv
4-5       FAT1
6-7       FAT2
8-39      ROOT  //cluster 1
40-103    cluster 2 //CMD
104-167   cluster 3 //DAT
168-231   cluster 4
...
*/


uint8_t MSCRBuffer[2048] __aligned(4);
uint8_t MSCWRBuf[2048] __aligned(4);

uint8_t msc_rec_disk_root[] =
    {
        // first entry is volume label
        'E',
        'O',
        'S',
        'R',
        'E',
        'C',
        'D',
        'I',
        'S',
        'K',
        ' ',
        0x08,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x4F,
        0x6D,
        0x65,
        0x43,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,

        'C',
        'M',
        'D',
        '_',
        'P',
        'O',
        'R',
        'T',
        ' ',
        ' ',
        ' ',
        ' ',
        0x08,
        0xAE,
        0x15,
        0xAB,
        0xAE,
        0x54,
        0xAE,
        0x54,
        0x00,
        0x00,
        0xA3,
        0xAC,
        0xAE,
        0x54,
        /*cluster*/ 0x02,
        0x00,
        /*File size*/ 0x00,
        0x80,
        0x00,
        0x00,

        'D',
        'A',
        'T',
        '_',
        'P',
        'O',
        'R',
        'T',
        ' ',
        ' ',
        ' ',
        ' ',
        0x08,
        0xAE,
        0x15,
        0xAB,
        0xAE,
        0x54,
        0xAE,
        0x54,
        0x00,
        0x00,
        0xA3,
        0xAC,
        0xAE,
        0x54,
        /*cluster*/ 0x03,
        0x00,
        /*File size*/ 0x00,
        0x80,
        0x00,
        0x00,
};

uint8_t msc_rec_disk_fat[] =
    {
        0xF8, 0xFF, 0xFF, /*Resv*/ 0xFF, 0xFF, 0xFF, 0x0F};

uint8_t MscCmdBuf[16];

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {
    (void)lun;

    const char vid[] = "ExistOS USB";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted

extern uint32_t CurMount;

bool tud_msc_test_unit_ready_cb(uint8_t lun) {
    (void)lun;

    return true;

    // return (CurMount > 0);
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size

extern uint32_t g_MSC_Configuration;

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
    (void)lun;
    /*
        mtdInfo_t *info = MTD_getDeviceInfo();
        *block_count = info->Blocks * info->PagesPerBlock;
        *block_size  = info->PageSize_B;
        return;
    */

    switch (g_MSC_Configuration) {
    case MSC_CONF_OSLOADER_EDB:
        *block_count = 256;
        *block_size = 512;
        break;
    case MSC_CONF_SYS_DATA:
    #ifndef RAW_FLASH_ACCESS
        *block_count = FTL_GetSectorCount() - FLASH_FTL_DATA_SECTOR;
        *block_size = FTL_GetSectorSize();
        
       #else
        *block_count = 65536;
        *block_size = 2048;
    #endif
        break;
    default:
        *block_count = 0;
        *block_size = 0;
        break;
    }

    // printf("MSC block_count %d,block_size %d\n",*block_count, *block_size);

    /*
      if(CurMount > 0){
        *block_count = MSCpartSectors;
        *block_size  = FTL_GetSectorSize();
      }else{
        *block_count = 0;
        *block_size  = 0;
      }
    */
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
    (void)lun;
    (void)power_condition;

    if (load_eject) {
        if (start) {
            // load disk storage
        } else {
            // unload disk storage
            if (g_MSC_Configuration == MSC_CONF_SYS_DATA) {
                FTL_Sync();
                return false;
            }
        }
    } else {
        if (start) {

        } else {
            if (g_MSC_Configuration == MSC_CONF_SYS_DATA) {
                FTL_Sync();
                return false;
                //tud_disconnect();
            }
        }
    }

    return true;
}

extern char *binBuf;
// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
    (void)lun;

    //
    //printf("RD:lba%d, off:%d, len:%d\n",lba, offset, bufsize);
    switch (g_MSC_Configuration) {
    case MSC_CONF_OSLOADER_EDB:

        switch (lba) {
        case 0:
            memcpy(buffer, msc_rec_disk_pbr, sizeof(msc_rec_disk_pbr));
            ((char *)buffer)[511] = 0xAA;
            ((char *)buffer)[510] = 0x55;
            break;
        case 4:
        case 6:
            memcpy(buffer, msc_rec_disk_fat, sizeof(msc_rec_disk_fat));
            break;
        case 8:
            memcpy(buffer, msc_rec_disk_root, sizeof(msc_rec_disk_root));
            break;
        case 40:
            memcpy(buffer, MscCmdBuf, sizeof(MscCmdBuf));
            break;
        default:
            memset(buffer, 0, bufsize);
            // 104 - 167
            if ((lba >= 104) && (lba <= 167)) {
                if (bufsize == 2048) 
                {
                    memcpy(buffer, &binBuf[2048 * (lba - 104)], 2048);
                }
            }

            break;
        }

        break;

    case MSC_CONF_SYS_DATA:
        //printf("RD:lba%d, off:%d, len:%d\n",lba, offset, bufsize);
        {
            //
            if(bufsize <= 2048)
            {/*
            FTL_ReadSector(FLASH_FTL_DATA_SECTOR + lba, 1, MSCRBuffer);
            uint8_t const *addr = MSCRBuffer + offset;
            memcpy(buffer, addr, bufsize);*/
            
            #ifndef RAW_FLASH_ACCESS
            FTL_ReadSector(FLASH_FTL_DATA_SECTOR + lba, 1, buffer);
            #else
            MTD_ReadPhyPage(lba, offset, bufsize, buffer);
            #endif
            }else{
                printf("RD:lba=%ld, off:%ld, len:%ld\n",lba, offset, bufsize);
            }

        }

        break;

    default:
        break;
    }

    // FTL_ReadSector(lba, MSCRBuffer);
    // uint8_t const* addr = MSCRBuffer + offset;
    // memcpy(buffer, addr, bufsize);

    // FTL_ReadSector(MSCpartStartSector + lba, 1, buffer);

    // MTD_ReadPhyPage(lba, offset, bufsize, buffer);

    // memset(buffer, 0xFF, bufsize);
    // MTD_ReadPhyPageMeta(lba, 19, buffer);

    return bufsize;
}

void parseCDCCommand(char *cmd);
void MscSetCmd(char *cmd) {
    memset(MscCmdBuf, 0, sizeof(MscCmdBuf));
    strcpy((char *)MscCmdBuf, cmd);
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
uint32_t last_lba = 0xFFFFFFFF;
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
    (void)lun;
    //
    //
    //printf("WR:lba%d, off:%d, len:%d\n",lba, offset, bufsize);
    switch (g_MSC_Configuration) {
    case MSC_CONF_OSLOADER_EDB:

        if (lba == 40) {
            buffer[bufsize - 1] = 0;
            if ((strlen((const char *)buffer) - 1) > 0) {
                buffer[(uint32_t)((strlen((const char *)buffer) - 1))] = 0;
            }
            parseCDCCommand((char *)buffer);
        }

        if (binBuf != NULL) {
            // 104 - 167
            if ((lba >= 104) && (lba <= 167)) {
                memcpy(&binBuf[512 * (lba - 104)], buffer, bufsize);
            }
        }
        break;

    case MSC_CONF_SYS_DATA:
        //printf("WR:lba%d, off:%d, len:%d\n",lba, offset, bufsize);
        //FTL_WriteSector(FLASH_FTL_DATA_SECTOR + lba, 1, buffer);
        if(bufsize ==  2048)
        {/*
            if (last_lba != lba) {
                FTL_ReadSector(FLASH_FTL_DATA_SECTOR + lba, 1, MSCWRBuf);
                last_lba = lba;
            }
            uint8_t *addr = MSCWRBuf + offset;
            memcpy(addr, buffer, bufsize);
            if(offset == 1536)
                FTL_WriteSector(FLASH_FTL_DATA_SECTOR + lba, 1, MSCWRBuf);
                */
            //MTD_WritePhyPage(lba, buffer);
            
            #ifndef RAW_FLASH_ACCESS
            FTL_WriteSector(FLASH_FTL_DATA_SECTOR + lba, 1, buffer);
            #else
            MTD_WritePhyPage(lba, buffer);
            #endif
            
        }else{
            printf("RD:lba=%ld, off:%ld, len:%ld\n",lba, offset, bufsize);
        }
        break;

    default:
        break;
    }

    /*
      uint8_t *addr = MSCWRBuf + offset;
      memcpy(addr, buffer, bufsize);
      FTL_WriteSector(lba, MSCWRBuf);
    */

    // MTD_WritePhyPage(lba, buffer);

    // FTL_WriteSector(MSCpartStartSector + lba, 1, buffer);

    return bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize) {
    // read10 & write10 has their own callback and MUST not be handled here

    void const *response = NULL;
    uint16_t resplen = 0;

    // most scsi handled is input
    bool in_xfer = true;

    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        // Host is about to read/write etc ... better not to disconnect disk
        resplen = 0;
        break;

    default:
        // Set Sense = Invalid Command Operation
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

        // negative means error -> tinyusb could stall and/or response with failed status
        resplen = -1;
        break;
    }

    // return resplen must not larger than bufsize
    if (resplen > bufsize)
        resplen = bufsize;

    if (response && (resplen > 0)) {
        if (in_xfer) {
            memcpy(buffer, response, resplen);
        } else {
            // SCSI output
        }
    }

    return resplen;
}

#endif
