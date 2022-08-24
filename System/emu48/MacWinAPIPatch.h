#ifndef _MAC_WINAPIPATCH_H_
#define _MAC_WINAPIPATCH_H_
/*
 *  MacWinAPIPatch.h
 *  Emu48
 *
 *  Created by Da Woon Jung on Wed Dec 10 2003.
 *  Copyright (c) 2003 dwj. All rights reserved.
 *
 */

#include "MacTypePatch.h"
#include <stdio.h>
#include <string.h>

#define __max(a,b) (((a) > (b)) ? (a) : (b))
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))


enum
{
    GENERIC_READ   = 1,
    GENERIC_WRITE  = 2,
    FILE_SHARE_READ  = 1,
    FILE_SHARE_WRITE = 2,
    OPEN_EXISTING = 121,
    FILE_ATTRIBUTE_NORMAL = 1,
    FILE_FLAG_OVERLAPPED  = 2,
    FILE_FLAG_SEQUENTIAL_SCAN = 4,
    PAGE_READONLY,
    FILE_MAP_READ,
    CREATE_ALWAYS,
    PAGE_READWRITE,
    PAGE_WRITECOPY,
    FILE_MAP_WRITE,
    FILE_MAP_COPY,

    //errors
    INVALID_HANDLE_VALUE = -1,
    ERROR_ALREADY_EXISTS = 1
};


enum MsgBoxFlagType {
    IDOK     = 0,
    IDYES    = 1,
    IDNO     = 2,
    IDCANCEL = 3,
    MB_OK              = 1,
    MB_YESNO           = 1 << 1,
    MB_YESNOCANCEL     = 1 << 2,
    MB_ICONWARNING     = 1 << 3,
    MB_ICONERROR       = 1 << 4,
    MB_ICONSTOP        = 1 << 5,
    MB_ICONINFORMATION = 1 << 6,
    MB_ICONQUESTION    = 1 << 7,
    MB_ICONEXCLAMATION = 1 << 8,
    MB_DEFBUTTON2      = 1 << 9
};

#define MB_APPLMODAL        0
#define MB_SETFOREGROUND    0

// These are passed to tab control callbacks
enum {
  WM_INITDIALOG,	// fills in tab controls when first shown
  WM_COMMAND,	// respond to validation/dimming commands
  WM_GETDLGVALUES	// read off the control settings
};

// Constants for the msg parameter in SendDlgItemMessage()
enum {
  CB_ERR = -1,
  CB_GETCURSEL = 1,
  CB_GETLBTEXT,
  CB_RESETCONTENT,
  CB_SETCURSEL,
  CB_ADDSTRING,
  WMU_SETWINDOWVALUE
};

// Constants for SetFilePointer
enum FilePointerType {
  INVALID_SET_FILE_POINTER = -1,
  FILE_BEGIN = 1,
  FILE_CURRENT,
  FILE_END
};

// Supposedly specifies that Windows use default window coords
enum { CW_USEDEFAULT };

enum {
    AC_LINE_OFFLINE,
    AC_LINE_ONLINE,
    AC_LINE_UNKNOWN = 255
};

enum {
    BATTERY_FLAG_HIGH = 1,
    BATTERY_FLAG_LOW = 2,
    BATTERY_FLAG_CRITICAL = 4,
    BATTERY_FLAG_CHARGING = 8,
    BATTERY_FLAG_NO_BATTERY = 128,
    BATTERY_FLAG_UNKNOWN = 255
};


// Macros
#ifdef _T
#undef _T
//#define _T(x)	@x
#endif
#define _T(x)   x

#define TEXT(x)	@x
#define LOWORD(x)	x

#define HIBYTE(i)   ((i)>>8)
#define LOBYTE(i)   (i)
#define _istdigit(c)    ((c)>='0' && (c)<='9')
#define lstrlen(s)      strlen(s)
#define lstrcpy(d,s)    strcpy(d,s)
#define _tcsspn(s,k)    strspn(s,k)
#define wsprintf(s,f,...)   sprintf(s,f, ## __VA_ARGS__)

#define HeapAlloc(h,v,s)    (malloc(s))
//#define HeapAlloc(h,v,s)    malloc((size_t)s)
#define HeapFree(h,v,p)     do{vPortFree(p);(p)=v;}while(0)
//#define HeapFree(h,v,p)     do{free(p);(p)=v;}while(0)

