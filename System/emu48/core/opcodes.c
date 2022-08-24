/*
 *   opcodes.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *   Copyright (C) 1999 Christoph Gieﬂelink
 *
 */
#include "pch.h"
#include "emu48.h"
#include "opcodes.h"
#include "apple.h"
#include "io.h"								// I/O register definitions

#define w Chipset
#define GOYES3    {if(w.carry) o_goyes3(I);else{w.pc+=2;return;}}
#define GOYES5    {if(w.carry) o_goyes5(I);else{w.pc+=2;return;}}

#if !defined _BIGENDIAN
#define REG(t,r) (*((t*)&(r)))								// little endian machine
#else
#define REG(t,r) (*((t*)((BYTE*)&(r)+sizeof(r)-sizeof(t))))	// big endian machine
#endif

#pragma intrinsic(memset,memcpy)

#include "ops.h"

// Fields start and length
UINT F_s[16] = {0/*P*/,0,2,0,15,3,0,0,0,0,0,0,0,0,0,0};
UINT F_l[16] = {1,1/*P+1*/,1,3,1,12,2,16,0,0,0,0,0,0,0,5};

VOID o00(LPBYTE I) // RTNSXM
{
	w.cycles+=9;
	w.pc = rstkpop();
	w.HST |= XM;
	return;
}

VOID o01(LPBYTE I) // RTN
{
	w.cycles+=9;
	w.pc = rstkpop();
	return;
}

VOID o02(LPBYTE I) // RTNSC
{
	w.cycles+=9;
	w.pc = rstkpop();
	w.carry = TRUE;
	return;
}

VOID o03(LPBYTE I) // RTNCC
{
	w.cycles+=9;
	w.pc = rstkpop();
	w.carry = FALSE;
	return;
}

VOID o04(LPBYTE I) // SETHEX
{
	w.cycles+=3;
	w.pc+=2;
	w.mode_dec = FALSE;
	return;
}

VOID o05(LPBYTE I) // SETDEC
{
	w.cycles+=3;
	w.pc+=2;
	w.mode_dec = TRUE;
	return;
}

VOID o06(LPBYTE I) // RSTK=C
{
	w.cycles+=8;
	w.pc+=2;
	rstkpush(Npack(w.C,5));
	return;
}

VOID o07(LPBYTE I) // C=RSTK
{
	w.cycles+=8;
	w.pc+=2;
	Nunpack(w.C,rstkpop(),5);
	return;
}

VOID o08(LPBYTE I) // CLRST
{
	w.cycles+=5;
	w.pc+=2;
	memset(w.ST, 0, 3);
	return;
}

VOID o09(LPBYTE I) // C=ST
{
	w.cycles+=5;
	w.pc+=2;
	memcpy(w.C, w.ST, 3);
	return;
}

VOID o0A(LPBYTE I) // ST=C
{
	w.cycles+=5;
	w.pc+=2;
	memcpy(w.ST, w.C, 3);
	return;
}

VOID o0B(LPBYTE I) // CSTEX
{
	w.cycles+=5;
	w.pc+=2;
	Nxchg(w.C, w.ST, 3);
	return;
}

VOID o0C(LPBYTE I) // P=P+1
{
	w.cycles+=3;
	w.pc+=2;
	if (w.P<15)
	{
		w.P++;
		w.carry=FALSE;
	}
	else
	{
		w.P=0;
		w.carry=TRUE;
	}
	PCHANGED;
	return;
}

VOID o0D(LPBYTE I) // P=P-1
{
	w.cycles+=3;
	w.pc+=2;
	if (w.P)
	{
		w.P--;
		w.carry=FALSE;
	}
	else
	{
		w.P=0xF;
		w.carry=TRUE;
	}
	PCHANGED;
	return;
}

VOID o0Ef0(LPBYTE I) // A=A&B f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.A,w.B,I[2]);
	return;
}
VOID o0Ef1(LPBYTE I) // B=B&C f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.B,w.C,I[2]);
	return;
}
VOID o0Ef2(LPBYTE I) // C=C&A f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.C,w.A,I[2]);
	return;
}
VOID o0Ef3(LPBYTE I) // D=D&C f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.D,w.C,I[2]);
	return;
}
VOID o0Ef4(LPBYTE I) // B=B&A f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.B,w.A,I[2]);
	return;
}
VOID o0Ef5(LPBYTE I) // C=C&B f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.C,w.B,I[2]);
	return;
}
VOID o0Ef6(LPBYTE I) // A=A&C f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.A,w.C,I[2]);
	return;
}
VOID o0Ef7(LPBYTE I) // C=C&D f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFand(w.C,w.D,I[2]);
	return;
}

VOID o0Ef8(LPBYTE I) // A=A!B f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.A,w.B,I[2]);
	return;
}
VOID o0Ef9(LPBYTE I) // B=B!C f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.B,w.C,I[2]);
	return;
}
VOID o0EfA(LPBYTE I) // C=C!A f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.C,w.A,I[2]);
	return;
}
VOID o0EfB(LPBYTE I) // D=D!C f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.D,w.C,I[2]);
	return;
}
VOID o0EfC(LPBYTE I) // B=B!A f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.B,w.A,I[2]);
	return;
}
VOID o0EfD(LPBYTE I) // C=C!B f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.C,w.B,I[2]);
	return;
}
VOID o0EfE(LPBYTE I) // A=A!C f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.A,w.C,I[2]);
	return;
}
VOID o0EfF(LPBYTE I) // C=C!D f
{
	w.cycles+=4+F_l[I[2]];
	w.pc+=4;
	NFor(w.C,w.D,I[2]);
	return;
}

VOID o0F(LPBYTE I) // RTI
{
	w.cycles+=9;
	w.pc = rstkpop();
	w.inte = TRUE;							// enable interrupt

	if ((w.intd && w.intk) || w.IR15X)		// keyboard interrupt pending
	{
		w.intd = FALSE;						// no keyboard interrupt pending any more
		INTERRUPT;							// restart interrupt handler
	}

	// low interrupt lines
	{
		BOOL bNINT2 = Chipset.IORam[SRQ1] == 0 && (Chipset.IORam[SRQ2] & LSRQ) == 0;
		BOOL bNINT  = (Chipset.IORam[SRQ2] & NINT) != 0;

		// card detection off and timer running
		if ((Chipset.IORam[CARDCTL] & ECDT) == 0 && (Chipset.IORam[TIMER2_CTRL] & RUN) != 0)
		{
			// state of CDT2
			bNINT2 = bNINT2 && (Chipset.cards_status & (P2W|P2C)) != P2C;
			// state of CDT1
			bNINT  = bNINT  && (Chipset.cards_status & (P1W|P1C)) != P1C;
		}

		if (!bNINT2 || !bNINT)				// NINT2 or NINT interrupt line low
			INTERRUPT;						// restart interrupt handler
	}

	// restart interrupt handler when timer interrupt
	if (w.IORam[TIMER1_CTRL]&INTR)			// INT bit of timer1 is set
		ReadT1();							// check for int

	if (w.IORam[TIMER2_CTRL]&INTR)			// INT bit of timer2 is set
		ReadT2();							// check for int
	return;
}

VOID o100(LPBYTE I) // R0=A W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R0, w.A, 16);
	return;
}

VOID o101(LPBYTE I) // R1=A W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R1, w.A, 16);
	return;
}

VOID o102(LPBYTE I) // R2=A W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R2, w.A, 16);
	return;
}

VOID o103(LPBYTE I) // R3=A W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R3, w.A, 16);
	return;
}

VOID o104(LPBYTE I) // R4=A W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R4, w.A, 16);
	return;
}

VOID o108(LPBYTE I) // R0=C W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R0, w.C, 16);
	return;
}

VOID o109(LPBYTE I) // R1=C W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R1, w.C, 16);
	return;
}

VOID o10A(LPBYTE I) // R2=C W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R2, w.C, 16);
	return;
}

VOID o10B(LPBYTE I) // R3=C W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R3, w.C, 16);
	return;
}

VOID o10C(LPBYTE I) // R4=C W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.R4, w.C, 16);
	return;
}

VOID o110(LPBYTE I) // A=R0 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.A, w.R0, 16);
	return;
}

VOID o111(LPBYTE I) // A=R1 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.A, w.R1, 16);
	return;
}

