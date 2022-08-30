/*
 *   Emu48.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */
#include "pch.h"
#include "resource.h"
#include "emu48.h"
#include "io.h"
#include "kml.h"
#include "debugger.h"

#define VERSION   "1.47"

// #define MONOCHROME						// CF_BITMAP clipboard format

#ifdef _DEBUG
LPCTSTR szNoTitle = _T("Emu48 ")_T(VERSION)_T(" Debug");
#else
LPCTSTR szNoTitle = _T("Emu48 ")_T(VERSION);
#endif
LPTSTR szAppName = _T("Emu48");				// application name for DDE server
LPTSTR szTopic   = _T("Stack");				// topic for DDE server
LPTSTR szTitle   = NULL;

static BOOL bOwnCursor = FALSE;

static const LPCTSTR szLicence =
	_T("This program is free software; you can redistribute it and/or modify\r\n")
	_T("it under the terms of the GNU General Public License as published by\r\n")
	_T("the Free Software Foundation; either version 2 of the License, or\r\n")
	_T("(at your option) any later version.\r\n")
	_T("\r\n")
	_T("This program is distributed in the hope that it will be useful,\r\n")
	_T("but WITHOUT ANY WARRANTY; without even the implied warranty of\r\n")
	_T("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\r\n")
	_T("See the GNU General Public License for more details.\r\n")
	_T("\r\n")
	_T("You should have received a copy of the GNU General Public License\r\n")
	_T("along with this program; if not, write to the Free Software Foundation,\r\n")
	_T("Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA");


CRITICAL_SECTION csGDILock;					// critical section for hWindowDC
CRITICAL_SECTION csLcdLock;					// critical section for display update
CRITICAL_SECTION csKeyLock;					// critical section for key scan
CRITICAL_SECTION csIOLock;					// critical section for I/O access
CRITICAL_SECTION csT1Lock;					// critical section for timer1 access
CRITICAL_SECTION csT2Lock;					// critical section for timer2 access
CRITICAL_SECTION csTxdLock;					// critical section for transmit byte
CRITICAL_SECTION csRecvLock;				// critical section for receive byte
CRITICAL_SECTION csSlowLock;				// critical section for speed slow down
INT              nArgc;						// no. of command line arguments
LPCTSTR          *ppArgv;					// command line arguments
LARGE_INTEGER    lFreq;						// high performance counter frequency
LARGE_INTEGER    lAppStart;					// high performance counter value at Appl. start
DWORD            idDdeInst;					// DDE server id
UINT             uCF_HpObj;					// DDE clipboard format
HANDLE           hHeap;
HANDLE           hThread;
DWORD            lThreadId;
HANDLE           hEventShutdn;				// event handle to stop cpu thread

HINSTANCE        hApp = NULL;
HWND             hWnd = NULL;
HWND             hDlgDebug = NULL;			// handle for debugger dialog
HWND             hDlgFind = NULL;			// handle for debugger find dialog
HWND             hDlgProfile = NULL;		// handle for debugger profile dialog
HDC              hWindowDC = NULL;
HPALETTE         hPalette = NULL;
HPALETTE         hOldPalette = NULL;		// old palette of hWindowDC
HCURSOR          hCursorArrow = NULL;
HCURSOR          hCursorHand = NULL;
BOOL             bAutoSave = FALSE;
BOOL             bAutoSaveOnExit = TRUE;
BOOL             bSaveDefConfirm = TRUE;	// yes
BOOL             bStartupBackup = FALSE;
BOOL             bAlwaysDisplayLog = TRUE;
BOOL             bLoadObjectWarning = TRUE;
BOOL             bAlwaysOnTop = FALSE;		// emulator window always on top
BOOL             bActFollowsMouse = FALSE;	// emulator window activation follows mouse



//################
//#
//#    Window Status
//#
//################

VOID SetWindowTitle(LPCTSTR szString)
{
	if (szTitle) HeapFree(hHeap,0,szTitle);

	_ASSERT(hWnd != NULL);
	if (szString)
	{
		szTitle = DuplicateString(szString);
		SetWindowText(hWnd, szTitle);
	}
	else
	{
		szTitle = NULL;
		SetWindowText(hWnd, szNoTitle);
	}
	return;
}

VOID UpdateWindowStatus(VOID)
{
	if (hWnd)								// window open
	{
		// disable stack loading items on HP38G, HP39/40G, HP39G+
		BOOL bStackEnable = cCurrentRomType!='6' && cCurrentRomType!='A' && cCurrentRomType!='E' && cCurrentRomType!='P';  // CdB for HP: add apples
		BOOL bRun         = nState == SM_RUN || nState == SM_SLEEP;

		UINT uStackEnable = (bRun && bStackEnable) ? MF_ENABLED : MF_GRAYED;
		UINT uRun         = bRun                   ? MF_ENABLED : MF_GRAYED;
		UINT uBackup      = bBackup                ? MF_ENABLED : MF_GRAYED;

		HMENU hMenu = GetMenu(hWnd);		// get menu handle

		EnableMenuItem(hMenu,ID_FILE_NEW,MF_ENABLED);
		EnableMenuItem(hMenu,ID_FILE_OPEN,MF_ENABLED);
		EnableMenuItem(hMenu,ID_FILE_SAVE,(bRun && szCurrentFilename[0]) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu,ID_FILE_SAVEAS,uRun);
		EnableMenuItem(hMenu,ID_FILE_CLOSE,uRun);
		EnableMenuItem(hMenu,ID_OBJECT_LOAD,uStackEnable);
		EnableMenuItem(hMenu,ID_OBJECT_SAVE,uStackEnable);
		EnableMenuItem(hMenu,ID_VIEW_COPY,uRun);
		EnableMenuItem(hMenu,ID_STACK_COPY,uStackEnable);
		EnableMenuItem(hMenu,ID_STACK_PASTE,uStackEnable);
		EnableMenuItem(hMenu,ID_VIEW_RESET,uRun);
		EnableMenuItem(hMenu,ID_BACKUP_SAVE,uRun);
		EnableMenuItem(hMenu,ID_BACKUP_RESTORE,uBackup);
		EnableMenuItem(hMenu,ID_BACKUP_DELETE,uBackup);
		EnableMenuItem(hMenu,ID_VIEW_SCRIPT,uRun);
		EnableMenuItem(hMenu,ID_TOOL_DISASM,uRun);
		EnableMenuItem(hMenu,ID_TOOL_DEBUG,(bRun && nDbgState == DBG_OFF) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu,ID_TOOL_MACRO_RECORD,(bRun && nMacroState == MACRO_OFF) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu,ID_TOOL_MACRO_PLAY,(bRun && nMacroState == MACRO_OFF) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu,ID_TOOL_MACRO_STOP,(bRun && nMacroState != MACRO_OFF) ? MF_ENABLED : MF_GRAYED);
	}
	return;
}

VOID ForceForegroundWindow(HWND hWnd)
{
	// force window to foreground
	DWORD dwEmuThreadID = GetCurrentThreadId();
	DWORD dwActThreadID = GetWindowThreadProcessId(GetForegroundWindow(),NULL);

	AttachThreadInput(dwEmuThreadID,dwActThreadID,TRUE);
	SetForegroundWindow(hWnd);
	AttachThreadInput(dwEmuThreadID,dwActThreadID,FALSE);
	return;
}



//################
//#
//#    Clipboard Tool
//#
//################

