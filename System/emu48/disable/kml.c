/*
 *   kml.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */
#include "pch.h"
#include "resource.h"
#include "emu48.h"
#include "kml.h"

static VOID      FatalError(VOID);
static VOID      InitLex(LPTSTR szScript);
static VOID      CleanLex(VOID);
static VOID      SkipWhite(UINT nMode);
static TokenId   ParseToken(UINT nMode);
static DWORD     ParseInteger(VOID);
static LPTSTR    ParseString(VOID);
static TokenId   Lex(UINT nMode);
static KmlLine*  ParseLine(TokenId eCommand);
static KmlLine*  IncludeLines(LPCTSTR szFilename);
static KmlLine*  ParseLines(VOID);
static KmlBlock* ParseBlock(TokenId eBlock);
static KmlBlock* IncludeBlocks(LPCTSTR szFilename);
static KmlBlock* ParseBlocks(VOID);
static VOID      FreeLines(KmlLine* pLine);
static VOID      PressButton(UINT nId);
static VOID      ReleaseButton(UINT nId);
static VOID      PressButtonById(UINT nId);
static VOID      ReleaseButtonById(UINT nId);
static LPTSTR    GetStringParam(KmlBlock* pBlock, TokenId eBlock, TokenId eCommand, UINT nParam);
static DWORD     GetIntegerParam(KmlBlock* pBlock, TokenId eBlock, TokenId eCommand, UINT nParam);
static KmlLine*  SkipLines(KmlLine* pLine, TokenId eCommand);
static KmlLine*  If(KmlLine* pLine, BOOL bCondition);
static KmlLine*  RunLine(KmlLine* pLine);
static KmlBlock* LoadKMLGlobal(LPCTSTR szFilename);

KmlBlock* pKml;
static KmlBlock* pVKey[256];
static BYTE      byVKeyMap[256];
static KmlButton pButton[256];
static KmlAnnunciator pAnnunciator[6];
static UINT      nButtons = 0;
static UINT      nScancodes = 0;
static UINT      nAnnunciators = 0;
static BOOL      bDebug = TRUE;
static UINT      nLexLine;
static UINT      nLexInteger;
static UINT      nBlocksIncludeLevel;
static UINT      nLinesIncludeLevel;
static DWORD     nKMLFlags = 0;
static LPTSTR    szLexString;
static LPTSTR    szText;
static LPCTSTR   szLexDelim[] =
{
	_T(""),
	_T(" \t\n\r"),
	_T(" \t\n\r"),
	_T(" \t\r")
};

static KmlToken pLexToken[] =
{
	{TOK_ANNUNCIATOR,000001,11,_T("Annunciator")},
	{TOK_BACKGROUND, 000000,10,_T("Background")},
	{TOK_IFPRESSED,  000001, 9,_T("IfPressed")},
	{TOK_RESETFLAG,  000001, 9,_T("ResetFlag")},
	{TOK_SCANCODE,   000001, 8,_T("Scancode")},
	{TOK_HARDWARE,   000002, 8,_T("Hardware")},
	{TOK_MENUITEM,   000001, 8,_T("MenuItem")},
	{TOK_SETFLAG,    000001, 7,_T("SetFlag")},
	{TOK_RELEASE,    000001, 7,_T("Release")},
	{TOK_VIRTUAL,    000000, 7,_T("Virtual")},
	{TOK_INCLUDE,    000002, 7,_T("Include")},
	{TOK_NOTFLAG,    000001, 7,_T("NotFlag")},
	{TOK_GLOBAL,     000000, 6,_T("Global")},
	{TOK_AUTHOR,     000002, 6,_T("Author")},
	{TOK_BITMAP,     000002, 6,_T("Bitmap")},
	{TOK_OFFSET,     000011, 6,_T("Offset")},
	{TOK_BUTTON,     000001, 6,_T("Button")},
	{TOK_IFFLAG,     000001, 6,_T("IfFlag")},
	{TOK_ONDOWN,     000000, 6,_T("OnDown")},
	{TOK_NOHOLD,     000000, 6,_T("NoHold")},
	{TOK_TITLE,      000002, 5,_T("Title")},
	{TOK_OUTIN,      000011, 5,_T("OutIn")},
	{TOK_PATCH,      000002, 5,_T("Patch")},
	{TOK_PRINT,      000002, 5,_T("Print")},
	{TOK_DEBUG,      000001, 5,_T("Debug")},
	{TOK_COLOR,      001111, 5,_T("Color")},
	{TOK_MODEL,      000002, 5,_T("Model")},
	{TOK_CLASS,      000001, 5,_T("Class")},
	{TOK_PRESS,      000001, 5,_T("Press")},
	{TOK_TYPE,       000001, 4,_T("Type")},
	{TOK_SIZE,       000011, 4,_T("Size")},
	{TOK_ZOOM,       000001, 4,_T("Zoom")},
	{TOK_DOWN,       000011, 4,_T("Down")},
	{TOK_ELSE,       000000, 4,_T("Else")},
	{TOK_ONUP,       000000, 4,_T("OnUp")},
	{TOK_MAP,        000011, 3,_T("Map")},
	{TOK_ROM,        000002, 3,_T("Rom")},
	{TOK_LCD,        000000, 3,_T("Lcd")},
	{TOK_END,        000000, 3,_T("End")},
	{0,              000000, 0,_T("")},
};

static TokenId eIsBlock[] =
{
	TOK_IFFLAG,
	TOK_IFPRESSED,
	TOK_ONDOWN,
	TOK_ONUP,
	TOK_NONE
};

static BOOL bClicking = FALSE;
static UINT uButtonClicked = 0;

static BOOL bPressed = FALSE;				// no key pressed
static UINT uLastPressedKey = 0;			// var for last pressed key

//################
//#
//#    Compilation Result
//#
//################

static UINT nLogLength = 0;
static LPTSTR szLog = NULL;
static BOOL bKmlLogOkEnabled = FALSE;

static VOID ClearLog()
{
	nLogLength = 0;
	if (szLog != NULL)
	{
		HeapFree(hHeap,0,szLog);
		szLog = NULL;
	}
	return;
}

static VOID AddToLog(LPTSTR szString)
{
	UINT nLength = lstrlen(szString);
	if (szLog == NULL)
	{
		nLogLength = nLength + 3;			// CR+LF+\0
		szLog = HeapAlloc(hHeap,0,nLogLength*sizeof(szLog[0]));
		if (szLog==NULL)
		{
			nLogLength = 0;
			return;
		}
		lstrcpy(szLog,szString);
	}
	else
	{
		LPTSTR szLogTmp = HeapReAlloc(hHeap,0,szLog,(nLogLength+nLength+2)*sizeof(szLog[0]));
		if (szLogTmp == NULL)
		{
			ClearLog();
			return;
		}
		szLog = szLogTmp;
		lstrcpy(&szLog[nLogLength-1],szString);
		nLogLength += nLength + 2;			// CR+LF
	}
	szLog[nLogLength-3] = _T('\r');
	szLog[nLogLength-2] = _T('\n');
	szLog[nLogLength-1] = 0;
	return;
}

static VOID __cdecl PrintfToLog(LPCTSTR lpFormat, ...)
{
	LPTSTR lpOutput;
	va_list arglist;

	va_start(arglist,lpFormat);
	lpOutput = HeapAlloc(hHeap,0,1024 * sizeof(lpOutput[0]));
	wvsprintf(lpOutput,lpFormat,arglist);
	AddToLog(lpOutput);
	HeapFree(hHeap,0,lpOutput);
	va_end(arglist);
	return;
}

