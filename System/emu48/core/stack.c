/*
 *   stack.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 2005 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "resource.h"
#include "emu48.h"
#include "io.h"

#define fnRadix		51			// fraction mark
#define fnApprox	105			// exact / approx. mode (HP49G)

#define DOINT		0x02614		// Precision Integer (HP49G)
#define DOREAL		0x02933		// Real
#define DOCSTR		0x02A2C		// String

//################
//#
//#    Low level subroutines
//#
//################

static INT RPL_GetZInt(BYTE CONST *pbyNum,INT nIntLen,LPTSTR cp,INT nSize)
{
	INT i = 0;								// character counter

	_ASSERT(nSize > 0);						// target buffer size

	if (nIntLen > 1)						// has sign nibble
	{
		--nIntLen;							// remove sign from digit length

		// check for valid sign
		_ASSERT(pbyNum[nIntLen] == 0 || pbyNum[nIntLen] == 9);
		if (pbyNum[nIntLen] == 9)			// negative number
		{
			*cp++ = _T('-');				// add sign
			--nSize;						// dec dest buffer size
			++i;							// wrote one character
		}
	}

	if (nIntLen >= nSize) return 0;			// dest buffer overflow
	i += nIntLen;							// adjust character counter

	while (nIntLen-- > 0)					// write all digits
	{
		// check for valid digit
		_ASSERT(pbyNum[nIntLen] >= 0 && pbyNum[nIntLen] <= 9);
		*cp++ = _T('0') + pbyNum[nIntLen];	// and write
	}
	*cp = 0;								// set EOS
	return i;
}

static INT RPL_SetZInt(LPCTSTR cp,LPBYTE pbyNum,INT nSize)
{
	BYTE bySign;
	INT  nStrLen,nNumSize;

	_ASSERT(nSize > 0);						// target buffer size

	nStrLen = lstrlen(cp);					// source string length

	if (   nStrLen == 0						// empty string
		// precisition integer contain only these numbers
		|| _tcsspn(cp,_T("0123456789+-")) != (SIZE_T) nStrLen)
		return 0;

	bySign = (*cp != _T('-')) ? 0 : 9;		// set sign nibble
	if (*cp == _T('-') || *cp == _T('+'))	// skip sign character
	{
		++cp;
		--nStrLen;
	}

	if (nStrLen == 1 && *cp == _T('0'))		// special code for zero
	{
		*pbyNum = 0;						// zero data
		return 1;							// finish
	}

	// nStrLen = no. of digits without sign
    if (nStrLen >= nSize)					// destination buffer too small
		return 0;

	nNumSize = nStrLen + 1;					// no. of written data

	while (--nStrLen >= 0)					// eval all digits
	{
		TCHAR c = cp[nStrLen];

		// only '0' .. '9' are valid here
		if (!((c >= _T('0')) || (c <= _T('9'))))
			return 0;

		c -= _T('0');		
		*pbyNum++ = (BYTE) c;
	}
	*pbyNum = bySign;						// add sign

	return nNumSize;
}

static INT RPL_GetBcd(BYTE CONST *pbyNum,INT nMantLen,INT nExpLen,CONST TCHAR cDec,LPTSTR cp,INT nSize)
{
	BYTE byNib;
	LONG v,lExp;
	BOOL bPflag,bExpflag;
	INT  i;

	lExp = 0;
	for (v = 1; nExpLen--; v *= 10)			// fetch exponent
	{
		lExp += (LONG) *pbyNum++ * v;		// calc. exponent
	}

	if (lExp > v / 2) lExp -= v;			// negative exponent

	lExp -= nMantLen - 1;					// set decimal point to end of mantissa

	i = 0;									// first character
	bPflag = FALSE;							// show no decimal point

	// scan mantissa
	for (v = (LONG) nMantLen - 1; v >= 0 || bPflag; v--)
	{
		if (v >= 0L)						// still mantissa digits left
			byNib = *pbyNum++;
		else
			byNib = 0;						// zero for negativ exponent

		if (!i)								// still delete zeros at end
		{
			if (byNib == 0 && lExp && v > 0) // delete zeros
			{
				lExp++;						// adjust exponent
				continue;
			}

			// TRUE at x.E
			bExpflag = v + lExp > 14 || v + lExp < -nMantLen;
			bPflag = !bExpflag && v < -lExp; // decimal point flag at neg. exponent
		}

		// set decimal point
		if ((bExpflag && v == 0) || (!lExp && i))
		{
			if (i >= nSize) return 0;		// dest buffer overflow
			cp[i++] = cDec;					// write decimal point
			if (v < 0)						// no mantissa digits any more
			{
				if (i >= nSize) return 0;	// dest buffer overflow
				cp[i++] = _T('0');			// write heading zero
			}
			bPflag = FALSE;					// finished with negativ exponents
		}

		if (v >= 0 || bPflag)
		{
			if (i >= nSize) return 0;		// dest buffer overflow
			cp[i++] = (TCHAR) byNib + _T('0'); // write character
		}

		lExp++;								// next position
	}

	if (*pbyNum == 9)						// negative number
	{
		if (i >= nSize) return 0;			// dest buffer overflow
		cp[i++] = _T('-');					// write sign
	}

	if (i >= nSize) return 0;				// dest buffer overflow
	cp[i] = 0;								// set EOS

	for (v = 0; v < (i / 2); v++)			// reverse string
	{
		TCHAR cNib = cp[v];					// swap chars
		cp[v] = cp[i-v-1];
		cp[i-v-1] = cNib;
	}

	// write number with exponent
	if (bExpflag)
	{
		if (i + 5 >= nSize) return 0;		// dest buffer overflow
		i += wsprintf(&cp[i],_T("E%d"),lExp-1);
	}
	return i;
}

static INT RPL_SetBcd(LPCTSTR cp,INT nMantLen,INT nExpLen,CONST TCHAR cDec,LPBYTE pbyNum,INT nSize)
{
	TCHAR cVc[] = _T(".0123456789eE+-");

	BYTE byNum[80];
	INT  i,nIp,nDp,nMaxExp;
	LONG lExp;

	cVc[0] = cDec;							// replace decimal char

	if (   nMantLen + nExpLen >= nSize		// destination buffer too small
		|| !*cp								// empty string
		|| _tcsspn(cp,cVc) != (SIZE_T) lstrlen(cp) // real contain only these numbers
		|| lstrlen(cp) >= ARRAYSIZEOF(byNum)) // ignore too long reals
		return 0;

	byNum[0] = (*cp != _T('-')) ? 0 : 9;	// set sign nibble
	if (*cp == _T('-') || *cp == _T('+'))	// skip sign character
		cp++;

	// only '.', '0' .. '9' are valid here
	if (!((*cp == cDec) || (*cp >= _T('0')) || (*cp <= _T('9'))))
		return 0;

	nIp = 0;								// length of integer part
	if (*cp != cDec)						// no decimal point
	{
		// count integer part
	    while (*cp >= _T('0') && *cp <= _T('9'))
			byNum[++nIp] = *cp++ - _T('0');
		if (!nIp) return 0;
	}

	// only '.', 'E', 'e' or end are valid here
	if (!(!*cp || (*cp == cDec) || (*cp == _T('E')) || (*cp == _T('e'))))
		return 0;

	nDp = 0;								// length of decimal part
	if (*cp == cDec)						// decimal point
	{
		cp++;								// skip '.'

		// count decimal part
		while (*cp >= _T('0') && *cp <= _T('9'))
			byNum[nIp + ++nDp] = *cp++ - _T('0');
	}

	// count number of heading zeros in mantissa
	for (i = 0; byNum[i+1] == 0 && i + 1 < nIp + nDp; ++i) { }

	if (i > 0)								// have to normalize
	{
		INT j;

		nIp -= i;							// for later ajust of exponent
		for (j = 1; j <= nIp + nDp; ++j)	// normalize mantissa
			byNum[j] = byNum[j + i];
	}

	if(byNum[1] == 0)						// number is 0
	{
		ZeroMemory(pbyNum,nMantLen + nExpLen + 1);
		return nMantLen + nExpLen + 1;
	}

	for (i = nIp + nDp; i < nMantLen;)		// fill rest of mantissa with 0
		byNum[++i] = 0;

	// must be 'E', 'e' or end
	if (!(!*cp || (*cp == _T('E')) || (*cp == _T('e'))))
		return 0;

	lExp = 0;
	if (*cp == _T('E') || *cp == _T('e'))
	{
		cp++;								// skip 'E'

		i = FALSE;							// positive exponent
		if (*cp == _T('-') || *cp == _T('+'))
		{
			i = (*cp++ == _T('-'));			// adjust exponent sign
		}

		// exponent symbol must be followed by number
		if (*cp < _T('0') || *cp > _T('9')) return 0;

		while (*cp >= _T('0') && *cp <= _T('9'))
			lExp = lExp * 10 + *cp++ - _T('0');

		if(i) lExp = -lExp;
	}

	if (*cp != 0) return 0;

	// adjust exponent value with exponent from normalized mantissa
	lExp += nIp - 1;

	// calculate max. posive exponent
	for (nMaxExp = 5, i = 1; i < nExpLen; ++i)
		nMaxExp *= 10;

	// check range of exponent
	if ((lExp < 0 && -lExp >= nMaxExp) || (lExp >= nMaxExp))
		return 0;

	if (lExp < 0) lExp += 2 * nMaxExp;		// adjust negative offset

	for (i = nExpLen; i > 0; --i)			// convert number into digits
	{
		byNum[nMantLen + i] = (BYTE) (lExp % 10);
		lExp /= 10;
	}

	// copy to target in reversed order
	for (i = nMantLen + nExpLen; i >= 0; --i)
		*pbyNum++ = byNum[i];

	return nMantLen + nExpLen + 1;
}

//################
//#
//#    Object subroutines
//#
//################

#if 0
static INT IsRealNumber(LPCTSTR cp,INT nMantLen,INT nExpLen,CONST TCHAR cDec,LPBYTE pbyNum,INT nSize)
{
	LPTSTR lpszNumber;
	INT    nLength = 0;

	if ((lpszNumber = DuplicateString(cp)) != NULL)
	{
		LPTSTR p = lpszNumber;
		INT i;

		// cut heading whitespaces
		for (; *p == _T(' ') || *p == _T('\t'); ++p) { }

		// cut tailing whitespaces
		for (i = lstrlen(p); --i >= 0;)
		{
			if (p[i] != _T(' ') && p[i] != _T('\t'))
				break;
		}
		p[++i] = 0;							// new EOS

		nLength = RPL_SetBcd(p,nMantLen,nExpLen,cDec,pbyNum,nSize);
		HeapFree(hHeap,0,lpszNumber);
	}
	return nLength;
}
#endif

static TCHAR GetRadix(VOID)
{
	// get locale decimal point
	// GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDECIMAL,&cDecimal,1);

	return RPL_GetSystemFlag(fnRadix) ? _T(',') : _T('.');
}

static INT DoInt(DWORD dwAddress,LPTSTR cp,INT nSize)
{
	LPBYTE lpbyData;
	INT    nLength,nIntLen;

	nIntLen = Read5(dwAddress) - 5;			// no. of digits
	if (nIntLen <= 0) return 0;				// error in calculator object

	nLength = 0;
	if ((lpbyData = HeapAlloc(hHeap,0,nIntLen)))
	{
		// get precisition integer object content and decode it
		Npeek(lpbyData,dwAddress+5,nIntLen);
		nLength = RPL_GetZInt(lpbyData,nIntLen,cp,nSize);
		HeapFree(hHeap,0,lpbyData);
	}
	return nLength;
}

static INT DoReal(DWORD dwAddress,LPTSTR cp,INT nSize)
{
	BYTE byNumber[16];

	// get real object content and decode it
	Npeek(byNumber,dwAddress,ARRAYSIZEOF(byNumber));
	return RPL_GetBcd(byNumber,12,3,GetRadix(),cp,nSize);
}


//################
//#
//#    Stack routines
//#
//################
#if 0
//
// ID_STACK_COPY
//
LRESULT OnStackCopy(VOID)					// copy data from stack
{
	TCHAR  cBuffer[128];
	HANDLE hClipObj;
	LPBYTE lpbyData;
	DWORD  dwAddress,dwObject,dwSize;
	UINT   uClipboardFormat;

	_ASSERT(nState == SM_RUN);				// emulator must be in RUN state
	if (WaitForSleepState())				// wait for cpu SHUTDN then sleep state
	{
		InfoMessage(_T("The emulator is busy."));
		return 0;
	}

	_ASSERT(nState == SM_SLEEP);

	if ((dwAddress = RPL_Pick(1)) == 0)		// pick address of level1 object
	{
		MessageBeep(MB_OK);					// error beep
		goto error;
	}

	switch (dwObject = Read5(dwAddress))	// select object
	{
	case DOINT:  // Precision Integer (HP49G)
	case DOREAL: // real object
		dwAddress += 5;						// object content

		switch (dwObject)
		{
		case DOINT: // Precision Integer (HP49G)
			// get precision integer object content and decode it
			dwSize = DoInt(dwAddress,cBuffer,ARRAYSIZEOF(cBuffer));
			break;
		case DOREAL: // real object
			// get real object content and decode it
			dwSize = DoReal(dwAddress,cBuffer,ARRAYSIZEOF(cBuffer));
			break;
		}

		// calculate buffer size
		dwSize = (dwSize + 1) * sizeof(*cBuffer);

		// memory allocation for clipboard data
		if ((hClipObj = GlobalAlloc(GMEM_MOVEABLE,dwSize)) == NULL)
			goto error;

		if ((lpbyData = GlobalLock(hClipObj)))
		{
			// copy data to memory
			CopyMemory(lpbyData,cBuffer,dwSize);
			GlobalUnlock(hClipObj);			// unlock memory
		}

		#if defined _UNICODE
			uClipboardFormat = CF_UNICODETEXT;
		#else
			uClipboardFormat = CF_TEXT;
		#endif
		break;
	case DOCSTR: // string
		dwAddress += 5;						// address of string length
		dwSize = (Read5(dwAddress) - 5) / 2; // length of string

		// memory allocation for clipboard data
		if ((hClipObj = GlobalAlloc(GMEM_MOVEABLE,dwSize + 1)) == NULL)
			goto error;

		if ((lpbyData = GlobalLock(hClipObj))) // lock memory
		{
			// copy data into clipboard buffer
			for (dwAddress += 5;dwSize-- > 0;dwAddress += 2,++lpbyData)
				*lpbyData = Read2(dwAddress);
			*lpbyData = 0;					// set EOS

			GlobalUnlock(hClipObj);			// unlock memory
			uClipboardFormat = CF_TEXT;		// always text
		}
		break;
	default:
		MessageBeep(MB_OK);					// error beep
		goto error;
	}

	if (OpenClipboard(hWnd))
	{
		if (EmptyClipboard())
			SetClipboardData(uClipboardFormat,hClipObj);
		else
			GlobalFree(hClipObj);
		CloseClipboard();
	}
	else										// clipboard open failed
	{
		GlobalFree(hClipObj);
	}

error:
	SwitchToState(SM_RUN);
	return 0;
}

//
// ID_STACK_PASTE
//
LRESULT OnStackPaste(VOID)					// paste data to stack
{
	#if defined _UNICODE
		#define CF_TEXTFORMAT CF_UNICODETEXT
	#else
		#define CF_TEXTFORMAT CF_TEXT
	#endif

	HANDLE hClipObj;

	BOOL bSuccess = FALSE;

	// check if clipboard format is available
	if (!IsClipboardFormatAvailable(CF_TEXTFORMAT))
	{
		MessageBeep(MB_OK);					// error beep
		return 0;
	}

	SuspendDebugger();						// suspend debugger
	bDbgAutoStateCtrl = FALSE;				// disable automatic debugger state control

	// calculator off, turn on
	if (!(Chipset.IORam[BITOFFSET]&DON))
	{
		KeyboardEvent(TRUE,0,0x8000);
		KeyboardEvent(FALSE,0,0x8000);

		// wait for sleep mode
		while(Chipset.Shutdn == FALSE) Sleep(0);
	}

	_ASSERT(nState == SM_RUN);				// emulator must be in RUN state
	if (WaitForSleepState())				// wait for cpu SHUTDN then sleep state
	{
		InfoMessage(_T("The emulator is busy."));
		goto cancel;
	}

	_ASSERT(nState == SM_SLEEP);

	if (OpenClipboard(hWnd))
	{
		if ((hClipObj = GetClipboardData(CF_TEXTFORMAT)))
		{
			LPCTSTR lpstrClipdata;
			LPBYTE  lpbyData;

			if ((lpstrClipdata = GlobalLock(hClipObj)))
			{
				BYTE  byNumber[128];
				DWORD dwAddress;
				INT   s;

				do
				{
					// HP49G or HP49G+ in exact mode
					if (   (cCurrentRomType == 'X' || cCurrentRomType == 'Q')
						&& !RPL_GetSystemFlag(fnApprox))
					{
						// try to convert string to HP49 precision integer
						s = RPL_SetZInt(lpstrClipdata,byNumber,sizeof(byNumber));

						if (s > 0)			// is a real number for exact mode
						{
							// get TEMPOB memory for HP49 precision integer object
							dwAddress = RPL_CreateTemp(s+5+5,TRUE);
							if ((bSuccess = (dwAddress > 0)))
							{
								Write5(dwAddress,DOINT); // prolog
								Write5(dwAddress+5,s+5); // size
								Nwrite(byNumber,dwAddress+10,s); // data

								// push object to stack
								RPL_Push(1,dwAddress);
							}
							break;
						}
					}

					// try to convert string to real format
					s = RPL_SetBcd(lpstrClipdata,12,3,GetRadix(),byNumber,sizeof(byNumber));

					if (s > 0)				// is a real number
					{
						// get TEMPOB memory for real object
						dwAddress = RPL_CreateTemp(16+5,TRUE);
						if ((bSuccess = (dwAddress > 0)))
						{
							Write5(dwAddress,DOREAL); // prolog
							Nwrite(byNumber,dwAddress+5,s); // data

							// push object to stack
							RPL_Push(1,dwAddress);
						}
						break;
					}

					// any other format
					{
						DWORD dwSize = lstrlen(lpstrClipdata);
						if ((lpbyData = HeapAlloc(hHeap,0,dwSize * 2)))
						{
							LPBYTE lpbySrc,lpbyDest;
							DWORD  dwLoop;

							#if defined _UNICODE
								// copy data UNICODE -> ASCII
								WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK,
													lpstrClipdata, dwSize,
													lpbyData+dwSize, dwSize, NULL, NULL);
							#else
								// copy data
								memcpy(lpbyData+dwSize,lpstrClipdata,dwSize);
							#endif

							// unpack data
							lpbySrc = lpbyData+dwSize;
							lpbyDest = lpbyData;
							dwLoop = dwSize;
							while (dwLoop-- > 0)
							{
								BYTE byTwoNibs = *lpbySrc++;
								*lpbyDest++ = (BYTE) (byTwoNibs & 0xF);
								*lpbyDest++ = (BYTE) (byTwoNibs >> 4);
							}

							dwSize *= 2;	// size in nibbles

							// get TEMPOB memory for string object
							dwAddress = RPL_CreateTemp(dwSize+10,TRUE);
							if ((bSuccess = (dwAddress > 0)))
							{
								Write5(dwAddress,DOCSTR); // String
								Write5(dwAddress+5,dwSize+5); // length of String
								Nwrite(lpbyData,dwAddress+10,dwSize); // data

								// push object to stack
								RPL_Push(1,dwAddress);
							}
							HeapFree(hHeap,0,lpbyData);
						}
					}
				}
				while(FALSE);

				GlobalUnlock(hClipObj);
			}
		}
		CloseClipboard();
	}

	SwitchToState(SM_RUN);					// run state
	while (nState!=nNextState) Sleep(0);
	_ASSERT(nState == SM_RUN);

	if (bSuccess == FALSE)					// data not copied
		goto cancel;

	KeyboardEvent(TRUE,0,0x8000);
	Sleep(200);
	KeyboardEvent(FALSE,0,0x8000);

	// wait for sleep mode
	while(Chipset.Shutdn == FALSE) Sleep(0);

cancel:
	bDbgAutoStateCtrl = TRUE;				// enable automatic debugger state control
	ResumeDebugger();
	return 0;
	#undef CF_TEXTFORMAT
}


#endif

