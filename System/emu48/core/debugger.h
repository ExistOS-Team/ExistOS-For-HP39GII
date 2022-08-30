/*
 *   debugger.h
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1999 Christoph Gieﬂelink
 *
 */

// breakpoint type definitions
#define BP_EXEC		0x01					// code breakpoint
#define BP_READ		0x02					// read memory breakpoint
#define BP_WRITE	0x04					// write memory breakpoint
#define BP_RPL      0x08					// RPL breakpoint
#define BP_ACCESS	(BP_READ|BP_WRITE)		// read/write memory breakpoint

// breakpoint notify definitions
#define BN_ASM		0						// ASM breakpoint
#define BN_RPL		1						// RPL breakpoint
#define BN_ASM_BT	2						// ASM and RPL breakpoint

// debugger state definitions
#define DBG_SUSPEND		-1
#define DBG_OFF			0
#define DBG_RUN			1
#define DBG_STEPINTO	2
#define DBG_STEPOVER    3
#define DBG_STEPOUT     4

// debugger.c
extern VOID    UpdateDbgCycleCounter(VOID);
extern BOOL    CheckBreakpoint(DWORD dwAddr, DWORD wRange, UINT nType);
extern VOID    NotifyDebugger(INT nType);
extern VOID    DisableDebugger(VOID);
extern LRESULT OnToolDebug(VOID);
extern VOID    LoadBreakpointList(HANDLE hFile);
extern VOID    SaveBreakpointList(HANDLE hFile);
