/*
 *   files.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */
#include "pch.h"
#include "emu48.h"
#include "ops.h"
#include "io.h"								// I/O register definitions
#include "kml.h"
#include "i28f160.h"						// flash support
#include "debugger.h"

TCHAR  szEmuDirectory[MAX_PATH];
TCHAR  szCurrentDirectory[MAX_PATH];
TCHAR  szCurrentKml[MAX_PATH];
TCHAR  szBackupKml[MAX_PATH];
TCHAR  szCurrentFilename[MAX_PATH];
TCHAR  szBackupFilename[MAX_PATH];
TCHAR  szBufferFilename[MAX_PATH];
TCHAR  szPort2Filename[MAX_PATH];

LPBYTE pbyRom = NULL;
static HANDLE  hRomFile = NULL;
static HANDLE  hRomMap = NULL;
DWORD  dwRomSize = 0;
BYTE   cCurrentRomType = 0;					// Model -> hardware
UINT   nCurrentClass = 0;					// Class -> derivate
BOOL   bRomWriteable = TRUE;				// flag if ROM writeable

static HANDLE  hPort2File = NULL;
static HANDLE  hPort2Map = NULL;
LPBYTE pbyPort2 = NULL;
BOOL   bPort2Writeable = FALSE;
BOOL   bPort2IsShared = FALSE;
DWORD  dwPort2Size = 0;						// size of mapped port2
DWORD  dwPort2Mask = 0;

// document signatures
static BYTE pbySignatureA[16] = "Emu38 Document\xFE";
static BYTE pbySignatureB[16] = "Emu39 Document\xFE";
static BYTE pbySignatureE[16] = "Emu48 Document\xFE";
static BYTE pbySignatureW[16] = "Win48 Document\xFE";
static BYTE pbySignatureV[16] = "Emu49 Document\xFE";
static HANDLE hCurrentFile = NULL;

static CHIPSET BackupChipset;

BOOL   bBackup = FALSE;

//################
//#
//#    Subroutine for Write on Stack
//#
//################

WORD WriteStack(LPBYTE lpBuf,DWORD dwSize)	// separated from LoadObject()
{
	BOOL   bBinary;
	DWORD  dwAddress, i;

	bBinary =  ((lpBuf[dwSize+0]=='H')
	        &&  (lpBuf[dwSize+1]=='P')
	        &&  (lpBuf[dwSize+2]=='H')
	        &&  (lpBuf[dwSize+3]=='P')
	        &&  (lpBuf[dwSize+4]=='4')
	        &&  (lpBuf[dwSize+5]==((cCurrentRomType!='X') ? '8' : '9'))
	        &&  (lpBuf[dwSize+6]=='-'));

	for (i = 0; i < dwSize; i++)
	{
		BYTE byTwoNibs = lpBuf[i+dwSize];
		lpBuf[i*2  ] = (BYTE)(byTwoNibs&0xF);
		lpBuf[i*2+1] = (BYTE)(byTwoNibs>>4);
	}

	if (bBinary == TRUE)
	{ // load as binary
		dwSize    = RPL_ObjectSize(lpBuf+16);
		dwAddress = RPL_CreateTemp(dwSize);
		if (dwAddress == 0) return S_ERR_BINARY;

		Nwrite(lpBuf+16,dwAddress,dwSize);
	}
	else
	{ // load as string
		BYTE lpHead[5];
		dwSize *= 2;
		dwAddress = RPL_CreateTemp(dwSize+10);
		if (dwAddress == 0) return S_ERR_ASCII;

		Nunpack(lpHead,0x02A2C,5);			// String
		Nwrite(lpHead,dwAddress,5);
		Nunpack(lpHead,dwSize+5,5);			// length of String
		Nwrite(lpHead,dwAddress+5,5);
		Nwrite(lpBuf,dwAddress+10,dwSize);	// data
	}
	RPL_Push(dwAddress);
	return S_ERR_NO;
}



//################
//#
//#    Window Position Tools
//#
//################

VOID SetWindowLocation(HWND hWnd,INT nPosX,INT nPosY)
{
	WINDOWPLACEMENT wndpl;
	RECT *pRc = &wndpl.rcNormalPosition;

	wndpl.length = sizeof(wndpl);
	GetWindowPlacement(hWnd,&wndpl);
	pRc->right = pRc->right - pRc->left + nPosX;
	pRc->bottom = pRc->bottom - pRc->top + nPosY;
	pRc->left = nPosX;
	pRc->top = nPosY;
	SetWindowPlacement(hWnd,&wndpl);
	return;
}



//################
//#
//#    Patch
//#
//################

static __inline BYTE Asc2Nib(BYTE c)
{
	if (c<'0') return 0;
	if (c<='9') return c-'0';
	if (c<'A') return 0;
	if (c<='F') return c-'A'+10;
	if (c<'a') return 0;
	if (c<='f') return c-'a'+10;
	return 0;
}

// functions to restore ROM patches
typedef struct tnode
{
	DWORD  dwAddress;						// patch address
	BYTE   byROM;							// original ROM value
	BYTE   byPatch;							// patched ROM value
	struct tnode *next;						// next node
} TREENODE;

static TREENODE *nodePatch = NULL;

static BOOL PatchNibble(DWORD dwAddress, BYTE byPatch)
{
	TREENODE *p;

	_ASSERT(pbyRom);						// ROM defined
	if((p = HeapAlloc(hHeap,0,sizeof(TREENODE))) == NULL)
		return TRUE;

	p->dwAddress = dwAddress;				// save current values
	p->byROM = pbyRom[dwAddress];
	p->byPatch = byPatch;
	p->next = nodePatch;					// save node
	nodePatch = p;

	pbyRom[dwAddress] = byPatch;			// patch ROM
	return FALSE;
}

static VOID RestorePatches(VOID)
{
	TREENODE *p;

	_ASSERT(pbyRom);						// ROM defined
	while (nodePatch != NULL)
	{
		// restore original data
		pbyRom[nodePatch->dwAddress] = nodePatch->byROM;

		p = nodePatch->next;				// save pointer to next node
		HeapFree(hHeap,0,nodePatch);		// free node
		nodePatch = p;						// new node
	}
	return;
}

