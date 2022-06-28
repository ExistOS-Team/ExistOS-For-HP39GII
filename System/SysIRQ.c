
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "llapi_code.h"
#include "sys_llapi.h"

uint32_t Timer_Count = 0;

extern uint32_t g_key;
extern uint32_t g_ket_press;

uint32_t irq_saved_context[32]; //{SPSR , R0-R15}
uint32_t swi_saved_context[32]; //{SPSR , R0-R15}

extern volatile void *volatile pxCurrentTCB;
extern volatile uint32_t ulCriticalNesting;

//volatile void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3) __attribute__((naked));
volatile void IRQ_ISR(uint32_t IRQNum, uint32_t par1, uint32_t par2, uint32_t par3) {
    
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    
    ll_get_context((uint32_t)irq_saved_context);

    volatile uint32_t *cur_task_sp = (uint32_t *)irq_saved_context[1 + 13];
    cur_task_sp -= 21;
    memcpy((uint32_t *)cur_task_sp, irq_saved_context, 17 * 4);
    cur_task_sp--;
    *cur_task_sp = ll_get_tmp_storage_val(0);
    ((volatile uint32_t *)pxCurrentTCB)[0] = (uint32_t)cur_task_sp;

    //printf("IRQ B,Task:%08x,Stack:%08x\n",pxCurrentTCB, ((volatile uint32_t *)pxCurrentTCB)[0]);
    switch (IRQNum) {
    case LL_IRQ_TIMER:
        Timer_Count++;
        //printf("tick\n");
        if (xTaskIncrementTick() != pdFALSE) {
            vTaskSwitchContext();
        }
        break;
    case LL_IRQ_KEYBOARD:
        printf("Key:%d, %d\n", par1, par2);
        g_key = par1;
        g_ket_press = par2;
        break;

    default:
        break;
    }

    cur_task_sp = (uint32_t *)(((uint32_t *)pxCurrentTCB)[0]);
	((uint32_t *)pxCurrentTCB)[0] += 21 * 4;
    ulCriticalNesting = *cur_task_sp;
    cur_task_sp++;
    ll_set_tmp_storage_val(0, ulCriticalNesting);
    ll_restore_context((uint32_t)cur_task_sp, ulCriticalNesting == 0);

    while (1)
        ;
}

volatile void SWI_ISR() __attribute__((naked));
volatile void SWI_ISR() {
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");

    ll_get_context((uint32_t)swi_saved_context);

    volatile uint32_t *cur_task_sp = (uint32_t *)swi_saved_context[1 + 13];
    cur_task_sp -= 20;
    memcpy((uint32_t *)cur_task_sp, swi_saved_context, 17 * 4);
    cur_task_sp--;
    *cur_task_sp = ll_get_tmp_storage_val(0);
    //cur_task_sp--;
    ((volatile uint32_t *)pxCurrentTCB)[0] = (uint32_t)cur_task_sp;


    vTaskSwitchContext();

    cur_task_sp = (uint32_t *)(((uint32_t *)pxCurrentTCB)[0]);
	((uint32_t *)pxCurrentTCB)[0] += 21 * 4;
    ulCriticalNesting = *cur_task_sp;
    cur_task_sp++;
    ll_set_tmp_storage_val(0, ulCriticalNesting);
    ll_restore_context((uint32_t)cur_task_sp, ulCriticalNesting == 0);

    while (1)
        ;
}
