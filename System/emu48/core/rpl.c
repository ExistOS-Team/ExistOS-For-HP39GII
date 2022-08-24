/*
 *   rpl.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */
#include "pch.h"
#include "emu48.h"
#include "ops.h"
#include "io.h"

//| 38G  | 39G  | 40G  | 48SX | 48GX | 49G  | Name
//#F0688 #806E9 #806E9 #7056A #806E9 #806E9 =TEMPOB
//#F068D #806EE #806EE #7056F #806EE #806EE =TEMPTOP
//#F0692 #806F3 #806F3 #70574 #806F3 #806F3 =RSKTOP   (B)
//#F0697 #806F8 #806F8 #70579 #806F8 #806F8 =DSKTOP   (D1)
//#F069C #806FD #806FD #7057E #806FD #806FD =EDITLINE
//#F0DEA #80E9B #80E9B #7066E #807ED #80E9B =AVMEM    (D)
//#F0705 #8076B #8076B #705B0 #8072F #8076B =INTRPPTR (D0)
//#F0E42 #80F02 #80F02 #706C5 #80843 #80F02 =SystemFlags

#define TEMPOB      ((cCurrentRomType=='S')?0x7056A:0x806E9)
#define TEMPTOP     ((cCurrentRomType=='S')?0x7056F:0x806EE)
#define RSKTOP      ((cCurrentRomType=='S')?0x70574:0x806F3)
#define DSKTOP      ((cCurrentRomType=='S')?0x70579:0x806F8)
#define EDITLINE    ((cCurrentRomType=='S')?0x7057E:0x806FD)
// CdB for HP: add apples
#define AVMEM       ((cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')?((cCurrentRomType=='S')?0x7066E:0x807ED):0x80E9B)
#define INTRPPTR    ((cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')?((cCurrentRomType=='S')?0x705B0:0x8072F):0x8076B)
#define SYSTEMFLAGS ((cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')?((cCurrentRomType=='S')?0x706C5:0x80843):0x80F02)

#define DOINT		0x02614		// Precision Integer (HP49G)
#define DOLNGREAL	0x0263A		// Precision Real (HP49G)
#define DOLNGCMP	0x02660		// Precision Complex (HP49G)
#define DOMATRIX	0x02686		// Symbolic matrix (HP49G)
#define DOFLASHP	0x026AC		// Flash PTR (HP49G)
#define DOAPLET		0x026D5		// Aplet (HP49G)
#define DOMINIFONT	0x026FE		// Mini Font (HP49G)
#define DOBINT		0x02911		// System Binary
#define DOREAL		0x02933		// Real
#define DOEREAL		0x02955		// Long Real
#define DOCMP		0x02977		// Complex
#define DOECMP		0x0299D		// Long Complex
#define DOCHAR		0x029BF		// Character
#define DOARRY		0x029E8		// Array
#define DOLNKARRY	0x02A0A		// Linked Array
#define DOCSTR		0x02A2C		// String
#define DOHSTR		0x02A4E		// Binary Integer
#define DOLIST		0x02A74		// List
#define DORRP		0x02A96		// Directory
#define DOSYMB		0x02AB8		// Algebraic
#define DOTAG		0x02AFC		// Tagged
#define DOEXT1		0x02BAA		// Extended Pointer
#define DOEXT		0x02ADA		// Unit
#define DOGROB		0x02B1E		// Graphic
#define DOLIB		0x02B40		// Library
#define DOBAK		0x02B62		// Backup
#define DOEXT0		0x02B88		// Library Data
#define DOEXT2		0x02BCC		// Reserved 1, Font (HP49G)
#define DOEXT3		0x02BEE		// Reserved 2
#define DOEXT4		0x02C10		// Reserved 3
#define DOCOL		0x02D9D		// Program
#define DOCODE		0x02DCC		// Code
#define DOIDNT		0x02E48		// Global Name
#define DOLAM		0x02E6D		// Local Name
#define DOROMP		0x02E92		// XLIB Name
#define SEMI		0x0312B		// ;

#define GARBAGECOL	0x0613E		// =GARBAGECOL entry for HP48S/G/GII and HP49G(+)

// check for Metakernel version
#define METAKERNEL	Metakernel()

