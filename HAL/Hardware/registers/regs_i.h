/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _REGS_I_H
#define _REGS_I_H  1


//
// macros for single instance registers
//

#define BFi_SET(i, reg, field)       HWi_##reg##_SET(i, BM_##reg##_##field)
#define BFi_CLR(i, reg, field)       HWi_##reg##_CLR(i, BM_##reg##_##field)
#define BFi_TOG(i, reg, field)       HWi_##reg##_TOG(i, BM_##reg##_##field)

#define BFi_SETV(i, reg, field, v)   HWi_##reg##_SET(i, BF_##reg##_##field(v))
#define BFi_CLRV(i, reg, field, v)   HWi_##reg##_CLR(i, BF_##reg##_##field(v))
#define BFi_TOGV(i, reg, field, v)   HWi_##reg##_TOG(i, BF_##reg##_##field(v))

#define BFi_RD(i, reg, field)        HWi_##reg(i).B.field
#define BFi_WR(i, reg, field, v)     BWi_##reg##_##field(i, v)

#define BFi_CS1(i, reg, f1, v1)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1)))

#define BFi_CS2(i, reg, f1, v1, f2, v2)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2)))

#define BFi_CS3(i, reg, f1, v1, f2, v2, f3, v3)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2 |      \
                            BM_##reg##_##f3),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2) |  \
                            BF_##reg##_##f3(v3)))

#define BFi_CS4(i, reg, f1, v1, f2, v2, f3, v3, f4, v4)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2 |      \
                            BM_##reg##_##f3 |      \
                            BM_##reg##_##f4),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2) |  \
                            BF_##reg##_##f3(v3) |  \
                            BF_##reg##_##f4(v4)))

#define BFi_CS5(i, reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2 |      \
                            BM_##reg##_##f3 |      \
                            BM_##reg##_##f4 |      \
                            BM_##reg##_##f5),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2) |  \
                            BF_##reg##_##f3(v3) |  \
                            BF_##reg##_##f4(v4) |  \
                            BF_##reg##_##f5(v5)))

#define BFi_CS6(i, reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2 |      \
                            BM_##reg##_##f3 |      \
                            BM_##reg##_##f4 |      \
                            BM_##reg##_##f5 |      \
                            BM_##reg##_##f6),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2) |  \
                            BF_##reg##_##f3(v3) |  \
                            BF_##reg##_##f4(v4) |  \
                            BF_##reg##_##f5(v5) |  \
                            BF_##reg##_##f6(v6)))

#define BFi_CS7(i, reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2 |      \
                            BM_##reg##_##f3 |      \
                            BM_##reg##_##f4 |      \
                            BM_##reg##_##f5 |      \
                            BM_##reg##_##f6 |      \
                            BM_##reg##_##f7),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2) |  \
                            BF_##reg##_##f3(v3) |  \
                            BF_##reg##_##f4(v4) |  \
                            BF_##reg##_##f5(v5) |  \
                            BF_##reg##_##f6(v6) |  \
                            BF_##reg##_##f7(v7)))

