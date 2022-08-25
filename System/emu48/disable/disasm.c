/*
 *   Disasm.c
 *
 *   This file is part of Emu48, a ported version of x48
 *
 *   Copyright (C) 1994 Eddie C. Dost
 *   Copyright (C) 1998 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "emu48.h"

#define TAB_SKIP	8

BOOL disassembler_mode = HP_MNEMONICS;
WORD disassembler_map  = MEM_MAP;

static LPCTSTR hex[] =
{
  _T("0123456789ABCDEF"),
  _T("0123456789abcdef")
};

static LPCTSTR opcode_0_tbl[32] =
{
	/*
	 * HP Mnemonics
	 */
	_T("RTNSXM"), _T("RTN"), _T("RTNSC"), _T("RTNCC"),
	_T("SETHEX"), _T("SETDEC"), _T("RSTK=C"), _T("C=RSTK"),
	_T("CLRST"), _T("C=ST"), _T("ST=C"), _T("CSTEX"),
	_T("P=P+1"), _T("P=P-1"), _T("(NULL)"), _T("RTI"),
	/*
	 * Class Mnemonics
	 */
	_T("rtnsxm"), _T("rtn"), _T("rtnsc"), _T("rtncc"),
	_T("sethex"), _T("setdec"), _T("push"), _T("pop"),
	_T("clr.3   st"), _T("move.3  st, c"), _T("move.3  c, st"), _T("exg.3   c, st"),
	_T("inc.1   p"), _T("dec.1   p"), _T("(null)"), _T("rti")
};

static LPCTSTR op_str_0[16] =
{
	/*
	 * HP Mnemonics
	 */
	_T("A=A%cB"), _T("B=B%cC"), _T("C=C%cA"), _T("D=D%cC"),
	_T("B=B%cA"), _T("C=C%cB"), _T("A=A%cC"), _T("C=C%cD"),
	/*
	 * Class Mnemonics
	 */
	_T("b, a"), _T("c, b"), _T("a, c"), _T("c, d"),
	_T("a, b"), _T("b, c"), _T("c, a"), _T("d, c")
};

static LPCTSTR op_str_1[16] =
{
	/*
	 * HP Mnemonics
	 */
	_T("DAT0=A"), _T("DAT1=A"), _T("A=DAT0"), _T("A=DAT1"),
	_T("DAT0=C"), _T("DAT1=C"), _T("C=DAT0"), _T("C=DAT1"),
	/*
	 * Class Mnemonics
	 */
	_T("a, (d0)"), _T("a, (d1)"), _T("(d0), a"), _T("(d1), a"),
	_T("c, (d0)"), _T("c, (d1)"), _T("(d0), c"), _T("(d1), c")
};

static LPCTSTR in_str_80[32] =
{
	/*
	 * HP Mnemonics
	 */
	_T("OUT=CS"), _T("OUT=C"), _T("A=IN"), _T("C=IN"),
	_T("UNCNFG"), _T("CONFIG"), _T("C=ID"), _T("SHUTDN"),
	NULL, _T("C+P+1"), _T("RESET"), _T("BUSCC"),
	NULL, NULL, _T("SREQ?"), NULL,
	/*
	 * Class Mnemonics
	 */
	_T("move.s  c, out"), _T("move.3  c, out"), _T("move.4  in, a"), _T("move.4  in, c"),
	_T("uncnfg"), _T("config"), _T("c=id"), _T("shutdn"),
	NULL, _T("add.a   p+1, c"), _T("reset"), _T("buscc"),
	NULL, NULL, _T("sreq?"), NULL
};

static LPCTSTR in_str_808[32] =
{
	/*
	 * HP Mnemonics
	 */
	_T("INTON"), NULL, NULL, _T("BUSCB"),
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	_T("PC=(A)"), _T("BUSCD"), _T("PC=(C)"), _T("INTOFF"),
	/*
	 * Class Mnemonics
	 */
	_T("inton"), NULL, NULL, _T("buscb"),
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	_T("jmp     (a)"), _T("buscd"), _T("jmp     (c)"), _T("intoff")
};

static LPCTSTR op_str_81[8] =
{
	/*
	 * HP Mnemonics
	 */
	_T("A"), _T("B"), _T("C"), _T("D"),
	/*
	 * Class Mnemonics
	 */
	_T("a"), _T("b"), _T("c"), _T("d")
};

static LPCTSTR in_str_81b[32] =
{
	/*
	 * HP Mnemonics
	 */
	NULL, NULL, _T("PC=A"), _T("PC=C"),
	_T("A=PC"), _T("C=PC"), _T("APCEX"), _T("CPCEX"),
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL,
	/*
	 * Class Mnemonics
	 */
	NULL, NULL, _T("jmp     a"), _T("jmp     c"),
	_T("move.a  pc, a"), _T("move.a  pc, c"), _T("exg.a   a, pc"), _T("exg.a   c, pc"),
	NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL
};

static LPCTSTR in_str_9[16] =
{
	/*
	 * HP Mnemonics
	 */
	_T("="), _T("#"), _T("="), _T("#"),
	_T(">"), _T("<"), _T(">="), _T("<="),
	/*
	 * Class Mnemonics
	 */
	_T("eq"), _T("ne"), _T("eq"), _T("ne"),
	_T("gt"), _T("lt"), _T("ge"), _T("le")
};

static LPCTSTR op_str_9[16] =
{
	/*
	 * HP Mnemonics
	 */
	_T("?A%sB"), _T("?B%sC"), _T("?C%sA"), _T("?D%sC"),
	_T("?A%s0"), _T("?B%s0"), _T("?C%s0"), _T("?D%s0"),
	/*
	 * Class Mnemonics
	 */
	_T("a, b"), _T("b, c"), _T("c, a"), _T("d, c"),
	_T("a, 0"), _T("b, 0"), _T("c, 0"), _T("d, 0")
};

