#include "pch.h"
#include "emu48.h"
#include "io.h"
#include "i28f160.h"

#include "FreeRTOS.h"
#include "task.h"

#include "sys_llapi.h"

#include "emu48_kb_39g.h"
#include "keyboard_gii39.h"

CRITICAL_SECTION csGDILock;					// critical section for hWindowDC
CRITICAL_SECTION csLcdLock;					// critical section for display update
CRITICAL_SECTION csKeyLock;					// critical section for key scan
CRITICAL_SECTION csIOLock;					// critical section for I/O access
CRITICAL_SECTION csT1Lock;					// critical section for timer1 access
CRITICAL_SECTION csT2Lock;					// critical section for timer2 access
CRITICAL_SECTION csTxdLock;					// critical section for transmit byte
CRITICAL_SECTION csRecvLock;				// critical section for receive byte
CRITICAL_SECTION csSlowLock;				// critical section for speed slow down


HANDLE           hEventShutdn;				// event handle to stop cpu thread
HANDLE           hThread;



BYTE   cCurrentRomType = 0;					// Model -> hardware
UINT   nCurrentClass = 0;					// Class -> derivate
BOOL   bRomWriteable = TRUE;				// flag if ROM writeable

LPBYTE pbyRom = NULL;
DWORD  dwRomSize = 0;


LPBYTE pbyPort2 = NULL;
BOOL   bPort2Writeable = FALSE;
BOOL   bPort2IsShared = FALSE;
DWORD  dwPort2Size = 0;						// size of mapped port2
DWORD  dwPort2Mask = 0;


LARGE_INTEGER    lFreq;						// high performance counter frequency
LARGE_INTEGER    lAppStart;					// high performance counter value at Appl. start


static uint8_t full_screen_buf[257 * 128];


bool rom_is_packed();
int load_rom(const char *romfile);
void *get_rom_addr();
size_t get_rom_size();

