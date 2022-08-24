/*
 *   types.h
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */

// HST bits
#define XM 1
#define SB 2
#define SR 4
#define MP 8

#define	SWORD SHORT							// signed   16 Bit variable
#define	QWORD ULONGLONG						// unsigned 64 Bit variable

#define CHIPSET Chipset_t
typedef struct
{
	SWORD	nPosX;							// position of window
	SWORD	nPosY;
	BYTE	type;							// calculator type

	DWORD	Port0Size;						// real size of module in KB
	DWORD	Port1Size;						// real size of module in KB
	DWORD	Port2Size;						// real size of module in KB (HP49G only)
	LPBYTE  Port0;
	LPBYTE  Port1;
	LPBYTE  Port2;

	DWORD   pc;
	DWORD	d0;
	DWORD	d1;
	DWORD	rstkp;
	DWORD	rstk[8];
	BYTE    A[16];
	BYTE    B[16];
	BYTE    C[16];
	BYTE    D[16];
	BYTE    R0[16];
	BYTE    R1[16];
	BYTE    R2[16];
	BYTE    R3[16];
	BYTE    R4[16];
	BYTE    ST[4];
	BYTE    HST;
	BYTE    P;
	WORD    out;
	WORD	in;
	BOOL	SoftInt;
	BOOL	Shutdn;
	BOOL    mode_dec;
	BOOL	inte;							// interrupt status flag (FALSE = int in service)
	BOOL	intk;							// 1 ms keyboard scan flag (TRUE = enable)
	BOOL	intd;							// keyboard interrupt pending (TRUE = int pending)
	BOOL	carry;

	WORD    crc;
	WORD	wPort2Crc;						// fingerprint of port2
	WORD	wRomCrc;						// fingerprint of ROM
#if defined _USRDLL							// DLL version
	QWORD	cycles;							// oscillator cycles
#else										// EXE version
	DWORD	cycles;							// oscillator cycles
	DWORD	cycles_reserved;				// reserved for MSB of oscillator cycles
#endif
	DWORD	dwKdnCycles;					// cpu cycles at start of 1ms key handler

	UINT    Bank_FF;						// save state of HP48GX port2 or state of HP49G ROM FF
	UINT    FlashRomState;					// WSM state of flash memory (unused)
	BYTE    cards_status;
	BYTE    IORam[64];						// I/O hardware register
	UINT    IOBase;							// address of I/O modules page 
	BOOL    IOCfig;							// I/O module configuration flag
	BYTE    P0Base, BSBase, P1Base, P2Base;	// address of modules first 2KB page 
	BYTE    P0Size, BSSize, P1Size, P2Size;	// mapped size of module in 2KB
	BYTE    P0End,  BSEnd,  P1End,  P2End;	// address of modules last 2KB page
	BOOL    P0Cfig, BSCfig, P1Cfig, P2Cfig;	// module address configuration flag
	BOOL    P0Cfg2, BSCfg2, P1Cfg2, P2Cfg2;	// module size configuration flag

	BYTE    t1;
	DWORD   t2;

	BOOL    bShutdnWake;					// flag for wake up from SHUTDN mode

	BYTE    Keyboard_Row[9];
	WORD    IR15X;
	UINT    Keyboard_State;					// not used

	signed short loffset;
	signed int   width;
	UINT    boffset;
	UINT    lcounter;
	UINT    sync;							// not used
	BYTE    contrast;
	BOOL    dispon;							// not used
	DWORD   start1;
	DWORD   start12;
	DWORD   end1;
	DWORD   start2, end2;

	// CdB for HP: add apples header
	DWORD   d0size;							// no. of header display lines
	BYTE    d0memory[4096*2];				// memory for header display area
	DWORD   d0offset;						// offset inside the header display for the content
	DWORD   d0address;						// address in saturn addr area for d0memory (2 pages)
//	BOOL    d0Cfig;							// modul configured
} Chipset_t;
