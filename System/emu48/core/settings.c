/*
 *   settings.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 2000 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "emu48.h"
#include "i28f160.h"

// #define REGISTRY							// use registry instead of *.ini file

//################
//#
//#    Low level subroutines
//#
//################

#if !defined REGISTRY

// INI-file handling

#if !defined EMU48_INI
	#define EMU48_INI "Emu48.ini"
#endif

#define ReadString(sec,key,dv,v,sv) GetPrivateProfileString(sec,key,dv,v,sv,_T(EMU48_INI))
#define ReadInt(sec,key,dv)         GetPrivateProfileInt(sec,key,dv,_T(EMU48_INI));
#define WriteString(sec,key,v)      WritePrivateProfileString(sec,key,v,_T(EMU48_INI))
#define WriteInt(sec,key,v)         WritePrivateProfileInt(sec,key,v,_T(EMU48_INI))
#define DelKey(sec,key)             WritePrivateProfileString(sec,key,NULL,_T(EMU48_INI))

static BOOL WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR lpszFilename)
{
	TCHAR s[16];
	wsprintf(s,_T("%i"),nValue);
	return WritePrivateProfileString(lpszSection, lpszEntry, s, lpszFilename);
}

#else

// registry handling

#if !defined REGISTRYKEY
	#define REGISTRYKEY "Software\\Emu48"
#endif

#define ReadString(sec,key,dv,v,sv) GetRegistryString(sec,key,dv,v,sv)
#define ReadInt(sec,key,dv)         GetRegistryInt(sec,key,dv)
#define WriteString(sec,key,v)      WriteReg(sec,key,REG_SZ,(BYTE *) v,(lstrlen(v)+1) * sizeof(*v))
#define WriteInt(sec,key,v)         WriteReg(sec,key,REG_DWORD,(BYTE *) &v,sizeof(int))
#define DelKey(sec,key)             DelReg(sec,key)

static VOID ReadReg(LPCTSTR lpSubKey, LPCTSTR lpValueName, LPBYTE lpData, DWORD *pdwSize)
{
	TCHAR lpKey[256] = _T(REGISTRYKEY) _T("\\");

	DWORD retCode,dwType;
	HKEY  hKey;

	lstrcat(lpKey, lpSubKey);				// full registry key

	retCode = RegOpenKeyEx(HKEY_CURRENT_USER,
					       lpKey,
						   0,
						   KEY_QUERY_VALUE,
						   &hKey);
	if (retCode == ERROR_SUCCESS) 
	{
		retCode = RegQueryValueEx(hKey,lpValueName,NULL,&dwType,lpData,pdwSize);
		RegCloseKey(hKey);
	}

	if (retCode != ERROR_SUCCESS)			// registry entry not found
		*pdwSize = 0;						// return zero size
	return;
} 

static VOID WriteReg(LPCTSTR lpSubKey, LPCTSTR lpValueName, DWORD dwType, CONST BYTE *lpData, DWORD cbData)
{
	TCHAR lpKey[256] = _T(REGISTRYKEY) _T("\\");

	DWORD retCode;
	HKEY  hKey;
	DWORD dwDisposition;

	lstrcat(lpKey, lpSubKey);				// full registry key

	retCode = RegCreateKeyEx(HKEY_CURRENT_USER,
							 lpKey,
							 0,_T(""),
                             REG_OPTION_NON_VOLATILE,
							 KEY_WRITE,
                             NULL,
							 &hKey,
							 &dwDisposition);
	_ASSERT(retCode == ERROR_SUCCESS);

	RegSetValueEx(hKey,lpValueName,0,dwType,lpData,cbData);
	RegCloseKey(hKey);
	return;
}

static VOID DelReg(LPCTSTR lpSubKey, LPCTSTR lpValueName)
{
	TCHAR lpKey[256] = _T(REGISTRYKEY) _T("\\");

	DWORD retCode;
	HKEY  hKey;

	lstrcat(lpKey, lpSubKey);				// full registry key

	retCode = RegOpenKeyEx(HKEY_CURRENT_USER,
					       lpKey,
						   0,
						   KEY_SET_VALUE,
						   &hKey);
	if (retCode == ERROR_SUCCESS) 
	{
		retCode = RegDeleteValue(hKey,lpValueName);
		RegCloseKey(hKey);
	}
	return;
}

static DWORD GetRegistryString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpDefault, LPTSTR lpData, DWORD dwSize)
{
	dwSize *= sizeof(*lpData);				// buffer size in bytes
	ReadReg(lpszSection,lpszEntry,(LPBYTE) lpData,&dwSize);
	if (dwSize == 0)
	{
		lstrcpy(lpData,lpDefault);
		dwSize = lstrlen(lpData);
	}
	else
	{
		dwSize = (dwSize / sizeof(*lpData)) - 1;
	}
	return dwSize;
}

static INT GetRegistryInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault)
{
	UINT  nValue;
	DWORD dwSize = sizeof(nValue);

	ReadReg(lpszSection,lpszEntry,(LPBYTE) &nValue,&dwSize);
	return dwSize ? nValue : nDefault;
}

#endif


//################
//#
//#    Public functions
//#
//################

VOID ReadSettings(VOID)
{
	// Files
	ReadString(_T("Files"),_T("Emu48Directory"),szCurrentDirectory,szEmuDirectory,ARRAYSIZEOF(szEmuDirectory));
	bAutoSave          = ReadInt(_T("Files"),_T("AutoSave"),bAutoSave);
	bAutoSaveOnExit    = ReadInt(_T("Files"),_T("AutoSaveOnExit"),bAutoSaveOnExit);
	bSaveDefConfirm    = ReadInt(_T("Files"),_T("SaveDefaultConfirm"),bSaveDefConfirm);
	bStartupBackup     = ReadInt(_T("Files"),_T("StartupBackup"),bStartupBackup);
	bLoadObjectWarning = ReadInt(_T("Files"),_T("LoadObjectWarning"),bLoadObjectWarning);
	// Port2
	bPort2IsShared  = ReadInt(_T("Port2"),_T("IsShared"),bPort2IsShared);
	ReadString(_T("Port2"),_T("Filename"),_T("SHARED.BIN"),szPort2Filename,ARRAYSIZEOF(szPort2Filename));
	// KML
	bAlwaysDisplayLog = ReadInt(_T("KML"),_T("AlwaysDisplayLog"),bAlwaysDisplayLog);
	// Disassembler
	disassembler_mode = ReadInt(_T("Disassembler"),_T("Mnemonics"),disassembler_mode);
	// Emulator
	bAlwaysOnTop     = ReadInt(_T("Emulator"),_T("AlwaysOnTop"),bAlwaysOnTop);
	bActFollowsMouse = ReadInt(_T("Emulator"),_T("ActivationFollowsMouse"),bActFollowsMouse);
	bRealSpeed       = ReadInt(_T("Emulator"),_T("RealSpeed"),bRealSpeed);
	dwSXCycles       = ReadInt(_T("Emulator"),_T("SXCycles"),dwSXCycles);
	dwGXCycles       = ReadInt(_T("Emulator"),_T("GXCycles"),dwGXCycles);
	bGrayscale       = ReadInt(_T("Emulator"),_T("Grayscale"),bGrayscale);
	bWaveBeep        = ReadInt(_T("Emulator"),_T("WaveBeep"),bWaveBeep);
	dwWaveVol        = ReadInt(_T("Emulator"),_T("WaveVolume"),dwWaveVol);
	SetSpeed(bRealSpeed);					// set speed
	// LowBat
	bLowBatDisable = ReadInt(_T("LowBat"),_T("Disable"),bLowBatDisable);
	// Macro
	bMacroRealSpeed = ReadInt(_T("Macro"),_T("RealSpeed"),bMacroRealSpeed);
	nMacroTimeout   = ReadInt(_T("Macro"),_T("ReplayTimeout"),nMacroTimeout);
	// Serial
	ReadString(_T("Serial"),_T("Wire"),_T(NO_SERIAL),szSerialWire,ARRAYSIZEOF(szSerialWire));
	ReadString(_T("Serial"),_T("Ir"),_T(NO_SERIAL),szSerialIr,ARRAYSIZEOF(szSerialIr));
	// ROM
	bRomWriteable = ReadInt(_T("ROM"),_T("Writeable"),bRomWriteable);
	bWP = ReadInt(_T("ROM"),_T("WP#"),bWP);
	return;
}

VOID WriteSettings(VOID)
{
	// Files
	WriteString(_T("Files"),_T("Emu48Directory"),szEmuDirectory);
	WriteInt(_T("Files"),_T("AutoSave"),bAutoSave);
	WriteInt(_T("Files"),_T("AutoSaveOnExit"),bAutoSaveOnExit);
	WriteInt(_T("Files"),_T("SaveDefaultConfirm"),bSaveDefConfirm);
	WriteInt(_T("Files"),_T("StartupBackup"),bStartupBackup);
	WriteInt(_T("Files"),_T("LoadObjectWarning"),bLoadObjectWarning);
	// Port2
	WriteInt(_T("Port2"),_T("IsShared"),bPort2IsShared);
	WriteString(_T("Port2"),_T("Filename"),szPort2Filename);
	// KML
	WriteInt(_T("KML"),_T("AlwaysDisplayLog"),bAlwaysDisplayLog);
	// Disassembler
	WriteInt(_T("Disassembler"),_T("Mnemonics"),disassembler_mode);
	// Emulator
	WriteInt(_T("Emulator"),_T("AlwaysOnTop"),bAlwaysOnTop);
	WriteInt(_T("Emulator"),_T("ActivationFollowsMouse"),bActFollowsMouse);
	WriteInt(_T("Emulator"),_T("RealSpeed"),bRealSpeed);
	WriteInt(_T("Emulator"),_T("SXCycles"),dwSXCycles);
	WriteInt(_T("Emulator"),_T("GXCycles"),dwGXCycles);
	WriteInt(_T("Emulator"),_T("Grayscale"),bGrayscale);
	WriteInt(_T("Emulator"),_T("WaveBeep"),bWaveBeep);
	WriteInt(_T("Emulator"),_T("WaveVolume"),dwWaveVol);
	// LowBat
	WriteInt(_T("LowBat"),_T("Disable"),bLowBatDisable);
	// Macro
	WriteInt(_T("Macro"),_T("RealSpeed"),bMacroRealSpeed);
	WriteInt(_T("Macro"),_T("ReplayTimeout"),nMacroTimeout);
	// Serial
	WriteString(_T("Serial"),_T("Wire"),szSerialWire);
	WriteString(_T("Serial"),_T("Ir"),szSerialIr);
	// ROM
	WriteInt(_T("ROM"),_T("Writeable"),bRomWriteable);
	return;
}

VOID ReadLastDocument(LPTSTR szFilename, DWORD nSize)
{
	ReadString(_T("Files"),_T("LastDocument"),_T(""),szFilename,nSize);
	return;
}

VOID WriteLastDocument(LPCTSTR szFilename)
{
	WriteString(_T("Files"),_T("LastDocument"),szFilename);
	return;
}

VOID ReadSettingsString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpDefault, LPTSTR lpData, DWORD dwSize)
{
	ReadString(lpszSection,lpszEntry,lpDefault,lpData,dwSize);
	return;
}

VOID WriteSettingsString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPTSTR lpData)
{
	WriteString(lpszSection,lpszEntry,lpData);
	return;
}

INT ReadSettingsInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault)
{
	return ReadInt(lpszSection,lpszEntry,nDefault);
}

VOID WriteSettingsInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nValue)
{
	WriteInt(lpszSection,lpszEntry,nValue);
	return;
}

VOID DelSettingsKey(LPCTSTR lpszSection, LPCTSTR lpszEntry)
{
	DelKey(lpszSection,lpszEntry);
	return;
}
