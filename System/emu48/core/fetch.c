/*
 *   fetch.c
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1999 Christoph Gie�elink
 *
 */
#include "pch.h"
#include "opcodes.h"

#define F	0xFF							// F = function

typedef const struct
{
	LPCVOID     pLnk;
	const DWORD dwTyp;
} JMPTAB, *PJMPTAB;

// jump tables
static JMPTAB oF_[] =
{
	(LPCVOID) oF0,			F,
	(LPCVOID) oF1,			F,
	(LPCVOID) oF2,			F,
	(LPCVOID) oF3,			F,
	(LPCVOID) oF4,			F,
	(LPCVOID) oF5,			F,
	(LPCVOID) oF6,			F,
	(LPCVOID) oF7,			F,
	(LPCVOID) oF8,			F,
	(LPCVOID) oF9,			F,
	(LPCVOID) oFA,			F,
	(LPCVOID) oFB,			F,
	(LPCVOID) oFC,			F,
	(LPCVOID) oFD,			F,
	(LPCVOID) oFE,			F,
	(LPCVOID) oFF,			F
};

static JMPTAB oE_[] =
{
	(LPCVOID) oE0,			F,
	(LPCVOID) oE1,			F,
	(LPCVOID) oE2,			F,
	(LPCVOID) oE3,			F,
	(LPCVOID) oE4,			F,
	(LPCVOID) oE5,			F,
	(LPCVOID) oE6,			F,
	(LPCVOID) oE7,			F,
	(LPCVOID) oE8,			F,
	(LPCVOID) oE9,			F,
	(LPCVOID) oEA,			F,
	(LPCVOID) oEB,			F,
	(LPCVOID) oEC,			F,
	(LPCVOID) oED,			F,
	(LPCVOID) oEE,			F,
	(LPCVOID) oEF,			F
};

static JMPTAB oD_[] =
{
	(LPCVOID) oD0,			F,
	(LPCVOID) oD1,			F,
	(LPCVOID) oD2,			F,
	(LPCVOID) oD3,			F,
	(LPCVOID) oD4,			F,
	(LPCVOID) oD5,			F,
	(LPCVOID) oD6,			F,
	(LPCVOID) oD7,			F,
	(LPCVOID) oD8,			F,
	(LPCVOID) oD9,			F,
	(LPCVOID) oDA,			F,
	(LPCVOID) oDB,			F,
	(LPCVOID) oDC,			F,
	(LPCVOID) oDD,			F,
	(LPCVOID) oDE,			F,
	(LPCVOID) oDF,			F
};

static JMPTAB oC_[] =
{
	(LPCVOID) oC0,			F,
	(LPCVOID) oC1,			F,
	(LPCVOID) oC2,			F,
	(LPCVOID) oC3,			F,
	(LPCVOID) oC4,			F,
	(LPCVOID) oC5,			F,
	(LPCVOID) oC6,			F,
	(LPCVOID) oC7,			F,
	(LPCVOID) oC8,			F,
	(LPCVOID) oC9,			F,
	(LPCVOID) oCA,			F,
	(LPCVOID) oCB,			F,
	(LPCVOID) oCC,			F,
	(LPCVOID) oCD,			F,
	(LPCVOID) oCE,			F,
	(LPCVOID) oCF,			F
};

static JMPTAB oBb_[] =
{
	(LPCVOID) oBb0,			F,
	(LPCVOID) oBb1,			F,
	(LPCVOID) oBb2,			F,
	(LPCVOID) oBb3,			F,
	(LPCVOID) oBb4,			F,
	(LPCVOID) oBb5,			F,
	(LPCVOID) oBb6,			F,
	(LPCVOID) oBb7,			F,
	(LPCVOID) oBb8,			F,
	(LPCVOID) oBb9,			F,
	(LPCVOID) oBbA,			F,
	(LPCVOID) oBbB,			F,
	(LPCVOID) oBbC,			F,
	(LPCVOID) oBbD,			F,
	(LPCVOID) oBbE,			F,
	(LPCVOID) oBbF,			F
};

