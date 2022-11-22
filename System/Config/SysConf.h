#ifndef __SYSCONF_H__
#define __SYSCONF_H__

#define INDICATE_LEFT      (1 << 0)
#define INDICATE_RIGHT     (1 << 1)
#define INDICATE_A__Z      (1 << 2)
#define INDICATE_a__z      (1 << 3)
#define INDICATE_BUSY      (1 << 4)
#define INDICATE_TX        (1 << 5)
#define INDICATE_RX        (1 << 6)

#define RAM_BASE    (0x02000000)
#define RAM_SIZE    (3 * 1048576)

#define FS_FATFS        0
#define FS_LITTLEFS     1
#define FS_TYPE         FS_FATFS

#define NORMAL_STACK_ADDR   (RAM_BASE + RAM_SIZE - 8)
#define IRQ_STACK_ADDR      (NORMAL_STACK_ADDR  - 100 * 4)
#define SWI_STACK_ADDR      (IRQ_STACK_ADDR     - 100 * 4)
#define KERNEL_HEAP_END     (SWI_STACK_ADDR - 100 * 4)

#endif