static LPCTSTR op_str_af[32] =
{
	/*
	 * HP Mnemonics
	 */
	_T("A=A%sB"), _T("B=B%sC"), _T("C=C%sA"), _T("D=D%sC"),
	_T("A=A%sA"), _T("B=B%sB"), _T("C=C%sC"), _T("D=D%sD"),
	_T("B=B%sA"), _T("C=C%sB"), _T("A=A%sC"), _T("C=C%sD"),
	_T("A=B%sA"), _T("B=C%sB"), _T("C=A%sC"), _T("D=C%sD"),
	/*
	 * Class Mnemonics
	 */
	_T("b, a"), _T("c, b"), _T("a, c"), _T("c, d"),
	_T("a, a"), _T("b, b"), _T("c, c"), _T("d, d"),
	_T("a, b"), _T("b, c"), _T("c, a"), _T("d, c"),
	_T("b, a"), _T("c, b"), _T("a, c"), _T("c, d")
};

static LPCTSTR hp_reg_1_af = _T("ABCDABCDBCACABAC");
static LPCTSTR hp_reg_2_af = _T("0000BCACABCDBCCD");

static LPCTSTR field_tbl[32] =
{
	/*
	 * HP Mnemonics
	 */
	_T("P"), _T("WP"), _T("XS"), _T("X"),
	_T("S"), _T("M"), _T("B"), _T("W"),
	_T("P"), _T("WP"), _T("XS"), _T("X"),
	_T("S"), _T("M"), _T("B"), _T("A"),
	/*
	 * Class Mnemonics
	 */
	_T(".p"), _T(".wp"), _T(".xs"), _T(".x"),
	_T(".s"), _T(".m"), _T(".b"), _T(".w"),
	_T(".p"), _T(".wp"), _T(".xs"), _T(".x"),
	_T(".s"), _T(".m"), _T(".b"), _T(".a")
};

static LPCTSTR hst_bits[8] =
{
	/*
	 * HP Mnemonics
	 */
	_T("XM"), _T("SB"), _T("SR"), _T("MP"),
	/*
	 * Class Mnemonics
	 */
	_T("xm"), _T("sb"), _T("sr"), _T("mp"),
};


// static functions

static BYTE rn_map (DWORD *p)
{
	BYTE byVal;

	Npeek(&byVal, *p, 1);
	*p = ++(*p) & 0xFFFFF;
	return byVal;
}

static BYTE rn_rom (DWORD *p)
{
	DWORD d = *p;

	*p = ++(*p) & (dwRomSize - 1);

	_ASSERT(d < dwRomSize);
	return *(pbyRom + d);
}

static BYTE rn_ram (DWORD *p)
{
	DWORD d = *p;

	*p = ++(*p) & (Chipset.Port0Size * 2048 - 1);

	_ASSERT(d < Chipset.Port0Size * 2048);
	return *(Chipset.Port0 + d);
}

static BYTE rn_port1 (DWORD *p)
{
	DWORD d = *p;

	*p = ++(*p) & (Chipset.Port1Size * 2048 - 1);

	_ASSERT(d < Chipset.Port1Size * 2048);
	return *(Chipset.Port1 + d);
}

static BYTE rn_port2 (DWORD *p)
{
	BYTE  *pbyVal;
	DWORD d = *p;

	if (Chipset.Port2Size)					// HP39/40G, HP49G
	{
		*p = ++(*p) & (Chipset.Port2Size * 2048 - 1);

		_ASSERT(d < Chipset.Port2Size * 2048);
		pbyVal = Chipset.Port2;
	}
	else									// HP48SX/GX
	{
		*p = ++(*p) & (((dwPort2Mask + 1) << 18) - 1);

		_ASSERT(d < ((dwPort2Mask + 1) << 18));
		pbyVal = pbyPort2;
	}
	return *(pbyVal + d);
}

static BYTE read_nibble (DWORD *p)
{
	BYTE (*pnread[])(DWORD *) = { rn_map, rn_rom, rn_ram, rn_port1, rn_port2 };

	_ASSERT(disassembler_map < ARRAYSIZEOF(pnread));
	return pnread[disassembler_map](p);
}

// general functions

static int read_int (DWORD *addr, int n)
{
	int i, t;

	for (i = 0, t = 0; i < n; i++)
		t |= read_nibble (addr) << (i * 4);

	return t;
}

static LPTSTR append_str (LPTSTR buf, LPCTSTR str)
{
	while ((*buf = *str++))
		buf++;
	return buf;
}

static LPTSTR append_tab (LPTSTR buf)
{
	int n;
	LPTSTR p;

	n = lstrlen (buf);
	p = &buf[n];
	n = TAB_SKIP - (n % TAB_SKIP);
	while (n--)
		*p++ = _T(' ');
	*p = 0;
	return p;
}

static LPTSTR append_field (LPTSTR buf, BYTE fn)
{
	return append_str (buf, field_tbl[fn + 16 * disassembler_mode]);
}

static LPTSTR append_imm_nibble (LPTSTR buf, DWORD *addr, int n)
{
	int i;
	TCHAR t[16];

	if (disassembler_mode == CLASS_MNEMONICS)
	{
		*buf++ = _T('#');
		if (n > 1)
			*buf++ = _T('$');
	}
	else									// HP Mnemonics
	{
		if (n > 1)							// hex mode
			*buf++ = _T('#');				// insert hex header
	}
	if (n > 1)
	{
		for (i = 0; i < n; i++)
			t[i] = hex[disassembler_mode][read_nibble (addr)];
		for (i = n - 1; i >= 0; i--)
		{
			*buf++ = t[i];
		}
		*buf = 0;
	}
	else
	{
		wsprintf (t, _T("%d"), read_nibble (addr));
		buf = append_str (buf, t);
	}
	return buf;
}

