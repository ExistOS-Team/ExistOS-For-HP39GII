/*
 *   ops.h
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1995 Sebastien Carlier
 *
 */

#define NFunpack(a, b, f) Nunpack((a)+F_s[f], b, F_l[f])
#define NFread(a, b, f)   Nread((a)+F_s[f], b, F_l[f])
#define NFwrite(a, b, f)  Nwrite((a)+F_s[f], b, F_l[f])
#define NFcopy(a, b, f)   memcpy((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFxchg(a, b, f)   Nxchg((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFadd(a, b, f)    Nadd((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFsub(a, b, f)    Nsub((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFrsub(a, b, f)   Nrsub((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFand(a, b, f)    Nand((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFor(a, b, f)     Nor((a)+F_s[f], (b)+F_s[f], F_l[f])
#define NFzero(a,f)       memset((a)+F_s[f], 0, F_l[f])
#define NFpack(a, f)      Npack((a)+F_s[f], F_l[f])
#define NFinc(a, f)       Ninc(a, F_l[f], F_s[f])
#define NFdec(a, f)       Ndec(a, F_l[f], F_s[f])
#define NFnot(a, f)       Nnot((a)+F_s[f], F_l[f])
#define NFneg(a, f)       Nneg((a)+F_s[f], F_l[f])
#define NFsl(a, f)        Nsl((a)+F_s[f], F_l[f])
#define NFsr(a, f)        Nsr((a)+F_s[f], F_l[f])
#define NFsrb(a, f)       Nsrb((a)+F_s[f], F_l[f])
#define TFe(a, b, f)      Te((a)+F_s[f], (b)+F_s[f], F_l[f])
#define TFa(a, b, f)      Ta((a)+F_s[f], (b)+F_s[f], F_l[f])
#define TFb(a, b, f)      Tb((a)+F_s[f], (b)+F_s[f], F_l[f])
#define TFz(a, f)         Tz((a)+F_s[f], F_l[f])
#define TFne(a, b, f)     Tne((a)+F_s[f], (b)+F_s[f], F_l[f])
#define TFae(a, b, f)     Tae((a)+F_s[f], (b)+F_s[f], F_l[f])
#define TFbe(a, b, f)     Tbe((a)+F_s[f], (b)+F_s[f], F_l[f])
#define TFnz(a, f)        Tnz((a)+F_s[f], F_l[f])

LPBYTE FASTPTR(DWORD d);

/*
static __inline LPBYTE FASTPTR(DWORD d)
{
	static BYTE pbyNULL[16];
	LPBYTE lpbyPage;
	
	d &= 0xFFFFF;							// handle address overflows

	if ((Chipset.IOCfig)&&((d&0xFFFC0)==Chipset.IOBase))
		return Chipset.IORam+d-Chipset.IOBase;

	if ((lpbyPage = RMap[d>>12]) != NULL)	// page valid
	{
		lpbyPage += d & 0xFFF;				// full address
	}
	else
	{
		lpbyPage = pbyNULL;					// memory allocation
		Npeek(lpbyPage, d, 13);				// fill with data (LA(8) = longest opcode)
	}
	return lpbyPage;
}
*/

static __inline void rstkpush(DWORD d)
{
	Chipset.rstk[Chipset.rstkp] = d;
	Chipset.rstkp=(Chipset.rstkp+1)&7;
}

static __inline DWORD rstkpop(VOID)
{
	DWORD r;

	Chipset.rstkp=(Chipset.rstkp-1)&7;
	r = Chipset.rstk[Chipset.rstkp];
	Chipset.rstk[Chipset.rstkp] = 0;
	return r;
}

static __inline DWORD Npack(BYTE *a, UINT s)
{
	DWORD r = 0;
	while (s--) {
		r = (r<<4) | a[s];
	}
	return r;
}

static __inline VOID Nunpack(BYTE *a, DWORD b, UINT s)
{
	UINT i;
	for (i=0; i<s; i++) { a[i] = (BYTE)(b&0xf); b>>=4; }
}

static __inline QWORD Npack64(BYTE *a, UINT s)
{
	QWORD r = 0;
    
	while (s--) r = (r<<4)|a[s];
	return r;
}

static __inline VOID Nunpack64(BYTE *a, QWORD b, UINT s)
{
	UINT i;
	for (i=0; i<s; i++) { a[i] = (BYTE)(b&0xf); b>>=4; }
}

