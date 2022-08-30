/*
 *   mops.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */
#include "pch.h"
#include "emu48.h"
#include "ops.h"
#include "opcodes.h"
#include "io.h"
#include "i28f160.h"						// flash support

#define DEBUG_FLASH

// #define DEBUG_SERIAL						// switch for SERIAL debug purpose
// #define DEBUG_IO							// switch for I/O debug purpose
// #define DEBUG_FLASH						// switch for FLASH MEMORY debug purpose

// defines for reading an open data bus
#define READEVEN  0x0D
#define READODD   0x0E

// on mapping boundary adjusted base addresses
#define P0MAPBASE ((BYTE)(Chipset.P0Base & ~Chipset.P0Size))
#define P1MAPBASE ((BYTE)(Chipset.P1Base & ~Chipset.P1Size))
#define P2MAPBASE ((BYTE)(Chipset.P2Base & ~Chipset.P2Size))
#define BSMAPBASE ((BYTE)(Chipset.BSBase & ~Chipset.BSSize))

BOOL bFlashRomArray = TRUE;					// flag ROM mode

BYTE disp = 0;								// flag for update display area

static LPBYTE pbyRomView[2] = {NULL, NULL};	// HP49G ROM views

// CRC calculation
static WORD crc_table[] =
{
	0x0000, 0x1081, 0x2102, 0x3183, 0x4204, 0x5285, 0x6306, 0x7387,
	0x8408, 0x9489, 0xA50A, 0xB58B, 0xC60C, 0xD68D, 0xE70E, 0xF78F
};
static __inline VOID UpCRC(BYTE nib)
{
	Chipset.crc = (WORD)((Chipset.crc>>4)^crc_table[(Chipset.crc^nib)&0xf]);
}

static __inline UINT MIN(UINT a, UINT b)
{
	return (a<b)?a:b;
}

static __inline UINT MAX(UINT a, UINT b)
{
	return (a>b)?a:b;
}

// generate UCK signal
static __inline BYTE UckBit(BYTE byBaudIndex)
{
	// table content = baudrate * 16
	const DWORD dwBaudrates[] = { 19200, 30720, 38400, 61440, 76800, 122880, 153600, 245760 };

	LARGE_INTEGER lLC;

	_ASSERT(byBaudIndex < ARRAYSIZEOF(dwBaudrates));

	if ((Chipset.IORam[IOC] & SON) == 0)	// UART off
		return UCK;							// UCK bit always set

	QueryPerformanceCounter(&lLC);			// get counter value

	// calculate UCK frequency
	return (((BYTE)(((lLC.QuadPart - lAppStart.QuadPart) * dwBaudrates[byBaudIndex])
		   / lFreq.QuadPart) & 0x1) << 3);
}

// calculate nibble based linear flash address
DWORD FlashROMAddr(DWORD d)
{
	DWORD dwLinAddr;

	// 6 bit of latch (was A6-A1 of address bus)
	dwLinAddr = (Chipset.Bank_FF >> 1) & 0x3f;
	// decode A21-A18
	dwLinAddr = ((d & 0x40000) ? (dwLinAddr & 0xf) : (dwLinAddr >> 4)) << 18;
	// decode A21-A18, A17-A0
	dwLinAddr |= d & 0x3FFFF;
	return dwLinAddr;
}

// update display
static __inline VOID UpdateDisplay(DWORD d, UINT s)
{
	BYTE  p[16];
	DWORD u;
	UINT  c;

	// address in display main area?
	if ((d<Chipset.end1)&&(d+s>Chipset.start12))
	{
		// write to display main area
		u = d;								// copy destination ptr
		c = MIN(s,Chipset.end1-d);			// number of nibbles to copy

		if (d < Chipset.start12)			// first address is out of display area
		{
			u = Chipset.start12;			// set destination ptr to start of display area
			c -= Chipset.start12 - d;		// - number of bytes that aren't in display area
		}

		_ASSERT(c <= ARRAYSIZEOF(p));
		Npeek(p,u,c);						// get source data
		WriteToMainDisplay(p,u,c);
	}
	// address in display menu area?
	if ((d<Chipset.end2)&&(d+s>Chipset.start2))
	{
		// write to display menu area
		u = d;								// copy destination ptr
		c = MIN(s,Chipset.end2-d);			// number of nibbles to copy

		if (d < Chipset.start2)				// first address is out of display area
		{
			u = Chipset.start2;				// set destination ptr to start of display area
			c -= Chipset.start2 - d;		// - number of bytes that are not in display area
		}

		_ASSERT(c <= ARRAYSIZEOF(p));
		Npeek(p,u,c);						// get source data
		WriteToMenuDisplay(p,u,c);
	}
	return;
}

// port mapping

LPBYTE RMap[256] = {NULL,};
LPBYTE WMap[256] = {NULL,};

static VOID MapP0(BYTE a, BYTE b)
{
	UINT i;
	DWORD p, m;

	a = (BYTE)MAX(a,P0MAPBASE);				// adjust base to mapping boundary
	b = (BYTE)MIN(b,Chipset.P0End);
	m = (Chipset.Port0Size*2048)-1;
	p = (a<<12)&m;							// offset to begin of P0 in nibbles
	for (i=a; i<=b; i++)
	{
		// mapping area may have holes
		if (((i ^ Chipset.P0Base) & ~Chipset.P0Size) == 0)
		{
			RMap[i]=Chipset.Port0 + p;
			WMap[i]=Chipset.Port0 + p;
		}
		p = (p+0x1000)&m;
	}
	return;
}

static VOID MapBS(BYTE a, BYTE b)
{
	UINT i;

	a = (BYTE)MAX(a,BSMAPBASE);				// adjust base to mapping boundary
	b = (BYTE)MIN(b,Chipset.BSEnd);
	for (i=a;i<=b;i++)
	{
		// mapping area may have holes
		if (((i ^ Chipset.BSBase) & ~Chipset.BSSize) == 0)
		{
			RMap[i]=NULL;					// no read cycle, open data bus
			WMap[i]=NULL;
		}
	}
	return;
}

static VOID MapP1(BYTE a, BYTE b)
{
	UINT i;
	DWORD p, m;

	// clear mapping area if port1 is configured but not plugged
	a = (BYTE)MAX(a,P1MAPBASE);				// lowest address for use is P1Base
	b = (BYTE)MIN(b,Chipset.P1End);			// highest address for use is P1End

	// port1 not plugged
	if (Chipset.Port1 == NULL || !(Chipset.cards_status & PORT1_PRESENT))
	{
		for (i=a; i<=b; i++)				// scan each 2KB page
		{
			if (((i ^ Chipset.P1Base) & ~Chipset.P1Size) == 0)
			{
				RMap[i]=NULL;
				WMap[i]=NULL;
			}
		}
		return;
	}

	m = (Chipset.Port1Size*2048)-1;			// real size of module, address mask for mirroring
	p = (a<<12)&m;							// offset to begin of P1 in nibbles

	if (Chipset.cards_status & PORT1_WRITE)	// port1 write enabled
	{
		for (i=a; i<=b; i++)				// scan each 2KB page
		{
			// mapping area may have holes
			if (((i ^ Chipset.P1Base) & ~Chipset.P1Size) == 0)
			{
				RMap[i]=Chipset.Port1 + p;	// save page address for read
				WMap[i]=Chipset.Port1 + p;	// save page address for write
			}
			p = (p+0x1000)&m;				// next page, mirror page if real size smaller allocated size
		}
	}
	else									// port1 read only
	{
		for (i=a; i<=b; i++)				// scan each 2KB page
		{
			// mapping area may have holes
			if (((i ^ Chipset.P1Base) & ~Chipset.P1Size) == 0)
			{
				RMap[i]=Chipset.Port1 + p;	// save page address for read
				WMap[i]=NULL;				// no writing
			}
			p = (p+0x1000)&m;				// next page, mirror page if real size smaller allocated size
		}
	}
	return;
}