VOID UpdatePatches(BOOL bPatch)
{
	TREENODE *p = nodePatch;

	_ASSERT(pbyRom);						// ROM defined
	while (p != NULL)
	{
		if (bPatch)
		{
			// save original data and patch address
			p->byROM = pbyRom[p->dwAddress];
			pbyRom[p->dwAddress] = p->byPatch;
		}
		else
		{
			// restore original data
			pbyRom[p->dwAddress] = p->byROM;
		}

		p = p->next;						// next node
	}
	return;
}

BOOL PatchRom(LPCTSTR szFilename)
{
	HANDLE hFile = NULL;
	DWORD  dwFileSizeLow = 0;
	DWORD  dwFileSizeHigh = 0;
	DWORD  lBytesRead = 0;
	LPBYTE lpStop,lpBuf = NULL;
	DWORD  dwAddress = 0;
	UINT   nPos = 0;

	if (pbyRom == NULL) return FALSE;
	SetCurrentDirectory(szEmuDirectory);
	hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	SetCurrentDirectory(szCurrentDirectory);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;
	dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
	if (dwFileSizeLow <= 5)
	{ // file is too small.
		CloseHandle(hFile);
		return FALSE;
	}
	if (dwFileSizeHigh != 0)
	{ // file is too large.
		CloseHandle(hFile);
		return FALSE;
	}
	lpBuf = HeapAlloc(hHeap,0,dwFileSizeLow+1);
	if (lpBuf == NULL)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	ReadFile(hFile, lpBuf, dwFileSizeLow, &lBytesRead, NULL);
	CloseHandle(hFile);
	lpBuf[dwFileSizeLow] = 0;
	nPos = 0;
	while (lpBuf[nPos])
	{
		do // remove blank space
		{
			if (  (lpBuf[nPos]!=' ')
				&&(lpBuf[nPos]!='\n')
				&&(lpBuf[nPos]!='\r')
				&&(lpBuf[nPos]!='\t')) break;
			nPos++;
		} while (lpBuf[nPos]);
		if (lpBuf[nPos]==';') // comment ?
		{
			do
			{
				nPos++;
				if (lpBuf[nPos]=='\n')
				{
					nPos++;
					break;
				}
			} while (lpBuf[nPos]);
			continue;
		}
		dwAddress = strtoul(&lpBuf[nPos],(CHAR **)&lpStop,16);
		nPos += lpStop-&lpBuf[nPos]+1;
		if (*lpStop != ':' || *lpStop == 0)
			continue;
		while (lpBuf[nPos])
		{
			if (isxdigit(lpBuf[nPos]) == FALSE) break;
			// patch ROM and save original nibble
			PatchNibble(dwAddress, Asc2Nib(lpBuf[nPos]));
			dwAddress = (dwAddress+1)&(dwRomSize-1);
			nPos++;
		}
	}
	HeapFree(hHeap,0,lpBuf);
	return TRUE;
}



//################
//#
//#    ROM
//#
//################

static WORD CrcRom(VOID)					// calculate fingerprint of ROM
{
	DWORD dwCount;
	DWORD dwFileSize;
	DWORD dwCrc = 0;

	dwFileSize = GetFileSize(hRomFile, &dwCount); // get real filesize
	_ASSERT(dwCount == 0);					// isn't created by MapRom()

	_ASSERT(pbyRom);						// view on ROM
	// use checksum, because it's faster
	for (dwCount = 0;dwCount < dwFileSize; dwCount+=sizeof(dwCrc))
		dwCrc += *((DWORD *) &pbyRom[dwCount]);

	return (WORD) dwCrc;
}

BOOL MapRom(LPCTSTR szFilename)
{
	DWORD dwFileSizeHigh;

	// open ROM for writing
	BOOL bWrite = (cCurrentRomType == 'X') ? bRomWriteable : FALSE;

	if (pbyRom != NULL)
	{
		return FALSE;
	}
	SetCurrentDirectory(szEmuDirectory);
	if (bWrite)								// ROM writeable
	{
		hRomFile = CreateFile(szFilename,
							  GENERIC_READ|GENERIC_WRITE,
							  FILE_SHARE_READ,
							  NULL,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);
		if (hRomFile == INVALID_HANDLE_VALUE)
		{
			bWrite = FALSE;					// ROM not writeable
			hRomFile = CreateFile(szFilename,
								  GENERIC_READ,
								  FILE_SHARE_READ|FILE_SHARE_WRITE,
								  NULL,
								  OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL,
								  NULL);
		}
	}
	else									// writing ROM disabled
	{
		hRomFile = CreateFile(szFilename,
							  GENERIC_READ,
							  FILE_SHARE_READ,
							  NULL,
							  OPEN_EXISTING,
							  FILE_ATTRIBUTE_NORMAL,
							  NULL);
	}
	SetCurrentDirectory(szCurrentDirectory);
	if (hRomFile == INVALID_HANDLE_VALUE)
	{
		hRomFile = NULL;
		return FALSE;
	}
	dwRomSize = GetFileSize(hRomFile, &dwFileSizeHigh);
	if (dwFileSizeHigh != 0)
	{ // file is too large.
		CloseHandle(hRomFile);
		hRomFile = NULL;
		dwRomSize = 0;
		return FALSE;
	}
	hRomMap = CreateFileMapping(hRomFile, NULL, bWrite ? PAGE_READWRITE : PAGE_WRITECOPY, 0, dwRomSize, NULL);
	if (hRomMap == NULL)
	{
		CloseHandle(hRomFile);
		hRomFile = NULL;
		dwRomSize = 0;
		return FALSE;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		AbortMessage(_T("Sharing file mapping handle."));
	}
	pbyRom = MapViewOfFile(hRomMap, bWrite ? FILE_MAP_WRITE : FILE_MAP_COPY, 0, 0, dwRomSize);
	if (pbyRom == NULL)
	{
		CloseHandle(hRomMap);
		CloseHandle(hRomFile);
		hRomMap = NULL;
		hRomFile = NULL;
		dwRomSize = 0;
		return FALSE;
	}
	return TRUE;
}

