
#include "FreeRTOSConfig.h"

#define VERSION     "0.2.0"


#define MEMORY_BASE     (0)
#define MEMORY_SIZE     (512*1024)

#define SYS_LOG_BUFSIZE    (512)
#define CDC_PATH_LOADER  0
#define CDC_PATH_SYS     1
#define CDC_PATH_SCRCAP  2


#define ABT_STACK_ADDR      (MEMORY_BASE + MEMORY_SIZE - 4)
#define UND_STACK_ADDR      (ABT_STACK_ADDR - 0x400)
#define FIQ_STACK_ADDR      (UND_STACK_ADDR - 0x200)
#define IRQ_STACK_ADDR      (FIQ_STACK_ADDR - 0x200)
#define SVC_STACK_ADDR      (IRQ_STACK_ADDR - 0x400)
#define SYS_STACK_ADDR      (SVC_STACK_ADDR - 0x400)


#define HEAP_END        (SYS_STACK_ADDR - 0x200)


#define SEG_SIZE            1048576

#define USE_TINY_PAGE        (1)
#define VMRAM_USE_FTL        (1)
#define USE_HARDWARE_DFLPT   (1)

#define SEPARATE_VMM_CACHE  VMRAM_USE_FTL

#if SEPARATE_VMM_CACHE
    #define NONE     0
    #define MINILZO  1      // 2 KB Work Buffer
    #define QUICKLZ  2      // 60 KB Work buffer
    #define MEM_COMPRESSION_ALGORITHM     (MINILZO) //algorithm
#endif

#define TOTAL_MEM_PAGE  (292)

#if VMRAM_USE_FTL
    #if USE_TINY_PAGE
        #if USE_HARDWARE_DFLPT
            #if SEPARATE_VMM_CACHE
                #if MEM_COMPRESSION_ALGORITHM
                    #define NUM_CACHEPAGE_VROM             ( 160 )  
                    #define NUM_CACHEPAGE_VRAM             ( 40 ) 
                    #define ZRAM_SIZE                      ( (TOTAL_MEM_PAGE - NUM_CACHEPAGE_VROM - NUM_CACHEPAGE_VRAM) * 1024 ) 
                    #define ZRAM_COMPRESSED_SIZE           (int)( ZRAM_SIZE * 3 ) // (Assume that the compression ratio is 0.33)
                #else 
                    #define NUM_CACHEPAGE_VROM             ( 64 )  
                    #define NUM_CACHEPAGE_VRAM             ( 64 ) 
                    #define ZRAM_SIZE                      ( 170 * 1024 ) // compression ratio = 1
                    #define ZRAM_COMPRESSED_SIZE           ( ZRAM_SIZE ) // 1:1
                #endif
            #else
                #define NUM_CACHEPAGE             ( 292 ) // 292 KB (Reserve 4KB for OSL)
            #endif
        #else
        
            #if SEPARATE_VMM_CACHE
                #define NUM_CACHEPAGE_VROM             ( 64 )  
                #define NUM_CACHEPAGE_VRAM             ( 32 ) 
                #define ZRAM_SIZE                      ( 128 * 1024 )
            #else
                #define NUM_CACHEPAGE             ( 260 ) // 292 KB (Reserve 4KB for OSL)
            #endif
        #endif
    #else
        #define NUM_CACHEPAGE             ( 79 ) // 79 * 4 = 316 KB
    #endif
#else
    #if USE_TINY_PAGE
        #define NUM_CACHEPAGE             ( 32 )
        #define VM_RAM_SIZE_NONE_FTL      ( 256 * 1024 )
    #else
        #define NUM_CACHEPAGE             ( 32 )
        #define VM_RAM_SIZE_NONE_FTL      ( 168 * 1024 )
    #endif
#endif




#if USE_TINY_PAGE
    #define PAGE_SIZE           (1024)
#else
    #define PAGE_SIZE           (4096)
#endif

#define PAGES_SWAPFILE      (SIZE_SWAPFILE_MB * 1048576 / PAGE_SIZE)

#define VM_ROM_BASE             (0x00100000)
#define VM_ROM_SIZE             (1048576 * 6)
#define VM_ROM_SEG              (VM_ROM_BASE >> 20)
#define VM_ROM_NUM_SEG          (VM_ROM_SIZE / SEG_SIZE)

#define VM_RAM_BASE             (0x02000000)
#define VM_RAM_SIZE             (1048576 * 3)
#define VM_RAM_SEG              (VM_ROM_BASE >> 20)
#define VM_RAM_NUM_SEG          (VM_RAM_SIZE / SEG_SIZE)

#define VM_SYS_ROM_BASE         (0x03000000)
#define VM_SYS_ROM_SIZE         (1048576 * 4)
#define VM_SYS_ROM_SEG          (VM_SYS_ROM_BASE >> 20)
#define VM_SYS_ROM_NUM_SEG      (VM_SYS_ROM_SIZE / SEG_SIZE)

/*
#define VM_SYS_RAM_BASE         (0x02000000)
#define VM_SYS_RAM_SIZE         (1048576 * 6)
#define VM_SYS_RAM_SEG          (VM_ROM_BASE >> 20)
#define VM_SYS_RAM_NUM_SEG      (VM_RAM_SIZE / SEG_SIZE)
*/

#define VM_CACHE_ENABLE     (true)
#define VM_BUFFER_ENABLE    (true)

#define TOTAL_VM_SEG        ((VM_SYS_ROM_SIZE + VM_ROM_SIZE + VM_RAM_SIZE) / SEG_SIZE)


#define SIZE_SWAPAREA_MB    (VM_RAM_SIZE / 1048576)

#define CPU_DIVIDE_NORMAL       1
#define CPU_DIVIDE_PWRSAVE      3
#define CPU_DIVIDE_IDLE_INTIAL  10

// Flash is divided in sectors of size 2K,
// a block is 64 sectors, i.e. 128K,
// the flash is on blocks number 0 to 1024 (excluded)
// or on pages (sectprs) 0 to 65536 (excluded)
// edb writes pages 16 by 16 (32K)
#define FLASH_LOADER_BLOCK      22 // page 22*64=1408 (*2K)
#define FLASH_CONFIG_BLOCK      23
#define FLASH_SYSTEM_BLOCK      31 // page 31*64=1984 (*2K)
#define FLASH_DATA_BLOCK        160 // page 10240 (*2K), also named DATA_START_BLOCK 

#define FLASH_FTL_DATA_SECTOR   4096    //8MB Start

#define MSC_CONF_OSLOADER_EDB   0
#define MSC_CONF_SYS_DATA       1

#define VM_STATUS_SUSPEND           1
#define VM_STATUS_RUNNING           2
#define VM_STATUS_UNCONSCIOUS       3