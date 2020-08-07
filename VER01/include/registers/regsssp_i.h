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


#ifndef _REGSSSP_I_H
#define _REGSSSP_I_H  1

#include "regsssp.h"
#include "regs_i.h"


#define HWi_SSP_OFFSET  (REGS_SSP2_BASE - REGS_SSP1_BASE)
#define REGSi_SSP_BASE(i) ((REGS_SSP1_BASE - HWi_SSP_OFFSET) + HWi_SSP_OFFSET * (i))


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_CTRL0 - instance-indexed SSP Control Register 0
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_CTRL0_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000000)
#define HWi_SSP_CTRL0_SET_ADDR(i)  (REGSi_SSP_BASE(i) + 0x00000004)
#define HWi_SSP_CTRL0_CLR_ADDR(i)  (REGSi_SSP_BASE(i) + 0x00000008)
#define HWi_SSP_CTRL0_TOG_ADDR(i)  (REGSi_SSP_BASE(i) + 0x0000000C)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_CTRL0(i)           (*(volatile hw_ssp_ctrl0_t *) HWi_SSP_CTRL0_ADDR(i))
#define HWi_SSP_CTRL0_RD(i)        (HWi_SSP_CTRL0(i).U)
#define HWi_SSP_CTRL0_WR(i, v)     (HWi_SSP_CTRL0(i).U = (v))
#define HWi_SSP_CTRL0_SET(i, v)    ((*(volatile reg32_t *) HWi_SSP_CTRL0_SET_ADDR(i)) = (v))
#define HWi_SSP_CTRL0_CLR(i, v)    ((*(volatile reg32_t *) HWi_SSP_CTRL0_CLR_ADDR(i)) = (v))
#define HWi_SSP_CTRL0_TOG(i, v)    ((*(volatile reg32_t *) HWi_SSP_CTRL0_TOG_ADDR(i)) = (v))

#define BWi_SSP_CTRL0_SFTRST(i, v)           BFi_CS1(i, SSP_CTRL0, SFTRST, v)
#define BWi_SSP_CTRL0_CLKGATE(i, v)          BFi_CS1(i, SSP_CTRL0, CLKGATE, v)
#define BWi_SSP_CTRL0_RUN(i, v)              BFi_CS1(i, SSP_CTRL0, RUN, v)
#define BWi_SSP_CTRL0_SDIO_IRQ_CHECK(i, v)   BFi_CS1(i, SSP_CTRL0, SDIO_IRQ_CHECK, v)
#define BWi_SSP_CTRL0_LOCK_CS(i, v)          BFi_CS1(i, SSP_CTRL0, LOCK_CS, v)
#define BWi_SSP_CTRL0_IGNORE_CRC(i, v)       BFi_CS1(i, SSP_CTRL0, IGNORE_CRC, v)
#define BWi_SSP_CTRL0_READ(i, v)             BFi_CS1(i, SSP_CTRL0, READ, v)
#define BWi_SSP_CTRL0_DATA_XFER(i, v)        BFi_CS1(i, SSP_CTRL0, DATA_XFER, v)
#define BWi_SSP_CTRL0_BUS_WIDTH(i, v)        BFi_CS1(i, SSP_CTRL0, BUS_WIDTH, v)
#define BWi_SSP_CTRL0_WAIT_FOR_IRQ(i, v)     BFi_CS1(i, SSP_CTRL0, WAIT_FOR_IRQ, v)
#define BWi_SSP_CTRL0_WAIT_FOR_CMD(i, v)     BFi_CS1(i, SSP_CTRL0, WAIT_FOR_CMD, v)
#define BWi_SSP_CTRL0_LONG_RESP(i, v)        BFi_CS1(i, SSP_CTRL0, LONG_RESP, v)
#define BWi_SSP_CTRL0_CHECK_RESP(i, v)       BFi_CS1(i, SSP_CTRL0, CHECK_RESP, v)
#define BWi_SSP_CTRL0_GET_RESP(i, v)         BFi_CS1(i, SSP_CTRL0, GET_RESP, v)
#define BWi_SSP_CTRL0_ENABLE(i, v)           BFi_CS1(i, SSP_CTRL0, ENABLE, v)
#define BWi_SSP_CTRL0_XFER_COUNT(i, v)       (HWi_SSP_CTRL0(i).B.XFER_COUNT = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_CMD0 - instance-indexed SD/MMC and MS Command Register 0
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_CMD0_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000010)
#define HWi_SSP_CMD0_SET_ADDR(i)  (REGSi_SSP_BASE(i) + 0x00000014)
#define HWi_SSP_CMD0_CLR_ADDR(i)  (REGSi_SSP_BASE(i) + 0x00000018)
#define HWi_SSP_CMD0_TOG_ADDR(i)  (REGSi_SSP_BASE(i) + 0x0000001C)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_CMD0(i)           (*(volatile hw_ssp_cmd0_t *) HWi_SSP_CMD0_ADDR(i))
#define HWi_SSP_CMD0_RD(i)        (HWi_SSP_CMD0(i).U)
#define HWi_SSP_CMD0_WR(i, v)     (HWi_SSP_CMD0(i).U = (v))
#define HWi_SSP_CMD0_SET(i, v)    ((*(volatile reg32_t *) HWi_SSP_CMD0_SET_ADDR(i)) = (v))
#define HWi_SSP_CMD0_CLR(i, v)    ((*(volatile reg32_t *) HWi_SSP_CMD0_CLR_ADDR(i)) = (v))
#define HWi_SSP_CMD0_TOG(i, v)    ((*(volatile reg32_t *) HWi_SSP_CMD0_TOG_ADDR(i)) = (v))

