
#include <stdio.h>

#include "SystemConfig.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "tusb.h"
#include "usbd.h"

#include "FTL_up.h"
#include "board_up.h"
#include "display_up.h"
#include "keyboard_up.h"
#include "llapi.h"
#include "llapi_code.h"
#include "mtd_up.h"
#include "rtc_up.h"
#include "vmMgr.h"

#include "../debug.h"

#include "stmp37xxNandConf.h"
#include "stmp_NandControlBlock.h"

#include "regs.h"
#include "regspower.h"

#include "regsdigctl.h"

#include "logo.h"

void vTaskTinyUSB(void *pvParameters);
void vMTDSvc(void *pvParameters);
void vFTLSvc(void *pvParameters);
void vKeysSvc(void *pvParameters);
void vVMMgrSvc(void *pvParameters);
void vLLAPISvc(void *pvParameters);
void vDispSvc(void *pvParameters);

TaskHandle_t pDispTask = NULL;
TaskHandle_t pSysTask = NULL;
TaskHandle_t pLLAPITask = NULL;
TaskHandle_t pLLIRQTask = NULL;
TaskHandle_t pLLIOTask = NULL;
TaskHandle_t pBattmon = NULL;

extern uint32_t g_mtd_write_cnt;
extern uint32_t g_mtd_read_cnt;
extern uint32_t g_mtd_erase_cnt;
extern uint32_t g_mtd_ecc_cnt;
extern uint32_t g_mtd_ecc_fatal_cnt;

uint32_t CurMount = 0;
uint32_t g_FTL_status = 10;
bool g_sysfault_auto_reboot = true;
uint32_t g_MSC_Configuration = MSC_CONF_OSLOADER_EDB;
extern uint32_t volatile g_latest_key_status;
volatile uint32_t g_vm_status = VM_STATUS_SUSPEND;
bool vm_needto_reset = false;

char pcWriteBuffer[4096 + 1024];
uint32_t HCLK_Freq;

extern uint32_t g_page_vram_fault_cnt;
extern uint32_t g_page_vrom_fault_cnt;

uint32_t g_core_temp, g_batt_volt;
uint32_t g_core_cur_freq_mhz = 1;

uint32_t check_frequency() {
    volatile uint32_t s0, s1;
    s0 = *((volatile uint32_t *)0x8001C020);
    vTaskDelay(pdMS_TO_TICKS(1000));
    s1 = *((volatile uint32_t *)0x8001C020);
    return (s1 - s0) / 1;
}
void printTaskList() {
    /*
    vTaskList((char *)&pcWriteBuffer);
    printf("=================OS Loader TASK==================\r\n");
    printf("Task Name         Task Status   Priority   Stack   ID\n");
    printf("%s\n", pcWriteBuffer);
    printf("Task Name                Running Count           CPU %%\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s\n", pcWriteBuffer);
    printf("Status:  X-Running  R-Ready  B-Block  S-Suspend  D-Delete\n");
    */
    printf("Free PhyMem:   %d Bytes\n", (unsigned int)xPortGetFreeHeapSize());

    printf("=================OS Loader Info==================\r\n");
    printf("VRAM PageFault:   %ld \n", g_page_vram_fault_cnt);
    printf("VROM PageFault:   %ld \n", g_page_vrom_fault_cnt);
    printf("HCLK Freq:%ld MHz\n", HCLK_Freq / 1000000);
    printf("CPU Freq:%ld MHz\n", g_core_cur_freq_mhz);
    printf("Flash IO_Writes:%lu\n", g_mtd_write_cnt);
    printf("Flash IO_Reads:%lu\n", g_mtd_read_cnt);
    printf("Flash IO_Erases:%lu\n", g_mtd_erase_cnt);
    printf("Flash ECC Count:%lu\n", g_mtd_ecc_cnt);
    printf("Flash ECC FATAL:%lu\n", g_mtd_ecc_fatal_cnt);
    printf("Batt Charge:%d\n", HW_POWER_STS.B.CHRGSTS);
    printf("PWD_BATTCHRG:%d\n", HW_POWER_CHARGE.B.PWD_BATTCHRG);
    printf("RTC:%ld\n", rtc_get_seconds());
    printf("=============================================\r\n\n");
}

void vTask1(void *pvParameters) {
    // printf("Start vTask1\n");
    int c = 0;
    for (;;) {
        HCLK_Freq = check_frequency();
        g_core_cur_freq_mhz = (HCLK_Freq / 1000000) * (*((uint32_t *)0x80040030) & 0x1F);
        c++;
        if (c == 30) {
            printTaskList();
            c = 0;
        }
    }
}