static JMPTAB oBa_[] =
{
	(LPCVOID) oBa0,			F,
	(LPCVOID) oBa1,			F,
	(LPCVOID) oBa2,			F,
	(LPCVOID) oBa3,			F,
	(LPCVOID) oBa4,			F,
	(LPCVOID) oBa5,			F,
	(LPCVOID) oBa6,			F,
	(LPCVOID) oBa7,			F,
	(LPCVOID) oBa8,			F,
	(LPCVOID) oBa9,			F,
	(LPCVOID) oBaA,			F,
	(LPCVOID) oBaB,			F,
	(LPCVOID) oBaC,			F,
	(LPCVOID) oBaD,			F,
	(LPCVOID) oBaE,			F,
	(LPCVOID) oBaF,			F
};

static JMPTAB oB_[] =
{
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBa_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2,
	(LPCVOID) oBb_,			2
};

static JMPTAB oAb_[] =
{
	(LPCVOID) oAb0,			F,
	(LPCVOID) oAb1,			F,
	(LPCVOID) oAb2,			F,
	(LPCVOID) oAb3,			F,
	(LPCVOID) oAb4,			F,
	(LPCVOID) oAb5,			F,
	(LPCVOID) oAb6,			F,
	(LPCVOID) oAb7,			F,
	(LPCVOID) oAb8,			F,
	(LPCVOID) oAb9,			F,
	(LPCVOID) oAbA,			F,
	(LPCVOID) oAbB,			F,
	(LPCVOID) oAbC,			F,
	(LPCVOID) oAbD,			F,
	(LPCVOID) oAbE,			F,
	(LPCVOID) oAbF,			F
};

static JMPTAB oAa_[] =
{
	(LPCVOID) oAa0,			F,
	(LPCVOID) oAa1,			F,
	(LPCVOID) oAa2,			F,
	(LPCVOID) oAa3,			F,
	(LPCVOID) oAa4,			F,
	(LPCVOID) oAa5,			F,
	(LPCVOID) oAa6,			F,
	(LPCVOID) oAa7,			F,
	(LPCVOID) oAa8,			F,
	(LPCVOID) oAa9,			F,
	(LPCVOID) oAaA,			F,
	(LPCVOID) oAaB,			F,
	(LPCVOID) oAaC,			F,
	(LPCVOID) oAaD,			F,
	(LPCVOID) oAaE,			F,
	(LPCVOID) oAaF,			F
};

static JMPTAB oA_[] =
{
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAa_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2,
	(LPCVOID) oAb_,			2
};

static JMPTAB o9b_[] =
{
	(LPCVOID) o9b0,			F,
	(LPCVOID) o9b1,			F,
	(LPCVOID) o9b2,			F,
	(LPCVOID) o9b3,			F,
	(LPCVOID) o9b4,			F,
	(LPCVOID) o9b5,			F,
	(LPCVOID) o9b6,			F,
	(LPCVOID) o9b7,			F,
	(LPCVOID) o9b8,			F,
	(LPCVOID) o9b9,			F,
	(LPCVOID) o9bA,			F,
	(LPCVOID) o9bB,			F,
	(LPCVOID) o9bC,			F,
	(LPCVOID) o9bD,			F,
	(LPCVOID) o9bE,			F,
	(LPCVOID) o9bF,			F
};

static JMPTAB o9a_[] =
{
	(LPCVOID) o9a0,			F,
	(LPCVOID) o9a1,			F,
	(LPCVOID) o9a2,			F,
	(LPCVOID) o9a3,			F,
	(LPCVOID) o9a4,			F,
	(LPCVOID) o9a5,			F,
	(LPCVOID) o9a6,			F,
	(LPCVOID) o9a7,			F,
	(LPCVOID) o9a8,			F,
	(LPCVOID) o9a9,			F,
	(LPCVOID) o9aA,			F,
	(LPCVOID) o9aB,			F,
	(LPCVOID) o9aC,			F,
	(LPCVOID) o9aD,			F,
	(LPCVOID) o9aE,			F,
	(LPCVOID) o9aF,			F
};

static JMPTAB o9_[] =
{
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9a_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2,
	(LPCVOID) o9b_,			2
};

static JMPTAB o8B_[] =
{
	(LPCVOID) o8B0,			F,
	(LPCVOID) o8B1,			F,
	(LPCVOID) o8B2,			F,
	(LPCVOID) o8B3,			F,
	(LPCVOID) o8B4,			F,
	(LPCVOID) o8B5,			F,
	(LPCVOID) o8B6,			F,
	(LPCVOID) o8B7,			F,
	(LPCVOID) o8B8,			F,
	(LPCVOID) o8B9,			F,
	(LPCVOID) o8BA,			F,
	(LPCVOID) o8BB,			F,
	(LPCVOID) o8BC,			F,
	(LPCVOID) o8BD,			F,
	(LPCVOID) o8BE,			F,
	(LPCVOID) o8BF,			F
};

