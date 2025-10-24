#ifndef _BOOT_MANAGER_H_
#define _BOOT_MANAGER_H_

// Function declarations
void _init(void);
void _boot(void);
void switch_mode(int mode);
void set_stack(unsigned int newstackptr);

// Stack address definitions (these should be defined in SystemConfig.h)
#ifndef ABT_STACK_ADDR
#define ABT_STACK_ADDR 0x0080000
#endif

#ifndef UND_STACK_ADDR
#define UND_STACK_ADDR 0x0080000
#endif

#ifndef FIQ_STACK_ADDR
#define FIQ_STACK_ADDR 0x0080000
#endif

#ifndef IRQ_STACK_ADDR
#define IRQ_STACK_ADDR 0x0080000
#endif

#ifndef SYS_STACK_ADDR
#define SYS_STACK_ADDR 0x0080000
#endif

#ifndef SVC_STACK_ADDR
#define SVC_STACK_ADDR 0x0080000
#endif

#endif // _BOOT_MANAGER_H_