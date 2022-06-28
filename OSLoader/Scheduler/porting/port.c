/*
 * FreeRTOS Kernel V10.3.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ST STR75x ARM7
 * port.
 *----------------------------------------------------------*/

/* Library includes. */
#include "portmacro.h"
 
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "timer_up.h"

/* Constants required to setup the initial stack. */
#define portINITIAL_SPSR				( ( StackType_t ) (0x1F) ) /* SYS mode, ARM mode, interrupts enabled. */
//#define portINITIAL_SPSR				( ( StackType_t ) (0xDF) ) /* SYS mode, ARM mode, interrupts disable. */
#define portTHUMB_MODE_BIT				( ( StackType_t ) 0x20 )
#define portINSTRUCTION_SIZE			( ( StackType_t ) 4 )

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING 		( ( uint32_t ) 0 )
  
/*-----------------------------------------------------------*/

/* Setup the TB to generate the tick interrupts. */
void prvSetupTimerInterrupt( void );

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */


#if ( portCONTEXT_REGS_IN_TCB == 1 )

StackType_t *pxPortInitialiseStack(StackType_t * pxREGSFrameStack, StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	
	
	*pxREGSFrameStack = ( StackType_t ) portNO_CRITICAL_NESTING;
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) portINITIAL_SPSR;
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) pvParameters; /* R0 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x01010101; /* R1 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x02020202; /* R2 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x03030303; /* R3 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x04040404; /* R4 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x05050505; /* R5 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x06060606; /* R6 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x07070707; /* R7 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x08080808; /* R8 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x09090909; /* R9 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x10101010; /* R10 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x11111111; /* R11 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x12121212; /* R12 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) pxTopOfStack; /* R13 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) 0x14141414; /* R14 */
	pxREGSFrameStack++;
	*pxREGSFrameStack =	( StackType_t ) pxCode + portINSTRUCTION_SIZE; /* LR */
	pxREGSFrameStack++;

	return pxREGSFrameStack;
	
}

#else

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
StackType_t *pxOriginalTOS;

	pxOriginalTOS = pxTopOfStack;

	/* To ensure asserts in tasks.c don't fail, although in this case the assert
	is not really required. */
	pxTopOfStack--;

	/* Setup the initial stack of the task.  The stack is set exactly as
	expected by the portRESTORE_CONTEXT() macro. */

	/* First on the stack is the return address - which in this case is the
	start of the task.  The offset is added to make the return address appear
	as it would within an IRQ ISR. */
	*pxTopOfStack = ( StackType_t ) pxCode + portINSTRUCTION_SIZE;		
	pxTopOfStack--;

	*pxTopOfStack = ( StackType_t ) 0x14141414;	/* R14 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x12121212;	/* R12 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x11111111;	/* R11 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x10101010;	/* R10 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x09090909;	/* R9 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x08080808;	/* R8 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x07070707;	/* R7 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x06060606;	/* R6 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x05050505;	/* R5 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x04040404;	/* R4 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x03030303;	/* R3 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x02020202;	/* R2 */
	pxTopOfStack--;	
	*pxTopOfStack = ( StackType_t ) 0x01010101;	/* R1 */
	pxTopOfStack--;	

	/* When the task starts is will expect to find the function parameter in
	R0. */
	*pxTopOfStack = ( StackType_t ) pvParameters; /* R0 */
	pxTopOfStack--;

	/* The status register is set for system mode, with interrupts enabled. */
	*pxTopOfStack = ( StackType_t ) portINITIAL_SPSR;

	#ifdef THUMB_INTERWORK
	{
		/* We want the task to start in thumb mode. */
		*pxTopOfStack |= portTHUMB_MODE_BIT;
	}
	#endif

	pxTopOfStack--;

	/* Interrupt flags cannot always be stored on the stack and will
	instead be stored in a variable, which is then saved as part of the
	tasks context. */
	*pxTopOfStack = portNO_CRITICAL_NESTING;

	return pxTopOfStack;	
}

#endif




BaseType_t xPortStartScheduler( void )
{
extern void vPortISRStartFirstTask( void );

	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	up_TimerSetup();
	
	/* Start the first task. */
#if ( portCONTEXT_REGS_IN_TCB == 1 )
	__asm volatile ("LDR	R0, =pxCurrentTCB");
	__asm volatile ("LDR	R0, [R0]");
	__asm volatile ("ADD	R0, R0, #4");
	__asm volatile ("MOV	R3, R0");
	__asm volatile ("LDR	R0, [R0]");
	__asm volatile ("MOV	LR, R0");

	__asm volatile ("SUB	R0,R0, #72");
	__asm volatile ("SUB	LR,LR, #68");

	__asm volatile ("LDR	R2, =ulCriticalNesting");
	__asm volatile ("LDR	R1, [R0]");
	__asm volatile ("STR	R1, [R2]");

	__asm volatile ("LDMIA	LR!, {R0}");
	__asm volatile ("MSR	SPSR, R0");

	__asm volatile ("SUB	R4, LR, #8");
	__asm volatile ("STR	R4, [R3]");

	__asm volatile ("LDMIA	LR, {R0-R14}^");
	__asm volatile ("LDR	LR, [LR, #+60]");
	__asm volatile ("SUBS 	PC, LR, #4");


#else

	vPortISRStartFirstTask();	
#endif


	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the ARM port will require this function as there
	is nothing to return to.  */
}
/*-----------------------------------------------------------*/