#define BWi_SSP_CMD0_APPEND_8CYC(i, v)   BFi_CS1(i, SSP_CMD0, APPEND_8CYC, v)
#define BWi_SSP_CMD0_BLOCK_SIZE(i, v)    BFi_CS1(i, SSP_CMD0, BLOCK_SIZE, v)
#define BWi_SSP_CMD0_BLOCK_COUNT(i, v)   (HWi_SSP_CMD0(i).B.BLOCK_COUNT = (v))
#define BWi_SSP_CMD0_CMD(i, v)           (HWi_SSP_CMD0(i).B.CMD = (v))
#define BWi_SSP_CMD0_CONT_CLKING_EN(i, v) (HWi_SSP_CMD0(i).B.CONT_CLKING_EN = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_CMD1 - instance-indexed SD/MMC Command Register 1
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_CMD1_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000020)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_CMD1(i)           (*(volatile hw_ssp_cmd1_t *) HWi_SSP_CMD1_ADDR(i))
#define HWi_SSP_CMD1_RD(i)        (HWi_SSP_CMD1(i).U)
#define HWi_SSP_CMD1_WR(i, v)     (HWi_SSP_CMD1(i).U = (v))
#define HWi_SSP_CMD1_SET(i, v)    (HWi_SSP_CMD1_WR(i, HWi_SSP_CMD1_RD(i) |  (v)))
#define HWi_SSP_CMD1_CLR(i, v)    (HWi_SSP_CMD1_WR(i, HWi_SSP_CMD1_RD(i) & ~(v)))
#define HWi_SSP_CMD1_TOG(i, v)    (HWi_SSP_CMD1_WR(i, HWi_SSP_CMD1_RD(i) ^  (v)))

#define BWi_SSP_CMD1_CMD_ARG(i, v)   (HWi_SSP_CMD1(i).B.CMD_ARG = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_COMPREF - instance-indexed SD/MMC and MS Compare Reference
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_COMPREF_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000030)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_COMPREF(i)           (*(volatile hw_ssp_compref_t *) HWi_SSP_COMPREF_ADDR(i))
#define HWi_SSP_COMPREF_RD(i)        (HWi_SSP_COMPREF(i).U)
#define HWi_SSP_COMPREF_WR(i, v)     (HWi_SSP_COMPREF(i).U = (v))
#define HWi_SSP_COMPREF_SET(i, v)    (HWi_SSP_COMPREF_WR(i, HWi_SSP_COMPREF_RD(i) |  (v)))
#define HWi_SSP_COMPREF_CLR(i, v)    (HWi_SSP_COMPREF_WR(i, HWi_SSP_COMPREF_RD(i) & ~(v)))
#define HWi_SSP_COMPREF_TOG(i, v)    (HWi_SSP_COMPREF_WR(i, HWi_SSP_COMPREF_RD(i) ^  (v)))