VOID UnmapRom(VOID)
{
	if (pbyRom==NULL) return;
	RestorePatches();						// restore ROM Patches
	UnmapViewOfFile(pbyRom);
	CloseHandle(hRomMap);
	CloseHandle(hRomFile);
	pbyRom = NULL;
	hRomMap = NULL;
	hRomFile = NULL;
	dwRomSize = 0;
	return;
}



//################
//#
//#    Port2
//#
//################

static WORD CrcPort2(VOID)					// calculate fingerprint of port2
{
	DWORD dwCount;
	DWORD dwFileSize;
	WORD wCrc = 0;

	// port2 CRC isn't available
	if (pbyPort2 == NULL) return wCrc;

	dwFileSize = GetFileSize(hPort2File, &dwCount); // get real filesize
	_ASSERT(dwCount == 0);					// isn't created by MapPort2()

	for (dwCount = 0;dwCount < dwFileSize; ++dwCount)
		wCrc = (wCrc >> 4) ^ (((wCrc ^ ((WORD) pbyPort2[dwCount])) & 0xf) * 0x1081);
	return wCrc;
}

BOOL MapPort2(LPCTSTR szFilename)
{
	DWORD dwFileSizeLo;
	DWORD dwFileSizeHi;

	if (pbyPort2 != NULL) return FALSE;
	bPort2Writeable = TRUE;
	dwPort2Size = 0;						// reset size of port2

	SetCurrentDirectory(szEmuDirectory);
	hPort2File = CreateFile(szFilename,
							GENERIC_READ|GENERIC_WRITE,
							bPort2IsShared ? FILE_SHARE_READ : 0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	if (hPort2File == INVALID_HANDLE_VALUE)
	{
		bPort2Writeable = FALSE;
		hPort2File = CreateFile(szFilename,
								GENERIC_READ,
								bPort2IsShared ? (FILE_SHARE_READ|FILE_SHARE_WRITE) : 0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
		if (hPort2File == INVALID_HANDLE_VALUE)
		{
			SetCurrentDirectory(szCurrentDirectory);
			hPort2File = NULL;
			return FALSE;
		}
	}
	SetCurrentDirectory(szCurrentDirectory);
	dwFileSizeLo = GetFileSize(hPort2File, &dwFileSizeHi);
	if (dwFileSizeHi != 0)
	{ // file is too large.
		CloseHandle(hPort2File);
		hPort2File = NULL;
		dwPort2Mask = 0;
		bPort2Writeable = FALSE;
		return FALSE;
	}
	if (dwFileSizeLo & 0x2FFFF)
	{ // file size is wrong
		CloseHandle(hPort2File);
		hPort2File = NULL;
		dwPort2Mask = 0;
		bPort2Writeable = FALSE;
		return FALSE;
	}
	dwPort2Mask = (dwFileSizeLo - 1) >> 18;	// mask for valid address lines of the BS-FF
	hPort2Map = CreateFileMapping(hPort2File, NULL, bPort2Writeable ? PAGE_READWRITE : PAGE_READONLY,
								  0, dwFileSizeLo, NULL);
	if (hPort2Map == NULL)
	{
		CloseHandle(hPort2File);
		hPort2File = NULL;
		dwPort2Mask = 0;
		bPort2Writeable = FALSE;
		return FALSE;
	}
	pbyPort2 = MapViewOfFile(hPort2Map, bPort2Writeable ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, dwFileSizeLo);
	if (pbyPort2 == NULL)
	{
		CloseHandle(hPort2Map);
		CloseHandle(hPort2File);
		hPort2Map = NULL;
		hPort2File = NULL;
		dwPort2Mask = 0;
		bPort2Writeable = FALSE;
		return FALSE;
	}
	dwPort2Size = dwFileSizeLo / 2048;		// mapping size of port2
	return TRUE;
}

VOID UnmapPort2(VOID)
{
	if (pbyPort2==NULL) return;
	UnmapViewOfFile(pbyPort2);
	CloseHandle(hPort2Map);
	CloseHandle(hPort2File);
	pbyPort2 = NULL;
	hPort2Map = NULL;
	hPort2File = NULL;
	dwPort2Size = 0;						// reset size of port2
	dwPort2Mask = 0;
	bPort2Writeable = FALSE;
	return;
}



//################
//#
//#    Documents
//#
//################

VOID ResetDocument(VOID)
{
	if (szCurrentKml[0])
	{
		KillKML();
	}
	if (hCurrentFile)
	{
		CloseHandle(hCurrentFile);
		hCurrentFile = NULL;
	}
	szCurrentKml[0] = 0;
	szCurrentFilename[0]=0;
	if (Chipset.Port0) HeapFree(hHeap,0,Chipset.Port0);
	if (Chipset.Port1) HeapFree(hHeap,0,Chipset.Port1);
	if (Chipset.Port2) HeapFree(hHeap,0,Chipset.Port2); else UnmapPort2();
	ZeroMemory(&Chipset,sizeof(Chipset));
	ZeroMemory(&RMap,sizeof(RMap));			// delete MMU mappings
	ZeroMemory(&WMap,sizeof(WMap));
	UpdateWindowStatus();
	return;
}

BOOL NewDocument(VOID)
{
	ResetDocument();

	if (!DisplayChooseKml(0)) goto restore;
	if (!InitKML(szCurrentKml,FALSE)) goto restore;
	Chipset.type = cCurrentRomType;

	if (Chipset.type == '6' || Chipset.type == 'A')	// HP38G
	{
		Chipset.Port0Size = (Chipset.type == 'A') ? 32 : 64;
		Chipset.Port1Size = 0;
		Chipset.Port2Size = 0;

		Chipset.cards_status = 0x0;
	}
	if (Chipset.type == 'E')				// HP39/40G
	{
		Chipset.Port0Size = 128;
		Chipset.Port1Size = 0;
		Chipset.Port2Size = 128;

		Chipset.cards_status = 0xF;

		bPort2Writeable = TRUE;				// port2 is writeable
	}
	if (Chipset.type == 'S')				// HP48SX
	{
		Chipset.Port0Size = 32;
		Chipset.Port1Size = 128;
		Chipset.Port2Size = 0;

		Chipset.cards_status = 0x5;

		// use 2nd command line argument if defined
		MapPort2((nArgc < 3) ? szPort2Filename : ppArgv[2]);
	}
	if (Chipset.type == 'G')				// HP48GX
	{
		Chipset.Port0Size = 128;
		Chipset.Port1Size = 128;
		Chipset.Port2Size = 0;

		Chipset.cards_status = 0xA;

		// use 2nd command line argument if defined
		MapPort2((nArgc < 3) ? szPort2Filename : ppArgv[2]);
	}
	if (Chipset.type == 'X')				// HP49G
	{
		Chipset.Port0Size = 256;
		Chipset.Port1Size = 128;
		Chipset.Port2Size = 128;

		Chipset.cards_status = 0xF;
		bPort2Writeable = TRUE;				// port2 is writeable

		FlashInit();						// init flash structure
	}

	Chipset.IORam[LPE] = RST;				// set ReSeT bit at power on reset

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
	LoadBreakpointList(NULL);				// clear debugger breakpoint list
	RomSwitch(0);							// boot ROM view of HP49G and map memory
	SaveBackup();
	return TRUE;
restore:
	RestoreBackup();
	ResetBackup();

	// HP48SX/GX
	if(Chipset.type == 'S' || Chipset.type == 'G')
	{
		// use 2nd command line argument if defined
		MapPort2((nArgc < 3) ? szPort2Filename : ppArgv[2]);
	}
	if (pbyRom)
	{
		SetWindowLocation(hWnd,Chipset.nPosX,Chipset.nPosY);
		Map(0x00,0xFF);
	}
	return FALSE;
}

BOOL OpenDocument(LPCTSTR szFilename)
{
	HANDLE  hFile = INVALID_HANDLE_VALUE;
	DWORD   lBytesRead,lSizeofChipset;
	BYTE    pbyFileSignature[16];
	LPBYTE  pbySig;
	UINT    ctBytesCompared;
	UINT    nLength;

	SaveBackup();
	ResetDocument();

	// Open file
	if (lstrcmpi(szBackupFilename, szFilename)==0)
	{
		if (YesNoMessage(_T("Do you want to reload this document ?")) == IDNO)
			goto restore;
	}
	hFile = CreateFile(szFilename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AbortMessage(_T("This file is missing or already loaded in another instance of Emu48."));
		goto restore;
	}

	// Read and Compare signature
	ReadFile(hFile, pbyFileSignature, 16, &lBytesRead, NULL);
	switch (pbyFileSignature[0])
	{
	case 'E':
		pbySig = (pbyFileSignature[3] == '3')
			   ? ((pbyFileSignature[4] == '8') ? pbySignatureA : pbySignatureB)
			   : ((pbyFileSignature[4] == '8') ? pbySignatureE : pbySignatureV);
		for (ctBytesCompared=0; ctBytesCompared<14; ctBytesCompared++)
		{
			if (pbyFileSignature[ctBytesCompared]!=pbySig[ctBytesCompared])
			{
				AbortMessage(_T("This file is not a valid Emu48 document."));
				goto restore;
			}
		}
		break;
	case 'W':
		for (ctBytesCompared=0; ctBytesCompared<14; ctBytesCompared++)
		{
			if (pbyFileSignature[ctBytesCompared]!=pbySignatureW[ctBytesCompared])
			{
				AbortMessage(_T("This file is not a valid Win48 document."));
				goto restore;
			}
		}
		break;
	default:
		AbortMessage(_T("This file is not a valid document."));
		goto restore;
	}

	switch (pbyFileSignature[14])
	{
	case 0xFE: // Win48 2.1 / Emu4x 0.99.x format
		ReadFile(hFile,&nLength,sizeof(nLength),&lBytesRead,NULL);
		#if defined _UNICODE
		{
			LPSTR szTmp = HeapAlloc(hHeap,0,nLength);
			if (szTmp == NULL)
			{
				AbortMessage(_T("Memory Allocation Failure."));
				goto restore;
			}
			ReadFile(hFile, szTmp, nLength, &lBytesRead, NULL);
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTmp, lBytesRead,
								szCurrentKml, ARRAYSIZEOF(szCurrentKml));
			HeapFree(hHeap,0,szTmp);
		}
		#else
		{
			ReadFile(hFile, szCurrentKml, nLength, &lBytesRead, NULL);
		}
		#endif
		if (nLength != lBytesRead) goto read_err;
		szCurrentKml[nLength] = 0;
		break;
	case 0xFF: // Win48 2.05 format
		break;
	default:
		AbortMessage(_T("This file is for an unknown version of Emu48."));
		goto restore;
	}

	// read chipset size inside file
	ReadFile(hFile, &lSizeofChipset, sizeof(lSizeofChipset), &lBytesRead, NULL);
	if (lBytesRead != sizeof(lSizeofChipset)) goto read_err;
	if (lSizeofChipset <= sizeof(Chipset))	// actual or older chipset version
	{
		// read chipset content
		ZeroMemory(&Chipset,sizeof(Chipset));	// init chipset
		ReadFile(hFile, &Chipset, lSizeofChipset, &lBytesRead, NULL);
	}
	else									// newer chipset version
	{
		// read my used chipset content
		ReadFile(hFile, &Chipset, sizeof(Chipset), &lBytesRead, NULL);

		// skip rest of chipset
		SetFilePointer(hFile, lSizeofChipset-sizeof(Chipset), NULL, FILE_CURRENT);
		lSizeofChipset = sizeof(Chipset);
	}
	if (lBytesRead != lSizeofChipset) goto read_err;
	Chipset.Port0 = NULL;
	Chipset.Port1 = NULL;
	Chipset.Port2 = NULL;

	SetWindowLocation(hWnd,Chipset.nPosX,Chipset.nPosY);

	if (szCurrentKml == NULL)
	{
		if (!DisplayChooseKml(Chipset.type))
			goto restore;
	}
	while (TRUE)
	{
		BOOL bOK;

		bOK = InitKML(szCurrentKml,FALSE);
		bOK = bOK && (cCurrentRomType == Chipset.type);
		if (bOK) break;

		KillKML();
		if (!DisplayChooseKml(Chipset.type))
			goto restore;
	}

	FlashInit();							// init flash structure

	if (Chipset.Port0Size)
	{
		Chipset.Port0 = HeapAlloc(hHeap,0,Chipset.Port0Size*2048);
		if (Chipset.Port0 == NULL)
		{
			AbortMessage(_T("Memory Allocation Failure."));
			goto restore;
		}

		ReadFile(hFile, Chipset.Port0, Chipset.Port0Size*2048, &lBytesRead, NULL);
		if (lBytesRead != Chipset.Port0Size*2048) goto read_err;
	}

	if (Chipset.Port1Size)
	{
		Chipset.Port1 = HeapAlloc(hHeap,0,Chipset.Port1Size*2048);
		if (Chipset.Port1 == NULL)
		{
			AbortMessage(_T("Memory Allocation Failure."));
			goto restore;
		}

		ReadFile(hFile, Chipset.Port1, Chipset.Port1Size*2048, &lBytesRead, NULL);
		if (lBytesRead != Chipset.Port1Size*2048) goto read_err;
	}

	// HP48SX/GX
	if(cCurrentRomType=='S' || cCurrentRomType=='G')
	{
		MapPort2((nArgc < 3) ? szPort2Filename : ppArgv[2]);
		// port2 changed and card detection enabled
		if (    Chipset.wPort2Crc != CrcPort2()
			&& (Chipset.IORam[CARDCTL] & ECDT) != 0 && (Chipset.IORam[TIMER2_CTRL] & RUN) != 0
		   )
		{
			Chipset.HST |= MP;				// set Module Pulled
			IOBit(SRQ2,NINT,FALSE);			// set NINT to low
			Chipset.SoftInt = TRUE;			// set interrupt
			bInterrupt = TRUE;
		}
	}
	else									// HP38G, HP39/40G, HP49G
	{
		if (Chipset.Port2Size)
		{
			Chipset.Port2 = HeapAlloc(hHeap,0,Chipset.Port2Size*2048);
			if (Chipset.Port2 == NULL)
			{
				AbortMessage(_T("Memory Allocation Failure."));
				goto restore;
			}

			ReadFile(hFile, Chipset.Port2, Chipset.Port2Size*2048, &lBytesRead, NULL);
			if (lBytesRead != Chipset.Port2Size*2048) goto read_err;

			bPort2Writeable = TRUE;
			Chipset.cards_status = 0xF;
		}
	}

	LoadBreakpointList(hFile);				// load debugger breakpoint list
	RomSwitch(Chipset.Bank_FF);				// reload ROM view of HP49G and map memory

	if (Chipset.wRomCrc != CrcRom())		// ROM changed
	{
		CpuReset();
		Chipset.Shutdn = FALSE;				// automatic restart
	}

	lstrcpy(szCurrentFilename, szFilename);
	_ASSERT(hCurrentFile == NULL);
	hCurrentFile = hFile;
	#if defined _USRDLL						// DLL version
		// notify main proc about current document file
		if (pEmuDocumentNotify) pEmuDocumentNotify(szCurrentFilename);
	#endif
	SetWindowTitle(szCurrentFilename);
	UpdateWindowStatus();
	return TRUE;

read_err:
	AbortMessage(_T("This file must be truncated, and cannot be loaded."));
restore:
	if (INVALID_HANDLE_VALUE != hFile)		// close if valid handle
		CloseHandle(hFile);
	RestoreBackup();
	ResetBackup();

	// HP48SX/GX
	if(cCurrentRomType=='S' || cCurrentRomType=='G')
	{
		// use 2nd command line argument if defined
		MapPort2((nArgc < 3) ? szPort2Filename : ppArgv[2]);
	}
	return FALSE;
}

BOOL SaveDocument(VOID)
{
	DWORD           lBytesWritten;
	DWORD           lSizeofChipset;
	LPBYTE          pbySig;
	UINT            nLength;
	WINDOWPLACEMENT wndpl;

	if (hCurrentFile == NULL) return FALSE;

	_ASSERT(hWnd);							// window open
	wndpl.length = sizeof(wndpl);			// update saved window position
	GetWindowPlacement(hWnd, &wndpl);
	Chipset.nPosX = (SWORD) wndpl.rcNormalPosition.left;
	Chipset.nPosY = (SWORD) wndpl.rcNormalPosition.top;

	SetFilePointer(hCurrentFile,0,NULL,FILE_BEGIN);

	// get document signature
	pbySig = (Chipset.type=='6' || Chipset.type=='A')
		   ? pbySignatureA : (Chipset.type=='E' ? pbySignatureB : ((Chipset.type!='X') ? pbySignatureE : pbySignatureV));

	if (!WriteFile(hCurrentFile, pbySig, 16, &lBytesWritten, NULL))
	{
		AbortMessage(_T("Could not write into file !"));
		return FALSE;
	}

	Chipset.wRomCrc = CrcRom();				// save fingerprint of ROM
	Chipset.wPort2Crc = CrcPort2();			// save fingerprint of port2

	nLength = lstrlen(szCurrentKml);
	WriteFile(hCurrentFile, &nLength, sizeof(nLength), &lBytesWritten, NULL);
	#if defined _UNICODE
	{
		LPSTR szTmp = HeapAlloc(hHeap,0,nLength);
		if (szTmp != NULL)
		{
			WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK,
								szCurrentKml, nLength,
								szTmp, nLength, NULL, NULL);
			WriteFile(hCurrentFile, szTmp, nLength, &lBytesWritten, NULL);
			HeapFree(hHeap,0,szTmp);
		}
	}
	#else
	{
		WriteFile(hCurrentFile, szCurrentKml, nLength, &lBytesWritten, NULL);
	}
	#endif
	lSizeofChipset = sizeof(CHIPSET);
	WriteFile(hCurrentFile, &lSizeofChipset, sizeof(lSizeofChipset), &lBytesWritten, NULL);
	WriteFile(hCurrentFile, &Chipset, lSizeofChipset, &lBytesWritten, NULL);
	if (Chipset.Port0Size) WriteFile(hCurrentFile, Chipset.Port0, Chipset.Port0Size*2048, &lBytesWritten, NULL);
	if (Chipset.Port1Size) WriteFile(hCurrentFile, Chipset.Port1, Chipset.Port1Size*2048, &lBytesWritten, NULL);
	if (Chipset.Port2Size) WriteFile(hCurrentFile, Chipset.Port2, Chipset.Port2Size*2048, &lBytesWritten, NULL);
	SaveBreakpointList(hCurrentFile);		// save debugger breakpoint list
	SetEndOfFile(hCurrentFile);				// cut the rest
	return TRUE;
}