static JMPTAB o8A_[] =
{
	(LPCVOID) o8A0,			F,
	(LPCVOID) o8A1,			F,
	(LPCVOID) o8A2,			F,
	(LPCVOID) o8A3,			F,
	(LPCVOID) o8A4,			F,
	(LPCVOID) o8A5,			F,
	(LPCVOID) o8A6,			F,
	(LPCVOID) o8A7,			F,
	(LPCVOID) o8A8,			F,
	(LPCVOID) o8A9,			F,
	(LPCVOID) o8AA,			F,
	(LPCVOID) o8AB,			F,
	(LPCVOID) o8AC,			F,
	(LPCVOID) o8AD,			F,
	(LPCVOID) o8AE,			F,
	(LPCVOID) o8AF,			F
};

static JMPTAB o81B_[] =
{
	(LPCVOID) o_invalid4,	F,
	(LPCVOID) o81B1,		F,				// normally o_invalid4, Apple: LOOP
	(LPCVOID) o81B2,		F,
	(LPCVOID) o81B3,		F,
	(LPCVOID) o81B4,		F,
	(LPCVOID) o81B5,		F,
	(LPCVOID) o81B6,		F,
	(LPCVOID) o81B7,		F,
	(LPCVOID) o_invalid4,	F,				// Apple: SKPTOP
	(LPCVOID) o_invalid4,	F,				// Apple: SKBOT
	(LPCVOID) o_invalid4,	F,				// Apple: SKIP
	(LPCVOID) o_invalid4,	F,				// Apple: SKPLEN
	(LPCVOID) o_invalid4,	F,				// Apple: SKPID
	(LPCVOID) o_invalid4,	F,				// Apple: $SEMI
	(LPCVOID) o_invalid4,	F,				// Apple: DOCOL
	(LPCVOID) o_invalid4,	F
};

static JMPTAB o81Af2_[] =
{
	(LPCVOID) o81Af20,		F,
	(LPCVOID) o81Af21,		F,
	(LPCVOID) o81Af22,		F,
	(LPCVOID) o81Af23,		F,
	(LPCVOID) o81Af24,		F,
	(LPCVOID) o81Af21,		F,
	(LPCVOID) o81Af22,		F,
	(LPCVOID) o81Af23,		F,
	(LPCVOID) o81Af28,		F,
	(LPCVOID) o81Af29,		F,
	(LPCVOID) o81Af2A,		F,
	(LPCVOID) o81Af2B,		F,
	(LPCVOID) o81Af2C,		F,
	(LPCVOID) o81Af29,		F,
	(LPCVOID) o81Af2A,		F,
	(LPCVOID) o81Af2B,		F
};

static JMPTAB o81Af1_[] =
{
	(LPCVOID) o81Af10,		F,
	(LPCVOID) o81Af11,		F,
	(LPCVOID) o81Af12,		F,
	(LPCVOID) o81Af13,		F,
	(LPCVOID) o81Af14,		F,
	(LPCVOID) o81Af11,		F,
	(LPCVOID) o81Af12,		F,
	(LPCVOID) o81Af13,		F,
	(LPCVOID) o81Af18,		F,
	(LPCVOID) o81Af19,		F,
	(LPCVOID) o81Af1A,		F,
	(LPCVOID) o81Af1B,		F,
	(LPCVOID) o81Af1C,		F,
	(LPCVOID) o81Af19,		F,
	(LPCVOID) o81Af1A,		F,
	(LPCVOID) o81Af1B,		F
};

static JMPTAB o81Af0_[] =
{
	(LPCVOID) o81Af00,		F,
	(LPCVOID) o81Af01,		F,
	(LPCVOID) o81Af02,		F,
	(LPCVOID) o81Af03,		F,
	(LPCVOID) o81Af04,		F,
	(LPCVOID) o81Af01,		F,
	(LPCVOID) o81Af02,		F,
	(LPCVOID) o81Af03,		F,
	(LPCVOID) o81Af08,		F,
	(LPCVOID) o81Af09,		F,
	(LPCVOID) o81Af0A,		F,
	(LPCVOID) o81Af0B,		F,
	(LPCVOID) o81Af0C,		F,
	(LPCVOID) o81Af09,		F,
	(LPCVOID) o81Af0A,		F,
	(LPCVOID) o81Af0B,		F
};

