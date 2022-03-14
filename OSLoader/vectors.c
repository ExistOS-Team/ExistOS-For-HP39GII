
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "timers.h"

#include "SystemConfig.h"
#include <stdio.h>

#include "interrupt_up.h"
#include "portmacro.h"
#include "debug.h"

#include "vmMgr.h"
#include "mmu.h"


#include "llapi.h"
#include "llapi_code.h"

extern volatile void * volatile pxCurrentTCB;	
extern volatile uint32_t ulCriticalNesting;	

unsigned int faultAddress;
volatile unsigned int insAddress;
unsigned int FSR;
volatile uint32_t swapping = 0;

extern TaskHandle_t upSystem;
extern uint32_t savedBy;

uint32_t sysIRQcnt = 0;

uint32_t _dump_REGS[16];

void __dumpRegs() __attribute__((naked));
void __dumpRegs()
{
    __asm volatile("PUSH    {R14}");
	__asm volatile("LDR     R14, =_dump_REGS");
	__asm volatile("ADD		R14,R14,#60");
	__asm volatile("STMDB   R14, {R0-R14}^");
    __asm volatile("POP     {R14}");
    __asm volatile("bx      lr");

}

void printRegs()
{
    for(int i=0;i<15;i++)
        {
            printf("REG %d:%08x\n",i,_dump_REGS[i]);
        }
        printf("\n");
}


uint32_t SWI_REGS_Frame[18];
TaskHandle_t beComp = (TaskHandle_t)0x12345678;
uint32_t send = 0;

void volatile arm_vector_swi() __attribute__((naked));
void volatile arm_vector_swi()
{
    __asm volatile ( "ADD   LR, LR, #4" );

	__asm volatile("STMDB   SP, {R0-R14}^");
	__asm volatile("LDR     R0, =SWI_REGS_Frame");
	__asm volatile("ADD		R0, R0, #60");
	__asm volatile("STMDB   R0, {R1-R14}^");
	__asm volatile("MRS		R1,SPSR");
	__asm volatile("STR		R1,[R0]");

    __asm volatile("STR		LR,[R0, #4]");
	__asm volatile("LDR		R1,[SP, #-60]");
	__asm volatile("STR		R1,[R0, #-60]");

    
    volatile uint32_t *pRegFram = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
    pRegFram[0] = ulCriticalNesting;    //Save Interrupt flags
    pRegFram[1] = SWI_REGS_Frame[15];       //Save task SPSR
    pRegFram[17] = SWI_REGS_Frame[16];      //Save Exception return LR
//Copy registers to task TCB.
    __asm volatile("PUSH	{R0-R12,R14}");
    __asm volatile("LDR		R0,=pxCurrentTCB");
    __asm volatile("LDR		R0,[R0]");
    __asm volatile("LDR		R0,[R0, #+4]");
    __asm volatile("ADD		R0, R0, #8");

    __asm volatile("LDR		R1,=SWI_REGS_Frame");       // Save R0-R14:
    __asm volatile("LDMIA	R1!,{R2-R12, R14}");    //for(int i=2;i<17;i++){
    __asm volatile("STMIA	R0!,{R2-R12, R14}");    //    pRegFram[i] = SWI_REGS_Frame[i - 2];
    __asm volatile("LDMIA	R1,{R2-R4}");           //}
    __asm volatile("STMIA	R0,{R2-R4}");
    __asm volatile("POP		{R0-R12,R14}");
//Copy end.

    ((uint32_t *)pxCurrentTCB)[1] = ((uint32_t *)pxCurrentTCB)[1] + 18*4;   //Reset Registers frame stack pointer
    ((uint32_t *)pxCurrentTCB)[0] = SWI_REGS_Frame[13];                         //Reset task stack pointer

    if(pxCurrentTCB == upSystem){
        LL_Scheduler_(L_SWI, (uint32_t *)&pRegFram[2]);
    }

    {
        vTaskSwitchContext();
    }


    portRESTORE_CONTEXT();
}