static LPTSTR append_addr (LPTSTR buf, DWORD addr)
{
	int shift;
	long mask;

	if (disassembler_mode == CLASS_MNEMONICS)
	{
		*buf++ = _T('$');
	}
	for (mask = 0xf0000, shift = 16; mask != 0; mask >>= 4, shift -= 4)
		*buf++ = hex[disassembler_mode][(addr & mask) >> shift];
	*buf = 0;
	return buf;
}

static LPTSTR append_r_addr (LPTSTR buf, DWORD * pc, long disp, int n, int offset)
{
	long sign;

	sign = 1 << (n * 4 - 1);
	if (disp & sign)
	disp |= ~(sign - 1);
	*pc += disp;

	switch (disassembler_mode)
	{
		case HP_MNEMONICS:
			if (disp < 0)
			{
				buf = append_str(buf, _T("-"));
				disp = -disp - offset;
			}
			else
			{
				buf = append_str(buf, _T("+"));
				disp += offset;
			}
			buf = append_addr(buf, disp);
			break;
		case CLASS_MNEMONICS:
			if (disp < 0)
			{
				buf = append_str(buf, _T("-"));
				disp = -disp - offset;
			}
			else
			{
				buf = append_str(buf, _T("+"));
				disp += offset;
			}
			buf = append_addr(buf, disp);
			break;
		default:
			buf = append_str (buf, _T("Unknown disassembler mode"));
			break;
	}
	return buf;
}

static LPTSTR append_pc_comment (LPTSTR buf, DWORD pc, BOOL view)
{
	LPTSTR p = buf;

	if (view == VIEW_LONG)					// output of address in remarks
	{
		while (lstrlen (buf) < 4 * TAB_SKIP)
			p = append_tab (buf);

		switch (disassembler_mode)
		{
			case HP_MNEMONICS:
				p = append_str (p, _T("# Address: "));
				p = append_addr (p, pc);
				break;
			case CLASS_MNEMONICS:
				p = append_str (p, _T("; address: "));
				p = append_addr (p, pc);
				break;
			default:
			p = append_str (p, _T("Unknown disassembler mode"));
			break;
		}
	}
	else									// output of address in brackets
	{
		while (*p) ++p;
		p = append_str (p, _T(" ["));
		p = append_addr (p, pc);
		p = append_str (p, _T("]"));
	}
	return p;
}

static LPTSTR append_hst_bits (LPTSTR buf, int n)
{
	int i;
	LPTSTR p = buf;

	switch (disassembler_mode)
	{
		case HP_MNEMONICS:
			for (i = 0; i < 4; i++)
				if (n & (1 << i))
				{
					p = append_str (p, hst_bits[i + 4 * disassembler_mode]);
				}
			break;
		case CLASS_MNEMONICS:
			while (lstrlen (buf) < 4 * TAB_SKIP)
				p = append_tab (buf);
			p = &buf[lstrlen (buf)];
			p = append_str (p, _T("; hst bits: "));

			for (buf = p, i = 0; i < 4; i++)
				if (n & (1 << i))
				{
					if (p != buf)
						p = append_str (p, _T(", "));
					p = append_str (p, hst_bits[i + 4 * disassembler_mode]);
				}
			break;
		default:
			p = append_str (p, _T("Unknown disassembler mode"));
			break;
	}
	return p;
}

