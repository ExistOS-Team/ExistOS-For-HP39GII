
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include <stdio.h>
#include <string.h>

#include "board_up.h"
#include "display_up.h"
#include "keyboard_up.h"

#include "llapi.h"
#include "llapi_code.h"

#include "../debug.h"

#include "FTL_up.h"
#include "mmu.h"
#include "vmMgr.h"

#include "tusb.h"

QueueHandle_t LLAPI_Queue;
QueueHandle_t LLIRQ_Queue;
QueueHandle_t LLAPI_KBDQueue;

extern uint32_t g_CDC_TransTo;
extern uint32_t g_latest_key_status;

uint32_t vm_irq_stack_address, vm_svc_stack_address;
uint32_t vm_irq_vector_address, vm_svc_vector_address;

bool vm_enable_irq = false;
bool vm_in_exception = false;
bool vm_key_input_enable = false;
bool vm_serial_enable = false;
bool g_vm_in_pagefault = false;


TimerHandle_t vm_timer = NULL;
TaskHandle_t vm_sys;

volatile uint32_t vm_saved_context[17 * 5]; //{CPSR, R0-R15}
volatile uint32_t context_ptr = 0;

volatile bool isSaved = false;

volatile uint32_t curExp = 0;

static void vm_load_context(uint32_t *from, bool add_pc_4) {
    if(!isSaved){
        if((from >= vm_saved_context) && (from <= &vm_saved_context[17*5 - 1]))
        {
            INFO("Context has been Loaded!\n");
        }
    }
    isSaved = false;

    uint32_t *pRegFram = (uint32_t *)((volatile uint32_t *)vm_sys)[1];
    pRegFram -= 17;
    // memcpy(&pRegFram[1], from, 16 * 4);
    // pRegFram[0] = from[16];
    memcpy(pRegFram, from, 17 * 4);
    pRegFram[0] &= ~(0x1F);
    pRegFram[0] |= 0x10;
    if (add_pc_4) {
        pRegFram[2 + 15] += 4;
    }
}

void vm_save_context() {/*
    if(isSaved){
        INFO("Context has been saved!\n");
    }*/

    isSaved = true;

    uint32_t *pRegFram = (uint32_t *)((volatile uint32_t *)vm_sys)[1];
    pRegFram -= 17;
    // memcpy(vm_saved_context, &pRegFram[1], 16 * 4);
    // vm_saved_context[16] = pRegFram[0];


    memcpy((uint32_t *)(&vm_saved_context[context_ptr]), pRegFram, 17 * 4);
    context_ptr += 17;
}

static void get_saved_context(uint32_t *to_addr)
{
    if(context_ptr < 17)
    {
        INFO("No saved context!\n");
        return;
    }
    context_ptr -= 17;
    memcpy(to_addr, (void *)&vm_saved_context[context_ptr], 17 * 4);
}

void vm_jump_irq() 
{
    if(vm_in_exception && (curExp == 1))
    {
        INFO("Reenter IRQ Exception!\n");
        //return;
    }
    curExp = 1;
    vm_in_exception = true;
    uint32_t *pRegFram = (uint32_t *)((volatile uint32_t *)vm_sys)[1];
    pRegFram -= 16;
    pRegFram[-1] = 0x10;
    pRegFram[15] = vm_irq_vector_address;
    pRegFram[13] = vm_irq_stack_address;
}

static void vm_jump_svc() {/*
    if(vm_in_exception  && (curExp == 2))
    {
        INFO("Reenter SVC Exception!\n");
    }*/
    curExp = 2;
    vm_in_exception = true;
    uint32_t *pRegFram = (uint32_t *)((volatile uint32_t *)vm_sys)[1];
    pRegFram -= 16;
    pRegFram[-1] = 0x10;
    pRegFram[15] = vm_svc_vector_address;
    pRegFram[13] = vm_svc_stack_address;
}

void vm_set_irq_num(uint32_t IRQNum, uint32_t r1, uint32_t r2, uint32_t r3) 
{
    uint32_t *pRegFram = (uint32_t *)((uint32_t *)vm_sys)[1];
    pRegFram -= 16;
    pRegFram[0] = IRQNum;
    pRegFram[1] = r1;
    pRegFram[2] = r2;
    pRegFram[3] = r3;
}