VOID o112(LPBYTE I) // A=R2 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.A, w.R2, 16);
	return;
}

VOID o113(LPBYTE I) // A=R3 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.A, w.R3, 16);
	return;
}

VOID o114(LPBYTE I) // A=R4 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.A, w.R4, 16);
	return;
}

VOID o118(LPBYTE I) // C=R0 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.C, w.R0, 16);
	return;
}

VOID o119(LPBYTE I) // C=R1 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.C, w.R1, 16);
	return;
}

VOID o11A(LPBYTE I) // C=R2 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.C, w.R2, 16);
	return;
}

VOID o11B(LPBYTE I) // C=R3 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.C, w.R3, 16);
	return;
}

VOID o11C(LPBYTE I) // C=R4 W
{
	w.cycles+=19;
	w.pc+=3;
	memcpy(w.C, w.R4, 16);
	return;
}

VOID o120(LPBYTE I) // AR0EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.A, w.R0, 16);
	return;
}

VOID o121(LPBYTE I) // AR1EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.A, w.R1, 16);
	return;
}

VOID o122(LPBYTE I) // AR2EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.A, w.R2, 16);
	return;
}

VOID o123(LPBYTE I) // AR3EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.A, w.R3, 16);
	return;
}

VOID o124(LPBYTE I) // AR4EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.A, w.R4, 16);
	return;
}

VOID o128(LPBYTE I) // CR0EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.C, w.R0, 16);
	return;
}

VOID o129(LPBYTE I) // CR1EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.C, w.R1, 16);
	return;
}

VOID o12A(LPBYTE I) // CR2EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.C, w.R2, 16);
	return;
}

VOID o12B(LPBYTE I) // CR3EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.C, w.R3, 16);
	return;
}

VOID o12C(LPBYTE I) // CR4EX W
{
	w.cycles+=19;
	w.pc+=3;
	Nxchg(w.C, w.R4, 16);
	return;
}

VOID o130(LPBYTE I) // D0=A
{
	w.cycles+=8;
	w.pc+=3;
	w.d0=Npack(w.A,5);
	return;
}

VOID o131(LPBYTE I) // D1=A
{
	w.cycles+=8;
	w.pc+=3;
	w.d1=Npack(w.A,5);
	return;
}

VOID o132(LPBYTE I) // AD0EX
{
	DWORD d = w.d0;
	w.d0=Npack(w.A,5);
	Nunpack(w.A,d,5);
	w.cycles+=8;
	w.pc+=3;
	return;
}

VOID o133(LPBYTE I) // AD1EX
{
	DWORD d=w.d1;
	w.d1=Npack(w.A,5);
	Nunpack(w.A,d,5);
	w.cycles+=8;
	w.pc+=3;
	return;
}

VOID o134(LPBYTE I) // D0=C
{
	w.cycles+=8;
	w.pc+=3;
	w.d0=Npack(w.C,5);
	return;
}

VOID o135(LPBYTE I) // D1=C
{
	w.cycles+=8;
	w.pc+=3;
	w.d1=Npack(w.C,5);
	return;
}

VOID o136(LPBYTE I) // CD0EX
{
	DWORD d=w.d0;
	w.d0=Npack(w.C,5);
	Nunpack(w.C,d,5);
	w.cycles+=8;
	w.pc+=3;
	return;
}

VOID o137(LPBYTE I) // CD1EX
{
	DWORD d=w.d1;
	w.d1=Npack(w.C,5);
	Nunpack(w.C,d,5);
	w.cycles+=8;
	w.pc+=3;
	return;
}

VOID o138(LPBYTE I) // D0=AS
{
	w.cycles+=7;
	w.pc+=3;
	REG(WORD,w.d0)=(WORD)Npack(w.A,4);
	return;
}

VOID o139(LPBYTE I) // D1=AS
{
	w.cycles+=7;
	w.pc+=3;
	REG(WORD,w.d1)=(WORD)Npack(w.A,4);
	return;
}

VOID o13A(LPBYTE I) // AD0XS
{
	DWORD d=w.d0;
	REG(WORD,w.d0)=(WORD)Npack(w.A,4);
	Nunpack(w.A,d,4);
	w.cycles+=7;
	w.pc+=3;
	return;
}

VOID o13B(LPBYTE I) // AD1XS
{
	DWORD d=w.d1;
	REG(WORD,w.d1)=(WORD)Npack(w.A,4);
	Nunpack(w.A,d,4);
	w.cycles+=7;
	w.pc+=3;
	return;
}

VOID o13C(LPBYTE I) // D0=CS
{
	w.cycles+=7;
	w.pc+=3;
	REG(WORD,w.d0)=(WORD)Npack(w.C,4);
	return;
}

VOID o13D(LPBYTE I) // D1=CS
{
	w.cycles+=7;
	w.pc+=3;
	REG(WORD,w.d1)=(WORD)Npack(w.C,4);
	return;
}

VOID o13E(LPBYTE I) // CD0XS
{
	DWORD d=w.d0;
	REG(WORD,w.d0)=(WORD)Npack(w.C,4);
	Nunpack(w.C,d,4);
	w.cycles+=7;
	w.pc+=3;
	return;
}

VOID o13F(LPBYTE I) // CD1XS
{
	DWORD d=w.d1;
	REG(WORD,w.d1)=(WORD)Npack(w.C,4);
	Nunpack(w.C,d,4);
	w.cycles+=7;
	w.pc+=3;
	return;
}

VOID o140(LPBYTE I) { w.cycles+=17; w.pc+=3; Nwrite(w.A, w.d0, 5); return; } // DAT0=A A
VOID o141(LPBYTE I) { w.cycles+=17; w.pc+=3; Nwrite(w.A, w.d1, 5); return; } // DAT1=A A
VOID o144(LPBYTE I) { w.cycles+=17; w.pc+=3; Nwrite(w.C, w.d0, 5); return; } // DAT0=C A
VOID o145(LPBYTE I) { w.cycles+=17; w.pc+=3; Nwrite(w.C, w.d1, 5); return; } // DAT1=C A
VOID o148(LPBYTE I) { w.cycles+=14; w.pc+=3; Nwrite(w.A, w.d0, 2); return; } // DAT0=A B
VOID o149(LPBYTE I) { w.cycles+=14; w.pc+=3; Nwrite(w.A, w.d1, 2); return; } // DAT1=A B
VOID o14C(LPBYTE I) { w.cycles+=14; w.pc+=3; Nwrite(w.C, w.d0, 2); return; } // DAT0=C B
VOID o14D(LPBYTE I) { w.cycles+=14; w.pc+=3; Nwrite(w.C, w.d1, 2); return; } // DAT1=C B

VOID o142(LPBYTE I) { w.cycles+=18; w.pc+=3; Nread(w.A, w.d0, 5); return; } // A=DAT0 A
VOID o143(LPBYTE I) { w.cycles+=18; w.pc+=3; Nread(w.A, w.d1, 5); return; } // A=DAT1 A
VOID o146(LPBYTE I) { w.cycles+=18; w.pc+=3; Nread(w.C, w.d0, 5); return; } // C=DAT0 A
VOID o147(LPBYTE I) { w.cycles+=18; w.pc+=3; Nread(w.C, w.d1, 5); return; } // C=DAT1 A
VOID o14A(LPBYTE I) { w.cycles+=15; w.pc+=3; Nread(w.A, w.d0, 2); return; } // A=DAT0 B
VOID o14B(LPBYTE I) { w.cycles+=15; w.pc+=3; Nread(w.A, w.d1, 2); return; } // A=DAT1 B
VOID o14E(LPBYTE I) { w.cycles+=15; w.pc+=3; Nread(w.C, w.d0, 2); return; } // C=DAT0 B
VOID o14F(LPBYTE I) { w.cycles+=15; w.pc+=3; Nread(w.C, w.d1, 2); return; } // C=DAT0 B