// search for "MDGKER:MK2.30" or "MDGKER:PREVIE" in port1 of a HP48GX
static BOOL Metakernel(VOID)
{
	BOOL bMkDetect = FALSE;

	// card in slot1 of a HP48GX enabled
	if (cCurrentRomType=='G' && Chipset.Port1 && Chipset.cards_status & PORT1_PRESENT)
	{
		// check for Metakernel string "MDGKER:"
		if (!strncmp(&Chipset.Port1[12],"\xD\x4\x4\x4\x7\x4\xB\x4\x5\x4\x2\x5\xA\x3",14))
		{
			bMkDetect = TRUE;				// Metakernel detected
			// check for "MK"
			if (!strncmp(&Chipset.Port1[26],"\xD\x4\xB\x4",4))
			{
				// get version number
				WORD wVersion = ((Chipset.Port1[30] * 10) + Chipset.Port1[34]) * 10
					            + Chipset.Port1[36];

				// version newer then V2.30, then compatible with HP OS
				bMkDetect = (wVersion <= 230);
			}
		}
	}
	return bMkDetect;
}

static DWORD RPL_GarbageCol(VOID)			// RPL variables must be in system RAM
{
	CHIPSET OrgChipset;
	DWORD   dwAVMEM;
	
	// only for HP48SX, HP48GX, HP49G, HP48GII and HP49G+
	_ASSERT(   cCurrentRomType == 'S' || cCurrentRomType == 'G' || cCurrentRomType == 'X'
			|| cCurrentRomType == '2' || cCurrentRomType == 'Q');

	OrgChipset = Chipset;					// save original chipset

	// entry for =GARBAGECOL
	Chipset.P = 0;							// P=0
	Chipset.mode_dec = FALSE;				// hex mode
	Chipset.pc = GARBAGECOL;				// =GARBAGECOL entry
	rstkpush(0xFFFFF);						// return address for stopping

	while (Chipset.pc != 0xFFFFF)			// wait for stop address
	{
		EvalOpcode(FASTPTR(Chipset.pc));	// execute opcode
	}

	dwAVMEM = Npack(Chipset.C,5);			// available AVMEM
	Chipset = OrgChipset;					// restore original chipset
	return dwAVMEM;
}

BOOL RPL_GetSystemFlag(INT nFlag)
{
	DWORD dwAddr;
	BYTE byMask,byFlag;

	_ASSERT(nFlag > 0);						// first flag is 1

	// calculate memory address and bit mask
	dwAddr = SYSTEMFLAGS + (nFlag - 1) / 4;
	byMask = 1 << ((nFlag - 1) & 0x3);

	Npeek(&byFlag,dwAddr,sizeof(byFlag));
	return (byFlag & byMask) != 0;
}