static BOOL CALLBACK KMLLogProc(HWND hDlg, UINT message, DWORD wParam, LONG lParam)
{
	LPTSTR szString;

	switch (message)
	{
	case WM_INITDIALOG:
		// set OK
		EnableWindow(GetDlgItem(hDlg,IDOK),bKmlLogOkEnabled);
		// set IDC_TITLE
		szString = GetStringParam(pKml, TOK_GLOBAL, TOK_TITLE, 0);
		if (szString == NULL) szString = _T("Untitled");
		SetDlgItemText(hDlg,IDC_TITLE,szString);
		// set IDC_AUTHOR
		szString = GetStringParam(pKml, TOK_GLOBAL, TOK_AUTHOR, 0);
		if (szString == NULL) szString = _T("<Unknown Author>");
		SetDlgItemText(hDlg,IDC_AUTHOR,szString);
		// set IDC_KMLLOG
		if (szLog == NULL)
			SetDlgItemText(hDlg,IDC_KMLLOG,_T("Memory Allocation Failure."));
		else
			SetDlgItemText(hDlg,IDC_KMLLOG,szLog);
		// set IDC_ALWAYSDISPLOG
		CheckDlgButton(hDlg,IDC_ALWAYSDISPLOG,bAlwaysDisplayLog);
		// redraw window
		InvalidateRect(hDlg, NULL, TRUE);
		return TRUE;
	case WM_COMMAND:
		wParam = LOWORD(wParam);
		if ((wParam==IDOK)||(wParam==IDCANCEL))
		{
			bAlwaysDisplayLog = IsDlgButtonChecked(hDlg, IDC_ALWAYSDISPLOG);
			EndDialog(hDlg, wParam);
			return TRUE;
		}
		break;
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
	UNREFERENCED_PARAMETER(hDlg);
}

BOOL DisplayKMLLog(BOOL bOkEnabled)
{
	BOOL bResult;
	bKmlLogOkEnabled = bOkEnabled;
	bResult = DialogBox(hApp, MAKEINTRESOURCE(IDD_KMLLOG), hWnd, (DLGPROC)KMLLogProc);
	return (bResult == IDOK);
}



//################
//#
//#    Choose Script
//#
//################

typedef struct _KmlScript
{
	LPTSTR szFilename;
	LPTSTR szTitle;
	DWORD   nId;
	struct _KmlScript* pNext;
} KmlScript;

static UINT nKmlFiles;
static KmlScript* pKmlList;
static CHAR cKmlType;

static VOID DestroyKmlList(VOID)
{
	KmlScript* pList;

	while (pKmlList)
	{
		pList = pKmlList->pNext;
		HeapFree(hHeap,0,pKmlList->szFilename);
		HeapFree(hHeap,0,pKmlList->szTitle);
		HeapFree(hHeap,0,pKmlList);
		pKmlList = pList;
	}
	nKmlFiles = 0;
	return;
}

static VOID CreateKmlList(VOID)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA pFindFileData;

	SetCurrentDirectory(szEmuDirectory);
	hFindFile = FindFirstFile(_T("*.KML"),&pFindFileData);
	SetCurrentDirectory(szCurrentDirectory);
	nKmlFiles = 0;
	if (hFindFile == INVALID_HANDLE_VALUE) return;
	do
	{
		KmlScript* pScript;
		KmlBlock*  pBlock;
		LPTSTR szTitle;

		pBlock = LoadKMLGlobal(pFindFileData.cFileName);
		if (pBlock == NULL) continue;
		// check for correct KML script platform
		szTitle = GetStringParam(pBlock,TOK_GLOBAL,TOK_HARDWARE,0);
		if (szTitle && lstrcmpi(_T(HARDWARE),szTitle) != 0)
		{
			FreeBlocks(pBlock);
			continue;
		}
		if (cKmlType)
		{
			szTitle = GetStringParam(pBlock,TOK_GLOBAL,TOK_MODEL,0);
			if ((!szTitle)||(szTitle[0]!=cKmlType))
			{
				FreeBlocks(pBlock);
				continue;
			}
		}
		pScript = HeapAlloc(hHeap,0,sizeof(KmlScript));
		pScript->szFilename = DuplicateString(pFindFileData.cFileName);
		szTitle = GetStringParam(pBlock,TOK_GLOBAL,TOK_TITLE,0);
		if (szTitle == NULL) szTitle = DuplicateString(pScript->szFilename);
		pScript->szTitle = DuplicateString(szTitle);
		FreeBlocks(pBlock);
		pScript->nId = nKmlFiles;
		pScript->pNext = pKmlList;
		pKmlList = pScript;
		nKmlFiles++;
	} while (FindNextFile(hFindFile,&pFindFileData));
	FindClose(hFindFile);
	return;
};

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];

	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		SendMessage(hwnd,BFFM_SETSELECTION,TRUE,pData);
		break;
	case BFFM_SELCHANGED:
		// Set the status window to the currently selected path.
        if (SHGetPathFromIDList((LPITEMIDLIST) lp,szDir))
		{
			SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM) szDir);
		}
		break;
	}
	return 0;
}

static VOID BrowseFolder(HWND hDlg)
{
	TCHAR szDir[MAX_PATH];
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc;

	// gets the shell's default allocator
	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		GetDlgItemText(hDlg,IDC_EMUDIR,szDir,ARRAYSIZEOF(szDir));

		ZeroMemory(&bi,sizeof(bi));
		bi.hwndOwner = hDlg;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = NULL;
		bi.lpszTitle = _T("Choose a folder:");
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM) szDir;			// current setting

		pidl = SHBrowseForFolder(&bi);
		if (pidl)
		{
			if (SHGetPathFromIDList(pidl,szDir))
			{
				SetDlgItemText(hDlg,IDC_EMUDIR,szDir);
			}
			// free the PIDL allocated by SHBrowseForFolder
			pMalloc->lpVtbl->Free(pMalloc,pidl);
		}
		// release the shell's allocator
		pMalloc->lpVtbl->Release(pMalloc);
	}
	return;
}

static BOOL CALLBACK ChooseKMLProc(HWND hDlg, UINT message, DWORD wParam, LONG lParam)
{
	HWND hList;
	KmlScript* pList;
	UINT nIndex;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg,IDC_EMUDIR,szEmuDirectory);
		hList = GetDlgItem(hDlg,IDC_KMLSCRIPT);
		SendMessage(hList, CB_RESETCONTENT, 0, 0);
		pList = pKmlList;
		while (pList)
		{
			nIndex = SendMessage(hList, CB_ADDSTRING, 0, (LPARAM)pList->szTitle);
			SendMessage(hList, CB_SETITEMDATA, nIndex, (LPARAM)pList->nId);
			pList = pList->pNext;
		}
		SendMessage(hList, CB_SETCURSEL, 0, 0);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_EMUDIRSEL:
			BrowseFolder(hDlg);				// select new folder for IDC_EMUDIR
			// fall into IDC_UPDATE to search for KML files in new folder 
		case IDC_UPDATE:
			DestroyKmlList();
			GetDlgItemText(hDlg,IDC_EMUDIR,szEmuDirectory,ARRAYSIZEOF(szEmuDirectory));
			CreateKmlList();
			hList = GetDlgItem(hDlg,IDC_KMLSCRIPT);
			SendMessage(hList, CB_RESETCONTENT, 0, 0);
			pList = pKmlList;
			while (pList)
			{
				nIndex = SendMessage(hList, CB_ADDSTRING, 0, (LPARAM)pList->szTitle);
				SendMessage(hList, CB_SETITEMDATA, nIndex, (LPARAM)pList->nId);
				pList = pList->pNext;
			}
			SendMessage(hList, CB_SETCURSEL, 0, 0);
			return TRUE;
		case IDOK:
			GetDlgItemText(hDlg,IDC_EMUDIR,szEmuDirectory,ARRAYSIZEOF(szEmuDirectory));
			hList = GetDlgItem(hDlg,IDC_KMLSCRIPT);
			nIndex = SendMessage(hList, CB_GETCURSEL, 0, 0);
			nIndex = SendMessage(hList, CB_GETITEMDATA, nIndex, 0);
			pList = pKmlList;
			while (pList)
			{
				if (pList->nId == nIndex) break;
				pList = pList->pNext;
			}
			if (pList)
			{
				lstrcpy(szCurrentKml, pList->szFilename);
				EndDialog(hDlg, IDOK);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
	}
	return FALSE;
	UNREFERENCED_PARAMETER(lParam);
}

BOOL DisplayChooseKml(CHAR cType)
{
	BOOL bResult;
	cKmlType = cType;
	CreateKmlList();
	bResult = DialogBox(hApp, MAKEINTRESOURCE(IDD_CHOOSEKML), hWnd, (DLGPROC)ChooseKMLProc);
	DestroyKmlList();
	return (bResult == IDOK);
}