VOID o150a(LPBYTE I) { w.cycles+=16+F_l[I[3]]; w.pc+=4; NFwrite(w.A, w.d0, I[3]); return; } // DAT0=A a
VOID o151a(LPBYTE I) { w.cycles+=16+F_l[I[3]]; w.pc+=4; NFwrite(w.A, w.d1, I[3]); return; } // DAT1=A a
VOID o154a(LPBYTE I) { w.cycles+=16+F_l[I[3]]; w.pc+=4; NFwrite(w.C, w.d0, I[3]); return; } // DAT0=C a
VOID o155a(LPBYTE I) { w.cycles+=16+F_l[I[3]]; w.pc+=4; NFwrite(w.C, w.d1, I[3]); return; } // DAT1=C a
VOID o152a(LPBYTE I) { w.cycles+=17+F_l[I[3]]; w.pc+=4; NFread(w.A, w.d0, I[3]); return; } // A=DAT0 a
VOID o153a(LPBYTE I) { w.cycles+=17+F_l[I[3]]; w.pc+=4; NFread(w.A, w.d1, I[3]); return; } // A=DAT1 a
VOID o156a(LPBYTE I) { w.cycles+=17+F_l[I[3]]; w.pc+=4; NFread(w.C, w.d0, I[3]); return; } // C=DAT0 a
VOID o157a(LPBYTE I) { w.cycles+=17+F_l[I[3]]; w.pc+=4; NFread(w.C, w.d1, I[3]); return; } // C=DAT1 a

VOID o158x(LPBYTE I) { w.cycles+=16+I[3]+1; w.pc+=4; Nwrite(w.A, w.d0, I[3]+1); return; } // DAT0=A x
VOID o159x(LPBYTE I) { w.cycles+=16+I[3]+1; w.pc+=4; Nwrite(w.A, w.d1, I[3]+1); return; } // DAT1=A x
VOID o15Cx(LPBYTE I) { w.cycles+=16+I[3]+1; w.pc+=4; Nwrite(w.C, w.d0, I[3]+1); return; } // DAT0=C x
VOID o15Dx(LPBYTE I) { w.cycles+=16+I[3]+1; w.pc+=4; Nwrite(w.C, w.d1, I[3]+1); return; } // DAT1=C x
VOID o15Ax(LPBYTE I) { w.cycles+=17+I[3]+1; w.pc+=4; Nread(w.A, w.d0, I[3]+1); return; } // A=DAT0 x
VOID o15Bx(LPBYTE I) { w.cycles+=17+I[3]+1; w.pc+=4; Nread(w.A, w.d1, I[3]+1); return; } // A=DAT1 x
VOID o15Ex(LPBYTE I) { w.cycles+=17+I[3]+1; w.pc+=4; Nread(w.C, w.d0, I[3]+1); return; } // C=DAT0 x
VOID o15Fx(LPBYTE I) { w.cycles+=17+I[3]+1; w.pc+=4; Nread(w.C, w.d1, I[3]+1); return; } // C=DAT1 x

VOID o16x(LPBYTE I) // D0=D0+ (n+1)
{
	w.cycles+=7;
	w.pc+=3;
	w.d0+=I[2]+1;
	if (w.d0>0xfffff)
	{
		w.d0&=0xfffff;
		w.carry=TRUE;
	}
	else
	{
		w.carry=FALSE;
	}
	return;
}

VOID o17x(LPBYTE I) // D1=D1+ (n+1)
{
	w.cycles+=7;
	w.pc+=3;
	w.d1+=I[2]+1;
	if (w.d1>0xfffff)
	{
		w.d1&=0xfffff;
		w.carry=TRUE;
	}
	else
	{
		w.carry=FALSE;
	}
	return;
}

VOID o18x(LPBYTE I) // D0=D0- (n+1)
{
	w.cycles+=7;
	w.pc+=3;
	w.d0-=I[2]+1;
	if (w.d0>0xfffff)
	{
		w.d0&=0xfffff;
		w.carry=TRUE;
	}
	else
	{
		w.carry=FALSE;
	}
	return;
}

VOID o19d2(LPBYTE I) // D0=(2) #dd
{
	w.cycles+=4;
	w.pc+=4;
	REG(BYTE,w.d0)=(BYTE)Npack(I+2,2);
	return;
}

VOID o1Ad4(LPBYTE I) // D0=(4) #dddd
{
	w.cycles+=6;
	w.pc+=6;
	REG(WORD,w.d0)=(WORD)Npack(I+2,4);
	return;
}

VOID o1Bd5(LPBYTE I) // D0=(5) #ddddd
{
	w.cycles+=7;
	w.pc+=7;
	w.d0=Npack(I+2,5);
	return;
}

VOID o1Cx(LPBYTE I) // D1=D1- (n+1)
{
	w.cycles+=7;
	w.pc+=3;
	w.d1-=I[2]+1;
	if (w.d1>0xfffff)
	{
		w.d1&=0xfffff;
		w.carry=TRUE;
	}
	else
	{
		w.carry=FALSE;
	}
	return;
}

VOID o1Dd2(LPBYTE I) // D1=(2) #dd
{
	w.cycles+=4;
	w.pc+=4;
	REG(BYTE,w.d1)=(BYTE)Npack(I+2,2);
	return;
}

VOID o1Ed4(LPBYTE I) // D1=(4) #dddd
{
	w.cycles+=6;
	w.pc+=6;
	REG(WORD,w.d1)=(WORD)Npack(I+2,4);
	return;
}

VOID o1Fd5(LPBYTE I) // D1=(5) #ddddd
{
	w.cycles+=7;
	w.pc+=7;
	w.d1=Npack(I+2,5);
	return;
}

VOID o2n(LPBYTE I) // P= n
{
	w.cycles+=2;
	w.pc+=2;
	w.P=I[1];
	PCHANGED;
	return;
}

VOID o3X(LPBYTE I) // LCHEX
{
	UINT n=I[1]+1;
	UINT d=16-w.P;
	w.cycles+=3+I[1];
	w.pc+=2;
	I+=2; // UNSAFE
	if (n<=d)
	{
		memcpy(w.C+w.P,I,n);
	}
	else
	{
		memcpy(w.C+w.P,I,d);
		memcpy(w.C,I+d,n-d);
	}
	w.pc+=n;
	return;
}

VOID o4d2(LPBYTE I) // GOC #dd
{
	if (!w.carry)
	{
		w.cycles+=3;
		w.pc+=3;
	}
	else
	{
		signed char jmp=I[1]+(I[2]<<4);
		if (jmp)
			w.pc+=jmp+1;
		else
			w.pc=rstkpop();
		w.cycles+=10;
		w.pc&=0xFFFFF;
	}
	return;
}

VOID o5d2(LPBYTE I) // GONC
{
	if (w.carry)
	{
		w.cycles+=3;
		w.pc+=3;
	}
	else
	{
		signed char jmp=I[1]+(I[2]<<4);
		if (jmp)
			w.pc+=jmp+1;
		else
			w.pc=rstkpop();
		w.cycles+=10;
		w.pc&=0xFFFFF;
	}
	return;
}

VOID o6d3(LPBYTE I) // GOTO
{
	DWORD d=Npack(I+1,3);
	if (d&0x800)
		w.pc-=0xFFF-d;
	else
		w.pc+=d+1;
	w.cycles+=11;
	w.pc&=0xFFFFF;
	return;
}

VOID o7d3(LPBYTE I) // GOSUB
{
	DWORD d=Npack(I+1,3);
	rstkpush(w.pc+4);
	if (d&0x800) w.pc-=0xFFC-d; else w.pc+=d+4;
	w.cycles+=12;
	w.pc&=0xFFFFF;
	return;
}

VOID o800(LPBYTE I) // OUT=CS
{
	w.cycles+=4;
	w.pc+=3;
	w.out = (w.out&0xff0) | w.C[0];
	ScanKeyboard(FALSE,FALSE);				// 1ms keyboard poll
	return;
}

VOID o801(LPBYTE I) // OUT=C
{
	w.cycles+=6;
	w.pc+=3;
	w.out = (WORD)Npack(w.C, 3);
	ScanKeyboard(FALSE,FALSE);				// 1ms keyboard poll
	return;
}

VOID o802(LPBYTE I) // A=IN
{
	w.cycles+=7;
	// emulate Clarke/Yorke bug
	if ((w.pc & 1) == 0 || MapData(w.pc) == M_IO)
		w.pc+=3;
	ScanKeyboard(TRUE,FALSE);				// update Chipset.in register (direct)
	IOBit(0x19,8,w.in != 0);				// update KDN bit in the SRQ register
	Nunpack(w.A, w.in, 4);
	return;
}