static __inline void Nxchg(BYTE *a, BYTE *b, UINT s)
{
	BYTE X[16];

	memcpy(X, b, s);
	memcpy(b, a, s);
	memcpy(a, X, s);
}

static __inline void Ninc(BYTE *a, UINT s, UINT d)
{
	UINT i;

	if (Chipset.mode_dec)
	{
		BYTE c = 1;
		for (i=d; i<s+d; ++i)
		{
			// no register wrap
			_ASSERT(i < ARRAYSIZEOF(((CHIPSET *) NULL)->A));

			// illegal number in dec mode
			if (a[i] >= 10) a[i] &= 0x7;

			a[i] += c;
			c = (a[i] >= 10);
			if (c) a[i] -= 10;
		}
		Chipset.carry = (c==1);
	}
	else
	{
		for (i=d; i<s+d; ++i)
		{
			// no register wrap
			_ASSERT(i < ARRAYSIZEOF(((CHIPSET *) NULL)->A));

			a[i]++;
			if (a[i] < 16)
			{
				Chipset.carry = FALSE;
				return;
			}
			a[i] -= 16;
		}
		Chipset.carry = TRUE;
	}
}

static __inline void Ninc16(BYTE *a, UINT s, UINT d)
{
	UINT i;

	for (i=d; i<s+d; ++i)
	{
		a[i&0xf]++;
		if (a[i&0xf] < 16)
		{
			Chipset.carry = FALSE;
			return;
		}
		a[i&0xf] -= 16;
	}
	Chipset.carry = TRUE;
}

static __inline void Nincx(BYTE *a, UINT s)
{
	UINT i;

	for (i=0; i<s; ++i)
	{
		a[i]++;
		if (a[i] < 16)
		{
			Chipset.carry = FALSE;
			return;
		}
		a[i] -= 16;
	}
	Chipset.carry = TRUE;
}

static __inline void Ndec(BYTE *a, UINT s, UINT d)
{
	UINT i;
	BYTE cBase = Chipset.mode_dec ? 10 : 16;

	for (i=d; i<s+d; ++i)
	{
		// no register wrap
		_ASSERT(i < ARRAYSIZEOF(((CHIPSET *) NULL)->A));

		a[i]--;
		if ((a[i] & 0xF0) == 0)				// check overflow
		{
			Chipset.carry = FALSE;
			return;
		}
		a[i] += cBase;
	}
	Chipset.carry = TRUE;
}

static __inline void Ndec16(BYTE *a, UINT s, UINT d)
{
	UINT i;

	for (i=d; i<s+d; ++i)
	{
		a[i&0xf]--;
		if (a[i&0xf] < 16)
		{
			Chipset.carry = FALSE;
			return;
		}
		a[i&0xf] += 16;
	}
	Chipset.carry = TRUE;
}

static __inline void Nadd(BYTE *a, BYTE *b, UINT s)
{
	UINT i;
	BYTE c = 0;
	BYTE cBase = Chipset.mode_dec ? 10 : 16;

	for (i=0; i<s; ++i)
	{
		// illegal number in dec mode
		if (a[i] >= cBase) a[i] &= 0x7;

		a[i] += b[i] + c;
		if (a[i] >= cBase)
		{
			a[i] -= cBase;
			c = 1;
		}
		else
			c = 0;
	}
	Chipset.carry = (c==1);
}

static __inline void Nsub(BYTE *a, BYTE *b, UINT s)
{
	UINT i;
	BYTE c = 0;
	BYTE cBase = Chipset.mode_dec ? 10 : 16;

	for (i=0; i<s; ++i)
	{
		a[i] = a[i] - b[i] - c;
		if ((a[i] & 0xF0) != 0)				// check overflow
		{
			a[i] += cBase;
			// illegal number in dec mode
			if ((a[i] & 0xF0) != 0) a[i] &= 0x7;
			c = 1;
		}
		else
			c = 0;
	}
	Chipset.carry = (c==1);
}

static __inline void Nrsub(BYTE *a, BYTE *b, UINT s)
{
	UINT i;
	BYTE c = 0;
	BYTE cBase = Chipset.mode_dec ? 10 : 16;

	for (i=0; i<s; ++i)
	{
		a[i] = b[i] - a[i] - c;
		if ((a[i] & 0xF0) != 0)				// check overflow
		{
			a[i] += cBase;
			// illegal number in dec mode
			if ((a[i] & 0xF0) != 0) a[i] &= 0x7;
			c = 1;
		}
		else
			c = 0;
	}
	Chipset.carry = (c==1);
}