#define ZeroMemory(p,s)     memset(p,0,s)
#define FillMemory(p,n,v)   memset(p,v,n*sizeof(*(p)))
#define CopyMemory(d,src,s) memcpy(d,src,s)
//inline unsigned char HIBYTE(int i) { return i>>8; }
//inline unsigned char LOBYTE(int i) { return i; }
//inline BOOL _istdigit(unichar c) { return (c>='0' && c<='9'); }
/*
extern RGBColor RGB(int r, int g, int b);
inline ControlRef GetDlgItem(WindowRef window, SInt32 control_code)
{
  ControlRef control;
  ControlID	control_id = { app_signature, control_code };
  FailOSErr(GetControlByID(window, &control_id, &control));
  return control;
}
// For radio groups, 1=first choice, 2=second, etc
extern void CheckDlgButton(WindowRef, SInt32 control_code, SInt32 value);
extern void SetDlgItemInt(WindowRef, SInt32 control_code, int value, bool);
extern void SetDlgItemText(WindowRef, SInt32 control_code, CFStringRef);
// Use this to fiddle with dropdown menus (pass a CB_ constant for msg)
// Indexes are zero-based. Also handles WMU_SETWINDOWVALUE
extern int SendDlgItemMessage(WindowRef, SInt32 control_code, int msg, int p1, void *p2);
// For checkboxes, result will be zero or one (can cast directly to bool)
extern SInt32 IsDlgButtonChecked(WindowRef, SInt32 control_code);
// Despite its name, this dims/undims controls
extern void EnableWindow(ControlRef, bool enable);
// Used to fill in font dropdown menus
typedef FMFontFamilyCallbackFilterProcPtr FONTENUMPROC;
extern void EnumFontFamilies(void *, void *, FONTENUMPROC, void *userdata);
*/
extern BOOL SetCurrentDirectory(LPCTSTR);	// returns NO if fails
extern int CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, LPVOID hTemplateFile);
/*
extern int ReadFile(int fd, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPVOID lpOverlapped);
// WriteFile() writes to a temp file, then swaps that with the real file
extern OSErr WriteFile(FSPtr &, void *buf, SInt64 size, void *, void *);
extern UInt64 GetFileSize(FSPtr &, void *);
// Do a strcmp-like comparison between 64-bit times
extern int CompareFileTime(FILETIME *, FILETIME *);
extern long SetFilePointer(FSPtr &, long offset, void *, FilePointerType startpoint);

extern void SetTimer(void *, TimerType, int msec, void *);
*/

  typedef UINT MMVERSION;
  typedef UINT MMRESULT;

extern MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD_PTR dwUser, UINT fuEvent);
extern MMRESULT timeKillEvent(UINT uTimerID);
extern int MessageBox(HANDLE, LPCTSTR szMessage, LPCTSTR szTitle, int flags);
extern BOOL QueryPerformanceFrequency(PLARGE_INTEGER l);
extern BOOL QueryPerformanceCounter(PLARGE_INTEGER l);
extern DWORD timeGetTime();
extern void EnterCriticalSection(CRITICAL_SECTION *);
extern void LeaveCriticalSection(CRITICAL_SECTION *);
extern HANDLE CreateEvent(WORD, BOOL, BOOL, WORD);
extern void SetEvent(HANDLE);
extern void DestroyEvent(HANDLE);
extern BOOL ResetEvent(HANDLE);
#define CloseEvent(h);

#define WAIT_OBJECT_0   0
#define WAIT_TIMEOUT    0x00000102
#define WAIT_FAILED     0xFFFFFFFF
extern DWORD WaitForSingleObject(HANDLE, int);
extern void Sleep(int);
#define UNREFERENCED_PARAMETER(a)

#define TIME_ONESHOT    0x0000   /* program timer for single event */
#define TIME_PERIODIC   0x0001   /* program for continuous periodic event */

typedef unsigned long ULONG;
typedef short SHORT;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef uint64_t ULONGLONG;
typedef int64_t LONGLONG;
typedef int64_t __int64;
typedef float FLOAT;

typedef unsigned char BYTE;
typedef BYTE byte;
typedef unsigned short WORD;
typedef WORD *LPWORD;
typedef uint32_t DWORD;
typedef DWORD *LPDWORD;
typedef BYTE *LPBYTE;
typedef uint16_t WORD;
typedef uint32_t UINT;
typedef UINT * LPUINT;
typedef int32_t INT;
typedef char CHAR;
typedef void VOID;
typedef void *LPVOID;
typedef void *PVOID;
typedef int32_t LONG;
typedef LONG *PLONG;

#if INTPTR_MAX == INT32_MAX
    // 32 bits environment
    typedef int INT_PTR, *PINT_PTR;
    typedef unsigned int UINT_PTR, *PUINT_PTR;
    typedef int LONG_PTR, *PLONG_PTR;
    typedef unsigned long ULONG_PTR, *PULONG_PTR;