VOID o803(LPBYTE I) // C=IN
{
	w.cycles+=7;
	// emulate Clarke/Yorke bug
	if ((w.pc & 1) == 0 || MapData(w.pc) == M_IO)
		w.pc+=3;
	ScanKeyboard(TRUE,FALSE);				// update Chipset.in register (direct)
	IOBit(0x19,8,w.in != 0);				// update KDN bit in the SRQ register
	Nunpack(w.C, w.in, 4);
	return;
}

VOID o804(LPBYTE I) // UNCNFG
{
	w.cycles+=12;
	w.pc+=3;
	Uncnfg();
	return;
}

VOID o805(LPBYTE I) // CONFIG
{
	w.cycles+=11;
	w.pc+=3;
	Config();
	return;
}

VOID o806(LPBYTE I) // C=ID
{
	w.cycles+=11;
	w.pc+=3;
	C_Eq_Id();
	return;
}

VOID o807(LPBYTE I) // SHUTDN
{
	BOOL bShutdn = TRUE;					// shut down

	// only shut down when no timer wake up
	if (w.IORam[TIMER1_CTRL]&WKE)			// WKE bit of timer1 is set
	{
		if (ReadT1()&0x08)					// and MSB of timer1 is set
		{
			w.IORam[TIMER1_CTRL] &= ~WKE;	// clear WKE
			bShutdn = FALSE;				// don't shut down
		}
	}
	if (w.IORam[TIMER2_CTRL]&WKE)			// WKE bit of timer2 is set
	{
		if (ReadT2()&0x80000000)			// and MSB of timer2 is set
		{
			w.IORam[TIMER2_CTRL] &= ~WKE;	// clear WKE
			bShutdn = FALSE;				// don't shut down
		}
	}
	ScanKeyboard(TRUE,FALSE);				// update Chipset.in register (direct)
	if (w.in == 0 && bShutdn)				// shut down only when enabled
	{
		w.Shutdn = TRUE;					// set mode before exit emulation loop
		bInterrupt = TRUE;

		// emulation of BS reset circuit in deep sleep
		// HP39/40G, HP48GX, HP49G, display off, card control off or in slow mode
		if (   (cCurrentRomType=='E' || cCurrentRomType=='G' || cCurrentRomType=='X' || cCurrentRomType=='2' || cCurrentRomType=='P' || cCurrentRomType=='Q')  // CdB for HP: add apples
		    && (w.IORam[BITOFFSET]&DON) == 0
			&& ((w.IORam[CARDCTL]&(ECDT|RCDT)) != (ECDT|RCDT)))
		{
			// on HP48GX ROM must be selected (DA19=1) and
			// the NOT MA18 address line must be low (test for high)
			// else we get power on VCO from the ROM chip
			// (MA18 input pin security diode to VCC pin)
			if (   cCurrentRomType!='G'
				|| ((w.IORam[LINECOUNT+1]&DA19) && (w.pc & 0x80000)))
			{
				w.Bank_FF = 0;				// reset bank switcher FF
				RomSwitch(w.Bank_FF);		// force new mapping
			}
		}
	}
	w.cycles+=6;
	w.pc+=3;
	return;
}

VOID o8080(LPBYTE I) // INTON
{
	w.cycles+=5;
	w.pc+=4;
	w.intk = TRUE;
	ScanKeyboard(FALSE,FALSE);				// 1ms keyboard poll
	if (w.intd || w.IR15X)					// keyboard interrupt pending
	{
		w.intd = FALSE;						// no keyboard interrupt pending any more
		INTERRUPT;							// restart interrupt handler
	}
	return;
}

VOID o80810(LPBYTE I) // RSI
{
	w.cycles+=6;
	w.pc+=5;
	ScanKeyboard(TRUE,TRUE);				// one input bit high (direct)?

	// enable KDN update
	w.dwKdnCycles = (DWORD) (w.cycles & 0xFFFFFFFF) - (DWORD) T2CYCLES * 16;

	if (w.in && w.inte == FALSE)			// key interrupt pending
		w.intd = TRUE;						// keyboard interrupt pending
	return;
}

VOID o8082X(LPBYTE I) // LA
{
	UINT n=I[4]+1;
	UINT d=16-w.P;
	w.cycles+=6+I[4];
	w.pc+=5+n;
	I+=5; // UNSAFE
	if (n<=d)
	{
		memcpy(w.A+w.P,I,n);
	}
	else
	{
		memcpy(w.A+w.P,I,d);
		memcpy(w.A,I+d,n-d);
	}
	return;
}

VOID o8083(LPBYTE I) // BUSCB
{
	w.cycles+=7;
	w.pc+=4;
	// emulated as NOP
	// InfoMessage(_T("BUSCB instruction executed."));
	return;
}

VOID o8084n(LPBYTE I) // ABIT=0 n
{
	w.cycles+=6;
	w.pc+=5;
	Nbit0(w.A, I[4]);
	return;
}

VOID o8085n(LPBYTE I) // ABIT=1 n
{
	w.cycles+=6;
	w.pc+=5;
	Nbit1(w.A, I[4]);
	return;
}

VOID o8086n(LPBYTE I) // ?ABIT=0 n
{
	w.cycles+=9;
	w.pc+=5;
	Tbit0(w.A, I[4]);
	GOYES5;
}

VOID o8087n(LPBYTE I) // ?ABIT=1 n
{
	w.cycles+=9;
	w.pc+=5;
	Tbit1(w.A, I[4]);
	GOYES5;
}

VOID o8088n(LPBYTE I) // CBIT=0 n
{
	w.cycles+=6;
	w.pc+=5;
	Nbit0(w.C, I[4]);
	return;
}

VOID o8089n(LPBYTE I) // CBIT=1 n
{
	w.cycles+=6;
	w.pc+=5;
	Nbit1(w.C, I[4]);
	return;
}

VOID o808An(LPBYTE I) // ?CBIT=0 n
{
	w.cycles+=9;
	w.pc+=5;
	Tbit0(w.C, I[4]);
	GOYES5;
}

VOID o808Bn(LPBYTE I) // ?CBIT=1 n
{
	w.cycles+=9;
	w.pc+=5;
	Tbit1(w.C, I[4]);
	GOYES5;
}

VOID o808C(LPBYTE I) // PC=(A)
{
	BYTE p[5];
	w.cycles+=23;
	Nread(p,Npack(w.A,5),5);				// read (A) and update CRC
	w.pc=Npack(p,5);
	return;
}

VOID o808D(LPBYTE I) // BUSCD
{
	w.cycles+=7;
	w.pc+=4;
	// emulated as NOP
	// InfoMessage(_T("BUSCD instruction executed."));
	return;
}

VOID o808E(LPBYTE I) // PC=(C)
{
	BYTE p[5];
	w.cycles+=23;
	Nread(p,Npack(w.C,5),5);				// read (C) and update CRC
	w.pc=Npack(p,5);
	return;
}

VOID o808F(LPBYTE I) // INTOFF
{
	w.cycles+=5;
	w.pc+=4;
	UpdateKdnBit();							// update KDN bit
	w.intk = FALSE;
	return;
}

VOID o809(LPBYTE I) // C+P+1 - HEX MODE
{
	w.cycles+=8;
	w.pc+=3;
	w.C[0]+=w.P; Nincx(w.C,5);
	return;
}

VOID o80A(LPBYTE I) // RESET
{
	w.cycles+=6;
	w.pc+=3;
	Reset();
	return;
}

VOID o80B(LPBYTE I) // BUSCC
{
	w.cycles+=6;
	w.pc+=3;
	// emulated as NOP
	// InfoMessage(_T("BUSCC instruction executed."));
	if (cCurrentRomType=='Q' || cCurrentRomType=='2' || cCurrentRomType=='P')
	{
		o80BExt(I);							// Saturnator extentions
	}
	return;
}

VOID o80Cn(LPBYTE I) // C=P n
{
	w.cycles+=6;
	w.pc+=4;
	w.C[I[3]] = w.P;
	return;
}

VOID o80Dn(LPBYTE I) // P=C n
{
	w.cycles+=6;
	w.pc+=4;
	w.P = w.C[I[3]];
	PCHANGED;
	return;
}

VOID o80E(LPBYTE I) // SREQ?
{
	w.cycles+=7;
	w.pc+=3;
	w.C[0]=0;								// no device
	return;
}