static void getKey(uint32_t *key, uint32_t *press) {
    {
        *key = g_latest_key_status & 0xFFFF;
        *press = g_latest_key_status >> 16;
    }
}

static uint32_t waitAnyKey() {
    uint32_t key;
    uint32_t lkey;

    key = g_latest_key_status & 0xFFFF;
    lkey = key;
    do {
        vTaskDelay(pdMS_TO_TICKS(30));
        key = g_latest_key_status & 0xFFFF;
    } while (key == lkey);
    return key;
}

extern bool g_vm_inited;
uint32_t *bootAddr;
uint32_t *atagsAddr;
void System(void *par) {
    bootAddr = (uint32_t *)VM_ROM_BASE;

    // vTaskDelay(pdMS_TO_TICKS(5000));
    INFO("Booting...\n");
    // DisplayClean();

    // DisplayFillBox(32, 32, 224, 64, 128);
    DisplayFlushArea(103, 32, 152, 56, &logo, false);
    // DisplayPutStr(64, 42, "System Booting...", 255, 128, 16);

    for (int i = 90; i <= 120; ++i)
        DisplayFillBox(i - 2, 84, i, 92, 72);

    // DisplayPutStr(64, 16 * 2, "System Booting...", 255, 32, 16);
    // DisplayPutStr(64, 16 * 3, "Waiting for Flash GC...", 255, 32, 16);


    vTaskDelay(pdMS_TO_TICKS(100));
    uint32_t k, kp;
    getKey(&k, &kp);
    if ((k == KEY_F2) && kp) {
        slowDownEnable(false);
        tud_disconnect();

        DisplayFillBox(32, 32, 224, 64, 128);
        DisplayPutStr(80, 42, "USB MSC Mode", 255, 128, 16);
        DisplayPutStr(42, 8, "Press [Views] to exit", 128, 255, 16);

        vTaskDelay(pdMS_TO_TICKS(200));
        g_MSC_Configuration = MSC_CONF_SYS_DATA;
        vTaskDelay(pdMS_TO_TICKS(10));
        tud_connect();

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(200));
            getKey(&k, &kp);
            if ((k == KEY_VIEWS) && kp) {
                FTL_Sync();

                tud_disconnect();

                DisplayFillBox(42, 8, 210, 24, 255);
                DisplayFillBox(32, 32, 224, 64, 255);
                DisplayFlushArea(103, 32, 152, 56, &logo, false);
                // DisplayPutStr(64, 42, "System Booting...", 255, 128, 16);

                g_MSC_Configuration = MSC_CONF_OSLOADER_EDB;
                vTaskDelay(pdMS_TO_TICKS(10));
                tud_connect();
                break;
            }
        }
    }

    for (int i = 120; i <= 150; ++i)
        DisplayFillBox(i - 2, 84, i, 92, 72);

    if ((*bootAddr != 0xEF5AE0EF) && *(bootAddr + 1) != 0xFECDAFDE) {
        slowDownEnable(false);
        // DisplayClean();
        // DisplayPutStr(0, 16 * 0, "========[Exist OS Loader]======", 0, 255, 16);
        // DisplayPutStr(0, 16 * 1, "Could not find the System!", 0, 255, 16);

        DisplayFillBox(32, 32, 224, 64, 128);
        DisplayFillBox(48, 80, 208, 96, 255);
        DisplayPutStr(54, 42, "No System Installed ", 255, 128, 16);

        for (int i = 16; i >= 0; --i) {
            DisplayFillBox(115, 76, 141, 128, 255);
            DisplayFillBox(123, 112 + i, 132, 128 + i, 128);
            DisplayFillBox(120, 76 + i, 136, 80 + i, 192);
            DisplayFillBox(115, 80 + i, 141, 112 + i, 0);
            vTaskDelay(pdMS_TO_TICKS(4 * (16 - i)));
        }

        g_vm_status = VM_STATUS_SUSPEND;
        vTaskSuspend(NULL);
    }




    for (int i = 150; i <= 180; ++i)
        DisplayFillBox(i - 2, 84, i, 92, 72);

    setCPUDivider(CPU_DIVIDE_NORMAL);
    bootAddr += 4;
    atagsAddr = (uint32_t *)(VM_ROM_BASE + (4234 - 1984) * 2048);

    g_vm_status = VM_STATUS_RUNNING;

    for (int i = 180; i <= 202; ++i)
        DisplayFillBox(i - 2, 84, i, 92, 72);


    vTaskPrioritySet(pDispTask, configMAX_PRIORITIES - 5);

    __asm volatile("mrs r1,cpsr_all");
    __asm volatile("bic r1,r1,#0x1f");
    __asm volatile("orr r1,r1,#0x10");
    __asm volatile("msr cpsr_all,r1");

    __asm volatile("mov r13,#0x02300000");
    __asm volatile("add r13,#0x000FA000");

    __asm volatile("mov r0,#0");

    __asm volatile("mov r1,#0x02300000"); // machine type id
    __asm volatile("add r1,#0x00023000");

    __asm volatile("ldr r2,=atagsAddr"); // atags/dtb pointer
    __asm volatile("ldr r2,[r2]");

    __asm volatile("ldr r4,=bootAddr");
    __asm volatile("ldr r4,[r4]");
    __asm volatile("mov pc,r4");

    while (1)
        ;
}