BOOL SaveDocumentAs(LPCTSTR szFilename)
{
	HANDLE hFile;

	if (hCurrentFile)						// already file in use
	{
		CloseHandle(hCurrentFile);			// close it, even it's same, so data always will be saved
		hCurrentFile = NULL;
	}
	hFile = CreateFile(szFilename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)		// error, couldn't create a new file
	{
		AbortMessage(_T("This file must be currently used by another instance of Emu48."));
		return FALSE;
	}
	lstrcpy(szCurrentFilename, szFilename);	// save new file name
	hCurrentFile = hFile;					// and the corresponding handle
	#if defined _USRDLL						// DLL version
		// notify main proc about current document file
		if (pEmuDocumentNotify) pEmuDocumentNotify(szCurrentFilename);
	#endif
	SetWindowTitle(szCurrentFilename);		// update window title line
	UpdateWindowStatus();					// and draw it
	return SaveDocument();					// save current content
}



//################
//#
//#    Backup
//#
//################

BOOL SaveBackup(VOID)
{
	WINDOWPLACEMENT wndpl;

	if (pbyRom == NULL) return FALSE;

	// save window position
	_ASSERT(hWnd);							// window open
	wndpl.length = sizeof(wndpl);			// update saved window position
	GetWindowPlacement(hWnd, &wndpl);
	Chipset.nPosX = (SWORD) wndpl.rcNormalPosition.left;
	Chipset.nPosY = (SWORD) wndpl.rcNormalPosition.top;

	lstrcpy(szBackupFilename, szCurrentFilename);
	lstrcpy(szBackupKml, szCurrentKml);
	if (BackupChipset.Port0) HeapFree(hHeap,0,BackupChipset.Port0);
	if (BackupChipset.Port1) HeapFree(hHeap,0,BackupChipset.Port1);
	if (BackupChipset.Port2) HeapFree(hHeap,0,BackupChipset.Port2);
	CopyMemory(&BackupChipset, &Chipset, sizeof(Chipset));
	BackupChipset.Port0 = HeapAlloc(hHeap,0,Chipset.Port0Size*2048);
	CopyMemory(BackupChipset.Port0,Chipset.Port0,Chipset.Port0Size*2048);
	BackupChipset.Port1 = HeapAlloc(hHeap,0,Chipset.Port1Size*2048);
	CopyMemory(BackupChipset.Port1,Chipset.Port1,Chipset.Port1Size*2048);
	BackupChipset.Port2 = NULL;
	if (Chipset.Port2Size)					// internal port2
	{
		BackupChipset.Port2 = HeapAlloc(hHeap,0,Chipset.Port2Size*2048);
		CopyMemory(BackupChipset.Port2,Chipset.Port2,Chipset.Port2Size*2048);
	}
	bBackup = TRUE;
	UpdateWindowStatus();
	return TRUE;
}