VOID o80Fn(LPBYTE I) // CPEX n
{
	BYTE n = w.P;
	w.P = w.C[I[3]];
	w.C[I[3]] = n;
	PCHANGED;
	w.cycles+=6;
	w.pc+=4;
	return;
}

VOID o810(LPBYTE I) // ASLC
{
	w.cycles+=21;
	w.pc+=3;
	Nslc(w.A, 16);
	return;
}

VOID o811(LPBYTE I) // BSLC
{
	w.cycles+=21;
	w.pc+=3;
	Nslc(w.B, 16);
	return;
}

VOID o812(LPBYTE I) // CSLC
{
	w.cycles+=21;
	w.pc+=3;
	Nslc(w.C, 16);
	return;
}

VOID o813(LPBYTE I) // DSLC
{
	w.cycles+=21;
	w.pc+=3;
	Nslc(w.D, 16);
	return;
}

VOID o814(LPBYTE I) // ASRC
{
	w.cycles+=21;
	w.pc+=3;
	Nsrc(w.A, 16);
	return;
}

VOID o815(LPBYTE I) // BSRC
{
	w.cycles+=21;
	w.pc+=3;
	Nsrc(w.B, 16);
	return;
}

VOID o816(LPBYTE I) // CSRC
{
	w.cycles+=21;
	w.pc+=3;
	Nsrc(w.C, 16);
	return;
}

VOID o817(LPBYTE I) // DSRC
{
	w.cycles+=21;
	w.pc+=3;
	Nsrc(w.D, 16);
	return;
}

VOID o818f0x(LPBYTE I) // A=A+x+1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.A[F_s[I[3]]]+=I[5];					// add constant value-1
	Ninc16(w.A,nF_l,F_s[I[3]]);				// add one and adjust in HEX mode
	return;
}

VOID o818f1x(LPBYTE I) // B=B+x+1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.B[F_s[I[3]]]+=I[5];					// add constant value-1
	Ninc16(w.B,nF_l,F_s[I[3]]);				// add one and adjust in HEX mode
	return;
}

VOID o818f2x(LPBYTE I) // C=C+x+1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.C[F_s[I[3]]]+=I[5];					// add constant value-1
	Ninc16(w.C,nF_l,F_s[I[3]]);				// add one and adjust in HEX mode
	return;
}

VOID o818f3x(LPBYTE I) // D=D+x+1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.D[F_s[I[3]]]+=I[5];					// add constant value-1
	Ninc16(w.D,nF_l,F_s[I[3]]);				// add one and adjust in HEX mode
	return;
}

VOID o818f8x(LPBYTE I) // A=A-x-1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.A[F_s[I[3]]]-=I[5];					// sub constant value+1
	Ndec16(w.A,nF_l,F_s[I[3]]);				// dec one and adjust in HEX mode
	return;
}

VOID o818f9x(LPBYTE I) // B=B-x-1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.B[F_s[I[3]]]-=I[5];					// sub constant value+1
	Ndec16(w.B,nF_l,F_s[I[3]]);				// dec one and adjust in HEX mode
	return;
}

VOID o818fAx(LPBYTE I) // C=C-x-1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.C[F_s[I[3]]]-=I[5];					// sub constant value+1
	Ndec16(w.C,nF_l,F_s[I[3]]);				// dec one and adjust in HEX mode
	return;
}

VOID o818fBx(LPBYTE I) // D=D-x-1 f
{
	// register length with saturn bug emulation
	UINT nF_l = (F_l[I[3]] == 1) ? 0x11 : F_l[I[3]];

	w.cycles+=5+F_l[I[3]];
	w.pc+=6;
	w.D[F_s[I[3]]]-=I[5];					// sub constant value+1
	Ndec16(w.D,nF_l,F_s[I[3]]);				// dec one and adjust in HEX mode
	return;
}

VOID o819f0(LPBYTE I) // ASRB.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=5;
	NFsrb(w.A, I[3]);
	return;
}

VOID o819f1(LPBYTE I) // BSRB.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=5;
	NFsrb(w.B, I[3]);
	return;
}

VOID o819f2(LPBYTE I) // CSRB.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=5;
	NFsrb(w.C, I[3]);
	return;
}

VOID o819f3(LPBYTE I) // DSRB.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=5;
	NFsrb(w.D, I[3]);
	return;
}

VOID o81Af00(LPBYTE I) // R0=A.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R0, w.A, I[3]);
	return;
}

VOID o81Af01(LPBYTE I) // R1=A.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R1, w.A, I[3]);
	return;
}

VOID o81Af02(LPBYTE I) // R2=A.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R2, w.A, I[3]);
	return;
}

VOID o81Af03(LPBYTE I) // R3=A.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R3, w.A, I[3]);
	return;
}

VOID o81Af04(LPBYTE I) // R4=A.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R4, w.A, I[3]);
	return;
}

VOID o81Af08(LPBYTE I) // R0=C.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R0, w.C, I[3]);
	return;
}

VOID o81Af09(LPBYTE I) // R1=C.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R1, w.C, I[3]);
	return;
}

VOID o81Af0A(LPBYTE I) // R2=C.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R2, w.C, I[3]);
	return;
}

VOID o81Af0B(LPBYTE I) // R3=C.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R3, w.C, I[3]);
	return;
}

VOID o81Af0C(LPBYTE I) // R4=C.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.R4, w.C, I[3]);
	return;
}

VOID o81Af10(LPBYTE I) // A=R0.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.A, w.R0, I[3]);
	return;
}

VOID o81Af11(LPBYTE I) // A=R1.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.A, w.R1, I[3]);
	return;
}

VOID o81Af12(LPBYTE I) // A=R2.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.A, w.R2, I[3]);
	return;
}

VOID o81Af13(LPBYTE I) // A=R3.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.A, w.R3, I[3]);
	return;
}

VOID o81Af14(LPBYTE I) // A=R4.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.A, w.R4, I[3]);
	return;
}

VOID o81Af18(LPBYTE I) // C=R0.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.C, w.R0, I[3]);
	return;
}

VOID o81Af19(LPBYTE I) // C=R1.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.C, w.R1, I[3]);
	return;
}

VOID o81Af1A(LPBYTE I) // C=R2.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.C, w.R2, I[3]);
	return;
}

VOID o81Af1B(LPBYTE I) // C=R3.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.C, w.R3, I[3]);
	return;
}

VOID o81Af1C(LPBYTE I) // C=R4.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFcopy(w.C, w.R4, I[3]);
	return;
}

VOID o81Af20(LPBYTE I) // AR0EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.A, w.R0, I[3]);
	return;
}

VOID o81Af21(LPBYTE I) // AR1EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.A, w.R1, I[3]);
	return;
}

VOID o81Af22(LPBYTE I) // AR2EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.A, w.R2, I[3]);
	return;
}

VOID o81Af23(LPBYTE I) // AR3EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.A, w.R3, I[3]);
	return;
}

VOID o81Af24(LPBYTE I) // AR4EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.A, w.R4, I[3]);
	return;
}

VOID o81Af28(LPBYTE I) // CR0EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.C, w.R0, I[3]);
	return;
}

VOID o81Af29(LPBYTE I) // CR1EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.C, w.R1, I[3]);
	return;
}

VOID o81Af2A(LPBYTE I) // CR2EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.C, w.R2, I[3]);
	return;
}

VOID o81Af2B(LPBYTE I) // CR3EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.C, w.R3, I[3]);
	return;
}

VOID o81Af2C(LPBYTE I) // CR4EX.F f
{
	w.cycles+=6+F_l[I[3]];
	w.pc+=6;
	NFxchg(w.C, w.R4, I[3]);
	return;
}

VOID o81B2(LPBYTE I) // PC=A
{
	w.cycles+=16;
	w.pc = Npack(w.A,5);
	return;
}

VOID o81B3(LPBYTE I) // PC=C
{
	w.cycles+=16;
	w.pc = Npack(w.C,5);
	return;
}

VOID o81B4(LPBYTE I) // A=PC
{
	w.cycles+=9;
	w.pc+=4;
	Nunpack(w.A,w.pc,5);
	return;
}

VOID o81B5(LPBYTE I) // C=PC
{
	w.cycles+=9;
	w.pc+=4;
	Nunpack(w.C,w.pc,5);
	return;
}

VOID o81B6(LPBYTE I) // APCEX
{
	DWORD d=w.pc+4;
	w.cycles+=16;
	w.pc=Npack(w.A,5);
	Nunpack(w.A,d,5);
	return;
}

