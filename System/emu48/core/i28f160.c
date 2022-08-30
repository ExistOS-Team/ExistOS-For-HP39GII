/*
 *   i28f160.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 2000 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "emu48.h"
#include "i28f160.h"

#define ARRAYSIZEOF(a)	(sizeof(a) / sizeof(a[0]))

// Flash Command Set
#define READ_ARRAY			0xFF
#define READ_ID_CODES		0x90
#define READ_QUERY			0x98
#define READ_STATUS_REG		0x70
#define CLEAR_STATUS_REG	0x50
#define WRITE_BUFFER		0xE8
#define WORD_BYTE_PROG1		0x40
#define WORD_BYTE_PROG2		0x10
#define BLOCK_ERASE			0x20
#define BLOCK_ERASE_SUSPEND	0xB0
#define BLOCK_ERASE_RESUME	0xD0
#define STS_CONFIG			0xB8
#define SET_CLR_BLOCK_LOCK	0x60
#define FULL_CHIP_ERASE		0x30

#define CONFIRM				0xD0

// Status Register Definition
#define WSMS				0x80			// WRITE STATE MACHINE STATUS
#define ESS					0x40			// ERASE SUSPEND STATUS
#define ECLBS				0x20			// ERASE AND CLEAR LOCK-BIT STATUS
#define BWSLBS				0x10			// PROGRAM AND SET LOCK-BIT STATUS
#define VPPS				0x08			// Vpp STATUS
#define BWSS				0x04			// PROGRAM SUSPEND STATUS
#define DPS					0x02			// DEVICE PROTECT STATUS

// Extended Status Register Definition
#define WBS					0x80			// WRITE BUFFER STATUS

// write state defines
#define WRS_DATA			0				// idle state
#define WRS_WR_BUFFER_N		1				// write buffer no. of data
#define WRS_WR_BUFFER_D		2				// write buffer data
#define WRS_WR_BUFFER_C		3				// write buffer confirm
#define WRS_WR_BYTE			4				// write byte/word
#define WRS_BLOCK_ERASE		5				// block erase
#define WRS_CHIP_ERASE		6				// full chip erase
#define WRS_STS_PIN_CONFIG	7				// STS pin Configuration
#define WRS_LOCK_BITS		8				// Set/Clear Block Lock-Bits

// read state defines
#define RDS_DATA			0				// data read
#define RDS_ID				1				// read identifier codes
#define RDS_QUERY			2				// read query
#define RDS_SR				3				// read status register
#define RDS_XSR				4				// read extended status register

// global data
WSMSET WSMset;
BOOL   bWP = FALSE;							// WP# = low, locked blocks cannot be erased

// function prototypes
// write function WSM state
static VOID WrStateIdle(BYTE a, DWORD d);
static VOID WrStateE8(DWORD d);
static VOID WrStateE8N(BYTE a, DWORD d);
static VOID WrStateE8D(BYTE a, DWORD d);
static VOID WrStateE8C(BYTE a, DWORD d);
static VOID WrState40(DWORD d);
static VOID WrState40D(BYTE a, DWORD d);
static VOID WrState20(DWORD d);
static VOID WrState20C(BYTE a, DWORD d);
static VOID WrState30(DWORD d);
static VOID WrState30C(BYTE a, DWORD d);
static VOID WrStateB8(DWORD d);
static VOID WrStateB8D(BYTE a, DWORD d);
static VOID WrState60(DWORD d);
static VOID WrState60D(BYTE a, DWORD d);

static VOID (*CONST fnWrState[])(BYTE a, DWORD d) =
{
	WrStateIdle, 
	WrStateE8N, WrStateE8D, WrStateE8C,
	WrState40D,
	WrState20C,
	WrState30C,
	WrStateB8D,
	WrState60D
};

// read function WSM state
static BYTE RdStateData(DWORD d);
static BYTE RdStateId(DWORD d);
static BYTE RdStateQuery(DWORD d);
static BYTE RdStateSR(DWORD d);
static BYTE RdStateXSR(DWORD d);

static BYTE (*CONST fnRdState[])(DWORD d) =
{
	RdStateData, RdStateId, RdStateQuery, RdStateSR, RdStateXSR
};


// read query table
// device address A16-A1, A0 unused
static CONST BYTE byQueryTab[] =
{
	// access with "Read Identifier Codes" command
	// Identifier codes
	0xB0,	// 00, Manufacturer Code
	0xD0,	// 01, Device Code (16 Mbit)
	0x00,	// 02, Block Lock Configuration
	0x02,	// 03, ??

	0x00,	// 04, Reserved for vendor-specific information
	0x00,	// 05,					"
	0x00,	// 06,					"
	0x00,	// 07,					"
	0x00,	// 08,					"
	0x00,	// 09,					"
	0x00,	// 0A,					"
	0x00,	// 0B,					"
	0x00,	// 0C,					"
	0x00,	// 0D,					"
	0x00,	// 0E,					"
	0x00,	// 0F,					"

	// access with "Read Query" command
	// CFI query identification string
	0x51,	// 10, Query-Unique ASCII string "Q"
	0x52,	// 11, Query-Unique ASCII string "R"
	0x59,	// 12, Query-Unique ASCII string "Y"
	0x01,	// 13, Primary Vendor Command Set and Control Interface ID CODE
	0x00,	// 14,					"
	0x31,	// 15, Address for Primary Algorithm Extended Query Table
	0x00,	// 16,					"
	0x00,	// 17, Alternate Vendor Command Set and Control Interface ID Code
	0x00,	// 18,					"
	0x00,	// 19, Address for Secondary Algorithm Extended Query Table
	0x00,	// 1A,					"

	// System interface information
	0x30,	// 1B, Vcc Logic Supply Minimum Program/Erase Voltage (0x27 intel doc, 0x30 real chip)
	0x55,	// 1C, Vcc Logic Supply Maximum Program/Erase Voltage
	0x30,	// 1D, Vpp [Programming] Supply Minimum Program/Erase Voltage  (0x27 intel doc, 0x30 real chip)
	0x55,	// 1E, Vpp [Programming] Supply Maximum Program/Erase Voltage
	0x03,	// 1F, Typical Time-Out per Single Byte/Word Program
	0x06,	// 20, Typical Time-Out for Max. Buffer Write
	0x0A,	// 21, Typical Time-Out per Individual Block Erase
	0x0F,	// 22, Typical Time-Out for Full Chip Erase
	0x04,	// 23, Maximum Time-Out for Byte/Word Program
	0x04,	// 24, Maximum Time-Out for Buffer Write
	0x04,	// 25, Maximum Time-Out per Individual Block Erase
	0x04,	// 26, Maximum Time-Out for Full Chip Erase
	0x15,	// 27, Device Size
	0x02,	// 28, Flash Device Interface Description
	0x00,	// 29,					"
	0x05,	// 2A, Maximum Number of Bytes in Write Buffer
	0x00,	// 2B,					"
	0x01,	// 2C, Number of Erase Block Regions within Device
	0x1F,	// 2D, Erase Block Region Information
	0x00,	// 2E,					"
	0x00,	// 2F,					"
	0x01,	// 30,					"

	// Intel-specific extended query table
	0x50,	// 31, Primary Extended Query Table, Unique ASCII string "P"
	0x52,	// 32, Primary Extended Query Table, Unique ASCII string "R"
	0x49,	// 33, Primary Extended Query Table, Unique ASCII string "I"
	0x31,	// 34, Major Version Number, ASCII
	0x30,	// 35, Minor Version Number, ASCII
	0x0F,	// 36, Optional Feature & Command Support
	0x00,	// 37,					"
	0x00,	// 38,					"
	0x00,	// 39,					"
	0x01,	// 3A, Supported Functions after Suspend
	0x03,	// 3B, Block Status Register Mask
	0x00,	// 3C,					"
	0x50,	// 3D, Vcc Logic Supply Optimum Program/Erase voltage
	0x50,	// 3E, Vpp [Programming] Supply Optimum Program/Erase voltage
	0x00	// 3F, ??
};


//
// write state functions
//

static VOID WrStateIdle(BYTE a, DWORD d)
{
	WSMset.bRomArray = FALSE;				// register access

	switch(a)
	{
	case READ_ARRAY: // read array mode, normal operation
		WSMset.bRomArray = TRUE;			// data array access
		WSMset.uWrState = WRS_DATA;
		WSMset.uRdState = RDS_DATA;
		break;
	case READ_ID_CODES: // read identifier codes register
		WSMset.uRdState = RDS_ID;
		break;
	case READ_QUERY: // read query register	
		WSMset.uRdState = RDS_QUERY;
		break;
	case READ_STATUS_REG: // read status register
		WSMset.uRdState = RDS_SR;
		break;
	case CLEAR_STATUS_REG: // clear status register
		WSMset.byStatusReg = 0;
		break;
	case WRITE_BUFFER: // write to buffer
		WrStateE8(d);
		break;
	case WORD_BYTE_PROG1:
	case WORD_BYTE_PROG2: // byte/word program
		WrState40(d);
		break;
	case BLOCK_ERASE: // block erase
		WrState20(d);
		break;
	case BLOCK_ERASE_SUSPEND: // block erase, word/byte program suspend
		WSMset.byStatusReg |= WSMS;			// operation suspended
		WSMset.byStatusReg &= ~ESS;			// block erase completed (because no timing emulation)
		WSMset.byStatusReg &= ~BWSS;		// program completed (because no timing emulation)
		WSMset.uRdState = RDS_SR;
		break;
	case BLOCK_ERASE_RESUME: // block erase, word/byte program resume
		WSMset.byStatusReg &= ~WSMS;		// operation in progress
		WSMset.byStatusReg &= ~ESS;			// block erase in progress
		WSMset.byStatusReg &= ~BWSS;		// program in progress
		WSMset.byStatusReg |= WSMS;			// operation completed (because no timing emulation)
		WSMset.uRdState = RDS_SR;
		break;
	case STS_CONFIG:
		WSMset.bRomArray = bFlashRomArray;	// old access mode
		WrStateB8(d);
		break;
	case SET_CLR_BLOCK_LOCK: // set/clear block lock-bits
		WrState60(d);
		break;
	case FULL_CHIP_ERASE: // full chip erase
		WrState30(d);
		break;
	default: // wrong command
		WSMset.bRomArray = bFlashRomArray;	// old access mode
		break;
	}

	if (bFlashRomArray != WSMset.bRomArray)	// new access mode
	{
		bFlashRomArray = WSMset.bRomArray;	// change register access
		Map(0x00,0xFF);						// update memory mapping
		UpdatePatches(bFlashRomArray);		// patch/unpatch ROM again
	}
	return;
}

// write to buffer initial command
static VOID WrStateE8(DWORD d)
{
	// @todo add 2nd write buffer implementation
	// @todo add program timing implementation

	WSMset.byExStatusReg = 0;				// no write buffer
	if (WSMset.byWrite1No == 0)				// buffer1 available
	{
		WSMset.byWrite1No = 1;				// buffer1 in use
		WSMset.dwWrite1Addr = d;			// byte block address of buffer1
		WSMset.byExStatusReg = WBS;			// write buffer available
		// fill write buffer
		FillMemory(WSMset.pbyWrite1,ARRAYSIZEOF(WSMset.pbyWrite1),0xFF);
		WSMset.uWrState = WRS_WR_BUFFER_N;	// set state machine
		WSMset.uRdState = RDS_XSR;
	}
	return;
}

// write to buffer number of byte
static VOID WrStateE8N(BYTE a, DWORD d)
{
	if (a < (1 << byQueryTab[0x2A]))		// byte is length information
	{
		WSMset.byWrite1No += a;				// save no. of byte to program
		WSMset.byWrite1Size = a;			// save size to check write buffer boundaries
		WSMset.dwWrite1Addr = d;			// byte block address of buffer1
		WSMset.byStatusReg &= ~WSMS;		// state machine busy
		WSMset.uWrState = WRS_WR_BUFFER_D;
	}
	else
	{
		WSMset.byWrite1No = 0;				// free write buffer
		// improper command sequence
		WSMset.byStatusReg |= (ECLBS | BWSLBS);
		WSMset.byStatusReg |= WSMS;			// data written
		WSMset.uWrState = WRS_DATA;
	}
	WSMset.uRdState = RDS_SR;
	return;
}

// write to buffer data
static VOID WrStateE8D(BYTE a, DWORD d)
{
	// first data byte
	if (WSMset.byWrite1No == WSMset.byWrite1Size + 1)
	{
		DWORD dwBlockMask = ~(((byQueryTab[0x30] << 8) | byQueryTab[0x2F]) * 256 - 1);

		// same block
		if ((WSMset.dwWrite1Addr & dwBlockMask) == (d & dwBlockMask))
		{
			WSMset.dwWrite1Addr = d;		// byte block address of buffer1
			WSMset.pbyWrite1[0] = a;		// save byte
		}
		else
		{
			WSMset.byWrite1No = 0;			// free write buffer
			// improper command sequence
			WSMset.byStatusReg |= (ECLBS | BWSLBS);
			WSMset.byStatusReg |= WSMS;		// data written
			WSMset.uWrState = WRS_DATA;
			return;
		}
	}
	else
	{
		// write address within buffer
		if (d >= WSMset.dwWrite1Addr && d <= WSMset.dwWrite1Addr + WSMset.byWrite1Size)
		{
			// save byte in buffer
			WSMset.pbyWrite1[d-WSMset.dwWrite1Addr] = a;
		}
		else
		{
			WSMset.byWrite1No = 0;			// free write buffer
			// improper command sequence
			WSMset.byStatusReg |= (ECLBS | BWSLBS);
			WSMset.byStatusReg |= WSMS;		// data written
			WSMset.uWrState = WRS_DATA;
			return;
		}
	}

	if (--WSMset.byWrite1No == 0)			// last byte written
		WSMset.uWrState = WRS_WR_BUFFER_C;	// goto confirm state
	return;
}

// write to buffer confirm
static VOID WrStateE8C(BYTE a, DWORD d)
{
	if (CONFIRM == a)						// write buffer confirm?
	{
		BYTE  byPos;

		d = WSMset.dwWrite1Addr << 1;		// nibble start address

		for (byPos = 0; byPos <= WSMset.byWrite1Size; ++byPos)
		{
			a = WSMset.pbyWrite1[byPos];	// get char from buffer

			_ASSERT(d+1 < dwRomSize);		// address valid?
			// no error set in BWSLBS, because I could alway program a "0"
			*(pbyRom+d++) &= (a & 0x0F);	// write LSB
			*(pbyRom+d++) &= (a >> 4);		// write MSB
		}
	}
	else
	{
		WSMset.byWrite1No = 0;				// free write buffer
		// improper command sequence
		WSMset.byStatusReg |= (ECLBS | BWSLBS);
	}
	WSMset.byStatusReg |= WSMS;				// data written
	WSMset.uWrState = WRS_DATA;
	return;
}

// byte/word program initial command
static VOID WrState40(DWORD d)
{
	WSMset.byStatusReg &= ~WSMS;			// state machine busy
	WSMset.uWrState = WRS_WR_BYTE;
	WSMset.uRdState = RDS_SR;
	return;
	UNREFERENCED_PARAMETER(d);
}

// byte/word program data
static VOID WrState40D(BYTE a, DWORD d)
{
	d <<= 1;								// nibble start address
	_ASSERT(d+1 < dwRomSize);				// address valid?
	// no error set in BWSLBS, because I could alway program a "0"
	*(pbyRom+d++) &= (a & 0x0F);			// write LSB
	*(pbyRom+d)   &= (a >> 4);				// write MSB
	WSMset.byStatusReg |= WSMS;				// data written
	WSMset.uWrState = WRS_DATA;
	return;
}

// block erase initial command
static VOID WrState20(DWORD d)
{
	WSMset.byStatusReg &= ~WSMS;			// state machine busy
	WSMset.uWrState = WRS_BLOCK_ERASE;
	WSMset.uRdState = RDS_SR;
	return;
	UNREFERENCED_PARAMETER(d);
}

// block erase data & confirm
static VOID WrState20C(BYTE a, DWORD d)
{
	if (CONFIRM == a)						// block erase confirm?
	{
		// lock bit of block is set
		if ((WSMset.dwLockCnfg & (1<<(d>>16))) != 0)	
		{
			WSMset.byStatusReg |= ECLBS;	// error in block erasure
			WSMset.byStatusReg |= DPS;		// lock bit detected
		}
		else
		{
			DWORD dwBlockSize = ((byQueryTab[0x30] << 8) | byQueryTab[0x2F]) * 256;

			d &= ~(dwBlockSize-1);			// start of block
			dwBlockSize *= 2;				// block size in nibbles
			_ASSERT(d+dwBlockSize <= dwRomSize); // address valid?
			// write 128K nibble
			FillMemory(pbyRom + (d << 1),dwBlockSize,0x0F);
		}
	}
	else
	{
		// improper command sequence
		WSMset.byStatusReg |= (ECLBS | BWSLBS);
	}
	WSMset.byStatusReg |= WSMS;				// block erased
	WSMset.uWrState = WRS_DATA;
	return;
}

// full chip erase initial command
static VOID WrState30(DWORD d)
{
	WSMset.byStatusReg &= ~WSMS;			// state machine busy
	WSMset.uWrState = WRS_CHIP_ERASE;
	WSMset.uRdState = RDS_SR;
	return;
	UNREFERENCED_PARAMETER(d);
}

// full chip erase confirm
static VOID WrState30C(BYTE a, DWORD d)
{
	if (CONFIRM == a)						// chip erase confirm?
	{
		UINT  i;

		WORD  wNoOfBlocks = (byQueryTab[0x2E] << 8) | byQueryTab[0x2D];
		DWORD dwBlockSize = ((byQueryTab[0x30] << 8) | byQueryTab[0x2F]) * 256;

		LPBYTE pbyBlock = pbyRom;

		dwBlockSize *= 2;					// block size in nibbles

		for (i = 0; i <= wNoOfBlocks; ++i)	// check all blocks
		{
			_ASSERT((i+1)*dwBlockSize <= dwRomSize);

			// lock bit of block is set & WP# = low, locked blocks cannot be erased
			if ((WSMset.dwLockCnfg & (1<<i)) == 0 || bWP != FALSE)
			{
				// clear block lock bit
				WSMset.dwLockCnfg &= ~(1<<i);

				// write 128K nibble
				FillMemory(pbyBlock,dwBlockSize,0x0F);
			}

			pbyBlock += dwBlockSize;		// next block
		}
	}
	else
	{
		// improper command sequence
		WSMset.byStatusReg |= (ECLBS | BWSLBS);
	}
	WSMset.byStatusReg |= WSMS;				// chip erased
	WSMset.uWrState = WRS_DATA;
	return;
	UNREFERENCED_PARAMETER(d);
}

// STS pin Configuration initial command
static VOID WrStateB8(DWORD d)
{
	WSMset.uWrState = WRS_STS_PIN_CONFIG;
	return;
	UNREFERENCED_PARAMETER(d);
}

// STS pin Configuration data
static VOID WrStateB8D(BYTE a, DWORD d)
{
	// no emulation of STS pin Configuration
	WSMset.uWrState = WRS_DATA;
	return;
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(d);
}

// Set/Clear block Lock-Bits initial command
static VOID WrState60(DWORD d)
{
	WSMset.byStatusReg &= ~WSMS;			// state machine busy
	WSMset.uWrState = WRS_LOCK_BITS;
	WSMset.uRdState = RDS_SR;
	return;
	UNREFERENCED_PARAMETER(d);
}

// Set/Clear block Lock-Bits confirm
static VOID WrState60D(BYTE a, DWORD d)
{
	UINT i;

	switch(a)
	{
	case 0x01: // set block lock bit
		if (bWP)							// WP# = high, can change block lock status
			WSMset.dwLockCnfg |= (1<<(d>>16));	// set block lock bit
		else
			WSMset.byStatusReg |= (BWSLBS | DPS); // device protect detect, WP# = low
		break;
	case CONFIRM: // clear block lock bits
		if (bWP)							// WP# = high, can change block lock status
		{
			WORD wNoOfBlocks = (byQueryTab[0x2E] << 8) | byQueryTab[0x2D];

			for (i = 0; i <= wNoOfBlocks; ++i) // clear all lock bits
			{
				WSMset.dwLockCnfg &= ~(1 << i); // clear block lock bit
			}
		}
		else
		{
			WSMset.byStatusReg |= (ECLBS | DPS); // device protect detect, WP# = low
		}
		break;
	default: // improper command sequence
		WSMset.byStatusReg |= (ECLBS | BWSLBS);
	}
	WSMset.byStatusReg |= WSMS;				// block lock-bit changed
	WSMset.uWrState = WRS_DATA;
	return;
}


//
// read state functions
//

// read array
static BYTE RdStateData(DWORD d)
{
	d <<= 1;								// nibble address
	_ASSERT(d+1 < dwRomSize);				// address valid?
	return *(pbyRom+d)|(*(pbyRom+d+1)<<4);	// get byte
}

// read identifier codes
static BYTE RdStateId(DWORD d)
{
	BYTE byData;

	d >>= 1;								// A0 is not connected, ignore it
	if ((d & 0x03) != 0x02)					// id code request
	{
		d &= 0x03;							// data repetition
		byData = byQueryTab[d];				// get data from first 4 bytes id/query table
	}
	else									// block lock table
	{
		// get data from block lock table
		byData = (BYTE) ((WSMset.dwLockCnfg >> (d >> 15)) & 1);

		d &= 0x1F;							// data repetition
		if (d >= 4)	byData |= 0x02;			// set bit 1 on wrong ID adress
	}
	return byData;
}

// read query
static BYTE RdStateQuery(DWORD d)
{
	BYTE byData;

	d >>= 1;								// A0 is not connected, ignore it
	if ((d & 0x7F) != 0x02)					// query request
	{
		d &= 0x7F;							// data repetition

		// get data from id/query table
		byData = (d >= 0x40 && d < 0x50) ? 0 : byQueryTab[d&0x3F];
	}
	else									// block lock table
	{
		// get data from block lock table
		byData = (BYTE) ((WSMset.dwLockCnfg >> (d >> 15)) & 1);
	}
	return byData;
}

// read status register
static BYTE RdStateSR(DWORD d)
{
	return WSMset.byStatusReg;
	UNREFERENCED_PARAMETER(d);
}

// read extended status register
static BYTE RdStateXSR(DWORD d)
{
	return WSMset.byExStatusReg;
	UNREFERENCED_PARAMETER(d);
}


//
// public functions
//

VOID FlashInit(VOID)
{
	// check if locking bit table has more or equal than 32 bit
	_ASSERT(sizeof(WSMset.dwLockCnfg) * 8 >= 32);

	ZeroMemory(&WSMset,sizeof(WSMset));
	strcpy((char *)WSMset.byType, "WSM");			// Write State Machine header
	WSMset.uSize = sizeof(WSMset);			// size of this structure
	WSMset.byVersion = WSMVER;				// version of flash implementation structure

	// factory setting of locking bits
	WSMset.dwLockCnfg = (1 << 0);			// first 64KB block is locked

	WSMset.uWrState = WRS_DATA;
	WSMset.uRdState = RDS_DATA;

	// data mode of ROM
	WSMset.bRomArray = bFlashRomArray = TRUE;
	return;
}

VOID FlashRead(BYTE *a, DWORD d, UINT s)
{
	BYTE v;
	printf("Flash Read:%d %d\n", d ,s);
	while (s)								// each nibble
	{
		// output muliplexer
		_ASSERT(WSMset.uRdState < ARRAYSIZEOF(fnRdState));
		v = fnRdState[WSMset.uRdState](d>>1);

		if ((d & 1) == 0)					// even address
		{
			*a++ = v & 0xf; ++d; --s;
		}
		if (s && (d & 1))					// odd address
		{
			*a++ = v >> 4; ++d; --s;
		}
	}
	return;
}

VOID FlashWrite(BYTE *a, DWORD d, UINT s)
{
	BYTE  v;
	DWORD p;
	printf("Flash Wr:%d %d\n", d ,s);
	while (s)								// each nibble
	{
		p = d >> 1;							// byte address
		if (s > 1 && (d & 1) == 0)			// more than one byte on even address
		{
			v =  *a++;						// LSB
			v |= *a++ << 4;					// MSB
			d += 2; s -= 2;
		}
		else
		{
			// get byte from output muliplexer
			_ASSERT(WSMset.uRdState < ARRAYSIZEOF(fnRdState));
			v = fnRdState[WSMset.uRdState](p);

			if (d & 1)						// odd address
				v = (v & 0x0F) | (*a << 4);	// replace MSB
			else							// even address
				v = (v & 0xF0) | *a;		// replace LSB
			++a; ++d; --s;
		}

		_ASSERT(WSMset.uWrState < ARRAYSIZEOF(fnWrState));
		fnWrState[WSMset.uWrState](v,p);	// WSM
	}
	return;
}