static VOID MapP2(BYTE a, BYTE b)
{
	UINT i;
	DWORD p, m;
	LPBYTE pbyTemp;

	// clear mapping area if port2 is configured but not plugged
	a = (BYTE)MAX(a,P2MAPBASE);				// adjust base to mapping boundary
	b = (BYTE)MIN(b,Chipset.P2End);

	if (Chipset.Port2Size)					// internal port2
	{
		m = (Chipset.Port2Size*2048)-1;
		p = (a<<12)&m;						// offset to begin of P0 in nibbles
		for (i=a; i<=b; i++)
		{
			// mapping area may have holes
			if (((i ^ Chipset.P2Base) & ~Chipset.P2Size) == 0)
			{
				RMap[i]=Chipset.Port2 + p;
				WMap[i]=Chipset.Port2 + p;
			}
			p = (p+0x1000)&m;
		}
		return;
	}

	// HP48SX / HP48GX
	// only fill mapping table when CE2.2 is set
	for (i=a; i<=b; i++)					// fill mapping area with not configured
	{
		// mapping area may have holes
		if (((i ^ Chipset.P2Base) & ~Chipset.P2Size) == 0)
		{
			RMap[i]=NULL;
			WMap[i]=NULL;
		}
	}

	// port2 not plugged
	if (pbyPort2 == NULL || !(Chipset.cards_status & PORT2_PRESENT))
		return;

	pbyTemp = pbyPort2;
	if (cCurrentRomType != 'S')				// bank switching only with GX
	{
		// Chipset.Port2_Bank is the saved port2 FF content
		pbyTemp += (((Chipset.Bank_FF>>1)-1)&dwPort2Mask) << 18;
	}

	// max. size per bank is 128KB
	m = (dwPort2Size > 128) ? 128 : dwPort2Size;

	m = (m * 2048) - 1;						// real size of module, address mask for mirroring
	p = (a << 12) & m;						// offset to begin of P2 in nibbles

	// SX: CE2.2 = CE2
	// GX: CE2.2 = BEN & /DA19 & /NCE3
	if (cCurrentRomType == 'S' || ((Chipset.IORam[0x29]&DA19) == 0 && (Chipset.Bank_FF&0x40)))
	{
		if (bPort2Writeable)
		{
			for (i=a; i<=b; i++)
			{
				// mapping area may have holes
				if (((i ^ Chipset.P2Base) & ~Chipset.P2Size) == 0)
				{
					RMap[i]=pbyTemp + p;
					WMap[i]=pbyTemp + p;
				}
				p = (p+0x1000)&m;
			}
		}
		else
		{
			for (i=a; i<=b; i++)
			{
				// mapping area may have holes
				if (((i ^ Chipset.P2Base) & ~Chipset.P2Size) == 0)
				{
					RMap[i]=pbyTemp + p;
				}
				p = (p+0x1000)&m;
			}
		}
	}
	return;
}

static VOID MapROM(BYTE a, BYTE b)
{
	UINT i;
	DWORD p, m;

	// HP39(+)/40G, HP49G(+) HP48Gii // CdB for HP: add apples memory
	if (cCurrentRomType == 'E' || cCurrentRomType == 'X' || cCurrentRomType == 'P' || cCurrentRomType == '2' || cCurrentRomType == 'Q')
	{
		if (bFlashRomArray)					// view flash ROM data
		{
			_ASSERT(pbyRomView[0]);			// check ROM bank set
			_ASSERT(pbyRomView[1]);

			m = (128*1024*2)-1;				// mapped in 128KB pages
			p = (a<<12)&m;					// offset to the begin of ROM in nibbles
			for (i=a; i<=b; i++)			// scan each 2KB page
			{
				RMap[i]=pbyRomView[(i & 0x40)!=0] + p;
				WMap[i]=NULL;				// no writing
				p = (p+0x1000)&m;
			}
		}
		else								// view flash ROM register
		{
			for (i=a; i<=b; i++)			// scan each 2KB page
			{
				RMap[i]=NULL;				// view flash register
				WMap[i]=NULL;				// no writing
			}
		}
		return;
	}

	// HP38G / HP48SX / HP48GX
	m = dwRomSize - 1;						// ROM address mask for mirroring
	// when 512KB ROM and DA19=0 (ROM disabled)
	if ((m & 0x80000) != 0 && (Chipset.IORam[0x29]&DA19) == 0)
		m >>= 1;							// mirror ROM at #80000 (AR18=0)
	p = (a*0x1000)&m;						// data offset in nibbles
	for (i=a;i<=b;i++)						// scan each 2KB page
	{
		RMap[i]=pbyRom + p;					// save page address for read
		WMap[i]=NULL;						// no writing
		p = (p+0x1000)&m;					// next page, mirror page if real size smaller allocated size
	}
	return;
}

VOID Map(BYTE a, BYTE b)					// maps 2KB pages with priority
{
	// On HP39/40G and HP49G Chipset.cards_status must be 0xF
	_ASSERT((cCurrentRomType!='E' && cCurrentRomType!='X'  && cCurrentRomType!='P' && cCurrentRomType!='2' && cCurrentRomType!='Q') || !Chipset.P1Cfig || Chipset.cards_status == 0xF); // CdB for HP: add apples

	// priority order is HDW, RAM, CE2, CE1, NCE3, ROM
	MapROM(a,b);							// ROM, lowest priority, always mapped
	if (cCurrentRomType == 'S')				// HP48SX
	{
		if (Chipset.BSCfig) MapBS(a,b);		// NCE3, not used in S(X)
		if (Chipset.P1Cfig) MapP1(a,b);		// CE1, port1 (lower priority than CE2)
		if (Chipset.P2Cfig) MapP2(a,b);		// CE2, port2 (higher priority than CE1)
	}
	else									// HP48GX / HP49G
	{
		if (Chipset.P2Cfig)					// NCE3, port2
		{
			// LED bit set on a HP49
			if ((cCurrentRomType=='X' || cCurrentRomType=='Q') && (Chipset.IORam[LCR]&LED))  // CdB for HP: add apples
				MapROM(a,b);				// NCE3, ROM
			else
				MapP2(a,b);					// NCE3, port2
		}
		if (Chipset.BSCfig) MapBS(a,b);		// CE1, bank select (lower priority than CE2)
		if (Chipset.P1Cfig) MapP1(a,b);		// CE2, port1 (higher priority than CE1)
	}
	if (Chipset.P0Cfig) MapP0(a,b);			// RAM, highest priority (execpt HDW)
    // CdB for HP: add apples header
    // @todo cg, bug if display header area is mapped to addr 0
	if (Chipset.d0address!=0)
	{
        RMap[Chipset.d0address]=&(Chipset.d0memory[0]); RMap[Chipset.d0address+1]=&(Chipset.d0memory[2048*2]);  
        WMap[Chipset.d0address]=&(Chipset.d0memory[0]); WMap[Chipset.d0address+1]=&(Chipset.d0memory[2048*2]);  
	}
	return;
}

VOID RomSwitch(DWORD adr)
{
	// only HP39/40G, HP49G
	if (cCurrentRomType=='E' || cCurrentRomType=='X' || cCurrentRomType=='P' || cCurrentRomType=='2' || cCurrentRomType=='Q')  // CdB for HP: add apples
	{
		Chipset.Bank_FF = adr;				// save address line
		adr = (adr >> 1) & 0x3f;			// 6 bit of latch (was A6-A1 of address bus)
		// lower  4 bit (16 banks) for 2nd ROM view
		pbyRomView[1] = pbyRom + (((adr & 0xf) * 128 * 1024 * 2) & (dwRomSize - 1));
		// higher 2 bit (4  banks) for 1st ROM view
		pbyRomView[0] = pbyRom + (((adr >> 4)  * 128 * 1024 * 2) & (dwRomSize - 1));
	}
	Map(0x00,0xFF);							// update memory mapping
	return;
}

////////////////////////////////////////////////////////////////////////////////
//
// Bus Commands
//
////////////////////////////////////////////////////////////////////////////////