#elif INTPTR_MAX == INT64_MAX
    // 64 bits environment
    typedef int64_t INT_PTR, *PINT_PTR;
    typedef uint64_t UINT_PTR, *PUINT_PTR;
    typedef int64_t LONG_PTR, *PLONG_PTR;
    typedef uint64_t ULONG_PTR, *PULONG_PTR;
#endif

typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;

typedef struct _OVERLAPPED {
    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

#define TIMERR_NOERROR        (0)                  /* no error */

extern VOID OutputDebugString(LPCSTR lpOutputString);

extern DWORD GetCurrentDirectory(DWORD nBufferLength, LPTSTR lpBuffer);
extern HANDLE CreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPVOID lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, LPVOID hTemplateFile);
extern BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
extern DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
extern DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
extern BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
extern HANDLE CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
extern LPVOID MapViewOfFile(HANDLE hFileMappingObject,DWORD dwDesiredAccess, DWORD dwFileOffsetHigh,DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
extern BOOL UnmapViewOfFile(LPCVOID lpBaseAddress);
extern BOOL SetEndOfFile(HANDLE hFile);

// This is not a Win32 function
extern BOOL SaveMapViewToFile(LPCVOID lpBaseAddress);


typedef UINT_PTR (CALLBACK *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef struct tagOFNA {
    DWORD        lStructSize;
    HWND         hwndOwner;
    HINSTANCE    hInstance;
    LPCSTR       lpstrFilter;
    LPSTR        lpstrCustomFilter;
    DWORD        nMaxCustFilter;
    DWORD        nFilterIndex;
    LPSTR        lpstrFile;
    DWORD        nMaxFile;
    LPSTR        lpstrFileTitle;
    DWORD        nMaxFileTitle;
    LPCSTR       lpstrInitialDir;
    LPCSTR       lpstrTitle;
    DWORD        Flags;
    WORD         nFileOffset;
    WORD         nFileExtension;
    LPCSTR       lpstrDefExt;
    LPARAM       lCustData;
    LPOFNHOOKPROC lpfnHook;
    LPCSTR       lpTemplateName;
#ifdef _MAC
    LPEDITMENU   lpEditInfo;
   LPCSTR       lpstrPrompt;
#endif
#if (_WIN32_WINNT >= 0x0500)
    void *        pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
} OPENFILENAMEA, *LPOPENFILENAMEA;
typedef OPENFILENAMEA OPENFILENAME;
typedef LPOPENFILENAMEA LPOPENFILENAME;
#define OFN_READONLY                 0x00000001
#define OFN_OVERWRITEPROMPT          0x00000002
#define OFN_HIDEREADONLY             0x00000004
#define OFN_PATHMUSTEXIST            0x00000800
#define OFN_FILEMUSTEXIST            0x00001000
#define OFN_CREATEPROMPT             0x00002000
#define OFN_EXPLORER                 0x00080000     // new look commdlg
extern BOOL GetOpenFileName(LPOPENFILENAME);
extern BOOL GetSaveFileName(LPOPENFILENAME);
#define IMAGE_BITMAP        0
#define IMAGE_ICON          1
#define LR_LOADFROMFILE     0x00000010
#define LR_DEFAULTSIZE      0x00000040
#define LR_SHARED           0x00008000
extern HANDLE LoadImage(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);

#define WM_USER                         0x0400
#define WM_SETICON                      0x0080
#define ICON_SMALL          0
#define ICON_BIG            1
// message from browser
#define BFFM_INITIALIZED        1
#define BFFM_SELCHANGED         2
#define BFFM_SETSTATUSTEXTA     (WM_USER + 100)
#define BFFM_SETSELECTIONA      (WM_USER + 102)
#define BFFM_SETSTATUSTEXT  BFFM_SETSTATUSTEXTA
#define BFFM_SETSELECTION   BFFM_SETSELECTIONA
#define CB_GETITEMDATA              0x0150
#define CB_SETITEMDATA              0x0151
#define TBM_GETPOS              (WM_USER)
#define TBM_SETPOS              (WM_USER+5)
#define TBM_SETRANGE            (WM_USER+6)
#define TBM_SETTICFREQ          (WM_USER+20)
#define BST_CHECKED        0x0001
#define LB_ADDSTRING            0x0180
#define LB_GETCOUNT             0x018B
#define LB_GETSEL               0x0187
#define LB_GETSELCOUNT          0x0190
#define LB_GETITEMDATA          0x0199
#define LB_SETITEMDATA          0x019A

extern BOOL GetSystemPowerStatus(LPSYSTEM_POWER_STATUS l);
#endif
