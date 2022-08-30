/*
 *   Serial.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1998 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "emu48.h"
#include "io.h"

#define INTERRUPT ((void)(Chipset.SoftInt=TRUE,bInterrupt=TRUE))

// state of USRQ
#define NINT2ERBZ ((Chipset.IORam[IOC] & (SON | ERBZ)) == (SON | ERBZ) && (Chipset.IORam[RCS] & RBZ) != 0)
#define	NINT2ERBF ((Chipset.IORam[IOC] & (SON | ERBF)) == (SON | ERBF) && (Chipset.IORam[RCS] & RBF) != 0)
#define NINT2ETBE ((Chipset.IORam[IOC] & (SON | ETBE)) == (SON | ETBE) && (Chipset.IORam[TCS] & TBF) == 0)

#define NINT2USRQ (NINT2ERBZ || NINT2ERBF || NINT2ETBE)

static HANDLE     hComm = NULL;

static HANDLE     hCThreadTxd;
static HANDLE     hCThreadEv;

static HANDLE     hEventTxd;
static BOOL       bWriting;
static BYTE       tbr;

static BOOL       bReading;
static BYTE       cBuffer[32];
static WORD       nRp;
static DWORD      dwBytesRead;

static DWORD WINAPI TransmitThread(LPVOID pParam)
{
	OVERLAPPED osWr = { 0 };

	osWr.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	while (bWriting)
	{
		WaitForSingleObject(hEventTxd,INFINITE);
		if (bWriting)
		{
			DWORD dwWritten;
			if (!WriteFile(hComm,(LPCVOID) &tbr,1,&dwWritten,&osWr))
				if (GetLastError() == ERROR_IO_PENDING)
					GetOverlappedResult(hComm,&osWr,&dwWritten,TRUE);
		}
	}

	CloseHandle(osWr.hEvent);				// close write event handle
	return 0;
	UNREFERENCED_PARAMETER(pParam);
}

static DWORD WINAPI EventThread(LPVOID pParam)
{
	DWORD dwEvent;

	bReading = TRUE;						// flag for SerialThread started
	while (bReading)
	{
		_ASSERT(hComm != NULL);
		WaitCommEvent(hComm,&dwEvent,NULL);	// wait for serial event
		if (dwEvent & EV_RXCHAR)			// signal char received
		{
			CommReceive();					// get data
			// interrupt request and emulation thread down
			if (Chipset.SoftInt && Chipset.Shutdn)
			{
				Chipset.bShutdnWake = TRUE;	// wake up from SHUTDN mode
				SetEvent(hEventShutdn);		// wake up emulation thread
			}
		}
		if (dwEvent & EV_TXEMPTY)			// signal transmit buffer empty
		{
			IOBit(TCS,TBZ,FALSE);			// clear transmitter busy bit
			CommTransmit();					// check for new char to transmit
		}
		if (dwEvent & EV_ERR)				// signal error received
		{
			DWORD dwError;

			ClearCommError(hComm,&dwError,NULL);
			if (dwError & (CE_FRAME | CE_OVERRUN | CE_BREAK))
				IOBit(RCS,RER,TRUE);		// receiver error
		}
	}
	return 0;
	UNREFERENCED_PARAMETER(pParam);
}

BOOL CommOpen(LPTSTR strWirePort,LPTSTR strIrPort)
{
	COMMTIMEOUTS CommTimeouts = { MAXDWORD, 0L, 0L, 0L, 0L };

	LPCTSTR strPort = (Chipset.IORam[IR_CTRL] & EIRU) ? strIrPort : strWirePort;

	_ASSERT(Chipset.IORam[IOC] & SON);		// UART on
	CommClose();							// close port if already open

	if (lstrcmp(strPort, _T(NO_SERIAL)))	// port defined
	{
		TCHAR szDevice[256] = _T("\\\\.\\");

		// check if device buffer is big enough
		_ASSERT(lstrlen(szDevice) + lstrlen(strPort) < ARRAYSIZEOF(szDevice));
		if (lstrlen(szDevice) + lstrlen(strPort) >= ARRAYSIZEOF(szDevice))
			return hComm != NULL;

		_tcscat(szDevice,strPort);			// device name
		hComm = CreateFile(szDevice,
			               GENERIC_READ | GENERIC_WRITE,
						   0,
						   NULL,
						   OPEN_EXISTING,
			               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
						   NULL);

		if (hComm != INVALID_HANDLE_VALUE)
		{
			DWORD dwThreadId;

			nRp = 0;						// reset receiver state
			dwBytesRead = 0L;

			SetCommTimeouts(hComm,&CommTimeouts);
			CommSetBaud();

			CommTxBRK();					// update BRK condition

			// event to transmit character
			hEventTxd = CreateEvent(NULL,FALSE,FALSE,NULL);

			// create char transmit handler
			bWriting = TRUE;
			hCThreadTxd = CreateThread(NULL,0,&TransmitThread,NULL,CREATE_SUSPENDED,&dwThreadId);
			_ASSERT(hCThreadTxd);
			SetThreadPriority(hCThreadTxd,THREAD_PRIORITY_ABOVE_NORMAL);
 			ResumeThread(hCThreadTxd);		// start thread

			// create Comm event handler
			bReading = FALSE;
			SetCommMask(hComm,EV_RXCHAR | EV_TXEMPTY | EV_ERR); // event on RX, TX, error
			hCThreadEv = CreateThread(NULL,0,&EventThread,NULL,CREATE_SUSPENDED,&dwThreadId);
			_ASSERT(hCThreadEv);
			SetThreadPriority(hCThreadEv,THREAD_PRIORITY_ABOVE_NORMAL);
 			ResumeThread(hCThreadEv);		// start thread
			while (!bReading) Sleep(0);		// wait for SerialThread started
		}
		else
			hComm = NULL;
	}

	#if defined DEBUG_SERIAL
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("COM port %s.\n"),hComm ? _T("opened"): _T("open error"));
		OutputDebugString(buffer);
	}
	#endif
	return hComm != NULL;
}

VOID CommClose(VOID)
{
	if (hComm != NULL)						// port open
	{
		// workaround to fix problems with some Kermit server programs
		// reason: on one hand we have the character transmitting time base on the
		// selected baudrate, on the other hand the time between sending the last
		// character and closing the port. The last time is much longer on the real
		// calculator than on the emulator running at full speed, therefore the
		// slow down time on the emulator
		Sleep(25);							// slow down time

		bReading = FALSE;					// kill event thread
		SetCommMask(hComm,0L);				// clear all events and force WaitCommEvent to return
		WaitForSingleObject(hCThreadEv,INFINITE);
		CloseHandle(hCThreadEv);

		bWriting = FALSE;					// kill write thread
		SetEvent(hEventTxd);				// continue write thread
		WaitForSingleObject(hCThreadTxd,INFINITE);
		CloseHandle(hCThreadTxd);

		CloseHandle(hEventTxd);				// close Txd event
		CloseHandle(hComm);					// close port
		hComm = NULL;
		#if defined DEBUG_SERIAL
			OutputDebugString(_T("COM port closed.\n"));
		#endif
	}
	return;
}

VOID CommSetBaud(VOID)
{
	if (hComm != NULL)
	{
		const DWORD dwBaudrates[] = { 1200, 1920, 2400, 3840, 4800, 7680, 9600, 15360 };

		struct DCB dcb;

		ZeroMemory(&dcb,sizeof(dcb));
		dcb.DCBlength = sizeof(dcb);
		dcb.BaudRate = dwBaudrates[Chipset.IORam[BAUD] & 0x7];
		dcb.fBinary = TRUE;
		dcb.fParity = TRUE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE;
		dcb.fDsrSensitivity = FALSE;
		dcb.fOutX = FALSE;
		dcb.fErrorChar = FALSE;
		dcb.fNull = FALSE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
		dcb.fAbortOnError = FALSE;			// may changed in further implementations
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;				// no parity check, emulated by software
		dcb.StopBits = TWOSTOPBITS;

		#if defined DEBUG_SERIAL
		{
			TCHAR buffer[256];
			wsprintf(buffer,_T("CommsetBaud: %ld\n"),dcb.BaudRate);
			OutputDebugString(buffer);
		}
		#endif

		SetCommState(hComm,&dcb);
	}
	return;
}

BOOL UpdateUSRQ(VOID)						// USRQ handling
{
	BOOL bUSRQ = NINT2USRQ;
	IOBit(SRQ1,USRQ,bUSRQ);					// update USRQ bit
	return bUSRQ;
}

VOID CommTxBRK(VOID)
{
	if (Chipset.IORam[TCS] & BRK)			// BRK condition
	{
		if (hComm != NULL)					// com port open
		{
			// abort data transfer
			PurgeComm(hComm,PURGE_TXABORT | PURGE_TXCLEAR);
			SetCommBreak(hComm);			// set into BRK state
		}

		// TBF and TBZ bits of TCS are undefined

		if (Chipset.IORam[TCS] & LPB)		// is loopback bit set
		{
			dwBytesRead = nRp = 0;			// clear receive buffer
			cBuffer[dwBytesRead++] = 0;		// save character in receive buffer

			CommReceive();					// receive available byte
			IOBit(RCS,RER,TRUE);			// receiver error (no stop bit)
		}
	}
	else
	{
		if (hComm != NULL)					// com port open
		{
			ClearCommBreak(hComm);			// clear BRK state
		}
	}
	return;
}

VOID CommTransmit(VOID)
{
	BOOL bTxChar = FALSE;

	EnterCriticalSection(&csTxdLock);
	if (   (Chipset.IORam[TCS] & TBZ) == 0	// transmitter not busy
		&& (Chipset.IORam[TCS] & TBF) != 0)	// transmit buffer full
	{
		tbr = (Chipset.IORam[TBR_MSB] << 4) | Chipset.IORam[TBR_LSB];

		IOBit(TCS,TBF,FALSE);				// clear transmit buffer full bit
		IOBit(TCS,TBZ,TRUE);				// set transmitter busy bit

		bTxChar = TRUE;
	}
	LeaveCriticalSection(&csTxdLock);

	if (bTxChar)							// character to transmit
	{
		#if defined DEBUG_SERIAL
		{
			TCHAR buffer[256];
			if (isprint(tbr))
				wsprintf(buffer,_T("-> '%c'\n"),tbr);
			else
				wsprintf(buffer,_T("-> %02X\n"),tbr);
			OutputDebugString(buffer);
		}
		#endif

		if (Chipset.IORam[TCS] & LPB)		// is loopback bit set
		{
			if (dwBytesRead == 0) nRp = 0;	// no character received, reset read pointer
			cBuffer[nRp+dwBytesRead] = tbr;	// save character in receive buffer
			++dwBytesRead;

			CommReceive();					// receive available byte
		}

		if (hComm != NULL)					// com port open
		{
			SetEvent(hEventTxd);			// write TBR byte
		}
		else
		{
			IOBit(TCS,TBZ,FALSE);			// clear transmitter busy bit
		}
	}
	if (UpdateUSRQ())						// update USRQ bit
		INTERRUPT;
	return;
}

VOID CommReceive(VOID)
{
	OVERLAPPED os = { 0 };

	if (!(Chipset.IORam[IOC] & SON))		// UART off
	{
		dwBytesRead = 0L;					// no bytes received
		return;
	}

	EnterCriticalSection(&csRecvLock);
	do
	{
		if (Chipset.IORam[RCS] & RBF)		// receive buffer full
			break;

		// reject reading if com port is closed and not whole operation
		if (hComm && dwBytesRead == 0L)		// com port open and buffer empty
		{
			if (ReadFile(hComm,&cBuffer,sizeof(cBuffer),&dwBytesRead,&os) == FALSE)
				dwBytesRead = 0L;
			else							// bytes received
				nRp = 0;					// reset read pointer
		}

		if (dwBytesRead == 0L)				// receive buffer empty
			break;

		#if defined DEBUG_SERIAL
		{
			TCHAR buffer[256];
			if (isprint(cBuffer[nRp]))
				wsprintf(buffer,_T("<- '%c'\n"),cBuffer[nRp]);
			else
				wsprintf(buffer,_T("<- %02X\n"),cBuffer[nRp]);
			OutputDebugString(buffer);
		}
		#endif

		Chipset.IORam[RBR_MSB] = (cBuffer[nRp] >> 4);
		Chipset.IORam[RBR_LSB] = (cBuffer[nRp] & 0x0f);
		++nRp;
		--dwBytesRead;

		Chipset.IORam[RCS] |= RBF;			// receive buffer full
		if (UpdateUSRQ())					// update USRQ bit
			INTERRUPT;
	}
	while (FALSE);
	LeaveCriticalSection(&csRecvLock);
	return;
}
