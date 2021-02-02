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
#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flash_mapping.h"
#include "flash_mapping_const.h"
#include "raw_flash.h"
#include "uart_debug.h"

#define SUPPOSE_MAX_REGIONS 10
#define PAGES_PER_BLOCK 64

//1 LBA  ->  64 page

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define swap_16(a) ((a >> 8) | ((a & 0xFF) << 8))

extern unsigned int __DMA_NAND_PALLOAD_BUFFER;
extern unsigned int __DMA_NAND_AUXILIARY_BUFFER;

unsigned int dataDriverStartBlock;
unsigned int first_lba_offset_block;
unsigned int first_lba_block_entry_offset;

BootBlockStruct_t bootBlockInfo;
NandConfigBlockInfo_t nandConfigBlockInfo;
NandConfigBlockRegionInfo_t RegionsInfo[SUPPOSE_MAX_REGIONS];

MetadataFields *metadata;
unsigned int *xmap_buffer;

int search_bootBlockInfo() {
    BootBlockStruct_t *__bootBlockInfo;
    for (int i = 0; i < 0x2000; i += 0x100) {
        read_nand_pages(i, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        __bootBlockInfo = ((BootBlockStruct_t *)(&__DMA_NAND_PALLOAD_BUFFER));
        if (__bootBlockInfo->m_u32FingerPrint1 == STMP_FINGERPRINT) {
            memcpy(&bootBlockInfo, __bootBlockInfo, sizeof(bootBlockInfo));
            return i;
        }
    }
    return -1;
}

int search_nandConfigBlockInfo() {
    NandConfigBlockInfo_t *__nandConfigBlockInfo;
    for (int i = 1; i < 0x1000; i += 0x100) {
        read_nand_pages(i, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        __nandConfigBlockInfo = ((NandConfigBlockInfo_t *)(&__DMA_NAND_PALLOAD_BUFFER));
        if ((__nandConfigBlockInfo->iMagicCookie == NAND_CONFIG_BLOCK_MAGIC_COOKIE) &&
            (__nandConfigBlockInfo->iVersionNum == NAND_CONFIG_BLOCK_VERSION)) {
            memcpy(&nandConfigBlockInfo, __nandConfigBlockInfo, sizeof(nandConfigBlockInfo));
            for (int i = 0; i < min(nandConfigBlockInfo.iNumRegions, SUPPOSE_MAX_REGIONS); i++) {
                memcpy(&RegionsInfo[i],
                       (&__DMA_NAND_PALLOAD_BUFFER) + sizeof(NandConfigBlockInfo_t) / 4 + i * (sizeof(NandConfigBlockRegionInfo_t) / 4),
                       sizeof(NandConfigBlockRegionInfo_t));
            }
            return i;
        }
    }
    return -1;
}

unsigned int metadata_get_signature(MetadataFields *m_fields) {
    return ((m_fields->tag0 << 24) | (m_fields->tag1 << 16) | (m_fields->tag2 << 8) | (m_fields->tag3));
}

unsigned int metadata_get_Lba(MetadataFields *m_fields) {
    return m_fields->lba0 | (m_fields->lba1 << 16);
}

unsigned int metadata_get_Lsi(MetadataFields *m_fields) {
    return m_fields->lsi;
}

unsigned int metadata_get_Flag(MetadataFields *m_fields) {
    return m_fields->flags;
}

unsigned int metadata_get_BlockNumber(MetadataFields *m_fields) {
    return m_fields->blockNumber;
}

unsigned int metadata_isMarkedBad(MetadataFields *m_fields) {
    return m_fields->blockStatus != 0xff;
}

unsigned int metadata_isErased(MetadataFields *m_fields) {
    return ((uint32_t *)m_fields)[0] == 0xffffffff && ((uint32_t *)m_fields)[1] == 0xffffffff && m_fields->flags == 0xff && m_fields->reserved == 0xff;
}

unsigned short xmap_buffer_get_entry(unsigned int block_entry, unsigned int block_offset) {
    return ((xmap_buffer[6 + (block_entry + block_offset) / 2] >> (((block_entry + block_offset) % 2) * 16)) & 0xFFFF);
}

int search_phys_maps() {
    for (unsigned int pageStart = dataDriverStartBlock * PAGES_PER_BLOCK; pageStart < 0x8000; pageStart += PAGES_PER_BLOCK) {
        read_nand_pages(pageStart, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        metadata = (MetadataFields *)(&__DMA_NAND_AUXILIARY_BUFFER);
        if ((metadata_get_signature(metadata) == PHYS_STRING_PAGE1) &&
            (*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == kNandMapHeaderSignature) &&
            *((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER) + 1) == kNandPhysMapSignature) {
#ifdef DEBUG
            printf("phys_map at page:%x\n", pageStart);
#endif
            return pageStart;
        }
    }
    return -1;
}

int search_zone_maps(unsigned phys_page) {

    for (int pageStart = phys_page; pageStart > dataDriverStartBlock * PAGES_PER_BLOCK; pageStart--) {
        read_nand_pages(pageStart, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        metadata = (MetadataFields *)(&__DMA_NAND_AUXILIARY_BUFFER);
        if ((metadata_get_signature(metadata) == LBA_STRING_PAGE1) &&
            (*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == kNandMapHeaderSignature) &&
            *((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER) + 1) == kNandZoneMapSignature) {
#ifdef DEBUG
            printf("zone_maps at page:%x\n", pageStart);
#endif
            return pageStart;
        }
    }

    return -1;
}

int search_first_lba_block() {
    for (unsigned int pageStart = dataDriverStartBlock * PAGES_PER_BLOCK; pageStart < 0xFFFF; pageStart += PAGES_PER_BLOCK) {
        read_nand_pages(pageStart, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        if ((*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x211DEBFA) &&
            *((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER) + 127) == 0xAA550000) {
#ifdef DEBUG
            printf("first_lba_block at page:%x\n", pageStart);
#endif
            return pageStart;
        }
    }
    return -1;
}

unsigned int search_page_offset_in_block(unsigned int block, unsigned int pageOffset) {
    for (unsigned int pageStart = block * PAGES_PER_BLOCK; pageStart < (block + 1) * PAGES_PER_BLOCK; pageStart++) {
        read_nand_pages(pageStart, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        metadata = (MetadataFields *)(&__DMA_NAND_AUXILIARY_BUFFER);
        if (metadata_get_Lsi(metadata) == pageOffset) {
            return (pageStart - block * PAGES_PER_BLOCK);
        }
    }
#ifdef DEBUG
//printf("offset not found.\n");
#endif
    return 0;
}

void mapping_read_lba_page(unsigned int lba_start_page, unsigned int *buffer, unsigned int pages) {
    unsigned int tr_block, entryBlock;
    for (unsigned int startPage = lba_start_page; startPage < lba_start_page + pages; startPage++) {
        tr_block = startPage / PAGES_PER_BLOCK;
        if (xmap_buffer == NULL) {
            entryBlock = first_lba_offset_block; //+ tr_block;
        } else {
            entryBlock = xmap_buffer_get_entry(tr_block, first_lba_block_entry_offset);
            //printf("entryBlock:%x\n",entryBlock);
        }

        if (entryBlock == 0xFFFF) {
            memset(buffer, 0, pages * 2048);
            return;
        } else {
            //printf("RD page:%x\n",entryBlock*PAGES_PER_BLOCK+((startPage)%PAGES_PER_BLOCK));
            read_nand_pages(entryBlock * PAGES_PER_BLOCK + search_page_offset_in_block(entryBlock, (startPage) % PAGES_PER_BLOCK), 1, buffer, 0);
        }
    }
}

void debug_dump() {
    volatile unsigned int cdcd = 0;
    for (int pageStart = 0; pageStart < 0xFFFF; pageStart++) {
        read_nand_pages(pageStart, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        metadata = (MetadataFields *)(&__DMA_NAND_AUXILIARY_BUFFER);

        if ((*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == kNandMapHeaderSignature))
            cdcd = 1;
        if ((*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x68656164))
            cdcd = 1;
        if ((*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == 0x48454144))
            cdcd = 1;

        if (metadata_get_signature(metadata) == LBA_STRING_PAGE1)
            cdcd = 1;
        if (metadata_get_signature(metadata) == PHYS_STRING_PAGE1)
            cdcd = 1;
        //if(metadata_get_Lsi(metadata) == 0)cdcd=1;
        //{
        //	cdcd=1;

        //}

        if (cdcd) {
            uartdbg_printf("PAGE %x, BLOCK %x:\n", pageStart, pageStart / PAGES_PER_BLOCK);
            uartdbg_printf("LSI=%x,LBA=%x,FLAG=%x\n", metadata_get_Lsi(metadata), metadata_get_Lba(metadata), metadata_get_Flag(metadata));

            if ((*((unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER)) == kNandMapHeaderSignature))
                if (metadata_get_signature(metadata) == LBA_STRING_PAGE1) {
                    uartdbg_printf("LBA_STRING_PAGE1\n");
                } else {
                    uartdbg_printf("PHYS_STRING_PAGE1\n");
                }

            for (unsigned int i = (unsigned int)&__DMA_NAND_AUXILIARY_BUFFER; i < (unsigned int)(&__DMA_NAND_AUXILIARY_BUFFER + (20 / 4)); i++) {
                uartdbg_printf("%x ", *((unsigned char *)i));
            }

            uartdbg_printf("\n=======================\n");
            for (unsigned int i = (unsigned int)&__DMA_NAND_PALLOAD_BUFFER; i < (unsigned int)(&__DMA_NAND_PALLOAD_BUFFER + (0x800) / 4); i += 4) {
                uartdbg_printf("%x ", *((unsigned int *)i));
            }

            uartdbg_putc('\n');
            cdcd--;
        }
    }
}

void dump_phy_page(unsigned int page) {
    read_nand_pages(page, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
    uartdbg_printf("PAGE %x BLOCK:%x:\n", page, page / PAGES_PER_BLOCK);
    uartdbg_printf("LSI=%x,LBA=%x,FLAG=%x\n", metadata_get_Lsi(metadata), metadata_get_Lba(metadata), metadata_get_Flag(metadata));
    for (unsigned int i = (unsigned int)&__DMA_NAND_PALLOAD_BUFFER; i < (unsigned int)(&__DMA_NAND_PALLOAD_BUFFER + (0x800) / 4); i += 4) {
        uartdbg_printf("%x ", *((unsigned int *)i));
    }
    uartdbg_putc('\n');
}

void dump_phy_block(unsigned int block) {

    for (unsigned int k = block * PAGES_PER_BLOCK; k < ((block + 1) * PAGES_PER_BLOCK); k++) {
        read_nand_pages(k * PAGES_PER_BLOCK, 1, (unsigned int *)(&__DMA_NAND_PALLOAD_BUFFER), 0);
        metadata = (MetadataFields *)(&__DMA_NAND_AUXILIARY_BUFFER);
        uartdbg_printf("PAGE %x BLOCK %x :\n", k, k / PAGES_PER_BLOCK);
        uartdbg_printf("LSI=%x,LBA=%x,FLAG=%x\n", metadata_get_Lsi(metadata), metadata_get_Lba(metadata), metadata_get_Flag(metadata));
        for (unsigned int i = (unsigned int)&__DMA_NAND_PALLOAD_BUFFER; i < (unsigned int)(&__DMA_NAND_PALLOAD_BUFFER + (0x800) / 4); i += 4) {
            uartdbg_printf("%x ", *((unsigned int *)i));
        }
        uartdbg_putc('\n');
    }
}

int Flash_mapping_init() {
    int res;
    xmap_buffer = NULL;

    res = search_bootBlockInfo();
#ifdef DEBUG
    printf("bootBlockInfo at:%x\n", res);
    printf("firmware_startingNAND:%x\n", bootBlockInfo.LDLB_Block2.m_u32Firmware_startingNAND);
#endif
    res = search_nandConfigBlockInfo();
    printf("nandConfigBlockInfo at:%x\n", res);

    if (res == -1)
        return res;

    for (int i = 0; i < nandConfigBlockInfo.iNumRegions; i++) {
        if (RegionsInfo[i].eDriveType == kDriveTypeData) {
            dataDriverStartBlock = RegionsInfo[i].iStartBlock;
        }
#ifdef DEBUG
        printf("Region %d:DriveType %d, StartBlock %x\n", i, RegionsInfo[i].eDriveType, RegionsInfo[i].iStartBlock);
//dump_phy_block(RegionsInfo[i].iStartBlock);
#endif
    }

    res = search_phys_maps();
    if (res == -1) { //不存在映射表，数据区中只存在一个块
        res = search_first_lba_block();
        if (res != -1) {
            first_lba_offset_block = res / PAGES_PER_BLOCK;
        } else {
//ERROR
#ifdef DEBUG
            printf("mapping init fail. 1\n");
#endif
            return -1;
        }
    } else { //存在映射表
        res = search_zone_maps(res);
        if (res != -1) {
            xmap_buffer = malloc(2048);
            if (xmap_buffer == NULL) {
#ifdef DEBUG
                printf("out of memory\n");
#endif
                return -2;
            }
            memcpy(xmap_buffer, (char *)&__DMA_NAND_PALLOAD_BUFFER, 2048); //映射表缓存

            //dump_phy_page(res);

            for (unsigned int i = 0; i < 0x3F4; i++) {
                if (xmap_buffer_get_entry(i, 0) != 0xFFFF) {
                    first_lba_offset_block = xmap_buffer_get_entry(i, 0);
                    first_lba_block_entry_offset = i;
                    break;
                }
            }
        } else {
//ERROR
#ifdef DEBUG
            printf("mapping init fail. 2\n");
#endif
            return -1;
        }
    }

#ifdef DEBUG
    /*		printf("first_lba_offset_block:%x\n",first_lba_offset_block);
		printf("first_lba_block_entry_offset:%x\n",first_lba_block_entry_offset);
		printf("test entry:%x\n",xmap_buffer_get_entry(16,first_lba_block_entry_offset));
		dump_phy_block(xmap_buffer_get_entry(16,first_lba_block_entry_offset));
	uartdbg_putc('\n');
	for(unsigned int i = (unsigned int)xmap_buffer; i < (unsigned int)(xmap_buffer + (0x800)/4); i+=4)
	{
		uartdbg_printf("%x ",*((unsigned int *)i));
	}
	uartdbg_putc('\n');
	//dump_phy_block(0xca);
	uartdbg_putc('\n');*/

//dump_phy_block(xmap_buffer_get_entry(0,first_lba_block_entry_offset));
#endif
    //debug_dump();
    //dump_phy_block(0x3F8);
    /*
	for(unsigned int pageStart = 63; pageStart < 65 ;pageStart++){
		mapping_read_lba_page(pageStart,(unsigned int *)&__DMA_NAND_PALLOAD_BUFFER,1);
		metadata = (MetadataFields *)(&__DMA_NAND_AUXILIARY_BUFFER);
		uartdbg_printf("LSI=%x,LBA=%x\n",metadata_get_Lsi(metadata),metadata_get_Lba(metadata));
		uartdbg_printf("PAGE %x, BLOCK %x:\n",pageStart,pageStart/PAGES_PER_BLOCK);
		for(unsigned int i = (unsigned int )&__DMA_NAND_PALLOAD_BUFFER; i < (unsigned int)(&__DMA_NAND_PALLOAD_BUFFER + (0x800)/4); i+=4)
							{
								uartdbg_printf("%x ",*((unsigned int *)i));
							}
	}
	*/

    return 0;
}