VOID Config()								// configure modules in fixed order
{
	DWORD d = Npack(Chipset.C,5);			// decode size or address
	BYTE  b = (BYTE)(d>>12);				// number of 2KB pages or page address
	BYTE  s = (BYTE)(b^0xFF);				// size in pages-1, offset to last page

	// config order is HDW, RAM, CE1, CE2, NCE3
	if (!Chipset.IOCfig)					// address of HDW, first module, ROM always configured
	{
		Chipset.IOCfig = TRUE;
		Chipset.IOBase = d&0xFFFC0;			// save HDW base on a 64 nib boundary
		Map(b,b);
		return;
	}
	if (!Chipset.P0Cfg2)					// RAM size, port0
	{
		Chipset.P0Cfg2 = TRUE;
		Chipset.P0Size = s;					// offset to last used page
		return;
	}
	if (!Chipset.P0Cfig)					// RAM address, port0
	{
		Chipset.P0Cfig = TRUE;
		Chipset.P0Base = b;					// save first page address
		b &= ~Chipset.P0Size;				// adjust base to mapping boundary
		Chipset.P0End = b+Chipset.P0Size;	// save last page address
		Map(b,Chipset.P0End);				// refresh mapping
		return;
	}
	if (cCurrentRomType=='S')				// HP48SX
	{
		if (!Chipset.P1Cfg2)				// CE1 size, port1
		{
			Chipset.P1Cfg2 = TRUE;
			Chipset.P1Size = s;
			return;
		}
		if (!Chipset.P1Cfig)				// CE1 address, port1
		{
			Chipset.P1Cfig = TRUE;
			Chipset.P1Base = b;
			b &= ~Chipset.P1Size;			// adjust base to mapping boundary
			Chipset.P1End = b+Chipset.P1Size;
			Map(b,Chipset.P1End);			// refresh mapping
			return;
		}
		if (!Chipset.P2Cfg2)				// CE2 size, port2
		{
			Chipset.P2Cfg2 = TRUE;
			Chipset.P2Size = s;
			return;
		}
		if (!Chipset.P2Cfig)				// CE2 address, port2
		{
			Chipset.P2Cfig = TRUE;
			Chipset.P2Base = b;
			b &= ~Chipset.P2Size;			// adjust base to mapping boundary
			Chipset.P2End = b+Chipset.P2Size;
			Map(b,Chipset.P2End);			// refresh mapping
			return;
		}
		if (!Chipset.BSCfg2)				// NCE3 size, not used in S(X)
		{
			Chipset.BSCfg2 = TRUE;
			Chipset.BSSize = s;
			return;
		}
		if (!Chipset.BSCfig)				// NCE3 address, not used in S(X)
		{
			Chipset.BSCfig = TRUE;
			Chipset.BSBase = b;
			b &= ~Chipset.BSSize;			// adjust base to mapping boundary
			Chipset.BSEnd = b+Chipset.BSSize;
			Map(b,Chipset.BSEnd);			// refresh mapping
			return;
		}
	}
	else									// HP48GX / HP49G
	{
		if (!Chipset.BSCfg2)				// CE1 size, bank select
		{
			Chipset.BSCfg2 = TRUE;
			Chipset.BSSize = s;
			return;
		}
		if (!Chipset.BSCfig)				// CE1 address, bank select
		{
			Chipset.BSCfig = TRUE;
			Chipset.BSBase = b;
			b &= ~Chipset.BSSize;			// adjust base to mapping boundary
			Chipset.BSEnd = b+Chipset.BSSize;
			Map(b,Chipset.BSEnd);			// refresh mapping
			return;
		}
		if (!Chipset.P1Cfg2)				// CE2 size, port1
		{
			Chipset.P1Cfg2 = TRUE;
			Chipset.P1Size = s;
			return;
		}
		if (!Chipset.P1Cfig)				// CE2 address, port1
		{
			Chipset.P1Cfig = TRUE;
			Chipset.P1Base = b;
			b &= ~Chipset.P1Size;			// adjust base to mapping boundary
			Chipset.P1End = b+Chipset.P1Size;
			Map(b,Chipset.P1End);			// refresh mapping
			return;
		}
		if (!Chipset.P2Cfg2)				// NCE3 size, port2
		{
			Chipset.P2Cfg2 = TRUE;
			Chipset.P2Size = s;
			return;
		}
		if (!Chipset.P2Cfig)				// NCE3 address, port2
		{
			Chipset.P2Cfig = TRUE;
			Chipset.P2Base = b;
			b &= ~Chipset.P2Size;			// adjust base to mapping boundary
			Chipset.P2End = b+Chipset.P2Size;
			Map(b,Chipset.P2End);			// refresh mapping
			return;
		}
	}
	return;
}

VOID Uncnfg()
{
	DWORD d=Npack(Chipset.C,5);				// decode address
	BYTE  b=(BYTE)(d>>12);					// page address

	// unconfig order is HDW, RAM, CE2, CE1, NCE3
	if ((Chipset.IOCfig)&&((d&0xFFFC0)==Chipset.IOBase))
		{Chipset.IOCfig=FALSE;Map(b,b);return;}
	if ((Chipset.P0Cfig)&&((b&~Chipset.P0Size)==P0MAPBASE))
		{Chipset.P0Cfig=FALSE;Chipset.P0Cfg2=FALSE;Map(P0MAPBASE,Chipset.P0End);return;}
	if (cCurrentRomType=='S')				// HP48SX
	{
		if ((Chipset.P2Cfig)&&((b&~Chipset.P2Size)==P2MAPBASE))
			{Chipset.P2Cfig=FALSE;Chipset.P2Cfg2=FALSE;Map(P2MAPBASE,Chipset.P2End);return;}
		if ((Chipset.P1Cfig)&&((b&~Chipset.P1Size)==P1MAPBASE))
			{Chipset.P1Cfig=FALSE;Chipset.P1Cfg2=FALSE;Map(P1MAPBASE,Chipset.P1End);return;}
		if ((Chipset.BSCfig)&&((b&~Chipset.BSSize)==BSMAPBASE))
			{Chipset.BSCfig=FALSE;Chipset.BSCfg2=FALSE;Map(BSMAPBASE,Chipset.BSEnd);return;}
	}
	else									// HP48GX / HP49G
	{
		if ((Chipset.P1Cfig)&&((b&~Chipset.P1Size)==P1MAPBASE))
			{Chipset.P1Cfig=FALSE;Chipset.P1Cfg2=FALSE;Map(P1MAPBASE,Chipset.P1End);return;}
		if ((Chipset.BSCfig)&&((b&~Chipset.BSSize)==BSMAPBASE))
			{Chipset.BSCfig=FALSE;Chipset.BSCfg2=FALSE;Map(BSMAPBASE,Chipset.BSEnd);return;}
		if ((Chipset.P2Cfig)&&((b&~Chipset.P2Size)==P2MAPBASE))
			{Chipset.P2Cfig=FALSE;Chipset.P2Cfg2=FALSE;Map(P2MAPBASE,Chipset.P2End);return;}
	}
	return;
}

VOID Reset()
{
	Chipset.IOCfig=FALSE;Chipset.IOBase=0x100000;
	Chipset.P0Cfig=FALSE;Chipset.P0Cfg2=FALSE;Chipset.P0Base=0;Chipset.P0Size=0;Chipset.P0End=0;
	Chipset.BSCfig=FALSE;Chipset.BSCfg2=FALSE;Chipset.BSBase=0;Chipset.BSSize=0;Chipset.BSEnd=0;
	Chipset.P1Cfig=FALSE;Chipset.P1Cfg2=FALSE;Chipset.P1Base=0;Chipset.P1Size=0;Chipset.P1End=0;
	Chipset.P2Cfig=FALSE;Chipset.P2Cfg2=FALSE;Chipset.P2Base=0;Chipset.P2Size=0;Chipset.P2End=0;
	Map(0x00,0xFF);							// refresh mapping
	return;
}

