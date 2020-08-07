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

#ifndef REGSSIMDBG_H
#define REGSSIMDBG_H

// for documentation purposes only..... these addresses are in use, but not defined here.
#define REGSSIMGPMISELBEG 0x8003c050
#define REGSSIMGPMISELEND 0x8003c05c

#define REGSSIMMEMSELBEG  0x8003c070
#define REGSSIMMEMSELEND  0x8003c07c

#define REGSSIMSSPSELBEG  0x8003c0a0
#define REGSSIMSSPSELEND  0x8003c0ac

#define HW_BROWNOUT_GENERATOR_ADDR_DEFINITION  0x8003c500

// real definitions
#define HW_SIMDBG_CS_REG_ADDR    0x8003C000
#define HW_SIMDBG_PTR_REG_ADDR   0x8003C010
#define HW_SIMDBG_TERM_REG_ADDR  0x8003C020

#define HW_SIMDBG_UART_REG_ADDR    0x8003C090
#define HW_SIMDBG_CORESYS_REG_ADDR 0x8003c100
#define HW_SIMDBG_SAIF_LOOPBACK_REG_ADDR 0x8003c110

#define HW_SIMDBG_CORESYS_REG_WR(v) ((*(volatile u32 *) HW_SIMDBG_CORESYS_REG_ADDR) = (v))
#define HW_SIMDBG_UART_REG_WR(v)  ((*(volatile u32 *) HW_SIMDBG_UART_REG_ADDR) = (v))
#define HW_SIMDBG_SAIF_LOOPBACK_REG_WR(v)  ((*(volatile u32 *) HW_SIMDBG_SAIF_LOOPBACK_REG_ADDR) = (v))

#define HW_SIMDBG_ANATOP_PROBE_OUTPUT_SEL_ADDR 0x8003c200
#define HW_SIMDBG_ANATOP_PROBE_INPUT_SEL_ADDR  0x8003c210
#define HW_SIMDBG_ANATOP_PROBE_DATA_ADDR       0x8003c220
#define HW_SIMDBG_ANATOP_PROBE_DIGTOP_ADDR     0x8003c230

#define HW_SIMDBG_ANATOP_PROBE_OUTPUT_SEL_WR(v) ((*(volatile u32*) HW_SIMDBG_ANATOP_PROBE_OUTPUT_SEL_ADDR) = (v))
#define HW_SIMDBG_ANATOP_PROBE_INPUT_SEL_WR(v)  ((*(volatile u32*) HW_SIMDBG_ANATOP_PROBE_INPUT_SEL_ADDR) = (v))
#define HW_SIMDBG_ANATOP_PROBE_DATA_WR(v)       ((*(volatile u32*) HW_SIMDBG_ANATOP_PROBE_DATA_ADDR) = (v))
#define HW_SIMDBG_ANATOP_PROBE_DIGTOP_WR(v)     ((*(volatile u32*) HW_SIMDBG_ANATOP_PROBE_DIGTOP_ADDR) = (v))

#define HW_SIMDBG_LCDIFMON_ADDR                0x8003c600
#define HW_SIMDBG_LCDIFMON_LEN_ADDR            0x8003c610
#define HW_SIMDBG_FSDBDUMPING_ADDR             0x8003c520

#define HW_SIMDBG_LCDIFMONADDR_WR(v)            ((*(volatile u32*) HW_SIMDBG_LCDIFMON_ADDR) = (v))
#define HW_SIMDBG_LCDIFMONLEN_WR(v)             ((*(volatile u32*) HW_SIMDBG_LCDIFMON_LEN_ADDR) = (v))
#define HW_SIMDBG_FSDBDUMPING_WR(v)             ((*(volatile u32*) HW_SIMDBG_FSDBDUMPING_ADDR) = (v))

#endif