static JMPTAB o81A_[] =
{
	(LPCVOID) o81Af0_,		5,
	(LPCVOID) o81Af1_,		5,
	(LPCVOID) o81Af2_,		5,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F
};

static JMPTAB o819_[] =
{
	(LPCVOID) o819f0,		F,
	(LPCVOID) o819f1,		F,
	(LPCVOID) o819f2,		F,
	(LPCVOID) o819f3,		F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F
};

static JMPTAB o818_[] =
{
	(LPCVOID) o818f0x,		F,
	(LPCVOID) o818f1x,		F,
	(LPCVOID) o818f2x,		F,
	(LPCVOID) o818f3x,		F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o818f8x,		F,
	(LPCVOID) o818f9x,		F,
	(LPCVOID) o818fAx,		F,
	(LPCVOID) o818fBx,		F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F,
	(LPCVOID) o_invalid6,	F
};

static JMPTAB o81_[] =
{
	(LPCVOID) o810,			F,
	(LPCVOID) o811,			F,
	(LPCVOID) o812,			F,
	(LPCVOID) o813,			F,
	(LPCVOID) o814,			F,
	(LPCVOID) o815,			F,
	(LPCVOID) o816,			F,
	(LPCVOID) o817,			F,
	(LPCVOID) o818_,		4,
	(LPCVOID) o819_,		4,
	(LPCVOID) o81A_,		4,
	(LPCVOID) o81B_,		3,
	(LPCVOID) o81C,			F,
	(LPCVOID) o81D,			F,
	(LPCVOID) o81E,			F,
	(LPCVOID) o81F,			F
};

static JMPTAB o8081_[] =
{
	(LPCVOID) o80810,		F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F,
	(LPCVOID) o_invalid5,	F
};

static JMPTAB o808_[] =
{
	(LPCVOID) o8080,		F,
	(LPCVOID) o8081_,		4,
	(LPCVOID) o8082X,		F,
	(LPCVOID) o8083,		F,
	(LPCVOID) o8084n,		F,
	(LPCVOID) o8085n,		F,
	(LPCVOID) o8086n,		F,
	(LPCVOID) o8087n,		F,
	(LPCVOID) o8088n,		F,
	(LPCVOID) o8089n,		F,
	(LPCVOID) o808An,		F,
	(LPCVOID) o808Bn,		F,
	(LPCVOID) o808C,		F,
	(LPCVOID) o808D,		F,
	(LPCVOID) o808E,		F,
	(LPCVOID) o808F,		F
};

static JMPTAB o80_[] =
{
	(LPCVOID) o800,			F,
	(LPCVOID) o801,			F,
	(LPCVOID) o802,			F,
	(LPCVOID) o803,			F,
	(LPCVOID) o804,			F,
	(LPCVOID) o805,			F,
	(LPCVOID) o806,			F,
	(LPCVOID) o807,			F,
	(LPCVOID) o808_,		3,
	(LPCVOID) o809,			F,
	(LPCVOID) o80A,			F,
	(LPCVOID) o80B,			F,
	(LPCVOID) o80Cn,		F,
	(LPCVOID) o80Dn,		F,
	(LPCVOID) o80E,			F,
	(LPCVOID) o80Fn,		F
};

static JMPTAB o8_[] =
{
	(LPCVOID) o80_,			2,
	(LPCVOID) o81_,			2,
	(LPCVOID) o82n,			F,
	(LPCVOID) o83n,			F,
	(LPCVOID) o84n,			F,
	(LPCVOID) o85n,			F,
	(LPCVOID) o86n,			F,
	(LPCVOID) o87n,			F,
	(LPCVOID) o88n,			F,
	(LPCVOID) o89n,			F,
	(LPCVOID) o8A_,			2,
	(LPCVOID) o8B_,			2,
	(LPCVOID) o8Cd4,		F,
	(LPCVOID) o8Dd5,		F,
	(LPCVOID) o8Ed4,		F,
	(LPCVOID) o8Fd5,		F
};