VOID o81B7(LPBYTE I) // CPCEX
{
	DWORD d=w.pc+4;
	w.cycles+=16;
	w.pc=Npack(w.C,5);
	Nunpack(w.C,d,5);
	return;
}

VOID o81C(LPBYTE I) // ASRB
{
	w.cycles+=20;
	w.pc+=3;
	Nsrb(w.A, 16);
	return;
}

VOID o81D(LPBYTE I) // BSRB
{
	w.cycles+=20;
	w.pc+=3;
	Nsrb(w.B, 16);
	return;
}

VOID o81E(LPBYTE I) // CSRB
{
	w.cycles+=20;
	w.pc+=3;
	Nsrb(w.C, 16);
	return;
}

VOID o81F(LPBYTE I) // DSRB
{
	w.cycles+=20;
	w.pc+=3;
	Nsrb(w.D, 16);
	return;
}

VOID o82n(LPBYTE I) // HST=0 m
{
	w.cycles+=3;
	w.pc+=3;
	w.HST&=~I[2];
	return;
}

VOID o83n(LPBYTE I) // ?HST=0 m
{
	w.cycles+=6;
	w.pc+=3;
	w.carry=((w.HST&I[2])==0);
	GOYES3;
}

VOID o84n(LPBYTE I) // ST=0 n
{
	w.cycles+=4;
	w.pc+=3;
	Nbit0(w.ST, I[2]);
	return;
}

VOID o85n(LPBYTE I) // ST=1 n
{
	w.cycles+=4;
	w.pc+=3;
	Nbit1(w.ST, I[2]);
	return;
}

VOID o86n(LPBYTE I) // ?ST=0 n
{
	w.cycles+=7;
	w.pc+=3;
	Tbit0(w.ST, I[2]);
	GOYES3;
}

VOID o87n(LPBYTE I) // ?ST=1 n
{
	w.cycles+=7;
	w.pc+=3;
	Tbit1(w.ST, I[2]);
	GOYES3;
}

VOID o88n(LPBYTE I) // ?P# n
{
	w.cycles+=6;
	w.pc+=3;
	w.carry=(w.P!=I[2]);
	GOYES3;
}

VOID o89n(LPBYTE I) // ?P= n
{
	w.cycles+=6;
	w.pc+=3;
	w.carry=(w.P==I[2]);
	GOYES3;
}

VOID o8A0(LPBYTE I) // ?A=B A
{
	w.cycles+=11;
	w.pc+=3;
	Te(w.A, w.B, 5);
	GOYES3;
}

VOID o8A1(LPBYTE I) // ?B=C A
{
	w.cycles+=11;
	w.pc+=3;
	Te(w.B, w.C, 5);
	GOYES3;
}

VOID o8A2(LPBYTE I) // ?C=A A
{
	w.cycles+=11;
	w.pc+=3;
	Te(w.C, w.A, 5);
	GOYES3;
}

VOID o8A3(LPBYTE I) // ?D=C A
{
	w.cycles+=11;
	w.pc+=3;
	Te(w.D, w.C, 5);
	GOYES3;
}

VOID o8A4(LPBYTE I) // ?A#B A
{
	w.cycles+=11;
	w.pc+=3;
	Tne(w.A, w.B, 5);
	GOYES3;
}

VOID o8A5(LPBYTE I) // ?B#C A
{
	w.cycles+=11;
	w.pc+=3;
	Tne(w.B, w.C, 5);
	GOYES3;
}

VOID o8A6(LPBYTE I) // ?C#A A
{
	w.cycles+=11;
	w.pc+=3;
	Tne(w.C, w.A, 5);
	GOYES3;
}

VOID o8A7(LPBYTE I) // ?D#C A
{
	w.cycles+=11;
	w.pc+=3;
	Tne(w.D, w.C, 5);
	GOYES3;
}

VOID o8A8(LPBYTE I) // ?A=0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tz(w.A, 5);
	GOYES3;
}

VOID o8A9(LPBYTE I) // ?B=0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tz(w.B, 5);
	GOYES3;
}

VOID o8AA(LPBYTE I) // ?C=0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tz(w.C, 5);
	GOYES3;
}

VOID o8AB(LPBYTE I) // ?D=0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tz(w.D, 5);
	GOYES3;
}

VOID o8AC(LPBYTE I) // ?A#0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tnz(w.A, 5);
	GOYES3;
}

VOID o8AD(LPBYTE I) // ?B#0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tnz(w.B, 5);
	GOYES3;
}

VOID o8AE(LPBYTE I) // ?C#0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tnz(w.C, 5);
	GOYES3;
}

VOID o8AF(LPBYTE I) // ?D#0 A
{
	w.cycles+=11;
	w.pc+=3;
	Tnz(w.D, 5);
	GOYES3;
}

VOID o8B0(LPBYTE I) // ?A>B A
{
	w.cycles+=11;
	w.pc+=3;
	Ta(w.A, w.B, 5);
	GOYES3;
}

VOID o8B1(LPBYTE I) // ?B>C A
{
	w.cycles+=11;
	w.pc+=3;
	Ta(w.B, w.C, 5);
	GOYES3;
}

VOID o8B2(LPBYTE I) // ?C>A A
{
	w.cycles+=11;
	w.pc+=3;
	Ta(w.C, w.A, 5);
	GOYES3;
}

VOID o8B3(LPBYTE I) // ?D>C A
{
	w.cycles+=11;
	w.pc+=3;
	Ta(w.D, w.C, 5);
	GOYES3;
}

VOID o8B4(LPBYTE I) // ?A<B A
{
	w.cycles+=11;
	w.pc+=3;
	Tb(w.A, w.B, 5);
	GOYES3;
}

VOID o8B5(LPBYTE I) // ?B<C A
{
	w.cycles+=11;
	w.pc+=3;
	Tb(w.B, w.C, 5);
	GOYES3;
}

VOID o8B6(LPBYTE I) // ?C<A A
{
	w.cycles+=11;
	w.pc+=3;
	Tb(w.C, w.A, 5);
	GOYES3;
}

VOID o8B7(LPBYTE I) // ?D<C A
{
	w.cycles+=11;
	w.pc+=3;
	Tb(w.D, w.C, 5);
	GOYES3;
}

VOID o8B8(LPBYTE I) // ?A>=B A
{
	w.cycles+=11;
	w.pc+=3;
	Tae(w.A, w.B, 5);
	GOYES3;
}

VOID o8B9(LPBYTE I) // ?B>=C A
{
	w.cycles+=11;
	w.pc+=3;
	Tae(w.B, w.C, 5);
	GOYES3;
}

VOID o8BA(LPBYTE I) // ?C>=A A
{
	w.cycles+=11;
	w.pc+=3;
	Tae(w.C, w.A, 5);
	GOYES3;
}

VOID o8BB(LPBYTE I) // ?D>=C A
{
	w.cycles+=11;
	w.pc+=3;
	Tae(w.D, w.C, 5);
	GOYES3;
}

VOID o8BC(LPBYTE I) // ?A<=B A
{
	w.cycles+=11;
	w.pc+=3;
	Tbe(w.A, w.B, 5);
	GOYES3;
}

VOID o8BD(LPBYTE I) // ?B<=C A
{
	w.cycles+=11;
	w.pc+=3;
	Tbe(w.B, w.C, 5);
	GOYES3;
}

VOID o8BE(LPBYTE I) // ?C<=A A
{
	w.cycles+=11;
	w.pc+=3;
	Tbe(w.C, w.A, 5);
	GOYES3;
}

VOID o8BF(LPBYTE I) // ?D<=C A
{
	w.cycles+=11;
	w.pc+=3;
	Tbe(w.D, w.C, 5);
	GOYES3;
}

VOID o8Cd4(LPBYTE I) // GOLONG #dddd
{
	DWORD d=Npack(I+2, 4);
	if (d&0x8000) w.pc -= 0xfffe - d; else w.pc+= d + 2;
	w.cycles+=14;
	w.pc&=0xFFFFF;
	return;
}

VOID o8Dd5(LPBYTE I) // GOVLNG #ddddd
{
	w.cycles+=14;
	w.pc = Npack(I+2, 5);
	return;
}