bool VMsavedIrq;
void VMSuspend() {
    if (g_vm_status == VM_STATUS_SUSPEND) {
        return;
    }
    if (pSysTask && pLLAPITask && pLLIRQTask) {

        vTaskSuspend(pLLIRQTask);
        vTaskSuspend(pLLAPITask);
        vTaskSuspend(pSysTask);
        VMsavedIrq = LLIRQ_enable(false);
        LLIRQ_ClearIRQs();

        g_vm_status = VM_STATUS_SUSPEND;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void VMResume() {
    if (g_vm_status == VM_STATUS_RUNNING) {
        return;
    }
    if (pSysTask && pLLAPITask && pLLIRQTask) {
        LLIRQ_ClearIRQs();
        LLIRQ_enable(VMsavedIrq);
        vTaskResume(pLLIRQTask);
        vTaskResume(pLLAPITask);
        vTaskResume(pSysTask);

        g_vm_status = VM_STATUS_RUNNING;
    }
}



void VM_Unconscious(TaskHandle_t task, char *res, uint32_t address) {
    char buf[19];
    uint8_t i;

    // if ((task == pSysTask) && g_sysfault_auto_reboot)
    {

        vTaskSuspend(pLLIRQTask);
        vTaskSuspend(pLLAPITask);
        LLIRQ_enable(false);

        uint32_t *pRegFram = (uint32_t *)(((uint32_t *)pSysTask)[1]);
        pRegFram -= 16;

        DisplayClean();
        //DisplayFillBox(4, 4, 252, 20, 0);
        //DisplayPutStr(16, 5, "System Panic! ", 255, 0, 16);
        //DisplayFillBox(8, 24, 248, 120, 208);

        if (res != NULL) {
            DisplayPutStr(56, 16, strcat(res, " "), 208, 0, 16);
        }

        //DisplayPutStr(24, 16 * 2 - 8, "[ON+F5] > Maintenance Menu", 96, 208, 16);

        for(i = 0; i < 4; i++){
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "%08lx %08lx ", pRegFram[12 + i], pRegFram[i]);
            DisplayPutStr(56, 16 * (i + 2), buf, 0, 208, 16);
        }
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%08lx %08lx ", pRegFram[-1], address);
        DisplayPutStr(56, 96, buf, 0, 208, 16);
        
        /*
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "R12:%08lx  R0:%08lx ", pRegFram[12], pRegFram[0]);
        DisplayPutStr(24, 16 * 3 - 8, buf, 0, 208, 16);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "R13:%08lx  R1:%08lx ", pRegFram[13], pRegFram[1]);
        DisplayPutStr(24, 16 * 4 - 8, buf, 0, 208, 16);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "R14:%08lx  R2:%08lx ", pRegFram[14], pRegFram[2]);
        DisplayPutStr(24, 16 * 5 - 8, buf, 0, 208, 16);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "R15:%08lx  R3:%08lx ", pRegFram[15], pRegFram[3]);
        DisplayPutStr(24, 16 * 6 - 8, buf, 0, 208, 16);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "CPSR:%08lx FAR:%08lx ", pRegFram[-1], address);
        DisplayPutStr(24, 16 * 7 - 8, buf, 0, 208, 16);
        */

        g_vm_status = VM_STATUS_UNCONSCIOUS;

        // portBoardReset();
    }
}

unsigned char blockChksum(char *block, unsigned int blockSize) {
    unsigned char sum = 0x5A;
    for (int i = 0; i < blockSize; i++) {
        sum += block[i];
    }
    return sum;
}

#define CDC_BINMODE_BUFSIZE 32768
bool transBinMode = false;
char *binBuf = NULL;
uint32_t cdcBlockCnt;
void MscSetCmd(char *cmd);
void mkSTMPNandStructure(uint32_t OLStartBlock, uint32_t OLPages);
void parseCDCCommand(char *cmd) {
    if (strcmp(cmd, "PING") == 0) {
        slowDownEnable(false);
        printf("CDC PING\n");
        MscSetCmd("PONG\n");

        return;
    }

    if (strcmp(cmd, "RESETDBUF") == 0) {
        slowDownEnable(false);
        VMSuspend();

        if (binBuf == NULL) {
            binBuf = (char *)VMMGR_GetCacheAddress();
        }
        printf("REC DATA BUF.\n");
        memset(binBuf, 0xFF, CDC_BINMODE_BUFSIZE);
        MscSetCmd("READY\n");

        return;
    }

    if (strcmp(cmd, "BUFCHK") == 0) {
        uint8_t chk;
        if (binBuf == NULL) {
            binBuf = (char *)VMMGR_GetCacheAddress();
        }
        char res[16];

        chk = blockChksum(binBuf, CDC_BINMODE_BUFSIZE);
        // printf("CHKSUM:%02x\n", chk);
        sprintf(res, "CHKSUM:%02x\n", chk);
        MscSetCmd(res);
        
        return;
    }

    if (memcmp(cmd, "ERASEB", 6) == 0) {
        uint32_t erase_blk = 30;
        sscanf(cmd, "ERASEB:%ld", &erase_blk);
        printf("ERASEB:%ld\n", erase_blk);

        MTD_ErasePhyBlock(erase_blk);

        MscSetCmd("EROK\n");
        

        return;
    }

    if (memcmp(cmd, "PROGP", 5) == 0) {
        uint32_t prog_page = 1111;
        uint32_t wrMeta;
        uint8_t *mtbuff = NULL;
        sscanf(cmd, "PROGP:%ld,%ld", &prog_page, &wrMeta);
        printf("PROGP:%ld,%ld\n", prog_page, wrMeta);

        if (wrMeta) {
            mtbuff = pvPortMalloc(19);
            memset(mtbuff, 0xFF, 19);
            mtbuff[1] = 0x00;
            mtbuff[2] = 0x53; // S
            mtbuff[3] = 0x54; // T
            mtbuff[4] = 0x4D; // M
            mtbuff[5] = 0x50; // P
        }

        for (int i = 0; i < CDC_BINMODE_BUFSIZE / 2048; i++) {
            if (wrMeta) {
                MTD_WritePhyPageWithMeta(prog_page + i, 6, (uint8_t *)&binBuf[i * 2048], mtbuff);
            } else {
                MTD_WritePhyPage(prog_page + i, (uint8_t *)&binBuf[i * 2048]);
            }
        }

        if (wrMeta) {
            vPortFree(mtbuff);
        }

        MscSetCmd("PGOK\n");
        
        return;
    }

    if (memcmp(cmd, "MKNCB", 5) == 0) {
        uint32_t stblock, pages;
        sscanf(cmd, "MKNCB:%ld,%ld", &stblock, &pages);
        printf("MKNCB:%ld,%ld\n", stblock, pages);
        mkSTMPNandStructure(stblock, pages);
        MscSetCmd("MKOK\n");
        
        return;
    }



    if (strcmp(cmd, "REBOOT") == 0) {
        portBoardReset();
        return;
    }

    if (strcmp(cmd, "ERASEALL") == 0) {
        MTD_EraseAllBLock();
        return;
    }

    if (strcmp(cmd, "MSCDATA") == 0) {
        tud_disconnect();
        DisplayClean();
        DisplayPutStr(0, 0, "USB MSC Mode.", 0, 255, 16);
        vTaskDelay(pdMS_TO_TICKS(200));
        g_MSC_Configuration = MSC_CONF_SYS_DATA;
        vTaskDelay(pdMS_TO_TICKS(10));
        tud_connect();
        return;
    }
}

uint32_t g_CDC_TransTo = 100;
// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
    (void)itf;
    (void)rts;
    cdc_line_coding_t c;

    // connected
    if (dtr) {
        // print initial message when connected
        // tud_cdc_write_str("\r\nTinyUSB CDC MSC device example\r\n");
        printf("CDC RESET\n");
        // tud_cdc_write_flush();
    }
    tud_cdc_get_line_coding(&c);

    switch (c.bit_rate) {
    case 14400:

        printf("CDC LOADER PATH\n");
        g_CDC_TransTo = CDC_PATH_LOADER;
        // if(tud_cdc_write_available())
        {
            tud_cdc_write_str("USB CDC-ACM OPEN.\n");
            // tud_cdc_write_flush();
        }
        break;
    case 9600:
        printf("CDC SYS PATH\n");
        g_CDC_TransTo = CDC_PATH_SYS;
        break;
    case 38400:
        printf("CDC SCRCAP PATH\n");
        g_CDC_TransTo = CDC_PATH_SCRCAP;
        break;
    default:
        break;
    }

    tud_cdc_read_flush();
    transBinMode = rts;
    if (transBinMode) {
        cdcBlockCnt = 0;
        printf("CDC BIN MODE\n");
    } else {
        printf("CDC TEXT MODE\n");
    }
}