#define BWi_SSP_COMPREF_REFERENCE(i, v)   (HWi_SSP_COMPREF(i).B.REFERENCE = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_COMPMASK - instance-indexed SD/MMC and MS compare mask
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_COMPMASK_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000040)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_COMPMASK(i)           (*(volatile hw_ssp_compmask_t *) HWi_SSP_COMPMASK_ADDR(i))
#define HWi_SSP_COMPMASK_RD(i)        (HWi_SSP_COMPMASK(i).U)
#define HWi_SSP_COMPMASK_WR(i, v)     (HWi_SSP_COMPMASK(i).U = (v))
#define HWi_SSP_COMPMASK_SET(i, v)    (HWi_SSP_COMPMASK_WR(i, HWi_SSP_COMPMASK_RD(i) |  (v)))
#define HWi_SSP_COMPMASK_CLR(i, v)    (HWi_SSP_COMPMASK_WR(i, HWi_SSP_COMPMASK_RD(i) & ~(v)))
#define HWi_SSP_COMPMASK_TOG(i, v)    (HWi_SSP_COMPMASK_WR(i, HWi_SSP_COMPMASK_RD(i) ^  (v)))

#define BWi_SSP_COMPMASK_MASK(i, v)   (HWi_SSP_COMPMASK(i).B.MASK = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_TIMING - instance-indexed SSP Timing Register
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_TIMING_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000050)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_TIMING(i)           (*(volatile hw_ssp_timing_t *) HWi_SSP_TIMING_ADDR(i))
#define HWi_SSP_TIMING_RD(i)        (HWi_SSP_TIMING(i).U)
#define HWi_SSP_TIMING_WR(i, v)     (HWi_SSP_TIMING(i).U = (v))
#define HWi_SSP_TIMING_SET(i, v)    (HWi_SSP_TIMING_WR(i, HWi_SSP_TIMING_RD(i) |  (v)))
#define HWi_SSP_TIMING_CLR(i, v)    (HWi_SSP_TIMING_WR(i, HWi_SSP_TIMING_RD(i) & ~(v)))
#define HWi_SSP_TIMING_TOG(i, v)    (HWi_SSP_TIMING_WR(i, HWi_SSP_TIMING_RD(i) ^  (v)))

#define BWi_SSP_TIMING_TIMEOUT(i, v)        (HWi_SSP_TIMING(i).B.TIMEOUT = (v))
#define BWi_SSP_TIMING_CLOCK_DIVIDE(i, v)   (HWi_SSP_TIMING(i).B.CLOCK_DIVIDE = (v))
#define BWi_SSP_TIMING_CLOCK_RATE(i, v)     (HWi_SSP_TIMING(i).B.CLOCK_RATE = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_CTRL1 - instance-indexed SSP Control Register 1
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_CTRL1_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000060)
#define HWi_SSP_CTRL1_SET_ADDR(i)  (REGSi_SSP_BASE(i) + 0x00000064)
#define HWi_SSP_CTRL1_CLR_ADDR(i)  (REGSi_SSP_BASE(i) + 0x00000068)
#define HWi_SSP_CTRL1_TOG_ADDR(i)  (REGSi_SSP_BASE(i) + 0x0000006C)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_CTRL1(i)           (*(volatile hw_ssp_ctrl1_t *) HWi_SSP_CTRL1_ADDR(i))
#define HWi_SSP_CTRL1_RD(i)        (HWi_SSP_CTRL1(i).U)
#define HWi_SSP_CTRL1_WR(i, v)     (HWi_SSP_CTRL1(i).U = (v))
#define HWi_SSP_CTRL1_SET(i, v)    ((*(volatile reg32_t *) HWi_SSP_CTRL1_SET_ADDR(i)) = (v))
#define HWi_SSP_CTRL1_CLR(i, v)    ((*(volatile reg32_t *) HWi_SSP_CTRL1_CLR_ADDR(i)) = (v))
#define HWi_SSP_CTRL1_TOG(i, v)    ((*(volatile reg32_t *) HWi_SSP_CTRL1_TOG_ADDR(i)) = (v))

