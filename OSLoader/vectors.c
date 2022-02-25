
#include "FreeRTOS.h"
#include "task.h"

#include "SystemConfig.h"
#include <stdio.h>

#include "interrupt_up.h"
#include "portmacro.h"
#include "debug.h"

extern volatile void * volatile pxCurrentTCB;	
extern volatile uint32_t ulCriticalNesting;	

unsigned int faultAddress;
unsigned int insAddress;
unsigned int FSR;

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

void arm_vector_swi() __attribute__((naked));
void arm_vector_swi()
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
    __asm volatile("LDR		R0,[R0, #4]");
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


    if(SWI_REGS_Frame[0] == 0xFE002200){
        vTaskSuspend(NULL);
    }


	vTaskSwitchContext();

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
    __asm volatile("LDR		R0,[R0, #4]");
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
    
   

    if(up_isr())
    {
        vTaskSwitchContext();
    }


    portRESTORE_CONTEXT();

}


void arm_vector_und()  __attribute__((naked));
void arm_vector_und()
{
    register uint32_t r0 asm("r0");
    __asm volatile("MOV	 R0, LR");

    printf("ERROR: Undefind Instruction AT:%08x\n", r0);

    __dumpRegs();printRegs();

    while(1);
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

    __asm volatile("STR		LR, [R0, #4]");
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
    


    asm volatile("str r0,%0"  : "=m"(insAddress)); 
    asm volatile("mrc p15, 0, r0, c6, c0, 0");
    asm volatile("str r0,%0"  : "=m"(faultAddress));
    asm volatile("mrc p15, 0, r0, c5, c0, 0"); 
    asm volatile("str r0,%0"  : "=m"(FSR));


    printf("TASK [%s] DAB. AT:%08x access %08x, FSR:%08x\n", pcTaskGetName(NULL),insAddress, faultAddress, FSR);
    

    for(int i=0;i<15;i++)
        {
            printf("REG %d:%08x\n",i,DAB_REGS_Frame[i]);
        }
        printf("\n");

    vTaskSuspend(NULL);

    //__dumpRegs();printRegs();
    //vTaskSwitchContext();
    //while(1);

    portRESTORE_CONTEXT();

}




void arm_vector_pab()  __attribute__((naked));
void arm_vector_pab()
{
    asm volatile("str r0,%0": "=m"(insAddress));

    printf("PAB. AT:%08x\n", insAddress);
    __dumpRegs();printRegs();
    while(1);
}


void arm_vector_fiq()
{
    printf("ERROR: FIQ Mode Unsupported.\n");
    while(1);
}



