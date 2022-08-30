/*
 *   i28f160.h
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 2000 Christoph Gieﬂelink
 *
 */

#define WSMVER	0							// version of flash implementation structure

#define WSMSET WSMset_t
typedef struct
{
	BYTE	byType[4];						// "WSM"
	UINT	uSize;							// size of this structure
	BYTE    byVersion;						// WSM version

	BOOL    bRomArray;						// copy of bFlashRomArray
	DWORD   dwLockCnfg;						// block lock table (32 entries)
	UINT	uWrState;						// state of write function WSM
	UINT	uRdState;						// state of read function WSM
	BYTE    byStatusReg;					// status register
	BYTE    byExStatusReg;					// extended status register
	BYTE	byWrite1No;						// no. of written data in write buffer1
	BYTE	byWrite1Size;					// no. of valid data in write buffer1
	DWORD	dwWrite1Addr;					// destination address of buffer1
	BYTE	pbyWrite1[32];					// write buffer1
//	BYTE	byWrite2No;						// no. of written data in write buffer2
//	BYTE	byWrite2Size;					// no. of valid data in write buffer2
//	DWORD	dwWrite2Addr;					// destination address of buffer2
//	BYTE	pbyWrite2[32];					// write buffer2
} WSMset_t;

// i28f160.h
extern WSMSET WSMset;
extern BOOL   bWP;
extern VOID   FlashInit(VOID);
extern VOID   FlashRead(BYTE *a, DWORD d, UINT s);
extern VOID   FlashWrite(BYTE *a, DWORD d, UINT s);