BOOL RestoreBackup(VOID)
{
	if (!bBackup) return FALSE;
	ResetDocument();
	// need chipset for contrast setting in InitKML()
	Chipset.contrast = BackupChipset.contrast;
	if (!InitKML(szBackupKml,TRUE))
	{
		InitKML(szCurrentKml,TRUE);
		return FALSE;
	}
	lstrcpy(szCurrentKml, szBackupKml);
	lstrcpy(szCurrentFilename, szBackupFilename);
	if (szCurrentFilename[0])
	{
		hCurrentFile = CreateFile(szCurrentFilename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hCurrentFile == INVALID_HANDLE_VALUE)
		{
			hCurrentFile = NULL;
			szCurrentFilename[0] = 0;
		}
	}
	CopyMemory(&Chipset, &BackupChipset, sizeof(Chipset));
	Chipset.Port0 = HeapAlloc(hHeap,0,Chipset.Port0Size*2048);
	CopyMemory(Chipset.Port0,BackupChipset.Port0,Chipset.Port0Size*2048);
	Chipset.Port1 = HeapAlloc(hHeap,0,Chipset.Port1Size*2048);
	CopyMemory(Chipset.Port1,BackupChipset.Port1,Chipset.Port1Size*2048);
	if (Chipset.Port2Size)					// internal port2
	{
		Chipset.Port2 = HeapAlloc(hHeap,0,Chipset.Port2Size*2048);
		CopyMemory(Chipset.Port2,BackupChipset.Port2,Chipset.Port2Size*2048);
	}
	// map port2
	else
	{
		if(cCurrentRomType=='S' || cCurrentRomType=='G') // HP48SX/GX
		{
			// use 2nd command line argument if defined
			MapPort2((nArgc < 3) ? szPort2Filename : ppArgv[2]);
		}
	}
	SetWindowLocation(hWnd,Chipset.nPosX,Chipset.nPosY);
	UpdateWindowStatus();
	Map(0x00,0xFF);
	return TRUE;
}

