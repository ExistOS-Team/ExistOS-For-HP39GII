#ifndef SERVICE_SWAP_H
#define SERVICE_SWAP_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "ff.h"
#include "queue.h"
#include "task.h"

#define NUM_L1_ENTRIES 4
#define NUM_VM 256

QueueHandle_t Q_MEM_Exception;

typedef struct MEM_Exception {
    TaskHandle_t ExceptionTaskHandle;
    unsigned int accessFaultAddress;
    unsigned int insFaultAddress;
    unsigned int FSR;
} MEM_Exception;

typedef struct SwapPageInfo {
    unsigned addr : 20;    // page在VM中的地址，前20位
    unsigned vmNum : 8;    // VM号
    unsigned resv : 2;     // reserved
    unsigned writable : 1; // 是否可写
    unsigned inUse : 1;    // 是否使用
} SwapPageInfo;            // swap中一个page的相关信息

typedef struct MemPageInfo {
    unsigned inUse : 1;      // 是否使用
    unsigned dirty : 1;      // 脏位
    unsigned associated : 1; // 是否是从swap读入的，如果是要写回相应地方
    union {
        unsigned swapPageIndex : 29; // 如果是从swap读入的，对应SwapPageInfo在swapPages中的index
        struct {
            unsigned addr : 20;    // 如果不是，记录page在VM中的地址前20位。用于在unassociated时，为unmap提供虚拟地址
            unsigned vmNum : 8;    // VM号
            unsigned writable : 1; // 是否可写
        };
    };
} MemPageInfo; // buffer中一个page的相关信息

typedef struct FileLoc {
    FIL *file;           // 与文件相关联
    unsigned int offset; // 在文件中的偏移量
} FileLoc;

typedef struct VmSection {
    char *start;            // 起始地址
    unsigned len : 31;      // 长度，不得超过2G
    unsigned writable : 1;  // 是否可写
    FileLoc *fileloc;       // 关联文件
    struct VmSection *next; // 链表下一项
} VmSection;                // 一个VM中一段内容

void switch_vm(unsigned int vmNum);

unsigned int pageFaultISR(TaskHandle_t ExceptionTaskHandle, unsigned int accessFaultAddress, unsigned int insFaultAddress, unsigned int FSR);

void vServiceSwap(void *pvParameters);
FIL *getSwapfileHandle();

#endif