#define BWi_SSP_CTRL1_SDIO_IRQ(i, v)              BFi_CS1(i, SSP_CTRL1, SDIO_IRQ, v)
#define BWi_SSP_CTRL1_SDIO_IRQ_EN(i, v)           BFi_CS1(i, SSP_CTRL1, SDIO_IRQ_EN, v)
#define BWi_SSP_CTRL1_RESP_ERR_IRQ(i, v)          BFi_CS1(i, SSP_CTRL1, RESP_ERR_IRQ, v)
#define BWi_SSP_CTRL1_RESP_ERR_IRQ_EN(i, v)       BFi_CS1(i, SSP_CTRL1, RESP_ERR_IRQ_EN, v)
#define BWi_SSP_CTRL1_RESP_TIMEOUT_IRQ(i, v)      BFi_CS1(i, SSP_CTRL1, RESP_TIMEOUT_IRQ, v)
#define BWi_SSP_CTRL1_RESP_TIMEOUT_IRQ_EN(i, v)   BFi_CS1(i, SSP_CTRL1, RESP_TIMEOUT_IRQ_EN, v)
#define BWi_SSP_CTRL1_DATA_TIMEOUT_IRQ(i, v)      BFi_CS1(i, SSP_CTRL1, DATA_TIMEOUT_IRQ, v)
#define BWi_SSP_CTRL1_DATA_TIMEOUT_IRQ_EN(i, v)   BFi_CS1(i, SSP_CTRL1, DATA_TIMEOUT_IRQ_EN, v)
#define BWi_SSP_CTRL1_DATA_CRC_IRQ(i, v)          BFi_CS1(i, SSP_CTRL1, DATA_CRC_IRQ, v)
#define BWi_SSP_CTRL1_DATA_CRC_IRQ_EN(i, v)       BFi_CS1(i, SSP_CTRL1, DATA_CRC_IRQ_EN, v)
#define BWi_SSP_CTRL1_FIFO_UNDERRUN_IRQ(i, v)     BFi_CS1(i, SSP_CTRL1, FIFO_UNDERRUN_IRQ, v)
#define BWi_SSP_CTRL1_FIFO_UNDERRUN_EN(i, v)      BFi_CS1(i, SSP_CTRL1, FIFO_UNDERRUN_EN, v)
#define BWi_SSP_CTRL1_CEATA_CCS_ERR_IRQ(i, v)     BFi_CS1(i, SSP_CTRL1, CEATA_CCS_ERR_IRQ, v)
#define BWi_SSP_CTRL1_CEATA_CCS_ERR_IRQ_EN(i, v)  BFi_CS1(i, SSP_CTRL1, CEATA_CCS_ERR_IRQ_EN, v)
#define BWi_SSP_CTRL1_RECV_TIMEOUT_IRQ(i, v)      BFi_CS1(i, SSP_CTRL1, RECV_TIMEOUT_IRQ, v)
#define BWi_SSP_CTRL1_RECV_TIMEOUT_IRQ_EN(i, v)   BFi_CS1(i, SSP_CTRL1, RECV_TIMEOUT_IRQ_EN, v)
#define BWi_SSP_CTRL1_FIFO_OVERRUN_IRQ(i, v)      BFi_CS1(i, SSP_CTRL1, FIFO_OVERRUN_IRQ, v)
#define BWi_SSP_CTRL1_FIFO_OVERRUN_IRQ_EN(i, v)   BFi_CS1(i, SSP_CTRL1, FIFO_OVERRUN_IRQ_EN, v)
#define BWi_SSP_CTRL1_DMA_ENABLE(i, v)            BFi_CS1(i, SSP_CTRL1, DMA_ENABLE, v)
#define BWi_SSP_CTRL1_CEATA_CCS_ERR_EN(i, v)      BFi_CS1(i, SSP_CTRL1, CEATA_CCS_ERR_EN, v)
#define BWi_SSP_CTRL1_SLAVE_OUT_DISABLE(i, v)     BFi_CS1(i, SSP_CTRL1, SLAVE_OUT_DISABLE, v)
#define BWi_SSP_CTRL1_PHASE(i, v)                 BFi_CS1(i, SSP_CTRL1, PHASE, v)
#define BWi_SSP_CTRL1_POLARITY(i, v)              BFi_CS1(i, SSP_CTRL1, POLARITY, v)
#define BWi_SSP_CTRL1_SLAVE_MODE(i, v)            BFi_CS1(i, SSP_CTRL1, SLAVE_MODE, v)
#define BWi_SSP_CTRL1_WORD_LENGTH(i, v)           BFi_CS1(i, SSP_CTRL1, WORD_LENGTH, v)
#define BWi_SSP_CTRL1_SSP_MODE(i, v)              BFi_CS1(i, SSP_CTRL1, SSP_MODE, v)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_DATA - instance-indexed SSP Data Register
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_DATA_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000070)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_DATA(i)           (*(volatile hw_ssp_data_t *) HWi_SSP_DATA_ADDR(i))
#define HWi_SSP_DATA_RD(i)        (HWi_SSP_DATA(i).U)
#define HWi_SSP_DATA_WR(i, v)     (HWi_SSP_DATA(i).U = (v))
#define HWi_SSP_DATA_SET(i, v)    (HWi_SSP_DATA_WR(i, HWi_SSP_DATA_RD(i) |  (v)))
#define HWi_SSP_DATA_CLR(i, v)    (HWi_SSP_DATA_WR(i, HWi_SSP_DATA_RD(i) & ~(v)))
#define HWi_SSP_DATA_TOG(i, v)    (HWi_SSP_DATA_WR(i, HWi_SSP_DATA_RD(i) ^  (v)))

