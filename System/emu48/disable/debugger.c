/*
 *   debugger.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1999 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "resource.h"
#include "emu48.h"
#include "opcodes.h"
#include "ops.h"
//#include "color.h"
#include "debugger.h"

#define MAXCODELINES     15					// number of lines in code window
#define MAXMEMLINES       6					// number of lines in memory window
#define MAXMEMITEMS      16					// number of address items in a memory window line
#define MAXBREAKPOINTS  256					// max. number of breakpoints

#define	REG_START  IDC_REG_A				// first register in register update table
#define	REG_STOP   IDC_MISC_BS				// last register in register update table
#define REG_SIZE   (REG_STOP-REG_START+1)	// size of register update table

// assert for register update
#define _ASSERTREG(r) _ASSERT(r >= REG_START && r <= REG_STOP)

#define INSTRSIZE  256						// size of last instruction buffer

#define WM_UPDATE (WM_USER+0x1000)			// update debugger dialog box

#define	MEMWNDMAX (sizeof(nCol) / sizeof(nCol[0]))

#define RT_TOOLBAR MAKEINTRESOURCE(241)		// MFC toolbar resource type

typedef struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	WORD aItems[1];
} CToolBarData;

typedef struct								// type of breakpoint table
{
	BOOL	bEnable;						// breakpoint enabled
	UINT	nType;							// breakpoint type
	DWORD	dwAddr;							// breakpoint address
} BP_T;

typedef struct								// type of model memory mapping
{
	CONST BYTE   byType;					// calculator type
	CONST LPBYTE *ppbyNCE1;					// NCE1 data
	CONST DWORD  *pdwNCE1Size;				// NCE1 size
	CONST LPBYTE *ppbyNCE2;					// NCE2 data
	CONST DWORD  *pdwNCE2Size;				// NCE2 size
	CONST LPBYTE *ppbyCE1;					// CE1 data
	CONST DWORD  *pdwCE1Size;				// CE1 size
	CONST LPBYTE *ppbyCE2;					// CE2 data
	CONST DWORD  *pdwCE2Size;				// CE2 size
	CONST LPBYTE *ppbyNCE3;					// NCE3 data
	CONST DWORD  *pdwNCE3Size;				// NCE3 size
} MODEL_MAP_T;

static CONST int nCol[] =
{
	IDC_DEBUG_MEM_COL0, IDC_DEBUG_MEM_COL1, IDC_DEBUG_MEM_COL2, IDC_DEBUG_MEM_COL3,
	IDC_DEBUG_MEM_COL4, IDC_DEBUG_MEM_COL5, IDC_DEBUG_MEM_COL6, IDC_DEBUG_MEM_COL7
};

static CONST TCHAR cHex[] = { _T('0'),_T('1'),_T('2'),_T('3'),
							  _T('4'),_T('5'),_T('6'),_T('7'),
							  _T('8'),_T('9'),_T('A'),_T('B'),
							  _T('C'),_T('D'),_T('E'),_T('F') };

static CONST LPBYTE pbyNoMEM = NULL;		// no memory module

static CONST MODEL_MAP_T MemMap[] =
{
	{
		0,									// default
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL						// nc.
	},
	{
		'6',								// HP38G (64K)
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0,	&Chipset.Port0Size,	// RAM
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL						// nc.
	},
	{
		'A',								// HP38G
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0,	&Chipset.Port0Size,	// RAM
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL,					// nc.
		&pbyNoMEM, NULL						// nc.
	},
	{
		'E',								// HP39/40G
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0, &Chipset.Port0Size,	// RAM part 1
		&pbyNoMEM, NULL,					// BS
		&pbyNoMEM, NULL,					// nc.
		&Chipset.Port2,	&Chipset.Port2Size	// RAM part 2
	},
	{
		'G',								// HP48GX
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0, &Chipset.Port0Size,	// RAM
		&pbyNoMEM, NULL,					// BS
		&Chipset.Port1,	&Chipset.Port1Size,	// Card slot 1
		&pbyPort2, &dwPort2Size				// Card slot 2
	},
	{
		'S',								// HP48SX
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0, &Chipset.Port0Size,	// RAM
		&Chipset.Port1,	&Chipset.Port1Size,	// Card slot 1
		&pbyPort2, &dwPort2Size,			// Card slot 2
		&pbyNoMEM, NULL						// nc.
	},
	{
		'X',								// HP49G
		&pbyRom, &dwRomSize,				// Flash
		&Chipset.Port0, &Chipset.Port0Size,	// RAM
		&pbyNoMEM, NULL,					// BS
		&Chipset.Port1, &Chipset.Port1Size,	// Port 1 part 1
		&Chipset.Port2, &Chipset.Port2Size	// Port 1 part 2
	},
	{ // CdB for HP: add Q type
		'Q',								// HP49G+
		&pbyRom, &dwRomSize,				// Flash
		&Chipset.Port0, &Chipset.Port0Size,	// RAM
		&pbyNoMEM, NULL,					// BS
		&Chipset.Port1, &Chipset.Port1Size,	// Port 1 part 1
		&Chipset.Port2, &Chipset.Port2Size	// Port 1 part 2
	},
	{ // CdB for HP: add 2 type
		'2',								// HP48Gii
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0, &Chipset.Port0Size,	// RAM
		&pbyNoMEM, NULL,					// BS
		&pbyNoMEM, NULL,					// Port 1 part 1
		&pbyNoMEM, NULL,					// Port 1 part 2
	},
	{ // CdB for HP: add P type
		'P',								// HP39G+
		&pbyRom, &dwRomSize,				// ROM
		&Chipset.Port0, &Chipset.Port0Size,	// RAM part 1
		&pbyNoMEM, NULL,					// BS
		&pbyNoMEM, NULL,					// nc.
		&Chipset.Port2,	&Chipset.Port2Size	// RAM part 2
	}
};

static INT    nDbgPosX = 0;					// position of debugger window
static INT    nDbgPosY = 0;

static WORD   wBreakpointCount = 0;			// number of breakpoints
static BP_T   sBreakpoint[MAXBREAKPOINTS];	// breakpoint table

static INT    nRplBreak;					// RPL breakpoint

static DWORD  dwAdrLine[MAXCODELINES];		// addresses of disassember lines in code window
static DWORD  dwAdrMem = 0;					// start address of memory window

static UINT   uIDFol = ID_DEBUG_MEM_FNONE;	// follow mode
static DWORD  dwAdrMemFol = 0;				// follow address memory window

static UINT   uIDMap = ID_DEBUG_MEM_MAP;	// current memory view mode
static LPBYTE lbyMapData;					// data
static DWORD  dwMapDataSize;				// data size

static LONG   lCharWidth;					// width of a character (is a fix font)

static HMENU  hMenuCode,hMenuMem,hMenuStack;// handle of context menues
static HWND   hWndToolbar;					// toolbar handle

static MODEL_MAP_T CONST *pMapping;			// model specific memory mapping

static DWORD   dwDbgRefCycles;				// cpu cycles counter from last opcode

static CHIPSET OldChipset;					// old chipset content
static BOOL    bRegUpdate[REG_SIZE];		// register update table

static HBITMAP hBmpCheckBox;				// checked and unchecked bitmap

// function prototypes
static BOOL    OnMemFind(HWND hDlg);
static BOOL    OnProfile(HWND hDlg);
static INT_PTR OnNewValue(LPTSTR lpszValue);
static VOID    OnEnterAddress(HWND hDlg, DWORD *dwValue);
static BOOL    OnEditBreakpoint(HWND hDlg);
static BOOL    OnInfoIntr(HWND hDlg);
static BOOL    OnInfoWoRegister(HWND hDlg);
static VOID    UpdateProfileWnd(HWND hDlg);

//################
//#
//#    Low level subroutines
//#
//################

//
// disable menu keys
//
static VOID DisableMenuKeys(HWND hDlg)
{
	HMENU hMenu = GetMenu(hDlg);

	EnableMenuItem(hMenu,ID_DEBUG_RUN,MF_GRAYED);
	EnableMenuItem(hMenu,ID_DEBUG_RUNCURSOR,MF_GRAYED);
	EnableMenuItem(hMenu,ID_DEBUG_STEP,MF_GRAYED);
	EnableMenuItem(hMenu,ID_DEBUG_STEPOVER,MF_GRAYED);
	EnableMenuItem(hMenu,ID_DEBUG_STEPOUT,MF_GRAYED);
	EnableMenuItem(hMenu,ID_INFO_LASTINSTRUCTIONS,MF_GRAYED);
	EnableMenuItem(hMenu,ID_INFO_WRITEONLYREG,MF_GRAYED);

	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_RUN,MAKELONG((FALSE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_BREAK,MAKELONG((TRUE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_RUNCURSOR,MAKELONG((FALSE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_STEP,MAKELONG((FALSE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_STEPOVER,MAKELONG((FALSE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_STEPOUT,MAKELONG((FALSE),0));
	return;
}

//
// set mapping menu
//
static VOID SetMappingMenu(HWND hDlg,UINT uID)
{
	LPTSTR szCaption;

	CheckMenuItem(hMenuMem,uIDMap,MF_UNCHECKED);

	switch (uID)
	{
	case ID_DEBUG_MEM_MAP:
		szCaption = _T("Memory");
		lbyMapData = NULL;					// data
		dwMapDataSize = 512;				// data size
		break;
	case ID_DEBUG_MEM_NCE1:
		szCaption = _T("Memory (NCE1)");
		lbyMapData = *pMapping->ppbyNCE1;
		dwMapDataSize = *pMapping->pdwNCE1Size / 2048; // ROM size is always in nibbles
		break;
	case ID_DEBUG_MEM_NCE2:
		szCaption = _T("Memory (NCE2)");
		lbyMapData = *pMapping->ppbyNCE2;
		dwMapDataSize = *pMapping->pdwNCE2Size;
		break;
	case ID_DEBUG_MEM_CE1:
		szCaption = _T("Memory (CE1)");
		lbyMapData = *pMapping->ppbyCE1;
		dwMapDataSize = *pMapping->pdwCE1Size;
		break;
	case ID_DEBUG_MEM_CE2:
		szCaption = _T("Memory (CE2)");
		lbyMapData = *pMapping->ppbyCE2;
		dwMapDataSize = *pMapping->pdwCE2Size;
		break;
	case ID_DEBUG_MEM_NCE3:
		szCaption = _T("Memory (NCE3)");
		lbyMapData = *pMapping->ppbyNCE3;
		dwMapDataSize = *pMapping->pdwNCE3Size;
		break;
	default: _ASSERT(0);
	}

	dwMapDataSize *= 2048;					// size in nibble

	if (uIDMap != uID) dwAdrMem = 0;		// view from address 0

	uIDMap = uID;
	CheckMenuItem(hMenuMem,uIDMap,MF_CHECKED);

	SetDlgItemText(hDlg,IDC_STATIC_MEMORY,szCaption);
	return;
};

//
// set/reset breakpoint
//
static __inline VOID ToggleBreakpoint(DWORD dwAddr)
{
	INT i;

	for (i = 0; i < wBreakpointCount; ++i)	// scan all breakpoints
	{
		// code breakpoint found
		if (sBreakpoint[i].nType == BP_EXEC && sBreakpoint[i].dwAddr == dwAddr)
		{
			if (!sBreakpoint[i].bEnable)	// breakpoint disabled
			{
				sBreakpoint[i].bEnable = TRUE;
				return;
			}

			// purge breakpoint
			for (++i; i < wBreakpointCount; ++i)
				sBreakpoint[i-1] = sBreakpoint[i];
			--wBreakpointCount;
			return;
		}
	}

	// breakpoint not found
	if (wBreakpointCount >= MAXBREAKPOINTS)	// breakpoint buffer full
	{
		AbortMessage(_T("Reached maximum number of breakpoints !"));
		return;
	}

	sBreakpoint[wBreakpointCount].bEnable = TRUE;
	sBreakpoint[wBreakpointCount].nType   = BP_EXEC;
	sBreakpoint[wBreakpointCount].dwAddr  = dwAddr;
	++wBreakpointCount;
	return;
}

//
// init memory mapping table and mapping menu entry
//
static __inline VOID InitMemMap(HWND hDlg)
{
	BOOL bActive;
	INT  i;

	pMapping = MemMap;						// init default mapping

	// scan for all table entries
	for (i = 0; i < ARRAYSIZEOF(MemMap); ++i)
	{
		if (MemMap[i].byType == cCurrentRomType)
		{
			pMapping = &MemMap[i];			// found entry
			break;
		}
	}

	_ASSERT(hMenuMem);

	// enable menu mappings
	EnableMenuItem(hMenuMem,ID_DEBUG_MEM_NCE1,(*pMapping->ppbyNCE1) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenuMem,ID_DEBUG_MEM_NCE2,(*pMapping->ppbyNCE2) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenuMem,ID_DEBUG_MEM_CE1, (*pMapping->ppbyCE1)  ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenuMem,ID_DEBUG_MEM_CE2, (*pMapping->ppbyCE2)  ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenuMem,ID_DEBUG_MEM_NCE3,(*pMapping->ppbyNCE3) ? MF_ENABLED : MF_GRAYED);

	bActive =  (ID_DEBUG_MEM_NCE1 == uIDMap && *pMapping->ppbyNCE1 != NULL)
		    || (ID_DEBUG_MEM_NCE2 == uIDMap && *pMapping->ppbyNCE2 != NULL)
		    || (ID_DEBUG_MEM_CE1  == uIDMap && *pMapping->ppbyCE1  != NULL)
		    || (ID_DEBUG_MEM_CE2  == uIDMap && *pMapping->ppbyCE2  != NULL)
		    || (ID_DEBUG_MEM_NCE3 == uIDMap && *pMapping->ppbyNCE3 != NULL);

	SetMappingMenu(hDlg,bActive ? uIDMap : ID_DEBUG_MEM_MAP);
	return;
}

//
// init bank switcher area
//
static __inline VOID InitBsArea(HWND hDlg)
{
	// not 38 / 48S // CdB for HP: add apples type
	if (cCurrentRomType!='A' && cCurrentRomType!='S')
	{
		EnableWindow(GetDlgItem(hDlg,IDC_MISC_BS_TXT),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_MISC_BS),TRUE);
	}
	return;
}

//
// convert nibble register to string
//
static LPTSTR RegToStr(BYTE *pReg, WORD wNib)
{
	static TCHAR szBuffer[32];

	WORD i;

	for (i = 0;i < wNib;++i)
		szBuffer[i] = cHex[pReg[wNib-i-1]];
	szBuffer[i] = 0;

	return szBuffer;
}

//
// convert string to nibble register
//
static VOID StrToReg(BYTE *pReg, WORD wNib, LPTSTR lpszValue)
{
	int i,nValuelen;

	nValuelen = lstrlen(lpszValue);
	for (i = wNib - 1;i >= 0;--i)
	{
		if (i >= nValuelen)					// no character in string
		{
			pReg[i] = 0;					// fill with zero
		}
		else
		{
			// convert to number
			pReg[i] = _totupper(*lpszValue) - _T('0');
			if (pReg[i] > 9) pReg[i] -= _T('A') - _T('9') - 1;
			++lpszValue;
		}
	}
	return;
}

//
// write code window
//
static VOID ViewCodeWnd(HWND hWnd, DWORD dwAddress)
{
	INT   i,j;
	TCHAR szAddress[64];

	_ASSERT(disassembler_map == MEM_MAP);	// disassemble in mapped mode
	SendMessage(hWnd,WM_SETREDRAW,FALSE,0);
	SendMessage(hWnd,LB_RESETCONTENT,0,0);
	for (i = 0; i < MAXCODELINES; ++i)
	{
		dwAdrLine[i] = dwAddress;
		j = wsprintf(szAddress,
					 (dwAddress == Chipset.pc) ? _T("%05lX-%c ") : _T("%05lX   "),
					 dwAddress,nRplBreak ? _T('R') : _T('>'));
		dwAddress = disassemble(dwAddress,&szAddress[j],VIEW_SHORT);
		SendMessage(hWnd,LB_ADDSTRING,0,(LPARAM) szAddress);
	}
	SendMessage(hWnd,WM_SETREDRAW,TRUE,0);
	return;
}

//
// write memory window
//
static VOID ViewMemWnd(HWND hDlg, DWORD dwAddress)
{
	#define TEXTOFF 32

	INT   i,j,k;
	TCHAR szBuffer[16],szItem[4];
	BYTE  cChar;

	szItem[2] = 0;							// end of string
	dwAdrMem = dwAddress;					// save start address of memory window

	// purge all list boxes
	SendDlgItemMessage(hDlg,IDC_DEBUG_MEM_ADDR,LB_RESETCONTENT,0,0);
	SendDlgItemMessage(hDlg,IDC_DEBUG_MEM_TEXT,LB_RESETCONTENT,0,0);
	for (j = 0; j < MEMWNDMAX; ++j)
		SendDlgItemMessage(hDlg,nCol[j],LB_RESETCONTENT,0,0);

	for (i = 0; i < MAXMEMLINES; ++i)
	{
		BYTE byLineData[MAXMEMITEMS];

		if (ID_DEBUG_MEM_MAP == uIDMap)		// mapped memory content
		{
			// fetch mapping data line
			Npeek(byLineData, dwAddress, MAXMEMITEMS);

			wsprintf(szBuffer,_T("%05lX"),dwAddress);
		}
		else								// module memory content
		{
			INT i;

			_ASSERT(lbyMapData);			// valid module

			// fetch modul data line
			for (i = 0; i < MAXMEMITEMS; ++i)
			{
				byLineData[i] = lbyMapData[(dwAddress + i) & (dwMapDataSize - 1)];
			}

			wsprintf(szBuffer,_T("%06lX"),dwAddress);
		}
		SendDlgItemMessage(hDlg,IDC_DEBUG_MEM_ADDR,LB_ADDSTRING,0,(LPARAM) szBuffer);

		for (k = 0, j = 0; j < MAXMEMITEMS; ++j)
		{
			// read from fetched data line
			szItem[j&0x1] = cHex[byLineData[j]];
			// characters are saved in LBS, MSB order
			cChar = (cChar >> 4) | (byLineData[j] << 4);

			if ((j&0x1) != 0)
			{
				// byte field
				_ASSERT(j/2 < MEMWNDMAX);
				SendDlgItemMessage(hDlg,nCol[j/2],LB_ADDSTRING,0,(LPARAM) szItem);

				// text field
				szBuffer[j/2] = (isprint(cChar) != 0) ? cChar : _T('.');
			}
		}
		szBuffer[j/2] = 0;					// end of text string
		SendDlgItemMessage(hDlg,IDC_DEBUG_MEM_TEXT,LB_ADDSTRING,0,(LPARAM) szBuffer);

		dwAddress = (dwAddress + MAXMEMITEMS) & (dwMapDataSize - 1);
	}
	return;
	#undef TEXTOFF
}


//################
//#
//#    High level Window draw routines
//#
//################

//
// update code window with scrolling
//
static VOID UpdateCodeWnd(HWND hDlg)
{
	DWORD dwAddress;
	INT   i,j;

	HWND hWnd = GetDlgItem(hDlg,IDC_DEBUG_CODE);

	j = (INT) SendMessage(hWnd,LB_GETCOUNT,0,0); // no. of items in table

	// seach for actual address in code area
	for (i = 0; i < j; ++i)
	{
		if (dwAdrLine[i] == Chipset.pc)		// found new pc address line
			break;
	}

	// redraw code window
	dwAddress = dwAdrLine[0];				// redraw list box with modified pc
	if (i == j)								// address not found
	{
		dwAddress = Chipset.pc;				// begin with actual pc
		i = 0;								// set cursor on top
	}
	if (i > 10)								// cursor near bottom line
	{
		dwAddress = dwAdrLine[i-10];		// move that pc is in line 11
		i = 10;								// set cursor to actual pc
	}
	ViewCodeWnd(hWnd,dwAddress);			// init code area
	SendMessage(hWnd,LB_SETCURSEL,i,0);		// set
	return;
}

//
// update register window
//
static VOID UpdateRegisterWnd(HWND hDlg)
{
	TCHAR szBuffer[64];

	_ASSERTREG(IDC_REG_A);
	bRegUpdate[IDC_REG_A-REG_START] = memcmp(Chipset.A, OldChipset.A, sizeof(Chipset.A)) != 0;
	wsprintf(szBuffer,_T("A= %s"),RegToStr(Chipset.A,16));
	SetDlgItemText(hDlg,IDC_REG_A,szBuffer);
	_ASSERTREG(IDC_REG_B);
	bRegUpdate[IDC_REG_B-REG_START] = memcmp(Chipset.B, OldChipset.B, sizeof(Chipset.B)) != 0;
	wsprintf(szBuffer,_T("B= %s"),RegToStr(Chipset.B,16));
	SetDlgItemText(hDlg,IDC_REG_B,szBuffer);
	_ASSERTREG(IDC_REG_C);
	bRegUpdate[IDC_REG_C-REG_START] = memcmp(Chipset.C, OldChipset.C, sizeof(Chipset.C)) != 0;
	wsprintf(szBuffer,_T("C= %s"),RegToStr(Chipset.C,16));
	SetDlgItemText(hDlg,IDC_REG_C,szBuffer);
	_ASSERTREG(IDC_REG_D);
	bRegUpdate[IDC_REG_D-REG_START] = memcmp(Chipset.D, OldChipset.D, sizeof(Chipset.D)) != 0;
	wsprintf(szBuffer,_T("D= %s"),RegToStr(Chipset.D,16));
	SetDlgItemText(hDlg,IDC_REG_D,szBuffer);
	_ASSERTREG(IDC_REG_R0);
	bRegUpdate[IDC_REG_R0-REG_START] = memcmp(Chipset.R0, OldChipset.R0, sizeof(Chipset.R0)) != 0;
	wsprintf(szBuffer,_T("R0=%s"),RegToStr(Chipset.R0,16));
	SetDlgItemText(hDlg,IDC_REG_R0,szBuffer);
	_ASSERTREG(IDC_REG_R1);
	bRegUpdate[IDC_REG_R1-REG_START] = memcmp(Chipset.R1, OldChipset.R1, sizeof(Chipset.R1)) != 0;
	wsprintf(szBuffer,_T("R1=%s"),RegToStr(Chipset.R1,16));
	SetDlgItemText(hDlg,IDC_REG_R1,szBuffer);
	_ASSERTREG(IDC_REG_R2);
	bRegUpdate[IDC_REG_R2-REG_START] = memcmp(Chipset.R2, OldChipset.R2, sizeof(Chipset.R2)) != 0;
	wsprintf(szBuffer,_T("R2=%s"),RegToStr(Chipset.R2,16));
	SetDlgItemText(hDlg,IDC_REG_R2,szBuffer);
	_ASSERTREG(IDC_REG_R3);
	bRegUpdate[IDC_REG_R3-REG_START] = memcmp(Chipset.R3, OldChipset.R3, sizeof(Chipset.R3)) != 0;
	wsprintf(szBuffer,_T("R3=%s"),RegToStr(Chipset.R3,16));
	SetDlgItemText(hDlg,IDC_REG_R3,szBuffer);
	_ASSERTREG(IDC_REG_R4);
	bRegUpdate[IDC_REG_R4-REG_START] = memcmp(Chipset.R4, OldChipset.R4, sizeof(Chipset.R4)) != 0;
	wsprintf(szBuffer,_T("R4=%s"),RegToStr(Chipset.R4,16));
	SetDlgItemText(hDlg,IDC_REG_R4,szBuffer);
	_ASSERTREG(IDC_REG_D0);
	bRegUpdate[IDC_REG_D0-REG_START] = Chipset.d0 != OldChipset.d0;
	wsprintf(szBuffer,_T("D0=%05X"),Chipset.d0);
	SetDlgItemText(hDlg,IDC_REG_D0,szBuffer);
	_ASSERTREG(IDC_REG_D1);
	bRegUpdate[IDC_REG_D1-REG_START] = Chipset.d1 != OldChipset.d1;
	wsprintf(szBuffer,_T("D1=%05X"),Chipset.d1);
	SetDlgItemText(hDlg,IDC_REG_D1,szBuffer);
	_ASSERTREG(IDC_REG_P);
	bRegUpdate[IDC_REG_P-REG_START] = Chipset.P != OldChipset.P;
	wsprintf(szBuffer,_T("P=%X"),Chipset.P);
	SetDlgItemText(hDlg,IDC_REG_P,szBuffer);
	_ASSERTREG(IDC_REG_PC);
	bRegUpdate[IDC_REG_PC-REG_START] = Chipset.pc != OldChipset.pc;
	wsprintf(szBuffer,_T("PC=%05X"),Chipset.pc);
	SetDlgItemText(hDlg,IDC_REG_PC,szBuffer);
	_ASSERTREG(IDC_REG_OUT);
	bRegUpdate[IDC_REG_OUT-REG_START] = Chipset.out != OldChipset.out;
	wsprintf(szBuffer,_T("OUT=%03X"),Chipset.out);
	SetDlgItemText(hDlg,IDC_REG_OUT,szBuffer);
	_ASSERTREG(IDC_REG_IN);
	bRegUpdate[IDC_REG_IN-REG_START] = Chipset.in != OldChipset.in;
	wsprintf(szBuffer,_T("IN=%04X"),Chipset.in);
	SetDlgItemText(hDlg,IDC_REG_IN,szBuffer);
	_ASSERTREG(IDC_REG_ST);
	bRegUpdate[IDC_REG_ST-REG_START] = memcmp(Chipset.ST, OldChipset.ST, sizeof(Chipset.ST)) != 0;
	wsprintf(szBuffer,_T("ST=%s"),RegToStr(Chipset.ST,4));
	SetDlgItemText(hDlg,IDC_REG_ST,szBuffer);
	_ASSERTREG(IDC_REG_CY);
	bRegUpdate[IDC_REG_CY-REG_START] = Chipset.carry != OldChipset.carry;
	wsprintf(szBuffer,_T("CY=%d"),Chipset.carry);
	SetDlgItemText(hDlg,IDC_REG_CY,szBuffer);
	_ASSERTREG(IDC_REG_MODE);
	bRegUpdate[IDC_REG_MODE-REG_START] = Chipset.mode_dec != OldChipset.mode_dec;
	wsprintf(szBuffer,_T("Mode=%c"),Chipset.mode_dec ? _T('D') : _T('H'));
	SetDlgItemText(hDlg,IDC_REG_MODE,szBuffer);
	_ASSERTREG(IDC_REG_MP);
	bRegUpdate[IDC_REG_MP-REG_START] = ((Chipset.HST ^ OldChipset.HST) & MP) != 0;
	wsprintf(szBuffer,_T("MP=%d"),(Chipset.HST & MP) != 0);
	SetDlgItemText(hDlg,IDC_REG_MP,szBuffer);
	_ASSERTREG(IDC_REG_SR);
	bRegUpdate[IDC_REG_SR-REG_START] = ((Chipset.HST ^ OldChipset.HST) & SR) != 0;
	wsprintf(szBuffer,_T("SR=%d"),(Chipset.HST & SR) != 0);
	SetDlgItemText(hDlg,IDC_REG_SR,szBuffer);
	_ASSERTREG(IDC_REG_SB);
	bRegUpdate[IDC_REG_SB-REG_START] = ((Chipset.HST ^ OldChipset.HST) & SB) != 0;
	wsprintf(szBuffer,_T("SB=%d"),(Chipset.HST & SB) != 0);
	SetDlgItemText(hDlg,IDC_REG_SB,szBuffer);
	_ASSERTREG(IDC_REG_XM);
	bRegUpdate[IDC_REG_XM-REG_START] = ((Chipset.HST ^ OldChipset.HST) & XM) != 0;
	wsprintf(szBuffer,_T("XM=%d"),(Chipset.HST & XM) != 0);
	SetDlgItemText(hDlg,IDC_REG_XM,szBuffer);
	return;
}

//
// update memory window
//
static VOID UpdateMemoryWnd(HWND hDlg)
{
	// check follow mode setting for memory window
	switch(uIDFol)
	{
	case ID_DEBUG_MEM_FNONE:                                break;
	case ID_DEBUG_MEM_FADDR: dwAdrMem = Read5(dwAdrMemFol); break;
	case ID_DEBUG_MEM_FPC:   dwAdrMem = Chipset.pc;         break;
	case ID_DEBUG_MEM_FD0:   dwAdrMem = Chipset.d0;         break;
	case ID_DEBUG_MEM_FD1:   dwAdrMem = Chipset.d1;         break;
	default: _ASSERT(FALSE);
	}

	ViewMemWnd(hDlg,dwAdrMem);
	return;
}

//
// update stack window
//
static VOID UpdateStackWnd(HWND hDlg)
{
	INT   i;
	LONG  nPos;
	TCHAR szBuffer[64];

	HWND hWnd = GetDlgItem(hDlg,IDC_DEBUG_STACK);

	SendMessage(hWnd,WM_SETREDRAW,FALSE,0);
	nPos = (LONG) SendMessage(hWnd,LB_GETTOPINDEX,0,0);
	SendMessage(hWnd,LB_RESETCONTENT,0,0);
	for (i = 1; i <= ARRAYSIZEOF(Chipset.rstk); ++i)
	{
		INT j;

		wsprintf(szBuffer,_T("%d: %05X"), i, Chipset.rstk[(Chipset.rstkp-i)&7]);
		j = (INT) SendMessage(hWnd,LB_ADDSTRING,0,(LPARAM) szBuffer);
		SendMessage(hWnd,LB_SETITEMDATA,j,Chipset.rstk[(Chipset.rstkp-i)&7]);
	}
	SendMessage(hWnd,LB_SETTOPINDEX,nPos,0);
	SendMessage(hWnd,WM_SETREDRAW,TRUE,0);
	return;
}

//
// update MMU window
//
static VOID UpdateMmuWnd(HWND hDlg)
{
	TCHAR szBuffer[64];

	if (Chipset.IOCfig)
		wsprintf(szBuffer,_T("%05X"),Chipset.IOBase);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,IDC_MMU_IO_A,szBuffer);
	if (Chipset.P0Cfig)
		wsprintf(szBuffer,_T("%05X"),Chipset.P0Base<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,IDC_MMU_NCE2_A,szBuffer);
	if (Chipset.P0Cfg2)
		wsprintf(szBuffer,_T("%05X"),(Chipset.P0Size^0xFF)<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,IDC_MMU_NCE2_S,szBuffer);
	if (Chipset.P1Cfig)
		wsprintf(szBuffer,_T("%05X"),Chipset.P1Base<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,(cCurrentRomType=='S') ? IDC_MMU_CE1_A : IDC_MMU_CE2_A,szBuffer);
	if (Chipset.P1Cfg2)
		wsprintf(szBuffer,_T("%05X"),(Chipset.P1Size^0xFF)<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,(cCurrentRomType=='S') ? IDC_MMU_CE1_S : IDC_MMU_CE2_S,szBuffer);
	if (Chipset.P2Cfig)
		wsprintf(szBuffer,_T("%05X"),Chipset.P2Base<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,(cCurrentRomType=='S') ? IDC_MMU_CE2_A : IDC_MMU_NCE3_A,szBuffer);
	if (Chipset.P2Cfg2)
		wsprintf(szBuffer,_T("%05X"),(Chipset.P2Size^0xFF)<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,(cCurrentRomType=='S') ? IDC_MMU_CE2_S : IDC_MMU_NCE3_S,szBuffer);
	if (Chipset.BSCfig)
		wsprintf(szBuffer,_T("%05X"),Chipset.BSBase<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,(cCurrentRomType=='S') ? IDC_MMU_NCE3_A : IDC_MMU_CE1_A,szBuffer);
	if (Chipset.BSCfg2)
		wsprintf(szBuffer,_T("%05X"),(Chipset.BSSize^0xFF)<<12);
	else
		lstrcpy(szBuffer,_T("-----"));
	SetDlgItemText(hDlg,(cCurrentRomType=='S') ? IDC_MMU_NCE3_S : IDC_MMU_CE1_S,szBuffer);
	return;
}

//
// update miscellaneous window
//
static VOID UpdateMiscWnd(HWND hDlg)
{
	_ASSERTREG(IDC_MISC_INT);
	bRegUpdate[IDC_MISC_INT-REG_START] = Chipset.inte != OldChipset.inte;
	SetDlgItemText(hDlg,IDC_MISC_INT,Chipset.inte ? _T("On ") : _T("Off"));

	_ASSERTREG(IDC_MISC_KEY);
	bRegUpdate[IDC_MISC_KEY-REG_START] = Chipset.intk != OldChipset.intk;
	SetDlgItemText(hDlg,IDC_MISC_KEY,Chipset.intk ? _T("On ") : _T("Off"));

	_ASSERTREG(IDC_MISC_BS);
	bRegUpdate[IDC_MISC_BS-REG_START] = FALSE;
	// not 38/48S // CdB for HP: add Apples type
	if (cCurrentRomType!='A' && cCurrentRomType!='S')
	{
		TCHAR szBuffer[64];

		bRegUpdate[IDC_MISC_BS-REG_START] = (Chipset.Bank_FF & 0x7F) != (OldChipset.Bank_FF & 0x7F);
		wsprintf(szBuffer,_T("%02X"),Chipset.Bank_FF & 0x7F);
		SetDlgItemText(hDlg,IDC_MISC_BS,szBuffer);
	}
	return;
}

//
// update complete debugger dialog
//
VOID OnUpdate(HWND hDlg)
{
	nDbgState = DBG_STEPINTO;				// state "step into"
	dwDbgStopPC = -1;						// disable "cursor stop address"

	// enable debug buttons
	EnableMenuItem(GetMenu(hDlg),ID_DEBUG_RUN,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_DEBUG_RUNCURSOR,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_DEBUG_STEP,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_DEBUG_STEPOVER,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_DEBUG_STEPOUT,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_INFO_LASTINSTRUCTIONS,MF_ENABLED);
	EnableMenuItem(GetMenu(hDlg),ID_INFO_WRITEONLYREG,MF_ENABLED);

	// enable toolbar buttons
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_RUN,MAKELONG((TRUE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_BREAK,MAKELONG((FALSE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_RUNCURSOR,MAKELONG((TRUE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_STEP,MAKELONG((TRUE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_STEPOVER,MAKELONG((TRUE),0));
	SendMessage(hWndToolbar,TB_ENABLEBUTTON,ID_DEBUG_STEPOUT,MAKELONG((TRUE),0));

	// update windows
	UpdateCodeWnd(hDlg);					// update code window
	UpdateRegisterWnd(hDlg);				// update registers window
	UpdateMemoryWnd(hDlg);					// update memory window
	UpdateStackWnd(hDlg);					// update stack window
	UpdateMmuWnd(hDlg);						// update MMU window
	UpdateMiscWnd(hDlg);					// update bank switcher window
	UpdateProfileWnd(hDlgProfile);			// update profiler dialog
	ShowWindow(hDlg,SW_RESTORE);			// pop up if minimized
	SetFocus(hDlg);							// set focus to debugger
	return;
}


//################
//#
//#    Virtual key handler
//#
//################

//
// toggle breakpoint key handler (F2)
//
static BOOL OnKeyF2(HWND hDlg)
{
	HWND hWnd;
	RECT rc;
	LONG i;

	hWnd = GetDlgItem(hDlg,IDC_DEBUG_CODE);
	i = (LONG) SendMessage(hWnd,LB_GETCURSEL,0,0); // get selected item
	ToggleBreakpoint(dwAdrLine[i]);			// toggle breakpoint at address
	// update region of toggled item
	SendMessage(hWnd,LB_GETITEMRECT,i,(LPARAM)&rc);
	InvalidateRect(hWnd,&rc,TRUE);
	return -1;								// call windows default handler
}

//
// run key handler (F5)
//
static BOOL OnKeyF5(HWND hDlg)
{
	HWND  hWnd;
	INT   i,nPos;
	TCHAR szBuf[64];

	if (nDbgState != DBG_RUN)				// emulation stopped
	{
		DisableMenuKeys(hDlg);				// disable menu keys

		hWnd = GetDlgItem(hDlg,IDC_DEBUG_CODE);
		nPos = (INT) SendMessage(hWnd,LB_GETCURSEL,0,0);

		// clear "->" in code window
		for (i = 0; i < MAXCODELINES; ++i)
		{
			SendMessage(hWnd,LB_GETTEXT,i,(LPARAM) szBuf);
			if (szBuf[5] != _T(' '))		// PC in window
			{
				szBuf[5] = szBuf[6] = _T(' ');
				SendMessage(hWnd,LB_DELETESTRING,i,0);
				SendMessage(hWnd,LB_INSERTSTRING,i,(LPARAM) szBuf);
				break;
			}
		}
		SendMessage(hWnd,LB_SETCURSEL,nPos,0);

		nDbgState = DBG_RUN;				// state "run"
		OldChipset = Chipset;				// save chipset values
		SetEvent(hEventDebug);				// run emulation
	}
	return -1;								// call windows default handler
    UNREFERENCED_PARAMETER(hDlg);
}

//
// step cursor key handler (F6)
//
static BOOL OnKeyF6(HWND hDlg)
{
	if (nDbgState != DBG_RUN)				// emulation stopped
	{
		// get address of selected item
		INT nPos = (INT) SendDlgItemMessage(hDlg,IDC_DEBUG_CODE,LB_GETCURSEL,0,0);
		dwDbgStopPC = dwAdrLine[nPos];

		OnKeyF5(hDlg);						// run emulation
	}
	return -1;								// call windows default handler
}

//
// step into key handler (F7)
//
static BOOL OnKeyF7(HWND hDlg)
{
	if (nDbgState != DBG_RUN)				// emulation stopped
	{
		if (bDbgSkipInt)					// skip code in interrupt handler
			DisableMenuKeys(hDlg);			// disable menu keys

		nDbgState = DBG_STEPINTO;			// state "step into"
		OldChipset = Chipset;				// save chipset values
		SetEvent(hEventDebug);				// run emulation
	}
	return -1;								// call windows default handler
    UNREFERENCED_PARAMETER(hDlg);
}

//
// step over key handler (F8)
//
static BOOL OnKeyF8(HWND hDlg)
{
	if (nDbgState != DBG_RUN)				// emulation stopped
	{
		LPBYTE I = FASTPTR(Chipset.pc);

		if (bDbgSkipInt)					// skip code in interrupt handler
			DisableMenuKeys(hDlg);			// disable menu keys

		dwDbgRstkp = Chipset.rstkp;			// save stack level

		// GOSUB 7aaa, GOSUBL 8Eaaaa, GOSBVL 8Faaaaa
		if (I[0] == 0x7 || (I[0] == 0x8 && (I[1] == 0xE || I[1] == 0xF)))
		{
			nDbgState = DBG_STEPOVER;		// state "step over"
		}
		else
		{
			nDbgState = DBG_STEPINTO;		// state "step into"
		}
		OldChipset = Chipset;				// save chipset values
		SetEvent(hEventDebug);				// run emulation
	}
	return -1;								// call windows default handler
    UNREFERENCED_PARAMETER(hDlg);
}

//
// step out key handler (F9)
//
static BOOL OnKeyF9(HWND hDlg)
{
	if (nDbgState != DBG_RUN)				// emulation stopped
	{
		DisableMenuKeys(hDlg);				// disable menu keys
		dwDbgRstkp = (Chipset.rstkp-1)&7;	// save stack data
		dwDbgRstk  = Chipset.rstk[dwDbgRstkp];
		nDbgState = DBG_STEPOUT;			// state "step out"
		OldChipset = Chipset;				// save chipset values
		SetEvent(hEventDebug);				// run emulation
	}
	return -1;								// call windows default handler
    UNREFERENCED_PARAMETER(hDlg);
}

//
// break key handler (F11)
//
static BOOL OnKeyF11(HWND hDlg)
{
	nDbgState = DBG_STEPINTO;				// state "step into"
	if (Chipset.Shutdn)						// cpu thread stopped
		SetEvent(hEventShutdn);				// goto debug session
	return -1;								// call windows default handler
    UNREFERENCED_PARAMETER(hDlg);
}

//
// view of given address in disassembler window
//
static BOOL OnCodeGoAdr(HWND hDlg)
{
	DWORD dwAddress = -1;					// no address given

	OnEnterAddress(hDlg, &dwAddress);
	if (dwAddress != -1)
	{
		HWND hWnd = GetDlgItem(hDlg,IDC_DEBUG_CODE);
		ViewCodeWnd(hWnd,dwAddress & 0xFFFFF);
		SendMessage(hWnd,LB_SETCURSEL,0,0);
	}
	return -1;								// call windows default handler
}

//
// view pc in disassembler window
//
static BOOL OnCodeGoPC(HWND hDlg)
{
	UpdateCodeWnd(hDlg);
	return 0;
}

//
// set pc to selection
//
static BOOL OnCodeSetPcToSelection(HWND hDlg)
{
	Chipset.pc = dwAdrLine[SendDlgItemMessage(hDlg,IDC_DEBUG_CODE,LB_GETCURSEL,0,0)];
	return OnCodeGoPC(hDlg);
}

//
// view from address in memory window
//
static BOOL OnMemGoDx(HWND hDlg, DWORD dwAddress)
{
	HWND hWnd = GetDlgItem(hDlg,IDC_DEBUG_MEM_COL0);

	ViewMemWnd(hDlg, dwAddress);
	SendMessage(hWnd,LB_SETCURSEL,0,0);
	SetFocus(hWnd);
	return -1;								// call windows default handler
}

//
// view of given address in memory window
//
static BOOL OnMemGoAdr(HWND hDlg)
{
	DWORD dwAddress = -1;					// no address given

	OnEnterAddress(hDlg, &dwAddress);
	if (dwAddress != -1)					// not Cancel key
		OnMemGoDx(hDlg,dwAddress & (dwMapDataSize - 1));
	return -1;								// call windows default handler
}

//
// view from address in memory window
//
static BOOL OnMemFollow(HWND hDlg,UINT uID)
{
	if (ID_DEBUG_MEM_FADDR == uID)			// ask for follow address
	{
		DWORD dwAddress = -1;				// no address given

		OnEnterAddress(hDlg, &dwAddress);
		if (dwAddress == -1) return -1;		// return at cancel button

		dwAdrMemFol = dwAddress;
	}

	CheckMenuItem(hMenuMem,uIDFol,MF_UNCHECKED);
	uIDFol = uID;
	CheckMenuItem(hMenuMem,uIDFol,MF_CHECKED);
	UpdateMemoryWnd(hDlg);					// update memory window
	return -1;								// call windows default handler
}

//
// clear all breakpoints
//
static BOOL OnClearAll(HWND hDlg)
{
	wBreakpointCount = 0;
	// redraw code window
	InvalidateRect(GetDlgItem(hDlg,IDC_DEBUG_CODE),NULL,TRUE);
	return 0;
}

//
// toggle menu selection
//
static BOOL OnToggleMenuItem(HWND hDlg,UINT uIDCheckItem,BOOL *bCheck)
{
	*bCheck = !*bCheck;					// toggle flag
	CheckMenuItem(GetMenu(hDlg),uIDCheckItem,*bCheck ? MF_CHECKED : MF_UNCHECKED);
	return 0;
}

//
// change memory window mapping style
//
static BOOL OnMemMapping(HWND hDlg,UINT uID)
{
	if (uID == uIDMap) return -1;			// same view, call windows default handler

	SetMappingMenu(hDlg,uID);				// update menu settings
	UpdateMemoryWnd(hDlg);					// update memory window
	return 0;
}

//
// push value on hardware stack
//
static BOOL OnStackPush(HWND hDlg)
{
	TCHAR szBuffer[] = _T("00000");
	DWORD dwAddr;
	HWND  hWnd;
	INT   i,j;

	if (nDbgState != DBG_STEPINTO)			// not in single step mode
		return TRUE;

	hWnd = GetDlgItem(hDlg,IDC_DEBUG_STACK);

	i = (INT) SendMessage(hWnd,LB_GETCURSEL,0,0);
	if (LB_ERR == i) return TRUE;			// no selection

	if (IDOK != OnNewValue(szBuffer))		// canceled function
		return TRUE;
	_stscanf(szBuffer,_T("%5X"),&dwAddr);

	// push stack element
	for (j = ARRAYSIZEOF(Chipset.rstk); j > i + 1; --j)
	{
		Chipset.rstk[(Chipset.rstkp-j)&7] = Chipset.rstk[(Chipset.rstkp-j+1)&7];
	}
	Chipset.rstk[(Chipset.rstkp-j)&7] = dwAddr;

	UpdateStackWnd(hDlg);					// update stack window
	SendMessage(hWnd,LB_SETCURSEL,i,0);		// restore cursor postion
	return 0;
}

//
// pop value from hardware stack
//
static BOOL OnStackPop(HWND hDlg)
{
	HWND  hWnd;
	INT   i,j;

	if (nDbgState != DBG_STEPINTO)			// not in single step mode
		return TRUE;

	hWnd = GetDlgItem(hDlg,IDC_DEBUG_STACK);

	i = (INT) SendMessage(hWnd,LB_GETCURSEL,0,0);
	if (LB_ERR == i) return TRUE;			// no selection

	// pop stack element
	for (j = i + 1; j < ARRAYSIZEOF(Chipset.rstk); ++j)
	{
		Chipset.rstk[(Chipset.rstkp-j)&7] = Chipset.rstk[(Chipset.rstkp-j-1)&7];
	}
	Chipset.rstk[Chipset.rstkp] = 0;

	UpdateStackWnd(hDlg);					// update stack window
	SendMessage(hWnd,LB_SETCURSEL,i,0);		// restore cursor postion
	return 0;
}

// modify value on hardware stack
static BOOL OnStackModify(HWND hDlg)
{
	TCHAR szBuffer[16];
	HWND  hWnd;
	INT   i;

	if (nDbgState != DBG_STEPINTO)			// not in single step mode
		return TRUE;

	hWnd = GetDlgItem(hDlg,IDC_DEBUG_STACK);

	i = (INT) SendMessage(hWnd,LB_GETCURSEL,0,0);
	if (LB_ERR == i) return TRUE;			// no selection

	SendMessage(hWnd,LB_GETTEXT,i,(LPARAM) szBuffer);
	OnNewValue(&szBuffer[3]);
	_stscanf(&szBuffer[3],_T("%5X"),&Chipset.rstk[(Chipset.rstkp-i-1)&7]);

	UpdateStackWnd(hDlg);					// update stack window
	SendMessage(hWnd,LB_SETCURSEL,i,0);		// restore cursor postion
	return 0;
}

//
// new register setting
//
static BOOL OnLButtonUp(HWND hDlg, LPARAM lParam)
{
	TCHAR szBuffer[64];
	POINT pt;
	HWND  hWnd;
	INT   nId;

	if (nDbgState != DBG_STEPINTO)			// not in single step mode
		return TRUE;

	POINTSTOPOINT(pt,MAKEPOINTS(lParam));

	// handle of selected window
	hWnd = ChildWindowFromPointEx(hDlg,pt,CWP_SKIPDISABLED);
	nId  = GetDlgCtrlID(hWnd);				// control ID of window

	GetWindowText(hWnd,szBuffer,ARRAYSIZEOF(szBuffer));
	switch (nId)
	{
	case IDC_REG_A: // A
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.A,16,&szBuffer[3]);
		break;
	case IDC_REG_B: // B
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.B,16,&szBuffer[3]);
		break;
	case IDC_REG_C: // C
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.C,16,&szBuffer[3]);
		break;
	case IDC_REG_D: // D
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.D,16,&szBuffer[3]);
		break;
	case IDC_REG_R0: // R0
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.R0,16,&szBuffer[3]);
		break;
	case IDC_REG_R1: // R1
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.R1,16,&szBuffer[3]);
		break;
	case IDC_REG_R2: // R2
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.R2,16,&szBuffer[3]);
		break;
	case IDC_REG_R3: // R3
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.R3,16,&szBuffer[3]);
		break;
	case IDC_REG_R4: // R4
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.R4,16,&szBuffer[3]);
		break;
	case IDC_REG_D0: // D0
		OnNewValue(&szBuffer[3]);
		_stscanf(&szBuffer[3],_T("%5X"),&Chipset.d0);
		break;
	case IDC_REG_D1: // D1
		OnNewValue(&szBuffer[3]);
		_stscanf(&szBuffer[3],_T("%5X"),&Chipset.d1);
		break;
	case IDC_REG_P: // P
		OnNewValue(&szBuffer[2]);
		Chipset.P = _totupper(szBuffer[2]) - _T('0');
		if (Chipset.P > 9) Chipset.P -= 7;
		PCHANGED;
		break;
	case IDC_REG_PC: // PC
		OnNewValue(&szBuffer[3]);
		_stscanf(&szBuffer[3],_T("%5X"),&Chipset.pc);
		break;
	case IDC_REG_OUT: // OUT
		OnNewValue(&szBuffer[4]);
		_stscanf(&szBuffer[4],_T("%3X"),&Chipset.out);
		break;
	case IDC_REG_IN: // IN
		OnNewValue(&szBuffer[3]);
		_stscanf(&szBuffer[3],_T("%4X"),&Chipset.in);
		break;
	case IDC_REG_ST: // ST
		OnNewValue(&szBuffer[3]);
		StrToReg(Chipset.ST,4,&szBuffer[3]);
		break;
	case IDC_REG_CY: // CY
		Chipset.carry = !Chipset.carry;
		break;
	case IDC_REG_MODE: // MODE
		Chipset.mode_dec = !Chipset.mode_dec;
		break;
	case IDC_REG_MP: // MP
		Chipset.HST ^= MP;
		break;
	case IDC_REG_SR: // SR
		Chipset.HST ^= SR;
		break;
	case IDC_REG_SB: // SB
		Chipset.HST ^= SB;
		break;
	case IDC_REG_XM: // XM
		Chipset.HST ^= XM;
		break;
	case IDC_MISC_INT: // interrupt status
		Chipset.inte = !Chipset.inte;
		UpdateMiscWnd(hDlg);				// update miscellaneous window
		break;
	case IDC_MISC_KEY: // 1ms keyboard scan
		Chipset.intk = !Chipset.intk;
		UpdateMiscWnd(hDlg);				// update miscellaneous window
		break;
	case IDC_MISC_BS: // Bank switcher setting
		OnNewValue(szBuffer);
		_stscanf(szBuffer,_T("%2X"),&Chipset.Bank_FF);
		Chipset.Bank_FF &= 0x7F;
		RomSwitch(Chipset.Bank_FF);			// update memory mapping

		UpdateCodeWnd(hDlg);				// update code window
		UpdateMemoryWnd(hDlg);				// update memory window
		UpdateMiscWnd(hDlg);				// update miscellaneous window
		break;
	}
	UpdateRegisterWnd(hDlg);				// update register
	return TRUE;
}

//
// double click in list box area
//
static BOOL OnDblClick(HWND hWnd, WORD wId)
{
	TCHAR szBuffer[4];
	BYTE  byData;
	INT   i;
	DWORD dwAddress;

	if (wId == IDC_DEBUG_STACK)				// stack list box
	{
		// get cpu address of selected item
		i = (INT) SendMessage(hWnd,LB_GETCURSEL,0,0);
		dwAddress = (DWORD) SendMessage(hWnd,LB_GETITEMDATA,i,0);

		// show code of this address
		ViewCodeWnd(GetDlgItem(GetParent(hWnd),IDC_DEBUG_CODE),dwAddress);
		return TRUE;
	}

	for (i = 0; i < MEMWNDMAX; ++i)			// scan all Id's
		if (nCol[i] == wId)					// found ID
			break;

	// not IDC_DEBUG_MEM window or module mode -> default handler
	if (i == MEMWNDMAX || ID_DEBUG_MEM_MAP != uIDMap)
		return FALSE;

	// calculate address of byte
	dwAddress = i * 2;
	i = (INT) SendMessage(hWnd,LB_GETCARETINDEX,0,0);
	dwAddress += MAXMEMITEMS * i + dwAdrMem;

	// enter new value
	SendMessage(hWnd,LB_GETTEXT,i,(LPARAM) szBuffer);
	OnNewValue(szBuffer);
	_stscanf(szBuffer,_T("%2X"), &byData);
	byData = (byData >> 4) | (byData << 4);	// change nibbles for writing

	Write2(dwAddress, byData);				// write data
	ViewMemWnd(GetParent(hWnd),dwAdrMem);	// update memory window
	SendMessage(hWnd,LB_SETCURSEL,i,0);
	return FALSE;
}

//
// request for context menu
//
static VOID OnContextMenu(HWND hDlg, LPARAM lParam, WPARAM wParam)
{
	POINT pt;
	INT   nId;

	POINTSTOPOINT(pt,MAKEPOINTS(lParam));	// mouse position
	nId  = GetDlgCtrlID((HWND) wParam);		// control ID of window

	switch(nId)
	{
	case IDC_DEBUG_CODE: // handle code window
		TrackPopupMenu(hMenuCode,0,pt.x,pt.y,0,hDlg,NULL);
		break;

	case IDC_DEBUG_MEM_COL0:
	case IDC_DEBUG_MEM_COL1:
	case IDC_DEBUG_MEM_COL2:
	case IDC_DEBUG_MEM_COL3:
	case IDC_DEBUG_MEM_COL4:
	case IDC_DEBUG_MEM_COL5:
	case IDC_DEBUG_MEM_COL6:
	case IDC_DEBUG_MEM_COL7: // handle memory window
		TrackPopupMenu(hMenuMem,0,pt.x,pt.y,0,hDlg,NULL);
		break;

	case IDC_DEBUG_STACK: // handle code window
		TrackPopupMenu(hMenuStack,0,pt.x,pt.y,0,hDlg,NULL);
		break;
	}
	return;
}

//
// mouse setting for capturing window
//
static BOOL OnSetCursor(HWND hDlg)
{
	// debugger not active but cursor is over debugger window
	if (bActFollowsMouse && GetActiveWindow() != hDlg)
	{
		ForceForegroundWindow(hDlg);		// force debugger window to foreground
	}
	return FALSE;
}

//################
//#
//#    Dialog handler
//#
//################

//
// handle right/left keys in memory window
//
static __inline BOOL OnKeyRightLeft(HWND hWnd, WPARAM wParam)
{
	HWND hWndNew;
	WORD wX;
	INT  nId;

	nId  = GetDlgCtrlID(hWnd);				// control ID of window

	for (wX = 0; wX < MEMWNDMAX; ++wX)		// scan all Id's
		if (nCol[wX] == nId)				// found ID
			break;

	if (wX == MEMWNDMAX) return -1;			// not IDC_DEBUG_MEM window, default handler

	// new position
	wX = ((LOWORD(wParam) == VK_RIGHT) ? (wX + 1) : (wX + MEMWNDMAX - 1)) % MEMWNDMAX;

	// set new focus
	hWndNew = GetDlgItem(GetParent(hWnd),nCol[wX]);
	SendMessage(hWndNew,LB_SETCURSEL,HIWORD(wParam),0);
	SetFocus(hWndNew);
	return -2;
}

//
// handle (page) up/down keys in memory window
//
static __inline BOOL OnKeyUpDown(HWND hWnd, WPARAM wParam)
{
	INT  wX, wY;
	INT  nId;

	nId  = GetDlgCtrlID(hWnd);				// control ID of window

	for (wX = 0; wX < MEMWNDMAX; ++wX)		// scan all Id's
		if (nCol[wX] == nId)				// found ID
			break;

	if (wX == MEMWNDMAX) return -1;			// not IDC_DEBUG_MEM window, default handler

	wY = HIWORD(wParam);					// get old focus

	switch(LOWORD(wParam))
	{
	case VK_NEXT:
		dwAdrMem = (dwAdrMem + MAXMEMITEMS * MAXMEMLINES) & (dwMapDataSize - 1);
		ViewMemWnd(GetParent(hWnd),dwAdrMem);
		SendMessage(hWnd,LB_SETCURSEL,wY,0);
		return -2;

	case VK_PRIOR:
		dwAdrMem = (dwAdrMem - MAXMEMITEMS * MAXMEMLINES) & (dwMapDataSize - 1);
		ViewMemWnd(GetParent(hWnd),dwAdrMem);
		SendMessage(hWnd,LB_SETCURSEL,wY,0);
		return -2;

	case VK_DOWN:
		if (wY+1 >= MAXMEMLINES)
		{
			dwAdrMem = (dwAdrMem + MAXMEMITEMS) & (dwMapDataSize - 1);
			ViewMemWnd(GetParent(hWnd),dwAdrMem);
			SendMessage(hWnd,LB_SETCURSEL,wY,0);
			return -2;
		}
		break;

	case VK_UP:
		if (wY == 0)
		{
			dwAdrMem = (dwAdrMem - MAXMEMITEMS) & (dwMapDataSize - 1);
			ViewMemWnd(GetParent(hWnd),dwAdrMem);
			SendMessage(hWnd,LB_SETCURSEL,wY,0);
			return -2;
		}
		break;
	}
	return -1;
}

//
// handle keys in code window
//
static __inline BOOL OnKeyCodeWnd(HWND hDlg, WPARAM wParam)
{
	HWND hWnd  = GetDlgItem(hDlg,IDC_DEBUG_CODE);
	WORD wKey  = LOWORD(wParam);
	WORD wItem = HIWORD(wParam);

	// down key on last line
	if ((wKey == VK_DOWN || wKey == VK_NEXT) && wItem == MAXCODELINES - 1)
	{
		ViewCodeWnd(hWnd,dwAdrLine[1]);
		SendMessage(hWnd,LB_SETCURSEL,wItem,0);
	}
	// up key on first line
	if ((wKey == VK_UP || wKey == VK_PRIOR) && wItem == 0)
	{
		if (dwAdrLine[0] > 0) ViewCodeWnd(hWnd,dwAdrLine[0]-1);
	}

	if (wKey == _T('G')) return OnCodeGoAdr(GetParent(hWnd)); // goto new address

	return -1;
}

//
// handle drawing in code window
//
static __inline BOOL OnDrawCodeWnd(LPDRAWITEMSTRUCT lpdis)
{
	TCHAR    szBuf[64];
	COLORREF crBkColor;
	COLORREF crTextColor;
	BOOL     bBrk,bPC;

	if (lpdis->itemID == -1)				// no item in list box
		return TRUE;

	// get item text
	SendMessage(lpdis->hwndItem,LB_GETTEXT,lpdis->itemID,(LPARAM) szBuf);

	// check for codebreakpoint
	bBrk = CheckBreakpoint(dwAdrLine[lpdis->itemID],1,BP_EXEC);
	bPC = szBuf[5] != _T(' ');				// check if line of program counter

	crTextColor = COLOR_WHITE;				// standard text color

	if (lpdis->itemState & ODS_SELECTED)	// cursor line
	{
		if (bPC)							// PC line
		{
			crBkColor = bBrk ? COLOR_DKGRAY : COLOR_TEAL;
		}
		else								// normal line
		{
			crBkColor = bBrk ? COLOR_PURPLE : COLOR_NAVY;
		}
	}
	else									// not cursor line
	{
		if (bPC)							// PC line
		{
			crBkColor = bBrk ? COLOR_OLIVE : COLOR_GREEN;
		}
		else								// normal line
		{
			if (bBrk)
			{
				crBkColor   = COLOR_MAROON;
			}
			else
			{
				crBkColor   = COLOR_WHITE;
				crTextColor = COLOR_BLACK;
			}
		}
	}

	// write Text
	crBkColor   = SetBkColor(lpdis->hDC,crBkColor);
	crTextColor = SetTextColor(lpdis->hDC,crTextColor);

	ExtTextOut(lpdis->hDC,(int)(lpdis->rcItem.left)+2,(int)(lpdis->rcItem.top),
	           ETO_OPAQUE,(LPRECT)&lpdis->rcItem,szBuf,lstrlen(szBuf),NULL);

	SetBkColor(lpdis->hDC,crBkColor);
	SetTextColor(lpdis->hDC,crTextColor);

	if (lpdis->itemState & ODS_FOCUS)		// redraw focus
		DrawFocusRect(lpdis->hDC,&lpdis->rcItem);

	return TRUE;							// focus handled here
}

//
// detect changed register
//
static __inline BOOL OnCtlColorStatic(HWND hWnd)
{
	BOOL bError = FALSE;					// not changed

	int nId = GetDlgCtrlID(hWnd);
	if (nId >= REG_START && nId <= REG_STOP) // in register area
		bError = bRegUpdate[nId-REG_START];	// register changed?
	return bError;
}


//################
//#
//#    Public functions
//#
//################

//
// handle upper 32 bit of cpu cycle counter
//
VOID UpdateDbgCycleCounter(VOID)
{
	// update 64 bit cpu cycle counter
	if (Chipset.cycles < dwDbgRefCycles) ++Chipset.cycles_reserved;
	dwDbgRefCycles = (DWORD) (Chipset.cycles & 0xFFFFFFFF);
	return;
}

//
// check for breakpoints
//
BOOL CheckBreakpoint(DWORD dwAddr, DWORD dwRange, UINT nType)
{
	INT i;

	for (i = 0; i < wBreakpointCount; ++i)	// scan all breakpoints
	{
		// check address range and type
		if (   sBreakpoint[i].bEnable
			&& sBreakpoint[i].dwAddr >= dwAddr && sBreakpoint[i].dwAddr < dwAddr + dwRange
			&& (sBreakpoint[i].nType & nType) != 0)
			return TRUE;
	}
	return FALSE;
}

//
// notify debugger that emulation stopped
//
VOID NotifyDebugger(INT nType)				// update registers
{
	nRplBreak = nType;						// save breakpoint type
	_ASSERT(hDlgDebug);						// debug dialog box open
	PostMessage(hDlgDebug,WM_UPDATE,0,0);
	return;
}

//
// disable debugger
//
VOID DisableDebugger(VOID)
{
	if (hDlgDebug)							// debugger running
		DestroyWindow(hDlgDebug);			// then close debugger to renter emulation
	return;
}


//################
//#
//#    Debugger Message loop
//#
//################

//
// ID_TOOL_DEBUG
//
static __inline HWND CreateToolbar(HWND hWnd)
{
	HRSRC        hRes;
	HGLOBAL      hGlobal;
	CToolBarData *pData;
    TBBUTTON     *ptbb;
	INT          i,j;

	HWND hWndToolbar = NULL;				// toolbar window

	InitCommonControls();					// ensure that common control DLL is loaded

	if ((hRes = FindResource(hApp,MAKEINTRESOURCE(IDR_DEBUG_TOOLBAR),RT_TOOLBAR)) == NULL)
		goto quit;

	if ((hGlobal = LoadResource(hApp,hRes)) == NULL)
		goto quit;

	if ((pData = (CToolBarData*) LockResource(hGlobal)) == NULL)
		goto unlock;

	_ASSERT(pData->wVersion == 1);			// toolbar resource version

	// alloc memory for TBBUTTON stucture
    if (!(ptbb = HeapAlloc(hHeap,0,pData->wItemCount*sizeof(TBBUTTON))))
		goto unlock;

	// fill TBBUTTON stucture with resource data
	for (i = j = 0; i < pData->wItemCount; ++i)
    {
		if (pData->aItems[i])
		{
			ptbb[i].iBitmap = j++;
			ptbb[i].fsStyle = TBSTYLE_BUTTON;
		}
		else
		{
			ptbb[i].iBitmap = 5;			// separator width
			ptbb[i].fsStyle = TBSTYLE_SEP;
		}
		ptbb[i].idCommand = pData->aItems[i];
		ptbb[i].fsState = TBSTATE_ENABLED;
		ptbb[i].dwData  = 0;
		ptbb[i].iString = j;
    }

	hWndToolbar = CreateToolbarEx(hWnd,WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
								  IDR_DEBUG_TOOLBAR,j,hApp,IDR_DEBUG_TOOLBAR,ptbb,pData->wItemCount,
								  pData->wWidth,pData->wHeight,pData->wWidth,pData->wHeight,
								  sizeof(TBBUTTON));

	HeapFree(hHeap,0,ptbb);

unlock:
	FreeResource(hGlobal);
quit:
	return hWndToolbar;
}

static INT_PTR CALLBACK Debugger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HMENU hMenuMainCode,hMenuMainMem,hMenuMainStack;

	WINDOWPLACEMENT wndpl;
 	TEXTMETRIC tm;
	HDC   hDC;
	HFONT hFont;
	INT   i;

	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowLocation(hDlg,nDbgPosX,nDbgPosY);
		if (bAlwaysOnTop) SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
		SendMessage(hDlg,WM_SETICON,ICON_BIG,(LPARAM) LoadIcon(hApp,MAKEINTRESOURCE(IDI_EMU48)));
		hWndToolbar = CreateToolbar(hDlg);	// add toolbar
		CheckMenuItem(GetMenu(hDlg),ID_BREAKPOINTS_NOP3,  bDbgNOP3    ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(GetMenu(hDlg),ID_BREAKPOINTS_DOCODE,bDbgCode    ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(GetMenu(hDlg),ID_BREAKPOINTS_RPL,   bDbgRPL     ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(GetMenu(hDlg),ID_INTR_STEPOVERINT,  bDbgSkipInt ? MF_CHECKED : MF_UNCHECKED);
		hDlgDebug = hDlg;					// handle for debugger dialog
		hEventDebug = CreateEvent(NULL,FALSE,FALSE,NULL);
		if (hEventDebug == NULL)
		{
			AbortMessage(_T("Event creation failed !"));
			return TRUE;
		}
		hMenuMainCode = LoadMenu(hApp,MAKEINTRESOURCE(IDR_DEBUG_CODE));
		_ASSERT(hMenuMainCode);
		hMenuCode = GetSubMenu(hMenuMainCode, 0);
		_ASSERT(hMenuCode);
		hMenuMainMem = LoadMenu(hApp,MAKEINTRESOURCE(IDR_DEBUG_MEM));
		_ASSERT(hMenuMainMem);
		hMenuMem = GetSubMenu(hMenuMainMem, 0);
		_ASSERT(hMenuMem);
		hMenuMainStack = LoadMenu(hApp,MAKEINTRESOURCE(IDR_DEBUG_STACK));
		_ASSERT(hMenuMainStack);
		hMenuStack = GetSubMenu(hMenuMainStack, 0);
		_ASSERT(hMenuStack);

		// font settings
		SendDlgItemMessage(hDlg,IDC_STATIC_CODE,     WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_STATIC_REGISTERS,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_STATIC_MEMORY,   WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_STATIC_STACK,    WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_STATIC_MMU,      WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_STATIC_MISC,     WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));

		// init last instruction circular buffer
		pdwInstrArray = HeapAlloc(hHeap,0,INSTRSIZE*sizeof(*pdwInstrArray));
		wInstrSize = INSTRSIZE;				// size of last instruction array
		wInstrWp = wInstrRp = 0;			// write/read pointer

		// init "Follow" menu entry in debugger "Memory" context menu
		CheckMenuItem(hMenuMem,uIDFol,MF_CHECKED);

		InitMemMap(hDlg);					// init memory mapping table
		InitBsArea(hDlg);					// init bank switcher list box
		DisableMenuKeys(hDlg);				// set debug menu keys into run state

		disassembler_map = MEM_MAP;			// disassemble with mapped modules

		dwDbgStopPC = -1;					// no stop address for goto cursor
		dwDbgRplPC = -1;					// no stop address for RPL breakpoint

		// init reference cpu cycle counter for 64 bit debug cycle counter
		dwDbgRefCycles = (DWORD) (Chipset.cycles & 0xFFFFFFFF);

		nDbgState = DBG_STEPINTO;			// state "step into"
		if (Chipset.Shutdn)					// cpu thread stopped
			SetEvent(hEventShutdn);			// goto debug session

		UpdateWindowStatus();				// disable application menu items
		OldChipset = Chipset;				// save chipset values
		return TRUE;

	case WM_DESTROY:
		// SetHP48Time();					// update time & date
		nDbgState = DBG_OFF;				// debugger inactive
		bInterrupt = TRUE;					// exit opcode loop
		SetEvent(hEventDebug);
		if (pdwInstrArray)					// free last instruction circular buffer
		{
			HeapFree(hHeap,0,pdwInstrArray);
			pdwInstrArray = NULL;
		}
		CloseHandle(hEventDebug);
		wndpl.length = sizeof(wndpl);		// save debugger window position
		GetWindowPlacement(hDlg, &wndpl);
		nDbgPosX = wndpl.rcNormalPosition.left;
		nDbgPosY = wndpl.rcNormalPosition.top;
		DestroyMenu(hMenuMainCode);
		DestroyMenu(hMenuMainMem);
		DestroyMenu(hMenuMainStack);
 		_ASSERT(hWnd);
		UpdateWindowStatus();				// enable application menu items
		hDlgDebug = NULL;					// debugger windows closed
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_UPDATE:
		OnUpdate(hDlg);
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case LBN_DBLCLK:
			return OnDblClick((HWND) lParam, LOWORD(wParam));

		case LBN_SETFOCUS:
			i = (INT) SendMessage((HWND) lParam,LB_GETCARETINDEX,0,0);
			SendMessage((HWND) lParam,LB_SETCURSEL,i,0);
			return TRUE;

		case LBN_KILLFOCUS:
			SendMessage((HWND) lParam,LB_SETCURSEL,-1,0);
			return TRUE;
		}

		switch (LOWORD(wParam))
		{
		case ID_BREAKPOINTS_SETBREAK:     return OnKeyF2(hDlg);
		case ID_DEBUG_RUN:                return OnKeyF5(hDlg);
		case ID_DEBUG_RUNCURSOR:          return OnKeyF6(hDlg);
		case ID_DEBUG_STEP:               return OnKeyF7(hDlg);
		case ID_DEBUG_STEPOVER:           return OnKeyF8(hDlg);
		case ID_DEBUG_STEPOUT:            return OnKeyF9(hDlg);
		case ID_DEBUG_BREAK:              return OnKeyF11(hDlg);
		case ID_DEBUG_CODE_GOADR:         return OnCodeGoAdr(hDlg);
		case ID_DEBUG_CODE_GOPC:          return OnCodeGoPC(hDlg);
		case ID_DEBUG_CODE_SETPCTOSELECT: return OnCodeSetPcToSelection(hDlg);
		case ID_BREAKPOINTS_CODEEDIT:     return OnEditBreakpoint(hDlg);
		case ID_BREAKPOINTS_CLEARALL:     return OnClearAll(hDlg);
		case ID_BREAKPOINTS_NOP3:         return OnToggleMenuItem(hDlg,LOWORD(wParam),&bDbgNOP3);
		case ID_BREAKPOINTS_DOCODE:       return OnToggleMenuItem(hDlg,LOWORD(wParam),&bDbgCode);
		case ID_BREAKPOINTS_RPL:          return OnToggleMenuItem(hDlg,LOWORD(wParam),&bDbgRPL);
		case ID_INFO_LASTINSTRUCTIONS:    return OnInfoIntr(hDlg);
		case ID_INFO_PROFILE:             return OnProfile(hDlg);
		case ID_INFO_WRITEONLYREG:        return OnInfoWoRegister(hDlg);
		case ID_INTR_STEPOVERINT:         return OnToggleMenuItem(hDlg,LOWORD(wParam),&bDbgSkipInt);
		case ID_DEBUG_MEM_GOADR:          return OnMemGoAdr(hDlg);
		case ID_DEBUG_MEM_GOPC:           return OnMemGoDx(hDlg,Chipset.pc);
		case ID_DEBUG_MEM_GOD0:           return OnMemGoDx(hDlg,Chipset.d0);
		case ID_DEBUG_MEM_GOD1:           return OnMemGoDx(hDlg,Chipset.d1);
		case ID_DEBUG_MEM_GOSTACK:        return OnMemGoDx(hDlg,Chipset.rstk[(Chipset.rstkp-1)&7]);
		case ID_DEBUG_MEM_FNONE:
		case ID_DEBUG_MEM_FADDR:
		case ID_DEBUG_MEM_FPC:
		case ID_DEBUG_MEM_FD0:
		case ID_DEBUG_MEM_FD1:            return OnMemFollow(hDlg,LOWORD(wParam));
		case ID_DEBUG_MEM_FIND:           return OnMemFind(hDlg);
		case ID_DEBUG_MEM_MAP:
		case ID_DEBUG_MEM_NCE1:
		case ID_DEBUG_MEM_NCE2:
		case ID_DEBUG_MEM_CE1:
		case ID_DEBUG_MEM_CE2:
		case ID_DEBUG_MEM_NCE3:           return OnMemMapping(hDlg,LOWORD(wParam));
		case ID_DEBUG_STACK_PUSH:         return OnStackPush(hDlg);
		case ID_DEBUG_STACK_POP:          return OnStackPop(hDlg);
		case ID_DEBUG_STACK_MODIFY:       return OnStackModify(hDlg);

		case ID_DEBUG_CANCEL:             DestroyWindow(hDlg); return TRUE;
		}
		break;

	case WM_VKEYTOITEM:
		switch (LOWORD(wParam))				// always valid
		{
		case VK_F2:  return OnKeyF2(hDlg);	// toggle breakpoint
		case VK_F5:  return OnKeyF5(hDlg);	// key run
		case VK_F6:  return OnKeyF6(hDlg);  // key step cursor
		case VK_F7:  return OnKeyF7(hDlg);	// key step into
		case VK_F8:  return OnKeyF8(hDlg);	// key step over
		case VK_F9:  return OnKeyF9(hDlg);  // key step out
		case VK_F11: return OnKeyF11(hDlg);	// key break
		}

		switch(GetDlgCtrlID((HWND) lParam))	// calling window
		{
		// handle code window
		case IDC_DEBUG_CODE:
			return OnKeyCodeWnd(hDlg, wParam);

		// handle memory window
		case IDC_DEBUG_MEM_COL0:
		case IDC_DEBUG_MEM_COL1:
		case IDC_DEBUG_MEM_COL2:
		case IDC_DEBUG_MEM_COL3:
		case IDC_DEBUG_MEM_COL4:
		case IDC_DEBUG_MEM_COL5:
		case IDC_DEBUG_MEM_COL6:
		case IDC_DEBUG_MEM_COL7:
			switch (LOWORD(wParam))
			{
			case _T('G'):  return OnMemGoAdr(GetParent((HWND) lParam));
			case _T('F'):  return OnMemFind(GetParent((HWND) lParam));
			case VK_RIGHT:
			case VK_LEFT:  return OnKeyRightLeft((HWND) lParam, wParam);
			case VK_NEXT:
			case VK_PRIOR:
			case VK_DOWN:
			case VK_UP:    return OnKeyUpDown((HWND) lParam, wParam);
			}
			break;
		}
		return -1;							// default action

	case WM_LBUTTONUP:
		return OnLButtonUp(hDlg, lParam);

	case WM_CONTEXTMENU:
		OnContextMenu(hDlg, lParam, wParam);
		break;

	case WM_SETCURSOR:
		return OnSetCursor(hDlg);

	case WM_CTLCOLORSTATIC:					// register color highlighting
		// highlight text?
		if (OnCtlColorStatic((HWND) lParam))
		{
			SetTextColor((HDC) wParam, COLOR_RED);
			SetBkColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
			return (INT_PTR) GetStockObject(NULL_BRUSH); // transparent brush
		}
		break;

	case WM_NOTIFY:
		// tooltip for toolbar
		if (((LPNMHDR) lParam)->code == TTN_GETDISPINFO)
		{
			((LPTOOLTIPTEXT) lParam)->hinst = hApp;
			((LPTOOLTIPTEXT) lParam)->lpszText = MAKEINTRESOURCE(((LPTOOLTIPTEXT) lParam)->hdr.idFrom);
			break;
		}
		break;

	case WM_DRAWITEM:
		if (wParam == IDC_DEBUG_CODE) return OnDrawCodeWnd((LPDRAWITEMSTRUCT) lParam);
		break;

	case WM_MEASUREITEM:
		hDC = GetDC(hDlg);

		// GetTextMetrics from "Courier New 8" font
		hFont = CreateFont(-MulDiv(8,GetDeviceCaps(hDC, LOGPIXELSY),72),0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
		                   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,_T("Courier New"));

		hFont = SelectObject(hDC,hFont);
		GetTextMetrics(hDC,&tm);
		hFont = SelectObject(hDC,hFont);
		DeleteObject(hFont);

		((LPMEASUREITEMSTRUCT) lParam)->itemHeight = tm.tmHeight;
		lCharWidth = tm.tmAveCharWidth;

		ReleaseDC(hDlg,hDC);
		return TRUE;
	}
	return FALSE;
}

LRESULT OnToolDebug(VOID)					// debugger dialogbox call
{
	if ((hDlgDebug = CreateDialog(hApp,MAKEINTRESOURCE(IDD_DEBUG),NULL,
	    (DLGPROC)Debugger)) == NULL)
		AbortMessage(_T("Debugger Dialog Box Creation Error !"));
	return 0;
}


//################
//#
//#    Find dialog box
//#
//################

static __inline BOOL OnFindOK(HWND hDlg,BOOL bASCII,DWORD *pdwAddrLast)
{
	BYTE  *lpbySearch;
	INT   i,j;
	DWORD dwAddr;
	BOOL  bMatch;

	HWND hWnd = GetDlgItem(hDlg,IDC_FIND_DATA);

	i = GetWindowTextLength(hWnd) + 1;		// text length incl. EOS
	j = bASCII ? 2 : sizeof(*(LPTSTR)0);	// buffer width

	// allocate search buffer
	if ((lpbySearch = HeapAlloc(hHeap,0,i * j)) != NULL)
	{
		// get search text and real length
		i = GetWindowText(hWnd,(LPTSTR) lpbySearch,i);

		// add string to combo box
		if (SendMessage(hWnd,CB_FINDSTRINGEXACT,0,(LPARAM) lpbySearch) == CB_ERR)
			SendMessage(hWnd,CB_ADDSTRING,0,(LPARAM) lpbySearch);

		#if defined _UNICODE
		{
			// Unicode to byte translation
			LPTSTR szTmp = DuplicateString((LPTSTR) lpbySearch);
			if (szTmp != NULL)
			{
				WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK,
									szTmp, -1,
									lpbySearch, i+1, NULL, NULL);
				HeapFree(hHeap,0,szTmp);
			}
		}
		#endif

		// convert input format to nibble based format
		if (bASCII)							// ASCII input
		{
			// convert ASCII to number
			for (j = i - 1; j >= 0; --j)
			{
				// order LSB, MSB
				lpbySearch[j * 2 + 1] = lpbySearch[j] >> 4;
				lpbySearch[j * 2]     = lpbySearch[j] & 0xF;
			}
			i *= 2;							// no. of nibbles
		}
		else								// hex number input
		{
			// convert HEX to number
			for (i = 0, j = 0; lpbySearch[j] != 0; ++j)
			{
				if (lpbySearch[j] == ' ')	// skip space
					continue;

				if (isxdigit(lpbySearch[j]))
				{
					lpbySearch[i] = toupper(lpbySearch[j]) - '0';
					if (lpbySearch[i] > 9) lpbySearch[i] -= 'A' - '9' - 1;
				}
				else
				{
					i = 0;					// wrong format, no match
					break;
				}
				++i;						// inc, no. of nibbles
			}
		}

		bMatch = FALSE;						// no match
		dwAddr = dwAdrMem;					// calculate search start address
		if (*pdwAddrLast == dwAddr)
			++dwAddr;

		// scan mapping/module until match
		for (; i && !bMatch && dwAddr <= dwAdrMem + dwMapDataSize; ++dwAddr)
		{
			BYTE byC;

			// i = no. of nibbles that have to match
			for (bMatch = TRUE, j = 0;bMatch && j < i; ++j)
			{
				if (ID_DEBUG_MEM_MAP == uIDMap)	// mapped memory content
				{
					Npeek(&byC,(dwAddr + j) & 0xFFFFF,1);
				}
				else						// module memory content
				{
					byC = lbyMapData[(dwAddr + j) & (dwMapDataSize - 1)];
				}
				bMatch = (byC == lpbySearch[j]);
			}
		}
		dwAddr = (dwAddr - 1) & (dwMapDataSize - 1); // possible matching address
		HeapFree(hHeap,0,lpbySearch);

		// check match result
		if (bMatch)
		{
			OnMemGoDx(GetParent(hDlg),dwAddr);
			*pdwAddrLast = dwAddr;			// latest written address
		}
		else
		{
			MessageBox(hDlg,_T("Search string not found!"),_T("Find"),
					   MB_APPLMODAL|MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND);
		}
	}
	return TRUE;
}

//
// enter find dialog
//
static INT_PTR CALLBACK Find(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DWORD dwAddrEntry;
	static BOOL  bASCII = FALSE;

	switch (message)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg,IDC_FIND_ASCII,bASCII);
		dwAddrEntry = -1;
		return TRUE;

	case WM_DESTROY:
		hDlgFind = NULL;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_FIND_ASCII: bASCII = !bASCII; return TRUE;
		case IDOK:           return OnFindOK(hDlg,bASCII,&dwAddrEntry);
		case IDCANCEL:       DestroyWindow(hDlg); return TRUE;
		}
		break;
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
}

static BOOL OnMemFind(HWND hDlg)
{
	if (hDlgFind == NULL)					// no find dialog, create it
	{
		if ((hDlgFind = CreateDialog(hApp,MAKEINTRESOURCE(IDD_FIND),hDlg,
			(DLGPROC)Find)) == NULL)
			AbortMessage(_T("Find Dialog Box Creation Error !"));
	}
	else
	{
		SetFocus(hDlgFind);					// set focus on find dialog
	}
	return -1;								// call windows default handler
}


//################
//#
//#    Profile dialog box
//#
//################

//
// update profiler dialog content
//
static VOID UpdateProfileWnd(HWND hDlg)
{
	#define CPU_FREQ 524288					// base CPU frequency
	#define GP_RATE  0x1B*3 // CdB for HP: add high speed apples
	#define G2_RATE  0x1B*2 // CdB for HP: add low speed apples
	#define SX_RATE  0x0E
	#define GX_RATE  0x1B

	LPCTSTR pcUnit[] = { _T("s"),_T("ms"),_T("us"),_T("ns") };

	QWORD lVar;
	TCHAR szBuffer[64];
	INT   i;
	DWORD dwFreq, dwEndFreq;

	if (hDlg == NULL) return;				// dialog not open

	// 64 bit cpu cycle counter
	lVar = *(QWORD *)&Chipset.cycles - *(QWORD *)&OldChipset.cycles;

	// cycle counts
	_sntprintf(szBuffer,ARRAYSIZEOF(szBuffer),_T("%I64u"),lVar);
	SetDlgItemText(hDlg,IDC_PROFILE_LASTCYCLES,szBuffer);
	switch (cCurrentRomType) // CdB for HP: add apples speed selection
	{
	  case 'S': dwFreq= ((SX_RATE + 1) * CPU_FREQ / 4); break;
	  case 'X': case 'G': case 'E': case 'A': dwFreq= ((GX_RATE + 1) * CPU_FREQ / 4); break;
	  case 'P': case 'Q': dwFreq= ((GP_RATE + 1) * CPU_FREQ / 4); break;
	  case '2': dwFreq= ((G2_RATE + 1) * CPU_FREQ / 4); break;
	}
		   ? ((SX_RATE + 1) * CPU_FREQ / 4)
		   : ((GX_RATE + 1) * CPU_FREQ / 4);
	dwEndFreq = ((999 * 2 - 1) * dwFreq) / (2 * 1000);

	// search for unit
	for (i = 0; i < ARRAYSIZEOF(pcUnit) - 1; ++i)
	{
		if (lVar > dwEndFreq) break;		// found ENG unit
		lVar *= 1000;						// next ENG unit
	}

	// calculate rounded time
	lVar = (2 * lVar + dwFreq) / (2 * dwFreq);

	_ASSERT(i < ARRAYSIZEOF(pcUnit));
	_sntprintf(szBuffer,ARRAYSIZEOF(szBuffer),_T("%I64u %s"),lVar,pcUnit[i]);
	SetDlgItemText(hDlg,IDC_PROFILE_LASTTIME,szBuffer);
	#undef GP_RATE
	#undef G2_RATE
	return;
	#undef SX_CLK
	#undef GX_CLK
	#undef CPU_FREQ
}

//
// enter profile dialog
//
static INT_PTR CALLBACK Profile(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		UpdateProfileWnd(hDlg);
		return TRUE;

	case WM_DESTROY:
		hDlgProfile = NULL;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL: DestroyWindow(hDlg); return TRUE;
		}
		break;
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
}

static BOOL OnProfile(HWND hDlg)
{
	if (hDlgProfile == NULL)				// no profile dialog, create it
	{
		if ((hDlgProfile = CreateDialog(hApp,MAKEINTRESOURCE(IDD_PROFILE),hDlg,
			(DLGPROC)Profile)) == NULL)
			AbortMessage(_T("Profile Dialog Box Creation Error !"));
	}
	else
	{
		SetFocus(hDlgProfile);				// set focus on profile dialog
	}
	return -1;								// call windows default handler
}

//################
//#
//#    New Value dialog box
//#
//################

//
// enter new value dialog
//
static INT_PTR CALLBACK NewValue(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LPTSTR lpszBuffer;				// handle of buffer
	static int    nBufferlen;				// length of buffer

	HWND  hWnd;
	TCHAR szBuffer[64];
	LONG  i;

	switch (message)
	{
	case WM_INITDIALOG:
		lpszBuffer = (LPTSTR) lParam;
		// length with zero string terminator
		nBufferlen = lstrlen(lpszBuffer)+1;
		_ASSERT(ARRAYSIZEOF(szBuffer) >= nBufferlen);
		SetDlgItemText(hDlg,IDC_NEWVALUE,lpszBuffer);
		return TRUE;
	case WM_COMMAND:
		wParam = LOWORD(wParam);
		switch(wParam)
		{
		case IDOK:
			hWnd = GetDlgItem(hDlg,IDC_NEWVALUE);
			GetWindowText(hWnd,szBuffer,nBufferlen);
			// test if valid hex address
			for (i = 0; i < (LONG) lstrlen(szBuffer); ++i)
			{
				if (_istxdigit(szBuffer[i]) == 0)
				{
					SendMessage(hWnd,EM_SETSEL,0,-1);
					SetFocus(hWnd);			// focus to edit control
					return FALSE;
				}
			}
			lstrcpy(lpszBuffer,szBuffer);	// copy valid value
			// no break
		case IDCANCEL:
			EndDialog(hDlg,wParam);
			return TRUE;
		}
	}
	return FALSE;
    UNREFERENCED_PARAMETER(wParam);
}

static INT_PTR OnNewValue(LPTSTR lpszValue)
{
	INT_PTR nResult;

	if ((nResult = DialogBoxParam(hApp,
								  MAKEINTRESOURCE(IDD_NEWVALUE),
								  hDlgDebug,
								  (DLGPROC)NewValue,
								  (LPARAM)lpszValue)
								 ) == -1)
		AbortMessage(_T("Input Dialog Box Creation Error !"));
	return nResult;
}


//################
//#
//#    Goto Address dialog box
//#
//################

//
// enter goto address dialog
//
static INT_PTR CALLBACK EnterAddr(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DWORD *dwAddress;

	HWND  hWnd;
	TCHAR szBuffer[8];
	LONG  i;

	switch (message)
	{
	case WM_INITDIALOG:
		dwAddress = (DWORD *) lParam;
		return TRUE;
	case WM_COMMAND:
		wParam = LOWORD(wParam);
		switch(wParam)
		{
		case IDOK:
			hWnd = GetDlgItem(hDlg,IDC_ENTERADR);
			GetWindowText(hWnd,szBuffer,ARRAYSIZEOF(szBuffer));
			// test if valid hex address
			for (i = 0; i < (LONG) lstrlen(szBuffer); ++i)
			{
				if (_istxdigit(szBuffer[i]) == 0)
				{
					SendMessage(hWnd,EM_SETSEL,0,-1);
					SetFocus(hWnd);			// focus to edit control
					return FALSE;
				}
			}
			if (*szBuffer) _stscanf(szBuffer,_T("%6X"),dwAddress);
			// no break
		case IDCANCEL:
			EndDialog(hDlg,wParam);
			return TRUE;
		}
	}
	return FALSE;
    UNREFERENCED_PARAMETER(wParam);
}

static VOID OnEnterAddress(HWND hDlg, DWORD *dwValue)
{
	if (DialogBoxParam(hApp, MAKEINTRESOURCE(IDD_ENTERADR), hDlg, (DLGPROC)EnterAddr, (LPARAM)dwValue) == -1)
		AbortMessage(_T("Address Dialog Box Creation Error !"));
}


//################
//#
//#    Breakpoint dialog box
//#
//################

//
// enter breakpoint dialog
//
static INT_PTR CALLBACK EnterBreakpoint(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BP_T *sBp;

	HWND  hWnd;
	TCHAR szBuffer[8];
	LONG  i;

	switch (message)
	{
	case WM_INITDIALOG:
		sBp = (BP_T *) lParam;
		sBp->bEnable = TRUE;
		sBp->nType   = BP_EXEC;
		SendDlgItemMessage(hDlg,IDC_BPCODE,BM_SETCHECK,1,0);
		return TRUE;
	case WM_COMMAND:
		wParam = LOWORD(wParam);
		switch(wParam)
		{
		case IDC_BPCODE:   sBp->nType = BP_EXEC;   return TRUE;
		case IDC_BPRPL:    sBp->nType = BP_RPL;    return TRUE;
		case IDC_BPACCESS: sBp->nType = BP_ACCESS; return TRUE;
		case IDC_BPREAD:   sBp->nType = BP_READ;   return TRUE;
		case IDC_BPWRITE:  sBp->nType = BP_WRITE;  return TRUE;
		case IDOK:
			hWnd = GetDlgItem(hDlg,IDC_ENTERADR);
			GetWindowText(hWnd,szBuffer,ARRAYSIZEOF(szBuffer));
			// test if valid hex address
			for (i = 0; i < (LONG) lstrlen(szBuffer); ++i)
			{
				if (_istxdigit(szBuffer[i]) == 0)
				{
					SendMessage(hWnd,EM_SETSEL,0,-1);
					SetFocus(hWnd);
					return FALSE;
				}
			}
			if (*szBuffer) _stscanf(szBuffer,_T("%5X"),&sBp->dwAddr);
			// no break
		case IDCANCEL:
			EndDialog(hDlg,wParam);
			return TRUE;
		}
	}
	return FALSE;
    UNREFERENCED_PARAMETER(wParam);
}

static VOID OnEnterBreakpoint(HWND hDlg, BP_T *sValue)
{
	if (DialogBoxParam(hApp, MAKEINTRESOURCE(IDD_ENTERBREAK), hDlg, (DLGPROC)EnterBreakpoint, (LPARAM)sValue) == -1)
		AbortMessage(_T("Breakpoint Dialog Box Creation Error !"));
}


//################
//#
//#    Edit breakpoint dialog box
//#
//################

//
// handle drawing in breakpoint window
//
static __inline BOOL OnDrawBreakWnd(LPDRAWITEMSTRUCT lpdis)
{
	TCHAR    szBuf[64];
	COLORREF crBkColor,crTextColor;
	HDC      hdcMem;
	HBITMAP  hBmpOld;
	INT      i;

	if (lpdis->itemID == -1)				// no item in list box
		return TRUE;

	if (lpdis->itemState & ODS_SELECTED)	// cursor line
	{
		crBkColor   = COLOR_NAVY;
		crTextColor = COLOR_WHITE;
	}
	else
	{
		crBkColor   = COLOR_WHITE;
		crTextColor = COLOR_BLACK;
	}

	// write Text
	crBkColor   = SetBkColor(lpdis->hDC,crBkColor);
	crTextColor = SetTextColor(lpdis->hDC,crTextColor);

	SendMessage(lpdis->hwndItem,LB_GETTEXT,lpdis->itemID,(LPARAM) szBuf);
	ExtTextOut(lpdis->hDC,(int)(lpdis->rcItem.left)+17,(int)(lpdis->rcItem.top),
	           ETO_OPAQUE,(LPRECT)&lpdis->rcItem,szBuf,lstrlen(szBuf),NULL);

	SetBkColor(lpdis->hDC,crBkColor);
	SetTextColor(lpdis->hDC,crTextColor);

	// draw checkbox
	i = (INT) SendMessage(lpdis->hwndItem,LB_GETITEMDATA,lpdis->itemID,0);
	hdcMem = CreateCompatibleDC(lpdis->hDC);
	_ASSERT(hBmpCheckBox);
	hBmpOld = SelectObject(hdcMem,hBmpCheckBox);

	BitBlt(lpdis->hDC,lpdis->rcItem.left+2,lpdis->rcItem.top+2,
		   11,lpdis->rcItem.bottom - lpdis->rcItem.top,
		   hdcMem,sBreakpoint[i].bEnable ? 0 : 10,0,SRCCOPY);

	SelectObject(hdcMem,hBmpOld);
	DeleteDC(hdcMem);

	if (lpdis->itemState & ODS_FOCUS)		// redraw focus
		DrawFocusRect(lpdis->hDC,&lpdis->rcItem);

	return TRUE;							// focus handled here
}

//
// toggle breakpoint drawing
//
static BOOL ToggleBreakpointItem(HWND hWnd, INT nItem)
{
	RECT rc;

	// get breakpoint number
	INT i = (INT) SendMessage(hWnd,LB_GETITEMDATA,nItem,0);

	sBreakpoint[i].bEnable = !sBreakpoint[i].bEnable;
	// update region of toggled item
	SendMessage(hWnd,LB_GETITEMRECT,nItem,(LPARAM)&rc);
	InvalidateRect(hWnd,&rc,TRUE);
	return TRUE;
}

//
// draw breakpoint type
//
static VOID DrawBreakpoint(HWND hWnd, INT i)
{
	TCHAR *szText,szBuffer[32];
	INT   nItem;

	switch(sBreakpoint[i].nType)
	{
	case BP_EXEC:   // code breakpoint
		szText = _T("Code");
		break;
	case BP_RPL:   // RPL breakpoint
		szText = _T("RPL");
		break;
	case BP_READ:   // read memory breakpoint
		szText = _T("Memory Read");
		break;
	case BP_WRITE:  // write memory breakpoint
		szText = _T("Memory Write");
		break;
	case BP_ACCESS: // memory breakpoint
		szText = _T("Memory Access");
		break;
	default:        // unknown breakpoint type
		szText = _T("unknown");
		_ASSERT(0);
	}
	wsprintf(szBuffer,_T("%05X (%s)"),sBreakpoint[i].dwAddr,szText);
	nItem = (INT) SendMessage(hWnd,LB_ADDSTRING,0,(LPARAM) szBuffer);
	SendMessage(hWnd,LB_SETITEMDATA,nItem,i);
	return;
}

//
// enter edit breakpoint dialog
//
static INT_PTR CALLBACK EditBreakpoint(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TEXTMETRIC tm;

	HWND   hWnd;
	HDC    hDC;
	HFONT  hFont;
	BP_T   sBp;
	INT    i,nItem;

	switch (message)
	{
	case WM_INITDIALOG:
		// font settings
		SendDlgItemMessage(hDlg,IDC_STATIC_BREAKPOINT,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_BREAKEDIT_ADD,    WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_BREAKEDIT_DELETE, WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDCANCEL,             WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));

		hBmpCheckBox = LoadBitmap(hApp,MAKEINTRESOURCE(IDB_CHECKBOX));
		_ASSERT(hBmpCheckBox);

		hWnd = GetDlgItem(hDlg,IDC_BREAKEDIT_WND);
		SendMessage(hWnd,WM_SETREDRAW,FALSE,0);
		SendMessage(hWnd,LB_RESETCONTENT,0,0);
		for (i = 0; i < wBreakpointCount; ++i)
			DrawBreakpoint(hWnd,i);
		SendMessage(hWnd,WM_SETREDRAW,TRUE,0);
		return TRUE;

	case WM_DESTROY:
		DeleteObject(hBmpCheckBox);
		return TRUE;

	case WM_COMMAND:
		hWnd = GetDlgItem(hDlg,IDC_BREAKEDIT_WND);
		switch (HIWORD(wParam))
		{
		case LBN_DBLCLK:
			if (LOWORD(wParam) == IDC_BREAKEDIT_WND)
			{
				if ((nItem = (INT) SendMessage(hWnd,LB_GETCURSEL,0,0)) == LB_ERR)
					return FALSE;

				return ToggleBreakpointItem(hWnd,nItem);
			}
		}

		switch(LOWORD(wParam))
		{
		case IDC_BREAKEDIT_ADD:
			sBp.dwAddr = -1;				// no breakpoint given
			OnEnterBreakpoint(hDlg, &sBp);
			if (sBp.dwAddr != -1)
			{
				for (i = 0; i < wBreakpointCount; ++i)
				{
					if (sBreakpoint[i].dwAddr == sBp.dwAddr)
					{
						// tried to add used code breakpoint
						if (sBreakpoint[i].bEnable && (sBreakpoint[i].nType & sBp.nType & (BP_EXEC | BP_RPL)) != 0)
							return FALSE;

						// only modify memory breakpoints
						if (   (   sBreakpoint[i].bEnable == FALSE
							    && (sBreakpoint[i].nType & sBp.nType & (BP_EXEC | BP_RPL)) != 0)
							|| ((sBreakpoint[i].nType & BP_ACCESS) && (sBp.nType & BP_ACCESS)))
						{
							// replace breakpoint type
							sBreakpoint[i].bEnable = TRUE;
							sBreakpoint[i].nType   = sBp.nType;

							// redaw breakpoint list
							SendMessage(hWnd,WM_SETREDRAW,FALSE,0);
							SendMessage(hWnd,LB_RESETCONTENT,0,0);
							for (i = 0; i < wBreakpointCount; ++i)
								DrawBreakpoint(hWnd,i);
							SendMessage(hWnd,WM_SETREDRAW,TRUE,0);
							return FALSE;
						}
					}
				}

				// check for breakpoint buffer full
				if (wBreakpointCount >= MAXBREAKPOINTS)
				{
					AbortMessage(_T("Reached maximum number of breakpoints !"));
					return FALSE;
				}

				sBreakpoint[wBreakpointCount].bEnable = sBp.bEnable;
				sBreakpoint[wBreakpointCount].nType   = sBp.nType;
				sBreakpoint[wBreakpointCount].dwAddr  = sBp.dwAddr;

				DrawBreakpoint(hWnd,wBreakpointCount);

				++wBreakpointCount;
			}
			return TRUE;

		case IDC_BREAKEDIT_DELETE:
			// scan all breakpoints from top
			for (nItem = wBreakpointCount-1; nItem >= 0; --nItem)
			{
				// item selected
				if (SendMessage(hWnd,LB_GETSEL,nItem,0) > 0)
				{
					INT j;

					// get breakpoint index
					i = (INT) SendMessage(hWnd,LB_GETITEMDATA,nItem,0);
					SendMessage(hWnd,LB_DELETESTRING,nItem,0);
					--wBreakpointCount;

					// update remaining list box references
					for (j = 0; j < wBreakpointCount; ++j)
					{
						INT k = (INT) SendMessage(hWnd,LB_GETITEMDATA,j,0);
						if (k > i) SendMessage(hWnd,LB_SETITEMDATA,j,k-1);
					}

					// remove breakpoint from breakpoint table
					for (++i; i <= wBreakpointCount; ++i)
						sBreakpoint[i-1] = sBreakpoint[i];
				}
			}
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg,IDCANCEL);
			return TRUE;
		}

	case WM_VKEYTOITEM:
		if (LOWORD(wParam) == VK_SPACE)
		{
			hWnd = GetDlgItem(hDlg,IDC_BREAKEDIT_WND);
			for (nItem = 0; nItem < wBreakpointCount; ++nItem)
			{
				// item selected
				if (SendMessage(hWnd,LB_GETSEL,nItem,0) > 0)
					ToggleBreakpointItem(hWnd,nItem);
			}
			return -2;
		}
		return -1;							// default action

	case WM_DRAWITEM:
		if (wParam == IDC_BREAKEDIT_WND) return OnDrawBreakWnd((LPDRAWITEMSTRUCT) lParam);
		break;

	case WM_MEASUREITEM:
		hDC = GetDC(hDlg);

		// GetTextMetrics from "Courier New 8" font
		hFont = CreateFont(-MulDiv(8,GetDeviceCaps(hDC, LOGPIXELSY),72),0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,
		                   OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE,_T("Courier New"));

		hFont = SelectObject(hDC,hFont);
		GetTextMetrics(hDC,&tm);
		hFont = SelectObject(hDC,hFont);
		DeleteObject(hFont);

		((LPMEASUREITEMSTRUCT) lParam)->itemHeight = tm.tmHeight;

		ReleaseDC(hDlg,hDC);
		return TRUE;
	}
	return FALSE;
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
}

static BOOL OnEditBreakpoint(HWND hDlg)
{
	if (DialogBox(hApp, MAKEINTRESOURCE(IDD_BREAKEDIT), hDlg, (DLGPROC)EditBreakpoint) == -1)
		AbortMessage(_T("Edit Breakpoint Dialog Box Creation Error !"));

	// update code window
	InvalidateRect(GetDlgItem(hDlg,IDC_DEBUG_CODE),NULL,TRUE);
	return -1;
}


//################
//#
//#    Last Instruction dialog box
//#
//################

//
// view last instructions
//
static INT_PTR CALLBACK InfoIntr(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND  hWnd;
	TCHAR szBuffer[64];
	LONG  lIndex;
	WORD  i,j;

	switch (message)
	{
	case WM_INITDIALOG:
		// font settings
		SendDlgItemMessage(hDlg,IDC_INSTR_TEXT,  WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_INSTR_CLEAR, WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_INSTR_COPY,  WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDCANCEL,        WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));

		lIndex = 0;							// init lIndex
		hWnd = GetDlgItem(hDlg,IDC_INSTR_CODE);
		SendMessage(hWnd,WM_SETREDRAW,FALSE,0);
		SendMessage(hWnd,LB_RESETCONTENT,0,0);
		for (i = wInstrRp; i != wInstrWp; i = (i + 1) % wInstrSize)
		{
			j = wsprintf(szBuffer,_T("%05lX   "),pdwInstrArray[i]);
			disassemble(pdwInstrArray[i],&szBuffer[j],VIEW_SHORT);
			lIndex = (LONG) SendMessage(hWnd,LB_ADDSTRING,0,(LPARAM) szBuffer);
		}
		SendMessage(hWnd,WM_SETREDRAW,TRUE,0);
		SendMessage(hWnd,LB_SETCARETINDEX,lIndex,TRUE);
		return TRUE;
	case WM_COMMAND:
		hWnd = GetDlgItem(hDlg,IDC_INSTR_CODE);
		switch(LOWORD(wParam))
		{
		case IDC_INSTR_COPY:
			CopyItemsToClipboard(hWnd);		// copy selected items to clipboard
			return TRUE;
		case IDC_INSTR_CLEAR:				// clear instruction buffer
			wInstrRp = wInstrWp;
			SendMessage(hWnd,LB_RESETCONTENT,0,0);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg,IDCANCEL);
			return TRUE;
		}
	}
	return FALSE;
    UNREFERENCED_PARAMETER(lParam);
}

static BOOL OnInfoIntr(HWND hDlg)
{
	if (DialogBox(hApp, MAKEINTRESOURCE(IDD_INSTRUCTIONS), hDlg, (DLGPROC)InfoIntr) == -1)
		AbortMessage(_T("Last Instructions Dialog Box Creation Error !"));
	return 0;
}

//
// view write only I/O registers
//
static BOOL CALLBACK InfoWoRegister(HWND hDlg, UINT message, DWORD wParam, LONG lParam)
{
	TCHAR szBuffer[8];

	switch (message)
	{
	case WM_INITDIALOG:
		wsprintf(szBuffer,_T("%05X"),Chipset.start1);
		SetDlgItemText(hDlg,IDC_ADDR20_24,szBuffer);
		wsprintf(szBuffer,_T("%03X"),Chipset.loffset);
		SetDlgItemText(hDlg,IDC_ADDR25_27,szBuffer);
		wsprintf(szBuffer,_T("%02X"),Chipset.lcounter);
		SetDlgItemText(hDlg,IDC_ADDR28_29,szBuffer);
		wsprintf(szBuffer,_T("%05X"),Chipset.start2);
		SetDlgItemText(hDlg,IDC_ADDR30_34,szBuffer);
		return TRUE;
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK))
		{
			EndDialog(hDlg,IDOK);
			return TRUE;
		}
	}
	return FALSE;
    UNREFERENCED_PARAMETER(lParam);
}

static BOOL OnInfoWoRegister(HWND hDlg)
{
	if (DialogBox(hApp, MAKEINTRESOURCE(IDD_WRITEONLYREG), hDlg, (DLGPROC)InfoWoRegister) == -1)
		AbortMessage(_T("Write-Only Register Dialog Box Creation Error !"));
	return 0;
}


//################
//#
//#    File operations
//#
//################

//
// load breakpoint list
//
VOID LoadBreakpointList(HANDLE hFile)		// NULL = clear breakpoint list
{
	DWORD lBytesRead = 0;

	// read number of breakpoints
	if (hFile) ReadFile(hFile, &wBreakpointCount, sizeof(wBreakpointCount), &lBytesRead, NULL);
	if (lBytesRead)							// breakpoints found
	{
		WORD wBreakpointSize;

		// read size of one breakpoint
		ReadFile(hFile, &wBreakpointSize, sizeof(wBreakpointSize), &lBytesRead, NULL);
		if (lBytesRead == sizeof(wBreakpointSize) && wBreakpointSize == sizeof(sBreakpoint[0]))
		{
			// read breakpoints
			ReadFile(hFile, sBreakpoint, wBreakpointCount * sizeof(sBreakpoint[0]), &lBytesRead, NULL);
			_ASSERT(lBytesRead == wBreakpointCount * sizeof(sBreakpoint[0]));
		}
		else								// changed breakpoint structure
		{
			wBreakpointCount = 0;			// clear breakpoint list
		}
	}
	else
	{
		wBreakpointCount = 0;				// clear breakpoint list
	}
	return;
}

//
// save breakpoint list
//
VOID SaveBreakpointList(HANDLE hFile)
{
	if (wBreakpointCount)					// defined breakpoints
	{
		DWORD lBytesWritten;

		WORD wBreakpointSize = sizeof(sBreakpoint[0]);

		_ASSERT(hFile);						// valid file pointer?

		// write number of breakpoints
		WriteFile(hFile, &wBreakpointCount, sizeof(wBreakpointCount), &lBytesWritten, NULL);
		_ASSERT(lBytesWritten == sizeof(wBreakpointCount));

		// write size of one breakpoint
		WriteFile(hFile, &wBreakpointSize, sizeof(wBreakpointSize), &lBytesWritten, NULL);
		_ASSERT(lBytesWritten == sizeof(wBreakpointSize));

		// write breakpoints
		WriteFile(hFile, sBreakpoint, wBreakpointCount * sizeof(sBreakpoint[0]), &lBytesWritten, NULL);
		_ASSERT(lBytesWritten == wBreakpointCount * sizeof(sBreakpoint[0]));
	}
	return;
}