void tickTimer(TimerHandle_t xTimer) {

    LLIRQ_Info_t curIRQ;
    // if ((eTaskStateGet(vm_sys) == eReady))
    {
        if (vm_enable_irq) {
            curIRQ.IRQNum = LL_IRQ_TIMER;
            xQueueSend(LLIRQ_Queue, &curIRQ, portMAX_DELAY);
        }
    }
}
/*
void LLIO_ScanTask(void *pvParameters) {
    uint32_t val;
    LLIRQ_Info_t curIRQ;
    key_register_notify(xTaskGetCurrentTaskHandle());
    for (;;) {
        if (xTaskNotifyWait(0, 0xFFFFFFFF, (uint32_t *)&val, portMAX_DELAY) == pdTRUE) {
            // INFO("VAL:%08x\n", val);
            // g_latest_key_status = val;

            if (vm_key_input_enable && vm_enable_irq) {

                curIRQ.IRQNum = LL_IRQ_KEYBOARD;
                curIRQ.r1 = val & 0xFFFF;
                curIRQ.r2 = val >> 16;
                xQueueSendToFront(LLIRQ_Queue, &curIRQ, portMAX_DELAY);
            }
        }
    }
}
*/
void LLIO_NotifySerialRxAvailable() {
    LLIRQ_Info_t curIRQ;
    if (vm_serial_enable && vm_enable_irq) {
        curIRQ.IRQNum = LL_IRQ_SERIAL;
        curIRQ.r1 = 1;
        xQueueSend(LLIRQ_Queue, &curIRQ, portMAX_DELAY);
    }
}

void LLIO_NotifySerialTxAvailable() {
    LLIRQ_Info_t curIRQ;
    if (vm_serial_enable && vm_enable_irq) {
        curIRQ.IRQNum = LL_IRQ_SERIAL;
        curIRQ.r1 = 2;
        xQueueSend(LLIRQ_Queue, &curIRQ, portMAX_DELAY);
    }
}

void LL_CheckIRQAndTrap() {
    if ((vm_enable_irq == false) || vm_in_exception) {
        return;
    }
    LLIRQ_Info_t curIRQ;
    if (xQueueReceive(LLIRQ_Queue, &curIRQ, 0) != 0) {


        vm_in_exception = true;
        vm_save_context();
        vm_jump_irq();
        vm_set_irq_num(curIRQ.IRQNum, curIRQ.r1, curIRQ.r2, curIRQ.r3);


    }
}

void LLIRQ_task(void *pvParameters) {

    LLIRQ_Info_t curIRQ;
    //vTaskDelay(pdMS_TO_TICKS(100));
    for (;;) {
        while (xQueueReceive(LLIRQ_Queue, &curIRQ, portMAX_DELAY) == pdTRUE) {
            //vTaskSuspendAll();
            /*
            vTaskEnterCritical();
            if((vm_enable_irq == false) ||
                vm_in_exception )
            {
                vTaskExitCritical();
                //xTaskResumeAll();
                continue;
            }*/
            
            /*
            while (
                (vm_enable_irq == false) ||
                vm_in_exception ||
                (eTaskStateGet(vm_sys) != eReady)) {
                
                vTaskDelay(10);
            }*/

            do{
                vTaskDelay(2);
            }while(
                (g_vm_in_pagefault) ||
                (vm_enable_irq == false) ||
                vm_in_exception /*||
                (eTaskStateGet(vm_sys) != eReady)*/
            );

            

            //vm_in_exception = true;

            //vTaskSuspendAll();
            vTaskEnterCritical();
            
            vm_save_context();
            vm_jump_irq();
            vm_set_irq_num(curIRQ.IRQNum, curIRQ.r1, curIRQ.r2, curIRQ.r3);

            vTaskExitCritical();
            //xTaskResumeAll();
        }
    }
}

bool LLIRQ_enable(bool enable) {
    bool ret = vm_enable_irq;
    vm_enable_irq = enable;
    return ret;
}

void LLIRQ_ClearIRQs() {
    if (LLIRQ_Queue) {
        xQueueReset(LLIRQ_Queue);
    }
}

void LLAPI_ClearAPIs() {
    if (LLAPI_Queue) {
        xQueueReset(LLAPI_Queue);
    }
}