char cdc_path_loader_buffer[64];
// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
    (void)itf;
    int32_t nRead;

    if (g_CDC_TransTo == CDC_PATH_SYS) {
        nRead = tud_cdc_available();
        if (nRead) {
            LLIO_NotifySerialRxAvailable();
        }
    }

    if (g_CDC_TransTo == CDC_PATH_LOADER) {
        nRead = tud_cdc_available();
        if (nRead < sizeof(cdc_path_loader_buffer)) {
            tud_cdc_read(cdc_path_loader_buffer, sizeof(cdc_path_loader_buffer));
            cdc_path_loader_buffer[nRead] = 0;
            printf("cmd:%s\n", cdc_path_loader_buffer);

            if (strcmp(cdc_path_loader_buffer, "getstatus") == 0) {
                printTaskList();
                goto fin;
            }

            if (strcmp(cdc_path_loader_buffer, "poweroff") == 0) {
                portBoardPowerOff();
                goto fin;
            }

            if (strcmp(cdc_path_loader_buffer, "clearall") == 0) {
                FTL_ClearAllSector();
                printf("clear all sector.\n");
                goto fin;
            }
        }

    fin:
        tud_cdc_read_flush();
    }

}

static bool eraseDataMenu = false;
static bool transScr = false;
static int contrast_adj = 0;

