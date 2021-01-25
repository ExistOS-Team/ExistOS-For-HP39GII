#include "tusb.h"
#include <malloc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "map.h"
#include "mmu.h"

#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"
#include "ServiceUSBDevice.h"
#include "memory_map.h"
#include "startup_info.h"

#include "raw_flash.h"

#include "nr_micro_shell.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

extern unsigned int getCurrentHeapEnd();
extern unsigned char ecc_res[4];

uint8_t buf[64];
uint8_t console_output_buffer[1024];
extern char kmsgBuff[4096];
unsigned int MSC_MODE = 1;

unsigned char buffer[512];

char log[512] = {0};
char *logptr = log;

void cdc_p(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    logptr += vsprintf(logptr, fmt, args);
    va_end(args);
}

void cdc_flush() {
    cdc_printf(log);
    cdc_clear();
}

void cdc_clear() {
    logptr = log;
    memset(log, 0, 512);
}

void cdc_printf(const char *fmt, ...) {
    int i = 0;
    va_list args;
    tud_cdc_write_clear();
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    while (!tud_cdc_write_available()) {
        //vTaskDelay(1);
        return;
    }
    while (buffer[i]) {
        tud_cdc_write_char(buffer[i++]);
    }
    va_end(args);
    tud_cdc_write_flush();
    memset(buffer, 0, 512);
}

void cdc_putchar(const char c) {
    while (!tud_cdc_write_available()) {
        vTaskDelay(1);
    }
    tud_cdc_write_char(c);
    tud_cdc_write_flush();
}

void shell_tasklist_cmd(char argc, char *argv) {
    unsigned char *tasklist_buf = pvPortMalloc(1024);
    memset(tasklist_buf, 0, 1024);

    struct mallinfo mallocInfo;

    vTaskList(tasklist_buf);
    shell_printf("=======================================================\r\n");
    shell_printf("任务名                 任务状态   优先级   剩余栈   任务序号\r\n");
    shell_printf("%s\n", tasklist_buf);
    shell_printf("任务名                运行计数           CPU使用率\r\n");
    vTaskGetRunTimeStats(tasklist_buf);
    shell_printf("%s", tasklist_buf);
    shell_printf("任务状态:  X-运行  R-就绪  B-阻塞  S-挂起  D-删除\r\n");
    shell_printf("Task mode: %x\r\n", get_mode());

    //shell_printf("free memory size: %d \n",(unsigned int)xPortGetFreeHeapSize());

    mallocInfo = mallinfo();
    shell_printf("未分配空闲内存：%d Bytes\r\n", (unsigned int)xPortGetFreeHeapSize());

    unsigned int freePhyMem, remainPhyMem;

        freePhyMem = (getCurrentHeapEnd() - KHEAP_START_VIRT_ADDR);
        remainPhyMem = freePhyMem + mallocInfo.fordblks;
    

    shell_printf("未分配物理内存：%d Bytes\r\n", freePhyMem);
    shell_printf("剩余物理内存：%d Bytes\r\n", remainPhyMem);

    shell_printf("剩余内存：%d Bytes\r\n", mallocInfo.arena - mallocInfo.uordblks + (unsigned int)xPortGetFreeHeapSize());
    shell_printf("页面文件大小: %d Bytes\r\n", swapSizeMB * 1048576);

    shell_printf("total space allocated from system: %d\r\n", mallocInfo.arena);
    shell_printf("number of non-inuse chunks: %d\r\n", mallocInfo.ordblks);
    shell_printf("number of mmapped regions: %d\r\n", mallocInfo.hblks);
    shell_printf("total space in mmapped regions: %d\r\n", mallocInfo.hblkhd);
    shell_printf("total allocated space: %d\r\n", mallocInfo.uordblks);
    shell_printf("total non-inuse space: %d\r\n", mallocInfo.fordblks);
    shell_printf("top-most, releasable (via malloc_trim) space: %d\r\n", mallocInfo.keepcost);

    vPortFree(tasklist_buf);
}

/**
 * @brief ls command
 */
void shell_ls_cmd(char argc, char *argv) {
    unsigned int i = 0;
    if (argc > 1) {
        if (!strcmp("cmd", &argv[argv[1]])) {

            for (i = 0; nr_shell.static_cmd[i].fp != NULL; i++) {
                shell_printf("%s", nr_shell.static_cmd[i].cmd);
                shell_printf("\r\n");
            }
        } else if (!strcmp("-v", &argv[argv[1]])) {
            shell_printf("ls version 1.0.\r\n");
        } else if (!strcmp("-h", &argv[argv[1]])) {
            shell_printf("useage: ls [options]\r\n");
            shell_printf("options: \r\n");
            shell_printf("\t -h \t: show help\r\n");
            shell_printf("\t -v \t: show version\r\n");
            shell_printf("\t cmd \t: show all commands\r\n");
        }
    } else {
        shell_printf("ls need more arguments!\r\n");
    }
}

void shell_help_cmd(char argc, char *argv) {
    for (int i = 0; nr_shell.static_cmd[i].fp != NULL; i++) {
        shell_printf("%s", nr_shell.static_cmd[i].cmd);
        shell_printf("\r\n");
    }
}

/**
 * @brief test command
 */
void shell_test_cmd(char argc, char *argv) {
    unsigned int i;
    shell_printf("test command:\r\n");
    for (i = 0; i < argc; i++) {
        shell_printf("paras %d: %s\r\n", i, &(argv[argv[i]]));
    }
}

// extern int micropython_main();
// void shell_micropython_cmd(char argc, char *argv){
//  	micropython_main();
// };

const static_cmd_st static_cmd[] =
    {
        {"ls", shell_ls_cmd},
        {"help", shell_help_cmd},
        {"test", shell_test_cmd},
        {"tasklist", shell_tasklist_cmd},
        //		{"mpy",shell_micropython_cmd},
        {"\0", NULL}};

void vCDC_Console() {

    unsigned char ch;

    vTaskDelay(500);

    shell_init();

    printf("Starting shell ...\n");

    for (;;) {

        if (tud_cdc_available()) {

            //printf("%c \n",tud_cdc_read_char());
            ch = tud_cdc_read_char();
            shell(ch);
            //cdc_putchar(ch);

            //tud_cdc_read_flush();
        }
        vTaskDelay(10);
        //taskYIELD();
    }
}