void  __attribute__((target("arm"))) emu_thread(void *_)
{
    int ret;
    ret = WorkerThread(NULL);


    printf("WorkerThread Exit:%d\n",ret);
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


typedef struct emu48_emu_timer_t
{
    struct emu48_emu_timer_t *next;
    bool oneshot;
    uint32_t period_ms;
    uint32_t count;
    LPTIMECALLBACK cb;
    uint32_t userData;
}emu48_emu_timer_t;

static emu48_emu_timer_t *emu_timers = NULL;


uint32_t emu_add_timer(uint32_t period_ms, LPTIMECALLBACK cb, uint32_t userData, bool oneshot)
{
    emu48_emu_timer_t *t;
    emu48_emu_timer_t *t_prev;

    vPortEnterCritical();
    if(emu_timers)
    {
        t = emu_timers;
        do{
            t_prev = t;
            t = t->next;
        }while(t != NULL);
        t = pvPortMalloc(sizeof(emu48_emu_timer_t));
    }else{
        t = emu_timers = pvPortMalloc(sizeof(emu48_emu_timer_t));
        t_prev = NULL;
    }

    assert(t != NULL);

    memset(t, 0, sizeof(t));

    t->period_ms = period_ms;
    t->cb = cb;
    t->userData = userData;
    t->oneshot = oneshot;
    t->count = 0;
    t->next = NULL;

    if(t_prev){
        t_prev->next = t;
    }
    vPortExitCritical();

    return (uint32_t)t;
}


uint32_t emu_del_timer(emu48_emu_timer_t *t_del)
{
    emu48_emu_timer_t *t;
    emu48_emu_timer_t *t_prev;

    if(emu_timers == NULL)
    {
        return 0;
    }

    vPortEnterCritical();

    if(t_del == emu_timers)
    {
        if(t_del->next)
        {
            t = t_del->next;
            emu_timers = t;
            vPortFree(t_del);
            goto t_del_exit;
        }else{
            vPortFree(t_del);
            emu_timers = NULL;
            goto t_del_exit;
        }
    }else{

        t_prev = emu_timers;
        while ((t_prev->next != NULL) && (t_prev->next != t_del))
        {
            t_prev = t_prev->next;
            goto t_del_found;
        }
        goto t_del_exit;

        t_del_found:
        if(t_del->next)
        {
            t_prev->next = t_del->next;
            vPortFree(t_del);
        }else{
            t_prev->next = NULL;
            vPortFree(t_del);
        }


    }

    t_del_exit:

    vPortExitCritical();
    return 0;
}


void  __attribute__((target("arm"))) emu_timer_thread(void *_)
{
    emu48_emu_timer_t *t;
    for(;;)
    {
        if(emu_timers)
        {
            t = emu_timers;
            do{
                if((t->period_ms != 0) && (t->count < t->period_ms))
                {
                    t->count += 10;
                    if(t->count >= t->period_ms){
                        if(t->oneshot)
                        {
                            t->cb((UINT)t, 0, t->userData, 0, 0);
                            emu_del_timer(t);
                            break;
                            
                        }else{
                            t->count = 0;
                            t->cb((UINT)t, 0, t->userData, 0, 0);
                        }
                    }
                }
                t = t->next;
            }while(t != NULL);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


static bool need_Refresh = false;

void emu48_update_display()
{
    need_Refresh = true;
}

void  __attribute__((target("arm")))  emu48_disp_update_task(void *_)
{
    memset(full_screen_buf, 255, sizeof(full_screen_buf));
    for(;;)
    {
        if(need_Refresh)
        {
                float sx,sy;
                for(uint16_t y = 0; y < SCREENHEIGHT; y++)
                {
                    for(uint16_t x = 0; x < 144; x++)
                    {
                        sx = x * 1.96;
                        sy = y * 2;
                        if(sx < 255 && sy < 127){
                            full_screen_buf[(int) (256 * (sy   ) + (sx   )) ] = pbyLcd[144 * y + x];
                            full_screen_buf[(int) (256 * (sy +1) + (sx   )) ] = pbyLcd[144 * y + x];
                            full_screen_buf[(int) (256 * (sy   ) + (sx +1)) ] = pbyLcd[144 * y + x];
                            full_screen_buf[(int) (256 * (sy +1) + (sx +1)) ] = pbyLcd[144 * y + x];
                        }
                    }
                }
                //ll_disp_put_area(pbyLcd, 0,0, 144 - 1, SCREENHEIGHT - 1);
                ll_disp_put_area(full_screen_buf, 0,0, 255, 126);
                need_Refresh = false;
        }
        vTaskDelay(pdMS_TO_TICKS(76));
    }
}

extern void *emu_rom_addr;

static BYTE pbyNULL[16];
//static BYTE pbyCODE[128];

static BYTE pbyCODE_packed[64];
static BYTE pbyCODE_unpacked[sizeof(pbyCODE_packed) * 2];
LPBYTE FASTPTR(DWORD d)
{
	
	LPBYTE lpbyPage;
	
	d &= 0xFFFFF;							// handle address overflows

	if ((Chipset.IOCfig)&&((d&0xFFFC0)==Chipset.IOBase))
		return Chipset.IORam+d-Chipset.IOBase;

	if ((lpbyPage = RMap[d>>12]) != NULL)	// page valid
	{
		lpbyPage += d & 0xFFF;				// full address

        if((uint32_t)lpbyPage >= 0x70000000)
        {
            //if((uint32_t)lpbyPage < 0x70000000 + dwRomSize * 2)
            {

                //memcpy(pbyCODE, (void *)((uint32_t)lpbyPage - 0x70000000 + (uint32_t)emu_rom_addr) ,128);
                //return pbyCODE;

                uint32_t acc_addr = ((uint32_t)lpbyPage - 0x70000000) / 2 + (uint32_t)emu_rom_addr;

                memcpy(pbyCODE_packed, (void *)acc_addr ,sizeof(pbyCODE_packed));

                for(int i = 0, j = 0; i < sizeof(pbyCODE_packed); i++)
                {
                    pbyCODE_unpacked[j++] = pbyCODE_packed[i] & 0xF;
                    pbyCODE_unpacked[j++] = pbyCODE_packed[i] >> 4;
                }
                return &pbyCODE_unpacked[(uint32_t)lpbyPage & 1];
                
            }
        }else{
            return lpbyPage;
        }
        //printf("RD CODE ERR:%08x\n", lpbyPage);
	}
	else
	{
		lpbyPage = pbyNULL;					// memory allocation
		Npeek(lpbyPage, d, 13);				// fill with data (LA(8) = longest opcode)
	}
	return lpbyPage;
}

void emu48_main(int select)
{
    int ret;

    ret = load_rom("/rom.39g");
/*
    if(select == 1)
    {
        ret = load_rom("/rom.39g");
    }else if(select == 2)
    {
        ret = load_rom("/rom.39g.unpack");
    }
    */

    if(ret)
    {
        goto b_exit;
    }

    //pbyRom = get_rom_addr();

    pbyRom = (LPBYTE)0x70000000;

    dwRomSize = get_rom_size() * 2;
/*
    if(rom_is_packed())
    {
        dwRomSize *= 2;
    }
*/
    cCurrentRomType = 'E';
    nCurrentClass = 39;
    


    pbyLcd = pvPortMalloc(144 * SCREENHEIGHT);

    QueryPerformanceFrequency(&lFreq);
    QueryPerformanceCounter(&lAppStart);

    
	ZeroMemory(&Chipset,sizeof(Chipset));
	ZeroMemory(&RMap,sizeof(RMap));			// delete MMU mappings
	ZeroMemory(&WMap,sizeof(WMap));

    Chipset.type = cCurrentRomType;

	if (Chipset.type == 'E')				// HP39/40G
	{
		Chipset.Port0Size = 128;
		Chipset.Port1Size = 0;
		Chipset.Port2Size = 128;

		Chipset.cards_status = 0xF;

		bPort2Writeable = TRUE;				// port2 is writeable
	}
    Chipset.IORam[LPE] = RST;	

    //pdwInstrArray = HeapAlloc(hHeap,0,256*sizeof(*pdwInstrArray));

    SetSpeed(bRealSpeed);
    SetLcdMode(bGrayscale);

	// allocate port memory
	if (Chipset.Port0Size)
	{
		Chipset.Port0 = HeapAlloc(hHeap,HEAP_ZERO_MEMORY,Chipset.Port0Size*2048);
		_ASSERT(Chipset.Port0 != NULL);
	}
	if (Chipset.Port1Size)
	{
		Chipset.Port1 = HeapAlloc(hHeap,HEAP_ZERO_MEMORY,Chipset.Port1Size*2048);
		_ASSERT(Chipset.Port1 != NULL);
	}
	if (Chipset.Port2Size)
	{
		Chipset.Port2 = HeapAlloc(hHeap,HEAP_ZERO_MEMORY,Chipset.Port2Size*2048);
		_ASSERT(Chipset.Port2 != NULL);
	}
    //FlashInit();

	RomSwitch(0);							// boot ROM view of HP49G and map memory

/*
    if (pbyRom)
	{
		//SetWindowLocation(hWnd,Chipset.nPosX,Chipset.nPosY);
		Map(0x00,0xFF);
	}
*/

	CpuReset();
	Chipset.Shutdn = FALSE;				// automatic restart


	nState     = SM_RUN;					// init state must be <> nNextState
	nNextState = SM_INVALID;				// go into invalid state

    xTaskCreate(emu_thread, "emu48", 32768, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(emu_timer_thread, "emu48 timer", 1000, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(emu48_disp_update_task, "emu48 timer", 1000, NULL, configMAX_PRIORITIES - 3, NULL);

    ll_cpu_slowdown_enable(false);

    printf("emu_thread Created\n");


    vTaskDelay(pdMS_TO_TICKS(1000));

    SwitchToState(SM_RUN);
    vTaskDelay(pdMS_TO_TICKS(5000));


    int on_press = 0;
    for(;;)
    {
        //printf("emu cpu pc:%08x\n", Chipset.pc);

        uint32_t keys, key, kpress;
        static uint32_t last_key;
        static uint32_t last_press;
        
        do{
            keys = ll_vm_check_key();
            key = keys & 0xFFFF;
            kpress = keys >> 16;

            vTaskDelay(pdMS_TO_TICKS(40));

            if(on_press > 0)
            {
                on_press--;
                if(on_press == 1)
                {
                    KeyboardEvent(false, KEY_39G_ON.out, KEY_39G_ON.in);
                }
            }

        }while((last_key == key) && (last_press == kpress));

        #define KEYEVENT(name)  KeyboardEvent(kpress, name.out, name.in)
        #define KEY_CASE(w, s)  case w: KEYEVENT(s); break



        switch (key)
        {
            KEY_CASE(KEY_F1, KEY_39G_F1);
            KEY_CASE(KEY_F2, KEY_39G_F2);
            KEY_CASE(KEY_F3, KEY_39G_F3);
            KEY_CASE(KEY_F4, KEY_39G_F4);
            KEY_CASE(KEY_F5, KEY_39G_F5);
            KEY_CASE(KEY_F6, KEY_39G_F6);

            KEY_CASE(KEY_SYMB , KEY_39G_SYMB );
            KEY_CASE(KEY_PLOT , KEY_39G_PLOT );
            KEY_CASE(KEY_NUM  , KEY_39G_NUM  );
            KEY_CASE(KEY_HOME , KEY_39G_HOME );
            KEY_CASE(KEY_APPS , KEY_39G_APPS );
            KEY_CASE(KEY_VIEWS, KEY_39G_VIEWS);

            KEY_CASE(KEY_VARS      , KEY_39G_VARS  );
            KEY_CASE(KEY_MATH      , KEY_39G_MATH  );
            KEY_CASE(KEY_ABC       , KEY_39G_ABC   );
            KEY_CASE(KEY_XTPHIN    , KEY_39G_XTPHIN);
            KEY_CASE(KEY_BACKSPACE , KEY_39G_CLEAR );

            KEY_CASE(KEY_SIN    , KEY_39G_SIN   );
            KEY_CASE(KEY_COS    , KEY_39G_COS   );
            KEY_CASE(KEY_TAN    , KEY_39G_TAN   );
            KEY_CASE(KEY_LN     , KEY_39G_LN    );
            KEY_CASE(KEY_LOG    , KEY_39G_LOG   );

            KEY_CASE(KEY_X2              , KEY_39G_X2             );
            KEY_CASE(KEY_XY              , KEY_39G_XY             );
            KEY_CASE(KEY_LEFTBRACKET     , KEY_39G_LEFTBRACKET    );
            KEY_CASE(KEY_RIGHTBRACKET     , KEY_39G_RIGHTBRACKET   );
            KEY_CASE(KEY_DIVISION        , KEY_39G_DIVISION       );

            KEY_CASE(KEY_COMMA         , KEY_39G_COMMA         );
            KEY_CASE(KEY_7             , KEY_39G_7             );
            KEY_CASE(KEY_8             , KEY_39G_8             );
            KEY_CASE(KEY_9             , KEY_39G_9             );
            KEY_CASE(KEY_MULTIPLICATION, KEY_39G_MULTIPLICATION);

            KEY_CASE(KEY_ALPHA         , KEY_39G_ALPHA         );
            KEY_CASE(KEY_4             , KEY_39G_4             );
            KEY_CASE(KEY_5             , KEY_39G_5             );
            KEY_CASE(KEY_6             , KEY_39G_6             );
            KEY_CASE(KEY_SUBTRACTION   , KEY_39G_SUBTRACTION   );

            KEY_CASE(KEY_SHIFT        , KEY_39G_SHIFT        );
            KEY_CASE(KEY_1            , KEY_39G_1            );
            KEY_CASE(KEY_2            , KEY_39G_2            );
            KEY_CASE(KEY_3            , KEY_39G_3            );
            KEY_CASE(KEY_PLUS         , KEY_39G_PLUS         );

            //KEY_CASE(KEY_ON           , KEY_39G_ON           );
            KEY_CASE(KEY_0            , KEY_39G_0            );
            KEY_CASE(KEY_DOT          , KEY_39G_DOT          );
            KEY_CASE(KEY_NEGATIVE     , KEY_39G_NEGATIVE     );
            KEY_CASE(KEY_ENTER        , KEY_39G_ENTER        );


            KEY_CASE(KEY_RIGHT    , KEY_39G_RIGHT   );
            KEY_CASE(KEY_DOWN     , KEY_39G_DOWN    );
            KEY_CASE(KEY_LEFT     , KEY_39G_LEFT    );
            KEY_CASE(KEY_UP       , KEY_39G_UP      );

            case KEY_ON:
                {
                    KeyboardEvent(true, KEY_39G_ON.out, KEY_39G_ON.in);
                    on_press = 10;
                }

        
        default:
            break;
        }



        last_key = key;
        last_press = kpress;


    }

    b_exit:

    printf("Failed to open rom:%d\n", ret);

/*
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
    }*/

}

#include "SystemUI.h"
void __attribute__((target("arm"))) emu48_task(void *_)
{
    SystemUISuspend();
    emu48_main(0);
}

void EnterCriticalSection(CRITICAL_SECTION *obj)
{
    vPortEnterCritical();
}


void LeaveCriticalSection(CRITICAL_SECTION *obj)
{
    vPortExitCritical();
}

void SetEvent(HANDLE h)
{

    
}


BOOL ResetEvent(HANDLE h)
{
    return true;
}

DWORD WaitForSingleObject(HANDLE hHandle, int dwMilliseconds)
{
    return WAIT_OBJECT_0;
}

BOOL QueryPerformanceCounter(PLARGE_INTEGER l)
{
    l->QuadPart = ll_get_time_us();
    //printf("chktime:%ld\n", l->d.LwPart);
}


MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD_PTR dwUser, UINT fuEvent) 
{
    //printf("set timer:%d, %d, %d\n", uDelay, dwUser, fuEvent);
    
    return (MMRESULT)emu_add_timer(uDelay, fptc, (uint32_t)dwUser, fuEvent == TIME_ONESHOT);
}


MMRESULT timeKillEvent(UINT uTimerID)
{
    //printf("del timer:%08x\n", uTimerID);
    emu_del_timer((emu48_emu_timer_t *)uTimerID);
    return 0;
}

VOID DisableDebugger(VOID)
{

}

VOID GetLocalTime(struct SYSTEMTIME *lpSystemTime) {
    if(lpSystemTime) {

        lpSystemTime->wYear = 2022;
        lpSystemTime->wMonth = 8;
        //lpSystemTime->wDayOfWeek = 2;
        lpSystemTime->wDay = 23;
        lpSystemTime->wHour = 10;
        lpSystemTime->wMinute = 10 ;
        lpSystemTime->wSecond = ll_get_time_ms()/1000 ;
        lpSystemTime->wMilliseconds = ll_get_time_us() ;
        /*
        struct timespec ts = {0, 0};
        struct tm tm = {};
        clock_gettime(CLOCK_REALTIME, &ts);
        time_t tim = ts.tv_sec;
        localtime_r(&tim, &tm);
        lpSystemTime->wYear = (WORD) (1900 + tm.tm_year);
        lpSystemTime->wMonth = (WORD) (1 + tm.tm_mon);
        lpSystemTime->wDayOfWeek = (WORD) (tm.tm_wday);
        lpSystemTime->wDay = (WORD) (tm.tm_mday);
        lpSystemTime->wHour = (WORD) (tm.tm_hour);
        lpSystemTime->wMinute = (WORD) (tm.tm_min);
        lpSystemTime->wSecond = (WORD) (tm.tm_sec);
        lpSystemTime->wMilliseconds = (WORD) (ts.tv_nsec / 1e6);
        */
    }
}

MMRESULT timeGetDevCaps(struct TIMECAPS *ptc, UINT cbtc) {
    if(ptc) {
        ptc->wPeriodMin = 1; // ms
        ptc->wPeriodMax = 1000000; // ms -> 1000s
    }
    return 0; //No error
}

MMRESULT timeBeginPeriod(UINT uPeriod) {
    //TODO
    return 0; //No error
}
MMRESULT timeEndPeriod(UINT uPeriod) {
    //TODO
    return 0; //No error
}

VOID UpdateWindowStatus(VOID)
{

}


DWORD timeGetTime(void)
{
    time_t t = ll_get_time_ms();//;time(NULL);
    return (DWORD)t;
    //return (DWORD)(t * 1000);
}

BOOL QueryPerformanceFrequency(PLARGE_INTEGER l) {
    //https://msdn.microsoft.com/en-us/library/windows/desktop/ms644904(v=vs.85).aspx
    l->QuadPart = 1000000;
    return TRUE;
}

BOOL CheckBreakpoint(DWORD dwAddr, DWORD dwRange, UINT nType)
{
	return FALSE;
}

VOID NotifyDebugger(INT nType)				// update registers
{
    /*
	nRplBreak = nType;						// save breakpoint type
	_ASSERT(hDlgDebug);						// debug dialog box open
	PostMessage(hDlgDebug,WM_UPDATE,0,0);*/
	return;
}

VOID UpdatePatches(BOOL bPatch)
{

}

VOID UpdateDbgCycleCounter(VOID)
{
	// update 64 bit cpu cycle counter
	//if (Chipset.cycles < dwDbgRefCycles) ++Chipset.cycles_reserved;

	//dwDbgRefCycles = (DWORD) (Chipset.cycles & 0xFFFFFFFF);
    
	return;
}

void OutputDebugString(char *s)
{
    printf("%s\n",s);
}
