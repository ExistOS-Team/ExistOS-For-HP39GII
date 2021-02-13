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


/*
 * Implementation of pvPortMalloc() and vPortFree() that relies on the
 * compilers own malloc() and free() implementations.
 *
 * This file can only be used if the linker is configured to to generate
 * a heap memory area.
 *
 * See heap_1.c, heap_2.c and heap_4.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */
#include "startup_info.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <errno.h>  // ENOMEM

#include "memory_map.h"
#include "uart_debug.h"

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#if( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
	#error This file must not be used if configSUPPORT_DYNAMIC_ALLOCATION is 0
#endif

/*-----------------------------------------------------------*/
#ifndef NDEBUG
    static int totalBytesProvidedBySBRK = 0;
#endif

//extern char __HeapBase, __HeapLimit, HEAP_SIZE;  // make sure to define these symbols in linker command file
int heapBytesRemaining = (RAM_START_VIRT_ADDR + TOTAL_PHY_MEMORY - KHEAP_START_VIRT_ADDR); // that's (&__HeapLimit)-(&__HeapBase)

static char *currentHeapEnd = (unsigned char *)KHEAP_START_VIRT_ADDR;



void * _sbrk_r(struct _reent *pReent, int incr) {
    
	
	
    vTaskSuspendAll(); // Note: safe to use before FreeRTOS scheduler started, but not within an ISR
	
	//incr = (incr / TINY_PAGE_SIZE + 1) * 1024;
	
    if ((unsigned int)currentHeapEnd + incr > (KHEAP_START_VIRT_ADDR +(TOTAL_PHY_MEMORY - KHEAP_MAP_PHY_START) )) {
        // Ooops, no more memory available...
       /* #if( configUSE_MALLOC_FAILED_HOOK == 1 )
        {
            extern void vApplicationMallocFailedHook( void );
            vApplicationMallocFailedHook();
        }
        #else*/
            // Default, if you prefer to believe your application will gracefully trap out-of-memory...
			//printf("kmalloc fail...\n");
			uartdbg_printf("sbrk err. %x %x\n", incr,(KHEAP_START_VIRT_ADDR + (TOTAL_PHY_MEMORY - KHEAP_MAP_PHY_START)));
            pReent->_errno = ENOMEM; // newlib's thread-specific errno
            xTaskResumeAll();  // Note: safe to use before FreeRTOS scheduler started, but not within an ISR;
       // #endif
        return (char *)-1; // the malloc-family routine that called sbrk will return 0
    }
    // 'incr' of memory is available: update accounting and return it.
    char *previousHeapEnd = currentHeapEnd;
    currentHeapEnd += incr;
    heapBytesRemaining -= incr;

	//uartdbg_printf("currentHeapEnd %x\n",currentHeapEnd);
    #ifndef NDEBUG
        totalBytesProvidedBySBRK += incr;
    #endif
    xTaskResumeAll();  // Note: safe to use before FreeRTOS scheduler started, but not within an ISR
	
	
    return (char *) previousHeapEnd;
}

unsigned int getCurrentHeapEnd(){
	return (unsigned int)currentHeapEnd;
}


//void __env_lock()    {       vTaskSuspendAll();};
//void __env_unlock()  { (void)xTaskResumeAll();  };


size_t TotalMallocdBytes;
int MallocCallCnt;
static bool inside_malloc;



void __wrap_free(void *aptr){
	extern void __real_free(void *_aptr);
	
	__real_free(aptr);

}



void *__wrap_malloc(size_t nbytes) {
	extern void * __real_malloc(size_t nbytes);
	MallocCallCnt++;
	TotalMallocdBytes += nbytes;
	inside_malloc = true;
	

	void *p, *k;
	p = __real_malloc(nbytes); // will call malloc_r...
	
	inside_malloc = false;
	return p;
};


void *__wrap__malloc_r(void *reent, size_t nbytes) {
	extern void * __real__malloc_r(size_t nbytes);
	
	
	if(!inside_malloc) {
	MallocCallCnt++;
	TotalMallocdBytes += nbytes;
	}; 
	void *p = __real__malloc_r(nbytes);
	return p;
};



void *pvPortMalloc( size_t xWantedSize )
{
void *pvReturn;

	vTaskSuspendAll();
	{
		pvReturn = malloc( xWantedSize );
		//traceMALLOC( pvReturn, xWantedSize );
	}
	//uartdbg_print_regs();
	//uartdbg_printf("pvPortMalloc malloc end\n");
	xTaskResumeAll();


	if( pvReturn == NULL )
		{
			uartdbg_printf("\nk malloc fail.\n");
			while(1);
			
		}
	//uartdbg_printf("pvPortMalloc end\n");

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
	}
	#endif

	return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
	if( pv )
	{
		vTaskSuspendAll();

		{
			free( pv );
			//traceFREE( pv, 0 );
		}
		xTaskResumeAll();
	}
}



extern unsigned char *heap;

size_t xPortGetFreeHeapSize( void )
{
	return heapBytesRemaining;
}