void LLAPI_init(TaskHandle_t upSys) {
    vm_sys = upSys;
    LLAPI_Queue = xQueueCreate(8, sizeof(LLAPI_CallInfo_t));
    LLIRQ_Queue = xQueueCreate(32, sizeof(LLIRQ_Info_t));
    vm_timer = xTimerCreate("Tick Timer", pdMS_TO_TICKS(10), pdTRUE, NULL, tickTimer);
}

static uint32_t data_page_buffer[2048 / sizeof(uint32_t)];

bool g_llapi_fin = true;


void __attribute__((target("thumb"))) LLAPI_Task_thumb_entry() {
    LLAPI_CallInfo_t currentCall;
    for (;;) {
        while (xQueueReceive(LLAPI_Queue, &currentCall, portMAX_DELAY) == pdTRUE) {
            g_llapi_fin = false;
            vTaskSuspend(currentCall.task);
            switch (currentCall.SWINum) {
            case LL_SWI_SET_IRQ_STACK:
                vm_irq_stack_address = currentCall.para0;
                LLAPI_INFO("SET vm_irq_stack_address:%08x\n", vm_irq_stack_address);
                break;
            case LL_SWI_SET_IRQ_VECTOR:
                vm_irq_vector_address = currentCall.para0;
                LLAPI_INFO("SET vm_irq_vector_address:%08x\n", vm_irq_vector_address);
                break;
            case LL_SWI_SET_SVC_STACK:
                vm_svc_stack_address = currentCall.para0;
                LLAPI_INFO("SET vm_svc_stack_address:%08x\n", vm_svc_stack_address);
                break;
            case LL_SWI_SET_SVC_VECTOR:
                vm_svc_vector_address = currentCall.para0 + 4;
                LLAPI_INFO("SET vm_svc_vector_address:%08x\n", vm_svc_vector_address);
                break;

            case LL_SWI_ENABLE_TIMER: {
                uint32_t enable = currentCall.para0;
                uint32_t period_ms = currentCall.para1;
                if (enable) {
                    xTimerChangePeriod(vm_timer, pdMS_TO_TICKS(period_ms), 0);
                    xTimerStart(vm_timer, 1);
                } else {
                    xTimerStop(vm_timer, 0);
                }
                LLAPI_INFO("Set Timer:%d,%d ms\n", enable, period_ms);
                break;
            }
            case LL_SWI_ENABLE_IRQ:
                vm_enable_irq = currentCall.para0;
                LLAPI_INFO("enable IRQ:%d\n", vm_enable_irq);
                break;

            case LL_SWI_GET_CONTEXT: {
                uint32_t *to_addr = (uint32_t *)currentCall.para0;
                /*
                                uint32_t moved[17];
                                moved[16] = vm_saved_context[0];
                                memcpy(&moved[0], &vm_saved_context[1], 16 * 4);
                                memcpy(to_addr, moved, sizeof(vm_saved_context));
                */
                //memcpy(to_addr, (uint32_t *)vm_saved_context, sizeof(vm_saved_context));

                get_saved_context(to_addr);
                LLAPI_INFO("GET_CONTEXT:%08x\n", to_addr);
                break;
            }

            case LL_SWI_RESTORE_CONTEXT: {
                uint32_t *from_addr = (uint32_t *)currentCall.para0;

                /*
                                uint32_t moved[17];
                                moved[0] = from_addr[17];
                                memcpy(&moved[1], &from_addr[0], 16 * 4);
                                vm_load_context(moved);
                */
                if (!vmMgr_checkAddressValid(currentCall.para0, PERM_R)) {
                    INFO("FAIL TO RESTORE CONTEXT: MEM CAN NOT READ!\n");
                    break;
                }

                vm_load_context(from_addr, false);
                vm_enable_irq = currentCall.para1;
                vm_in_exception = false;
                curExp = 0;
                LLAPI_INFO("RESTORE_CONTEXT:%08x\n", from_addr);
                break;
            }

            case LL_SWI_SET_CONTEXT: {
                uint32_t *from_addr = (uint32_t *)currentCall.para0;
                /*
                                uint32_t moved[17];
                                moved[0] = from_addr[17];
                                memcpy(&moved[1], &from_addr[0], 16 * 4);
                                vm_load_context(moved);
                */


                vm_load_context(from_addr, false);
                vm_enable_irq = currentCall.para1;
                LLAPI_INFO("SET_CONTEXT:%08x\n", from_addr);
                break;
            }

            case LL_SWI_WRITE_STRING1: {
                if (vmMgr_checkAddressValid(currentCall.para0, PERM_R)) {
                    char *s = (char *)currentCall.para0;
                    int len = 400;
                    while (len-- && *s) {
                        if ((g_CDC_TransTo == CDC_PATH_SYS)) {
                            tud_cdc_write_char(*s);
                        }
                        putchar(*s++);
                    }
                    if ((g_CDC_TransTo == CDC_PATH_SYS))
                        tud_cdc_write_flush();
                }
                break;
            }

            case LL_SWI_WRITE_STRING2: {
                if (vmMgr_checkAddressValid(currentCall.para0, PERM_R) && vmMgr_checkAddressValid(currentCall.para0 + currentCall.para1, PERM_R)) {
                    char *s = (char *)currentCall.para0;
                    int len = currentCall.para1;
                    while (len--) {
                        if ((g_CDC_TransTo == CDC_PATH_SYS)) {
                            tud_cdc_write_char(*s);
                        }
                        putchar(*s++);
                    }
                    if ((g_CDC_TransTo == CDC_PATH_SYS))
                        tud_cdc_write_flush();
                }
                break;
            }

            case LL_SWI_PUT_CH: {
                putchar(currentCall.para0);
                if ((g_CDC_TransTo == CDC_PATH_SYS)) {
                    tud_cdc_write_char(currentCall.para0);
                    tud_cdc_write_flush();
                }
                break;
            }

            case LL_SWI_SERIAL_RX_COUNT: {
                *currentCall.pRet = tud_cdc_available();
                INFO("get cnt:%ld\n", *currentCall.pRet);
            } break;

            case LL_SWI_SERIAL_GETCH: {
                *currentCall.pRet = tud_cdc_read_char();
            } break;

            case LL_SWI_SET_KEY_REPORT: {
                vm_key_input_enable = currentCall.para0;
            } break;

            case LL_SWI_SET_SERIALPORT: {
                vm_serial_enable = currentCall.para0;
            } break;

            case LL_SWI_DISPLAY_SET_INDICATION:
            {
                DisplaySetIndicate(currentCall.para0, currentCall.para1);
            }
            break;

            case LL_SWI_DISPLAY_FLUSH:

            {
                /*
                if (currentCall.para0 % 4 != 0) {
                    break;
                }*/

                if ((!vmMgr_checkAddressValid(currentCall.para0, PERM_R)) || (!vmMgr_checkAddressValid(currentCall.sp, PERM_R))) {
                    break;
                }
                

                uint8_t *vrambuf = (uint8_t *)currentCall.para0;
                uint16_t x0 = currentCall.para1;
                uint16_t y0 = currentCall.para2;
                uint16_t x1 = currentCall.para3;
                uint16_t y1 = ((uint32_t *)currentCall.sp)[0] - 0;

                if((y0 >= 127) || (y1 >= 127)
                || (x0 >= 256) || (x1 >= 256)
                ){
                    break;
                }

                long bufSize = (y1 - y0 + 1) * (x1 - x0 + 1);
                if ((bufSize <= 0) || (bufSize > 33 * 1024)) {
                    break;
                }
                
                //portDispFlushAreaBuf(x0, y0, x1, y1, (uint8_t *)vrambuf);
                DisplayFlushArea(x0, y0, x1, y1, (uint8_t *)vrambuf, false);

            } break;

            case LL_SWI_CLKCTL_GET_DIV: {
                if ((!vmMgr_checkAddressValid(currentCall.para0, PERM_W)) ||
                    (!vmMgr_checkAddressValid(currentCall.para0 + 4, PERM_R)) ||
                    (!vmMgr_checkAddressValid(currentCall.para0 + 8, PERM_R))) {
                    break;
                }
                portGetCoreFreqDIV(
                    (uint32_t *)(currentCall.para0),
                    (uint32_t *)(currentCall.para0 + 4),
                    (uint32_t *)(currentCall.para0 + 8));

            } break;

            case LL_SWI_CLKCTL_SET_DIV: {
                uint32_t CPU_DIV = currentCall.para0;
                uint32_t CPU_FRAC = currentCall.para1;
                uint32_t HCLK_DIV = currentCall.para2;
                if ((CPU_DIV == 0) || (CPU_DIV > 1024)) {
                    break;
                }
                if ((CPU_FRAC < 18) || (CPU_FRAC > 35)) {
                    break;
                }
                if ((HCLK_DIV == 0) || (HCLK_DIV > 35)) {
                    break;
                }
                setHCLKDivider(HCLK_DIV);
                setCPUDivider(CPU_DIV);
                setCPUFracDivider(CPU_FRAC);

            } break;

            case LL_SWI_FLASH_PAGE_READ: { // start page, pages, buf
                int ret = 0;
                uint32_t spage = currentCall.para0;
                uint32_t pages = currentCall.para1;
                uint32_t *buffer = (uint32_t *)currentCall.para2;
                if ((!vmMgr_checkAddressValid(currentCall.para2, PERM_R)) ||
                    (!vmMgr_checkAddressValid(currentCall.para2 + (currentCall.para1 + 1) * 2048 - 1, PERM_R))) {
                    break;
                }
                LLAPI_INFO("VM Read:%d, pages:%d, buf:%08x\n", currentCall.para0, currentCall.para1, currentCall.para2);
                while (pages) {
                    ret = FTL_ReadSector(FLASH_FTL_DATA_SECTOR + spage, 1, (uint8_t *)data_page_buffer);
                    memcpy(buffer, data_page_buffer, 2048);
                    buffer += (2048 / sizeof(uint32_t));
                    spage++;
                    pages--;
                    if (ret) {
                        INFO("LL_SWI_FLASH_PAGE_READ FAIL:%d\n", ret);
                        *currentCall.pRet = ret;
                        break;
                    }
                }
                *currentCall.pRet = ret;
            } break;

            case LL_SWI_FLASH_PAGE_WRITE: { // start page, pages, buf
                int ret = 0;
                uint32_t spage = currentCall.para0;
                uint32_t pages = currentCall.para1;
                uint32_t *buffer = (uint32_t *)currentCall.para2;
                if ((!vmMgr_checkAddressValid(currentCall.para2, PERM_R)) ||
                    (!vmMgr_checkAddressValid(currentCall.para2 + (currentCall.para1 + 1) * 2048 - 1, PERM_R))) {
                    break;
                }
                LLAPI_INFO("VM Write:%d, pages:%d, buf:%08x\n", currentCall.para0, currentCall.para1, currentCall.para2);
                while (pages) {
                    memcpy(data_page_buffer, buffer, 2048);
                    ret = FTL_WriteSector(FLASH_FTL_DATA_SECTOR + spage, 1, (uint8_t *)data_page_buffer);
                    buffer += (2048 / sizeof(uint32_t));
                    spage++;
                    pages--;
                    if (ret) {
                        INFO("LL_SWI_FLASH_PAGE_WRITE FAIL:%d\n", ret);
                        *currentCall.pRet = ret;
                        break;
                    }
                }
                *currentCall.pRet = ret;
            } break;

            case LL_SWI_FLASH_PAGE_TRIM: {
                FTL_TrimSector(FLASH_FTL_DATA_SECTOR + currentCall.para0);
            } break;

            case LL_SWI_FLASH_PAGE_NUM: {
                *currentCall.pRet = FTL_GetSectorCount() - FLASH_FTL_DATA_SECTOR;
            } break;

            case LL_SWI_FLASH_PAGE_SIZE_B: {
                *currentCall.pRet = FTL_GetSectorSize();
            } break;

            case LL_SWI_FLASH_SYNC: {
                FTL_Sync();
            }break;

            case LL_SWI_CHARGE_ENABLE: 
            {
                portChargeEnable(currentCall.para0);
            }break;

            case LL_SWI_SLOW_DOWN_ENABLE:
            {
                slowDownEnable(currentCall.para0);
            }break;

            case LL_SWI_PWR_SPEED:
            {
                *currentCall.pRet = portGetPWRSpeed();
            }break;

            case LL_SWI_SLOW_DOWN_MINFRAC:
            {
                setSlowDownMinCpuFrac(currentCall.para0);
            }break;

            case LL_SWI_PWR_POWEROFF:
            {
                
                portBoardPowerOff();
            }break;

            default: {

                // while (vm_in_exception) {
                //     vTaskDelay(4);
                // }

                //vm_in_exception = true;
                vTaskEnterCritical();
                vm_save_context();
                vm_jump_svc();
                vTaskExitCritical();
            } break;
            }

            vTaskResume(currentCall.task);
            g_llapi_fin = true;
        }
    }
}


void LLAPI_Task()
{
    LLAPI_Task_thumb_entry();
}