VOID o8Ed4(LPBYTE I) // GOSUBL #dddd
{
	DWORD d=Npack(I+2,4);
	rstkpush(w.pc+6);
	if (d&0x8000) w.pc -= 0xfffa - d; else w.pc += d + 6;
	w.cycles+=14;
	w.pc&=0xFFFFF;
	return;
}

VOID o8Fd5(LPBYTE I) // GOSBVL #ddddd
{
	w.cycles+=15;
	rstkpush(w.pc+7);
	w.pc=Npack(I+2, 5);
	return;
}

      // ?r=s f
VOID o9a0(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFe(w.A, w.B, I[1]); GOYES3; }
VOID o9a1(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFe(w.B, w.C, I[1]); GOYES3; }
VOID o9a2(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFe(w.C, w.A, I[1]); GOYES3; }
VOID o9a3(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFe(w.D, w.C, I[1]); GOYES3; }

	  // ?r#s f
VOID o9a4(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFne(w.A, w.B, I[1]); GOYES3; }
VOID o9a5(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFne(w.B, w.C, I[1]); GOYES3; }
VOID o9a6(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFne(w.C, w.A, I[1]); GOYES3; }
VOID o9a7(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFne(w.D, w.C, I[1]); GOYES3; }

	  // ?r=0 f
VOID o9a8(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFz(w.A, I[1]); GOYES3; }
VOID o9a9(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFz(w.B, I[1]); GOYES3; }
VOID o9aA(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFz(w.C, I[1]); GOYES3; }
VOID o9aB(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFz(w.D, I[1]); GOYES3; }

	  // ?r#0 f
VOID o9aC(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFnz(w.A, I[1]); GOYES3; }
VOID o9aD(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFnz(w.B, I[1]); GOYES3; }
VOID o9aE(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFnz(w.C, I[1]); GOYES3; }
VOID o9aF(LPBYTE I) { w.cycles+=6+F_l[I[1]]; w.pc+=3; TFnz(w.D, I[1]); GOYES3; }

	  // ?s>r f
VOID o9b0(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFa(w.A, w.B, I[1]&7); GOYES3; }
VOID o9b1(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFa(w.B, w.C, I[1]&7); GOYES3; }
VOID o9b2(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFa(w.C, w.A, I[1]&7); GOYES3; }
VOID o9b3(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFa(w.D, w.C, I[1]&7); GOYES3; }

	  // ?r<s f
VOID o9b4(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFb(w.A, w.B, I[1]&7); GOYES3; }
VOID o9b5(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFb(w.B, w.C, I[1]&7); GOYES3; }
VOID o9b6(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFb(w.C, w.A, I[1]&7); GOYES3; }
VOID o9b7(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFb(w.D, w.C, I[1]&7); GOYES3; }

	  // ?r>=s f
VOID o9b8(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFae(w.A, w.B, I[1]&7); GOYES3; }
VOID o9b9(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFae(w.B, w.C, I[1]&7); GOYES3; }
VOID o9bA(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFae(w.C, w.A, I[1]&7); GOYES3; }
VOID o9bB(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFae(w.D, w.C, I[1]&7); GOYES3; }

	  // ?r<=s f
VOID o9bC(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFbe(w.A, w.B, I[1]&7); GOYES3; }
VOID o9bD(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFbe(w.B, w.C, I[1]&7); GOYES3; }
VOID o9bE(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFbe(w.C, w.A, I[1]&7); GOYES3; }
VOID o9bF(LPBYTE I) { w.cycles+=6+F_l[I[1]&7]; w.pc+=3; TFbe(w.D, w.C, I[1]&7); GOYES3; }

	  // r=r+s f
VOID oAa0(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.A, w.B, I[1]); return; }
VOID oAa1(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.B, w.C, I[1]); return; }
VOID oAa2(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.C, w.A, I[1]); return; }
VOID oAa3(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.D, w.C, I[1]); return; }

	  // r=r+r f
VOID oAa4(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.A, w.A, I[1]); return; }
VOID oAa5(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.B, w.B, I[1]); return; }
VOID oAa6(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.C, w.C, I[1]); return; }
VOID oAa7(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.D, w.D, I[1]); return; }

	  // s=s+r f
VOID oAa8(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.B, w.A, I[1]); return; }
VOID oAa9(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.C, w.B, I[1]); return; }
VOID oAaA(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.A, w.C, I[1]); return; }
VOID oAaB(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFadd(w.C, w.D, I[1]); return; }

	  // r=r-1 f
VOID oAaC(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFdec(w.A, I[1]); return; }
VOID oAaD(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFdec(w.B, I[1]); return; }
VOID oAaE(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFdec(w.C, I[1]); return; }
VOID oAaF(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFdec(w.D, I[1]); return; }

	  // r=0 f
VOID oAb0(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFzero(w.A, I[1]&7); return; }
VOID oAb1(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFzero(w.B, I[1]&7); return; }
VOID oAb2(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFzero(w.C, I[1]&7); return; }
VOID oAb3(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFzero(w.D, I[1]&7); return; }

	  // r=s f
VOID oAb4(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.A, w.B, I[1]&7); return; }
VOID oAb5(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.B, w.C, I[1]&7); return; }
VOID oAb6(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.C, w.A, I[1]&7); return; }
VOID oAb7(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.D, w.C, I[1]&7); return; }

	  // s=r f
VOID oAb8(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.B, w.A, I[1]&7); return; }
VOID oAb9(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.C, w.B, I[1]&7); return; }
VOID oAbA(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.A, w.C, I[1]&7); return; }
VOID oAbB(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFcopy(w.C, w.D, I[1]&7); return; }

	  // rsEX f
VOID oAbC(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFxchg(w.A, w.B, I[1]&7); return; }
VOID oAbD(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFxchg(w.B, w.C, I[1]&7); return; }
VOID oAbE(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFxchg(w.C, w.A, I[1]&7); return; }
VOID oAbF(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFxchg(w.D, w.C, I[1]&7); return; }

	  // r=r-s f
VOID oBa0(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.A, w.B, I[1]); return; }
VOID oBa1(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.B, w.C, I[1]); return; }
VOID oBa2(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.C, w.A, I[1]); return; }
VOID oBa3(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.D, w.C, I[1]); return; }

	  // r=r+1 f
VOID oBa4(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFinc(w.A, I[1]); return; }
VOID oBa5(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFinc(w.B, I[1]); return; }
VOID oBa6(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFinc(w.C, I[1]); return; }
VOID oBa7(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFinc(w.D, I[1]); return; }

	  // s=s-r f
VOID oBa8(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.B, w.A, I[1]); return; }
VOID oBa9(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.C, w.B, I[1]); return; }
VOID oBaA(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.A, w.C, I[1]); return; }
VOID oBaB(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFsub(w.C, w.D, I[1]); return; }

	  // r=s-r f
VOID oBaC(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFrsub(w.A, w.B, I[1]); return; }
VOID oBaD(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFrsub(w.B, w.C, I[1]); return; }
VOID oBaE(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFrsub(w.C, w.A, I[1]); return; }
VOID oBaF(LPBYTE I) { w.cycles+=3+F_l[I[1]]; w.pc+=3; NFrsub(w.D, w.C, I[1]); return; }

	  // rSL f
VOID oBb0(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsl(w.A, I[1]&7); return; }
VOID oBb1(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsl(w.B, I[1]&7); return; }
VOID oBb2(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsl(w.C, I[1]&7); return; }
VOID oBb3(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsl(w.D, I[1]&7); return; }

	  // rSR f
VOID oBb4(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsr(w.A, I[1]&7); return; }
VOID oBb5(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsr(w.B, I[1]&7); return; }
VOID oBb6(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsr(w.C, I[1]&7); return; }
VOID oBb7(LPBYTE I) { w.cycles+=4+F_l[I[1]&7]; w.pc+=3; NFsr(w.D, I[1]&7); return; }

	  // r=-r f
VOID oBb8(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFneg(w.A, I[1]&7); return; }
VOID oBb9(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFneg(w.B, I[1]&7); return; }
VOID oBbA(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFneg(w.C, I[1]&7); return; }
VOID oBbB(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFneg(w.D, I[1]&7); return; }

	  // r=-r-1 f
