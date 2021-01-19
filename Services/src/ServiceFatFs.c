#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ff.h"
#include "map.h"

/* System serive includes. */
#include "ServiceFatFs.h"
#include "ServiceFlashMap.h"

/* Library includes. */
#include "display.h"
#include "irq.h"
#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

unsigned int FSOK = 0;

FRESULT fr;
FATFS fs;
FATFS *fs1;

DWORD fre_clust, fre_sect, tot_sect;

unsigned int isfatFsInited() {

    return FSOK;
}

void vServiceFatfs(void *pvParameters) {

    while (!isFlashMapInited()) {
        vTaskDelay(5);
    }

    printf("Mounting Flash ...\n");
    fr = f_mount(&fs, "/", 1);
    fs1 = &fs;

    if (fr == 0) {
        FSOK = 1;

        f_getfree("/", &fre_clust, &fs1);
        tot_sect = (fs.n_fatent - 2) * fs.csize;
        fre_sect = fre_clust * fs.csize;
        printf("\tfre_clust:%d\n", fre_clust);
        printf("%10lu KiB total drive space.\n%10lu KiB available.\n", tot_sect * 2, fre_sect * 2);

    } else {

        printf("Mount fail: %d\n", fr);
    }

    vTaskDelete(NULL);
    for (;;) {
        vTaskSuspend(NULL);
    }
}