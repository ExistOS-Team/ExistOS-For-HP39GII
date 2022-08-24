/*
 *   timer.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */
#include "pch.h"
#include "emu48.h"
#include "ops.h"
#include "io.h"								// I/O definitions

#define AUTO_OFF    10						// Time in minutes for 'auto off'

// Ticks for 01.01.1970 00:00:00
#define UNIX_0_TIME	((ULONGLONG) 0x0001cf2e8f800000)

// Ticks for 'auto off'
#define OFF_TIME	((ULONGLONG) (AUTO_OFF * 60) << 13)

// memory address for clock and auto off
// S(X) = 0x70052-0x70070, G(X) = 0x80058-0x80076, 49G = 0x80058-0x80076
#define RPLTIME		((cCurrentRomType=='S')?0x52:0x58)

#define T1_FREQ		62						// Timer1 1/frequency in ms
#define T2_FREQ		8192					// Timer2 frequency

static BOOL  bStarted   = FALSE;
static BOOL  bOutRange  = FALSE;			// flag if timer value out of range
static UINT  uT1TimerId = 0;
static UINT  uT2TimerId = 0;

static BOOL  bNINT2T1 = FALSE;				// state of NINT2 affected from timer1
static BOOL  bNINT2T2 = FALSE;				// state of NINT2 affected from timer2

static BOOL bAccurateTimer;					// flag if accurate timer is used
static LARGE_INTEGER lT2Ref;				// counter value at timer2 start
static struct TIMECAPS tc;							// timer information
static UINT uT2MaxTicks;					// max. timer2 ticks handled by one timer event

static DWORD dwT2Ref;						// timer2 value at last timer2 access
static DWORD dwT2Cyc;						// cpu cycle counter at last timer2 access

