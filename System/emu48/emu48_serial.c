


#include "pch.h"
#include "emu48.h"
#include "io.h"



#define INTERRUPT ((void)(Chipset.SoftInt=TRUE,bInterrupt=TRUE))

// state of USRQ
#define NINT2ERBZ ((Chipset.IORam[IOC] & (SON | ERBZ)) == (SON | ERBZ) && (Chipset.IORam[RCS] & RBZ) != 0)
#define	NINT2ERBF ((Chipset.IORam[IOC] & (SON | ERBF)) == (SON | ERBF) && (Chipset.IORam[RCS] & RBF) != 0)
#define NINT2ETBE ((Chipset.IORam[IOC] & (SON | ETBE)) == (SON | ETBE) && (Chipset.IORam[TCS] & TBF) == 0)

#define NINT2USRQ (NINT2ERBZ || NINT2ERBF || NINT2ETBE)

static HANDLE     hEventTxd;
static BOOL       bWriting;
static BYTE       tbr;


static BOOL       bReading;
static BYTE       cBuffer[32];
static WORD       nRp;
static DWORD      dwBytesRead;


BOOL CommOpen(LPTSTR strWirePort,LPTSTR strIrPort)
{
	LPCTSTR strPort = (Chipset.IORam[IR_CTRL] & EIRU) ? strIrPort : strWirePort;

	_ASSERT(Chipset.IORam[IOC] & SON);		// UART on
	CommClose();							// close port if already open


}


VOID CommClose(VOID)
{

	return;
}


VOID CommSetBaud(VOID)
{

	//dwBaudrates[Chipset.IORam[BAUD] & 0x7];

	return;
}


VOID CommTxBRK(VOID)
{
	if (Chipset.IORam[TCS] & BRK)			// BRK condition
	{
		
			// abort data transfer


		// TBF and TBZ bits of TCS are undefined

		if (Chipset.IORam[TCS] & LPB)		// is loopback bit set
		{
            

			CommReceive();					// receive available byte
			IOBit(RCS,RER,TRUE);			// receiver error (no stop bit)
		}
	}
	else
	{
			// com port open
		
			// clear BRK state
		
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

		//if (hComm != NULL)					// com port open
		//{
		//	SetEvent(hEventTxd);			// write TBR byte
		//}
		//else
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
    
/*
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
*/
	return;
}


BOOL UpdateUSRQ(VOID)						// USRQ handling
{
	BOOL bUSRQ = NINT2USRQ;
	IOBit(SRQ1,USRQ,bUSRQ);					// update USRQ bit
	return bUSRQ;
}