volatile uint32_t IRQ_REGS_Frame[18];



void arm_vector_irq()  __attribute__((naked));
void arm_vector_irq()
{
    
	__asm volatile("STMDB   SP, {R0-R14}^");
    __asm volatile("SUB		SP, SP, #60");
	__asm volatile("LDR     R0, =IRQ_REGS_Frame");
	__asm volatile("ADD		R0, R0, #60");
	__asm volatile("STMDB   R0, {R1-R14}^");
	__asm volatile("MRS		R1, SPSR");
	__asm volatile("STR		R1, [R0]");

    __asm volatile("STR		LR, [R0, #4]");
	__asm volatile("LDR		R1, [SP]");         //Copy R0
	__asm volatile("STR		R1, [R0, #-60]");
    __asm volatile("ADD		SP, SP, #60");

    volatile uint32_t *pRegFram = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
    pRegFram[0] = ulCriticalNesting;    //Save Interrupt flags
    pRegFram[1] = IRQ_REGS_Frame[15];       //Save task SPSR
    pRegFram[17] = IRQ_REGS_Frame[16];      //Save Exception return LR

//Copy registers to task TCB.
    __asm volatile("PUSH	{R0-R12,R14}");
    __asm volatile("LDR		R0,=pxCurrentTCB");
    __asm volatile("LDR		R0,[R0]");
    __asm volatile("LDR		R0,[R0, #+4]");
    __asm volatile("ADD		R0, R0, #8");

    __asm volatile("LDR		R1,=IRQ_REGS_Frame");       // Save R0-R14:
    __asm volatile("LDMIA	R1!,{R2-R12, R14}");        //for(int i=2;i<17;i++){
    __asm volatile("STMIA	R0!,{R2-R12, R14}");        //    pRegFram[i] = IRQ_REGS_Frame[i - 2];
    __asm volatile("LDMIA	R1,{R2-R4}");               //}
    __asm volatile("STMIA	R0,{R2-R4}");
    __asm volatile("POP		{R0-R12,R14}");
//Copy end.

    ((uint32_t *)pxCurrentTCB)[1] = ((uint32_t *)pxCurrentTCB)[1] + 18*4;   //Reset Registers frame stack pointer
    ((uint32_t *)pxCurrentTCB)[0] = IRQ_REGS_Frame[13];                         //Reset task stack pointer
    

    if(pxCurrentTCB == upSystem){
        LL_Scheduler_(L_IRQ, (uint32_t *)&pRegFram[2]);
    }

    if(up_isr())
    {
        //vTaskSwitchContext();
    }

    portRESTORE_CONTEXT();

}


volatile uint32_t UND_REGS_Frame[18];