void __attribute__((target("thumb"))) vMainThread_thumb_entry(void *pvParameters) {

    // vTaskDelay(pdMS_TO_TICKS(100));
    setHCLKDivider(2);
    setCPUDivider(2);

    // portLRADCEnable(1, 7);
    //  MTD_EraseAllBLock();
    //  while (g_FTL_status == 10) {
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }

    printf("FTL Code:%ld\n", g_FTL_status);

    portLRADCConvCh(7, 1);

    g_CDC_TransTo = CDC_PATH_LOADER;
    // vTaskDelay(pdMS_TO_TICKS(1000));
    /*
    printf("Batt. voltage:%d mv, adc:%d\n", portGetBatterVoltage_mv(), portLRADCConvCh(7, 5));
    printf("VDDIO: %d mV\n", (int)(portLRADCConvCh(6, 5) * 0.9));
    printf("VDD5V: %d mV\n", (int)(portLRADCConvCh(5, 5) * 0.45 * 4));
    printf("Core Temp: %d ℃\n", (int)((portLRADCConvCh(4, 5) - portLRADCConvCh(3, 5)) * 1.012 / 4 - 273.15));
*/

    vTaskDelay(pdMS_TO_TICKS(1000));

    HW_POWER_5VCTRL.B.ENABLE_DCDC = 1;

    HW_POWER_CHARGE.B.CHRG_STS_OFF = 0;

    HW_POWER_CHARGE.B.BATTCHRG_I = 1 << 5;
    HW_POWER_CHARGE.B.STOP_ILIMIT = 0;

    HW_POWER_CHARGE.B.PWD_BATTCHRG = 1;

    HW_POWER_VDDDCTRL.B.DISABLE_FET = 1;

    // HW_POWER_VDDACTRL.B.DISABLE_FET = 1;
    // HW_POWER_VDDIOCTRL.B.DISABLE_FET = 1;

    for (;;) {

        vTaskDelay(pdMS_TO_TICKS(100));

        if (contrast_adj) {
            extern uint32_t g_lcd_contrast;
            g_lcd_contrast += contrast_adj;
            portDispSetContrast(g_lcd_contrast);
        }


        if (eraseDataMenu) {

            VMSuspend();
            DisplayClean();

            uint8_t key, op;
            op = 0;
            DisplayFillBox(4, 4, 252, 20, 0);
            DisplayPutStr(36, 5, "Device Maintenance Menu ", 255, 0, 16);

            do {
                DisplayFillBox(8, 24, 248, 120, 208);
                DisplayPutStr(36, 44, "[F1]  Clear System Data ", 32, 208, 16);
                DisplayPutStr(36, 60, "[F2]  Erase All Flash ", 32, 208, 16);
                DisplayPutStr(36, 76, "[F3]  Exit & Reboot ", 32, 208, 16);
                // DisplayFlushArea(20, 100, 39, 109, &logo, true);
                //  DisplayCircle(128, 64, 48, 129, true);

                vTaskDelay(pdMS_TO_TICKS(200));
                key = waitAnyKey();

                DisplayFillBox(8, 24, 248, 120, 208);
                vTaskDelay(pdMS_TO_TICKS(200));
                if (key == KEY_F1) {
                    DisplayPutStr(16, 32, "Clear System Data?", 32, 208, 16);
                    //DisplayPutStr(16, 48, "User data will be erased! ", 128, 208, 16);
                    DisplayPutStr(16, 104, "[Enter]: YES    [Else]: NO ", 32, 208, 16);

                    //vTaskDelay(pdMS_TO_TICKS(200));
                    key = waitAnyKey();

                    DisplayFillBox(8, 24, 248, 120, 208);
                    if (key == KEY_ENTER) {
                        DisplayFillBox(32, 32, 224, 64, 128);
                        DisplayPutStr(48, 42, "Clearing System Data", 255, 128, 16);
                        DisplayFillBox(48, 80, 208, 96, 200);
                        DisplayFillBox(50, 82, 206, 94, 255);

                        //vTaskDelay(pdMS_TO_TICKS(500));
                        for (int i = FLASH_DATA_BLOCK; i < 1024; i++) {
                            MTD_ErasePhyBlock(i);
                            DisplayFillBox(52, 84, 52 + i * 0.15, 92, 16);
                        }
                        op = 1;
                    }

                } else if (key == KEY_F2) {
                    DisplayPutStr(16, 32, "Erase ALL Flash? ", 32, 208, 16);
                    //DisplayPutStr(16, 48, "You need to ", 128, 208, 16);
                    //DisplayPutStr(16, 64, "reinstall firmware! ", 128, 208, 16);
                    DisplayPutStr(16, 104, "[Enter]: YES    [Else]: NO ", 32, 208, 16);

                    //vTaskDelay(pdMS_TO_TICKS(200));
                    key = waitAnyKey();

                    DisplayFillBox(8, 24, 248, 120, 208);
                    if (key == KEY_ENTER) {
                        DisplayFillBox(32, 32, 224, 64, 128);
                        DisplayPutStr(60, 42, "Erasing All Flash", 255, 128, 16);
                        DisplayFillBox(48, 80, 208, 96, 200);
                        DisplayFillBox(50, 82, 206, 94, 255);

                        //vTaskDelay(pdMS_TO_TICKS(500));
                        for (int i = 0; i < 1024; i++) {
                            MTD_ErasePhyBlock(i);
                            DisplayFillBox(52, 84, 52 + i * 0.15, 92, 16);
                        }

                        //DisplayFillBox(8, 24, 248, 120, 208);
                        //DisplayPutStr(30, 48, "ALL FLASH HAS BEEN ERASED ", 32, 208, 16);
                        //vTaskDelay(pdMS_TO_TICKS(1000));
                        op = 2;
                    }

                } else if (key == KEY_F3) {
                    op = 1;
                }
            } while (op == 0);

            DisplayFillBox(8, 24, 248, 120, 208);
            if (op == 1) {
                DisplayPutStr(48, 52, "Device is Rebooting", 32, 208, 16);
                vTaskDelay(pdMS_TO_TICKS(500));
                portBoardReset();
            } else if (op == 2) {
                DisplayClean();
                DisplayPutStr(76, 52, "Flash Cleared ", 0, 255, 16);
                vTaskDelay(pdMS_TO_TICKS(500));
                portBoardReset();
            }
        }

        uint8_t *vramBuf;
        if (transScr) {
            vTaskDelay(pdMS_TO_TICKS(1000));

            vramBuf = pvPortMalloc(256 * 3);
            if (!vramBuf) {
                goto fin;
            }

            for (int y = 0; y < 128; y += 2) {
                bool fin;
                DisplayReadArea(0, y, 255, y + 2, vramBuf, &fin);
                while (!fin) {
                    vTaskDelay(pdMS_TO_TICKS(30));
                }

                tud_cdc_write(vramBuf, 256 * 3);
                tud_cdc_write_flush();
                vTaskDelay(pdMS_TO_TICKS(15));
            }

            vPortFree(vramBuf);

        fin:
            vTaskDelay(pdMS_TO_TICKS(100));
            transScr = false;
        }
    }
}

