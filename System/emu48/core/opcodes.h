/*
 *   opcodes.h
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1999 Christoph Gießelink
 *
 */

#define PCHANGED  ((void)(F_s[0]=Chipset.P,F_l[1]=Chipset.P+1))
#define INTERRUPT ((void)(Chipset.SoftInt=TRUE,bInterrupt=TRUE))

extern UINT F_s[16];
extern UINT F_l[16];

extern VOID o00(LPBYTE I); // RTNSXM
extern VOID o01(LPBYTE I); // RTN
extern VOID o02(LPBYTE I); // RTNSC
extern VOID o03(LPBYTE I); // RTNCC
extern VOID o04(LPBYTE I); // SETHEX
extern VOID o05(LPBYTE I); // SETDEC
extern VOID o06(LPBYTE I); // RSTK=C
extern VOID o07(LPBYTE I); // C=RSTK
extern VOID o08(LPBYTE I); // CLRST
extern VOID o09(LPBYTE I); // C=ST
extern VOID o0A(LPBYTE I); // ST=C
extern VOID o0B(LPBYTE I); // CSTEX
extern VOID o0C(LPBYTE I); // P=P+1
extern VOID o0D(LPBYTE I); // P=P-1
extern VOID o0Ef0(LPBYTE I); // A=A&B f
extern VOID o0Ef1(LPBYTE I); // B=B&C f
extern VOID o0Ef2(LPBYTE I); // C=C&A f
extern VOID o0Ef3(LPBYTE I); // D=D&C f
extern VOID o0Ef4(LPBYTE I); // B=B&A f
extern VOID o0Ef5(LPBYTE I); // C=C&B f
extern VOID o0Ef6(LPBYTE I); // A=A&C f
extern VOID o0Ef7(LPBYTE I); // C=C&D f
extern VOID o0Ef8(LPBYTE I); // A=A!B f
extern VOID o0Ef9(LPBYTE I); // B=B!C f
extern VOID o0EfA(LPBYTE I); // C=C!A f
extern VOID o0EfB(LPBYTE I); // D=D!C f
extern VOID o0EfC(LPBYTE I); // B=B!A f
extern VOID o0EfD(LPBYTE I); // C=C!B f
extern VOID o0EfE(LPBYTE I); // A=A!C f
extern VOID o0EfF(LPBYTE I); // C=C!D f
extern VOID o0F(LPBYTE I); // RTI
extern VOID o100(LPBYTE I); // R0=A W
extern VOID o101(LPBYTE I); // R1=A W
extern VOID o102(LPBYTE I); // R2=A W
extern VOID o103(LPBYTE I); // R3=A W
extern VOID o104(LPBYTE I); // R4=A W
extern VOID o108(LPBYTE I); // R0=C W
extern VOID o109(LPBYTE I); // R1=C W
extern VOID o10A(LPBYTE I); // R2=C W
extern VOID o10B(LPBYTE I); // R3=C W
extern VOID o10C(LPBYTE I); // R4=C W
extern VOID o110(LPBYTE I); // A=R0 W
extern VOID o111(LPBYTE I); // A=R1 W
extern VOID o112(LPBYTE I); // A=R2 W
extern VOID o113(LPBYTE I); // A=R3 W
extern VOID o114(LPBYTE I); // A=R4 W
extern VOID o118(LPBYTE I); // C=R0 W
extern VOID o119(LPBYTE I); // C=R1 W
extern VOID o11A(LPBYTE I); // C=R2 W
extern VOID o11B(LPBYTE I); // C=R3 W
extern VOID o11C(LPBYTE I); // C=R4 W
extern VOID o120(LPBYTE I); // AR0EX W
extern VOID o121(LPBYTE I); // AR1EX W
extern VOID o122(LPBYTE I); // AR2EX W
extern VOID o123(LPBYTE I); // AR3EX W
extern VOID o124(LPBYTE I); // AR4EX W
extern VOID o128(LPBYTE I); // CR0EX W
extern VOID o129(LPBYTE I); // CR1EX W
extern VOID o12A(LPBYTE I); // CR2EX W
extern VOID o12B(LPBYTE I); // CR3EX W
extern VOID o12C(LPBYTE I); // CR4EX W
extern VOID o130(LPBYTE I); // D0=A
extern VOID o131(LPBYTE I); // D1=A
extern VOID o132(LPBYTE I); // AD0EX
extern VOID o133(LPBYTE I); // AD1EX
extern VOID o134(LPBYTE I); // D0=C
extern VOID o135(LPBYTE I); // D1=C
extern VOID o136(LPBYTE I); // CD0EX
extern VOID o137(LPBYTE I); // CD1EX
extern VOID o138(LPBYTE I); // D0=AS
extern VOID o139(LPBYTE I); // D1=AS
extern VOID o13A(LPBYTE I); // AD0XS
extern VOID o13B(LPBYTE I); // AD1XS
extern VOID o13C(LPBYTE I); // D0=CS
extern VOID o13D(LPBYTE I); // D1=CS
extern VOID o13E(LPBYTE I); // CD0XS
extern VOID o13F(LPBYTE I); // CD1XS
extern VOID o140(LPBYTE I); // DAT0=A A
extern VOID o141(LPBYTE I); // DAT0=A A
extern VOID o144(LPBYTE I); // DAT0=C A
extern VOID o145(LPBYTE I); // DAT1=C A
extern VOID o148(LPBYTE I); // DAT0=A B
extern VOID o149(LPBYTE I); // DAT1=A B
extern VOID o14C(LPBYTE I); // DAT0=C B
extern VOID o14D(LPBYTE I); // DAT1=C B
extern VOID o142(LPBYTE I); // A=DAT0 A
extern VOID o143(LPBYTE I); // A=DAT1 A
extern VOID o146(LPBYTE I); // C=DAT0 A
extern VOID o147(LPBYTE I); // C=DAT1 A
extern VOID o14A(LPBYTE I); // A=DAT0 B
extern VOID o14B(LPBYTE I); // A=DAT1 B
extern VOID o14E(LPBYTE I); // C=DAT0 B
extern VOID o14F(LPBYTE I); // C=DAT0 B
extern VOID o150a(LPBYTE I); // DAT0=A a
extern VOID o151a(LPBYTE I); // DAT1=A a
extern VOID o154a(LPBYTE I); // DAT0=C a
extern VOID o155a(LPBYTE I); // DAT1=C a
extern VOID o152a(LPBYTE I); // A=DAT0 a
extern VOID o153a(LPBYTE I); // A=DAT1 a
extern VOID o156a(LPBYTE I); // C=DAT0 a
extern VOID o157a(LPBYTE I); // C=DAT1 a
extern VOID o158x(LPBYTE I); // DAT0=A x
extern VOID o159x(LPBYTE I); // DAT1=A x
extern VOID o15Cx(LPBYTE I); // DAT0=C x
extern VOID o15Dx(LPBYTE I); // DAT1=C x
extern VOID o15Ax(LPBYTE I); // A=DAT0 x
extern VOID o15Bx(LPBYTE I); // A=DAT1 x
extern VOID o15Ex(LPBYTE I); // C=DAT0 x
extern VOID o15Fx(LPBYTE I); // C=DAT1 x
extern VOID o16x(LPBYTE I); // D0=D0+ (n+1)
extern VOID o17x(LPBYTE I); // D1=D1+ (n+1)
extern VOID o18x(LPBYTE I); // D0=D0- (n+1)
extern VOID o19d2(LPBYTE I); // D0=(2) #dd
extern VOID o1Ad4(LPBYTE I); // D0=(4) #dddd
extern VOID o1Bd5(LPBYTE I); // D0=(5) #ddddd
extern VOID o1Cx(LPBYTE I); // D1=D1- (n+1)
extern VOID o1Dd2(LPBYTE I); // D1=(2) #dd
extern VOID o1Ed4(LPBYTE I); // D1=(4) #dddd
extern VOID o1Fd5(LPBYTE I); // D1=(5) #ddddd
extern VOID o2n(LPBYTE I); // P= n
extern VOID o3X(LPBYTE I); // LCHEX
extern VOID o4d2(LPBYTE I); // GOC #dd
extern VOID o5d2(LPBYTE I); // GONC
extern VOID o6d3(LPBYTE I); // GOTO
extern VOID o7d3(LPBYTE I); // GOSUB
extern VOID o800(LPBYTE I); // OUT=CS
extern VOID o801(LPBYTE I); // OUT=C
extern VOID o802(LPBYTE I); // A=IN
extern VOID o803(LPBYTE I); // C=IN
extern VOID o804(LPBYTE I); // UNCNFG
extern VOID o805(LPBYTE I); // CONFIG
extern VOID o806(LPBYTE I); // C=ID
extern VOID o807(LPBYTE I); // SHUTDN
extern VOID o8080(LPBYTE I); // INTON
extern VOID o80810(LPBYTE I); // RSI
extern VOID o8082X(LPBYTE I); // LA
extern VOID o8083(LPBYTE I); // BUSCB
extern VOID o8084n(LPBYTE I); // ABIT=0 n
extern VOID o8085n(LPBYTE I); // ABIT=1 n
extern VOID o8086n(LPBYTE I); // ?ABIT=0 n
extern VOID o8087n(LPBYTE I); // ?ABIT=1 n
extern VOID o8088n(LPBYTE I); // CBIT=0 n
extern VOID o8089n(LPBYTE I); // CBIT=1 n
extern VOID o808An(LPBYTE I); // ?CBIT=0 n
extern VOID o808Bn(LPBYTE I); // ?CBIT=1 n
extern VOID o808C(LPBYTE I); // PC=(A)
extern VOID o808D(LPBYTE I); // BUSCD
extern VOID o808E(LPBYTE I); // PC=(C)
extern VOID o808F(LPBYTE I); // INTOFF
extern VOID o809(LPBYTE I); // C+P+1 - HEX MODE
extern VOID o80A(LPBYTE I); // RESET
extern VOID o80B(LPBYTE I); // BUSCC
extern VOID o80Cn(LPBYTE I); // C=P n
extern VOID o80Dn(LPBYTE I); // P=C n
extern VOID o80E(LPBYTE I); // SREQ?
extern VOID o80Fn(LPBYTE I); // CPEX n
extern VOID o810(LPBYTE I); // ASLC
extern VOID o811(LPBYTE I); // BSLC
extern VOID o812(LPBYTE I); // CSLC
extern VOID o813(LPBYTE I); // DSLC
extern VOID o814(LPBYTE I); // ASRC
extern VOID o815(LPBYTE I); // BSRC
extern VOID o816(LPBYTE I); // CSRC
extern VOID o817(LPBYTE I); // DSRC
extern VOID o818f0x(LPBYTE I); // A=A+x+1 f
extern VOID o818f1x(LPBYTE I); // B=B+x+1 f
extern VOID o818f2x(LPBYTE I); // C=C+x+1 f
extern VOID o818f3x(LPBYTE I); // D=D+x+1 f
extern VOID o818f8x(LPBYTE I); // A=A-x-1 f
extern VOID o818f9x(LPBYTE I); // B=B-x-1 f
extern VOID o818fAx(LPBYTE I); // C=C-x-1 f
extern VOID o818fBx(LPBYTE I); // D=D-x-1 f
extern VOID o819f0(LPBYTE I); // ASRB.F
extern VOID o819f1(LPBYTE I); // BSRB.F
extern VOID o819f2(LPBYTE I); // CSRB.F
extern VOID o819f3(LPBYTE I); // DSRB.F
extern VOID o81Af00(LPBYTE I); // R0=A.F f
extern VOID o81Af01(LPBYTE I); // R1=A.F f
extern VOID o81Af02(LPBYTE I); // R2=A.F f
extern VOID o81Af03(LPBYTE I); // R3=A.F f
extern VOID o81Af04(LPBYTE I); // R4=A.F f
extern VOID o81Af08(LPBYTE I); // R0=C.F f
extern VOID o81Af09(LPBYTE I); // R1=C.F f
extern VOID o81Af0A(LPBYTE I); // R2=C.F f
extern VOID o81Af0B(LPBYTE I); // R3=C.F f
extern VOID o81Af0C(LPBYTE I); // R4=C.F f
extern VOID o81Af10(LPBYTE I); // A=R0.F f
extern VOID o81Af11(LPBYTE I); // A=R1.F f
extern VOID o81Af12(LPBYTE I); // A=R2.F f
extern VOID o81Af13(LPBYTE I); // A=R3.F f
extern VOID o81Af14(LPBYTE I); // A=R4.F f
extern VOID o81Af18(LPBYTE I); // C=R0.F f
extern VOID o81Af19(LPBYTE I); // C=R1.F f
extern VOID o81Af1A(LPBYTE I); // C=R2.F f
extern VOID o81Af1B(LPBYTE I); // C=R3.F f
extern VOID o81Af1C(LPBYTE I); // C=R4.F f
extern VOID o81Af20(LPBYTE I); // AR0EX.F f
extern VOID o81Af21(LPBYTE I); // AR1EX.F f
extern VOID o81Af22(LPBYTE I); // AR2EX.F f
extern VOID o81Af23(LPBYTE I); // AR3EX.F f
extern VOID o81Af24(LPBYTE I); // AR4EX.F f
extern VOID o81Af28(LPBYTE I); // CR0EX.F f
extern VOID o81Af29(LPBYTE I); // CR1EX.F f
extern VOID o81Af2A(LPBYTE I); // CR2EX.F f
extern VOID o81Af2B(LPBYTE I); // CR3EX.F f
extern VOID o81Af2C(LPBYTE I); // CR4EX.F f
extern VOID o81B2(LPBYTE I); // PC=A
extern VOID o81B3(LPBYTE I); // PC=C
extern VOID o81B4(LPBYTE I); // A=PC
extern VOID o81B5(LPBYTE I); // C=PC
extern VOID o81B6(LPBYTE I); // APCEX
extern VOID o81B7(LPBYTE I); // CPCEX
extern VOID o81C(LPBYTE I); // ASRB
extern VOID o81D(LPBYTE I); // BSRB
extern VOID o81E(LPBYTE I); // CSRB
extern VOID o81F(LPBYTE I); // DSRB
extern VOID o82n(LPBYTE I); // HST=0 m
extern VOID o83n(LPBYTE I); // ?HST=0 m
extern VOID o84n(LPBYTE I); // ST=0 n
extern VOID o85n(LPBYTE I); // ST=1 n
extern VOID o86n(LPBYTE I); // ?ST=0 n
extern VOID o87n(LPBYTE I); // ?ST=1 n
extern VOID o88n(LPBYTE I); // ?P# n
extern VOID o89n(LPBYTE I); // ?P= n
extern VOID o8A0(LPBYTE I); // ?A=B A
extern VOID o8A1(LPBYTE I); // ?B=C A
extern VOID o8A2(LPBYTE I); // ?C=A A
extern VOID o8A3(LPBYTE I); // ?D=C A
extern VOID o8A4(LPBYTE I); // ?A#B A
extern VOID o8A5(LPBYTE I); // ?B#C A
extern VOID o8A6(LPBYTE I); // ?C#A A
extern VOID o8A7(LPBYTE I); // ?D#C A
extern VOID o8A8(LPBYTE I); // ?A=0 A
extern VOID o8A9(LPBYTE I); // ?B=0 A
extern VOID o8AA(LPBYTE I); // ?C=0 A
extern VOID o8AB(LPBYTE I); // ?D=0 A
extern VOID o8AC(LPBYTE I); // ?A#0 A
extern VOID o8AD(LPBYTE I); // ?B#0 A
extern VOID o8AE(LPBYTE I); // ?C#0 A
extern VOID o8AF(LPBYTE I); // ?D#0 A
extern VOID o8B0(LPBYTE I); // ?A>B A
extern VOID o8B1(LPBYTE I); // ?B>C A
extern VOID o8B2(LPBYTE I); // ?C>A A
extern VOID o8B3(LPBYTE I); // ?D>C A
extern VOID o8B4(LPBYTE I); // ?A<B A
extern VOID o8B5(LPBYTE I); // ?B<C A
extern VOID o8B6(LPBYTE I); // ?C<A A
extern VOID o8B7(LPBYTE I); // ?D<C A
extern VOID o8B8(LPBYTE I); // ?A>=B A
extern VOID o8B9(LPBYTE I); // ?B>=C A
extern VOID o8BA(LPBYTE I); // ?C>=A A
extern VOID o8BB(LPBYTE I); // ?D>=C A
extern VOID o8BC(LPBYTE I); // ?A<=B A
extern VOID o8BD(LPBYTE I); // ?B<=C A
extern VOID o8BE(LPBYTE I); // ?C<=A A
extern VOID o8BF(LPBYTE I); // ?D<=C A
extern VOID o8Cd4(LPBYTE I); // GOLONG #dddd
extern VOID o8Dd5(LPBYTE I); // GOVLNG #ddddd
extern VOID o8Ed4(LPBYTE I); // GOSUBL #dddd
extern VOID o8Fd5(LPBYTE I); // GOSBVL #ddddd
extern VOID o9a0(LPBYTE I); // ?A=B f
extern VOID o9a1(LPBYTE I); // ?B=C f
extern VOID o9a2(LPBYTE I); // ?C=A f
extern VOID o9a3(LPBYTE I); // ?D=C f
extern VOID o9a4(LPBYTE I); // ?A#B f
extern VOID o9a5(LPBYTE I); // ?B#C f
extern VOID o9a6(LPBYTE I); // ?C#A f
extern VOID o9a7(LPBYTE I); // ?D#C f
extern VOID o9a8(LPBYTE I); // ?A=0 f
extern VOID o9a9(LPBYTE I); // ?B=0 f
extern VOID o9aA(LPBYTE I); // ?C=0 f
extern VOID o9aB(LPBYTE I); // ?D=0 f
extern VOID o9aC(LPBYTE I); // ?A#0 f
extern VOID o9aD(LPBYTE I); // ?B#0 f
extern VOID o9aE(LPBYTE I); // ?C#0 f
extern VOID o9aF(LPBYTE I); // ?D#0 f
extern VOID o9b0(LPBYTE I); // ?A>B f
extern VOID o9b1(LPBYTE I); // ?B>C f
extern VOID o9b2(LPBYTE I); // ?C>A f
extern VOID o9b3(LPBYTE I); // ?D>C f
extern VOID o9b4(LPBYTE I); // ?A<B f
extern VOID o9b5(LPBYTE I); // ?B<C f
extern VOID o9b6(LPBYTE I); // ?C<A f
extern VOID o9b7(LPBYTE I); // ?D<C f
extern VOID o9b8(LPBYTE I); // ?A>=B f
extern VOID o9b9(LPBYTE I); // ?B>=C f
extern VOID o9bA(LPBYTE I); // ?C>=A f
extern VOID o9bB(LPBYTE I); // ?D>=C f
extern VOID o9bC(LPBYTE I); // ?A<=B f
extern VOID o9bD(LPBYTE I); // ?B<=C f
extern VOID o9bE(LPBYTE I); // ?C<=A f
extern VOID o9bF(LPBYTE I); // ?D<=C f
extern VOID oAa0(LPBYTE I); // A=A+B f
extern VOID oAa1(LPBYTE I); // B=B+C f
extern VOID oAa2(LPBYTE I); // C=C+A f
extern VOID oAa3(LPBYTE I); // D=D+C f
extern VOID oAa4(LPBYTE I); // A=A+A f
extern VOID oAa5(LPBYTE I); // B=B+B f
extern VOID oAa6(LPBYTE I); // C=C+C f
extern VOID oAa7(LPBYTE I); // D=D+D f
extern VOID oAa8(LPBYTE I); // B=B+A f
extern VOID oAa9(LPBYTE I); // C=C+B f
extern VOID oAaA(LPBYTE I); // A=A+C f
extern VOID oAaB(LPBYTE I); // C=C+D f
extern VOID oAaC(LPBYTE I); // A=A-1 f
extern VOID oAaD(LPBYTE I); // B=B-1 f
extern VOID oAaE(LPBYTE I); // C=C-1 f
extern VOID oAaF(LPBYTE I); // D=D-1 f
extern VOID oAb0(LPBYTE I); // A=0 f
extern VOID oAb1(LPBYTE I); // B=0 f
extern VOID oAb2(LPBYTE I); // C=0 f
extern VOID oAb3(LPBYTE I); // D=0 f
extern VOID oAb4(LPBYTE I); // A=B f
extern VOID oAb5(LPBYTE I); // B=C f
extern VOID oAb6(LPBYTE I); // C=A f
extern VOID oAb7(LPBYTE I); // D=C f
extern VOID oAb8(LPBYTE I); // B=A f
extern VOID oAb9(LPBYTE I); // C=B f
extern VOID oAbA(LPBYTE I); // A=C f
extern VOID oAbB(LPBYTE I); // C=D f
extern VOID oAbC(LPBYTE I); // ABEX f
extern VOID oAbD(LPBYTE I); // BCEX f
extern VOID oAbE(LPBYTE I); // CAEX f
extern VOID oAbF(LPBYTE I); // DCEX f
extern VOID oBa0(LPBYTE I); // A=A-B f
extern VOID oBa1(LPBYTE I); // B=B-C f
extern VOID oBa2(LPBYTE I); // C=C-A f
extern VOID oBa3(LPBYTE I); // D=D-C f
extern VOID oBa4(LPBYTE I); // A=A+1 f
extern VOID oBa5(LPBYTE I); // B=B+1 f
extern VOID oBa6(LPBYTE I); // C=C+1 f
extern VOID oBa7(LPBYTE I); // D=D+1 f
extern VOID oBa8(LPBYTE I); // B=B-A f
extern VOID oBa9(LPBYTE I); // C=C-B f
extern VOID oBaA(LPBYTE I); // A=A-C f
extern VOID oBaB(LPBYTE I); // C=C-D f
extern VOID oBaC(LPBYTE I); // A=B-A f
extern VOID oBaD(LPBYTE I); // B=C-B f
extern VOID oBaE(LPBYTE I); // C=A-C f
extern VOID oBaF(LPBYTE I); // D=C-D f
extern VOID oBb0(LPBYTE I); // ASL f
extern VOID oBb1(LPBYTE I); // BSL f
extern VOID oBb2(LPBYTE I); // CSL f
extern VOID oBb3(LPBYTE I); // DSL f
extern VOID oBb4(LPBYTE I); // ASR f
extern VOID oBb5(LPBYTE I); // BSR f
extern VOID oBb6(LPBYTE I); // CSR f
extern VOID oBb7(LPBYTE I); // DSR f
extern VOID oBb8(LPBYTE I); // A=-A f
extern VOID oBb9(LPBYTE I); // B=-B f
extern VOID oBbA(LPBYTE I); // C=-C f
extern VOID oBbB(LPBYTE I); // D=-D f
extern VOID oBbC(LPBYTE I); // A=-A-1 f
extern VOID oBbD(LPBYTE I); // B=-B-1 f
extern VOID oBbE(LPBYTE I); // C=-C-1 f
extern VOID oBbF(LPBYTE I); // D=-D-1 f
extern VOID oC0(LPBYTE I); // A=A+B A
extern VOID oC1(LPBYTE I); // B=B+C A
extern VOID oC2(LPBYTE I); // C=C+A A
extern VOID oC3(LPBYTE I); // D=D+C A
extern VOID oC4(LPBYTE I); // A=A+A A
extern VOID oC5(LPBYTE I); // B=B+B A
extern VOID oC6(LPBYTE I); // C=C+C A
extern VOID oC7(LPBYTE I); // D=D+D A
extern VOID oC8(LPBYTE I); // B=B+A A
extern VOID oC9(LPBYTE I); // C=C+B A
extern VOID oCA(LPBYTE I); // A=A+C A
extern VOID oCB(LPBYTE I); // C=C+D A
extern VOID oCC(LPBYTE I); // A=A-1 A
extern VOID oCD(LPBYTE I); // B=B-1 A
extern VOID oCE(LPBYTE I); // C=C-1 A
extern VOID oCF(LPBYTE I); // D=D-1 A
extern VOID oD0(LPBYTE I); // A=0 A
extern VOID oD1(LPBYTE I); // B=0 A
extern VOID oD2(LPBYTE I); // C=0 A
extern VOID oD3(LPBYTE I); // D=0 A
extern VOID oD4(LPBYTE I); // A=B A
extern VOID oD5(LPBYTE I); // B=C A
extern VOID oD6(LPBYTE I); // C=A A
extern VOID oD7(LPBYTE I); // D=C A
extern VOID oD8(LPBYTE I); // B=A A
extern VOID oD9(LPBYTE I); // C=B A
extern VOID oDA(LPBYTE I); // A=C A
extern VOID oDB(LPBYTE I); // C=D A
extern VOID oDC(LPBYTE I); // ABEX
extern VOID oDD(LPBYTE I); // BCEX
extern VOID oDE(LPBYTE I); // CAEX
extern VOID oDF(LPBYTE I); // DCEX
extern VOID oE0(LPBYTE I); // A=A-B A
extern VOID oE1(LPBYTE I); // B=B-C A
extern VOID oE2(LPBYTE I); // C=C-A A
extern VOID oE3(LPBYTE I); // D=D-C A
extern VOID oE4(LPBYTE I); // A=A+1 A
extern VOID oE5(LPBYTE I); // B=B+1 A
extern VOID oE6(LPBYTE I); // C=C+1 A
extern VOID oE7(LPBYTE I); // D=D+1 A
extern VOID oE8(LPBYTE I); // B=B-A A
extern VOID oE9(LPBYTE I); // C=C-B A
extern VOID oEA(LPBYTE I); // A=A-C A
extern VOID oEB(LPBYTE I); // C=C-D A
extern VOID oEC(LPBYTE I); // A=B-A A
extern VOID oED(LPBYTE I); // B=C-B A
extern VOID oEE(LPBYTE I); // C=A-C A
extern VOID oEF(LPBYTE I); // D=C-D A
extern VOID oF0(LPBYTE I); // ASL A
extern VOID oF1(LPBYTE I); // BSL A
extern VOID oF2(LPBYTE I); // CSL A
extern VOID oF3(LPBYTE I); // DSL A
extern VOID oF4(LPBYTE I); // ASR A
extern VOID oF5(LPBYTE I); // BSR A
extern VOID oF6(LPBYTE I); // CSR A
extern VOID oF7(LPBYTE I); // DSR A
extern VOID oF8(LPBYTE I); // A=-A A
extern VOID oF9(LPBYTE I); // B=-B A
extern VOID oFA(LPBYTE I); // C=-C A
extern VOID oFB(LPBYTE I); // D=-D A
extern VOID oFC(LPBYTE I); // A=-A-1 A
extern VOID oFD(LPBYTE I); // B=-B-1 A
extern VOID oFE(LPBYTE I); // C=-C-1 A
extern VOID oFF(LPBYTE I); // D=-D-1 A

extern VOID o_invalid3(LPBYTE I);
extern VOID o_invalid4(LPBYTE I);
extern VOID o_invalid5(LPBYTE I);
extern VOID o_invalid6(LPBYTE I);

extern VOID o_goyes3(LPBYTE I);
extern VOID o_goyes5(LPBYTE I);

extern VOID o81B1(LPBYTE I); // beep patch
