#ifndef MAC_TYPE_PATCH_H
#define MAC_TYPE_PATCH_H

#include <stdint.h>
#include <pthread.h>

#define __LITTLE_ENDIAN__ (1)

#ifndef __OBJC__
typedef signed char BOOL;   // deliberately same type as defined in objc
#endif
#define TRUE    1
#define FALSE   0
#define MAX_PATH    MAXPATHLEN
#define INFINITE    0

typedef	unsigned long ulong;	// ushort is already in types.h
typedef short SHORT;
typedef uint64_t ULONGLONG;
typedef int64_t LONGLONG;
typedef int64_t __int64;

typedef unsigned char BYTE;
typedef BYTE byte;
typedef uint32_t DWORD;
typedef BYTE* LPBYTE;
typedef unsigned short WORD;
typedef uint32_t UINT;
typedef int32_t INT;
typedef char CHAR;
typedef void VOID;
typedef void *LPVOID;
typedef long LONG;
typedef size_t SIZE_T;

#define CONST const

typedef int HANDLE;
typedef HANDLE HPALETTE;
typedef HANDLE HBITMAP;
typedef HANDLE HMENU;
typedef HANDLE HFONT;

typedef void *TIMECALLBACK ;
#define CALLBACK
typedef void(*LPTIMECALLBACK)(UINT uEventId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2) ;
//typedef TIMECALLBACK *LPTIMECALLBACK;
typedef long LRESULT;
typedef struct _RECT {
    short left;
    short right;
    short top;
    short bottom;
} RECT;
typedef BYTE *LPCVOID;
typedef struct _RGBQUAD {
    BYTE rgbRed;
    BYTE rgbGreen;
    BYTE rgbBlue;
    BYTE rgbReserved;
} RGBQUAD;

#define LowPart d.LwPart
typedef union _LARGE_INTEGER {
    uint64_t QuadPart;
    struct {
//#ifdef __LITTLE_ENDIAN__
        uint32_t LwPart;
        uint32_t HiPart;
//#else
//        uint32_t HiPart;
//        uint32_t LwPart;
//#endif
    }d;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef char *LPTSTR ;
typedef char *LPSTR ;
typedef char LPCSTR[255] ;
typedef const char *LPCTSTR ;
typedef char TCHAR;

typedef pthread_mutex_t CRITICAL_SECTION;
typedef HANDLE HINSTANCE;
typedef HANDLE HWND;
typedef void WNDCLASS;
typedef HANDLE HDC;
typedef HANDLE HCURSOR;

struct FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
};

struct SYSTEMTIME
{
	DWORD wYear;
	DWORD wMonth;
	DWORD wDay;
	DWORD wHour;
	DWORD wMinute;
	DWORD wSecond;
	DWORD wMilliseconds;
};
#define CALLBACK
typedef int HDDEDATA; //ignored in this port
typedef int HCONV;//ignored in this port
typedef int HSZ;//ignored in this port

struct TIMECAPS
{
	UINT wPeriodMin;
	UINT wPeriodMax;
};

struct DCB
{
	DWORD DCBlength;
	DWORD BaudRate;
	BOOL fBinary,
		 fParity,
		 fOutxCtsFlow,
		 fOutxDsrFlow,
		 fOutX,
		 fErrorChar,
		 fNull,
		 fAbortOnError;
		 
	WORD fDtrControl,
		 fDsrSensitivity,
		 fRtsControl,
		 ByteSize,
		 Parity,
		 StopBits;
};
enum
{
	DTR_CONTROL_DISABLE,
	RTS_CONTROL_DISABLE,
	NOPARITY,
	ONESTOPBIT
};
struct COMMTIMEOUTS
{
	DWORD a,b,c,d,e;
};
/*
struct OVERLAPPED
{
	HANDLE hEvent;
};
*/

typedef struct _SYSTEM_POWER_STATUS
{
    BYTE  ACLineStatus;
    BYTE  BatteryFlag;
    BYTE  BatteryLifePercent;
    BYTE  Reserved1;
    DWORD BatteryLifeTime;
    DWORD BatteryFullLifeTime;
} SYSTEM_POWER_STATUS, *LPSYSTEM_POWER_STATUS;

#define __cdecl
#define WINAPI
#endif
