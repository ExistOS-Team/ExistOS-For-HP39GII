
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "timers.h"

#include "SystemConfig.h"
#include <stdio.h>

#include "debug.h"
#include "interrupt_up.h"
#include "portmacro.h"

#include "mmu.h"
#include "vmMgr.h"

#include "board_up.h"
#include "llapi.h"
#include "llapi_code.h"

#include "rtc_up.h"

extern volatile void *pxCurrentTCB;
extern volatile uint32_t ulCriticalNesting;
uint32_t
    __attribute__((naked))
    _get_sp() {
    asm volatile("mov r0,sp");
    asm volatile("bx lr");
}

#define SAVE_CONTEXT()                                                                                                \
    asm volatile("STMDB   SP, {R0-R14}^"); /*{NESTING_CNT, SPSR, R0-R15 <- } 16*4*/                                   \
    asm volatile("STR     LR, [SP]");                                                                                 \
    asm volatile("MRS     R0, SPSR");                                                                                 \
    asm volatile("STR     R0, [SP, #-64]"); /*{NESTING_CNT, SPSR <-, R0-R15 } 16*4*/                                  \
    asm volatile("LDR     R0, =ulCriticalNesting");                                                                   \
    asm volatile("LDR     R0, [R0]");                                                                                 \
    asm volatile("STR     R0, [SP, #-68]"); /*{NESTING_CNT <-, SPSR, R0-R15 } 16*4*/                                  \
    asm volatile("SUB     SP, SP, #68");                                                                              \
    uint32_t *context = (uint32_t *)_get_sp();                                                                        \
    uint32_t *pRegFram = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];                                                   \
    memcpy(pRegFram, context, 18 * 4);                                                                                \
    ((uint32_t *)pxCurrentTCB)[1] = ((uint32_t *)pxCurrentTCB)[1] + 18 * 4; /*Set The Registers frame stack pointer*/ \
    ((uint32_t *)pxCurrentTCB)[0] = context[13 + 2];                        /*Set the task stack pointer*/

unsigned int faultAddress;
volatile unsigned int insAddress;
unsigned int FSR;
volatile uint32_t swapping = 0;
extern bool g_vm_in_pagefault;
extern TaskHandle_t upSystem;
extern uint32_t savedBy;

uint32_t _dump_REGS[16];
void __dumpRegs() __attribute__((naked));
void __dumpRegs() {
    __asm volatile("PUSH    {R14}");
    __asm volatile("LDR     R14, =_dump_REGS");
    __asm volatile("ADD		R14,R14,#60");
    __asm volatile("STMDB   R14, {R0-R14}^");
    __asm volatile("POP     {R14}");
    __asm volatile("bx      lr");
}

void printRegs() {
    for (int i = 0; i < 15; i++) {
        printf("REG %d:%08lx\n", i, _dump_REGS[i]);
    }
    printf("\n");
}

uint32_t vm_temp_storage[16];
void waitIRQ(int r);
extern uint32_t g_latest_key_status;
extern uint32_t g_core_temp, g_batt_volt, g_core_cur_freq_mhz;
extern bool vm_in_exception, g_chargeEnable;

bool is_pcm_buffer_idle();
void pcm_buffer_load(void *pcmdat);

