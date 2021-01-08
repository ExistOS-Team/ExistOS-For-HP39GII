#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdc_console.h"
#include "ff.h"
#include "init.h"
#include "keyboard.h"
#include "memory_map.h"
#include "mmu.h"
#include "regsdigctl.h"
#include "rtc.h"
#include "startup_info.h"

#include "ServiceFatFs.h"
#include "ServiceFlashMap.h"
#include "ServiceGraphic.h"
#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"
#include "ServiceSwap.h"
#include "ServiceUSBDevice.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "map.h"
#include "queue.h"
#include "task.h"
#include "uart_debug.h"

#define SWAP_PAGES (swapSizeMB * PAGES_PER_MIB)

// extern int heapBytesRemaining;

SwapPageInfo *swapPages;
MemPageInfo *memPages;

void *__pageBuffer;
unsigned char *pageBuffer;
unsigned int pageBufferPhy;

void *__l2_pgbuff_tabs;
unsigned char *l2_pgbuff_tabs;

VmSection **vmInfo;

unsigned int curL1Entry = 0;

unsigned int curMemPage = 0;

unsigned char curVmNum = 0;

FIL swapfileHandle;

FIL *getSwapfileHandle() {
    return &swapfileHandle;
}

unsigned int swap_file_inited = 0;

extern unsigned int *tlb_base;

void *mem_align_up(void *p, unsigned int u) {
    if ((unsigned int)p % u != 0) {
        return p;
    } else {
        return ((unsigned int)p / u + 1) * u;
    }
}

char inWhichL1Entry(unsigned int addr) {
    for (unsigned char i = 0; i < NUM_L1_ENTRIES; i++) {
        if (addr >= BF_RDn(DIGCTL_MPTEn_LOC, i, LOC) && addr < BF_RDn(DIGCTL_MPTEn_LOC, i, LOC) + 1024 * 1024) {
            return i;
        }
    }
    return -1;
}

void switch_vm(unsigned int vmNum) {
    memset(l2_pgbuff_tabs, 0, NUM_L1_ENTRIES * PAGE_TABLE_SIZE); // L2 table全部清零，等待重建
    curVmNum = vmNum;
}