//################
//#
//#    KML File Mapping
//#
//################

static LPTSTR MapKMLFile(HANDLE hFile)
{
	DWORD  lBytesRead;
	DWORD  dwFileSizeLow;
	DWORD  dwFileSizeHigh;
	LPTSTR lpBuf = NULL;

	dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
	if (dwFileSizeHigh != 0)
	{
		AddToLog(_T("File is too large."));
		goto fail;
	}

	lpBuf = HeapAlloc(hHeap,0,(dwFileSizeLow+1)*sizeof(lpBuf[0]));
	if (lpBuf == NULL)
	{
		PrintfToLog(_T("Cannot allocate %i bytes."), (dwFileSizeLow+1)*sizeof(lpBuf[0]));
		goto fail;
	}
	#if defined _UNICODE
	{
		LPSTR szTmp = HeapAlloc(hHeap,0,dwFileSizeLow+1);
		if (szTmp == NULL)
		{
			HeapFree(hHeap,0,lpBuf);
			lpBuf = NULL;
			PrintfToLog(_T("Cannot allocate %i bytes."), dwFileSizeLow+1);
			goto fail;
		}
		ReadFile(hFile, szTmp, dwFileSizeLow, &lBytesRead, NULL);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szTmp, lBytesRead, lpBuf, dwFileSizeLow+1);
		HeapFree(hHeap,0,szTmp);
	}
	#else
	{
		ReadFile(hFile, lpBuf, dwFileSizeLow, &lBytesRead, NULL);
	}
	#endif
	lpBuf[dwFileSizeLow] = 0;

fail:
	CloseHandle(hFile);
	return lpBuf;
}



//################
//#
//#    Script Parsing
//#
//################

static VOID FatalError(VOID)
{
	PrintfToLog(_T("Fatal Error at line %i"), nLexLine);
	szText[0] = 0;
	return;
}

static VOID InitLex(LPTSTR szScript)
{
	nLexLine = 1;
	szText = szScript;
	return;
}

static VOID CleanLex(VOID)
{
	nLexLine = 0;
	nLexInteger = 0;
	szLexString = NULL;
	szText = NULL;
	return;
}

// TODO: Change this poor (and slow!) code
static BOOL IsBlock(TokenId eId)
{
	UINT uBlock = 0;
	while (eIsBlock[uBlock] != TOK_NONE)
	{
		if (eId == eIsBlock[uBlock]) return TRUE;
		uBlock++;
	}
	return FALSE;
}


static LPCTSTR GetStringOf(TokenId eId)
{
	UINT i = 0;
	while (pLexToken[i].nLen)
	{
		if (pLexToken[i].eId == eId) return pLexToken[i].szName;
		i++;
	}
	return _T("<Undefined>");
}

static VOID SkipWhite(UINT nMode)
{
	UINT i;
loop:
	i = 0;
	while (szLexDelim[nMode][i])
	{
		if (*szText == szLexDelim[nMode][i]) break;
		i++;
	}
	if (szLexDelim[nMode][i] != 0)
	{
		if (szLexDelim[nMode][i]==_T('\n'))	nLexLine++;
		szText++;
		goto loop;
	}
	if (*szText==_T('#'))
	{
		do szText++; while (*szText != _T('\n'));
		if (nMode != LEX_PARAM) goto loop;
	}
	return;
}

static TokenId ParseToken(UINT nMode)
{
	UINT i, j, k;
	i = 0;
	while (szText[i])
	{
		j = 0;
		while (szLexDelim[nMode][j])
		{
			if (szLexDelim[nMode][j] == szText[i]) break;
			j++;
		}
		if (szLexDelim[nMode][j] == _T('\n')) nLexLine++;
		if (szLexDelim[nMode][j] != 0) break;
		i++;
	}
	if (i==0)
	{
		return TOK_NONE;
	}
	j = 0;
	while (pLexToken[j].nLen)
	{
		if (pLexToken[j].nLen>i)
		{
			j++;
			continue;
		}
		if (pLexToken[j].nLen<i) break;
		k = 0;
		if (_tcsncmp(pLexToken[j].szName, szText, i)==0)
		{
			szText += i;
			return pLexToken[j].eId;
		}
		j++;
	}
	szText[i] = 0;
	if (bDebug)
	{
		PrintfToLog(_T("%i: Undefined token %s"), nLexLine, szText);
		return TOK_NONE;
	}
	return TOK_NONE;
}

static DWORD ParseInteger(VOID)
{
	DWORD nNum = 0;
	while (_istdigit(*szText))
	{
		nNum = nNum * 10 + ((*szText) - _T('0'));
		szText++;
	} 
	return nNum;
}

static LPTSTR ParseString(VOID)
{
	LPTSTR szString;
	LPTSTR szBuffer;
	UINT   nLength;
	UINT   nBlock;

	szText++;
	nLength = 0;
	nBlock = 255;
	szBuffer = HeapAlloc(hHeap,0,(nBlock+1) * sizeof(szBuffer[0]));
	while (*szText != _T('"'))
	{
		if (*szText == _T('\\')) szText++;
		if (*szText == 0)
		{
			FatalError();
			return NULL;
		}
		szBuffer[nLength] = *szText;
		nLength++;
		if (nLength == nBlock)
		{
			nBlock += 256;
			szBuffer = HeapReAlloc(hHeap,0,szBuffer,(nBlock+1) * sizeof(szBuffer[0]));
		}
		szText++;
	}
	szText++;
	szBuffer[nLength] = 0;
	szString = DuplicateString(szBuffer);
	HeapFree(hHeap,0,szBuffer);
	return szString;
}		

static TokenId Lex(UINT nMode)
{
	SkipWhite(nMode);
	if (_istdigit(*szText))
	{
		nLexInteger = ParseInteger();
		return TOK_INTEGER;
	}
	if (*szText == _T('"'))
	{
		szLexString = ParseString();
		return TOK_STRING;
	}
	if ((nMode == LEX_PARAM) && (*szText == _T('\n')))
	{
		nLexLine++;
		szText++;
		return TOK_EOL;
	}
	return ParseToken(nMode);
}

static KmlLine* ParseLine(TokenId eCommand)
{
	UINT     i, j;
	DWORD    nParams;
	TokenId  eToken;
	KmlLine* pLine;

	i = 0;
	while (pLexToken[i].nLen)
	{
		if (pLexToken[i].eId == eCommand) break;
		i++;
	}
	if (pLexToken[i].nLen == 0) return NULL;

	j = 0;
	pLine = HeapAlloc(hHeap,HEAP_ZERO_MEMORY,sizeof(KmlLine));
	pLine->eCommand = eCommand;
	nParams = pLexToken[i].nParams;
loop:
	eToken = Lex(LEX_PARAM);
	if ((nParams&7)==TYPE_NONE)
	{
		if (eToken != TOK_EOL)
		{
			PrintfToLog(_T("%i: Too many parameters (%i expected)."), nLexLine, j);
			goto errline;					// free memory of arguments
		}
		return pLine;
	}
	if ((nParams&7)==TYPE_INTEGER)
	{
		if (eToken != TOK_INTEGER)
		{
			PrintfToLog(_T("%i: Parameter %i of %s must be an integer."), nLexLine, j+1, pLexToken[i].szName);
			goto errline;					// free memory of arguments
		}
		pLine->nParam[j++] = nLexInteger;
		nParams >>= 3;
		goto loop;
	}
	if ((nParams&7)==TYPE_STRING)
	{
		if (eToken != TOK_STRING)
		{
			PrintfToLog(_T("%i: Parameter %i of %s must be a string."), nLexLine, j+1, pLexToken[i].szName);
			goto errline;					// free memory of arguments
		}
		pLine->nParam[j++] = (DWORD)szLexString;
		nParams >>= 3;
		goto loop;
	}
	AddToLog(_T("Oops..."));
errline:
	// if last argument was string, free it
	if (eToken == TOK_STRING) HeapFree(hHeap,0,szLexString);

	nParams = pLexToken[i].nParams;			// get argument types of command
	for (i=0; i<j; i++)						// handle all scanned arguments
	{
		if ((nParams&7) == TYPE_STRING)		// string type
		{
			HeapFree(hHeap,0,(LPVOID)pLine->nParam[i]);
		}
		nParams >>= 3;						// next argument type
	}
	HeapFree(hHeap,0,pLine);
	return NULL;
}