BOOL ResetBackup(VOID)
{
	if (!bBackup) return FALSE;
	szBackupFilename[0] = 0;
	szBackupKml[0] = 0;
	if (BackupChipset.Port0) HeapFree(hHeap,0,BackupChipset.Port0);
	if (BackupChipset.Port1) HeapFree(hHeap,0,BackupChipset.Port1);
	if (BackupChipset.Port2) HeapFree(hHeap,0,BackupChipset.Port2);
	ZeroMemory(&BackupChipset,sizeof(BackupChipset));
	bBackup = FALSE;
	UpdateWindowStatus();
	return TRUE;
}



//################
//#
//#    Open File Common Dialog Boxes
//#
//################

static VOID InitializeOFN(LPOPENFILENAME ofn)
{
	ZeroMemory((LPVOID)ofn, sizeof(OPENFILENAME));
	ofn->lStructSize = sizeof(OPENFILENAME);
	ofn->hwndOwner = hWnd;
	ofn->Flags = OFN_EXPLORER|OFN_HIDEREADONLY;
	return;
}

BOOL GetOpenFilename(VOID)
{
	OPENFILENAME ofn;

	InitializeOFN(&ofn);
	ofn.lpstrFilter =
		_T("Emu38 Document (*.E38)\0*.E38\0")
		_T("Emu39 Document (*.E39)\0*.E39\0")
		_T("Emu48 Document (*.E48)\0*.E48\0")
		_T("Emu49 Document (*.E49)\0*.E49\0")
		_T("Win48 Document (*.W48)\0*.W48\0")
		_T("\0\0");
	ofn.lpstrDefExt = _T("E48");			// HP48SX/GX
	ofn.nFilterIndex = 3;
	if (cCurrentRomType=='6' || cCurrentRomType=='A') // HP38G
	{
		ofn.lpstrDefExt = _T("E38");
		ofn.nFilterIndex = 1;
	}
	if (cCurrentRomType=='E')				// HP39/40G
	{
		ofn.lpstrDefExt = _T("E39");
		ofn.nFilterIndex = 2;
	}
	if (cCurrentRomType=='X')				// HP49G
	{
		ofn.lpstrDefExt = _T("E49");
		ofn.nFilterIndex = 4;
	}
	ofn.lpstrFile = HeapAlloc(hHeap,0,sizeof(szBufferFilename));
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = ARRAYSIZEOF(szBufferFilename);
	ofn.Flags |= OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&ofn) == FALSE)
	{
		HeapFree(hHeap,0,ofn.lpstrFile);
		return FALSE;
	}
	lstrcpy(szBufferFilename, ofn.lpstrFile);
	HeapFree(hHeap,0,ofn.lpstrFile);
	return TRUE;
}