#define BFi_CS8(i, reg, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)  \
        (HWi_##reg##_CLR(i, BM_##reg##_##f1 |      \
                            BM_##reg##_##f2 |      \
                            BM_##reg##_##f3 |      \
                            BM_##reg##_##f4 |      \
                            BM_##reg##_##f5 |      \
                            BM_##reg##_##f6 |      \
                            BM_##reg##_##f7 |      \
                            BM_##reg##_##f8),      \
         HWi_##reg##_SET(i, BF_##reg##_##f1(v1) |  \
                            BF_##reg##_##f2(v2) |  \
                            BF_##reg##_##f3(v3) |  \
                            BF_##reg##_##f4(v4) |  \
                            BF_##reg##_##f5(v5) |  \
                            BF_##reg##_##f6(v6) |  \
                            BF_##reg##_##f7(v7) |  \
                            BF_##reg##_##f8(v8)))


//
// macros for multiple instance registers
//

#define BFi_SETn(i, reg, n, field)       HWi_##reg##_SET(i, n, BM_##reg##_##field)
#define BFi_CLRn(i, reg, n, field)       HWi_##reg##_CLR(i, n, BM_##reg##_##field)
#define BFi_TOGn(i, reg, n, field)       HWi_##reg##_TOG(i, n, BM_##reg##_##field)

#define BFi_SETVn(i, reg, n, field, v)   HWi_##reg##_SET(i, n, BF_##reg##_##field(v))
#define BFi_CLRVn(i, reg, n, field, v)   HWi_##reg##_CLR(i, n, BF_##reg##_##field(v))
#define BFi_TOGVn(i, reg, n, field, v)   HWi_##reg##_TOG(i, n, BF_##reg##_##field(v))

#define BFi_RDn(i, reg, n, field)        HWi_##reg(i, n).B.field
#define BFi_WRn(i, reg, n, field, v)     BWi_##reg##_##field(i, n, v)

#define BFi_CS1n(i, reg, n, f1, v1)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1))))

#define BFi_CS2n(i, reg, n, f1, v1, f2, v2)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2))))

#define BFi_CS3n(i, reg, n, f1, v1, f2, v2, f3, v3)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2 |       \
                                BM_##reg##_##f3)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2) |   \
                                BF_##reg##_##f3(v3))))

#define BFi_CS4n(i, reg, n, f1, v1, f2, v2, f3, v3, f4, v4)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2 |       \
                                BM_##reg##_##f3 |       \
                                BM_##reg##_##f4)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2) |   \
                                BF_##reg##_##f3(v3) |   \
                                BF_##reg##_##f4(v4))))

#define BFi_CS5n(i, reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2 |       \
                                BM_##reg##_##f3 |       \
                                BM_##reg##_##f4 |       \
                                BM_##reg##_##f5)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2) |   \
                                BF_##reg##_##f3(v3) |   \
                                BF_##reg##_##f4(v4) |   \
                                BF_##reg##_##f5(v5))))

#define BFi_CS6n(i, reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2 |       \
                                BM_##reg##_##f3 |       \
                                BM_##reg##_##f4 |       \
                                BM_##reg##_##f5 |       \
                                BM_##reg##_##f6)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2) |   \
                                BF_##reg##_##f3(v3) |   \
                                BF_##reg##_##f4(v4) |   \
                                BF_##reg##_##f5(v5) |   \
                                BF_##reg##_##f6(v6))))

#define BFi_CS7n(i, reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2 |       \
                                BM_##reg##_##f3 |       \
                                BM_##reg##_##f4 |       \
                                BM_##reg##_##f5 |       \
                                BM_##reg##_##f6 |       \
                                BM_##reg##_##f7)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2) |   \
                                BF_##reg##_##f3(v3) |   \
                                BF_##reg##_##f4(v4) |   \
                                BF_##reg##_##f5(v5) |   \
                                BF_##reg##_##f6(v6) |   \
                                BF_##reg##_##f7(v7))))

#define BFi_CS8n(i, reg, n, f1, v1, f2, v2, f3, v3, f4, v4, f5, v5, f6, v6, f7, v7, f8, v8)  \
        (HWi_##reg##_CLR(i, n, (BM_##reg##_##f1 |       \
                                BM_##reg##_##f2 |       \
                                BM_##reg##_##f3 |       \
                                BM_##reg##_##f4 |       \
                                BM_##reg##_##f5 |       \
                                BM_##reg##_##f6 |       \
                                BM_##reg##_##f7 |       \
                                BM_##reg##_##f8)),      \
         HWi_##reg##_SET(i, n, (BF_##reg##_##f1(v1) |   \
                                BF_##reg##_##f2(v2) |   \
                                BF_##reg##_##f3(v3) |   \
                                BF_##reg##_##f4(v4) |   \
                                BF_##reg##_##f5(v5) |   \
                                BF_##reg##_##f6(v6) |   \
                                BF_##reg##_##f7(v7) |   \
                                BF_##reg##_##f8(v8))))



#endif // _REGS_I_H

////////////////////////////////////////////////////////////////////////////////
