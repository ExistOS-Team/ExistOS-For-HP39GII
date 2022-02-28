#ifndef __LLAPI_CODE_H__
#define __LLAPI_CODE_H__

#define LL_SWI_BASE                     (0xEE00)
#define LL_SWI_NUM                      (64)


#define LL_SWI_DELAY_MS                (LL_SWI_BASE + 1)
#define LL_SWI_PUT_CH                  (LL_SWI_BASE + 2)
#define LL_SWI_WRITE_STRING            (LL_SWI_BASE + 3)


#define SWI(i)  __asm volatile ("" : : : "memory");\
                __asm volatile( "swi %0" :: "i"(i))
                
#define SWI1(i, a0) \
                    __asm volatile ("" : : : "r0");\
                    __asm volatile( "swi %0" :: "i"(i))

#endif