VOID C_Eq_Id()
{
	// config order is HDW, RAM, CE1, CE2, NCE3
	if (!Chipset.IOCfig) {Nunpack(Chipset.C,(Chipset.IOBase)       ^0x00019,5);return;}
	if (!Chipset.P0Cfg2) {Nunpack(Chipset.C,(Chipset.P0Size*0x1000)^0xFF003,5);return;}
	if (!Chipset.P0Cfig) {Nunpack(Chipset.C,(Chipset.P0Base*0x1000)^0x000F4,5);return;}
	if (cCurrentRomType=='S')				// HP48SX
	{
		if (!Chipset.P1Cfg2) {Nunpack(Chipset.C,(Chipset.P1Size*0x1000)^0xFF005,5);return;}
		if (!Chipset.P1Cfig) {Nunpack(Chipset.C,(Chipset.P1Base*0x1000)^0x000F6,5);return;}
		if (!Chipset.P2Cfg2) {Nunpack(Chipset.C,(Chipset.P2Size*0x1000)^0xFF007,5);return;}
		if (!Chipset.P2Cfig) {Nunpack(Chipset.C,(Chipset.P2Base*0x1000)^0x000F8,5);return;}
		if (!Chipset.BSCfg2) {Nunpack(Chipset.C,(Chipset.BSSize*0x1000)^0xFF001,5);return;}
		if (!Chipset.BSCfig) {Nunpack(Chipset.C,(Chipset.BSBase*0x1000)^0x000F2,5);return;}
	}
	else									// HP48GX / HP49G
	{
		if (!Chipset.BSCfg2) {Nunpack(Chipset.C,(Chipset.BSSize*0x1000)^0xFF005,5);return;}
		if (!Chipset.BSCfig) {Nunpack(Chipset.C,(Chipset.BSBase*0x1000)^0x000F6,5);return;}
		if (!Chipset.P1Cfg2) {Nunpack(Chipset.C,(Chipset.P1Size*0x1000)^0xFF007,5);return;}
		if (!Chipset.P1Cfig) {Nunpack(Chipset.C,(Chipset.P1Base*0x1000)^0x000F8,5);return;}
		if (!Chipset.P2Cfg2) {Nunpack(Chipset.C,(Chipset.P2Size*0x1000)^0xFF001,5);return;}
		if (!Chipset.P2Cfig) {Nunpack(Chipset.C,(Chipset.P2Base*0x1000)^0x000F2,5);return;}
	}
	memset(Chipset.C,0,5);
	return;
}

enum MMUMAP MapData(DWORD d)				// check MMU area
{
	BYTE u = (BYTE) (d>>12);

	if (Chipset.IOCfig && ((d&0xFFFC0)==Chipset.IOBase))                     return M_IO;
	if (Chipset.P0Cfig && (((u^Chipset.P0Base) & ~Chipset.P0Size) == 0))     return M_RAM;
	if (cCurrentRomType == 'S')
	{
		if (Chipset.P2Cfig && (((u^Chipset.P2Base) & ~Chipset.P2Size) == 0)) return M_P2;
		if (Chipset.P1Cfig && (((u^Chipset.P1Base) & ~Chipset.P1Size) == 0)) return M_P1;
		if (Chipset.BSCfig && (((u^Chipset.BSBase) & ~Chipset.BSSize) == 0)) return M_BS;
	}
	else
	{
		if (Chipset.P1Cfig && (((u^Chipset.P1Base) & ~Chipset.P1Size) == 0)) return M_P1;
		if (Chipset.BSCfig && (((u^Chipset.BSBase) & ~Chipset.BSSize) == 0)) return M_BS;
		if (Chipset.P2Cfig && (((u^Chipset.P2Base) & ~Chipset.P2Size) == 0)) return M_P2;
	}
	return M_ROM;
}

VOID CpuReset(VOID)							// register setting after Cpu Reset
{
	StopTimers();							// stop timer, do here because function change Chipset.t2

	Chipset.pc = 0;
	Chipset.rstkp = 0;
	ZeroMemory(Chipset.rstk,sizeof(Chipset.rstk));
	Chipset.HST = 0;
	Chipset.SoftInt = FALSE;
	Chipset.Shutdn = TRUE;
	Chipset.inte = TRUE;					// enable interrupts
	Chipset.intk = TRUE;					// INTON
	Chipset.intd = FALSE;					// no keyboard interrupts pending
	Chipset.crc = 0;
	Chipset.Bank_FF = 0;					// state of bank switcher FF
	Chipset.FlashRomState = 0;				// WSM state of flash memory
	ZeroMemory(Chipset.IORam,sizeof(Chipset.IORam));
	Chipset.IORam[LPE] = RST;				// set ReSeT bit at hardware reset
	Reset();								// reset MMU
	Chipset.t1 = 0;							// reset timer values
	Chipset.t2 = 0;
	Chipset.loffset = 0;					// right margin
	Chipset.boffset = 0;					// left margin
	Chipset.lcounter = 0;					// number of main display lines
	Chipset.contrast = 0;					// contrast dark

	UpdateContrast(Chipset.contrast);		// update contrast
	// display update when changing to run state
	CommSetBaud();							// new baudrate
	CheckSerial();							// close serial port

	RomSwitch(Chipset.Bank_FF);				// force new memory mapping
	return;
}

VOID Npeek(BYTE *a, DWORD d, UINT s)
{
	enum MMUMAP eMap;
	DWORD u, v;
	UINT  c;
	BYTE *p;

	do
	{
		eMap = MapData(d);					// get active memory controller
		if (M_IO == eMap)					// I/O access
		{
			v = d&0x3F;

			do
			{
				if (v == LPE)
				{
					// don't read LPE content with the function ReadIO()
					c = 1;
					memcpy(a, Chipset.IORam+v, c);
					break;
				}
				if (v >= RBR_LSB && v <= RBR_MSB)
				{
					// don't read RBR content with the function ReadIO()
					c = MIN(s,RBR_MSB-v+1);
					memcpy(a, Chipset.IORam+v, c);
					break;
				}
				// all others registers
				do
				{
					if (v < LPE)
					{
						c = MIN(s,LPE-v);
						break;
					}
					if (v < RBR_LSB && (v+s) > RBR_LSB)
					{
						c = MIN(s,RBR_LSB-v);
						break;
					}
					c = MIN(s,0x40-v);
				}
				while (0);
				ReadIO(a,v,c,FALSE);
			}
			while (0);
		}
		else
		{
			u = d>>12;
			v = d&0xFFF;
			c = MIN(s,0x1000-v);
			// Flash memory Read access
			if ((cCurrentRomType=='X' || cCurrentRomType=='2' || cCurrentRomType=='P' || cCurrentRomType=='Q') && (Chipset.IORam[LCR] & LED) && M_P2 == eMap)  // CdB for HP: add apples
			{
				FlashRead(a, FlashROMAddr(d), c);
			}
			else
			{
				if ((p=RMap[u]) != NULL)	// module mapped
				{
					//memcpy(a, p+v, c);
					rom_read_nibbles(a, (uint32_t)p+v, (uint32_t)c);
					//printf("PEEK:%08x, rom:%08x\n", p+v, pbyRom);
				}
				else						// open data bus
				{
					for (u=0; u<c; ++u)		// fill all nibbles
					{
						if ((v+u) & 1)		// odd address
							a[u] = READODD;
						else				// even address
							a[u] = READEVEN;
					}
				}
			}
		}
		if (s-=c) {a+=c; d=(d+c)&0xFFFFF;}
	} while (s);
	return;
}

VOID Nread(BYTE *a, DWORD d, UINT s)
{
	enum MMUMAP eMap;
	DWORD u, v;
	UINT  c;
	BYTE *p;

	do
	{
		eMap = MapData(d);					// get active memory controller
		if (M_IO == eMap)					// I/O access
		{
			v = d&0x3F;
			c = MIN(s,0x40-v);
			ReadIO(a,v,c,TRUE);

			// reading MSB of timer2 update the CRC register
			if (v+c == 0x40) UpCRC(a[c-1]);	// timer2 MSB touched, update the CRC register
		}
		else
		{
			u = d>>12;
			v = d&0xFFF;
			c = MIN(s,0x1000-v);
			// bank switcher access
			if (cCurrentRomType!='S' && M_BS == eMap)
			{
				if (cCurrentRomType=='G')	// HP48GX
				{
					Chipset.Bank_FF = v+c;	// save FF value
					Map(Chipset.P2Base,Chipset.P2End);
				}
				if (cCurrentRomType=='E' || cCurrentRomType=='X' || cCurrentRomType=='2' || cCurrentRomType=='P' || cCurrentRomType=='Q') // HP39/40G, HP49G  // CdB for HP: add apples
				{
					RomSwitch(v+c);
				}
			}
			// Flash memory Read access
			if ((cCurrentRomType=='X'  || cCurrentRomType=='2' || cCurrentRomType=='P' || cCurrentRomType=='Q') && (Chipset.IORam[LCR] & LED) && M_P2 == eMap)  // CdB for HP: add apples
			{
				DWORD dwLinAddr = FlashROMAddr(d);

				FlashRead(a, dwLinAddr, c);

				#if defined DEBUG_FLASH
				{
					TCHAR buffer[256];
					DWORD j;
					int   i;

					i = wsprintf(buffer,_T("%.5lx: Flash Read : %.5x (%.6x),%u = "),Chipset.pc,d,dwLinAddr,c);
					for (j = 0;j < c;++j,++i)
					{
						buffer[i] = a[j];
						if (buffer[i] > 9) buffer[i] += _T('a') - _T('9') - 1;
						buffer[i] += _T('0');
					}
					buffer[i++] = _T('\n');
					buffer[i] = 0;
					OutputDebugString(buffer);
				}
				#endif
			}
			else
			{
				if ((p=RMap[u]) != NULL)	// module mapped
				{
					//memcpy(a, p+v, c);
					rom_read_nibbles(a, (uint32_t)p+v, (uint32_t)c);
					//printf("READ:%08x, rom:%08x\n", p+v, pbyRom);
				}
				// simulate open data bus
				else						// open data bus
				{
					for (u=0; u<c; ++u)		// fill all nibbles
					{
						if ((v+u) & 1)		// odd address
							a[u] = READODD;
						else				// even address
							a[u] = READEVEN;
					}
				}
			}

			for (u=0; u<c; u++)				// update CRC
				UpCRC(a[u]);
		}
		if (s-=c) {a+=c; d=(d+c)&0xFFFFF;}
	} while (s);
	return;
}