void volatile arm_do_swi(uint32_t SWINum, uint32_t *pRegFram) {

    LLAPI_CallInfo_t currentCall;
    BaseType_t SwitchContext;

    switch (SWINum >> 8) {
    case 0xEF:

        switch (SWINum) {
        case LL_FAST_SWI_GET_STVAL:
            if (pRegFram[0 + 2] < 16) {
                pRegFram[0 + 2] = vm_temp_storage[0];
            }
            break;
        case LL_FAST_SWI_SET_STVAL:
            if (pRegFram[0 + 2] < 16) {
                vm_temp_storage[pRegFram[0 + 2]] = pRegFram[1 + 2];
            }
            break;

        case LL_FAST_SWI_GET_TIME_US:
            pRegFram[0 + 2] = portBoardGetTime_us();
            break;
        case LL_FAST_SWI_GET_TIME_MS:
            pRegFram[0 + 2] = portBoardGetTime_ms();
            break;
        case LL_FAST_SWI_VM_SLEEP_MS:
            vTaskDelayInSWI(pdMS_TO_TICKS(pRegFram[0 + 2]));
            vTaskSwitchContext();
            break;
        case LL_FAST_SWI_CHECK_KEY:
            pRegFram[0 + 2] = g_latest_key_status;
            break;

        case LL_FAST_SWI_PWR_VOLTAGE:
            pRegFram[0 + 2] = g_batt_volt;
            break;
        case LL_FAST_SWI_CORE_TEMP:
            pRegFram[0 + 2] = g_core_temp;
            break;

        case LL_FAST_SWI_SYSTEM_IDLE:
            waitIRQ(0);
            break;

        case LL_FAST_SWI_CORE_CUR_FREQ:
            pRegFram[0 + 2] = g_core_cur_freq_mhz;
            break;

        case LL_FAST_SWI_GET_CHARGE_STATUS:
            pRegFram[0 + 2] = g_chargeEnable;
            break;

        case LL_FAST_SWI_RTC_GET_SEC:
            pRegFram[0 + 2] = rtc_get_seconds();
            break;
        case LL_FAST_SWI_RTC_SET_SEC:
            rtc_set_seconds(pRegFram[0 + 2]);
            break;

        case LL_FAST_SWI_PCM_BUFFER_IS_IDLE:
            pRegFram[0 + 2] = 1;
#ifdef ENABLE_AUIDIOOUT
            pRegFram[0 + 2] = is_pcm_buffer_idle();
#endif
            break;

        case LL_FAST_SWI_PCM_BUFFER_PLAY:

#ifdef ENABLE_AUIDIOOUT
            pcm_buffer_load((void *)pRegFram[0 + 2]);
#endif
            break;

        default:
            break;
        }

        break;

    case 0xAC:
    case 0:
    case 0xEE:
        // vm_in_exception = true;
        currentCall.task = xTaskGetCurrentTaskHandle();
        currentCall.SWINum = SWINum;
        currentCall.para0 = pRegFram[0 + 2];
        currentCall.para1 = pRegFram[1 + 2];
        currentCall.para2 = pRegFram[2 + 2];
        currentCall.para3 = pRegFram[3 + 2];
        currentCall.sp = pRegFram[13 + 2];
        currentCall.pRet = &pRegFram[0 + 2];
        xQueueSendFromISR(LLAPI_Queue, &currentCall, &SwitchContext);
    default:
        vTaskSwitchContext();
        break;
    }
}

void volatile arm_vector_swi() __attribute__((naked));
void volatile arm_vector_swi() {
    __asm volatile("ADD   LR, LR, #4");
    SAVE_CONTEXT();

    uint32_t SWINum = *((uint32_t *)(context[15 + 2] - 8)) & 0x00FFFFFF;
    arm_do_swi(SWINum, pRegFram);

    asm volatile("ADD     SP, SP, #68");
    portRESTORE_CONTEXT();
}

volatile uint32_t IRQ_REGS_Frame[18];

void volatile arm_vector_irq() __attribute__((naked));
void volatile arm_vector_irq() {
    SAVE_CONTEXT();

    up_isr();

    asm volatile("ADD     SP, SP, #68");
    portRESTORE_CONTEXT();
}

void VM_Unconscious(TaskHandle_t task, char *res, uint32_t address);

void volatile arm_vector_und() __attribute__((naked));
void volatile arm_vector_und() {
    __asm volatile("SUB   LR, LR, #4");
    SAVE_CONTEXT();

    printf("ERROR: [%s] Undefind Instruction AT:%08lx\n", pcTaskGetName(xTaskGetCurrentTaskHandle()), pRegFram[2 + 15]);

    printf("NESTING:%ld\nSPSR:%08lx\n", pRegFram[0], pRegFram[1]);
    for (int i = 2; i < 18; i++) {
        printf("R%d: %08lx\n", i - 2, pRegFram[i]);
    }

    VM_Unconscious(NULL, "[UND]", pRegFram[2 + 15]);
    vTaskSuspend(NULL);

    asm volatile("ADD     SP, SP, #68");
    portRESTORE_CONTEXT();
}