unsigned int pageFaultISR(TaskHandle_t ExceptionTaskHandle, unsigned int accessFaultAddress,
                          unsigned int insFaultAddress, unsigned int FSR) {
    if (accessFaultAddress >= 0xc0000000) { // 不为内核段服务
        return 1;
    }
    cdc_printf("accessFaultAddress = 0x%x, FRS = 0x%x\n", accessFaultAddress, FSR);
    switch (FSR & 0b1101) {
    case 0b0101: // Translation fault，关注缺页异常

        VmSection *vs = vmInfo[curVmNum];
        while (vs != NULL) {
            if (vs->start <= accessFaultAddress && accessFaultAddress < vs->start + vs->len) {
                cdc_printf("In 0x%x, 0x%x\n", vs->start, vs->len);
                goto next;
            }
            vs = vs->next;
        }
        cdc_printf("访问未定义内存\n");
        return 1; // 访问未定义内存

    next:
        char entry = inWhichL1Entry(accessFaultAddress);
        if (entry < 0) {                                                               // 准备挪用一个L1 entry
            memset(l2_pgbuff_tabs + curL1Entry * PAGE_TABLE_SIZE, 0, PAGE_TABLE_SIZE); // 对应L2 table清零

            unsigned int sectionAddr = (unsigned int)accessFaultAddress >> 20;
            BF_WRn(DIGCTL_MPTEn_LOC, curL1Entry, LOC, sectionAddr);
            MMU_MAP_COARSE_RAM(pageBufferPhy, sectionAddr << 20);
            cdc_printf("L1 entry %d moved to 0x%x\n", pageBufferPhy);
            curL1Entry++; // 步进
            if (curL1Entry >= NUM_L1_ENTRIES) {
                curL1Entry -= NUM_L1_ENTRIES;
            };
        }

        unsigned int pageAddr = accessFaultAddress >> 12;

        // 复用
        for (unsigned int index = 0; index < BUFFER_PAGES; index++) {
            if (memPages[index].associated) {
                if (swapPages[memPages[index].swapPageIndex].addr != pageAddr || swapPages[memPages[index].swapPageIndex].vmNum != curVmNum) {
                    continue;
                }
            } else {
                if (memPages[index].addr != pageAddr || memPages[index].vmNum != curVmNum) {
                    continue;
                }
            }
            if (memPages[index].dirty) {
                MMU_MAP_SMALL_PAGE_CACHED_WITH_L2(pageBufferPhy + PAGE_SIZE * index, pageAddr << 12,
                                                  l2_pgbuff_tabs + entry * PAGE_TABLE_SIZE);

            } else {
                MMU_MAP_SMALL_PAGE_CACHED_RO_WITH_L2(pageBufferPhy + PAGE_SIZE * index, pageAddr << 12,
                                                     l2_pgbuff_tabs + entry * PAGE_TABLE_SIZE);
            }
            flush_cache();
            return 0; // 如果pageBuffer里已经有现成的了，就直接配好L2 table，return
        }

    skip:
        // 换出
        if (memPages[curMemPage].inUse && memPages[curMemPage].dirty) { // 脏了就写入swap，否则直接丢弃
            unsigned int memPageAddr =
                (memPages[curMemPage].associated ? swapPages[memPages[curMemPage].swapPageIndex].addr : memPages[curMemPage].addr) << 12;
            MMU_MAP_SMALL_PAGE_UNMAP_WITH_L2(memPageAddr,
                                             l2_pgbuff_tabs + inWhichL1Entry(memPageAddr) * PAGE_TABLE_SIZE); // 把牺牲页unmap掉

            FRESULT fr;

            if (memPages[curMemPage].associated) {
                fr = f_lseek(&swapfileHandle, memPages[curMemPage].swapPageIndex * PAGE_SIZE); // 如果有对应swap页面就写入
            } else {
                unsigned int index = 0;
                while (swapPages[index].inUse) { // 否则寻找空闲swap页面
                    index++;
                    if (index >= SWAP_PAGES) {
                        printf("Swapfile used up!");
                        return 1;
                    }
                }

                swapPages[index].inUse = 1; // 标记swap页面
                swapPages[index].addr = memPages[curMemPage].addr;
                swapPages[index].vmNum = memPages[curMemPage].vmNum;
                swapPages[index].writable = memPages[curMemPage].writable;

                fr = f_lseek(&swapfileHandle, index * PAGE_SIZE);
            }
            if (fr != 0) {
                printf("Lseek in swapfile failed!");
                return 1;
            }

            unsigned int bw;
            fr = f_write(&swapfileHandle, pageBuffer[curMemPage], PAGE_SIZE, &bw);
            if (fr != 0) {
                printf("Write to swapfile failed!");
                return 1;
            }
        }

        // L2 table里新建项
        // MMU_MAP_SMALL_PAGE_CACHED((unsigned int)pageBuffer + PAGE_SIZE * curMemPage, pageAddr << 12);
        MMU_MAP_SMALL_PAGE_CACHED_RO_WITH_L2(pageBufferPhy + PAGE_SIZE * curMemPage, pageAddr << 12,
                                             l2_pgbuff_tabs + entry * PAGE_TABLE_SIZE);

        // 换入
        unsigned int index = 0;
        while (swapPages[index].addr != pageAddr || swapPages[index].vmNum != curVmNum) { // 寻找相应swap页面，可优化成Tree
            index++;
            if (index >= SWAP_PAGES) {
                /* memset(pageBuffer + curMemPage * PAGE_SIZE, 0, PAGE_SIZE); // 找不到就开全0新页
                memPages[curMemPage].associated = 0;
                memPages[curMemPage].addr = pageAddr;
                memPages[curMemPage].vmNum = curVmNum;
                memPages[curMemPage].writable = 1; // TODO: 先默认可写 */

                memPages[curMemPage].associated = 0;
                memPages[curMemPage].addr = pageAddr;
                memPages[curMemPage].vmNum = curVmNum;
                memPages[curMemPage].writable = vs->writable;
                if (vs->fileloc == NULL) {
                    memset(pageBuffer + curMemPage * PAGE_SIZE, 0, PAGE_SIZE);
                } else {
                    FRESULT fr = f_lseek(vs->fileloc->file,
                                         (pageAddr << 12) - (unsigned int)vs->start + vs->fileloc->offset);
                    if (fr != 0) {
                        printf("Lseek in file failed!");
                        return 1;
                    }
                    unsigned int br;
                    fr = f_read(vs->fileloc->file, pageBuffer + curMemPage * PAGE_SIZE, PAGE_SIZE, &br);
                    if (fr != 0) {
                        printf("Read from file failed!");
                        return 1;
                    }
                    if (br < PAGE_SIZE) {
                        memset(pageBuffer + curMemPage * PAGE_SIZE + br, 0, PAGE_SIZE - br);
                    }
                }
                goto total;
            }
        }
        FRESULT fr = f_lseek(&swapfileHandle, index * PAGE_SIZE);
        if (fr != 0) {
            printf("Lseek in swapfile failed!");
            return 1;
        }
        unsigned int br;
        fr = f_read(&swapfileHandle, pageBuffer + curMemPage * PAGE_SIZE, PAGE_SIZE, &br);
        if (fr != 0) {
            printf("Read from swapfile failed!");
            return 1;
        }
        memPages[curMemPage].associated = 1; // 相关联
        memPages[curMemPage].swapPageIndex = index;
    total:
        memPages[curMemPage].inUse = 1;
        memPages[curMemPage].dirty = 0;
        curMemPage++; // 步进
        if (curMemPage >= BUFFER_PAGES) {
            curMemPage -= BUFFER_PAGES;
        };
        flush_cache();
        return 0;

    case 0b1101: // Permission fault，关注写只读异常
        unsigned int pageAddr = accessFaultAddress >> 12;
        for (unsigned int index = 0; index < BUFFER_PAGES; index++) {
            if (memPages[index].associated) {
                if (swapPages[memPages[index].swapPageIndex].addr != pageAddr || swapPages[memPages[index].swapPageIndex].vmNum != curVmNum) {
                    continue;
                }
                if (!swapPages[memPages[index].swapPageIndex].writable) {
                    return 1;
                }
            } else {
                if (memPages[index].addr != pageAddr || memPages[index].vmNum != curVmNum) {
                    continue;
                }
                if (!memPages[index].writable) {
                    return 1;
                }
            }
            unsigned char entry = inWhichL1Entry(accessFaultAddress);
            if (entry < 0) {
                return 1;
            }
            memPages[index].dirty = 1;
            MMU_MAP_SMALL_PAGE_CACHED_WITH_L2(pageBufferPhy + PAGE_SIZE * index, pageAddr << 12,
                                              l2_pgbuff_tabs + entry * PAGE_TABLE_SIZE);
            flush_cache();
            return 0;
        }
        return 1;

    default:
        return 1;
    }
}