static KmlLine* IncludeLines(LPCTSTR szFilename)
{
	HANDLE   hFile;
	LPTSTR   lpbyBuf;
	UINT     uOldLine;
	LPTSTR   szOldText;
	KmlLine* pLine;

	SetCurrentDirectory(szEmuDirectory);
	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	SetCurrentDirectory(szCurrentDirectory);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		PrintfToLog(_T("Error while opening include file %s."), szFilename);
		FatalError();
		return NULL;
	}
	if ((lpbyBuf = MapKMLFile(hFile)) == NULL)
	{
		FatalError();
		return NULL;
	}

	uOldLine  = nLexLine;
	szOldText = szText;

	nLinesIncludeLevel++;
	PrintfToLog(_T("l%i:Including %s"), nLinesIncludeLevel, szFilename);
	InitLex(lpbyBuf);
	pLine = ParseLines();
	CleanLex();
	nLinesIncludeLevel--;

	nLexLine  = uOldLine;
	szText    = szOldText;
	HeapFree(hHeap,0,lpbyBuf);

	return pLine;
}

static KmlLine* ParseLines(VOID)
{
	KmlLine* pFirst = NULL;
	KmlLine* pLine  = NULL;
	TokenId  eToken;
	UINT     nLevel = 0;

	while ((eToken = Lex(LEX_COMMAND)))
	{
		if (IsBlock(eToken)) nLevel++;
		if (eToken == TOK_INCLUDE)
		{
			LPTSTR szFilename;
			eToken = Lex(LEX_PARAM);		// get include parameter in 'szLexString'
			if (eToken != TOK_STRING)		// not a string (token don't begin with ")
			{
				AddToLog(_T("Include: string expected as parameter."));
				FatalError();
				goto abort;
			}
			szFilename = szLexString;		// save pointer to allocated memory
			if (pFirst)
			{
				pLine->pNext = IncludeLines(szLexString);
			}
			else
			{
				pLine = pFirst = IncludeLines(szLexString);
			}
			HeapFree(hHeap,0,szFilename);	// free memory
			if (pLine == NULL)				// parsing error
				goto abort;
			while (pLine->pNext) pLine=pLine->pNext;
			continue;
		}
		if (eToken == TOK_END)
		{
			if (nLevel)
			{
				nLevel--;
			}
			else
			{
				if (pLine) pLine->pNext = NULL;
				return pFirst;
			}
		}
		if (pFirst)
		{
			pLine = pLine->pNext = ParseLine(eToken);
		}
		else
		{
			pLine = pFirst = ParseLine(eToken);
		}
		if (pLine == NULL)					// parsing error
			goto abort;
	}
	if (nLinesIncludeLevel)
	{
		if (pLine) pLine->pNext = NULL;
		return pFirst;
	}	
	AddToLog(_T("Open block."));
abort:
	if (pFirst)
	{
		FreeLines(pFirst);
	}
	return NULL;
}

static KmlBlock* ParseBlock(TokenId eType)
{
	UINT      u1;
	KmlBlock* pBlock;
	TokenId   eToken;

	nLinesIncludeLevel = 0;

	pBlock = HeapAlloc(hHeap,HEAP_ZERO_MEMORY,sizeof(KmlBlock));
	pBlock->eType = eType;

	u1 = 0;
	while (pLexToken[u1].nLen)
	{
		if (pLexToken[u1].eId == eType) break;
		u1++;
	}
	if (pLexToken[u1].nParams)
	{
		eToken = Lex(LEX_COMMAND);
		switch (eToken)
		{
		case TOK_NONE:
			AddToLog(_T("Open Block at End Of File."));
			HeapFree(hHeap,0,pBlock);
			FatalError();
			return NULL;
		case TOK_INTEGER:
			if ((pLexToken[u1].nParams&7)!=TYPE_INTEGER)
			{
				AddToLog(_T("Wrong block argument."));
				HeapFree(hHeap,0,pBlock);
				FatalError();
				return NULL;
			}
			pBlock->nId = nLexInteger;
			break;
		default:
			AddToLog(_T("Wrong block argument."));
			HeapFree(hHeap,0,pBlock);
			FatalError();
			return NULL;
		}
	}

	pBlock->pFirstLine = ParseLines();

	if (pBlock->pFirstLine == NULL)			// break on ParseLines error
	{
		HeapFree(hHeap,0,pBlock);
		pBlock = NULL;
	}

	return pBlock;
}

static KmlBlock* IncludeBlocks(LPCTSTR szFilename)
{
	HANDLE    hFile;
	LPTSTR    lpbyBuf;
	UINT      uOldLine;
	LPTSTR    szOldText;
	KmlBlock* pFirst;

	SetCurrentDirectory(szEmuDirectory);
	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	SetCurrentDirectory(szCurrentDirectory);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		PrintfToLog(_T("Error while opening include file %s."), szFilename);
		FatalError();
		return NULL;
	}
	if ((lpbyBuf = MapKMLFile(hFile)) == NULL)
	{
		FatalError();
		return NULL;
	}

	uOldLine  = nLexLine;
	szOldText = szText;

	nBlocksIncludeLevel++;
	PrintfToLog(_T("b%i:Including %s"), nBlocksIncludeLevel, szFilename);
	InitLex(lpbyBuf);
	pFirst = ParseBlocks();
	CleanLex();
	nBlocksIncludeLevel--;

	nLexLine  = uOldLine;
	szText    = szOldText;
	HeapFree(hHeap,0,lpbyBuf);

	return pFirst;
}

static KmlBlock* ParseBlocks(VOID)
{
	TokenId eToken;
	KmlBlock* pFirst = NULL;
	KmlBlock* pBlock = NULL;

	while ((eToken=Lex(LEX_BLOCK))!=TOK_NONE)
	{
		if (eToken == TOK_INCLUDE)
		{
			LPTSTR szFilename;
			eToken = Lex(LEX_PARAM);		// get include parameter in 'szLexString'
			if (eToken != TOK_STRING)		// not a string (token don't begin with ")
			{
				AddToLog(_T("Include: string expected as parameter."));
				FatalError();
				goto abort;
			}
			szFilename = szLexString;		// save pointer to allocated memory
			if (pFirst)
				pBlock = pBlock->pNext = IncludeBlocks(szLexString);
			else
				pBlock = pFirst = IncludeBlocks(szLexString);
			HeapFree(hHeap,0,szFilename);	// free memory
			if (pBlock == NULL)				// parsing error
				goto abort;
			while (pBlock->pNext) pBlock=pBlock->pNext;
			continue;
		}
		if (pFirst)
			pBlock = pBlock->pNext = ParseBlock(eToken);
		else
			pBlock = pFirst = ParseBlock(eToken);
		if (pBlock == NULL)
		{
			AddToLog(_T("Invalid block."));
			FatalError();
			goto abort;
		}
	}
	if (pFirst) pBlock->pNext = NULL;
	return pFirst;
abort:
	if (pFirst) FreeBlocks(pFirst);
	return NULL;
}



//################
//#
//#    Initialization Phase
//#
//################