static __inline void Nand(BYTE *a, BYTE *b, UINT s)
{
	while (s--) a[s] &= b[s];
}

static __inline void Nor(BYTE *a, BYTE *b, UINT s)
{
	while (s--) a[s] |= b[s];
}

static __inline void Nnot(BYTE *a, UINT s)
{
	BYTE cBase = Chipset.mode_dec ? 9 : 15;

	while (s--)
	{
		a[s] = cBase - a[s];
		if ((a[s] & 0xF0) != 0)				// check overflow (dec mode only)
			a[s] &= 0x7;
	}
	Chipset.carry = FALSE;
}

static __inline void Nneg(BYTE *a, UINT s)
{
	UINT i;

	for (i=0; i<s && a[i]==0; ++i) { }		// search for non-zero digit
	if ((Chipset.carry = (i!=s)))			// value was non-zero
	{
		BYTE cBase = Chipset.mode_dec ? 9 : 15;

		_ASSERT(a[i] > 0);					// check for non-zero digit
		for (--a[i]; i<s; ++i)
		{
			a[i] = cBase - a[i];
			if ((a[i] & 0xF0) != 0)			// check overflow (dec mode only)
				a[i] &= 0x7;
		}
	}
}

static __inline void Nsl(BYTE *a, UINT s)
{
	while (--s) a[s] = a[s-1];
	*a = 0;
}

static __inline void Nslc(BYTE *a, UINT s)
{
	BYTE c = a[s-1];

	while (--s) a[s] = a[s-1];
	*a = c;
}

static __inline void Nsr(BYTE *a, UINT s)
{
	if (*a) Chipset.HST |= SB;
	while (--s) { *a = a[1]; a++; }
	*a = 0;
}

static __inline void Nsrc(BYTE *a, UINT s)
{
	BYTE c = *a;

	if (c) Chipset.HST |= SB;
	while (--s) { *a = a[1]; a++; }
	*a = c;
}

static __inline void Nsrb(BYTE *a, UINT s)
{
	if (*a & 1) Chipset.HST |= SB;
	while (--s)
	{
		*a >>= 1;
		*a |= ((a[1] & 1) << 3);
		a++;
	}
	*a >>= 1;
}

static __inline void Nbit0(BYTE *a, UINT b)
{
	a[b>>2] &= ~(1<<(b&3));
}

static __inline void Nbit1(BYTE *a, UINT b)
{
	a[b>>2] |= 1<<(b&3);
}

static __inline void Tbit0(BYTE *a, UINT b)
{
	Chipset.carry = ((a[b>>2] & (1<<(b&3))) == 0);
}

static __inline void Tbit1(BYTE *a, UINT b)
{
	Chipset.carry = ((a[b>>2] & (1<<(b&3))) != 0);
}

static __inline void Te(BYTE *a, BYTE *b, UINT s)
{
	while (s--)
	{
		if (a[s]!=b[s])
		{
			Chipset.carry = FALSE;
			return;
		}
	}
	Chipset.carry = TRUE;
}

static __inline void Tne(BYTE *a, BYTE *b, UINT s)
{
	while (s--)
	{
		if (a[s]!=b[s])
		{
			Chipset.carry = TRUE;
			return;
		}
	}
	Chipset.carry = FALSE;
}

static __inline void Tz(BYTE *a, UINT s)
{
	while (s--)
	{
		if (a[s]!=0)
		{
			Chipset.carry = FALSE;
			return;
		}
	}
	Chipset.carry = TRUE;
}

static __inline void Tnz(BYTE *a, UINT s)
{
	while (s--)
	{
		if (a[s]!=0)
		{
			Chipset.carry = TRUE;
			return;
		}
	}
	Chipset.carry = FALSE;
}

static __inline void Ta(BYTE *a, BYTE *b, UINT s)
{
	while (--s) if (a[s]!=b[s]) break;
	Chipset.carry = (a[s]>b[s]);
}

static __inline void Tb(BYTE *a, BYTE *b, UINT s)
{
	while (--s) if (a[s]!=b[s]) break;
	Chipset.carry = (a[s]<b[s]);
}

static __inline void Tae(BYTE *a, BYTE *b, UINT s)
{
	while (--s) if (a[s]!=b[s]) break;
	Chipset.carry = (a[s]>=b[s]);
}

static __inline void Tbe(BYTE *a, BYTE *b, UINT s)
{
	while (--s) if (a[s]!=b[s]) break;
	Chipset.carry = (a[s]<=b[s]);
}