void volatile arm_do_dab(uint32_t *context) {
    __asm volatile("PUSH	{R0}");
    __asm volatile("mrc p15, 0, r0, c6, c0, 0");
    __asm volatile("str r0,%0"
                   : "=m"(faultAddress));
    __asm volatile("mrc p15, 0, r0, c5, c0, 0");
    __asm volatile("str r0,%0"
                   : "=m"(FSR));
    __asm volatile("POP	{R0}");

    pageFaultInfo_t FaultInfo;
    FaultInfo.FaultTask = xTaskGetCurrentTaskHandle();
    FaultInfo.FaultMemAddr = faultAddress;

    switch (FSR & 0xF) {
    case 0x1:
    case 0x3:
        FaultInfo.FSR = FSR_DATA_UNALIGN;
        break;
    case 0x5:
    case 0x7:
        FaultInfo.FSR = FSR_DATA_ACCESS_UNMAP_DAB;
        break;
    case 0xD:
    case 0xF:
        FaultInfo.FSR = FSR_DATA_WR_RDONLY;
        break;
    default:
        FaultInfo.FSR = FSR_UNKNOWN;
        printf("TASK [%s] UNKNOWN DAB. AT:%08lx access %08x, FSR:%08x\n", pcTaskGetName(NULL), context[15 + 2], faultAddress, FSR);
        break;
    }

    FAULT_INFO("TASK [%s] DAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL), context[15 + 2], faultAddress, FSR);

    BaseType_t SwitchContext;
    g_vm_in_pagefault = true;
    xQueueSendFromISR(PageFaultQueue, &FaultInfo, &SwitchContext);

    if (SwitchContext) {
        vTaskSwitchContext();
    }
}

void volatile arm_vector_dab() __attribute__((naked));
void volatile arm_vector_dab() {

    __asm volatile("SUB   LR, LR, #4");
    SAVE_CONTEXT();

    arm_do_dab(context);

    asm volatile("ADD     SP, SP, #68");
    portRESTORE_CONTEXT();
}

void volatile arm_do_pab(uint32_t *context) {

    pageFaultInfo_t FaultInfo;
    FaultInfo.FaultTask = xTaskGetCurrentTaskHandle();
    FaultInfo.FaultMemAddr = context[15 + 2] - 4;
    FaultInfo.FSR = FSR_DATA_ACCESS_UNMAP_PAB;

    // printf("TASK [%s] PAB. AT:%08x\n", pcTaskGetName(NULL),context[15 + 2] - 4);
    // printf("TASK [%s] PAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL), DAB_REGS_Frame[16], faultAddress, FSR);

    BaseType_t SwitchContext;
    // printf("\n");
    // printf("TASK [%s] PAB Sent\n", pcTaskGetName(NULL) );
    // vTaskSuspend(FaultInfo.FaultTask);
    g_vm_in_pagefault = true;
    xQueueSendFromISR(PageFaultQueue, &FaultInfo, &SwitchContext);
    if (SwitchContext) {
        vTaskSwitchContext();
    }
}

void volatile arm_vector_pab() __attribute__((naked));
void volatile arm_vector_pab() {

    SAVE_CONTEXT();
    arm_do_pab(context);

    asm volatile("ADD     SP, SP, #68");
    portRESTORE_CONTEXT();
}

void volatile arm_vector_fiq() __attribute__((naked));
void volatile arm_vector_fiq() {

    /*
    SAVE_CONTEXT();

    up_isr();

    asm volatile("ADD     SP, SP, #68");
    portRESTORE_CONTEXT();
*/

    printf("ERROR: FIQ Mode Unsupported.\n");
    while (1)
        ;
}