static VOID InitGlobal(KmlBlock* pBlock)
{
	KmlLine* pLine = pBlock->pFirstLine;
	while (pLine)
	{
		switch (pLine->eCommand)
		{
		case TOK_TITLE:
			PrintfToLog(_T("Title: %s"), (LPTSTR)pLine->nParam[0]);
			break;
		case TOK_AUTHOR:
			PrintfToLog(_T("Author: %s"), (LPTSTR)pLine->nParam[0]);
			break;
		case TOK_PRINT:
			AddToLog((LPTSTR)pLine->nParam[0]);
			break;
		case TOK_HARDWARE:
			PrintfToLog(_T("Hardware Platform: %s"), (LPTSTR)pLine->nParam[0]);
			break;
		case TOK_MODEL:
			cCurrentRomType = ((BYTE *)pLine->nParam[0])[0];
			PrintfToLog(_T("Calculator Model : %c"), cCurrentRomType);
			break;
		case TOK_CLASS:
			nCurrentClass = pLine->nParam[0];
			PrintfToLog(_T("Calculator Class : %u"), nCurrentClass);
			break;
		case TOK_DEBUG:
			bDebug = pLine->nParam[0]&1;
			PrintfToLog(_T("Debug %s"), bDebug?_T("On"):_T("Off"));
			break;
		case TOK_ROM:
			if (pbyRom != NULL)
			{
				PrintfToLog(_T("Rom %s Ignored."), (LPTSTR)pLine->nParam[0]);
				AddToLog(_T("Please put only one Rom command in the Global block."));
				break;
			}
			if (!MapRom((LPTSTR)pLine->nParam[0]))
			{
				PrintfToLog(_T("Cannot open Rom %s"), (LPTSTR)pLine->nParam[0]);
				break;
			}
			PrintfToLog(_T("Rom %s Loaded."), (LPTSTR)pLine->nParam[0]);
			break;
		case TOK_PATCH:
			if (pbyRom == NULL)
			{
				PrintfToLog(_T("Patch %s ignored."), (LPTSTR)pLine->nParam[0]);
				AddToLog(_T("Please put the Rom command before any Patch."));
				break;
			}
			if (PatchRom((LPTSTR)pLine->nParam[0]) == TRUE)
				PrintfToLog(_T("Patch %s Loaded"), (LPTSTR)pLine->nParam[0]);
			else
				PrintfToLog(_T("Patch %s is Wrong or Missing"), (LPTSTR)pLine->nParam[0]);
			break;
		case TOK_BITMAP:
			if (hMainDC != NULL)
			{
				PrintfToLog(_T("Bitmap %s Ignored."), (LPTSTR)pLine->nParam[0]);
				AddToLog(_T("Please put only one Bitmap command in the Global block."));
				break;
			}
			if (!CreateMainBitmap((LPTSTR)pLine->nParam[0]))
			{
				PrintfToLog(_T("Cannot Load Bitmap %s."), (LPTSTR)pLine->nParam[0]);
				break;
			}
			PrintfToLog(_T("Bitmap %s Loaded."), (LPTSTR)pLine->nParam[0]);
			break;
		default:
			PrintfToLog(_T("Command %s Ignored in Block %s"), GetStringOf(pLine->eCommand), GetStringOf(pBlock->eType));
		}
		pLine = pLine->pNext;
	}
	return;
}

static KmlLine* InitBackground(KmlBlock* pBlock)
{
	KmlLine* pLine = pBlock->pFirstLine;
	while (pLine)
	{
		switch (pLine->eCommand)
		{
		case TOK_OFFSET:
			nBackgroundX = pLine->nParam[0];
			nBackgroundY = pLine->nParam[1];
			break;
		case TOK_SIZE:
			nBackgroundW = pLine->nParam[0];
			nBackgroundH = pLine->nParam[1];
			break;
		case TOK_END:
			return pLine;
		default:
			PrintfToLog(_T("Command %s Ignored in Block %s"), GetStringOf(pLine->eCommand), GetStringOf(pBlock->eType));
		}
		pLine = pLine->pNext;
	}
	return NULL;
}

static KmlLine* InitLcd(KmlBlock* pBlock)
{
	KmlLine* pLine = pBlock->pFirstLine;
	while (pLine)
	{
		switch (pLine->eCommand)
		{
		case TOK_OFFSET:
			nLcdX = pLine->nParam[0];
			nLcdY = pLine->nParam[1];
			break;
		case TOK_ZOOM:
			nLcdZoom = pLine->nParam[0];
			if (nLcdZoom != 1 && nLcdZoom != 2 && nLcdZoom != 4)
				nLcdZoom = 1;
			break;
		case TOK_COLOR:
			SetLcdColor(pLine->nParam[0],pLine->nParam[1],pLine->nParam[2],pLine->nParam[3]);
			break;
		case TOK_END:
			return pLine;
		default:
			PrintfToLog(_T("Command %s Ignored in Block %s"), GetStringOf(pLine->eCommand), GetStringOf(pBlock->eType));
		}
		pLine = pLine->pNext;
	}
	return NULL;
}

static KmlLine* InitAnnunciator(KmlBlock* pBlock)
{
	KmlLine* pLine = pBlock->pFirstLine;
	UINT nId = pBlock->nId-1;
	if (nId >= ARRAYSIZEOF(pAnnunciator))
	{
		PrintfToLog(_T("Wrong Annunciator Id %i"), nId);
		return NULL;
	}
	nAnnunciators++;
	while (pLine)
	{
		switch (pLine->eCommand)
		{
		case TOK_OFFSET:
			pAnnunciator[nId].nOx = pLine->nParam[0];
			pAnnunciator[nId].nOy = pLine->nParam[1];
			break;
		case TOK_DOWN:
			pAnnunciator[nId].nDx = pLine->nParam[0];
			pAnnunciator[nId].nDy = pLine->nParam[1];
			break;
		case TOK_SIZE:
			pAnnunciator[nId].nCx = pLine->nParam[0];
			pAnnunciator[nId].nCy = pLine->nParam[1];
			break;
		case TOK_END:
			return pLine;
		default:
			PrintfToLog(_T("Command %s Ignored in Block %s"), GetStringOf(pLine->eCommand), GetStringOf(pBlock->eType));
		}
		pLine = pLine->pNext;
	}
	return NULL;
}

static VOID InitButton(KmlBlock* pBlock)
{
	KmlLine* pLine = pBlock->pFirstLine;
	UINT nLevel = 0;
	if (nButtons>=256)
	{
		AddToLog(_T("Only the first 256 buttons will be defined."));
		return;
	}
	pButton[nButtons].nId = pBlock->nId;
	pButton[nButtons].bDown = FALSE;
	pButton[nButtons].nType = 0; // default : user defined button
	while (pLine)
	{
		if (nLevel)
		{
			if (pLine->eCommand == TOK_END) nLevel--;
			pLine = pLine->pNext;
			continue;
		}
		if (IsBlock(pLine->eCommand)) nLevel++;
		switch (pLine->eCommand)
		{
		case TOK_TYPE:
			pButton[nButtons].nType = pLine->nParam[0];
			break;
		case TOK_OFFSET:
			pButton[nButtons].nOx = pLine->nParam[0];
			pButton[nButtons].nOy = pLine->nParam[1];
			break;
		case TOK_DOWN:
			pButton[nButtons].nDx = pLine->nParam[0];
			pButton[nButtons].nDy = pLine->nParam[1];
			break;
		case TOK_SIZE:
			pButton[nButtons].nCx = pLine->nParam[0];
			pButton[nButtons].nCy = pLine->nParam[1];
			break;
		case TOK_OUTIN:
			pButton[nButtons].nOut = pLine->nParam[0];
			pButton[nButtons].nIn  = pLine->nParam[1];
			break;
		case TOK_ONDOWN:
			pButton[nButtons].pOnDown = pLine;
			break;
		case TOK_ONUP:
			pButton[nButtons].pOnUp = pLine;
			break;
		case TOK_NOHOLD:
			pButton[nButtons].dwFlags &= ~(BUTTON_VIRTUAL);
			pButton[nButtons].dwFlags |= BUTTON_NOHOLD;
			break;
		case TOK_VIRTUAL:
			pButton[nButtons].dwFlags &= ~(BUTTON_NOHOLD);
			pButton[nButtons].dwFlags |= BUTTON_VIRTUAL;
			break;
		default:
			PrintfToLog(_T("Command %s Ignored in Block %s %i"), GetStringOf(pLine->eCommand), GetStringOf(pBlock->eType), pBlock->nId);
		}
		pLine = pLine->pNext;
	}
	if (nLevel)
		PrintfToLog(_T("%i Open Block(s) in Block %s %i"), nLevel, GetStringOf(pBlock->eType), pBlock->nId);
	nButtons++;
	return;
}



//################
//#
//#    Execution
//#
//################