BOOL GetSaveAsFilename(VOID)
{
	OPENFILENAME ofn;

	InitializeOFN(&ofn);
	ofn.lpstrFilter =
		_T("Emu38 Document (*.E38)\0*.E38\0")
		_T("Emu39 Document (*.E39)\0*.E39\0")
		_T("Emu48 Document (*.E48)\0*.E48\0")
		_T("Emu49 Document (*.E49)\0*.E49\0")
		_T("Win48 Document (*.W48)\0*.W48\0")
		_T("\0\0");
	ofn.lpstrDefExt = _T("E48");			// HP48SX/GX
	ofn.nFilterIndex = 3;
	if (cCurrentRomType=='6' || cCurrentRomType=='A') // HP38G
	{
		ofn.lpstrDefExt = _T("E38");
		ofn.nFilterIndex = 1;
	}
	if (cCurrentRomType=='E')				// HP39/40G
	{
		ofn.lpstrDefExt = _T("E39");
		ofn.nFilterIndex = 2;
	}
	if (cCurrentRomType=='X')				// HP49G
	{
		ofn.lpstrDefExt = _T("E49");
		ofn.nFilterIndex = 4;
	}
	ofn.lpstrFile = HeapAlloc(hHeap,0,sizeof(szBufferFilename));
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = ARRAYSIZEOF(szBufferFilename);
	ofn.Flags |= OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&ofn) == FALSE)
	{
		HeapFree(hHeap,0,ofn.lpstrFile);
		return FALSE;
	}
	lstrcpy(szBufferFilename, ofn.lpstrFile);
	HeapFree(hHeap,0,ofn.lpstrFile);
	return TRUE;
}

BOOL GetLoadObjectFilename(VOID)
{
	OPENFILENAME ofn;

	InitializeOFN(&ofn);
	ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0") _T("\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = HeapAlloc(hHeap,0,sizeof(szBufferFilename));
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = ARRAYSIZEOF(szBufferFilename);
	ofn.Flags |= OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&ofn) == FALSE)
	{
		HeapFree(hHeap,0,ofn.lpstrFile);
		return FALSE;
	}
	lstrcpy(szBufferFilename, ofn.lpstrFile);
	HeapFree(hHeap,0,ofn.lpstrFile);
	return TRUE;
}

BOOL GetSaveObjectFilename(VOID)
{
	OPENFILENAME ofn;

	InitializeOFN(&ofn);
	ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0") _T("\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = HeapAlloc(hHeap,0,sizeof(szBufferFilename));
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = ARRAYSIZEOF(szBufferFilename);
	ofn.Flags |= OFN_CREATEPROMPT|OFN_OVERWRITEPROMPT;
	if (GetSaveFileName(&ofn) == FALSE)
	{
		HeapFree(hHeap,0,ofn.lpstrFile);
		return FALSE;
	}
	lstrcpy(szBufferFilename, ofn.lpstrFile);
	HeapFree(hHeap,0,ofn.lpstrFile);
	return TRUE;
}



//################
//#
//#    Load and Save HP48 Objects
//#
//################

BOOL LoadObject(LPCTSTR szFilename)			// separated stack writing part
{
	HANDLE hFile;
	DWORD  dwFileSizeLow;
	DWORD  dwFileSizeHigh;
	LPBYTE lpBuf;
	WORD   wError;

	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;
	dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
	if (dwFileSizeHigh != 0)
	{ // file is too large.
		CloseHandle(hFile);
		return FALSE;
	}
	lpBuf = HeapAlloc(hHeap,0,dwFileSizeLow*2);
	if (lpBuf == NULL)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	ReadFile(hFile, lpBuf+dwFileSizeLow, dwFileSizeLow, &dwFileSizeHigh, NULL);
	CloseHandle(hFile);

	wError = WriteStack(lpBuf,dwFileSizeLow);

	if (wError == S_ERR_BINARY)
		AbortMessage(_T("The calculator does not have enough\nfree memory to load this binary file."));

	if (wError == S_ERR_ASCII)
		AbortMessage(_T("The calculator does not have enough\nfree memory to load this text file."));

	HeapFree(hHeap,0,lpBuf);
	return (wError == S_ERR_NO);
}

