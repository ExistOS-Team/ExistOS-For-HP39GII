
#include "FreeRTOSConfig.h"

#define VERSION     "0.2.0"

#define MEMORY_BASE     (0)
#define MEMORY_SIZE     (512*1024)

#define SYS_LOG_BUFSIZE    (4096)
#define CDC_PATH_LOADER  0
#define CDC_PATH_SYS     1
#define CDC_PATH_EDB     2
#define CDC_PATH_SCRCAP  3


#define ABT_STACK_ADDR      (MEMORY_BASE + MEMORY_SIZE - 4)
#define UND_STACK_ADDR      (ABT_STACK_ADDR - 0x400)
#define FIQ_STACK_ADDR      (UND_STACK_ADDR - 0x200)
#define IRQ_STACK_ADDR      (FIQ_STACK_ADDR - 0x200)
#define SVC_STACK_ADDR      (IRQ_STACK_ADDR - 0x400)
#define SYS_STACK_ADDR      (SVC_STACK_ADDR - 0x400)


#define HEAP_END        (SYS_STACK_ADDR - 0x200)

// DATA   SYS   SWAP
//#define DISK_PARTITION      {80, 15, 5, 0}

#define USE_TINY_PAGE       (1)
#define VMRAM_USE_FTL       (1)

#define SEG_SIZE            1048576


#if VMRAM_USE_FTL
    #if USE_TINY_PAGE
        #define NUM_CACHEPAGE             ( 273 ) // 273 * 1 = 273 KB
    #else
        #define NUM_CACHEPAGE             ( 79 ) // 79 * 4 = 316 KB
    #endif
#else
    #define NUM_CACHEPAGE             ( 32 )
    #define VM_RAM_SIZE_NONE_FTL      ( 168 * 1024 )
#endif




#if USE_TINY_PAGE
    #define PAGE_SIZE           1024
#else
    #define PAGE_SIZE           4096
#endif

#define PAGES_SWAPFILE      (SIZE_SWAPFILE_MB * 1048576 / PAGE_SIZE)

#define VM_ROM_BASE         (0x00100000)
#define VM_ROM_SIZE         (1048576 * 4)
#define VM_ROM_SEG          (VM_ROM_BASE >> 20)
#define VM_ROM_NUM_SEG      (VM_ROM_SIZE / SEG_SIZE)

#define VM_RAM_BASE         (0x02000000)
#define VM_RAM_SIZE         (1048576 * 6)
#define VM_RAM_SEG          (VM_ROM_BASE >> 20)
#define VM_RAM_NUM_SEG      (VM_RAM_SIZE / SEG_SIZE)

#define VM_CACHE_ENABLE     (true)
#define VM_BUFFER_ENABLE    (true)

#define TOTAL_VM_SEG        ((VM_ROM_SIZE + VM_RAM_SIZE) / SEG_SIZE)


#define SIZE_SWAPAREA_MB    (VM_RAM_SIZE / 1048576)

#define CPU_DIVIDE_NORMAL       1
#define CPU_DIVIDE_IDLE_INTIAL  12

#define FLASH_LOADER_BLOCK      22
#define FLASH_CONFIG_BLOCK      23
#define FLASH_SYSTEM_BLOCK      31
#define FLASH_DATA_BLOCK        160

#define FLASH_FTL_DATA_SECTOR   4096    //8MB Start

#define MSC_CONF_OSLOADER_EDB   0
#define MSC_CONF_SYS_DATA       1

#define VM_STATUS_SUSPEND           1
#define VM_STATUS_RUNNING           2
#define VM_STATUS_UNCONSCIOUS       3