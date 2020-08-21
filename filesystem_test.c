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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "nand.h"
#include "timer.h"
#include "./filesystem/fatfs/ff.h"

unsigned int t;
extern unsigned int __DMA_NAND_PALLOAD_BUFFER;

unsigned int timers()
{
	t=0;
	timer_reset(0);
	return 1;
}


FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
	FRESULT res;
	DIR dir;
	UINT i;
	static FILINFO fno;


	res = f_opendir(&dir, path);                       /* Open the directory */
	printf("opendir:%d\n",res);
	if (res == FR_OK)
		{
			for (;;)
				{
					res = f_readdir(&dir, &fno);                   /* Read a directory item */
					if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
					if (fno.fattrib & AM_DIR)                      /* It is a directory */
						{
							i = strlen(path);
							sprintf(&path[i], "/%s", fno.fname);
							res = scan_files(path);                    /* Enter the directory */
							if (res != FR_OK) break;
							path[i] = 0;
						}
					else                                           /* It is a file. */
						{
							printf("%s/%s\n", path, fno.fname);
						}
				}
			f_closedir(&dir);
		}

	return res;
}


void fs_test_main()
{
	NAND_init();

	FATFS *fatfs, *fsinfo;
	fatfs = malloc(sizeof(FATFS));
	fsinfo = malloc(sizeof(FATFS));
	unsigned int res;
	DWORD clst;

	res = f_mount(fatfs,"0:",1);
	if(res == 0)
		{
			f_getfree("0:",&clst,&fsinfo);

			printf("mount C: ... OK\n");
			printf("File system type:");
			switch(fatfs->fs_type)
				{
				case FS_FAT12:
					printf("FAT12\n");
					break;
				case FS_FAT16:
					printf("FAT16\n");
					break;
				case FS_FAT32:
					printf("FAT32\n");
					break;
				case FS_EXFAT:
					printf("EXFAT\n");
					break;
				}
			printf("Total space(KB):%d\n",((fatfs->n_fatent-2) * fatfs->csize * 2048)/1024);
			printf("Free space(KB):%d\n",(fatfs->free_clst * fatfs->csize * 2048)/1024);
		}
	FIL* testfile;
	testfile = malloc(sizeof(FIL));

	unsigned char cont[32] = {0x33};
	//scan_files("/");


	res = f_open(testfile,"test2.txt",FA_READ);
	printf("Open file:%d\n",res);
	if(!res)
		{
			unsigned int i=0;
			printf("Open C:\\test2.txt size:%d.\n",f_size(testfile));
			f_gets(cont,f_size(testfile),testfile);
			while(i<f_size(testfile)){
				printf("%c",cont[i]);
				i++;
			}
			printf("\n");
		}

	t = 1;
	unsigned int page = 0;


	timer_init();
	timer_set(0,0,0x8,(unsigned int *)timers);
	timer_start(0,32*1000);
	while(t)
		{
			//mapping_read_lba_page(page,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),1);
			read_nand_pages(page,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
			page++;
		}
	printf("NAND Seq Read Speed:%d KB/s\n",page/2);

	t=1;
	page = 0; 
	timer_start(0,32*1000);
	while(t)
		{
			//mapping_read_lba_page(rand()%65535,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),1);
			read_nand_pages(rand()%65535,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
			page++;
		}
	printf("NAND Rand Read Speed:%d KB/s\n",page/2);
}



/*
	t = 1;
	unsigned int page = 0;


	timer_init();
	timer_set(0,0,0x8,(unsigned int *)timers);
	timer_start(0,32*1000);
	while(t)
		{
			read_nand_pages(page,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
			page++;
		}
	printf("NAND Seq Read Speed:%d KB/s\n",page/2);

	t=1;
	page = 0; 
	timer_start(0,32*1000);
	while(t)
		{
			read_nand_pages(rand()%65535,1,(unsigned int*)(&__DMA_NAND_PALLOAD_BUFFER),0);
			page++;
		}

*/