static JMPTAB o15_[] =
{
	(LPCVOID) o150a,		F,
	(LPCVOID) o151a,		F,
	(LPCVOID) o152a,		F,
	(LPCVOID) o153a,		F,
	(LPCVOID) o154a,		F,
	(LPCVOID) o155a,		F,
	(LPCVOID) o156a,		F,
	(LPCVOID) o157a,		F,
	(LPCVOID) o158x,		F,
	(LPCVOID) o159x,		F,
	(LPCVOID) o15Ax,		F,
	(LPCVOID) o15Bx,		F,
	(LPCVOID) o15Cx,		F,
	(LPCVOID) o15Dx,		F,
	(LPCVOID) o15Ex,		F,
	(LPCVOID) o15Fx,		F
};

static JMPTAB o14_[] =
{
	(LPCVOID) o140,			F,
	(LPCVOID) o141,			F,
	(LPCVOID) o142,			F,
	(LPCVOID) o143,			F,
	(LPCVOID) o144,			F,
	(LPCVOID) o145,			F,
	(LPCVOID) o146,			F,
	(LPCVOID) o147,			F,
	(LPCVOID) o148,			F,
	(LPCVOID) o149,			F,
	(LPCVOID) o14A,			F,
	(LPCVOID) o14B,			F,
	(LPCVOID) o14C,			F,
	(LPCVOID) o14D,			F,
	(LPCVOID) o14E,			F,
	(LPCVOID) o14F,			F
};

static JMPTAB o13_[] =
{
	(LPCVOID) o130,			F,
	(LPCVOID) o131,			F,
	(LPCVOID) o132,			F,
	(LPCVOID) o133,			F,
	(LPCVOID) o134,			F,
	(LPCVOID) o135,			F,
	(LPCVOID) o136,			F,
	(LPCVOID) o137,			F,
	(LPCVOID) o138,			F,
	(LPCVOID) o139,			F,
	(LPCVOID) o13A,			F,
	(LPCVOID) o13B,			F,
	(LPCVOID) o13C,			F,
	(LPCVOID) o13D,			F,
	(LPCVOID) o13E,			F,
	(LPCVOID) o13F,			F
};

static JMPTAB o12_[] =
{
	(LPCVOID) o120,			F,
	(LPCVOID) o121,			F,
	(LPCVOID) o122,			F,
	(LPCVOID) o123,			F,
	(LPCVOID) o124,			F,
	(LPCVOID) o121,			F,
	(LPCVOID) o122,			F,
	(LPCVOID) o123,			F,
	(LPCVOID) o128,			F,
	(LPCVOID) o129,			F,
	(LPCVOID) o12A,			F,
	(LPCVOID) o12B,			F,
	(LPCVOID) o12C,			F,
	(LPCVOID) o129,			F,
	(LPCVOID) o12A,			F,
	(LPCVOID) o12B,			F
};

static JMPTAB o11_[] =
{
	(LPCVOID) o110,			F,
	(LPCVOID) o111,			F,
	(LPCVOID) o112,			F,
	(LPCVOID) o113,			F,
	(LPCVOID) o114,			F,
	(LPCVOID) o111,			F,
	(LPCVOID) o112,			F,
	(LPCVOID) o113,			F,
	(LPCVOID) o118,			F,
	(LPCVOID) o119,			F,
	(LPCVOID) o11A,			F,
	(LPCVOID) o11B,			F,
	(LPCVOID) o11C,			F,
	(LPCVOID) o119,			F,
	(LPCVOID) o11A,			F,
	(LPCVOID) o11B,			F
};

static JMPTAB o10_[] =
{
	(LPCVOID) o100,			F,
	(LPCVOID) o101,			F,
	(LPCVOID) o102,			F,
	(LPCVOID) o103,			F,
	(LPCVOID) o104,			F,
	(LPCVOID) o101,			F,
	(LPCVOID) o102,			F,
	(LPCVOID) o103,			F,
	(LPCVOID) o108,			F,
	(LPCVOID) o109,			F,
	(LPCVOID) o10A,			F,
	(LPCVOID) o10B,			F,
	(LPCVOID) o10C,			F,
	(LPCVOID) o109,			F,
	(LPCVOID) o10A,			F,
	(LPCVOID) o10B,			F
};