static KmlLine* SkipLines(KmlLine* pLine, TokenId eCommand)
{
	UINT nLevel = 0;
	while (pLine)
	{
		if (IsBlock(pLine->eCommand)) nLevel++;
		if (pLine->eCommand==eCommand)
		{
			if (nLevel == 0) return pLine->pNext;
		}
		if (pLine->eCommand == TOK_END)
		{
			if (nLevel)
				nLevel--;
			else
				return NULL;
		}
		pLine = pLine->pNext;
	}
	return pLine;
}

static KmlLine* If(KmlLine* pLine, BOOL bCondition)
{
	pLine = pLine->pNext;
	if (bCondition)
	{
		while (pLine)
		{
			if (pLine->eCommand == TOK_END)
			{
				pLine = pLine->pNext;
				break;
			}
			if (pLine->eCommand == TOK_ELSE)
			{
				pLine = SkipLines(pLine, TOK_END);
				break;
			}
			pLine = RunLine(pLine);
		}
	}
	else
	{
		pLine = SkipLines(pLine, TOK_ELSE);
		while (pLine)
		{
			if (pLine->eCommand == TOK_END)
			{
				pLine = pLine->pNext;
				break;
			}
			pLine = RunLine(pLine);
		}
	}
	return pLine;
}

static KmlLine* RunLine(KmlLine* pLine)
{
	switch (pLine->eCommand)
	{
	case TOK_MAP:
		if (byVKeyMap[pLine->nParam[0]&0xFF]&1)
			PressButtonById(pLine->nParam[1]);
		else
			ReleaseButtonById(pLine->nParam[1]);
		break;
	case TOK_PRESS:
		PressButtonById(pLine->nParam[0]);
		break;
	case TOK_RELEASE:
		ReleaseButtonById(pLine->nParam[0]);
		break;
	case TOK_MENUITEM:
		PostMessage(hWnd, WM_COMMAND, 0x19C40+(pLine->nParam[0]&0xFF), 0);
		break;
	case TOK_SETFLAG:
		nKMLFlags |= 1<<(pLine->nParam[0]&0x1F);
		break;
	case TOK_RESETFLAG:
		nKMLFlags &= ~(1<<(pLine->nParam[0]&0x1F));
		break;
	case TOK_NOTFLAG:
		nKMLFlags ^= 1<<(pLine->nParam[0]&0x1F);
		break;
	case TOK_IFPRESSED:
		return If(pLine,byVKeyMap[pLine->nParam[0]&0xFF]);
		break;
	case TOK_IFFLAG:
		return If(pLine,(nKMLFlags>>(pLine->nParam[0]&0x1F))&1);
	default:
		break;
	}
	return pLine->pNext;
}



//################
//#
//#    Clean Up
//#
//################

static VOID FreeLines(KmlLine* pLine)
{
	while (pLine)
	{
		KmlLine* pThisLine = pLine;
		UINT i = 0;
		DWORD nParams;
		while (pLexToken[i].nLen)			// search in all token definitions
		{
			// break when token definition found
			if (pLexToken[i].eId == pLine->eCommand) break;
			i++;							// next token definition
		}
		nParams = pLexToken[i].nParams;		// get argument types of command
		i = 0;								// first parameter
		while ((nParams&7))					// argument left
		{
			if ((nParams&7) == TYPE_STRING)	// string type
			{
				HeapFree(hHeap,0,(LPVOID)pLine->nParam[i]);
			}
			i++;							// incr. parameter buffer index
			nParams >>= 3;					// next argument type
		}
		pLine = pLine->pNext;				// get next line
		HeapFree(hHeap,0,pThisLine);
	}
	return;
}

VOID FreeBlocks(KmlBlock* pBlock)
{
	while (pBlock)
	{
		KmlBlock* pThisBlock = pBlock;
		pBlock = pBlock->pNext;
		FreeLines(pThisBlock->pFirstLine);
		HeapFree(hHeap,0,pThisBlock);
	}
	return;
}

VOID KillKML(VOID)
{
	if ((nState==SM_RUN)||(nState==SM_SLEEP))
	{
		AbortMessage(_T("FATAL: KillKML while emulator is running !!!"));
		SwitchToState(SM_RETURN);
		DestroyWindow(hWnd);
	}
	UnmapRom();
	DestroyLcdBitmap();
	DestroyMainBitmap();
	if (hPalette)
	{
		BOOL err;

		if (hWindowDC) SelectPalette(hWindowDC, hOldPalette, FALSE);
		err = DeleteObject(hPalette);
		_ASSERT(err != FALSE);				// freed resource memory
		hPalette = NULL;
	}
	bClicking = FALSE;
	uButtonClicked = 0;
	FreeBlocks(pKml);
	pKml = NULL;
	nButtons = 0;
	nScancodes = 0;
	nAnnunciators = 0;
	bDebug = TRUE;
	nKMLFlags = 0;
	ZeroMemory(pButton, sizeof(pButton));
	ZeroMemory(pAnnunciator, sizeof(pAnnunciator));
	ZeroMemory(pVKey, sizeof(pVKey));
	ClearLog();
	nBackgroundX = 0;
	nBackgroundY = 0;
	nBackgroundW = 256;
	nBackgroundH = 0;
	nLcdZoom = 1;
	UpdateWindowStatus();
	ResizeWindow();
	return;
}



//################
//#
//#    Extract Keyword's Parameters
//#
//################

static LPTSTR GetStringParam(KmlBlock* pBlock, TokenId eBlock, TokenId eCommand, UINT nParam)
{
	while (pBlock)
	{
		if (pBlock->eType == eBlock)
		{
			KmlLine* pLine = pBlock->pFirstLine;
			while (pLine)
			{
				if (pLine->eCommand == eCommand)
				{
					return (LPTSTR)pLine->nParam[nParam];
				}
				pLine = pLine->pNext;
			}
		}
		pBlock = pBlock->pNext;
	}
	return NULL;
}

static DWORD GetIntegerParam(KmlBlock* pBlock, TokenId eBlock, TokenId eCommand, UINT nParam)
{
	while (pBlock)
	{
		if (pBlock->eType == eBlock)
		{
			KmlLine* pLine = pBlock->pFirstLine;
			while (pLine)
			{
				if (pLine->eCommand == eCommand)
				{
					return pLine->nParam[nParam];
				}
				pLine = pLine->pNext;
			}
		}
		pBlock = pBlock->pNext;
	}
	return 0;
}



//################
//#
//#    Buttons
//#
//################

static INT iSqrt(INT nNumber)				// integer y=sqrt(x) function
{
	INT m, b = 0, t = nNumber;

	do
	{
		m = (b + t + 1) / 2;				// median number
		if (m * m - nNumber > 0)			// calculate x^2-y
			t = m;							// adjust upper border
		else 
			b = m;							// adjust lower border
	}
	while(t - b > 1);

	return b;
}

static VOID AdjustPixel(UINT x, UINT y, BYTE byOffset)
{
	COLORREF rgb;
	WORD     wB, wG, wR;
					
	rgb = GetPixel(hWindowDC, x, y);

	// adjust color red
	wR = (((WORD) rgb) & 0x00FF) + byOffset;
	if (wR > 0xFF) wR = 0xFF;
	rgb >>= 8;
	// adjust color green
	wG = (((WORD) rgb) & 0x00FF) + byOffset;
	if (wG > 0xFF) wG = 0xFF;
	rgb >>= 8;
	// adjust color blue
	wB = (((WORD) rgb) & 0x00FF) + byOffset;
	if (wB > 0xFF) wB = 0xFF;

	SetPixel(hWindowDC, x, y, RGB(wR,wG,wB));
	return;
}

// draw transparent circle with center coordinates and radius in pixel
static __inline VOID TransparentCircle(UINT cx, UINT cy, UINT r)
{
	#define HIGHADJ 0x80					// color incr. at center
	#define LOWADJ  0x08					// color incr. at border

	INT x, y;
	INT rr = r * r;							// calculate r^2

	// y-rows of circle
	for (y = 0; y < (INT) r; ++y)
	{
		INT yy = y * y;						// calculate y^2

		// x-columns of circle 
		INT nXWidth = iSqrt(rr-yy);

		for (x = 0; x < nXWidth; ++x)
		{
			// color offset, sqrt(x*x+y*y) <= r !!!
			BYTE byOff = HIGHADJ - (BYTE) (iSqrt((x*x+yy) * (HIGHADJ-LOWADJ)*(HIGHADJ-LOWADJ) / rr));
					
								  AdjustPixel(cx+x, cy+y, byOff);
			if (x != 0)			  AdjustPixel(cx-x, cy+y, byOff);
			if (y != 0)			  AdjustPixel(cx+x, cy-y, byOff);
			if (x != 0 && y != 0) AdjustPixel(cx-x, cy-y, byOff);
		}
	}
	return;

	#undef HIGHADJ
	#undef LOWADJ
}

