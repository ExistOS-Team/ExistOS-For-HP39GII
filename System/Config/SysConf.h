#ifndef __SYSCONF_H__
#define __SYSCONF_H__

#define INDICATE_LEFT      (1 << 0)
#define INDICATE_RIGHT     (1 << 1)
#define INDICATE_A__Z      (1 << 2)
#define INDICATE_a__z      (1 << 3)
#define INDICATE_BUSY      (1 << 4)
#define INDICATE_TX        (1 << 5)
#define INDICATE_RX        (1 << 6)

#define FS_FATFS        0
#define FS_LITTLEFS     1
#define FS_TYPE         FS_FATFS

#define RAM_BASE            (0x02000000)
#define BASIC_RAM_SIZE      (256 * 1024)

//RAM_BASE


/*
malloc() priority:
   high:  Basic Memory   (200KB)
          Extern Memory  (256KB)
   low:   Swap Memory (default disable)

======================
|    Swap Memory     | 
|  (Swap with flash) |
|      (2 MB)        | (default disable)
|                    | 
|--EXTERN RAM END----|
|    Extern HEAP     | 
|      ~256KB        | (Enable memory compression and assuming the compression ratio of 0.3 ~ 0.4)
|                    |
|---BASIC RAM END----|
| Stack for RTOS (1K | 
|---BASIC HEAP END---|
|                    |  
|   Basic HEAP       | (KhiCAS and other app stack default allocated from here)
|    (~200 K)        | 
|--------------------|
|  .bss .data ...    |  
=======RAM_BASE======= (0x02000000)
   
*/


#define KhiCAS_STACK_SIZE   (8 * 1024)         //(Store in HEAP)


#define IRQ_STACK_ADDR      (RAM_BASE + BASIC_RAM_SIZE - 4)   //For RTOS
#define SWI_STACK_ADDR      (IRQ_STACK_ADDR  - 100 * 4)       //For RTOS
#define NORMAL_STACK_ADDR   (SWI_STACK_ADDR  - 100 * 4)       //For RTOS

#define BASIC_HEAP_END     (NORMAL_STACK_ADDR - 100 * 4)

#endif