VOID Nwrite(BYTE *a, DWORD d, UINT s)
{
	enum MMUMAP eMap;
	DWORD u, v;
	UINT  c;
	BYTE *p;

	do
	{
		eMap = MapData(d);					// get active memory controller
		if (M_IO == eMap)					// I/O access
		{
			v = d&0x3F;
			c = MIN(s,0x40-v);
			WriteIO(a, v, c);
		}
		else
		{
			u = d>>12;
			v = d&0xFFF;
			c = MIN(s,0x1000-v);
			// bank switcher access
			if (cCurrentRomType!='S' && M_BS == eMap)
			{
				BOOL bWrite = FALSE;

				// write enabled
				if (Chipset.cards_status & PORT2_WRITE)
				{
					Chipset.Bank_FF = v+c-1;// save FF value
					bWrite = TRUE;			// bank switched
				}
				else						// write disabled, so latch last read cycle
				{
					if ((v & 1) != 0)		// low address odd
					{
						Chipset.Bank_FF = v;// save FF value
						bWrite = TRUE;		// bank switched
					}

					if (((v+c) & 1) != 0)	// high address odd
					{
						Chipset.Bank_FF = v+c-1;// save FF value
						bWrite = TRUE;		// bank switched
					}
				}

				if (bWrite)					// write cycle?
				{
					// HP48GX
					if (cCurrentRomType=='G') Map(Chipset.P2Base,Chipset.P2End);
					// HP39/40G, HP49G
					if (cCurrentRomType=='E' || cCurrentRomType=='X' || cCurrentRomType=='2' || cCurrentRomType=='P' || cCurrentRomType=='Q') RomSwitch(Chipset.Bank_FF);  // CdB for HP: add apples
				}
			}
			// Flash memory Write access
			if ((cCurrentRomType=='X'  || cCurrentRomType=='2' || cCurrentRomType=='P' || cCurrentRomType=='Q') && (Chipset.IORam[LCR] & LED) && M_P2 == eMap)  // CdB for HP: add apples
			{
				DWORD dwLinAddr = FlashROMAddr(d);

				FlashWrite(a, dwLinAddr, c);

				#if defined DEBUG_FLASH
				{
					TCHAR buffer[256];
					DWORD j;
					int   i;

					i = wsprintf(buffer,_T("%.5lx: Flash Write: %.5x (%.6x),%u = "),Chipset.pc,d,dwLinAddr,c);
					for (j = 0;j < c;++j,++i)
					{
						buffer[i] = a[j];
						if (buffer[i] > 9) buffer[i] += _T('a') - _T('9') - 1;
						buffer[i] += _T('0');
					}
					buffer[i++] = _T('\n');
					buffer[i] = 0;
					OutputDebugString(buffer);
				}
				#endif
			}
			else
			{
				if ((p=WMap[u]) != NULL) memcpy(p+v, a, c);
			}
		}
		if (!bGrayscale) UpdateDisplay(d, c); // update display
		a+=c;
		d=(d+c)&0xFFFFF;
	} while (s-=c);
	return;
}

DWORD Read5(DWORD d)
{
	BYTE p[5];

	Npeek(p,d,5);
	return Npack(p,5);
}

BYTE Read2(DWORD d)
{
	BYTE p[2];

	Npeek(p,d,2);
	return (BYTE)(p[0]|(p[1]<<4));
}

VOID Write5(DWORD d, DWORD n)
{
	BYTE p[5];

	Nunpack(p,n,5);
	Nwrite(p,d,5);
	return;
}

VOID Write2(DWORD d, BYTE n)
{
	BYTE p[2];

	Nunpack(p,n,2);
	Nwrite(p,d,2);
	return;
}

VOID IOBit(DWORD d, BYTE b, BOOL s)			// set/clear bit in I/O section
{
	EnterCriticalSection(&csIOLock);
	{
		if (s)
			Chipset.IORam[d] |= b;			// set bit
		else
			Chipset.IORam[d] &= ~b;			// clear bit
	}
	LeaveCriticalSection(&csIOLock);
}

VOID ReadIO(BYTE *a, DWORD d, DWORD s, BOOL bUpdate)
{
	BOOL bNINT,bNINT2;
	BOOL bLBI,bVLBI;

	BYTE c = 0xFF;							// LINECOUNT not initialized
	BOOL rbr_acc = FALSE;					// flag to receive data

	#if defined DEBUG_IO
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("%.5lx: IO read : %02x,%u\n"),Chipset.pc,d,s);
		OutputDebugString(buffer);
	}
	#endif

	do
	{
		switch (d)
		{
		case 0x00: *a = (Chipset.IORam[d]&DON)|Chipset.boffset; break;
		case 0x01: *a = Chipset.contrast&0xF; break;
		case 0x02: *a = Chipset.contrast>>4; break;
		case 0x03: *a = 0;
		case 0x04: *a = (Chipset.crc    )&0xF; break;
		case 0x05: *a = (Chipset.crc>> 4)&0xF; break;
		case 0x06: *a = (Chipset.crc>> 8)&0xF; break;
		case 0x07: *a = (Chipset.crc>>12)&0xF; break;
		case 0x08: *a = 0; break;
		case 0x09: // LPE
			*a = Chipset.IORam[d];
			if (bUpdate)
			{
				Chipset.IORam[d] &= ~RST;	// clear RST bit after reading
			}
			break;
		case 0x0A: *a = 0; break;
//		case 0x0B: *a = Chipset.IORam[d]; break;
//		case 0x0C: *a = Chipset.IORam[d]; break;
		case 0x0D: // BAUD
			*a = Chipset.IORam[d] & 0x7;
			#if defined DEBUG_SERIAL		// return BAUD value
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: BAUD Read: %x\n"),Chipset.pc,*a);
				OutputDebugString(buffer);
			}
			#endif
			*a |= UckBit(*a);				// add UCK bit to BAUD rate register
			break;
		case 0x0E:
			// SMP is !NINT and SWINT is always 0
			// clear SMP and SWINT bit
			Chipset.IORam[d] &= (ECDT | RCDT);
			// SMP is !NINT
			if ((Chipset.IORam[SRQ2] & NINT) == 0)
				Chipset.IORam[d] |= SMP;
			*a = Chipset.IORam[d];
			break;
		case 0x0F:
			// card detection disabled
			if ((Chipset.IORam[CARDCTL] & ECDT) == 0)
			{
				*a = 0;						// no cards
			}
			else
			{
				// on a HP39/40G and HP49G Chipset.cards_status bust always be 0xF
//				_ASSERT((cCurrentRomType!='E' && cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='P' && cCurrentRomType!='Q') || Chipset.cards_status == 0xF);  // CdB for HP: add apples
				*a = Chipset.cards_status;
			}
			break;
		case 0x10: // IO CONTROL
			*a = Chipset.IORam[d];			// return IO CONTROL value
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: IOC Read: %x\n"),Chipset.pc,*a);
				OutputDebugString(buffer);
			}
			#endif
			break;
		case 0x11: // RCS
			*a = Chipset.IORam[d] | RX;		// return RCS value
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: RCS Read: %x\n"),Chipset.pc,*a);
				OutputDebugString(buffer);
			}
			#endif
			break;
		case 0x12: // TCS
			*a = Chipset.IORam[d];			// return TCS value
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: TCS Read: %x\n"),Chipset.pc,*a);
				OutputDebugString(buffer);
			}
			#endif
			break;
		case 0x13: // CRER
			*a = 0;
			break;
		case 0x14: // RBR LSB
		case 0x15: // RBR MSB
			if (bUpdate)
			{
				Chipset.IORam[RCS] &= ~RBF;
				*a = Chipset.IORam[d];			// return RBR value
				UpdateUSRQ();					// update USRQ
				rbr_acc = TRUE;					// search for new RBR value
				#if defined DEBUG_SERIAL
				{
					TCHAR buffer[256];
					wsprintf(buffer,_T("%.5lx: RBR %s Read: %x\n"),Chipset.pc,(d==0x14) ? "LSB" : "MSB",*a);
					OutputDebugString(buffer);
				}
				#endif
			}
			else
			{
				*a = Chipset.IORam[d];			// return RBR value
				UpdateUSRQ();					// update USRQ
			}
			break;
