

#include "FreeRTOSConfig.h"

#define MEMORY_BASE     (0)
#define MEMORY_SIZE     (512*1024)



#define ABT_STACK_ADDR         (MEMORY_SIZE - 0x400)
#define UND_STACK_ADDR      (ABT_STACK_ADDR - 0x400)
#define FIQ_STACK_ADDR      (UND_STACK_ADDR - 0x400)
#define IRQ_STACK_ADDR      (FIQ_STACK_ADDR - 0x400)
#define SVC_STACK_ADDR      (IRQ_STACK_ADDR - 0x400)
#define SYS_STACK_ADDR      (SVC_STACK_ADDR - 0x400)


#define HEAP_END        (SYS_STACK_ADDR - 0x100)