static LPTSTR disasm_1 (DWORD *addr, LPTSTR out)
{
	BYTE n;
	BYTE fn;
	LPTSTR p;
	TCHAR buf[20];
	TCHAR c;

	p = out;
	switch (n = read_nibble (addr))
	{
		case 0:
		case 1:
			fn = read_nibble (addr);
			c = (fn < 8);					// flag for operand register
			fn = (fn & 7);					// get register number
			if (fn > 4)						// illegal opcode
				break;						// no output
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					c = (TCHAR) (c ? _T('A') : _T('C'));
					if (n == 0)
						wsprintf (buf, _T("R%d=%c"), fn, c);
					else
						wsprintf (buf, _T("%c=R%d"), c, fn);
					p = append_str (out, buf);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, _T("move.w"));
					p = append_tab (out);
					c = (TCHAR) (c ? _T('a') : _T('c'));
					if (n == 0)
						wsprintf (buf, _T("%c, r%d"), c, fn);
					else
						wsprintf (buf, _T("r%d, %c"), fn, c);
					p = append_str (p, buf);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 2:
			fn = read_nibble (addr);
			c = (fn < 8);					// flag for operand register
			fn = (fn & 7);					// get register number
			if (fn > 4)						// illegal opcode
				break;						// no output
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					c = (TCHAR) (c ? _T('A') : _T('C'));
					wsprintf (buf, _T("%cR%dEX"), c, fn);
					p = append_str (out, buf);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, _T("exg.w"));
					p = append_tab (out);
					c = (TCHAR) (c ? _T('a') : _T('c'));
					wsprintf (buf, _T("%c, r%d"), c, fn);
					p = append_str (p, buf);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 3:
			n = read_nibble (addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					c = (n & 4) ? _T('C') : _T('A');
					if (n & 2)
					{
						if (n < 8)
						{
							wsprintf (buf, _T("%cD%dEX"), c, (n & 1));
						}
						else
						{
							wsprintf (buf, _T("%cD%dXS"), c, (n & 1));
						}
					}
					else
					{
						if (n < 8)
						{
							wsprintf (buf, _T("D%d=%c"), (n & 1), c);
						}
						else
						{
							wsprintf (buf, _T("D%d=%cS"), (n & 1), c);
						}
					}
					p = append_str (out, buf);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (n & 2) ? _T("exg.") : _T("move."));
					p = append_str (p, (n < 8) ? _T("a") : _T("4"));
					p = append_tab (out);
					c = (n & 4) ? _T('c') : _T('a');
					wsprintf (buf, _T("%c, d%d"), c, (n & 1));
					p = append_str (p, buf);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 4:
		case 5:
			fn = read_nibble (addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					p = append_str (out, op_str_1[(fn & 7) + 8 * disassembler_mode]);
					p = append_tab (out);
					if (n == 4)
					{
						p = append_str (p, (fn < 8) ? _T("A") : _T("B"));
					}
					else
					{
						n = read_nibble (addr);
						if (fn < 8)
						{
							p = append_field (p, n);
						}
						else
						{
							wsprintf (buf, _T("%d"), n + 1);
							p = append_str (p, buf);
						}
					}
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, _T("move"));
					if (n == 4)
					{
						p = append_str (p, _T("."));
						p = append_str (p, (fn < 8) ? _T("a") : _T("b"));
					}
					else
					{
						n = read_nibble (addr);
						if (fn < 8)
						{
							p = append_field (p, n);
						}
						else
						{
							wsprintf (buf, _T(".%d"), n + 1);
							p = append_str (p, buf);
						}
					}
					p = append_tab (out);
					p = append_str (p, op_str_1[(fn & 7) + 8 * disassembler_mode]);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 6:
		case 7:
		case 8:
		case 0xc:
			fn = read_nibble (addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if (n == 6 || n == 8)
						p = append_str (out, _T("D0=D0"));
					else
						p = append_str (out, _T("D1=D1"));
					if (n < 8)
						p = append_str (p, _T("+"));
					else
						p = append_str (p, _T("-"));
					p = append_tab (out);
					wsprintf (buf, _T("%d"), fn + 1);
					p = append_str (p, buf);
					break;
				case CLASS_MNEMONICS:
					if (n < 8)
						p = append_str (out, _T("add.a"));
					else
						p = append_str (out, _T("sub.a"));
					p = append_tab (out);
					wsprintf (buf, _T("#%d, "), fn + 1);
					p = append_str (p, buf);
					if (n == 6 || n == 8)
						p = append_str (p, _T("d0"));
					else
						p = append_str (p, _T("d1"));
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 9:
		case 0xa:
		case 0xb:
		case 0xd:
		case 0xe:
		case 0xf:
			c = (TCHAR) ((n < 0xd) ? _T('0') : _T('1'));
			switch (n & 3)
			{
				case 1:
					n = 2;
					break;
				case 2:
					n = 4;
					break;
				case 3:
					n = 5;
					break;
			}
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					wsprintf (buf, _T("D%c=(%d)"), c, n);
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_imm_nibble (p, addr, n);
					break;
				case CLASS_MNEMONICS:
					if (n == 5)
					{
						wsprintf (buf, _T("move.a"));
					}
					else
						if (n == 4)
						{
							wsprintf (buf, _T("move.as"));
						}
						else
						{
							wsprintf (buf, _T("move.b"));
						}
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_imm_nibble (p, addr, n);
					wsprintf (buf, _T(", d%c"), c);
					p = append_str (p, buf);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		default:
			break;
	}
	return p;
}