//		case 0x16: *a = Chipset.IORam[d]; break; // TBR LSB
//		case 0x17: *a = Chipset.IORam[d]; break; // TBR MSB
		case 0x19: // SREQ? MSB
			UpdateKdnBit();					// update KDN bit
			bNINT2 = Chipset.IORam[SRQ1] == 0 && (Chipset.IORam[SRQ2] & LSRQ) == 0;
			bNINT  = (Chipset.IORam[SRQ2] & NINT) != 0;
			// card detection off and timer running
			if ((Chipset.IORam[CARDCTL] & ECDT) == 0 && (Chipset.IORam[TIMER2_CTRL] & RUN) != 0)
			{
				// state of CDT2
				bNINT2 = bNINT2 && (Chipset.cards_status & (P2W|P2C)) != P2C;
				// state of CDT1
				bNINT  = bNINT  && (Chipset.cards_status & (P1W|P1C)) != P1C;
			}
			IOBit(SRQ2,NINT2,bNINT2);
			IOBit(SRQ2,NINT,bNINT);
			// no break!
		case 0x18: // SREQ? LSB
			*a = Chipset.IORam[d];			// return SREQ value
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: SEQ %s Read: %x\n"),Chipset.pc,(d==0x18) ? "LSB" : "MSB",*a);
				OutputDebugString(buffer);
			}
			#endif
			break;
		case 0x1A: // IR CONTROL
			if (cCurrentRomType=='E')		// HP39/40G
			{
				Chipset.IORam[d] = (nCurrentClass != 40)
				                 ? (Chipset.IORam[d] & ~IRI)	// HP39G
								 : (Chipset.IORam[d] |  IRI);	// HP40G
			}
			*a = Chipset.IORam[d];			// return IR CONTROL value
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: IRC Read: %x\n"),Chipset.pc,*a);
				OutputDebugString(buffer);
			}
			#endif
			break;
		case 0x1B: *a = 0; break;
		case 0x1C: *a = 0; break; // LED CONTROL
		case 0x1D: *a = 0; break; // LED BUFFER
//		case 0x1E: *a = Chipset.IORam[d]; break;
//		case 0x1F: *a = Chipset.IORam[d]; break;
		case 0x20: *a = 3; break;
		case 0x21: *a = 3; break;
		case 0x22: *a = 3; break;
		case 0x23: *a = 3; break;
		case 0x24: *a = 3; break;
		case 0x25: *a = 3; break;
		case 0x26: *a = 3; break;
		case 0x27: *a = 3; break;
		case 0x28: // LINECOUNT LSB
		case 0x29: // LINECOUNT MSB + DA19 M32
			if (Chipset.IORam[0x00]&DON)	// display on
			{
				if (c == 0xFF)				// no actual line information
				{
					c = GetLineCounter();	// get LCD update line
					// save line information in IO registers
					Chipset.IORam[0x28] = c & 0xF;
					Chipset.IORam[0x29] = (Chipset.IORam[0x29] & (DA19|M32)) | (c >> 4);
				}
			}
			*a = Chipset.IORam[d];

			if (d==0x29)					// address 0x29 is mirrored to 0x2A-0x2D
			{
				Chipset.IORam[0x2A] = *a;
				Chipset.IORam[0x2B] = *a;
				Chipset.IORam[0x2C] = *a;
				Chipset.IORam[0x2D] = *a;
			}
			break;
//		case 0x2A: *a = 0; break;
//		case 0x2B: *a = 0; break;
//		case 0x2C: *a = 0; break;
//		case 0x2D: *a = 0; break;
		case 0x2E:
			ReadT1();						// dummy read for update timer1 control register
			*a = Chipset.IORam[d];
			break;
		case 0x2F:
			ReadT2();						// dummy read for update timer2 control register
			*a = Chipset.IORam[d];
			break;
		case 0x30: *a = 3; break;
		case 0x31: *a = 3; break;
		case 0x32: *a = 3; break;
		case 0x33: *a = 3; break;
		case 0x34: *a = 3; break;
		case 0x35: *a = 0; break;
		case 0x36: *a = 0; break;
		case 0x37: *a = ReadT1(); break;
		case 0x38: Nunpack(a, ReadT2()    , s); return;
		case 0x39: Nunpack(a, ReadT2()>> 4, s); return;
		case 0x3A: Nunpack(a, ReadT2()>> 8, s); return;
		case 0x3B: Nunpack(a, ReadT2()>>12, s); return;
		case 0x3C: Nunpack(a, ReadT2()>>16, s); return;
		case 0x3D: Nunpack(a, ReadT2()>>20, s); return;
		case 0x3E: Nunpack(a, ReadT2()>>24, s); return;
		case 0x3F: Nunpack(a, ReadT2()>>28, s); return;
		default: *a = Chipset.IORam[d];
		}
		d++; a++;
	} while (--s);
	if (rbr_acc) CommReceive();				// look for new character
	return;
}

