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
#define BASIC_RAM_SIZE      (160 * 1024)
#define BASIC_HEAP_END      (RAM_BASE + BASIC_RAM_SIZE)


/*
malloc() priority:
   high:  Basic Memory   (~400 KB)
   low:   Swap Memory    (2~3MB, default disable)

======================
|    Swap Memory     | 
|  (Swap with flash) |
|      (2 MB)        | (default disable)
|                    | 
|---BASIC RAM END----|
|    Extern HEAP     | 
|      ~240KB        | (Enable memory compression and assuming the compression ratio of 0.3 ~ 0.4)
|                    |  
|   Basic HEAP       | (KhiCAS and other app stack default allocated from here)
|    (~160 K)        | 
|--------------------|
|  .sys_stack        |
|  .bss .data ...    |  
=======RAM_BASE======= (0x02000000)
   
*/


#define KhiCAS_STACK_SIZE   (24 * 1024)         //(Store in HEAP by malloc();)


#endif