BOOL SaveObject(LPCTSTR szFilename)			// separated stack reading part
{
	HANDLE	hFile;
	LPBYTE  pbyHeader;
	DWORD	lBytesWritten;
	DWORD   dwAddress;
	DWORD   dwLength;

	dwAddress = RPL_Pick(1);
	if (dwAddress == 0)
	{
		AbortMessage(_T("Too Few Arguments."));
		return FALSE;
	}
	dwLength = (RPL_SkipOb(dwAddress) - dwAddress + 1) / 2;

	hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AbortMessage(_T("Cannot open file."));
		return FALSE;
	}

	pbyHeader = (Chipset.type != 'X') ? BINARYHEADER48 : BINARYHEADER49;
	WriteFile(hFile, pbyHeader, 8, &lBytesWritten, NULL);

	while (dwLength--)
	{
		BYTE byByte = Read2(dwAddress);
		WriteFile(hFile, &byByte, 1, &lBytesWritten, NULL);
		dwAddress += 2;
	}
	CloseHandle(hFile);
	return TRUE;
}



//################
//#
//#    Load Bitmap
//#
//################

static __inline UINT DibNumColors(LPBITMAPINFOHEADER lpbi)
{
	if (lpbi->biClrUsed != 0) return (UINT)lpbi->biClrUsed;

	/* a 24 bitcount DIB has no color table */
	return (lpbi->biBitCount <= 8) ? (1 << lpbi->biBitCount) : 0;
}

static HPALETTE CreateBIPalette(LPBITMAPINFOHEADER lpbi)
{
	LOGPALETTE* pPal;
	HPALETTE    hpal = NULL;
	UINT        nNumColors;
	BYTE        red;
	BYTE        green;
	BYTE        blue;
	UINT        i;
	RGBQUAD*    pRgb;

	if (!lpbi)
		return NULL;

	if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
		return NULL;

	// Get a pointer to the color table and the number of colors in it
	pRgb = (RGBQUAD *)((LPBYTE)lpbi + (WORD)lpbi->biSize);
	nNumColors = DibNumColors(lpbi);

	if (nNumColors)
	{
		// Allocate for the logical palette structure
		pPal = HeapAlloc(hHeap,0,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
		if (!pPal) return NULL;

		pPal->palNumEntries = nNumColors;
		pPal->palVersion    = 0x300;

		// Fill in the palette entries from the DIB color table and
		// create a logical color palette.
		for (i = 0; i < nNumColors; i++)
		{
			pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
			pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
			pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
			pPal->palPalEntry[i].peFlags = 0;
		}
		hpal = CreatePalette(pPal);
		HeapFree(hHeap,0,pPal);
	}
	else
	{
		if (lpbi->biBitCount == 24)
		{
			// A 24 bitcount DIB has no color table entries so, set the
			// number of to the maximum value (256).
			nNumColors = 256;
			pPal = HeapAlloc(hHeap,0,sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
			if (!pPal) return NULL;

			pPal->palNumEntries = nNumColors;
			pPal->palVersion    = 0x300;

			red = green = blue = 0;

			// Generate 256 (= 8*8*4) RGB combinations to fill the palette
			// entries.
			for (i = 0; i < pPal->palNumEntries; i++)
			{
				pPal->palPalEntry[i].peRed   = red;
				pPal->palPalEntry[i].peGreen = green;
				pPal->palPalEntry[i].peBlue  = blue;
				pPal->palPalEntry[i].peFlags = 0;

				if (!(red += 32))
					if (!(green += 32))
						blue += 64;
			}
			hpal = CreatePalette(pPal);
			HeapFree(hHeap,0,pPal);
		}
	}
	return hpal;
}

HBITMAP LoadBitmapFile(LPCTSTR szFilename)
{
	HANDLE  hFile;
	HANDLE  hMap;
	LPBYTE  pbyFile;
	HBITMAP hBitmap;
	LPBITMAPFILEHEADER pBmfh;
	LPBITMAPINFO pBmi;

	if (pbyRom == NULL) return NULL;
	SetCurrentDirectory(szEmuDirectory);
	hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	SetCurrentDirectory(szCurrentDirectory);
	// opened with GENERIC_READ -> PAGE_READONLY
	hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMap == NULL)
	{
		CloseHandle(hFile);
		return NULL;
	}
	// opened with GENERIC_READ -> PAGE_READONLY -> FILE_MAP_READ
	pbyFile = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (pbyFile == NULL)
	{
		CloseHandle(hMap);
		CloseHandle(hFile);
		return NULL;
	}

	hBitmap = NULL;
	pBmfh = (LPBITMAPFILEHEADER)pbyFile;
	if (pBmfh->bfType != 0x4D42) goto quit; // "BM"
	pBmi = (LPBITMAPINFO)(pbyFile+sizeof(BITMAPFILEHEADER));

	_ASSERT(hPalette == NULL);				// resource free
	hPalette = CreateBIPalette(&pBmi->bmiHeader);
	// save old palette
	hOldPalette = SelectPalette(hWindowDC, hPalette, FALSE);
	RealizePalette(hWindowDC);

	hBitmap = CreateDIBitmap(
		hWindowDC,
		&pBmi->bmiHeader,
		CBM_INIT,
		pbyFile + pBmfh->bfOffBits,
		pBmi, DIB_RGB_COLORS);
	_ASSERT(hBitmap != NULL);

quit:
	UnmapViewOfFile(pbyFile);
	CloseHandle(hMap);
	CloseHandle(hFile);
	return hBitmap;
}