#define BWi_SSP_DATA_DATA(i, v)   (HWi_SSP_DATA(i).B.DATA = (v))

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_SDRESP0 - instance-indexed SD/MMC Card Response Register 0
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_SDRESP0_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000080)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_SDRESP0(i)           (*(volatile hw_ssp_sdresp0_t *) HWi_SSP_SDRESP0_ADDR(i))
#define HWi_SSP_SDRESP0_RD(i)        (HWi_SSP_SDRESP0(i).U)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_SDRESP1 - instance-indexed SD/MMC Card Response Register 1
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_SDRESP1_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000090)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_SDRESP1(i)           (*(volatile hw_ssp_sdresp1_t *) HWi_SSP_SDRESP1_ADDR(i))
#define HWi_SSP_SDRESP1_RD(i)        (HWi_SSP_SDRESP1(i).U)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_SDRESP2 - instance-indexed SD/MMC Card Response Register 2
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_SDRESP2_ADDR(i)      (REGSi_SSP_BASE(i) + 0x000000A0)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_SDRESP2(i)           (*(volatile hw_ssp_sdresp2_t *) HWi_SSP_SDRESP2_ADDR(i))
#define HWi_SSP_SDRESP2_RD(i)        (HWi_SSP_SDRESP2(i).U)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_SDRESP3 - instance-indexed SD/MMC Card Response Register 3
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_SDRESP3_ADDR(i)      (REGSi_SSP_BASE(i) + 0x000000B0)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_SDRESP3(i)           (*(volatile hw_ssp_sdresp3_t *) HWi_SSP_SDRESP3_ADDR(i))
#define HWi_SSP_SDRESP3_RD(i)        (HWi_SSP_SDRESP3(i).U)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_STATUS - instance-indexed SSP Status Register
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_STATUS_ADDR(i)      (REGSi_SSP_BASE(i) + 0x000000C0)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_STATUS(i)           (*(volatile hw_ssp_status_t *) HWi_SSP_STATUS_ADDR(i))
#define HWi_SSP_STATUS_RD(i)        (HWi_SSP_STATUS(i).U)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_DEBUG - instance-indexed SSP Debug Register
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_DEBUG_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000100)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_DEBUG(i)           (*(volatile hw_ssp_debug_t *) HWi_SSP_DEBUG_ADDR(i))
#define HWi_SSP_DEBUG_RD(i)        (HWi_SSP_DEBUG(i).U)

#endif


////////////////////////////////////////////////////////////////////////////////
//// HWi_SSP_VERSION - instance-indexed SSP Version Register
////////////////////////////////////////////////////////////////////////////////

#define HWi_SSP_VERSION_ADDR(i)      (REGSi_SSP_BASE(i) + 0x00000110)

#ifndef __LANGUAGE_ASM__

#define HWi_SSP_VERSION(i)           (*(volatile hw_ssp_version_t *) HWi_SSP_VERSION_ADDR(i))
#define HWi_SSP_VERSION_RD(i)        (HWi_SSP_VERSION(i).U)

#endif


#endif // _REGSSSP_I_H

////////////////////////////////////////////////////////////////////////////////