void arm_vector_und()  __attribute__((naked));
void arm_vector_und()
{
    

    __asm volatile("SUB   LR, LR, #4" );

	__asm volatile("STMDB   SP, {R0-R14}^");
    __asm volatile("SUB		SP, SP, #60");
	__asm volatile("LDR     R0, =UND_REGS_Frame");
	__asm volatile("ADD		R0, R0, #60");
	__asm volatile("STMDB   R0, {R1-R14}^");
	__asm volatile("MRS		R1, SPSR");
	__asm volatile("STR		R1, [R0]");

    __asm volatile("STR		LR, [R0, #+4]");
	__asm volatile("LDR		R1, [SP]");         //Copy R0
	__asm volatile("STR		R1, [R0, #-60]");
    __asm volatile("ADD		SP, SP, #60");

    volatile uint32_t *pRegFram = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
    pRegFram[0] = ulCriticalNesting;    //Save Interrupt flags
    pRegFram[1] = UND_REGS_Frame[15];       //Save task SPSR
    pRegFram[17] = UND_REGS_Frame[16];      //Save Exception return LR


//Copy registers to task TCB.
    __asm volatile("PUSH	{R0-R12,R14}");
    __asm volatile("LDR		R0,=pxCurrentTCB");
    __asm volatile("LDR		R0,[R0]");
    __asm volatile("LDR		R0,[R0, #4]");
    __asm volatile("ADD		R0, R0, #8");

    __asm volatile("LDR		R1,=UND_REGS_Frame");       // Save R0-R14:
    __asm volatile("LDMIA	R1!,{R2-R12, R14}");        //for(int i=2;i<17;i++){
    __asm volatile("STMIA	R0!,{R2-R12, R14}");        //    pRegFram[i] = UND_REGS_Frame[i - 2];
    __asm volatile("LDMIA	R1,{R2-R4}");               //}
    __asm volatile("STMIA	R0,{R2-R4}");
    __asm volatile("POP		{R0-R12,R14}");
//Copy end.
    
    ((uint32_t *)pxCurrentTCB)[1] = ((uint32_t *)pxCurrentTCB)[1] + 18*4;   //Reset Registers frame stack pointer
    ((uint32_t *)pxCurrentTCB)[0] = UND_REGS_Frame[13];                         //Reset task stack pointer



    printf("ERROR: [%s] Undefind Instruction AT:%08x\n",pcTaskGetName(xTaskGetCurrentTaskHandle()) , UND_REGS_Frame[14]);

    for(int i =0; i< 18; i++)
    {
        printf("REG%d: %08x\n", i, pRegFram[i]);
    }
    

    vTaskSuspend(NULL);

    //__dumpRegs();printRegs();
    //vTaskSwitchContext();
    //while(1);

    portRESTORE_CONTEXT();

}


volatile uint32_t DAB_REGS_Frame[18];