static LPTSTR disasm_8 (DWORD *addr, LPTSTR out, BOOL view)
{
	BYTE n;
	BYTE fn;
	LPTSTR p = out;
	TCHAR c;
	TCHAR buf[20];
	DWORD disp, pc;

	fn = read_nibble (addr);
	switch (fn)
	{
		case 0:
			n = read_nibble (addr);
			if (NULL != (p = (LPTSTR) in_str_80[n + 16 * disassembler_mode]))
			{
				p = append_str (out, p);
				return p;
			}
			switch (n)
			{
				case 8:
					fn = read_nibble (addr);
					if (NULL != (p = (LPTSTR) in_str_808[fn + 16 * disassembler_mode]))
					{
						p = append_str (out, p);
						return p;
					}
					switch (fn)
					{
						case 1:
							n = read_nibble (addr);
							if (n == 0)
							{
								switch (disassembler_mode)
								{
									case HP_MNEMONICS:
										p = append_str (out, _T("RSI"));
										break;
									case CLASS_MNEMONICS:
										p = append_str (out, _T("rsi"));
										break;
									default:
										p = append_str (out, _T("Unknown disassembler mode"));
										break;
								}
							}
							else
								p = out;	// illegal opcode, no output
							break;
						case 2:
							n = read_nibble (addr);
							switch (disassembler_mode)
							{
								case HP_MNEMONICS:
									if (n < 5)
									{
										wsprintf (buf, _T("LA(%d)"), n + 1);
									}
									else
									{
										wsprintf (buf, _T("LAHEX"));
									}
									p = append_str (out, buf);
									p = append_tab (out);
									p = append_imm_nibble (p, addr, n + 1);
									break;
								case CLASS_MNEMONICS:
									wsprintf (buf, _T("move.%d"), n + 1);
									p = append_str (out, buf);
									p = append_tab (out);
									p = append_imm_nibble (p, addr, n + 1);
									wsprintf (buf, _T(", a.p"));
									p = append_str (p, buf);
									break;
								default:
									p = append_str (out, _T("Unknown disassembler mode"));
									break;
							}
							break;

						case 4:
						case 5:
						case 8:
						case 9:
							switch (disassembler_mode)
							{
								case HP_MNEMONICS:
									wsprintf (buf, _T("%cBIT=%d"), (fn & 8) ? _T('C') : _T('A'),
									         (fn & 1) ? 1 : 0);
									p = append_str (out, buf);
									p = append_tab (out);
									p = append_imm_nibble (p, addr, 1);
									break;
								case CLASS_MNEMONICS:
									p = append_str (out, (fn & 1) ? _T("bset") : _T("bclr"));
									p = append_tab (out);
									p = append_imm_nibble (p, addr, 1);
									p = append_str (p, (fn & 8) ? _T(", c") : _T(", a"));
									break;
								default:
									p = append_str (out, _T("Unknown disassembler mode"));
									break;
							}
							break;

						case 6:
						case 7:
						case 0xa:
						case 0xb:
							n = read_nibble (addr);
							pc = *addr;
							disp = read_int (addr, 2);

							switch (disassembler_mode)
							{
								case HP_MNEMONICS:
									c = (TCHAR) ((fn < 0xa) ? _T('A') : _T('C'));
									wsprintf (buf, _T("?%cBIT=%d"), c, (fn & 1) ? 1 : 0);
									p = append_str (out, buf);
									p = append_tab (out);
									wsprintf (buf, _T("%d"), n);
									p = append_str (p, buf);
									if (disp != 0)
									{
										p = append_str (p, _T(", GOYES "));
										p = append_r_addr (p, &pc, disp, 2, 5);
										p = append_pc_comment (out, pc, view);
									}
									else
										p = append_str (p, _T(", RTNYES"));
									break;
								case CLASS_MNEMONICS:
									c = (TCHAR) ((fn < 0xa) ? _T('a') : _T('c'));
									p = append_str (out, (disp == 0) ? _T("rt") : _T("b"));
									p = append_str (p, (fn & 1) ? _T("bs") : _T("bc"));
									p = append_tab (out);
									wsprintf (buf, _T("#%d, %c"), n, c);
									p = append_str (p, buf);
									if (disp != 0)
									{
										p = append_str (p, _T(", "));
										p = append_r_addr (p, &pc, disp, 2, 5);
										p = append_pc_comment (out, pc, view);
									}
									break;
								default:
									p = append_str (out, _T("Unknown disassembler mode"));
									break;
							}
							break;

						default:
							break;
					}
					break;

				case 0xc:
				case 0xd:
				case 0xf:
					fn = read_nibble (addr);
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							wsprintf (buf, (n == 0xf) ? _T("%c%cEX") : _T("%c=%c"),
							         (n == 0xd) ? _T('P') : _T('C'), (n == 0xd) ? _T('C') : _T('P'));
							p = append_str (out, buf);
							p = append_tab (out);
							wsprintf (buf, _T("%d"), fn);
							p = append_str (p, buf);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, (n == 0xf) ? _T("exg.1") : _T("move.1"));
							p = append_tab (out);
							wsprintf (buf, (n == 0xd) ? _T("p, c.%d") : _T("c.%d, p"), fn);
							         p = append_str (p, buf);
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				default:
					break;
			}
			break;

		case 1:
			switch (n = read_nibble (addr))
			{
				case 0:
				case 1:
				case 2:
				case 3:
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							wsprintf (buf, _T("%sSLC"), op_str_81[(n & 3) + 4 * disassembler_mode]);
							p = append_str (out, buf);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, _T("rol.w"));
							p = append_tab (out);
							p = append_str (p, _T("#4, "));
							p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				case 4:
				case 5:
				case 6:
				case 7:
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							wsprintf (buf, _T("%sSRC"), op_str_81[(n & 3) + 4 * disassembler_mode]);
							p = append_str (out, buf);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, _T("ror.w"));
							p = append_tab (out);
							p = append_str (p, _T("#4, "));
							p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				case 8:
					fn = read_nibble (addr); // get number
					n = read_nibble (addr);	// get register selector
					if ((n & 7) > 3)		// illegal opcode
						break;				// no output
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							wsprintf (buf, _T("%s=%s%cCON"),
							         op_str_81[(n & 3) + 4 * disassembler_mode],
							         op_str_81[(n & 3) + 4 * disassembler_mode],
							         (n < 8) ? _T('+') : _T('-'));
							p = append_str (out, buf);
							p = append_tab (out);
							p = append_field (p, fn);
							fn = read_nibble (addr);
							wsprintf (buf, _T(", %d"), fn + 1);
							p = append_str (p, buf);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, (n < 8) ? _T("add") : _T("sub"));
							p = append_field (p, fn);
							p = append_tab (out);
							fn = read_nibble (addr);
							wsprintf (buf, _T("#%d, "), fn + 1);
							p = append_str (p, buf);
							p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				case 9:
					fn = read_nibble (addr); // get field selector
					n = read_nibble (addr);	// get register selector
					if (n > 3)				// illegal opcode
						break;				// no output
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							wsprintf (buf, _T("%sSRB.F"), op_str_81[n + 4 * disassembler_mode]);
							p = append_str (out, buf);
							p = append_tab (out);
							p = append_field (p, fn);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, _T("lsr"));
							p = append_field (p, fn);
							p = append_tab (out);
							p = append_str (p, _T("#1, "));
							p = append_str (p, op_str_81[n + 4 * disassembler_mode]);
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				case 0xa:
					fn = read_nibble (addr);
					n = read_nibble (addr);
					if (n > 2)				// illegal opcode
						break;				// no output
					c = (TCHAR) read_nibble (addr);
					if (((int) c & 7) > 4)	// illegal opcode
						break;				// no output
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							if (n == 2)
							{
								wsprintf (buf, _T("%cR%dEX.F"), ((int) c < 8) ? _T('A') : _T('C'),
								         (int) c & 7);
							}
							else
								if (n == 1)
								{
									wsprintf (buf, _T("%c=R%d.F"), ((int) c < 8) ? _T('A') : _T('C'),
									         (int) c & 7);
								}
								else
								{
									wsprintf (buf, _T("R%d=%c.F"), (int) c & 7,
									         ((int) c < 8) ? _T('A') : _T('C'));
								}
							p = append_str (out, buf);
							p = append_tab (out);
							p = append_field (p, fn);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, (n == 2) ? _T("exg") : _T("move"));
							p = append_field (p, fn);
							p = append_tab (out);
							if (n == 1)
							{
								wsprintf (buf, _T("r%d"), (int) c & 7);
								p = append_str (p, buf);
							}
							else
								p = append_str (p, ((int) c < 8) ? _T("a") : _T("c"));
							p = append_str (p, _T(", "));
							if (n == 1)
								p = append_str (p, ((int) c < 8) ? _T("a") : _T("c"));
							else
							{
								wsprintf (buf, _T("r%d"), (int) c & 7);
								p = append_str (p, buf);
							}
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				case 0xb:
					n = read_nibble (addr);
					if ((n < 2) || (n > 7))	// illegal opcode
						break;				// no output

					p = append_str (out, in_str_81b[n + 16 * disassembler_mode]);
					break;

				case 0xc:
				case 0xd:
				case 0xe:
				case 0xf:
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							wsprintf (buf, _T("%sSRB"), op_str_81[(n & 3) + 4 * disassembler_mode]);
							p = append_str (out, buf);
							break;
						case CLASS_MNEMONICS:
							p = append_str (out, _T("lsr.w"));
							p = append_tab (out);
							p = append_str (p, _T("#1, "));
							p = append_str (p, op_str_81[(n & 3) + 4 * disassembler_mode]);
							break;
						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							break;
					}
					break;

				default:
					break;
			}
			break;

		case 2:
			n = read_nibble (addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if (n == 0xf)
					{
						p = append_str (out, _T("CLRHST"));
					}
					else
					{
						// when not only one bit is set the HS=0 opcode is used
						if (n != 1 && n != 2 && n != 4 && n != 8)
						{
							p = append_str (out, _T("HS=0"));
							p = append_tab (out);
							wsprintf (buf, _T("%d"), n);
							p = append_str (p, buf);
						}
						else
						{
							p = append_hst_bits (out, n);
							p = append_str (p, _T("=0"));
						}
					}
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, _T("clr.1"));
					p = append_tab (out);
					wsprintf (buf, _T("#%d, hst"), n);
					p = append_str (p, buf);
					p = append_hst_bits (out, n);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 3:
			n = read_nibble (addr);
			pc = *addr;
			disp = read_int (addr, 2);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					p = append_str (out, _T("?"));
					p = append_hst_bits (p, n);
					p = append_str (p, _T("=0"));
					p = append_tab (out);
					if (disp != 0)
					{
						p = append_str (p, _T("GOYES "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					else
						p = append_str (p, _T("RTNYES"));
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (disp == 0) ? _T("rt") : _T("b"));
					p = append_str (p, _T("eq.1"));
					p = append_tab (out);
					wsprintf (buf, _T("#%d, hst"), n);
					p = append_str (p, buf);
					if (disp != 0)
					{
						p = append_str (p, _T(", "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					p = append_hst_bits (out, n);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 4:
		case 5:
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					wsprintf (buf, _T("ST=%d"), (fn == 4) ? 0 : 1);
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_imm_nibble (p, addr, 1);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (fn == 4) ? _T("bclr") : _T("bset"));
					p = append_tab (out);
					p = append_imm_nibble (p, addr, 1);
					p = append_str (p, _T(", st"));
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 6:
		case 7:
			n = read_nibble (addr);
			pc = *addr;
			disp = read_int (addr, 2);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					wsprintf (buf, _T("?ST=%d"), (fn == 6) ? 0 : 1);
					p = append_str (out, buf);
					p = append_tab (out);
					wsprintf (buf, _T("%d"), n);
					p = append_str (p, buf);
					if (disp != 0)
					{
						p = append_str (p, _T(", GOYES "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					else
						p = append_str (p, _T(", RTNYES"));
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (disp == 0) ? _T("rt") : _T("b"));
					p = append_str (p, (fn == 6) ? _T("bc") : _T("bs"));
					p = append_tab (out);
					wsprintf (buf, _T("#%d, st"), n);
					p = append_str (p, buf);
					if (disp != 0)
					{
						p = append_str (p, _T(", "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 8:
		case 9:
			n = read_nibble (addr);
			pc = *addr;
			disp = read_int (addr, 2);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					wsprintf (buf, _T("?P%c"), (fn == 8) ? _T('#') : _T('='));
					p = append_str (out, buf);
					p = append_tab (out);
					wsprintf (buf, _T("%d"), n);
					p = append_str (p, buf);
					if (disp != 0)
					{
						p = append_str (p, _T(", GOYES "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					else
						p = append_str (p, _T(", RTNYES"));
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (disp == 0) ? _T("rt") : _T("b"));
					p = append_str (p, (fn == 8) ? _T("ne.1") : _T("eq.1"));
					p = append_tab (out);
					wsprintf (buf, _T("#%d, p"), n);
					p = append_str (p, buf);
					if (disp != 0)
					{
						p = append_str (p, _T(", "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 0xc:
		case 0xe:
			pc = *addr;
			if (fn == 0xe)
				pc += 4;
			disp = read_int (addr, 4);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					p = append_str (out, (fn == 0xc) ? _T("GOLONG") : _T("GOSUBL"));
					p = append_tab (out);
					p = append_r_addr (p, &pc, disp, 4, (fn == 0xc) ? 2 : 6);
					p = append_pc_comment (out, pc, view);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (fn == 0xc) ? _T("bra.4") : _T("bsr.4"));
					p = append_tab (out);
					p = append_r_addr (p, &pc, disp, 4, (fn == 0xc) ? 2 : 6);
					p = append_pc_comment (out, pc, view);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 0xd:
		case 0xf:
			pc = read_int (addr, 5);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					p = append_str (out, (fn == 0xd) ? _T("GOVLNG") : _T("GOSBVL"));
					p = append_tab (out);
					p = append_addr (p, pc);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (fn == 0xd) ? _T("jmp") : _T("jsr"));
					p = append_tab (out);
					p = append_addr (p, pc);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		default:
			break;
	}
	return p;
}


// public functions

DWORD disassemble (DWORD addr, LPTSTR out, BOOL view)
{
	BYTE n;
	BYTE fn;
	LPTSTR p = out;
	TCHAR c;
	TCHAR buf[20];
	DWORD disp, pc;

	switch (n = read_nibble (&addr))
	{
		case 0:
			if ((n = read_nibble (&addr)) != 0xe)
			{
				p = append_str (out, opcode_0_tbl[n + 16 * disassembler_mode]);
				break;
			}
			fn = read_nibble (&addr);
			n = read_nibble (&addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					wsprintf (buf, op_str_0[(n & 7) + 8 * HP_MNEMONICS],
					          (n < 8) ? _T('&') : _T('!'));
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_field (p, fn);
					break;
				case CLASS_MNEMONICS:
					p = append_str (out, (n < 8) ? _T("and") : _T("or"));
					p = append_field (p, fn);
					p = append_tab (out);
					p = append_str (p, op_str_0[(n & 7) + 8 * CLASS_MNEMONICS]);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 1:
			p = disasm_1 (&addr, out);
			break;

		case 2:
			n = read_nibble (&addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					p = append_str (out, _T("P="));
					p = append_tab (out);
					wsprintf (buf, _T("%d"), n);
					p = append_str (p, buf);
					break;
				case CLASS_MNEMONICS:
					wsprintf (buf, _T("move.1  #%d, p"), n);
					p = append_str (out, buf);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 3:
			fn = read_nibble (&addr);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if (fn < 5)
					{
						wsprintf (buf, _T("LC(%d)"), fn + 1);
					}
					else
					{
						wsprintf (buf, _T("LCHEX"));
					}
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_imm_nibble (p, &addr, fn + 1);
					break;
				case CLASS_MNEMONICS:
					wsprintf (buf, _T("move.%d"), fn + 1);
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_imm_nibble (p, &addr, fn + 1);
					wsprintf (buf, _T(", c.p"));
					p = append_str (p, buf);
					break;
				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 4:
		case 5:
			pc = addr;
			disp = read_int (&addr, 2);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if (disp == 2)
					{
						p = append_str (out, _T("NOP3"));
						break;
					}
					wsprintf (buf, (disp == 0) ? _T("RTN%sC") : _T("GO%sC"), (n == 4) ? _T("") : _T("N"));
					p = append_str (out, buf);
					if (disp != 0)
					{
						p = append_tab (out);
						p = append_r_addr (p, &pc, disp, 2, 1);
						p = append_pc_comment (out, pc, view);
					}
					break;

				case CLASS_MNEMONICS:
					if (disp == 2)
					{
						p = append_str (out, _T("nop3"));
						break;
					}
					p = append_str (out, (disp == 0) ? _T("rtc") : _T("bc"));
					p = append_str (p, (n == 4) ? _T("s") : _T("c"));
					if (disp != 0)
					{
						p = append_tab (out);
						p = append_r_addr (p, &pc, disp, 2, 1);
						p = append_pc_comment (out, pc, view);
					}
					break;

				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 6:
			pc = addr;
			disp = read_int (&addr, 3);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if (disp == 3)
					{
						p = append_str (out, _T("NOP4"));
						break;
					}
					if (disp == 4)
					{
						p = append_str (out, _T("NOP5"));
						break;
					}
					p = append_str (out, _T("GOTO"));
					p = append_tab (out);
					p = append_r_addr (p, &pc, disp, 3, 1);
					p = append_pc_comment (out, pc, view);
					break;

				case CLASS_MNEMONICS:
					if (disp == 3)
					{
						p = append_str (out, _T("nop4"));
						break;
					}
					if (disp == 4)
					{
						p = append_str (out, _T("nop5"));
						break;
					}
					p = append_str (out, _T("bra.3"));
					p = append_tab (out);
					p = append_r_addr (p, &pc, disp, 3, 1);
					p = append_pc_comment (out, pc, view);
					break;

				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 7:
			pc = addr + 3;
			disp = read_int (&addr, 3);
			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					p = append_str (out, _T("GOSUB"));
					p = append_tab (out);
					p = append_r_addr (p, &pc, disp, 3, 4);
					p = append_pc_comment (out, pc, view);
					break;

				case CLASS_MNEMONICS:
					p = append_str (out, _T("bsr.3"));
					p = append_tab (out);
					p = append_r_addr (p, &pc, disp, 3, 4);
					p = append_pc_comment (out, pc, view);
					break;

				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		case 8:
			fn = read_nibble (&addr);	/* PEEK */
			--addr;
			if (fn != 0xa && fn != 0xb)
			{
				p = disasm_8 (&addr, out, view);
				break;
			}
			/* Fall through */

		case 9:
			fn = read_nibble (&addr);
			if (n == 8)
			{
				c = (TCHAR) ((fn == 0xa) ? 0 : 1);
				fn = 0xf;
			}
			else
			{
				c = (TCHAR) ((fn < 8) ? 0 : 1);
				fn &= 7;
			}

			n = read_nibble (&addr);
			pc = addr;
			disp = read_int (&addr, 2);

			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if ((c == 0) && (n >= 8))
						wsprintf (buf, op_str_9[(n & 3) + 8 * HP_MNEMONICS + 4],
						          in_str_9[((n >> 2) & 3) + 4 * c + 8 * HP_MNEMONICS]);
					else
						wsprintf (buf, op_str_9[(n & 3) + 8 * HP_MNEMONICS],
						          in_str_9[((n >> 2) & 3) + 4 * c + 8 * HP_MNEMONICS]);
					p = append_str (out, buf);
					p = append_tab (out);
					p = append_field (p, fn);
					p = append_str (p, _T(", "));
					p = append_str (p, (disp == 0) ? _T("RTNYES") : _T("GOYES "));
					if (disp != 0)
					{
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					break;

				case CLASS_MNEMONICS:
					p = append_str (out, (disp == 0) ? _T("rt") : _T("b"));
					p = append_str (p, in_str_9[((n >> 2) & 3) + 4 * c + 8 * CLASS_MNEMONICS]);
					p = append_field (p, fn);
					p = append_tab (out);
					if ((c == 0) && (n >= 8))
						p = append_str (p, op_str_9[(n & 3) + 8 * CLASS_MNEMONICS + 4]);
					else
						p = append_str (p, op_str_9[(n & 3) + 8 * CLASS_MNEMONICS]);
					if (disp != 0)
					{
						p = append_str (p, _T(", "));
						p = append_r_addr (p, &pc, disp, 2, 3);
						p = append_pc_comment (out, pc, view);
					}
					break;

				default:
					p = append_str (out, _T("Unknown disassembler mode"));
					break;
			}
			break;

		default:
			switch (n)
			{
				case 0xa:
					fn = read_nibble (&addr);
					c = (TCHAR) ((fn < 8) ? 0 : 1);
					fn &= 7;
					disp = 0xa;
					break;
				case 0xb:
					fn = read_nibble (&addr);
					c = (TCHAR) ((fn < 8) ? 0 : 1);
					fn &= 7;
					disp = 0xb;
					break;
				case 0xc:
				case 0xd:
					fn = 0xf;
					c = (TCHAR) (n & 1);
					disp = 0xa;
					break;
				case 0xe:
				case 0xf:
					fn = 0xf;
					c = (TCHAR) (n & 1);
					disp = 0xb;
					break;
				default:
					fn = 0;
					disp = 0;
					c = 0;
					break;
			}

			n = read_nibble (&addr);
			pc = 0;

			switch (disp)
			{
				case 0xa:
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							if (c == 0)
							{
								if (n < 0xc)
								{
									p = _T("+");
								}
								else
								{
									p = _T("%c=%c-1");
									pc = 2;
								}
							}
							else
							{
								if (n < 4)
								{
									p = _T("%c=0");
									pc = 1;
								}
								else
									if (n >= 0xc)
									{
										p = _T("%c%cEX");
										pc = 3;
									}
									else
									{
										p = _T("%c=%c");
										pc = 3;
									}
							}
							break;

						case CLASS_MNEMONICS:
							if (c == 0)
							{
								if (n < 0xc)
								{
									p = _T("add");
								}
								else
								{
									p = _T("dec");
									pc = 1;
								}
							}
							else
							{
								if (n < 4)
								{
									p = _T("clr");
									pc = 1;
								}
								else
									if (n >= 0xc)
									{
										p = _T("exg");
									}
									else
									{
										p = _T("move");
										if (n < 8)
											n -= 4;
									}
							}
							break;

						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							return addr;
					}
					break;

				case 0xb:
					switch (disassembler_mode)
					{
						case HP_MNEMONICS:
							if (c == 0)
							{
								if (n >= 0xc)
								{
									p = _T("-");
								}
								else
									if ((n >= 4) && (n <= 7))
									{
										p = _T("%c=%c+1");
										pc = 2;
										n -= 4;
									}
									else
									{
										p = _T("-");
									}
							}
							else
							{
								if (n < 4)
								{
									p = _T("%cSL");
									pc = 1;
								}
								else
									if (n < 8)
									{
										p = _T("%cSR");
										pc = 1;
									}
									else
										if (n < 0xc)
										{
											p = _T("%c=-%c");
											pc = 2;
										}
										else
										{
											p = _T("%c=-%c-1");
											pc = 2;
										}
							}
							break;

						case CLASS_MNEMONICS:
							if (c == 0)
							{
								if (n >= 0xc)
								{
									p = _T("subr");
								}
								else
									if ((n >= 4) && (n <= 7))
									{
										p = _T("inc");
										pc = 1;
										n -= 4;
									}
									else
									{
										p = _T("sub");
									}
							}
							else
							{
								pc = 1;
								if (n < 4)
								{
									p = _T("lsl");
								}
								else
									if (n < 8)
									{
										p = _T("lsr");
									}
									else
										if (n < 0xc)
										{
											p = _T("neg");
										}
										else
										{
											p = _T("not");
										}
							}
							break;

						default:
							p = append_str (out, _T("Unknown disassembler mode"));
							return addr;
					}
					break;

			}

			switch (disassembler_mode)
			{
				case HP_MNEMONICS:
					if (pc == 0)
					{
						wsprintf (buf, op_str_af[n + 16 * HP_MNEMONICS], p);
					}
					else
						if (pc == 1)
						{
							wsprintf (buf, p, (n & 3) + _T('A'));
						}
						else
							if (pc == 2)
							{
								wsprintf (buf, p, (n & 3) + _T('A'), (n & 3) + _T('A'));
							}
							else
							{
								wsprintf (buf, p, hp_reg_1_af[n], hp_reg_2_af[n]);
							}
							p = append_str (out, buf);
							p = append_tab (out);
							p = append_field (p, fn);
					break;

				case CLASS_MNEMONICS:
					p = append_str (out, p);
					p = append_field (p, fn);
					p = append_tab (out);
					if (pc == 1)
					{
						wsprintf (buf, _T("%c"), (n & 3) + _T('a'));
						p = append_str (p, buf);
					}
					else
					{
						p = append_str (p, op_str_af[n + 16 * CLASS_MNEMONICS]);
					}
					break;

				default:
					p = append_str (p, _T("Unknown disassembler mode"));
					break;
			}
			break;
	}
	*p = 0;

	return addr;
}