static VOID DrawButton(UINT nId)
{
	UINT x0 = pButton[nId].nOx;
	UINT y0 = pButton[nId].nOy;

	EnterCriticalSection(&csGDILock);		// solving NT GDI problems
	{
		switch (pButton[nId].nType)
		{
		case 0: // bitmap key
			if (pButton[nId].bDown)
			{
				BitBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, hMainDC, pButton[nId].nDx, pButton[nId].nDy, SRCCOPY);
			}
			else
			{
				// update background only
				BitBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, hMainDC, x0, y0, SRCCOPY);
			}
			break;
		case 1: // shift key to right down
			if (pButton[nId].bDown)
			{
				UINT x1 = x0+pButton[nId].nCx-1;
				UINT y1 = y0+pButton[nId].nCy-1;
				BitBlt(hWindowDC, x0+3,y0+3,pButton[nId].nCx-5,pButton[nId].nCy-5,hMainDC,x0+2,y0+2,SRCCOPY);
				SelectObject(hWindowDC, GetStockObject(BLACK_PEN));
				MoveToEx(hWindowDC, x0, y0, NULL); LineTo(hWindowDC, x1, y0);
				MoveToEx(hWindowDC, x0, y0, NULL); LineTo(hWindowDC, x0, y1);
				SelectObject(hWindowDC, GetStockObject(WHITE_PEN));
				MoveToEx(hWindowDC, x1, y0, NULL); LineTo(hWindowDC, x1,   y1);
				MoveToEx(hWindowDC, x0, y1, NULL); LineTo(hWindowDC, x1+1, y1);
			}
			else
			{
				BitBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, hMainDC, x0, y0, SRCCOPY);
			}
			break;
		case 2: // do nothing
			break;
		case 3: // invert key color, even in display
			if (pButton[nId].bDown)
			{
				PatBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, DSTINVERT);
			}
			else
			{
				RECT Rect;
				Rect.left = x0 - nBackgroundX;
				Rect.top  = y0 - nBackgroundY;
				Rect.right  = Rect.left + pButton[nId].nCx;
				Rect.bottom = Rect.top + pButton[nId].nCy;
				InvalidateRect(hWnd, &Rect, FALSE);	// call WM_PAINT for background and display redraw
			}
			break;
		case 4: // bitmap key, even in display
			if (pButton[nId].bDown)
			{
				// update background only
				BitBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, hMainDC, x0, y0, SRCCOPY);
			}
			else
			{
				RECT Rect;
				Rect.left = x0 - nBackgroundX;
				Rect.top  = y0 - nBackgroundY;
				Rect.right  = Rect.left + pButton[nId].nCx;
				Rect.bottom = Rect.top + pButton[nId].nCy;
				InvalidateRect(hWnd, &Rect, FALSE);	// call WM_PAINT for background and display redraw
			}
			break;
		case 5: // transparent circle
			if (pButton[nId].bDown)
			{
				TransparentCircle(x0 + pButton[nId].nCx / 2, // x-center coordinate
								  y0 + pButton[nId].nCy / 2, // y-center coordinate
								  min(pButton[nId].nCx,pButton[nId].nCy) / 2); // radius
			}
			else
			{
				// update background only
				BitBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, hMainDC, x0, y0, SRCCOPY);
			}
			break;
		default: // black key, default drawing on illegal types
			if (pButton[nId].bDown)
			{
				PatBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, BLACKNESS);
			}
			else
			{
				// update background only
				BitBlt(hWindowDC, x0, y0, pButton[nId].nCx, pButton[nId].nCy, hMainDC, x0, y0, SRCCOPY);
			}
		}
		GdiFlush();
	}
	LeaveCriticalSection(&csGDILock);
	return;
}

static VOID PressButton(UINT nId)
{
	if (pButton[nId].bDown) return;			// key already pressed -> exit

	pButton[nId].bDown = TRUE;
	DrawButton(nId);
	if (pButton[nId].nIn)
	{
		KeyboardEvent(TRUE,pButton[nId].nOut,pButton[nId].nIn);
	}
	else
	{
		KmlLine* pLine = pButton[nId].pOnDown;
		while ((pLine)&&(pLine->eCommand!=TOK_END))
		{
			pLine = RunLine(pLine);
		}
	}
	return;
}

static VOID ReleaseButton(UINT nId)
{
	pButton[nId].bDown = FALSE;
	DrawButton(nId);
	if (pButton[nId].nIn)
	{
		KeyboardEvent(FALSE,pButton[nId].nOut,pButton[nId].nIn);
	}
	else
	{
		KmlLine* pLine = pButton[nId].pOnUp;
		while ((pLine)&&(pLine->eCommand!=TOK_END))
		{
			pLine = RunLine(pLine);
		}
	}
	return;
}

static VOID PressButtonById(UINT nId)
{
	UINT i;
	for (i=0; i<nButtons; i++)
	{
		if (nId == pButton[i].nId)
		{
			PressButton(i);
			return;
		}
	}
	return;
}

static VOID ReleaseButtonById(UINT nId)
{
	UINT i;
	for (i=0; i<nButtons; i++)
	{
		if (nId == pButton[i].nId)
		{
			ReleaseButton(i);
			return;
		}
	}
	return;
}

static VOID ReleaseAllButtons(VOID)			// release all buttons
{
	UINT i;
	for (i=0; i<nButtons; i++)				// scan all buttons
	{
		if (pButton[i].bDown)				// button pressed
			ReleaseButton(i);				// release button
	}

	bPressed = FALSE;						// key not pressed
	bClicking = FALSE;						// var uButtonClicked not valid (no virtual or nohold key)
	uButtonClicked = 0;						// set var to default
}

VOID RefreshButtons(RECT *rc)
{
	UINT i;
	for (i=0; i<nButtons; i++) 
	{
		if (   pButton[i].bDown
			&& rc->right  >  (LONG) (pButton[i].nOx)
			&& rc->bottom >  (LONG) (pButton[i].nOy)
		    && rc->left   <= (LONG) (pButton[i].nOx + pButton[i].nCx)
			&& rc->top    <= (LONG) (pButton[i].nOy + pButton[i].nCy))
		{
			// on button type 3 and 5 clear complete key area before drawing
			if (pButton[i].nType == 3 || pButton[i].nType == 5)
			{
				UINT x0 = pButton[i].nOx;
				UINT y0 = pButton[i].nOy;
				EnterCriticalSection(&csGDILock); // solving NT GDI problems
				{
					BitBlt(hWindowDC, x0, y0, pButton[i].nCx, pButton[i].nCy, hMainDC, x0, y0, SRCCOPY);
					GdiFlush();
				}
				LeaveCriticalSection(&csGDILock);
			}
			DrawButton(i);					// redraw pressed button
		}
	}
	return;
}


//################
//#
//#    Annunciators
//#
//################

VOID DrawAnnunciator(UINT nId, BOOL bOn)
{
	UINT nSx,nSy;

	--nId;									// zero based ID
	if (nId >= ARRAYSIZEOF(pAnnunciator)) return;
	if (bOn)
	{
		nSx = pAnnunciator[nId].nDx;		// position of annunciator
		nSy = pAnnunciator[nId].nDy;
	}
	else
	{
		nSx = pAnnunciator[nId].nOx;		// position of background
		nSy = pAnnunciator[nId].nOy;
	}
	EnterCriticalSection(&csGDILock);		// solving NT GDI problems
	{
		BitBlt(hWindowDC,
			pAnnunciator[nId].nOx, pAnnunciator[nId].nOy,
			pAnnunciator[nId].nCx, pAnnunciator[nId].nCy,
			hMainDC,
			nSx, nSy,
			SRCCOPY);
		GdiFlush();
	}
	LeaveCriticalSection(&csGDILock);
	return;
}