char vmLoadFile(unsigned char vmNum, char *start, unsigned int len, FIL *file, unsigned int offset, unsigned char writable) {
    if ((unsigned int)start & 0xfff != 0 || len & 0xfff != 0) {
        return -1;
    }
    VmSection *p = vmInfo[vmNum];
    while (p != NULL) {
        if ((start <= p->start && p->start < start + len) || (p->start <= start && start < p->start + p->len)) {
            return -1;
        }
        p = p->next;
    }
    VmSection *vs = pvPortMalloc(sizeof(VmSection));
    if (vs == NULL) {
        printf("Create VmSection failed!");
        return -1;
    }
    vs->start = start;
    vs->len = len;
    vs->writable = writable;
    vs->next = vmInfo[vmNum];
    if (file == NULL) {
        vs->fileloc = NULL;
    } else {
        FileLoc *fl = pvPortMalloc(sizeof(FileLoc));
        if (fl == NULL) {
            printf("Create FileLoc failed!");
            pvPortFree(vs);
            return -1;
        }
        fl->file = file;
        fl->offset = offset;
        vs->fileloc = fl;
    }
    vmInfo[vmNum] = vs;
    return 0;
}

// char vmUnload(unsigned char vmNum, char *start) {
//     VmSection *p = vmInfo[vmNum];
//     while (p != NULL) {
//         if (p->start == start) {
//             if (p->fileloc != NULL) {
//                 pvPortFree(p->fileloc);
//             }
//             VmSection *next = p->next;
//             pvPortFree(p);
//             p = next;
//             return 0;
//         }
//         p = p->next;
//     }
//     return -1;
// }