DWORD RPL_SkipOb(DWORD d)
{
	BYTE X[8];
	DWORD n, l;

	Npeek(X,d,5);
	n = Npack(X, 5); // read prolog
	switch (n)
	{
        case DOFLASHP: l = (cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q') ? 5 : 12; break; // Flash PTR (HP49G)   // CdB for HP: add apples
	case DOBINT:   l = 10; break; // System Binary
	case DOREAL:   l = 21; break; // Real
	case DOEREAL:  l = 26; break; // Long Real
	case DOCMP:    l = 37; break; // Complex
	case DOECMP:   l = 47; break; // Long Complex
	case DOCHAR:   l =  7; break; // Character
	case DOROMP:   l = 11; break; // XLIB Name
	case DOMATRIX: // Symbolic matrix (HP49G)
        if (cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')   // CdB for HP: add apples
		{
			l = 5;
			break;
		}
	case DOLIST: // List
	case DOSYMB: // Algebraic
	case DOEXT:  // Unit
	case DOCOL:  // Program
		n=d+5;
		do
		{
			d=n; n=RPL_SkipOb(d);
		} while (d!=n);
		return n+5;
	case SEMI: return d; // SEMI
	case DOIDNT: // Global Name
	case DOLAM:  // Local Name
	case DOTAG:  // Tagged
		Npeek(X,d+5,2); n = 7 + Npack(X,2)*2;
		return RPL_SkipOb(d+n);
	case DORRP: // Directory
		d+=8;
		n = Read5(d);
		if (n==0)
		{
			return d+5;
		}
		else
		{
			d+=n;
			Npeek(X,d,2);
			n = Npack(X,2)*2 + 4;
			return RPL_SkipOb(d+n);
		}
	case DOINT:      // Precision Integer (HP49G)
	case DOAPLET:    // Aplet (HP49G)
	case DOMINIFONT: // Mini Font (HP49G)
        if (cCurrentRomType!='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')   // CdB for HP: add apples
		{
			l = 5;
			break;
		}
	case DOARRY:    // Array
	case DOLNKARRY: // Linked Array
	case DOCSTR:    // String
	case DOHSTR:    // Binary Integer
	case DOGROB:    // Graphic
	case DOLIB:     // Library
	case DOBAK:     // Backup
	case DOEXT0:	// Library Data
    case DOEXT1:	// Reserved 1
        if (n == DOEXT1 && cCurrentRomType != 'S')
        {
            // on HP48G series and later interpreted as DOACPTR
            l = 15; // ACcess PoinTeR
            break;
        }
	case DOEXT2:    // Reserved 1, Font (HP49G)
	case DOEXT3:    // Reserved 2
	case DOEXT4:    // Reserved 3
	case DOCODE:    // Code
		l = 5+Read5(d+5);
		break;
	case DOLNGREAL: // Precision Real (HP49G)
		l = 5;
        if (cCurrentRomType=='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')   // CdB for HP: add apples
		{
			l += Read5(d+l);
			l += Read5(d+l);
		}
		break;
	case DOLNGCMP: // Precision Complex (HP49G)
		l = 5;
        if (cCurrentRomType=='X' && cCurrentRomType!='2' && cCurrentRomType!='Q')   // CdB for HP: add apples
		{
			l += Read5(d+l);
			l += Read5(d+l);
			l += Read5(d+l);
			l += Read5(d+l);
		}
		break;
	default: return d+5;
	}
	return d+l;
}

DWORD RPL_ObjectSize(BYTE *o,DWORD s)
{
    #define SERIES49 (cCurrentRomType=='X' || cCurrentRomType=='2' || cCurrentRomType=='Q')

	DWORD n, l = 0;

	if (s < 5) return BAD_OB;				// size too small for prolog
	n = Npack(o,5);							// read prolog
	switch (n)
	{
    case DOFLASHP: l = (SERIES49) ? 12 : 5; break; // Flash PTR (HP49G)
	case DOBINT:   l = 10; break; // System Binary
	case DOREAL:   l = 21; break; // Real
	case DOEREAL:  l = 26; break; // Long Real
	case DOCMP:    l = 37; break; // Complex
	case DOECMP:   l = 47; break; // Long Complex
	case DOCHAR:   l =  7; break; // Character
	case DOROMP:   l = 11; break; // XLIB Name
	case DOMATRIX: // Symbolic matrix (HP49G)
        if (!SERIES49)
		{
			l = 5;
			break;
		}
	case DOLIST: // List
	case DOSYMB: // Algebraic
	case DOEXT:  // Unit
	case DOCOL:  // Program
		n = 5;								// prolog length
		do
		{
			l += n;
			if (l > s) return BAD_OB;		// prevent negative size argument
			n = RPL_ObjectSize(o+l,s-l);	// get new object
			if (n == BAD_OB) return BAD_OB;	// buffer overflow
		}
		while (n);
		l += 5;
		break;
	case SEMI: l =  0; break; // SEMI
	case DOIDNT: // Global Name
	case DOLAM:  // Local Name
	case DOTAG:  // Tagged
		if (s < 5 + 2) return BAD_OB;
		l = 7 + Npack(o+5,2) * 2;			// prolog + name length
		if (l > s) return BAD_OB;			// prevent negative size argument
		n = RPL_ObjectSize(o+l,s-l);		// get new object
		if (n == BAD_OB) return BAD_OB;		// buffer overflow
		l += n;
		break;
	case DORRP: // Directory
		if (s < 8 + 5) return BAD_OB;
		n = Npack(o+8,5);
		if (n == 0)							// empty dir
		{
			l = 13;
		}
		else
		{
			l = 8 + n;
			if (s < l + 2) return BAD_OB;
			n = Npack(o+l,2) * 2 + 4;
			l += n;
			if (l > s) return BAD_OB;		// prevent negative size argument
			n = RPL_ObjectSize(o+l,s-l);	// next rrp
			if (n == BAD_OB) return BAD_OB;	// buffer overflow
			l += n;
		}
		break;
	case DOINT:      // Precision Integer (HP49G)
	case DOAPLET:    // Aplet (HP49G)
	case DOMINIFONT: // Mini Font (HP49G)
        if (!SERIES49)
		{
			l = 5;
			break;
		}
	case DOARRY:    // Array
	case DOLNKARRY: // Linked Array
	case DOCSTR:    // String
	case DOHSTR:    // Binary Integer
	case DOGROB:    // Graphic
	case DOLIB:     // Library
	case DOBAK:     // Backup
	case DOEXT0:    // Library Data
    case DOEXT1:	// Reserved 1
        if (n == DOEXT1 && cCurrentRomType != 'S')
        {
            // on HP48G series and later interpreted as DOACPTR
            l = 15; // ACcess PoinTeR
            break;
        }
	case DOEXT2:    // Reserved 1, Font (HP49G)
	case DOEXT3:    // Reserved 2
	case DOEXT4:    // Reserved 3
	case DOCODE:    // Code
		if (s < 5 + 5) return BAD_OB;
		l = 5 + Npack(o+5,5);
		break;
	case DOLNGREAL: // Precision Real (HP49G)
		l = 5;
		if (SERIES49)
		{
			if (s < l + 5) return BAD_OB;
			l += Npack(o+l,5);
			if (s < l + 5) return BAD_OB;
			l += Npack(o+l,5);
		}
		break;
	case DOLNGCMP: // Precision Complex (HP49G)
		l = 5;
		if (SERIES49)
		{
			if (s < l + 5) return BAD_OB;
			l += Npack(o+l,5);
			if (s < l + 5) return BAD_OB;
			l += Npack(o+l,5);
			if (s < l + 5) return BAD_OB;
			l += Npack(o+l,5);
			if (s < l + 5) return BAD_OB;
			l += Npack(o+l,5);
		}
		break;
	default: l = 5;
	}
	return (s >= l) ? l : BAD_OB;

#undef SERIES49
}

DWORD RPL_CreateTemp(DWORD l,BOOL bGarbageCol)
{
	DWORD a, b, c;
	BYTE *p;

	l += 6;									// memory for link field (5) + marker (1) and end
	b = Read5(RSKTOP);						// tail address of rtn stack
	c = Read5(DSKTOP);						// top of data stack
	if ((b+l)>c && bGarbageCol)				// there's not enough memory to move DSKTOP
	{
		RPL_GarbageCol();					// do a garbage collection
		b = Read5(RSKTOP);					// reload tail address of rtn stack
		c = Read5(DSKTOP);					// reload top of data stack
	}
	if ((b+l)>c) return 0;					// check if now there's enough memory to move DSKTOP
	a = Read5(TEMPTOP);						// tail address of top object
	Write5(TEMPTOP, a+l);					// adjust new end of top object
	Write5(RSKTOP,  b+l);					// adjust new end of rtn stack
	Write5(AVMEM, (c-b-l)/5);				// calculate free memory (*5 nibbles)
	p = HeapAlloc(hHeap,0,b-a);				// move down rtn stack
	Npeek(p,a,b-a);
	Nwrite(p,a+l,b-a);
	HeapFree(hHeap,0,p);
	Write5(a+l-5,l);						// set object length field
	return (a+1);							// return base address of new object
}

UINT RPL_Depth(VOID)
{
	return (Read5(EDITLINE) - Read5(DSKTOP)) / 5 - 1;
}

DWORD RPL_Pick(UINT l)
{
	DWORD stkp;

	_ASSERT(l > 0);							// first stack element is one
	if (l == 0) return 0;
	if (METAKERNEL)	++l;					// Metakernel support
	if (RPL_Depth() < l) return 0;			// not enough elements on stack
	stkp = Read5(DSKTOP) + (l-1)*5;
	return Read5(stkp);						// return object address
}

VOID RPL_Replace(DWORD n)
{
	DWORD stkp;

	stkp = Read5(DSKTOP);
	if (METAKERNEL)	stkp+=5;				// Metakernel support
	Write5(stkp,n);
	return;
}

VOID RPL_Push(UINT l,DWORD n)
{
	UINT  i;
	DWORD stkp, avmem;

	if (l > RPL_Depth() + 1) return;		// invalid stack level

	avmem = Read5(AVMEM);					// amount of free memory
	if (avmem == 0) return;					// no memory free
	avmem--;								// fetch memory
	Write5(AVMEM,avmem);					// save new amount of free memory

	if (METAKERNEL) ++l;					// Metakernel, save MK object on stack level 1

	stkp = Read5(DSKTOP) - 5;				// get pointer of new stack level 1
	Write5(DSKTOP,stkp);	   				// save it

	for (i = 1; i < l; ++i)					// move down stack level entries before insert pos
	{
		Write5(stkp,Read5(stkp+5));			// move down stack level entry
		stkp += 5;							// next stack entry
	}

	Write5(stkp,n);							// save pointer of new object on given stack level
	return;
}