//################
//#
//#    Mouse
//#
//################

static BOOL ClipButton(UINT x, UINT y, UINT nId)
{
	x += nBackgroundX;						// source display offset
	y += nBackgroundY;

	return (pButton[nId].nOx<=x)
		&& (pButton[nId].nOy<=y)
		&&(x<(pButton[nId].nOx+pButton[nId].nCx))
		&&(y<(pButton[nId].nOy+pButton[nId].nCy));
}

VOID MouseButtonDownAt(UINT nFlags, DWORD x, DWORD y)
{
	UINT i;
	for (i=0; i<nButtons; i++)
	{
		if (ClipButton(x,y,i))
		{
			if (pButton[i].dwFlags&BUTTON_NOHOLD)
			{
				if (nFlags&MK_LBUTTON)		// use only with left mouse button
				{
					bClicking = TRUE;
					uButtonClicked = i;
					pButton[i].bDown = TRUE;
					DrawButton(i);
				}
				return;
			}
			if (pButton[i].dwFlags&BUTTON_VIRTUAL)
			{
				if (!(nFlags&MK_LBUTTON))	// use only with left mouse button
					return;
				bClicking = TRUE;
				uButtonClicked = i;
			}
			bPressed = TRUE;				// key pressed
			uLastPressedKey = i;			// save pressed key
			PressButton(i);
			return;
		}
	}
}

VOID MouseButtonUpAt(UINT nFlags, DWORD x, DWORD y)
{
	UINT i;
	if (bPressed)							// emulator key pressed
	{
		ReleaseAllButtons();				// release all buttons
		return;
	}
	for (i=0; i<nButtons; i++)
	{
		if (ClipButton(x,y,i))
		{
			if ((bClicking)&&(uButtonClicked != i)) break;
			ReleaseButton(i);
			break;
		}
	}
	bClicking = FALSE;
	uButtonClicked = 0;
	return;
 	UNREFERENCED_PARAMETER(nFlags);
}

VOID MouseMovesTo(UINT nFlags, DWORD x, DWORD y)
{
	UINT i;
	HCURSOR hCursor;

	// set cursor
	_ASSERT(hCursorArrow);
	_ASSERT(hCursorHand);
	hCursor = hCursorArrow;					// normal arrow cursor
	if (!bClassicCursor)
	{
		for (i = 0; i < nButtons; i++)		// scan all buttons
		{
			if (ClipButton(x,y,i))			// cursor over button?
			{
				hCursor = hCursorHand;		// hand cursor
				break;
			}
		}
	}
	// make sure that class cursor is NULL
	_ASSERT(GetClassLong(hWnd,GCL_HCURSOR) == 0);
	SetCursor(hCursor);

	if (!(nFlags&MK_LBUTTON)) return;						// left mouse key not pressed -> quit
	if ((bPressed) && !(ClipButton(x,y,uLastPressedKey)))	// not on last pressed key
		ReleaseAllButtons();								// release all buttons
	if (!bClicking) return;									// normal emulation key -> quit

	if (pButton[uButtonClicked].dwFlags&BUTTON_NOHOLD)
	{
		if (ClipButton(x,y, uButtonClicked) != pButton[uButtonClicked].bDown)
		{
			pButton[uButtonClicked].bDown = !pButton[uButtonClicked].bDown;
			DrawButton(uButtonClicked);
		}
		return;
	}
	if (pButton[uButtonClicked].dwFlags&BUTTON_VIRTUAL)
	{
		if (!ClipButton(x,y, uButtonClicked))
		{
			ReleaseButton(uButtonClicked);
			bClicking = FALSE;
			uButtonClicked = 0;
		}
		return;
	}
	return;
}



//################
//#
//#    Keyboard
//#
//################

VOID RunKey(BYTE nId, BOOL bPressed)
{
	if (pVKey[nId])
	{
		KmlLine* pLine = pVKey[nId]->pFirstLine;
		byVKeyMap[nId] = bPressed;
		while (pLine) pLine = RunLine(pLine);
	}
	else
	{
		if (bDebug&&bPressed)
		{
			TCHAR szTemp[128];
			wsprintf(szTemp,_T("Scancode %i"),nId);
			InfoMessage(szTemp);
		}
	}
	return;
}



//################
//#
//#    Load and Initialize Script
//#
//################

static KmlBlock* LoadKMLGlobal(LPCTSTR szFilename)
{
	HANDLE    hFile;
	LPTSTR    lpBuf;
	KmlBlock* pBlock;
	DWORD     eToken;

	SetCurrentDirectory(szEmuDirectory);
	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	SetCurrentDirectory(szCurrentDirectory);
	if (hFile == INVALID_HANDLE_VALUE) return NULL;
	if ((lpBuf = MapKMLFile(hFile)) == NULL)
		return NULL;

	InitLex(lpBuf);
	pBlock = NULL;
	eToken = Lex(LEX_BLOCK);
	if (eToken == TOK_GLOBAL)
	{
		pBlock = ParseBlock(eToken);
		if (pBlock) pBlock->pNext = NULL;
	}
	CleanLex();
	ClearLog();
	HeapFree(hHeap,0,lpBuf);
	return pBlock;
}


BOOL InitKML(LPCTSTR szFilename, BOOL bNoLog)
{
	HANDLE    hFile;
	LPTSTR    lpBuf;
	KmlBlock* pBlock;
	BOOL      bOk = FALSE;

	KillKML();

	nBlocksIncludeLevel = 0;
	PrintfToLog(_T("Reading %s"), szFilename);
	SetCurrentDirectory(szEmuDirectory);
	hFile = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	SetCurrentDirectory(szCurrentDirectory);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AddToLog(_T("Error while opening the file."));
		goto quit;
	}
	if ((lpBuf = MapKMLFile(hFile)) == NULL)
		goto quit;

	InitLex(lpBuf);
	pKml = ParseBlocks();
	CleanLex();

	HeapFree(hHeap,0,lpBuf);
	if (pKml == NULL) goto quit;

	pBlock = pKml;
	while (pBlock)
	{
		switch (pBlock->eType)
		{
		case TOK_BUTTON:
			InitButton(pBlock);
			break;
		case TOK_SCANCODE:
			nScancodes++;
			pVKey[pBlock->nId] = pBlock;
			break;
		case TOK_ANNUNCIATOR:
			InitAnnunciator(pBlock);
			break;
		case TOK_GLOBAL:
			InitGlobal(pBlock);
			break;
		case TOK_LCD:
			InitLcd(pBlock);
			break;
		case TOK_BACKGROUND:
			InitBackground(pBlock);
			break;
		default:
			PrintfToLog(_T("Block %s Ignored."), GetStringOf(pBlock->eType));
			pBlock = pBlock->pNext;
		}
		pBlock = pBlock->pNext;
	}
		
	if (cCurrentRomType == 0)
	{
		AddToLog(_T("This KML Script doesn't specify the ROM Type."));
		goto quit;
	}
	if (pbyRom == NULL)
	{
		AddToLog(_T("This KML Script doesn't specify the ROM to use, or the ROM could not be loaded."));
		goto quit;
	}
	CreateLcdBitmap();

	PrintfToLog(_T("%i Buttons Defined"), nButtons);
	PrintfToLog(_T("%i Scancodes Defined"), nScancodes);
	PrintfToLog(_T("%i Annunciators Defined"), nAnnunciators);

	bOk = TRUE;

quit:
	if (bOk)
	{
		// HP38G/HP39G/HP40G have no object loading, ignore
		DragAcceptFiles(hWnd,cCurrentRomType != '6' && cCurrentRomType != 'A' && cCurrentRomType != 'E');

		if (!bNoLog)
		{
			AddToLog(_T("Press Ok to Continue."));
			if (bAlwaysDisplayLog&&(!DisplayKMLLog(bOk)))
			{
				KillKML();
				return FALSE;
			}
		}
	}
	else
	{
		AddToLog(_T("Press Cancel to Abort."));
		if (!DisplayKMLLog(bOk))
		{
			KillKML();
			return FALSE;
		}
	}

	ResizeWindow();
	ClearLog();
	return bOk;
}
