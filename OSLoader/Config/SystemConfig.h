

#include "FreeRTOSConfig.h"

#define VERSION     "0.0.2 Beta"

#define MEMORY_BASE     (0)
#define MEMORY_SIZE     (512*1024)



#define ABT_STACK_ADDR         (MEMORY_SIZE - 0x80)
#define UND_STACK_ADDR      (ABT_STACK_ADDR - 0x400)
#define FIQ_STACK_ADDR      (UND_STACK_ADDR - 0x200)
#define IRQ_STACK_ADDR      (FIQ_STACK_ADDR - 0x200)
#define SVC_STACK_ADDR      (IRQ_STACK_ADDR - 0x200)
#define SYS_STACK_ADDR      (SVC_STACK_ADDR - 0x200)


#define HEAP_END        (SYS_STACK_ADDR - 0x200)

// DATA   SYS   SWAP
#define DISK_PARTITION      {80, 15, 5, 0}

#define SEG_SIZE            1048576

#define NUM_CACHEPAGE       (64 + 8 + 0)



//#define SIZE_SWAPFILE_MB    2
#define PAGE_SIZE           4096

#define PAGES_SWAPFILE      (SIZE_SWAPFILE_MB * 1048576 / PAGE_SIZE)

#define VM_ROM_BASE         (0x00100000)
#define VM_ROM_SIZE         (1048576 * 10)
#define VM_ROM_SEG          (VM_ROM_BASE >> 20)
#define VM_ROM_NUM_SEG      (VM_ROM_SIZE / SEG_SIZE)

#define VM_RAM_BASE         (0x02000000)
#define VM_RAM_SIZE         (1048576 * 4)
#define VM_RAM_SEG          (VM_ROM_BASE >> 20)
#define VM_RAM_NUM_SEG      (VM_RAM_SIZE / SEG_SIZE)

#define VM_CACHE_ENABLE     (true)
#define VM_BUFFER_ENABLE    (true)

#define TOTAL_VM_SEG        ((VM_ROM_SIZE + VM_RAM_SIZE) / SEG_SIZE)


#define SIZE_SWAPAREA_MB    (VM_RAM_SIZE / 1048576)