static JMPTAB o1_[] =
{
	(LPCVOID) o10_,			2,
	(LPCVOID) o11_,			2,
	(LPCVOID) o12_,			2,
	(LPCVOID) o13_,			2,
	(LPCVOID) o14_,			2,
	(LPCVOID) o15_,			2,
	(LPCVOID) o16x,			F,
	(LPCVOID) o17x,			F,
	(LPCVOID) o18x,			F,
	(LPCVOID) o19d2,		F,
	(LPCVOID) o1Ad4,		F,
	(LPCVOID) o1Bd5,		F,
	(LPCVOID) o1Cx,			F,
	(LPCVOID) o1Dd2,		F,
	(LPCVOID) o1Ed4,		F,
	(LPCVOID) o1Fd5,		F
};

static JMPTAB o0E_[] =
{
	(LPCVOID) o0Ef0,		F,
	(LPCVOID) o0Ef1,		F,
	(LPCVOID) o0Ef2,		F,
	(LPCVOID) o0Ef3,		F,
	(LPCVOID) o0Ef4,		F,
	(LPCVOID) o0Ef5,		F,
	(LPCVOID) o0Ef6,		F,
	(LPCVOID) o0Ef7,		F,
	(LPCVOID) o0Ef8,		F,
	(LPCVOID) o0Ef9,		F,
	(LPCVOID) o0EfA,		F,
	(LPCVOID) o0EfB,		F,
	(LPCVOID) o0EfC,		F,
	(LPCVOID) o0EfD,		F,
	(LPCVOID) o0EfE,		F,
	(LPCVOID) o0EfF,		F
};

static JMPTAB o0_[] =
{
	(LPCVOID) o00,			F,
	(LPCVOID) o01,			F,
	(LPCVOID) o02,			F,
	(LPCVOID) o03,			F,
	(LPCVOID) o04,			F,
	(LPCVOID) o05,			F,
	(LPCVOID) o06,			F,
	(LPCVOID) o07,			F,
	(LPCVOID) o08,			F,
	(LPCVOID) o09,			F,
	(LPCVOID) o0A,			F,
	(LPCVOID) o0B,			F,
	(LPCVOID) o0C,			F,
	(LPCVOID) o0D,			F,
	(LPCVOID) o0E_,			3,
	(LPCVOID) o0F,			F
};

static JMPTAB o_[] =
{
	(LPCVOID) o0_,			1,
	(LPCVOID) o1_,			1,
	(LPCVOID) o2n,			F,
	(LPCVOID) o3X,			F,
	(LPCVOID) o4d2,			F,
	(LPCVOID) o5d2,			F,
	(LPCVOID) o6d3,			F,
	(LPCVOID) o7d3,			F,
	(LPCVOID) o8_,			1,
	(LPCVOID) o9_,			1,
	(LPCVOID) oA_,			1,
	(LPCVOID) oB_,			1,
	(LPCVOID) oC_,			1,
	(LPCVOID) oD_,			1,
	(LPCVOID) oE_,			1,
	(LPCVOID) oF_,			1
};

#include "types.h"
extern CHIPSET Chipset;
// opcode dispatcher
VOID EvalOpcode(LPBYTE I)
{
	DWORD   dwTemp, dwIndex = 0;
	PJMPTAB pJmpTab = o_;

/*
	printf("jmpbase:%08x\n", pJmpTab);

	printf("test dump:\n");
	for(int i =0; i< 32 ; i++)
	{
		printf("%02x ", I[i]);
	}
	printf("\n----\n");*/

	void (*jumpfunc)(LPBYTE para);

	do
	{
		

		dwTemp = I[dwIndex];
		_ASSERT(dwTemp <= 0xf);			// found packed data
		
		dwIndex = pJmpTab[dwTemp].dwTyp; // table entry by opcode
		pJmpTab = (PJMPTAB)pJmpTab[dwTemp].pLnk;  // next pointer type


		jumpfunc = (void (*)(LPBYTE))pJmpTab;
	}
	while (dwIndex != F);					// reference to table? -> again
	
	
	//printf("pre call IP:%08x, I:%02x, jumpfunc:%02x\n", Chipset.pc, I, jumpfunc);
	//((VOID (*)(LPBYTE)) pJmpTab) (I);		// call function
	jumpfunc(I);
	
	//printf("post call IP:%08x, I:%02x, jumpfunc:%02x\n", Chipset.pc, I, jumpfunc);
	return;
}
