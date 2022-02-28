#ifndef __LLAPI_CODE_H__
#define __LLAPI_CODE_H__

#define LL_SWI_BASE                     (0xEE00)
#define LL_SWI_NUM                      (64)


#define LL_SWI_DELAY_MS                (LL_SWI_BASE + 1)
#define LL_SWI_PUT_CH                  (LL_SWI_BASE + 2)
#define LL_SWI_WRITE_STRING1           (LL_SWI_BASE + 3)
#define LL_SWI_WRITE_STRING2           (LL_SWI_BASE + 4)


#define LL_SWI_GET_TIME_US             (LL_SWI_BASE + 5)



#endif