VOID oBbC(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFnot(w.A, I[1]&7); return; }
VOID oBbD(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFnot(w.B, I[1]&7); return; }
VOID oBbE(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFnot(w.C, I[1]&7); return; }
VOID oBbF(LPBYTE I) { w.cycles+=3+F_l[I[1]&7]; w.pc+=3; NFnot(w.D, I[1]&7); return; }

	 // r=r+s A
VOID oC0(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.A, w.B, 5); return; }
VOID oC1(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.B, w.C, 5); return; }
VOID oC2(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.C, w.A, 5); return; }
VOID oC3(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.D, w.C, 5); return; }

	 // r=r+r A
VOID oC4(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.A, w.A, 5); return; }
VOID oC5(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.B, w.B, 5); return; }
VOID oC6(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.C, w.C, 5); return; }
VOID oC7(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.D, w.D, 5); return; }

	 // s=s+r A
VOID oC8(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.B, w.A, 5); return; }
VOID oC9(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.C, w.B, 5); return; }
VOID oCA(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.A, w.C, 5); return; }
VOID oCB(LPBYTE I) { w.cycles+=7; w.pc+=2; Nadd(w.C, w.D, 5); return; }

	 // r=r-1 A
VOID oCC(LPBYTE I) { w.cycles+=7; w.pc+=2; Ndec(w.A, 5, 0); return; }
VOID oCD(LPBYTE I) { w.cycles+=7; w.pc+=2; Ndec(w.B, 5, 0); return; }
VOID oCE(LPBYTE I) { w.cycles+=7; w.pc+=2; Ndec(w.C, 5, 0); return; }
VOID oCF(LPBYTE I) { w.cycles+=7; w.pc+=2; Ndec(w.D, 5, 0); return; }

	 // r=0 A
VOID oD0(LPBYTE I) { w.cycles+=7; w.pc+=2; memset(w.A, 0, 5); return; }
VOID oD1(LPBYTE I) { w.cycles+=7; w.pc+=2; memset(w.B, 0, 5); return; }
VOID oD2(LPBYTE I) { w.cycles+=7; w.pc+=2; memset(w.C, 0, 5); return; }
VOID oD3(LPBYTE I) { w.cycles+=7; w.pc+=2; memset(w.D, 0, 5); return; }

	 // r=s A
VOID oD4(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.A, w.B, 5); return; }
VOID oD5(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.B, w.C, 5); return; }
VOID oD6(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.C, w.A, 5); return; }
VOID oD7(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.D, w.C, 5); return; }

	 // s=r A
VOID oD8(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.B, w.A, 5); return; }
VOID oD9(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.C, w.B, 5); return; }
VOID oDA(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.A, w.C, 5); return; }
VOID oDB(LPBYTE I) { w.cycles+=7; w.pc+=2; memcpy(w.C, w.D, 5); return; }

	 // rsEX
VOID oDC(LPBYTE I) { w.cycles+=7; w.pc+=2; Nxchg(w.A, w.B, 5); return; }
VOID oDD(LPBYTE I) { w.cycles+=7; w.pc+=2; Nxchg(w.B, w.C, 5); return; }
VOID oDE(LPBYTE I) { w.cycles+=7; w.pc+=2; Nxchg(w.C, w.A, 5); return; }
VOID oDF(LPBYTE I) { w.cycles+=7; w.pc+=2; Nxchg(w.D, w.C, 5); return; }

	 // r=r-s A
VOID oE0(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.A, w.B, 5); return; }
VOID oE1(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.B, w.C, 5); return; }
VOID oE2(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.C, w.A, 5); return; }
VOID oE3(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.D, w.C, 5); return; }

	 // r=r+1 A
VOID oE4(LPBYTE I) { w.cycles+=7; w.pc+=2; Ninc(w.A, 5, 0); return; }
VOID oE5(LPBYTE I) { w.cycles+=7; w.pc+=2; Ninc(w.B, 5, 0); return; }
VOID oE6(LPBYTE I) { w.cycles+=7; w.pc+=2; Ninc(w.C, 5, 0); return; }
VOID oE7(LPBYTE I) { w.cycles+=7; w.pc+=2; Ninc(w.D, 5, 0); return; }

	 // s=s-r A
VOID oE8(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.B, w.A, 5); return; }
VOID oE9(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.C, w.B, 5); return; }
VOID oEA(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.A, w.C, 5); return; }
VOID oEB(LPBYTE I) { w.cycles+=7; w.pc+=2; Nsub(w.C, w.D, 5); return; }

	 // r=s-r A
VOID oEC(LPBYTE I) { w.cycles+=7; w.pc+=2; Nrsub(w.A, w.B, 5); return; }
VOID oED(LPBYTE I) { w.cycles+=7; w.pc+=2; Nrsub(w.B, w.C, 5); return; }
VOID oEE(LPBYTE I) { w.cycles+=7; w.pc+=2; Nrsub(w.C, w.A, 5); return; }
VOID oEF(LPBYTE I) { w.cycles+=7; w.pc+=2; Nrsub(w.D, w.C, 5); return; }

	 // rSL A
VOID oF0(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsl(w.A, 5); return; }
VOID oF1(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsl(w.B, 5); return; }
VOID oF2(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsl(w.C, 5); return; }
VOID oF3(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsl(w.D, 5); return; }

	 // rSR A
VOID oF4(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsr(w.A, 5); return; }
VOID oF5(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsr(w.B, 5); return; }
VOID oF6(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsr(w.C, 5); return; }
VOID oF7(LPBYTE I) { w.cycles+=8; w.pc+=2; Nsr(w.D, 5); return; }

	 // r=-r A
VOID oF8(LPBYTE I) { w.cycles+=7; w.pc+=2; Nneg(w.A, 5); return; }
VOID oF9(LPBYTE I) { w.cycles+=7; w.pc+=2; Nneg(w.B, 5); return; }
VOID oFA(LPBYTE I) { w.cycles+=7; w.pc+=2; Nneg(w.C, 5); return; }
VOID oFB(LPBYTE I) { w.cycles+=7; w.pc+=2; Nneg(w.D, 5); return; }

	 // r=-r-1 A
VOID oFC(LPBYTE I) { w.cycles+=7; w.pc+=2; Nnot(w.A, 5); return; }
VOID oFD(LPBYTE I) { w.cycles+=7; w.pc+=2; Nnot(w.B, 5); return; }
VOID oFE(LPBYTE I) { w.cycles+=7; w.pc+=2; Nnot(w.C, 5); return; }
VOID oFF(LPBYTE I) { w.cycles+=7; w.pc+=2; Nnot(w.D, 5); return; }

// length is guessed, just skip
VOID o_invalid3(LPBYTE I)
{
	_ASSERT(FALSE);								// invalid, length guessed, skip 3 nibbles
	w.pc+=3;
	return;
}

VOID o_invalid4(LPBYTE I)
{
	_ASSERT(FALSE);								// invalid, length guessed, skip 4 nibbles
	w.pc+=4;
	return;
}

VOID o_invalid5(LPBYTE I)
{
	_ASSERT(FALSE);								// invalid, length guessed, skip 5 nibbles
	w.pc+=5;
	return;
}

VOID o_invalid6(LPBYTE I)
{
	_ASSERT(FALSE);								// invalid, length guessed, skip 6 nibbles
	w.pc+=6;
	return;
}

VOID o_goyes3(LPBYTE I)
{
	signed char jmp = I[3]+(I[4]<<4);
	w.cycles+=7;
	if (jmp)
		w.pc=(w.pc+jmp)&0xFFFFF;
	else
		w.pc=rstkpop();
	return;
}

VOID o_goyes5(LPBYTE I)
{
	signed char jmp = I[5]+(I[6]<<4);
	w.cycles+=7;
	if (jmp)
		w.pc=(w.pc+jmp)&0xFFFFF;
	else
		w.pc=rstkpop();
	return;
}

//////// EXTENSIONS ////////
VOID o81B1(LPBYTE I)
{
	if (cCurrentRomType=='Q' || cCurrentRomType=='2' || cCurrentRomType=='P')
	{
		// The ARM-based calculators use this previously-unused opcode to return to RPL
		w.cycles+=48;
		o80B00();							// LOOP2
	}
	else
	{
		// Emu48 borrows this opcode for the beep patch on the non-ARM-based calculators
		External(&w);						// beep patch
		PCHANGED;							// update field select table
	}
	return;
}
////////////////////////////