static void CALLBACK TimeProc(UINT uEventId, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

static DWORD CalcT2(VOID)					// calculate timer2 value
{
	DWORD dwT2 = Chipset.t2;				// get value from chipset
	if (bStarted)							// timer2 running
	{
		LARGE_INTEGER lT2Act;
		DWORD         dwT2Dif;

		// timer should run a little bit faster (10%) than maschine in authentic speed mode
		DWORD dwCycPerTick = (9 * T2CYCLES) / 5;

		QueryPerformanceCounter(&lT2Act);	// actual time
		// calculate realtime timer2 ticks since reference point
		dwT2 -= (DWORD)
				(((lT2Act.QuadPart - lT2Ref.QuadPart) * T2_FREQ)
				/ lFreq.QuadPart);

		dwT2Dif = dwT2Ref - dwT2;			// timer2 ticks since last request

		// checking if the MSB of dwT2Dif can be used as sign flag
		_ASSERT((DWORD) tc.wPeriodMax < ((1<<(sizeof(dwT2Dif)*8-1))/8192)*1000);

		// 2nd timer call in a 32ms time frame or elapsed time is negative (Win2k bug)
		if (!Chipset.Shutdn && ((dwT2Dif > 0x01 && dwT2Dif <= 0x100) || (dwT2Dif & 0x80000000) != 0))
		{
			DWORD dwT2Ticks = ((DWORD) (Chipset.cycles & 0xFFFFFFFF) - dwT2Cyc) / dwCycPerTick;

			// estimated < real elapsed timer2 ticks or negative time
			if (dwT2Ticks < dwT2Dif || (dwT2Dif & 0x80000000) != 0)
			{
				// real time too long or got negative time elapsed
				dwT2 = dwT2Ref - dwT2Ticks;	// estimated timer2 value from CPU cycles
				dwT2Cyc += dwT2Ticks * dwCycPerTick; // estimated CPU cycles for the timer2 ticks
			}
			else
			{
				// reached actual time -> new synchronizing
				dwT2Cyc = (DWORD) (Chipset.cycles & 0xFFFFFFFF) - dwCycPerTick;
			}
		}
		else
		{
			// valid actual time -> new synchronizing
			dwT2Cyc = (DWORD) (Chipset.cycles & 0xFFFFFFFF) - dwCycPerTick;
		}

		// check if timer2 interrupt is active -> no timer2 value below 0xFFFFFFFF
		if (   Chipset.inte
			&& (dwT2 & 0x80000000) != 0
			&& (!Chipset.Shutdn || (Chipset.IORam[TIMER2_CTRL]&WKE))
			&& (Chipset.IORam[TIMER2_CTRL]&INTR)
		   )
		{
			dwT2 = 0xFFFFFFFF;
			dwT2Cyc = (DWORD) (Chipset.cycles & 0xFFFFFFFF) - dwCycPerTick;
		}

		dwT2Ref = dwT2;						// new reference time
	}
	return dwT2;
}

static VOID CheckT1(BYTE nT1)
{
	// implementation of TSRQ
	bNINT2T1 = (Chipset.IORam[TIMER1_CTRL]&INTR) != 0 && (nT1&8) != 0;
	IOBit(SRQ1,TSRQ,bNINT2T1 || bNINT2T2);

	if ((nT1&8) == 0)						// timer1 MSB not set
	{
		Chipset.IORam[TIMER1_CTRL] &= ~SRQ;	// clear SRQ bit
		return;
	}

	_ASSERT((nT1&8) != 0);					// timer1 MSB set

	// timer MSB and INT or WAKE bit is set
	if ((Chipset.IORam[TIMER1_CTRL]&(WKE|INTR)) != 0)
		Chipset.IORam[TIMER1_CTRL] |= SRQ;	// set SRQ
	// cpu not sleeping and T1 -> Interrupt
	if (   (!Chipset.Shutdn || (Chipset.IORam[TIMER1_CTRL]&WKE))
		&& (Chipset.IORam[TIMER1_CTRL]&INTR))
	{
		Chipset.SoftInt = TRUE;
		bInterrupt = TRUE;
	}
	// cpu sleeping and T1 -> Wake Up
	if (Chipset.Shutdn && (Chipset.IORam[TIMER1_CTRL]&WKE))
	{
		Chipset.IORam[TIMER1_CTRL] &= ~WKE;	// clear WKE bit
		Chipset.bShutdnWake = TRUE;			// wake up from SHUTDN mode
		SetEvent(hEventShutdn);				// wake up emulation thread
	}
	return;
}

static VOID CheckT2(DWORD dwT2)
{
	// implementation of TSRQ
	bNINT2T2 = (Chipset.IORam[TIMER2_CTRL]&INTR) != 0 && (dwT2&0x80000000) != 0;
	IOBit(SRQ1,TSRQ,bNINT2T1 || bNINT2T2);

	if ((dwT2&0x80000000) == 0)				// timer2 MSB not set
	{
		Chipset.IORam[TIMER2_CTRL] &= ~SRQ;	// clear SRQ bit
		return;
	}

	_ASSERT((dwT2&0x80000000) != 0);		// timer2 MSB set

	// timer MSB and INT or WAKE bit is set
	if ((Chipset.IORam[TIMER2_CTRL]&(WKE|INTR)) != 0)
		Chipset.IORam[TIMER2_CTRL] |= SRQ;	// set SRQ
	// cpu not sleeping and T2 -> Interrupt
	if (   (!Chipset.Shutdn || (Chipset.IORam[TIMER2_CTRL]&WKE))
		&& (Chipset.IORam[TIMER2_CTRL]&INTR))
	{
		Chipset.SoftInt = TRUE;
		bInterrupt = TRUE;
	}
	// cpu sleeping and T2 -> Wake Up
	if (Chipset.Shutdn && (Chipset.IORam[TIMER2_CTRL]&WKE))
	{
		Chipset.IORam[TIMER2_CTRL] &= ~WKE;	// clear WKE bit
		Chipset.bShutdnWake = TRUE;			// wake up from SHUTDN mode
		SetEvent(hEventShutdn);				// wake up emulation thread
	}
	return;
}

static VOID RescheduleT2(BOOL bRefPoint)
{
	UINT uDelay;
	_ASSERT(uT2TimerId == 0);				// timer2 must stopped
	if (bRefPoint)							// save reference time
	{
		dwT2Ref = Chipset.t2;				// timer2 value at last timer2 access
		dwT2Cyc = (DWORD) (Chipset.cycles & 0xFFFFFFFF); // cpu cycle counter at last timer2 access
		QueryPerformanceCounter(&lT2Ref);	// time of corresponding Chipset.t2 value
		uDelay = Chipset.t2;				// timer value for delay
	}
	else									// called without new refpoint, restart t2 with actual value
	{
		uDelay = CalcT2();					// actual timer value for delay
	}
	if ((bOutRange = uDelay > uT2MaxTicks))	// delay greater maximum delay
		uDelay = uT2MaxTicks;				// wait maximum delay time
	uDelay = (uDelay * 125 + 1023) / 1024;	// timer delay in ms (1000/8192 = 125/1024)
	uDelay = __max(tc.wPeriodMin,uDelay);	// wait minimum delay of timer
	_ASSERT(uDelay <= tc.wPeriodMax);		// inside maximum event delay
	// start timer2; schedule event, when Chipset.t2 will be zero
	VERIFY(uT2TimerId = timeSetEvent(uDelay,0,&TimeProc,2,TIME_ONESHOT));
	return;
}

static VOID AbortT2(VOID)
{
	_ASSERT(uT2TimerId);
	timeKillEvent(uT2TimerId);				// kill event
	uT2TimerId = 0;							// then reset var
	return;
}

static void CALLBACK TimeProc(UINT uEventId, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (uEventId == 0) return;				// illegal EventId

	if (uEventId == uT1TimerId)				// called from timer1 event (default period 16 Hz)
	{
		EnterCriticalSection(&csT1Lock);
		{
			Chipset.t1 = (Chipset.t1-1)&0xF;// decrement timer value
			CheckT1(Chipset.t1);			// test timer1 control bits
		}
		LeaveCriticalSection(&csT1Lock);
		return;
	}
	if (uEventId == uT2TimerId)				// called from timer2 event, Chipset.t2 should be zero
	{
		EnterCriticalSection(&csT2Lock);
		{
			uT2TimerId = 0;					// single shot timer timer2 stopped
			if (!bOutRange)					// timer event elapsed
			{
				// timer2 overrun, test timer2 control bits else restart timer2
				Chipset.t2 = CalcT2();		// calculate new timer2 value
				CheckT2(Chipset.t2);		// test timer2 control bits
			}
			RescheduleT2(!bOutRange);		// restart timer2
		}
		LeaveCriticalSection(&csT2Lock);
		return;
	}
	return;
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(dwUser);
	UNREFERENCED_PARAMETER(dw1);
	UNREFERENCED_PARAMETER(dw2);
}

static struct SYSTEMTIME ts;
VOID SetHP48Time(VOID)						// set date and time
{
	
	ULONGLONG  ticks, time;
	DWORD      dw;
	WORD       crc, i;
	BYTE       p[4];

	_ASSERT(sizeof(ULONGLONG) == 8);		// check size of datatype

    
	GetLocalTime(&ts);						// local time, _ftime() cause memory/resource leaks

	// calculate days until 01.01.1970
	dw = (DWORD) ts.wMonth;
	if (dw > 2)
		dw -= 3L;
	else
	{
		dw += 9L;
		--ts.wYear;
	}
	dw = (DWORD) ts.wDay + (153L * dw + 2L) / 5L;
	dw += (146097L * (((DWORD) ts.wYear) / 100L)) / 4L;
	dw +=   (1461L * (((DWORD) ts.wYear) % 100L)) / 4L;
	dw -= 719469L;

	// convert into seconds and add time
	dw = dw * 24L + (DWORD) ts.wHour;
	dw = dw * 60L + (DWORD) ts.wMinute;
	dw = dw * 60L + (DWORD) ts.wSecond;

	// create timerticks = (s + ms) * 8192
	ticks = ((ULONGLONG) dw << 13) | (((ULONGLONG) ts.wMilliseconds << 10) / 125);

	ticks += UNIX_0_TIME;					// add offset ticks from year 0
	ticks += Chipset.t2;					// add actual timer2 value

	time = ticks;							// save for calc. timeout
	time += OFF_TIME;						// add 10 min for auto off

	dw = RPLTIME;							// HP addresses for clock in port0

	crc = 0x0;								// reset crc value
	for (i = 0; i < 13; ++i, ++dw)			// write date and time
	{
		*p = (BYTE) ticks & 0xf;
		crc = (crc >> 4) ^ (((crc ^ ((WORD) *p)) & 0xf) * 0x1081);
		Chipset.Port0[dw] = *p;				// always store in port0
		ticks >>= 4;

	}

	Nunpack(p,crc,4);						// write crc
	memcpy(Chipset.Port0+dw,p,4);			// always store in port0

	dw += 4;								// HP addresses for timeout

	for (i = 0; i < 13; ++i, ++dw)			// write time for auto off
	{
		// always store in port0
		Chipset.Port0[dw] = (BYTE) time & 0xf;
		time >>= 4;
	}

	Chipset.Port0[dw] = 0xf;				// always store in port0
	return;
}

VOID StartTimers(VOID)
{
	if (bStarted)							// timer running
		return;								// -> quit
	if (Chipset.IORam[TIMER2_CTRL]&RUN)		// start timer1 and timer2 ?
	{
		bStarted = TRUE;					// flag timer running
		// initialisation of NINT2 lines
		bNINT2T1 = (Chipset.IORam[TIMER1_CTRL]&INTR) != 0 && (Chipset.t1 & 8) != 0;
		bNINT2T2 = (Chipset.IORam[TIMER2_CTRL]&INTR) != 0 && (Chipset.t2 & 0x80000000) != 0;
		timeGetDevCaps(&tc,sizeof(tc));		// get timer resolution

		// max. timer2 ticks that can be handled by one timer event
		uT2MaxTicks = __min((0xFFFFFFFF / 1024),tc.wPeriodMax);
		uT2MaxTicks = __min((0xFFFFFFFF - 1023) / 125,uT2MaxTicks * 1024 / 125);

		CheckT1(Chipset.t1);				// check for timer1 interrupts
		CheckT2(Chipset.t2);				// check for timer2 interrupts
		// set timer resolution to greatest possible one
		bAccurateTimer = (timeBeginPeriod(tc.wPeriodMin) == TIMERR_NOERROR);
		// set timer1 with given period
		VERIFY(uT1TimerId = timeSetEvent(T1_FREQ,0,&TimeProc,1,TIME_PERIODIC));
		RescheduleT2(TRUE);					// start timer2
	}
	return;
}

VOID StopTimers(VOID)
{
	if (!bStarted)							// timer stopped
		return;								// -> quit
	if (uT1TimerId != 0)					// timer1 running
	{
		// Critical Section handler may cause a dead lock
		timeKillEvent(uT1TimerId);			// stop timer1
		uT1TimerId = 0;						// set flag timer1 stopped
	}
	if (uT2TimerId != 0)					// timer2 running
	{
		EnterCriticalSection(&csT2Lock);
		{
			Chipset.t2 = CalcT2();			// update chipset timer2 value
		}
		LeaveCriticalSection(&csT2Lock);
		AbortT2();							// stop timer2 outside critical section
	}
	bStarted = FALSE;
	if (bAccurateTimer)						// "Accurate timer" running
	{
		timeEndPeriod(tc.wPeriodMin);		// finish service
	}
	return;
}

DWORD ReadT2(VOID)
{
	DWORD dwT2;
	EnterCriticalSection(&csT2Lock);
	{
		dwT2 = CalcT2();					// calculate timer2 value or if stopped last timer value
		CheckT2(dwT2);						// update timer2 control bits
	}
	LeaveCriticalSection(&csT2Lock);
	return dwT2;
}

VOID SetT2(DWORD dwValue)
{
	// calling AbortT2() inside Critical Section handler may cause a dead lock
	if (uT2TimerId != 0)					// timer2 running
		AbortT2();							// stop timer2
	EnterCriticalSection(&csT2Lock);
	{
		Chipset.t2 = dwValue;				// set new value
		CheckT2(Chipset.t2);				// test timer2 control bits
		if (bStarted)						// timer running
			RescheduleT2(TRUE);				// restart timer2
	}
	LeaveCriticalSection(&csT2Lock);
	return;
}

BYTE ReadT1(VOID)
{
	BYTE nT1;
	EnterCriticalSection(&csT1Lock);
	{
		nT1 = Chipset.t1;					// read timer1 value
		CheckT1(nT1);						// update timer1 control bits
	}
	LeaveCriticalSection(&csT1Lock);
	return nT1;
}

VOID SetT1(BYTE byValue)
{
	BOOL bEqual;

	_ASSERT(byValue < 0x10);				// timer1 is only a 4bit counter

	EnterCriticalSection(&csT1Lock);
	{
		bEqual = (Chipset.t1 == byValue);	// check for same value
	}
	LeaveCriticalSection(&csT1Lock);
	if (bEqual) return;						// same value doesn't restart timer period

	if (uT1TimerId != 0)					// timer1 running
	{
		timeKillEvent(uT1TimerId);			// stop timer1
		uT1TimerId = 0;						// set flag timer1 stopped
	}
	EnterCriticalSection(&csT1Lock);
	{
		Chipset.t1 = byValue;				// set new timer1 value
		CheckT1(Chipset.t1);				// test timer1 control bits
	}
	LeaveCriticalSection(&csT1Lock);
	if (bStarted)							// timer running
	{
		// restart timer1 to get full period of frequency
		VERIFY(uT1TimerId = timeSetEvent(T1_FREQ,0,&TimeProc,1,TIME_PERIODIC));
	}
	return;
}