VOID WriteIO(BYTE *a, DWORD d, DWORD s)
{
	DWORD b;
	BYTE  c;
	BOOL  tbr_acc = FALSE;					// flag to transmit data
	BOOL  bDISPADDR = FALSE;				// flag addr 0x120-0x124 changed
	BOOL  bLINEOFFS = FALSE;				// flag addr 0x125-0x127 changed
	BOOL  bMENUADDR = FALSE;				// flag addr 0x130-0x134 changed

	#if defined DEBUG_IO
	{
		TCHAR buffer[256];
		DWORD j;
		int   i;

		i = wsprintf(buffer,_T("%.5lx: IO write: %02x,%u = "),Chipset.pc,d,s);
		for (j = 0;j < s;++j,++i)
		{
			buffer[i] = a[j];
			if (buffer[i] > 9) buffer[i] += _T('a') - _T('9') - 1;
			buffer[i] += _T('0');
		}
		buffer[i++] = _T('\n');
		buffer[i] = 0;
		OutputDebugString(buffer);
	}
	#endif

	do
	{
		c = *a;
		switch (d)
		{
// 00100 =  NS:DISPIO
// 00100 @  Display bit offset and DON [DON OFF2 OFF1 OFF0]
// 00100 @  3 nibs for display offset (scrolling), DON=Display ON
		case 0x00:
			if ((c^Chipset.IORam[d])&DON)	// DON bit changed
			{
				disp |= (DISP_POINTER | DISP_MAIN | DISP_MENUE);

				// adjust VBL counter start/stop values
				if ((c & DON) != 0)			// set display on
				{
					Chipset.IORam[d] |= DON; // for StartDisplay();
					UpdateContrast(Chipset.contrast);
					StartDisplay((BYTE) Chipset.lcounter); // start display update
				}
				else						// display is off
				{
					Chipset.IORam[d] &= ~DON;
					UpdateContrast(Chipset.contrast);
					StopDisplay();			// stop display update
				}
			}
			// OFF bits changed
			if ((c^Chipset.IORam[d]) & (OFF2 | OFF1 | OFF0))
			{
				Chipset.boffset = c & (OFF2 | OFF1 | OFF0);
				disp |= (DISP_POINTER | DISP_MAIN);
			}
			Chipset.IORam[d] = c;
			break;

// 00101 =  NS:CONTRLSB
// 00101 @  Contrast Control [CON3 CON2 CON1 CON0]
// 00101 @  Higher value = darker screen
		case 0x01:
			if (c!=Chipset.IORam[d])
			{
				Chipset.IORam[d]=c;
				Chipset.contrast = (Chipset.contrast&0x10)|c;
				UpdateContrast(Chipset.contrast);
				disp |= (DISP_MAIN | DISP_MENUE);
			}
			break;

// 00102 =  NS:DISPTEST
// 00102 @  Display test [VDIG LID TRIM CON4] [LRT LRTD LRTC BIN]
// 00102 @  Normally zeros
		case 0x02:
			if (c!=Chipset.IORam[d])
			{
				Chipset.IORam[d]=c;
				Chipset.contrast = (Chipset.contrast&0x0f)|((c&1)<<4);
				UpdateContrast(Chipset.contrast);
				disp |= (DISP_MAIN | DISP_MENUE);
			}
			break;

		case 0x03: Chipset.IORam[d]=c; break;

// 00104 =  HP:CRC
// 00104 @  16 bit hardware CRC (104-107) (X^16+X^12+X^5+1)
// 00104 @  crc = ( crc >> 4 ) ^ ( ( ( crc ^ nib ) & 0x000F ) * 0x1081 );
		case 0x04: Chipset.crc = (Chipset.crc&0xfff0)|(c*0x0001); break;
		case 0x05: Chipset.crc = (Chipset.crc&0xff0f)|(c*0x0010); break;
		case 0x06: Chipset.crc = (Chipset.crc&0xf0ff)|(c*0x0100); break;
		case 0x07: Chipset.crc = (Chipset.crc&0x0fff)|(c*0x1000); break;

// 00108 =  NS:POWERSTATUS
// 00108 @  Low power registers (108-109)
// 00108 @  [LB2 LB1 LB0 VLBI] (read only)
// 00108 @  LowBat(2) LowBat(1) LowBat(S) VeryLowBat
		case 0x08: break; // read-only

// 00109 =  NS:POWERCTRL
// 00109 @  [ELBI EVLBI GRST RST] (read/write)
		case 0x09:
			Chipset.IORam[d]=c;
			if (c & RST)
			{
				CpuReset();					// emulate NRES signal
				disp |= (DISP_POINTER | DISP_MAIN | DISP_MENUE | DISP_ANNUN);
				bInterrupt = TRUE;			// SHUTDN
			}
			break;

// 0010A =  NS:MODE
// 0010A @  Mode Register (read-only)
		case 0x0A: break; // read-only

// 0010B =  HP:ANNCTRL
// 0010B @  Annunciator control [LA4 LA3 LA2 LA1] = [ alarm alpha -> <- ]
		case 0x0B:
		case 0x0C:
			if (c!=Chipset.IORam[d])
			{
				Chipset.IORam[d] = c;
				disp |= DISP_ANNUN;
			}
			break;

// 0010D =  NS:BAUD
// 0010D @  Serial baud rate [UCK BD2 BD1 BD0] (bit 3 is read-only)
// 0010D @  3 bits = {1200 1920 2400 3840 4800 7680 9600 15360}
		case 0x0D:
			Chipset.IORam[d]=(Chipset.IORam[d]&8)|(c&7); // bit 3 is read-only
			CommSetBaud();					// set baudrate
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: BAUD write: %x\n"),Chipset.pc,Chipset.IORam[d]);
				OutputDebugString(buffer);
			}
			#endif
			break;

// 0010E =  NS:CARDCTL
// 0010E @  [ECDT RCDT SMP SWINT] (read/write)
// 0010E @  Enable Card Det., Run Card Det., Set Module Pulled, Software interrupt
		case 0x0E:
			if (c & SWINT)					// SWINT bit set
			{
				c &= (ECDT | RCDT | SMP);	// clear SWINT bit
				Chipset.SoftInt = TRUE;
				bInterrupt = TRUE;
			}
			if ((c & SMP) == 0)				// SMP bit cleared
			{
				BOOL bNINT = TRUE;			// ack NINT interrupt -> NINT high
				// card detect disabled and CDT1 low -> retrigger
				if ((c & ECDT) == 0 && (Chipset.IORam[TIMER2_CTRL] & RUN) != 0)
					bNINT = (Chipset.cards_status & (P1W|P1C)) != P1C;
				IOBit(SRQ2,NINT,bNINT);
			}
			// falling edge of Enable Card Detect bit and timer running
			if (   ((c^Chipset.IORam[d]) & ECDT) != 0 && (c & ECDT) == 0
				&& (Chipset.IORam[TIMER2_CTRL] & RUN) != 0)
			{
				BOOL bNINT  = (Chipset.IORam[SRQ2] & NINT) != 0;
				// card in slot1 isn't Read Only
				if ((Chipset.cards_status & (P1W|P1C)) != P1C)
				{
					// use random state of NINT line
					bNINT = bNINT && (ReadT2() & 0x1) != 0;
				}
				IOBit(SRQ2,NINT,bNINT);

				Chipset.HST |= MP;			// set Module Pulled

				// Port1 and Port2 plugged and writeable or NINT2/NINT interrupt
				if (   Chipset.cards_status != (P2W|P1W|P2C|P1C)
					|| (Chipset.IORam[SRQ2] & NINT2) == 0
					|| (Chipset.IORam[SRQ2] & NINT ) == 0
				   )
				{
					Chipset.SoftInt = TRUE;	// set interrupt
					bInterrupt = TRUE;
				}
			}
			Chipset.IORam[d]=c;
			break;

// 0010F =  NS:CARDSTATUS
// 0010F @  [P2W P1W P2C P1C] (read-only) Port 2 writable .. Port 1 inserted
		case 0x0F: break; // read-only

// 00110 =  HP:IOC
// 00110 @  Serial I/O Control [SON ETBE ERBF ERBZ]
// 00110 @  Serial On, Interrupt On Recv.Buf.Empty, Full, Buzy
		case 0x10:
			Chipset.IORam[d]=c;
			CheckSerial();					// handle UART on/off
			if ((c & SON) == 0)				// SON bit cleared
			{
				Chipset.IORam[IOC] = 0;		// clear IOC
				Chipset.IORam[RCS] = 0;		// clear RCS
				Chipset.IORam[TCS] = 0;		// clear TCS
				Chipset.IORam[RBR_LSB] = 0;	// clear RBR
				Chipset.IORam[RBR_MSB] = 0;
				Chipset.IORam[TBR_LSB] = 0;	// clear TBR
				Chipset.IORam[TBR_MSB] = 0;
			}
			if (UpdateUSRQ()) INTERRUPT;	// update USRQ bit
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: IOC write: %x\n"),Chipset.pc,Chipset.IORam[d]);
				OutputDebugString(buffer);
			}
			#endif
			break;

// 00111 =  HP:RCS
// 00111  Serial Receive Control/Status [RX RER RBZ RBF] (bit 3 is read-only)
		case 0x11:
			if (Chipset.IORam[IOC] & SON)
			{
				EnterCriticalSection(&csIOLock);
				{
					// critical section because of RER access
					Chipset.IORam[d]=(Chipset.IORam[d]&8)|(c&7);
				}
				LeaveCriticalSection(&csIOLock);
				#if defined DEBUG_SERIAL
				{
					TCHAR buffer[256];
					wsprintf(buffer,_T("%.5lx: RCS write: %x\n"),Chipset.pc,Chipset.IORam[d]);
					OutputDebugString(buffer);
				}
				#endif
			}
			break;

// 00112 =  HP:TCS
// 00112 @  Serial Transmit Control/Status [BRK LPB TBZ TBF]
		case 0x12:
			if (Chipset.IORam[IOC] & SON)
			{
				Chipset.IORam[d]=c;
				CommTxBRK();				// update BRK condition
				#if defined DEBUG_SERIAL
				{
					TCHAR buffer[256];
					wsprintf(buffer,_T("%.5lx: TCS write: %x\n"),Chipset.pc,Chipset.IORam[d]);
					OutputDebugString(buffer);
				}
				#endif
			}
			break;

// 00113 =  HP:CRER
// 00113 @  Serial Clear RER (writing anything clears RER bit)
		case 0x13:
			IOBit(RCS,RER,FALSE);
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: CRER write: %x\n"),Chipset.pc,Chipset.IORam[d]);
				OutputDebugString(buffer);
			}
			#endif
			break;

// 00114 =  HP:RBR
// 00114 @  Serial Receive Buffer Register (Reading clears RBF bit)
		case 0x14: break; // read-only
		case 0x15: break; // read-only