void vServiceException(void *pvParameters) {

    Q_MEM_Exception = xQueueCreate(128, sizeof(MEM_Exception));
    MEM_Exception e;

    for (;;) {
        if (xQueueReceive(Q_MEM_Exception, (&e), (TickType_t)portMAX_DELAY) == pdTRUE) {
        }

        //vTaskDelay(100);
    }
}

void vServiceSwap(void *pvParameters) {

    xTaskCreate(vServiceException, "MEM Exception SVC", configMINIMAL_STACK_SIZE, NULL, 4, NULL);

    swap_file_inited = 0;
    if (swapSizeMB == 0) {
        vTaskDelete(NULL);
    }

    FRESULT fr = f_open(&swapfileHandle, "Pagefile", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (fr != 0) {
        printf("Create pagefile failed, %d\n", fr);
        vTaskDelete(NULL);
    }

    fr = f_expand(&swapfileHandle, swapSizeMB * 1024 * 1024, 1);
    if (fr != 0) {
        printf("Allocates %d MB pagefile failed, %d\n", swapSizeMB, fr);
        vTaskDelete(NULL);
    }

    f_sync(&swapfileHandle);
    flashSyncNow();

    unsigned int size = SWAP_PAGES * sizeof(SwapPageInfo);
    swapPages = pvPortMalloc(size);
    if (swapPages == NULL) {
        printf("Allocate %d Byte swapPageInfo failed\n", size);
        vTaskDelete(NULL);
    }
    memset(swapPages, 0, size);

    size = BUFFER_PAGES * sizeof(MemPageInfo);
    memPages = pvPortMalloc(size);
    if (memPages == NULL) {
        printf("Allocate %d Byte memPageInfo failed\n", size);
        vTaskDelete(NULL);
    }
    memset(memPages, 0, size);

    size = NUM_VM * sizeof(VmSection *);
    vmInfo = pvPortMalloc(size);
    if (vmInfo == NULL) {
        printf("Allocate %d Byte vmInfo failed\n", size);
        vTaskDelete(NULL);
    }
    memset(vmInfo, 0, size);

    __pageBuffer = pvPortMalloc((BUFFER_PAGES + 1) * PAGE_SIZE);
    if (__pageBuffer == NULL) {
        printf("Allocate %d Byte __pageBuffer failed\n", (BUFFER_PAGES + 1) * PAGE_SIZE);
        vTaskDelete(NULL);
    }
    pageBuffer = mem_align_up(__pageBuffer, PAGE_SIZE);
    // memset(pageBuffer, 0, BUFFER_PAGES * PAGE_SIZE); 不必
    pageBufferPhy = VIR_TO_PHY_ADDR(pageBuffer);

    __l2_pgbuff_tabs = pvPortMalloc(PAGE_TABLE_SIZE * (NUM_L1_ENTRIES + 1));
    if (__l2_pgbuff_tabs == NULL) {
        printf("Allocate %d Byte __l2_pgbuff_tabs failed\n", PAGE_TABLE_SIZE * 2);
        vTaskDelete(NULL);
    }
    l2_pgbuff_tabs = mem_align_up(__l2_pgbuff_tabs, PAGE_TABLE_SIZE);
    memset(l2_pgbuff_tabs, 0, PAGE_TABLE_SIZE);

    printf("page buffer addr:%08x , %08x\n", pageBuffer, l2_pgbuff_tabs);

    swap_file_inited = 1;
    vTaskDelete(NULL);
}