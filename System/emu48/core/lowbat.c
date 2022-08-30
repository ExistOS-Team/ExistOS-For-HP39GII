/*
 *   lowbat.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 2006 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "emu48.h"
#include "io.h"								// I/O definitions

#define BAT_SIMULATION					// switch low bat simulation

#define BAT_FREQ	(60*1000)				// bat update time in ms (real machine = 60us, HP28C = 60s)

BOOL bLowBatDisable = FALSE;

static HANDLE hCThreadBat = NULL;
static HANDLE hEventBat;

static DWORD WINAPI LowBatThread(LPVOID pParam)
{
	BOOL bLBI,bVLBI;

	do
	{
		GetBatteryState(&bLBI,&bVLBI);		// get battery state

		// very low bat detection
		bVLBI = bVLBI && (Chipset.IORam[LPE] & EVLBI) != 0;

		IOBit(LPD,VLBI,bVLBI);				// set VLBI
		IOBit(SRQ1,VSRQ,bVLBI);				// and service bit

		if (bVLBI)							// VLBI detected
		{
			Chipset.SoftInt = TRUE;
			bInterrupt = TRUE;

			if (Chipset.Shutdn)				// CPU shut down
			{
				Chipset.bShutdnWake = TRUE;	// wake up from SHUTDN mode
				SetEvent(hEventShutdn);		// wake up emulation thread
			}
		}
	}
	while (WaitForSingleObject(hEventBat,BAT_FREQ) == WAIT_TIMEOUT);

	return 0;
	UNREFERENCED_PARAMETER(pParam);
}

VOID StartBatMeasure(VOID)
{
	DWORD dwThreadId;

	if (hCThreadBat)						// Bat measuring thread running
		return;								// -> quit

	// event to cancel Bat refresh loop
	//hEventBat = CreateEvent(NULL,FALSE,FALSE,NULL);

//	VERIFY(hCThreadBat = CreateThread(NULL,0,&LowBatThread,NULL,0,&dwThreadId));
	return;
}

VOID StopBatMeasure(VOID)
{
	if (hCThreadBat == NULL)				// thread stopped
		return;								// -> quit

	SetEvent(hEventBat);					// leave Bat update thread
	WaitForSingleObject(hCThreadBat,INFINITE);
//	CloseHandle(hCThreadBat);
	hCThreadBat = NULL;						// set flag Bat update stopped
//	CloseHandle(hEventBat);					// close Bat event
	return;
}

VOID GetBatteryState(BOOL *pbLBI, BOOL *pbVLBI)
{
#if defined BAT_SIMULATION
	switch (2/*GetPrivateProfileInt(_T("LowBat"),_T("Level"),2,_T(".\\Lowbat.ini"))*/)
	{
	case 0: // empty
		*pbLBI = TRUE;
		*pbVLBI = TRUE;
		break;
	case 1: // low
		*pbLBI = TRUE;
		*pbVLBI = FALSE;
		break;
	default: // full
		*pbLBI = FALSE;
		*pbVLBI = FALSE;
		break;
	}
#else
	SYSTEM_POWER_STATUS sSps;

	*pbLBI = FALSE;							// no battery warning
	*pbVLBI = FALSE;

//	VERIFY(GetSystemPowerStatus(&sSps));

	// low bat emulation enabled and battery powered
	if (!bLowBatDisable && sSps.ACLineStatus == AC_LINE_OFFLINE)
	{
		// on critical battery state make sure that lowbat flag is also set
		if ((sSps.BatteryFlag & BATTERY_FLAG_CRITICAL) != 0)
			sSps.BatteryFlag |= BATTERY_FLAG_LOW;

		// low bat detection
		*pbLBI = ((sSps.BatteryFlag & BATTERY_FLAG_LOW) != 0);

		// very low bat detection
		*pbVLBI = ((sSps.BatteryFlag & BATTERY_FLAG_CRITICAL) != 0);
	}
#endif
	return;
}