VOID CopyItemsToClipboard(HWND hWnd)		// save selected Listbox Items to Clipboard
{
	LONG  i;
	LPINT lpnCount;

	// get number of selections
	if ((i = (LONG) SendMessage(hWnd,LB_GETSELCOUNT,0,0)) == 0)
		return;								// no items selected

	if ((lpnCount = HeapAlloc(hHeap,0,i * sizeof(INT))) != NULL)
	{
		LPTSTR lpszData;
		HANDLE hClipObj;
		LONG j,lMem = 0;

		// get indexes of selected items
		i = (LONG) SendMessage(hWnd,LB_GETSELITEMS,i,(LPARAM) lpnCount);
		for (j = 0;j < i;++j)				// scan all selected items
		{
			// calculate total amount of characters
			lMem += (LONG) SendMessage(hWnd,LB_GETTEXTLEN,lpnCount[j],0) + 2;
		}
		// allocate clipboard data
		if ((hClipObj = GlobalAlloc(GMEM_MOVEABLE,(lMem + 1) * sizeof(*lpszData))) != NULL)
		{
			if ((lpszData = GlobalLock(hClipObj)))
			{
				for (j = 0;j < i;++j)		// scan all selected items
				{
					lpszData += SendMessage(hWnd,LB_GETTEXT,lpnCount[j],(LPARAM) lpszData);
					*lpszData++ = _T('\r');
					*lpszData++ = _T('\n');
				}
				*lpszData = 0;				// set EOS
				GlobalUnlock(hClipObj);		// unlock memory
			}

			if (OpenClipboard(hWnd))
			{
				if (EmptyClipboard())
					#if defined _UNICODE
						SetClipboardData(CF_UNICODETEXT,hClipObj);
					#else
						SetClipboardData(CF_TEXT,hClipObj);
					#endif
				else
					GlobalFree(hClipObj);
				CloseClipboard();
			}
			else							// clipboard open failed
			{
				GlobalFree(hClipObj);
			}
		}
		HeapFree(hHeap,0,lpnCount);			// free item table
	}
	return;
}



//################
//#
//#    Settings
//#
//################

// get R/W state of file
static BOOL IsFileWriteable(LPCTSTR szFilename)
{
	DWORD dwFileAtt;

	BOOL bWriteable = FALSE;

	SetCurrentDirectory(szEmuDirectory);
	dwFileAtt = GetFileAttributes(szFilename);
	if (dwFileAtt != 0xFFFFFFFF)
		bWriteable = ((dwFileAtt & FILE_ATTRIBUTE_READONLY) == 0);
	SetCurrentDirectory(szCurrentDirectory);
	return bWriteable;
}

// set listfield for serial combo boxes
static VOID SetCommList(HWND hDlg,LPCTSTR szWireSetting,LPCTSTR szIrSetting)
{
	WPARAM wSelectWire,wSelectIr;
	HKEY   hKey;

	wSelectWire = wSelectIr = 0;			// set selections to disabled
	SendDlgItemMessage(hDlg,IDC_WIRE,CB_ADDSTRING,0,(LPARAM) _T(NO_SERIAL));
	SendDlgItemMessage(hDlg,IDC_IR  ,CB_ADDSTRING,0,(LPARAM) _T(NO_SERIAL));

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					 _T("Hardware\\DeviceMap\\SerialComm"),
					 0,
					 KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
					 &hKey) == ERROR_SUCCESS)
	{
		HANDLE hComm;
		TCHAR  szBuffer[256],cKey[256],cData[256];
		DWORD  dwKeySize,dwDataSize;
		WPARAM wSelIndexWire,wSelIndexIr;
		BOOL   bAddWire,bAddIr;
		DWORD  dwType,dwEnumVal;
		LONG   lRet;

		wSelIndexWire = wSelIndexIr = 1;	// preset selector

		dwEnumVal = 0;
		do
		{
			dwKeySize = ARRAYSIZEOF(cKey);	// init return buffer sizes
			dwDataSize = sizeof(cData);

			lRet = RegEnumValue(hKey,dwEnumVal++,
								cKey,&dwKeySize,
								NULL,&dwType,
								(LPBYTE) cData,&dwDataSize);

			if (lRet == ERROR_SUCCESS && dwType == REG_SZ)
			{
				wsprintf(szBuffer,_T("\\\\.\\%s"),cData);
				if ((bAddWire = (lstrcmp(&szBuffer[4],szWireSetting) == 0)))
					wSelectWire = wSelIndexWire;
				if ((bAddIr = (lstrcmp(&szBuffer[4],szIrSetting) == 0)))
					wSelectIr = wSelIndexIr;

				// test if COM port is valid
				hComm = CreateFile(szBuffer,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
				if (hComm != INVALID_HANDLE_VALUE)
				{
					VERIFY(CloseHandle(hComm));
					bAddWire = bAddIr = TRUE;
				}

				if (bAddWire)				// add item to wire combobox
				{
					SendDlgItemMessage(hDlg,IDC_WIRE,CB_ADDSTRING,0,(LPARAM) &szBuffer[4]);
					++wSelIndexWire;
				}
				if (bAddIr)					// add item to ir combobox
				{
					SendDlgItemMessage(hDlg,IDC_IR,CB_ADDSTRING,0,(LPARAM) &szBuffer[4]);
					++wSelIndexIr;
				}
			}
		}
		while (lRet == ERROR_SUCCESS);
		_ASSERT(lRet == ERROR_NO_MORE_ITEMS);
		RegCloseKey(hKey);
	}

	// set cursors
	SendDlgItemMessage(hDlg,IDC_WIRE,CB_SETCURSEL,wSelectWire,0L);
	SendDlgItemMessage(hDlg,IDC_IR  ,CB_SETCURSEL,wSelectIr  ,0L);
	return;
}