void arm_vector_dab()  __attribute__((naked));
void arm_vector_dab()
{

    __asm volatile("SUB   LR, LR, #4" );

	__asm volatile("STMDB   SP, {R0-R14}^");
    __asm volatile("SUB		SP, SP, #60");
	__asm volatile("LDR     R0, =DAB_REGS_Frame");
	__asm volatile("ADD		R0, R0, #60");
	__asm volatile("STMDB   R0, {R1-R14}^");
	__asm volatile("MRS		R1, SPSR");
	__asm volatile("STR		R1, [R0]");

    __asm volatile("STR		LR, [R0, #+4]");
	__asm volatile("LDR		R1, [SP]");         //Copy R0
	__asm volatile("STR		R1, [R0, #-60]");
    __asm volatile("ADD		SP, SP, #60");

    volatile uint32_t *pRegFram = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
    pRegFram[0] = ulCriticalNesting;    //Save Interrupt flags
    pRegFram[1] = DAB_REGS_Frame[15];       //Save task SPSR
    pRegFram[17] = DAB_REGS_Frame[16];      //Save Exception return LR
 
    

//Copy registers to task TCB.
    __asm volatile("PUSH	{R0-R12,R14}");
    __asm volatile("LDR		R0,=pxCurrentTCB");
    __asm volatile("LDR		R0,[R0]");
    __asm volatile("LDR		R0,[R0, #4]");
    __asm volatile("ADD		R0, R0, #8");

    __asm volatile("LDR		R1,=DAB_REGS_Frame");       // Save R0-R14:
    __asm volatile("LDMIA	R1!,{R2-R12, R14}");        //for(int i=2;i<17;i++){
    __asm volatile("STMIA	R0!,{R2-R12, R14}");        //    pRegFram[i] = DAB_REGS_Frame[i - 2];
    __asm volatile("LDMIA	R1,{R2-R4}");               //}
    __asm volatile("STMIA	R0,{R2-R4}");
    __asm volatile("POP		{R0-R12,R14}");
//Copy end.

    ((uint32_t *)pxCurrentTCB)[1] = ((uint32_t *)pxCurrentTCB)[1] + 18*4;   //Reset Registers frame stack pointer
    ((uint32_t *)pxCurrentTCB)[0] = DAB_REGS_Frame[13];                         //Reset task stack pointer
    
    if(pxCurrentTCB == upSystem){
        LL_Scheduler_(L_DAB,  (uint32_t *)&pRegFram[2]);
    }


    swapping = 1;

    __asm volatile("PUSH	{R0}");
    __asm volatile("mrc p15, 0, r0, c6, c0, 0");
    __asm volatile("str r0,%0"  : "=m"(faultAddress));
    __asm volatile("mrc p15, 0, r0, c5, c0, 0"); 
    __asm volatile("str r0,%0"  : "=m"(FSR));
    __asm volatile("POP	{R0}");

    
    
    pageFaultInfo_t FaultInfo;
    FaultInfo.FaultTask = xTaskGetCurrentTaskHandle();
    FaultInfo.FaultMemAddr = faultAddress;
    
    switch (FSR & 0xF)
    {
    case 0x1:
    case 0x3:
        FaultInfo.FSR = FSR_DATA_UNALIGN;    
        break;
    case 0x5:
    case 0x7:
        FaultInfo.FSR = FSR_DATA_ACCESS_UNMAP;
        break;
    case 0xD:
    case 0xF:
        FaultInfo.FSR = FSR_DATA_WR_RDONLY;
        break;
    default:
        FaultInfo.FSR = FSR_UNKNOWN;
        printf("TASK [%s] DAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL), DAB_REGS_Frame[16], faultAddress, FSR);
        break;
    }

    FAULT_INFO("TASK [%s] DAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL), DAB_REGS_Frame[16], faultAddress, FSR);

    BaseType_t SwitchContext;

    xQueueSendFromISR(PageFaultQueue, &FaultInfo, &SwitchContext);
    if(xTaskGetCurrentTaskHandle() == upSystem){
        savedBy = 2;
    }
    //if(SwitchContext)
    {
        vTaskSwitchContext();
    }

    swapping = 2;

    if(xTaskGetCurrentTaskHandle() == beComp)
    {
        volatile uint32_t *pReg = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
        pReg-=18;
        //INFO("DAB Start Schedule R13:%08x\n", pReg[13 + 2]);
    }

    portRESTORE_CONTEXT();

}


volatile uint32_t PAB_REGS_Frame[18];

void arm_vector_pab()  __attribute__((naked));
void arm_vector_pab()
{

	__asm volatile("STMDB   SP, {R0-R14}^");
/*
    __asm volatile("MRS		R0, CPSR");
    __asm volatile("ORR		R0, R0, #0xC0");
    __asm volatile("MSR		CPSR, R0");*/

    __asm volatile("SUB		SP, SP, #60");
	__asm volatile("LDR     R0, =PAB_REGS_Frame");
	__asm volatile("ADD		R0, R0, #60");
	__asm volatile("STMDB   R0, {R1-R14}^");
	__asm volatile("MRS		R1, SPSR");
	__asm volatile("STR		R1, [R0]");

    __asm volatile("STR		LR, [R0, #+4]");
	__asm volatile("LDR		R1, [SP]");         //Copy R0
	__asm volatile("STR		R1, [R0, #-60]");
    __asm volatile("ADD		SP, SP, #60");



    __asm volatile("PUSH	{R0}");
    asm volatile("mrc p15, 0, r0, c6, c0, 0");
    asm volatile("str r0,%0"  : "=m"(faultAddress));
    asm volatile("mrc p15, 0, r0, c5, c0, 0"); 
    asm volatile("str r0,%0"  : "=m"(FSR));
    __asm volatile("POP	{R0}");

    volatile uint32_t *pRegFram = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
    pRegFram[0] = ulCriticalNesting;    //Save Interrupt flags
    pRegFram[1] = PAB_REGS_Frame[15];       //Save task SPSR
    pRegFram[17] = PAB_REGS_Frame[16];      //Save Exception return LR
    swapping = 1;

    


//Copy registers to task TCB.
    __asm volatile("PUSH	{R0-R12,R14}");
    __asm volatile("LDR		R0,=pxCurrentTCB");
    __asm volatile("LDR		R0,[R0]");
    __asm volatile("LDR		R0,[R0, #4]");
    __asm volatile("ADD		R0, R0, #8");

    __asm volatile("LDR		R1,=PAB_REGS_Frame");       // Save R0-R14:
    __asm volatile("LDMIA	R1!,{R2-R12, R14}");        //for(int i=2;i<17;i++){
    __asm volatile("STMIA	R0!,{R2-R12, R14}");        //    pRegFram[i] = PAB_REGS_Frame[i - 2];
    __asm volatile("LDMIA	R1,{R2-R4}");               //}
    __asm volatile("STMIA	R0,{R2-R4}");
    __asm volatile("POP		{R0-R12,R14}");
//Copy end.

    ((uint32_t *)pxCurrentTCB)[1] = ((uint32_t *)pxCurrentTCB)[1] + 18*4;   //Reset Registers frame stack pointer
    ((uint32_t *)pxCurrentTCB)[0] = PAB_REGS_Frame[13];                         //Reset task stack pointer

    if(pxCurrentTCB == upSystem){
        LL_Scheduler_(L_PAB, (uint32_t *)&pRegFram[2]);
    }

    insAddress = pRegFram[17];

    FAULT_INFO("TASK [%s] PAB. AT %08x, faultAddress:%08x\n", pcTaskGetName(NULL), insAddress, faultAddress);

    pageFaultInfo_t FaultInfo;
    FaultInfo.FaultTask = xTaskGetCurrentTaskHandle();
    FaultInfo.FaultMemAddr = PAB_REGS_Frame[16] - 4;
    FaultInfo.FSR = FSR_DATA_ACCESS_UNMAP;
    /*
    FaultInfo.FSR = FSR;
    switch (FSR & 0xF)
    {
    case 0x1:
    case 0x3:
        FaultInfo.FSR = FSR_DATA_UNALIGN;    
        break;
    case 0x5:
    case 0x7:
        FaultInfo.FSR = FSR_DATA_ACCESS_UNMAP;
        break;
    case 0xD:
    case 0xF:
        FaultInfo.FSR = FSR_DATA_WR_RDONLY;
        break;
    default:
        FaultInfo.FSR = FSR_UNKNOWN;
        printf("TASK [%s] PAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL), DAB_REGS_Frame[16], faultAddress, FSR);
        break;
    }
*/

    //printf("TASK [%s] PAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL),PAB_REGS_Frame[16] - 4, faultAddress, FSR);
    //printf("TASK [%s] PAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL), DAB_REGS_Frame[16], faultAddress, FSR);
    if(xTaskGetCurrentTaskHandle() == upSystem){
        savedBy = 3;
    }
    

    BaseType_t SwitchContext;
    //printf("\n");
    //printf("TASK [%s] PAB Sent\n", pcTaskGetName(NULL) );
    //vTaskSuspend(FaultInfo.FaultTask);
    xQueueSendFromISR(PageFaultQueue, &FaultInfo, &SwitchContext);
    
    //if(SwitchContext)
    {
        vTaskSwitchContext();
    }
    
    swapping = 2;

        if(xTaskGetCurrentTaskHandle() == beComp)
    {
        volatile uint32_t *pReg = (uint32_t *)((uint32_t *)pxCurrentTCB)[1];
        pReg-=18;
        //("PAB Start Schedule R13:%08x\n", pReg[13 + 2]);
    }
    
    portRESTORE_CONTEXT();

}


void arm_vector_fiq()
{
    printf("ERROR: FIQ Mode Unsupported.\n");
    while(1);
}