// 00116 =  HP:TBR
// 00116 @  Serial Transmit Buffer Register (Writing sets TBF bit)
		case 0x16:
		case 0x17:
			if (Chipset.IORam[IOC] & SON)
			{
				Chipset.IORam[d]=c;
				tbr_acc = TRUE;				// new TBR value
				#if defined DEBUG_SERIAL
				{
					TCHAR buffer[256];
					wsprintf(buffer,_T("%.5lx: TBR %s write: %x\n"),Chipset.pc,(d==0x16) ? "LSB" : "MSB",*a);
					OutputDebugString(buffer);
				}
				#endif
			}
			break;

// 00118 =  NS:SRR
// 00118 @  Service Request Register (read-only)
// 00118 @  [ISRQ TSRQ USRQ VSRQ] [KDN NINT2 NINT LSRQ]
		case 0x18: break; // read-only
		case 0x19: break; // read-only

// 0011A =  HP:IRC
// 0011A @  IR Control Register [IRI EIRU EIRI IRE] (bit 3 is read-only)
// 0011A @  IR Input, Enable IR UART mode, Enable IR Interrupt, IR Event
		case 0x1A:
			// COM port open and EIRU bit changed
			if (bCommInit && ((c^Chipset.IORam[d]) & EIRU) != 0)
			{
				// save new value for COM open
				Chipset.IORam[d]=(Chipset.IORam[d]&8)|(c&7);
				// reopen COM port with new setting
				bCommInit = CommOpen(szSerialWire,szSerialIr);
			}
			Chipset.IORam[d]=(Chipset.IORam[d]&8)|(c&7);
			#if defined DEBUG_SERIAL
			{
				TCHAR buffer[256];
				wsprintf(buffer,_T("%.5lx: IRC write: %x\n"),Chipset.pc,Chipset.IORam[d]);
				OutputDebugString(buffer);
			}
			#endif
			break;

// 0011B =  NS:BASENIBOFF
// 0011B @  Used as addressto get BASENIB from 11F to the 5th nibble
		case 0x1B: break;

// 0011C =  NS:LCR
// 0011C @  Led Control Register [LED ELBE LBZ LBF] (Setting LED is draining)
		case 0x1C:
			// HP49G new mapping on LED bit change
			if (cCurrentRomType=='X' && ((c^Chipset.IORam[d])&LED))
			{
				Chipset.IORam[d]=c;			// save new value for mapping
				Map(Chipset.P2Base,Chipset.P2End); // new ROM mapping
				#if defined DEBUG_FLASH
				{
					TCHAR buffer[256];
					wsprintf(buffer,_T("%.5lx: NCE3: R%cM\n"),Chipset.pc,(c&LED) ? 'O' : 'A');
					OutputDebugString(buffer);
				}
				#endif
			}
			Chipset.IORam[d]=c;
			break;

// 0011D =  NS:LBR
// 0011D @  Led Buffer Register [0 0 0 LBO] (bits 1-3 read zero)
		case 0x1D: Chipset.IORam[d]=c&1; break;

// 0011E =  NS:SCRATCHPAD
// 0011E @  Scratch pad
		case 0x1E: Chipset.IORam[d]=c; break;

// 0011F =  NS:BASENIB
// 0011F @  7 or 8 for base memory
		case 0x1F: Chipset.IORam[d]=c; break;

// 00120 =  NS:DISPADDR
// 00120 @  Display Start Address (write only)
// 00120 @  bit 0 is ignored (display must start on byte boundary)
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
			Chipset.IORam[d]=c;
			bDISPADDR = TRUE;				// addr 0x120-0x124 changed
			break;

// 00125 =  NS:LINEOFFS
// 00125 @  Display Line offset (write only) (no of bytes skipped after each line)
// 00125 @  MSG sign extended
		case 0x25:
		case 0x26:
		case 0x27:
			Chipset.IORam[d]=c;
			bLINEOFFS = TRUE;				// addr 0x125-0x127 changed
			break;

// 00128 =  NS:LINECOUNT
// 00128 @  Display Line Counter and miscellaneous (28-29)
// 00128 @  [LC3 LC2 LC1 LC0] [DA19 M32 LC5 LC4]
// 00128 @  Line counter 6 bits -> max = 2^6-1 = 63 = disp height
// 00128 @  Normally has 55 -> Menu starts at display row 56
		case 0x28:
			// LSB of LINECOUNT changed
			if (c != (BYTE) (Chipset.lcounter & 0xf))
			{
				Chipset.lcounter = (Chipset.lcounter & ~0xF) | c;
				disp |= (DISP_POINTER | DISP_MAIN | DISP_MENUE);
			}
			break;

		case 0x29:
			// MSB of LINECOUNT changed
			b = (c & 0x3) << 4;				// upper two bits
			if (b != (Chipset.lcounter & 0x30))
			{
				Chipset.lcounter = (Chipset.lcounter & ~0x30) | b;
				disp |= (DISP_POINTER | DISP_MAIN | DISP_MENUE);
			}

			if ((c^Chipset.IORam[d])&DA19)	// DA19 changed
			{
				Chipset.IORam[d]^=DA19;		// save new DA19
				Map(0x00,0xFF);				// new ROM mapping
			}
			break;

		case 0x2A: break;
		case 0x2B: break;
		case 0x2C: break;
		case 0x2D: break;

// 0012E =  NS:TIMER1CTRL
// 0012E @  TIMER1 Control [SRQ WKE INT XTRA]
		case 0x2E:
			Chipset.IORam[d]=c;				// don't clear XTRA bit
			ReadT1();						// dummy read for checking control bits
			break;

// 0012F =  NS:TIMER2CTRL
// 0012F @  TIMER2 Control [SRQ WKE INT RUN]
		case 0x2F:
			Chipset.IORam[d]=c;
			ReadT2();						// dummy read for checking control bits
			if (c&1)
				StartTimers();
			else
				StopTimers();
			disp |= DISP_ANNUN;				// update annunciators
			break;

// 00130 =  NS:MENUADDR
// 00130 @  Display Secondary Start Address (write only) (30-34)
// 00130 @  Menu Display Address, no line offsets
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
			Chipset.IORam[d]=c;
			bMENUADDR = TRUE;				// addr 0x130-0x134 changed
			break;

		case 0x35: break;
		case 0x36: break;

// 00137 =  HP:TIMER1
// 00137 @  Decremented 16 times/s
		case 0x37:
			SetT1(c);						// set new value
			break;

// 00138 =  HP:TIMER2
// 00138 @  hardware timer (38-3F), decremented 8192 times/s
		// nothing - fall through to default

		default:
			Chipset.IORam[d]=c;				// write data

			if (d >= TIMER2)				// timer2 update
			{
				Nunpack(Chipset.IORam+TIMER2,ReadT2(),8);
				memcpy(Chipset.IORam+d,a,s);
				SetT2(Npack(Chipset.IORam+TIMER2,8));
				goto finish;
			}
		}
		a++; d++;
	} while (--s);

finish:
	if (bDISPADDR)							// 0x120-0x124 changed
	{
		b = Npack(Chipset.IORam+DISP1CTL,5)&0xFFFFE;
		if (b != Chipset.start1)
		{
			Chipset.start1 = b;
			disp |= (DISP_POINTER | DISP_MAIN);
		}
	}
	if (bLINEOFFS)							// addr 0x125-0x127 changed
	{
		signed short lo = (signed short)Npack(Chipset.IORam+LINENIBS,3);
		if (lo&0x800) lo-=0x1000;
		if (lo != Chipset.loffset)
		{
			Chipset.loffset = lo;
			disp |= (DISP_POINTER | DISP_MAIN);
		}
	}
	if (bMENUADDR)							// addr 0x130-0x134 changed
	{
		b = Npack(Chipset.IORam+DISP2CTL,5)&0xFFFFE;
		if (b != Chipset.start2)
		{
			Chipset.start2 = b;
			disp |= (DISP_POINTER | DISP_MENUE);
		}
	}

	if (tbr_acc)							// addr 0x116-0x117 changed
	{
		IOBit(TCS,TBF,TRUE);				// set transmit buffer full bit
		CommTransmit();						// transmit char
	}

	if (disp & DISP_POINTER)
	{
		disp &= ~DISP_POINTER;				// display pointer updated
		UpdateDisplayPointers();
	}
	if (disp & DISP_ANNUN)
	{
		disp &= ~DISP_ANNUN;				// annunciators updated
		UpdateAnnunciators();
	}
	return;
}
