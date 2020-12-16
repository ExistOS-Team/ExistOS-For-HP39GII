#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "map.h"
#include "ff.h"

/* System serive includes. */
#include "ServiceFatFs.h"

/* Library includes. */
#include "regsuartdbg.h"
#include "irq.h"
#include "display.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

FRESULT fr;
FATFS fs;
FIL fil;

DWORD fre_clust, fre_sect, tot_sect;
BYTE work[FF_MAX_SS];
	
void vServiceFatfs( void *pvParameters )
{
	
	vTaskDelay(500);

	
	printf("Mounting FLash ...\n");
	fr = f_mount(&fs, "0:", 1);
	printf("mount result: %d\n", fr);
	if(fr == 0){
		f_getfree("0:", &fre_clust, &fs);
		tot_sect = (fs.n_fatent - 2) * fs.csize;
		fre_sect = fre_clust * fs.csize;
		printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect , fre_sect );
		
		printf("create file :%d\n",fr);
		fr = f_open(&fil, "0:test.txt", FA_CREATE_ALWAYS);
		f_close(&fil);
		printf("fr: %d\n",fr);
		
		f_close(&fil);
	
	}
	
	
	
	for(;;){
		vTaskSuspend(NULL);
	}
	
	
}