void __attribute__((target("arm"))) vMainThread(void *pvParameters) {
    vMainThread_thumb_entry(pvParameters);
}

int __attribute__((target("thumb"))) capt_ON_Key(int ck, int cp) {

    if (ck == KEY_F3 && cp) {
        portBoardPowerOff();
    }

    if ((ck == KEY_PLUS)) {
        if (cp) {
            contrast_adj = 1;
        } else {
            contrast_adj = 0;
        }

        return 1;
    }
    if ((ck == KEY_SUBTRACTION)) {
        if (cp) {
            contrast_adj = -1;
        } else {
            contrast_adj = 0;
        }
        return 1;
    }

    if ((ck == KEY_F6) && cp) { // [ON] + [F6]
        // DisplayClean();

        // FTL_Sync();
        portBoardReset();
        return 1;
    }

    if ((ck == KEY_F5) && cp) { // [ON] + [F5]
        eraseDataMenu = true;
        return 1;
    }

    if ((ck == KEY_F2) && cp) {
        if (g_CDC_TransTo == CDC_PATH_SCRCAP) {
            // tud_cdc_write_clear();
            // tud_cdc_write_flush();
            if (!transScr) {
                transScr = true;
            }
        }

        return 1;
    }

    return 0;
}

void get_cpu_info() {
    register uint32_t val;
    __asm volatile("mrc p15,0,%0,c0,c0,0"
                   : "=r"(val));
    printf("CPU ID:%08lx\n", val);

    __asm volatile("mrc p15,0,%0,c0,c0,1"
                   : "=r"(val));

    printf("ICache Size[%08lx]:%d KB\n", val, 1 << ((((val >> 0) >> 6) & 0xF) - 1));
    printf("DCache Size[%08lx]:%d KB\n", val, 1 << ((((val >> 12) >> 6) & 0xF) - 1));

    __asm volatile("mrc p15,0,%0,c0,c0,2"
                   : "=r"(val));
    printf("DTCM:%ld, ITCM:%ld\n", (val >> 16) & 1, val & 1);

    __asm volatile("mrc p15,0,%0,c9,c0,1"
                   : "=r"(val));
    printf("Cache LOCK:%08lx\n", val);
}