static INT_PTR CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndInsertAfter;

	LPCTSTR szActPort2Filename = _T("");

	BOOL bPort2CfgChange = FALSE;
	BOOL bPort2AttChange = FALSE;

	switch (message)
	{
	case WM_INITDIALOG:
		// init speed checkbox
		CheckDlgButton(hDlg,IDC_REALSPEED,bRealSpeed);
		CheckDlgButton(hDlg,IDC_GRAYSCALE,bGrayscale);
		CheckDlgButton(hDlg,IDC_ALWAYSONTOP,bAlwaysOnTop);
		CheckDlgButton(hDlg,IDC_ACTFOLLOWSMOUSE,bActFollowsMouse);
		CheckDlgButton(hDlg,IDC_AUTOSAVE,bAutoSave);
		CheckDlgButton(hDlg,IDC_AUTOSAVEONEXIT,bAutoSaveOnExit);
		CheckDlgButton(hDlg,IDC_OBJECTLOADWARNING,bLoadObjectWarning);
		CheckDlgButton(hDlg,IDC_ALWAYSDISPLOG,bAlwaysDisplayLog);

		// set disassebler mode
		CheckDlgButton(hDlg,(disassembler_mode == HP_MNEMONICS) ? IDC_DISASM_HP : IDC_DISASM_CLASS,BST_CHECKED);

		// set sound slider
		SendDlgItemMessage(hDlg,IDC_SOUND_SLIDER,TBM_SETRANGE,FALSE,MAKELONG(0,255));
		SendDlgItemMessage(hDlg,IDC_SOUND_SLIDER,TBM_SETTICFREQ,256/8,0);
		SendDlgItemMessage(hDlg,IDC_SOUND_SLIDER,TBM_SETPOS,TRUE,dwWaveVol);

		// set sound radio button
		CheckDlgButton(hDlg,bWaveBeep ? IDC_SOUND_WAVE : IDC_SOUND_SPEAKER,BST_CHECKED);
		EnableWindow(GetDlgItem(hDlg,IDC_SOUND_SLIDER),bWaveBeep);

		// set combobox parameter
		SetCommList(hDlg,szSerialWire,szSerialIr);
		if (bCommInit)						// disable when port open
		{
			EnableWindow(GetDlgItem(hDlg,IDC_WIRE),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_IR),FALSE);
		}

		// HP48SX/GX
		if (cCurrentRomType=='S' || cCurrentRomType=='G' || cCurrentRomType==0)
		{
			// init port1 enable checkbox
			CheckDlgButton(hDlg,IDC_PORT1EN,(Chipset.cards_status & PORT1_PRESENT) != 0);
			CheckDlgButton(hDlg,IDC_PORT1WR,(Chipset.cards_status & PORT1_WRITE) != 0);

			if (nArgc < 3)					// port2 filename from Emu48.ini file
			{
				szActPort2Filename = szPort2Filename;
			}
			else							// port2 filename given from command line
			{
				szActPort2Filename = ppArgv[2];
				EnableWindow(GetDlgItem(hDlg,IDC_PORT2),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_PORT2LOAD),FALSE);
			}

			// init port2 shared and writeable checkbox and set port2 filename
			CheckDlgButton(hDlg,IDC_PORT2ISSHARED,bPort2IsShared);
			CheckDlgButton(hDlg,IDC_PORT2WR,IsFileWriteable(szActPort2Filename));
			SetDlgItemText(hDlg,IDC_PORT2,szActPort2Filename);
			if (nState == SM_INVALID)		// Invalid State
			{
				EnableWindow(GetDlgItem(hDlg,IDC_PORT1EN),FALSE);
				EnableWindow(GetDlgItem(hDlg,IDC_PORT1WR),FALSE);
			}
		else								// HP38G/HP39G/HP40G/HP49G/HP39G+/HP48Gii/HP49G+  // CdB for HP: add apples
		else								// HP38G/HP39G/HP40G/HP49G
		{
			EnableWindow(GetDlgItem(hDlg,IDC_PORT1EN),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_PORT1WR),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_PORT2ISSHARED),FALSE);
			EnableWindow(GetDlgItem(hDlg,IDC_PORT2WR),FALSE);
			if (cCurrentRomType=='X' || cCurrentRomType=='2' || cCurrentRomType=='Q')		// HP49G/HP48GII/HP49G+   // CdB for HP: add Apples
			EnableWindow(GetDlgItem(hDlg,IDC_PORT2LOAD),FALSE);
			if (cCurrentRomType=='X')		// HP49G
			{
				SendDlgItemMessage(hDlg,IDC_IR,CB_RESETCONTENT,0,0);
				EnableWindow(GetDlgItem(hDlg,IDC_IR),FALSE);
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SOUND_SPEAKER:
			EnableWindow(GetDlgItem(hDlg,IDC_SOUND_SLIDER),FALSE);
			return TRUE;
		case IDC_SOUND_WAVE:
			EnableWindow(GetDlgItem(hDlg,IDC_SOUND_SLIDER),TRUE);
			return TRUE;
		case IDC_PORT2LOAD:
			if (GetLoadObjectFilename(_T(BIN_FILTER),_T("BIN")))
			{
				TCHAR  szFilename[MAX_PATH];
				LPTSTR lpFilePart;

				// check if file path and Emu48 directory path is identical
				if (GetFullPathName(szBufferFilename,ARRAYSIZEOF(szFilename),szFilename,&lpFilePart))
				{
					*(lpFilePart-1) = 0;	// devide path and name

					// name is in the Emu48 directory -> use only name
					if (lstrcmpi(szEmuDirectory,szFilename) == 0)
						lstrcpy(szBufferFilename,lpFilePart);
				}
				SetDlgItemText(hDlg,IDC_PORT2,szBufferFilename);

				// adjust R/W checkbox
				CheckDlgButton(hDlg,IDC_PORT2WR,IsFileWriteable(szBufferFilename));
			if (Chipset.Port1Size && (cCurrentRomType!='X' || cCurrentRomType!='2' || cCurrentRomType!='Q'))   // CdB for HP: add apples
			return TRUE;
		case IDOK:
			if (Chipset.Port1Size && cCurrentRomType!='X')
			{
				UINT nOldState = SwitchToState(SM_SLEEP);
				// save old card status
				BYTE bCardsStatus = Chipset.cards_status;

				// port1 disabled?
				Chipset.cards_status &= ~(PORT1_PRESENT | PORT1_WRITE);
				if (IsDlgButtonChecked(hDlg, IDC_PORT1EN))
				{
					Chipset.cards_status |= PORT1_PRESENT;
					if (IsDlgButtonChecked(hDlg, IDC_PORT1WR))
						Chipset.cards_status |= PORT1_WRITE;
				}

				// changed card status in slot1?
				if (   ((bCardsStatus ^ Chipset.cards_status) & (PORT1_PRESENT | PORT1_WRITE)) != 0
					&& (Chipset.IORam[CARDCTL] & ECDT) != 0 && (Chipset.IORam[TIMER2_CTRL] & RUN) != 0
				   )
				{
					Chipset.HST |= MP;			// set Module Pulled
					IOBit(SRQ2,NINT,FALSE);		// set NINT to low
					Chipset.SoftInt = TRUE;		// set interrupt
					bInterrupt = TRUE;
				}
				SwitchToState(nOldState);
			}
			// HP48SX/GX port2 change settings detection
			if (cCurrentRomType=='S' || cCurrentRomType=='G' || cCurrentRomType==0)
			{
				TCHAR szFilename[MAX_PATH];
				BOOL  bOldPort2IsShared = bPort2IsShared;

				szActPort2Filename = (nArgc < 3) ? szPort2Filename : ppArgv[2];

				// shared port
				bPort2IsShared = IsDlgButtonChecked(hDlg,IDC_PORT2ISSHARED);
				if (bPort2IsShared != bOldPort2IsShared)
				{
					bPort2CfgChange = TRUE;	// slot2 configuration changed
				}

				if (nArgc < 3)				// port2 filename from Emu48.ini file
				{
					// get current filename and notify difference
					GetDlgItemText(hDlg,IDC_PORT2,szFilename,ARRAYSIZEOF(szFilename));
					if (lstrcmp(szPort2Filename,szFilename) != 0)
					{
						lstrcpyn(szPort2Filename,szFilename,ARRAYSIZEOF(szPort2Filename));
						bPort2CfgChange = TRUE;	// slot2 configuration changed
					}
				}

				// R/W port
				if (   *szActPort2Filename != 0
					&& (BOOL) IsDlgButtonChecked(hDlg,IDC_PORT2WR) != IsFileWriteable(szActPort2Filename))
				{
					bPort2AttChange = TRUE;	// slot2 file R/W attribute changed
					bPort2CfgChange = TRUE;	// slot2 configuration changed
				}
			}
			// get speed checkbox value
			bRealSpeed = IsDlgButtonChecked(hDlg,IDC_REALSPEED);
			bAlwaysOnTop = IsDlgButtonChecked(hDlg,IDC_ALWAYSONTOP);
			bActFollowsMouse = IsDlgButtonChecked(hDlg,IDC_ACTFOLLOWSMOUSE);
			bAutoSave = IsDlgButtonChecked(hDlg,IDC_AUTOSAVE);
			bAutoSaveOnExit = IsDlgButtonChecked(hDlg,IDC_AUTOSAVEONEXIT);
			bLoadObjectWarning = IsDlgButtonChecked(hDlg,IDC_OBJECTLOADWARNING);
			bAlwaysDisplayLog = IsDlgButtonChecked(hDlg,IDC_ALWAYSDISPLOG);
			SetSpeed(bRealSpeed);			// set speed
			// LCD grayscale checkbox has been changed
			if (bGrayscale != (BOOL) IsDlgButtonChecked(hDlg,IDC_GRAYSCALE))
			{
				UINT nOldState = SwitchToState(SM_INVALID);
				SetLcdMode(!bGrayscale);	// set new display mode
				SwitchToState(nOldState);
			}
			if (bPort2CfgChange)			// slot2 configuration changed
			{
				UINT nOldState = SwitchToState(SM_INVALID);

				UnmapPort2();				// unmap port2

				if (bPort2AttChange)		// slot2 R/W mode changed
				{
					DWORD dwFileAtt;

					SetCurrentDirectory(szEmuDirectory);
					dwFileAtt = GetFileAttributes(szActPort2Filename);
					if (dwFileAtt != 0xFFFFFFFF)
					{
						if (IsDlgButtonChecked(hDlg,IDC_PORT2WR))
							dwFileAtt &= ~FILE_ATTRIBUTE_READONLY;
						else
							dwFileAtt |= FILE_ATTRIBUTE_READONLY;

						SetFileAttributes(szActPort2Filename,dwFileAtt);
					}
					SetCurrentDirectory(szCurrentDirectory);
				}

				if (cCurrentRomType)		// ROM defined
				{
					MapPort2(szActPort2Filename);

					// port2 changed and card detection enabled
					if (   (bPort2AttChange || Chipset.wPort2Crc != wPort2Crc)
						&& (Chipset.IORam[CARDCTL] & ECDT) != 0 && (Chipset.IORam[TIMER2_CTRL] & RUN) != 0
					   )
					{
						Chipset.HST |= MP;		// set Module Pulled
						IOBit(SRQ2,NINT,FALSE);	// set NINT to low
						Chipset.SoftInt = TRUE;	// set interrupt
						bInterrupt = TRUE;
					}
					// save fingerprint of port2
					Chipset.wPort2Crc = wPort2Crc;
				}
				SwitchToState(nOldState);
			}
			// set disassembler mode
			disassembler_mode = IsDlgButtonChecked(hDlg,IDC_DISASM_HP) ? HP_MNEMONICS : CLASS_MNEMONICS;
			// set sound data
			// HP49G, 48GII, 49G+ Ir port is not connected
			if (cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')		// HP49G/HP48GII/HP49G+   // CdB for HP: add Apples
			bWaveBeep = IsDlgButtonChecked(hDlg,IDC_SOUND_WAVE);
			// set combobox parameter
			GetDlgItemText(hDlg,IDC_WIRE,szSerialWire,ARRAYSIZEOF(szSerialWire));
			if (cCurrentRomType!='X')		// HP49G Ir port is not connected
				GetDlgItemText(hDlg,IDC_IR,szSerialIr,ARRAYSIZEOF(szSerialIr));
			// set window Z order
			hWndInsertAfter = bAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST;
			SetWindowPos(hWnd,hWndInsertAfter,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
			if (hDlgDebug != NULL)
			{
				SetWindowPos(hDlgDebug,hWndInsertAfter,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
			}
			// no break
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
}


//################
//#
//#    Save Helper
//#
//################

//
// UINT SaveChanges(BOOL bAuto);
// Return code :
// IDYES    File successfuly saved
// IDNO     File not saved
// IDCANCEL Cancel command
//
static UINT SaveChanges(BOOL bAuto)
{
	UINT uReply;

	if (pbyRom == NULL) return IDNO;

	if (bAuto)
		uReply = IDYES;
	else
	{
		UINT uStyle = bSaveDefConfirm ? 0 : MB_DEFBUTTON2;
		uReply = YesNoCancelMessage(_T("Do you want to save changes ?"),uStyle);
	}

	if (uReply != IDYES) return uReply;

	if (szCurrentFilename[0] == 0)
	{ // Save As...
		if (GetSaveAsFilename())
		{
			if (SaveDocumentAs(szBufferFilename))
				return IDYES;
			else
				return IDCANCEL;
		}
		return IDNO;
	}

	SaveDocument();
	return IDYES;
}



//################
//#
//#    Message Handlers
//#
//################

//
// WM_CREATE
//
static LRESULT OnCreate(HWND hWindow)
{
	InitializeCriticalSection(&csGDILock);
	InitializeCriticalSection(&csLcdLock);
	InitializeCriticalSection(&csKeyLock);
	InitializeCriticalSection(&csIOLock);
	InitializeCriticalSection(&csT1Lock);
	InitializeCriticalSection(&csT2Lock);
	InitializeCriticalSection(&csTxdLock);
	InitializeCriticalSection(&csRecvLock);
	InitializeCriticalSection(&csSlowLock);

	// load cursors
	hCursorArrow = LoadCursor(NULL,IDC_ARROW);
	hCursorHand  = LoadCursor(NULL,IDC_HAND);
	if (hCursorHand == NULL)
	{
		// for Win95, NT4.0
		bOwnCursor = ((hCursorHand = CreateHandCursor()) != NULL);
	}

	hWnd = hWindow;
	hWindowDC = GetDC(hWnd);
	return 0;
}

//
// WM_DESTROY
//
static LRESULT OnDestroy(HWND hWindow)
{
	DragAcceptFiles(hWnd,FALSE);			// no WM_DROPFILES message any more
	if (hThread) SwitchToState(SM_RETURN);	// exit emulation thread
	SetWindowTitle(NULL);					// free memory of title
	ReleaseDC(hWnd, hWindowDC);
	hWindowDC = NULL;						// hWindowDC isn't valid any more
	hWnd = NULL;

	if (bOwnCursor)							// destroy hand cursor
	{
		DestroyCursor(hCursorHand);
		bOwnCursor = FALSE;
	}

	DeleteCriticalSection(&csGDILock);
	DeleteCriticalSection(&csLcdLock);
	DeleteCriticalSection(&csKeyLock);
	DeleteCriticalSection(&csIOLock);
	DeleteCriticalSection(&csT1Lock);
	DeleteCriticalSection(&csT2Lock);
	DeleteCriticalSection(&csTxdLock);
	DeleteCriticalSection(&csRecvLock);
	DeleteCriticalSection(&csSlowLock);

	#if defined _USRDLL						// DLL version
		DLLDestroyWnd();					// cleanup system
	#else									// EXE version
		PostQuitMessage(0);					// exit message loop
	#endif
	return 0;
	UNREFERENCED_PARAMETER(hWindow);
}

//
// WM_PAINT
//
static LRESULT OnPaint(HWND hWindow)
{
	PAINTSTRUCT Paint;
	HDC hPaintDC;

	hPaintDC = BeginPaint(hWindow, &Paint);
	if (hMainDC != NULL)
	{
		RECT rcMainPaint = Paint.rcPaint;
		rcMainPaint.left   += nBackgroundX;	// coordinates in source bitmap
		rcMainPaint.top    += nBackgroundY;
			UINT nLines = MAINSCREENHEIGHT;
		rcMainPaint.bottom += nBackgroundY;

		EnterCriticalSection(&csGDILock);	// solving NT GDI problems
		{
			// CdB for HP: add apples display stuff
			// redraw header display area
			StretchBlt(hPaintDC, nLcdX, nLcdY,
				   131*nLcdZoom, Chipset.d0size*nLcdZoom,
				   hLcdDC, Chipset.d0offset, 0, 131, Chipset.d0size, SRCCOPY);
			StretchBlt(hPaintDC, nLcdX, nLcdY+Chipset.d0size*nLcdZoom,
				   hLcdDC, Chipset.boffset, Chipset.d0size, 131, MAINSCREENHEIGHT, SRCCOPY);
			StretchBlt(hPaintDC, nLcdX, nLcdY+(MAINSCREENHEIGHT+Chipset.d0size)*nLcdZoom,
				   131*nLcdZoom, MENUHEIGHT*nLcdZoom,
				   hLcdDC, 0, (MAINSCREENHEIGHT+Chipset.d0size), 131, MENUHEIGHT, SRCCOPY);
			SetWindowOrgEx(hPaintDC, nBackgroundX, nBackgroundY, NULL);

			// redraw main display area
			BitBlt(hPaintDC, nLcdX, nLcdY,
				   131*nLcdZoom, nLines*nLcdZoom,
				   hLcdDC, Chipset.boffset*nLcdZoom, 0, SRCCOPY);
			// redraw menu display area
			BitBlt(hPaintDC, nLcdX, nLcdY+nLines*nLcdZoom,
				   131*nLcdZoom, (64-nLines)*nLcdZoom,
				   hLcdDC, 0, nLines*nLcdZoom, SRCCOPY);
			GdiFlush();
		}
		LeaveCriticalSection(&csGDILock);
		UpdateAnnunciators();
		RefreshButtons(&rcMainPaint);
	}
	EndPaint(hWindow, &Paint);
	return 0;
}

//
// WM_DROPFILES
//
static LRESULT OnDropFiles(HANDLE hFilesInfo)
{
	TCHAR szFileName[MAX_PATH];
	WORD  wNumFiles,wIndex;
	BOOL  bSuccess;

	// get number of files dropped
	wNumFiles = DragQueryFile (hFilesInfo,(UINT)-1,NULL,0);

	SuspendDebugger();						// suspend debugger
	bDbgAutoStateCtrl = FALSE;				// disable automatic debugger state control

	// calculator off, turn on
	if (!(Chipset.IORam[BITOFFSET]&DON))
	{
		// turn on HP
		KeyboardEvent(TRUE,0,0x8000);
		KeyboardEvent(FALSE,0,0x8000);
	}

	_ASSERT(nState == SM_RUN);				// emulator must be in RUN state
	if (WaitForSleepState())				// wait for cpu SHUTDN then sleep state
	{
		DragFinish (hFilesInfo);
		InfoMessage(_T("The emulator is busy."));
		goto cancel;
	}

	_ASSERT(nState == SM_SLEEP);

	// get each name and load it into the emulator
	for (wIndex = 0;wIndex < wNumFiles;++wIndex)
	{
		DragQueryFile (hFilesInfo,wIndex,szFileName,ARRAYSIZEOF(szFileName));

		// szFileName has file name, now try loading it
		if ((bSuccess = LoadObject(szFileName)) == FALSE)
			break;
	}

	DragFinish (hFilesInfo);
	SwitchToState(SM_RUN);					// run state
	while (nState!=nNextState) Sleep(0);
	_ASSERT(nState == SM_RUN);

	if (bSuccess == FALSE)					// data not copied
		goto cancel;

	KeyboardEvent(TRUE,0,0x8000);
	Sleep(200);
	KeyboardEvent(FALSE,0,0x8000);
	// wait for sleep mode
	while (Chipset.Shutdn == FALSE) Sleep(0);

cancel:
	bDbgAutoStateCtrl = TRUE;				// enable automatic debugger state control
	ResumeDebugger();
	return 0;
}

//
// ID_FILE_NEW
//
static LRESULT OnFileNew(VOID)
{
	if (pbyRom)
	{
		SwitchToState(SM_INVALID);
		if (IDCANCEL == SaveChanges(bAutoSave))
			goto cancel;

		SaveBackup();
	}
	if (NewDocument()) SetWindowTitle(_T("Untitled"));
	UpdateWindowStatus();
cancel:
	if (pbyRom) SwitchToState(SM_RUN);
	return 0;
}

//
// ID_FILE_OPEN
//
static LRESULT OnFileOpen(VOID)
{
	if (pbyRom)
	{
		SwitchToState(SM_INVALID);
		if (IDCANCEL == SaveChanges(bAutoSave))
			goto cancel;
	}
	if (GetOpenFilename())
	{
		if (OpenDocument(szBufferFilename))
			MruAdd(szCurrentFilename);
	}
cancel:
	if (pbyRom) SwitchToState(SM_RUN);
	return 0;
}

//
// ID_FILE_MRU_FILE1
//
static LRESULT OnFileMruOpen(UINT wID)
{
	LPCTSTR lpszFilename;

	wID -= ID_FILE_MRU_FILE1;				// zero based MRU index
	lpszFilename = MruFilename(wID);		// full filename from MRU list
	if (lpszFilename == NULL) return 0;		// MRU slot not filled

	if (pbyRom)
	{
		SwitchToState(SM_INVALID);
		if (IDCANCEL == SaveChanges(bAutoSave))
			goto cancel;
	}
	if (!OpenDocument(lpszFilename))		// document loading failed
	{
		MruRemove(wID);						// entry not valid any more
	}
cancel:
	if (pbyRom) SwitchToState(SM_RUN);
	return 0;
}

//
// ID_FILE_SAVE
//
static LRESULT OnFileSave(VOID)
{
	if (pbyRom == NULL) return 0;
	SwitchToState(SM_INVALID);
	SaveChanges(TRUE);
	SwitchToState(SM_RUN);
	return 0;
}

//
// ID_FILE_SAVEAS
//
static LRESULT OnFileSaveAs(VOID)
{
	if (pbyRom == NULL) return 0;
	SwitchToState(SM_INVALID);

	if (GetSaveAsFilename())
	{
		if (SaveDocumentAs(szBufferFilename))
			MruAdd(szCurrentFilename);
	}

	SwitchToState(SM_RUN);
	return 0;
}

//
// ID_FILE_CLOSE
//
static LRESULT OnFileClose(VOID)
{
	if (pbyRom == NULL) return 0;
	SwitchToState(SM_INVALID);
	if (SaveChanges(bAutoSave) != IDCANCEL)
	{
		ResetDocument();
		SetWindowTitle(NULL);
	}
	else
	{
		SwitchToState(SM_RUN);
	}
	return 0;
}

//
// ID_FILE_EXIT
//
// WM_SYS_CLOSE
//
static LRESULT OnFileExit(VOID)
{
	SwitchToState(SM_INVALID);				// hold emulation thread
	if (SaveChanges(bAutoSaveOnExit) == IDCANCEL)
	{
		SwitchToState(SM_RUN);				// on cancel restart emulation thread
		return 0;
	}
	DestroyWindow(hWnd);
	return 0;
}

//
// ID_VIEW_COPY
//
static LRESULT OnViewCopy(VOID)
{
	if (OpenClipboard(hWnd))
	{
		if (EmptyClipboard())
		{
#if !defined MONOCHROME
			// DIB bitmap
			#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)
			#define PALVERSION       0x300
			hBmp = CreateCompatibleBitmap(hLcdDC,131*nLcdZoom,SCREENHEIGHT*nLcdZoom);   // CdB for HP: add apples display stuff
			BITMAP bm;
			StretchBlt(hBmpDC,0,0,131*nLcdZoom,SCREENHEIGHT*nLcdZoom,hLcdDC,0,0, 131, SCREENHEIGHT, SRCCOPY);   // CdB for HP: add apples display stuff
			PLOGPALETTE ppal;
			HBITMAP hBmp;
			HDC hBmpDC;
			HANDLE hClipObj;
			WORD wBits;
			DWORD dwLen, dwSizeImage;

			_ASSERT(nLcdZoom == 1 || nLcdZoom == 2 || nLcdZoom == 4);
			hBmp = CreateCompatibleBitmap(hLcdDC,131*nLcdZoom,64*nLcdZoom);
			hBmpDC = CreateCompatibleDC(hLcdDC);
			hBmp = SelectObject(hBmpDC,hBmp);
			BitBlt(hBmpDC,0,0,131*nLcdZoom,64*nLcdZoom,hLcdDC,Chipset.boffset*nLcdZoom,0,SRCCOPY);
			hBmp = SelectObject(hBmpDC,hBmp);

			// fill BITMAP structure for size information
			GetObject(hBmp, sizeof(bm), &bm);

			wBits = bm.bmPlanes * bm.bmBitsPixel;
			// make sure bits per pixel is valid
			if (wBits <= 1)
				wBits = 1;
			else if (wBits <= 4)
				wBits = 4;
			else if (wBits <= 8)
				wBits = 8;
			else // if greater than 8-bit, force to 24-bit
				wBits = 24;

			dwSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * wBits) * bm.bmHeight;

			// calculate memory size to store CF_DIB data
			dwLen = sizeof(BITMAPINFOHEADER) + dwSizeImage;
			if (wBits != 24)				// a 24 bitcount DIB has no color table
			{
				// add size for color table
				dwLen += (DWORD) (1 << wBits) * sizeof(RGBQUAD);
			}

			// memory allocation for clipboard data
			if ((hClipObj = GlobalAlloc(GMEM_MOVEABLE, dwLen)) != NULL)
			{
				lpbi = GlobalLock(hClipObj);
				// initialize BITMAPINFOHEADER
				lpbi->biSize = sizeof(BITMAPINFOHEADER);
				lpbi->biWidth = bm.bmWidth;
				lpbi->biHeight = bm.bmHeight;
				lpbi->biPlanes = 1;
				lpbi->biBitCount = wBits;
				lpbi->biCompression = BI_RGB;
				lpbi->biSizeImage = dwSizeImage;
				lpbi->biXPelsPerMeter = 0;
				lpbi->biYPelsPerMeter = 0;
				lpbi->biClrUsed = 0;
				lpbi->biClrImportant = 0;
				// get bitmap color table and bitmap data
				GetDIBits(hBmpDC, hBmp, 0, lpbi->biHeight, (LPBYTE)lpbi + dwLen - dwSizeImage,
                          (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);
				GlobalUnlock(hClipObj);
				SetClipboardData(CF_DIB, hClipObj);

				// get number of entries in the logical palette
				GetObject(hPalette,sizeof(WORD),&wBits);

				// memory allocation for temporary palette data
				if ((ppal = HeapAlloc(hHeap,HEAP_ZERO_MEMORY,sizeof(LOGPALETTE) + wBits * sizeof(PALETTEENTRY))) != NULL)
				{
					ppal->palVersion    = PALVERSION;
					ppal->palNumEntries = wBits;
					GetPaletteEntries(hPalette, 0, wBits, ppal->palPalEntry);
					SetClipboardData(CF_PALETTE, CreatePalette(ppal));
			hBmp    = CreateBitmap(131*nLcdZoom,SCREENHEIGHT*nLcdZoom,1,1,NULL);   // CdB for HP: add apples display management
				}
			StretchBlt(hBmpDC,0,0,131*nLcdZoom,SCREENHEIGHT*nLcdZoom,hLcdDC,0,0, 131, SCREENHEIGHT, SRCCOPY);   // CdB for HP: add apples display stuff
			DeleteDC(hBmpDC);
			DeleteObject(hBmp);
			#undef WIDTHBYTES
			#undef PALVERSION
#else
			HBITMAP hOldBmp, hBmp;
			HDC hBmpDC;

			// don't work with background index <> 0
			_ASSERT(nLcdZoom == 1 || nLcdZoom == 2 || nLcdZoom == 4);
			hBmp    = CreateBitmap(131*nLcdZoom,64*nLcdZoom,1,1,NULL);
			hBmpDC  = CreateCompatibleDC(NULL);
			hOldBmp = (HBITMAP)SelectObject(hBmpDC,hBmp);
			BitBlt(hBmpDC,0,0,131*nLcdZoom,64*nLcdZoom,hLcdDC,Chipset.boffset*nLcdZoom,0,SRCCOPY);
			SetClipboardData(CF_BITMAP,hBmp);
			SelectObject(hBmpDC,hOldBmp);
			DeleteDC(hBmpDC);
#endif
		}
		CloseClipboard();
	}
	return 0;
}

//
// ID_VIEW_RESET
//
static LRESULT OnViewReset(VOID)
{
	if (nState != SM_RUN) return 0;
	if (YesNoMessage(_T("Are you sure you want to press the Reset Button ?"))==IDYES)
	{
		SwitchToState(SM_SLEEP);
		CpuReset();							// register setting after Cpu Reset
		SwitchToState(SM_RUN);
	}
	return 0;
}

//
// ID_VIEW_SETTINGS
//
static LRESULT OnViewSettings(VOID)
{
	// not in nState = SM_INVALID or port2 file must be closed from document
	_ASSERT(nState != SM_INVALID || pbyPort2 == NULL);

	if (DialogBox(hApp, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, (DLGPROC)SettingsProc) == -1)
		AbortMessage(_T("Settings Dialog Creation Error !"));

	WriteSettings();
	return 0;
}

//
// ID_VIEW_SCRIPT
//
static LRESULT OnViewScript(VOID)
{
	BYTE cType = cCurrentRomType;
	if (nState != SM_RUN)
	{
		InfoMessage(_T("You cannot change the KML script when Emu48 is not running.\n")
					_T("Use the File,New menu item to create a new calculator."));
		return 0;
	}
	SwitchToState(SM_INVALID);

	do
	{
		if (!DisplayChooseKml(cType)) break;
	}
	while (!InitKML(szCurrentKml,FALSE));

	SetWindowPathTitle(szCurrentFilename);	// update window title line
	if (pbyRom) SwitchToState(SM_RUN);
	return 0;
}

//
// ID_BACKUP_SAVE
//
static LRESULT OnBackupSave(VOID)
{
	UINT nOldState;
	if (pbyRom == NULL) return 0;
	nOldState = SwitchToState(SM_INVALID);
	SaveBackup();
	SwitchToState(nOldState);
	return 0;
}

//
// ID_BACKUP_RESTORE
//
static LRESULT OnBackupRestore(VOID)
{
	SwitchToState(SM_INVALID);
	RestoreBackup();
	if (pbyRom) SwitchToState(SM_RUN);
	return 0;
}

//
// ID_BACKUP_DELETE
//
static LRESULT OnBackupDelete(VOID)
{
	ResetBackup();
	return 0;
}

//
// ID_OBJECT_LOAD
//
static LRESULT OnObjectLoad(VOID)
{
	SuspendDebugger();						// suspend debugger
	bDbgAutoStateCtrl = FALSE;				// disable automatic debugger state control

	// calculator off, turn on
	if (!(Chipset.IORam[BITOFFSET]&DON))
	{
		KeyboardEvent(TRUE,0,0x8000);
		KeyboardEvent(FALSE,0,0x8000);

		// wait for sleep mode
		while (Chipset.Shutdn == FALSE) Sleep(0);
	}

	if (nState != SM_RUN)
	{
		InfoMessage(_T("The emulator must be running to load an object."));
		goto cancel;
	}

	if (WaitForSleepState())				// wait for cpu SHUTDN then sleep state
	{
		InfoMessage(_T("The emulator is busy."));
		goto cancel;
	}

	_ASSERT(nState == SM_SLEEP);

	if (bLoadObjectWarning)
	{
		UINT uReply = YesNoCancelMessage(
			_T("Warning: Trying to load an object while the emulator is busy\n")
			_T("will certainly result in a memory lost. Before loading an object\n")
			_T("you should be sure that the calculator is not doing anything.\n")
			_T("Do you want to see this warning next time you try to load an object ?"),0);
		switch (uReply)
		{
		case IDYES:
			break;
		case IDNO:
			bLoadObjectWarning = FALSE;
			break;
		case IDCANCEL:
			SwitchToState(SM_RUN);
			goto cancel;
		}
	}

	if (!GetLoadObjectFilename(_T(HP_FILTER),_T("HP")))
	{
		SwitchToState(SM_RUN);
		goto cancel;
	}

	if (!LoadObject(szBufferFilename))
	{
		SwitchToState(SM_RUN);
		goto cancel;
	}

	SwitchToState(SM_RUN);					// run state
	while (nState!=nNextState) Sleep(0);
	_ASSERT(nState == SM_RUN);
	KeyboardEvent(TRUE,0,0x8000);
	Sleep(200);
	KeyboardEvent(FALSE,0,0x8000);
	while (Chipset.Shutdn == FALSE) Sleep(0);

cancel:
	bDbgAutoStateCtrl = TRUE;				// enable automatic debugger state control
	ResumeDebugger();
	return 0;
}

//
// ID_OBJECT_SAVE
//
static LRESULT OnObjectSave(VOID)
{
	if (nState != SM_RUN)
	{
		InfoMessage(_T("The emulator must be running to save an object."));
		return 0;
	}

	if (WaitForSleepState())				// wait for cpu SHUTDN then sleep state
	{
		InfoMessage(_T("The emulator is busy."));
		return 0;
	}

	_ASSERT(nState == SM_SLEEP);

	if (GetSaveObjectFilename(_T(HP_FILTER),_T("HP")))
	{
		SaveObject(szBufferFilename);
	}

	SwitchToState(SM_RUN);
	return 0;
}

//
// ID_TOOL_DISASM
//
static INT_PTR CALLBACK Disasm(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static DWORD dwAddress, dwAddressMax;

	LONG  i;
	TCHAR *cpStop,szAddress[256] = _T("0");

	switch (message)
	{
	case WM_INITDIALOG:
		// set fonts & cursor
		SendDlgItemMessage(hDlg,IDC_DISASM_MODULE,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_MAP,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_ROM,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_RAM,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_PORT1,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_PORT2,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_MAP,BM_SETCHECK,1,0);
		SendDlgItemMessage(hDlg,IDC_ADDRESS,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_ADR,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_NEXT,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDC_DISASM_COPY,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SendDlgItemMessage(hDlg,IDCANCEL,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),MAKELPARAM(FALSE,0));
		SetDlgItemText(hDlg,IDC_DISASM_ADR,szAddress);
		disassembler_map = MEM_MAP;			// disassemble with mapped modules
		dwAddress = _tcstoul(szAddress,&cpStop,16);
		dwAddressMax = 0x100000;			// greatest address (mapped mode)
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		// decode radio buttons
		case IDC_DISASM_MAP:
			disassembler_map = MEM_MAP;
			dwAddressMax = 0x100000;
			dwAddressMax = ((cCurrentRomType=='E' || cCurrentRomType=='X' || cCurrentRomType=='P' || cCurrentRomType=='2' || cCurrentRomType=='Q')    // CdB for HP: add apples
		case IDC_DISASM_ROM:
			disassembler_map = MEM_ROM;
			dwAddressMax = dwRomSize;
			return TRUE;
		case IDC_DISASM_RAM:
			disassembler_map = MEM_RAM;
			dwAddressMax = Chipset.Port0Size * 2048;
			return TRUE;
		case IDC_DISASM_PORT1:
			disassembler_map = MEM_PORT1;
			dwAddressMax = ((Chipset.cards_status & PORT1_PRESENT) != 0) ? (Chipset.Port1Size * 2048) : 0;
			return TRUE;
		case IDC_DISASM_PORT2:
			disassembler_map = MEM_PORT2;
			dwAddressMax = ((cCurrentRomType=='E' || cCurrentRomType=='X')
				           ? Chipset.Port2Size
						   : dwPort2Size)
						 * 2048;
			return TRUE;
		case IDOK:
			SendDlgItemMessage(hDlg,IDC_DISASM_ADR,EM_SETSEL,0,-1);
			GetDlgItemText(hDlg,IDC_DISASM_ADR,szAddress,ARRAYSIZEOF(szAddress));
			// test if valid hex address
			for (i = 0; i < (LONG) lstrlen(szAddress); ++i)
			{
				if (_istxdigit(szAddress[i]) == FALSE)
					return FALSE;
			}
			dwAddress = _tcstoul(szAddress,&cpStop,16);
			// no break
		case IDC_DISASM_NEXT:
			if (dwAddress >= dwAddressMax)
				return FALSE;
			i = wsprintf(szAddress,(dwAddress <= 0xFFFFF) ? _T("%05lX   ") : _T("%06lX  "),dwAddress);
			dwAddress = disassemble(dwAddress,&szAddress[i],VIEW_LONG);
			i = (LONG) SendDlgItemMessage(hDlg,IDC_DISASM_WIN,LB_ADDSTRING,0,(LPARAM) szAddress);
			SendDlgItemMessage(hDlg,IDC_DISASM_WIN,LB_SELITEMRANGE,FALSE,MAKELPARAM(0,i));
			SendDlgItemMessage(hDlg,IDC_DISASM_WIN,LB_SETSEL,TRUE,i);
			SendDlgItemMessage(hDlg,IDC_DISASM_WIN,LB_SETTOPINDEX,i,0);
			return TRUE;
		case IDC_DISASM_COPY:
			// copy selected items to clipboard
			CopyItemsToClipboard(GetDlgItem(hDlg,IDC_DISASM_WIN));
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg,IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
}

//
// ID_ABOUT
//
static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,IDC_VERSION,szNoTitle);
		SetDlgItemText(hDlg,IDC_LICENSE,szLicence);
		return TRUE;
	case WM_COMMAND:
		wParam = LOWORD(wParam);
		if ((wParam==IDOK)||(wParam==IDCANCEL))
		{
			EndDialog(hDlg, wParam);
			return TRUE;
		}
		break;
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
}

static LRESULT OnToolDisasm(VOID)			// disasm dialogbox call
{
	if (pbyRom) SwitchToState(SM_SLEEP);
	if (DialogBox(hApp, MAKEINTRESOURCE(IDD_DISASM), hWnd, (DLGPROC)Disasm) == -1)
		AbortMessage(_T("Disassembler Dialog Box Creation Error !"));
	if (pbyRom) SwitchToState(SM_RUN);
	return 0;
}

static LRESULT OnAbout(VOID)
{
	if (DialogBox(hApp, MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)About) == -1)
		AbortMessage(_T("About Dialog Box Creation Error !"));
	return 0;
}

static LRESULT OnLButtonDown(UINT nFlags, WORD x, WORD y)
{
	if (nMacroState == MACRO_PLAY) return 0; // playing macro
	if (nState == SM_RUN) MouseButtonDownAt(nFlags, x,y);
	return 0;
}

static LRESULT OnLButtonUp(UINT nFlags, WORD x, WORD y)
{
	if (nMacroState == MACRO_PLAY) return 0; // playing macro
	if (nState == SM_RUN) MouseButtonUpAt(nFlags, x,y);
	return 0;
}

static LRESULT OnMouseMove(UINT nFlags, WORD x, WORD y)
{
	// emulator not active but cursor is over emulator window
	if (bActFollowsMouse && GetActiveWindow() != hWnd)
	{
		ForceForegroundWindow(hWnd);		// force emulator window to foreground
	}

	if (nMacroState == MACRO_PLAY) return 0; // playing macro
	if (nState == SM_RUN) MouseMovesTo(nFlags, x,y);
	return 0;
}

static LRESULT OnNcMouseMove(UINT nFlags, WORD x, WORD y)
{
	// emulator not active but cursor is over emulator window
	if (bActFollowsMouse && GetActiveWindow() != hWnd)
	{
		ForceForegroundWindow(hWnd);		// force emulator window to foreground
	}
	return 0;
	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(x);
	UNREFERENCED_PARAMETER(y);
}

static LRESULT OnKeyDown(int nVirtKey, LPARAM lKeyData)
{
	if (nMacroState == MACRO_PLAY) return 0; // playing macro
	// call RunKey() only once (suppress autorepeat feature)
	if (nState == SM_RUN && (lKeyData & 0x40000000) == 0)
		RunKey((BYTE)nVirtKey, TRUE);
	return 0;
}

static LRESULT OnKeyUp(int nVirtKey, LPARAM lKeyData)
{
	if (nMacroState == MACRO_PLAY) return 0; // playing macro
	if (nState == SM_RUN) RunKey((BYTE)nVirtKey, FALSE);
	return 0;
	UNREFERENCED_PARAMETER(lKeyData);
}

LRESULT CALLBACK MainWndProc(HWND hWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:          return OnCreate(hWindow);
	case WM_DESTROY:         return OnDestroy(hWindow);
	case WM_PAINT:           return OnPaint(hWindow);
	case WM_DROPFILES:       return OnDropFiles((HANDLE)wParam);
	case WM_ACTIVATE:
		if (LOWORD(wParam)==WA_INACTIVE) break;
	case WM_QUERYNEWPALETTE:
		if (hPalette)
		{
			SelectPalette(hWindowDC, hPalette, FALSE);
			if (RealizePalette(hWindowDC))
			{
				InvalidateRect(hWindow,NULL,TRUE);
				return TRUE;
			}
		}
		return FALSE;
	case WM_PALETTECHANGED:
		if ((HWND)wParam == hWindow) break;
		if (hPalette)
		{
			SelectPalette(hWindowDC, hPalette, FALSE);
			if (RealizePalette(hWindowDC))
			{
				// UpdateColors(hWindowDC);
				InvalidateRect (hWnd, (LPRECT) (NULL), 1);
			}
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEW:            return OnFileNew();
		case ID_FILE_OPEN:           return OnFileOpen();
		case ID_FILE_SAVE:           return OnFileSave();
		case ID_FILE_SAVEAS:         return OnFileSaveAs();
		case ID_FILE_CLOSE:          return OnFileClose();
		case ID_FILE_EXIT:           return OnFileExit();
		case ID_STACK_COPY:          return OnStackCopy();
		case ID_STACK_PASTE:         return OnStackPaste();
		case ID_VIEW_COPY:           return OnViewCopy();
		case ID_VIEW_RESET:          return OnViewReset();
		case ID_VIEW_SETTINGS:       return OnViewSettings();
		case ID_VIEW_SCRIPT:         return OnViewScript();
		case ID_BACKUP_SAVE:         return OnBackupSave();
		case ID_BACKUP_RESTORE:      return OnBackupRestore();
		case ID_BACKUP_DELETE:       return OnBackupDelete();
		case ID_OBJECT_LOAD:         return OnObjectLoad();
		case ID_OBJECT_SAVE:         return OnObjectSave();
		case ID_TOOL_DISASM:         return OnToolDisasm();
		case ID_TOOL_DEBUG:          return OnToolDebug();
		case ID_TOOL_MACRO_RECORD:   return OnToolMacroNew();
		case ID_TOOL_MACRO_PLAY:     return OnToolMacroPlay();
		case ID_TOOL_MACRO_STOP:     return OnToolMacroStop();
		case ID_TOOL_MACRO_SETTINGS: return OnToolMacroSettings();
		case ID_ABOUT:               return OnAbout();
		}
		// check if command ID belongs to MRU file area
		if (   (LOWORD(wParam) >= ID_FILE_MRU_FILE1)
			&& (LOWORD(wParam) <  ID_FILE_MRU_FILE1 + MruEntries()))
			return OnFileMruOpen(LOWORD(wParam));
		break;
	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
		{
		case SC_CLOSE: return OnFileExit();
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN: return OnLButtonDown((UINT) wParam, LOWORD(lParam), HIWORD(lParam));
	case WM_LBUTTONUP:   return OnLButtonUp((UINT) wParam, LOWORD(lParam), HIWORD(lParam));
	case WM_MOUSEMOVE:   return OnMouseMove((UINT) wParam, LOWORD(lParam), HIWORD(lParam));
	case WM_NCMOUSEMOVE: return OnNcMouseMove((UINT) wParam, LOWORD(lParam), HIWORD(lParam));
	case WM_KEYUP:       return OnKeyUp((int)wParam, lParam);
	case WM_KEYDOWN:     return OnKeyDown((int)wParam, lParam);
	}
	return DefWindowProc(hWindow, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	RECT rectWindow;
	HACCEL hAccel;
	HSZ hszService, hszTopic;				// variables for DDE server
	DWORD_PTR dwAffMask;
	LPTSTR lpFilePart;

	hApp = hInst;
	#if defined _UNICODE
	{
		ppArgv = CommandLineToArgvW(GetCommandLine(),&nArgc);
	}
	#else
	{
		nArgc = __argc;						// no. of command line arguments
		ppArgv = (LPCTSTR*) __argv;			// command line arguments
	}
	#endif

	hHeap = GetProcessHeap();
	if (hHeap == NULL)
	{
		AbortMessage(_T("Heap creation failed."));
		return FALSE;
	}

	wc.style = CS_BYTEALIGNCLIENT;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_EMU48));
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.lpszClassName = _T("CEmu48");

	if (!RegisterClass(&wc))
	{
		AbortMessage(
			_T("CEmu48 class registration failed.\n")
			_T("This application will now terminate."));
		return FALSE;
	}

	// Create window
	rectWindow.left   = 0;
	rectWindow.top    = 0;
	rectWindow.right  = 256;
	rectWindow.bottom = 0;
	AdjustWindowRect(&rectWindow, WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_OVERLAPPED, TRUE);

	hWnd = CreateWindow(_T("CEmu48"), _T("Emu48"),
		WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_OVERLAPPED,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rectWindow.right  - rectWindow.left,
		rectWindow.bottom - rectWindow.top,
		NULL,NULL,hApp,NULL
		);

	if (hWnd == NULL)
	{
		AbortMessage(_T("Window creation failed."));
		return FALSE;
	}

	VERIFY(hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_MENU)));

	// initialization
	QueryPerformanceFrequency(&lFreq);		// init high resolution counter
	QueryPerformanceCounter(&lAppStart);

	GetCurrentDirectory(ARRAYSIZEOF(szCurrentDirectory), szCurrentDirectory);
	szCurrentKml[0] = 0;					// no KML file selected
	ReadSettings();
	MruInit(4);								// init MRU entries

	UpdateWindowStatus();

	// create auto event handle
	hEventShutdn = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (hEventShutdn == NULL)
	{
		AbortMessage(_T("Event creation failed."));
		DestroyWindow(hWnd);
		return FALSE;
	}

	nState     = SM_RUN;					// init state must be <> nNextState
	nNextState = SM_INVALID;				// go into invalid state
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WorkerThread, NULL, CREATE_SUSPENDED, &lThreadId);
	if (hThread == NULL)
	{
		CloseHandle(hEventShutdn);			// close event handle
		AbortMessage(_T("Thread creation failed."));
		DestroyWindow(hWnd);
		return FALSE;
	}
	// on multiprocessor machines for QueryPerformanceCounter()
	dwAffMask = SetThreadAffinityMask(hThread,1);
	_ASSERT(dwAffMask != 0);
	ResumeThread(hThread);					// start thread
	while (nState!=nNextState) Sleep(0);	// wait for thread initialized

	idDdeInst = 0;							// initialize DDE server
	if (DdeInitialize(&idDdeInst,(PFNCALLBACK) &DdeCallback,
                      APPCLASS_STANDARD |
                      CBF_FAIL_EXECUTES | CBF_FAIL_ADVISES |
                      CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS,0))
	{
		TerminateThread(hThread, 0);		// kill emulation thread
		CloseHandle(hEventShutdn);			// close event handle
		AbortMessage(_T("Could not initialize server!"));
		DestroyWindow(hWnd);
		return FALSE;
	}

	// init clipboard format and name service
	uCF_HpObj = RegisterClipboardFormat(_T(CF_HPOBJ));
	hszService = DdeCreateStringHandle(idDdeInst,szAppName,0);
	hszTopic   = DdeCreateStringHandle(idDdeInst,szTopic,0);
	DdeNameService(idDdeInst,hszService,NULL,DNS_REGISTER);

	_ASSERT(hWnd != NULL);
	_ASSERT(hWindowDC != NULL);

	if (nArgc >= 2)							// use decoded parameter line
		lstrcpyn(szBufferFilename,ppArgv[1],ARRAYSIZEOF(szBufferFilename));
	else									// use last document setting
		ReadLastDocument(szBufferFilename, ARRAYSIZEOF(szBufferFilename));

	if (szBufferFilename[0])				// given default document
	{
		TCHAR szTemp[MAX_PATH+8] = _T("Loading ");
		RECT  rectClient;

		_ASSERT(hWnd != NULL);
		VERIFY(GetClientRect(hWnd,&rectClient));
		GetCutPathName(szBufferFilename,&szTemp[8],MAX_PATH,rectClient.right/11);
		SetWindowTitle(szTemp);
		if (OpenDocument(szBufferFilename))
		{
			MruAdd(szCurrentFilename);
			ShowWindow(hWnd,nCmdShow);
			goto start;
		}
	}

	SetWindowTitle(_T("New Document"));
	ShowWindow(hWnd,nCmdShow);				// show emulator menu

	// no default document, ask for new one
	if (NewDocument()) SetWindowTitle(_T("Untitled"));

start:
	if (bStartupBackup) SaveBackup();		// make a RAM backup at startup
	if (pbyRom) SwitchToState(SM_RUN);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (   !TranslateAccelerator(hWnd, hAccel, &msg)
			&& (hDlgDebug   == NULL || !IsDialogMessage(hDlgDebug,   &msg))
		    && (hDlgFind    == NULL || !IsDialogMessage(hDlgFind,    &msg))
			&& (hDlgProfile == NULL || !IsDialogMessage(hDlgProfile, &msg)))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// clean up DDE server
	DdeNameService(idDdeInst, hszService, NULL, DNS_UNREGISTER);
	DdeFreeStringHandle(idDdeInst, hszService);
	DdeFreeStringHandle(idDdeInst, hszTopic);
	DdeUninitialize(idDdeInst);

	// get full path name of szCurrentFilename
	GetFullPathName(szCurrentFilename,ARRAYSIZEOF(szBufferFilename),szBufferFilename,&lpFilePart);

	WriteLastDocument(szBufferFilename);	// save last document setting
	WriteSettings();						// save emulation settings

	CloseHandle(hThread);					// close thread handle
	CloseHandle(hEventShutdn);				// close event handle
	_ASSERT(nState == SM_RETURN);			// emulation thread down?
	ResetDocument();
	ResetBackup();
	MruCleanup();
	_ASSERT(pbyRom == NULL);				// rom file unmapped
	_ASSERT(pbyPort2 == NULL);				// port2 file unmapped
	_ASSERT(pKml == NULL);					// KML script not closed
	_ASSERT(szTitle == NULL);				// freed allocated memory
	_ASSERT(hPalette == NULL);				// freed resource memory

	return (int) msg.wParam;
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(hPrevInst);
}