void vBatteryMon(void *__n) {

    uint32_t vatt_adc = 0;
    uint32_t batt_voltage = 0;
    uint32_t vdd5v_voltage = 0;
    int coreTemp = 0;
    long t = 0;
    int n = 0;

    uint32_t show_bat_val;

    for (;;) {

        vatt_adc = portLRADCConvCh(7, 5);
        batt_voltage = portGetBatterVoltage_mv();
        vdd5v_voltage = (int)(portLRADCConvCh(5, 5) * 0.45 * 4);
        coreTemp = (int)((portLRADCConvCh(4, 5) - portLRADCConvCh(3, 5)) * 1.012 / 4 - 273.15);

        if (t % 5 == 0) {

            g_core_temp = coreTemp;
            g_batt_volt = batt_voltage;

            if (portGetBatteryMode() == 0) {
                printf("Battery = Li-ion\n");
            } else {
                printf("Battery = Single AA or AAA\n");
            }
            printf("Batt. voltage:%ld mv, adc:%ld\n", batt_voltage, vatt_adc);
            printf("VDDIO: %d mV\n", (int)(portLRADCConvCh(6, 5) * 0.9));
            printf("VDD5V: %ld mV\n", vdd5v_voltage);
            printf("VBG: %d mV\n", (int)(portLRADCConvCh(2, 5) * 0.45));
            printf("Core Temp: %d ℃\n", coreTemp);
            printf("Power Speed:%lu\n", portGetPWRSpeed());
        }
        t++;

        if (vdd5v_voltage > 3500) {
        }
        show_bat_val = batt_voltage;
        if (show_bat_val > 1500) {
            show_bat_val = 1500;
        }
        if (show_bat_val < 800) {
            show_bat_val = 800;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        n = 0;
        if (((show_bat_val - 800) * 100 / (1500 - 800)) >= ((100 / 4) * 1))
            n |= (1 << 0);
        if (((show_bat_val - 800) * 100 / (1500 - 800)) >= ((100 / 4) * 2))
            n |= (1 << 1);
        if (((show_bat_val - 800) * 100 / (1500 - 800)) >= ((100 / 4) * 3))
            n |= (1 << 2);
        if (((show_bat_val - 800) * 100 / (1500 - 800)) >= ((100 / 4) * 4))
            n |= (1 << 3);
        DisplaySetIndicate(-1, n);
    }
}

extern uint32_t log_i;
extern uint32_t log_j;
extern char log_buf[SYS_LOG_BUFSIZE];

void TaskUSBLog(void *_) {

    vTaskDelay(pdMS_TO_TICKS(2000));
    for (;;) {

        int ava;
        if (g_CDC_TransTo == CDC_PATH_LOADER) {
            ava = tud_cdc_write_available();
            if (ava > 0) {
            retest:
                if (log_j < log_i) {
                    if (log_i - log_j <= ava) {
                        tud_cdc_write(&log_buf[log_j], log_i - log_j);
                        tud_cdc_write_flush();
                        log_j = log_i;
                    } else {
                        tud_cdc_write(&log_buf[log_j], ava);
                        tud_cdc_write_flush();
                        log_j += ava;
                    }

                } else if (log_j > log_i) {
                    if (SYS_LOG_BUFSIZE - log_j <= ava) {
                        tud_cdc_write(&log_buf[log_j], SYS_LOG_BUFSIZE - log_j + 1);
                        tud_cdc_write_flush();
                        log_j = 0;
                    } else {
                        tud_cdc_write(&log_buf[log_j], ava);
                        tud_cdc_write_flush();
                        log_j += ava;
                    }

                    vTaskDelay(pdMS_TO_TICKS(200));
                    goto retest;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
extern bool g_slowdown_enable;
#include "regsclkctrl.h"
void waitIRQ(int r) {

    enterSlowDown();
    if (g_slowdown_enable) {
        HW_CLKCTRL_CPU.B.INTERRUPT_WAIT = 1;

        asm volatile("mov r0, #0");           // Rd SBZ (should be 0)
        asm volatile("mcr p15,0,r0,c7,c0,4"); // Drain write buffers, idle CPU clock & processor, and stop processor at this instruction
        asm volatile("nop");
    }

    exitSlowDown();
}

void vApplicationIdleHook(void) {
    waitIRQ(0);
}

extern int bootTimes;
volatile void _startup() {

    printf("OSLoader starting...\nreboot count: %d\n", bootTimes);

    get_cpu_info();

    boardInit();
    printf("booting .....\n");

    xTaskCreate(vTask1, "Status Print", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(vMTDSvc, "MTD Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(vFTLSvc, "FTL Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(vDispSvc, "Display Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &pDispTask);
    xTaskCreate(TaskUSBLog, "USB Log", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(vTaskTinyUSB, "TinyUSB", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 4, NULL);
    xTaskCreate(vVMMgrSvc, "VM Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 4, NULL);

    xTaskCreate(vKeysSvc, "Keys Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    xTaskCreate(vLLAPISvc, "LLAPI Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 5, &pLLAPITask);
    xTaskCreate(LLIRQ_task, "LLIRQ Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 6, &pLLIRQTask);
    //xTaskCreate(LLIO_ScanTask, "LLIO Svc", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 6, &pLLIOTask);

    xTaskCreate(System, "System", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 7, &pSysTask);

    xTaskCreate(vBatteryMon, "Battery Mon", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &pBattmon);
    xTaskCreate(vMainThread, "Main Thread", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    // pSysTask = xTaskCreateStatic( (TaskFunction_t)0x00100000, "System", VM_RAM_SIZE, NULL, 1, VM_RAM_BASE, pvPortMalloc(sizeof(StaticTask_t)));

    // vTaskSuspend(pSysTask);
    // vTaskSuspend(pLLAPITask);

    vTaskStartScheduler();
    printf("booting fail.\n");

    while (1)
        ;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    PANIC("StackOverflowHook:%s\n", pcTaskName);
}

void vAssertCalled(char *file, int line) {
    PANIC("ASSERT %s:%d\n", file, line);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
    *ppxTimerTaskStackBuffer = (StackType_t *)pvPortMalloc(configMINIMAL_STACK_SIZE * 4);
    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationMallocFailedHook() {
    PANIC("ASSERT: Out of Memory.\n");
}