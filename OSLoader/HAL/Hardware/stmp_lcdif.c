#include "board_up.h"
#include "display_up.h"

#include "regsapbh.h"
#include "regsclkctrl.h"
#include "regslcdif.h"
#include "regspinctrl.h"

#include "../debug.h"

#include "board_up.h"
#include "hw_irq.h"
#include "interrupt_up.h"

// Screen Size (8 + {127) * 256}
#define DISPLAY_INVERSE (1)

#define SCREEN_START_Y (8) // 0 - 126
#define SCREEN_END_Y (134)

#define SCREEN_START_X (0)
#define SCREEN_END_X (255 / 3) // 0 - 255

#define SCREEN_WIDTH (256)
#define SCREEN_HEIGHT (127)

typedef struct LCDIF_DMADesc {
    struct LCDIF_DMADesc *pNext;
    union {
        struct
        {
            union {
                struct
                {
                    uint8_t DMA_Command : 2;
                    uint8_t DMA_Chain : 1;
                    uint8_t DMA_IRQOnCompletion : 1;
                    uint8_t DMA_NANDLock : 1;
                    uint8_t DMA_NANDWaitForReady : 1;
                    uint8_t DMA_Semaphore : 1;
                    uint8_t DMA_WaitForEndCommand : 1;
                };
                uint8_t Bits;
            };
            uint8_t Reserved : 4;
            uint8_t DMA_PIOWords : 4;
            uint16_t DMA_XferBytes : 16;
        };
        uint32_t DMA_CommandBits;
    };
    uint32_t pDMABuffer;
    hw_lcdif_ctrl_t PioWord;

} LCDIF_DMADesc;

typedef struct LCDIF_Timing_t {
    unsigned char DataSetup_ns;
    unsigned char DataHold_ns;
    unsigned char CmdSetup_ns;
    unsigned char CmdHold_ns;
    uint32_t minOpaTime_us;
    uint32_t minReadBackTime_us;
} LCDIF_Timing_t;

static struct LCDIF_Timing_t defaultTiming =
    {
        .DataSetup_ns = 50 + 10,
        .DataHold_ns = 0 + 0,
        .CmdSetup_ns = 15 + 10,
        .CmdHold_ns = 15 + 10,
        .minOpaTime_us = 10,
        .minReadBackTime_us = 30};

static uint64_t LCDIFFreq;

static LCDIF_DMADesc chains_wr;
static LCDIF_DMADesc chains_emitIRQ;

static volatile bool opaFinish;

static uint8_t lineBuffer[SCREEN_WIDTH + 4] __aligned(4);

void portDispInterfaceInit() {
    BF_CS8(
        PINCTRL_MUXSEL2,
        BANK1_PIN07, 0,
        BANK1_PIN06, 0,
        BANK1_PIN05, 0,
        BANK1_PIN04, 0,
        BANK1_PIN03, 0,
        BANK1_PIN02, 0,
        BANK1_PIN01, 0,
        BANK1_PIN00, 0);

    BF_CS5(
        PINCTRL_MUXSEL3,
        BANK1_PIN20, 0,
        BANK1_PIN19, 0,
        BANK1_PIN18, 0,
        BANK1_PIN17, 0,
        BANK1_PIN16, 0);

    // LCDIFFreq = 96000000;

    // LCDIFFreq = 480000000 / 10;
    // LCDIFFreq = 480000000UL / 5;
    LCDIFFreq = 24000000;

    // BF_CLR(CLKCTRL_FRAC, CLKGATEPIX);
    // BF_CLR(CLKCTRL_CLKSEQ, BYPASS_PIX); //bypass 24MHz XTAL
    // BF_WR(CLKCTRL_FRAC, PIXFRAC, 18); //PLL Output (480 * (18/18)) MHz

    BF_CLR(CLKCTRL_PIX, CLKGATE);
    // BF_WR(CLKCTRL_PIX, DIV, 5);
}

static bool LCDIF_checkSendFinish() {
    return BF_RD(LCDIF_STAT, TXFIFO_EMPTY);
}

static bool LCDIF_checkReceiveFinish() {

    return BF_RD(LCDIF_STAT, RXFIFO_EMPTY);
}

static bool LCDIF_checkDMAFin() {
    return !((HW_APBH_CHn_DEBUG2(0).B.AHB_BYTES) && (HW_APBH_CHn_DEBUG2(0).B.APB_BYTES));
    // return ((HW_APBH_CHn_SEMA(0).B.INCREMENT_SEMA)==0);
}

static void LCDIF_EnableDMAChannel(bool enable) {
    if (enable) {
        BF_CLRV(APBH_CTRL0, RESET_CHANNEL, 0x1);
    } else {
        BF_SETV(APBH_CTRL0, RESET_CHANNEL, 0x1);
    }
}

static void LCDIF_ResetDMAChannel() {
    BF_SETV(APBH_CTRL0, RESET_CHANNEL, 0x1);
}

static void LCDIF_SetTiming() {

    BF_CS4(LCDIF_TIMING,
           DATA_SETUP, nsToCycles(defaultTiming.DataSetup_ns, 1000000000UL / LCDIFFreq, 1),
           DATA_HOLD, nsToCycles(defaultTiming.DataHold_ns, 1000000000UL / LCDIFFreq, 1),
           CMD_SETUP, nsToCycles(defaultTiming.CmdSetup_ns, 1000000000UL / LCDIFFreq, 1),
           CMD_HOLD, nsToCycles(defaultTiming.CmdHold_ns, 1000000000UL / LCDIFFreq, 1));
}

static void LCDIF_DMAChainsInit() {
    memset(&chains_wr, 0, sizeof(chains_wr));
    memset(&chains_emitIRQ, 0, sizeof(chains_emitIRQ));

    chains_wr.pNext = NULL;
    // chains_wr.pNext = &chains_emitIRQ;
    chains_wr.DMA_Semaphore = 1;
    chains_wr.DMA_Command = BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);
    chains_wr.DMA_Chain = 0;
    chains_wr.DMA_IRQOnCompletion = 1;
    chains_wr.DMA_NANDLock = 0;
    chains_wr.DMA_NANDWaitForReady = 0;
    chains_wr.DMA_PIOWords = 1;
    chains_wr.pDMABuffer = 00000000;
    chains_wr.DMA_XferBytes = 0000;

    chains_wr.PioWord.B.COUNT = 0000;
    chains_wr.PioWord.B.WORD_LENGTH = 1; // 8bit mode
    chains_wr.PioWord.B.DATA_SELECT = 0; // 0:command mode   1:data mode
    chains_wr.PioWord.B.RUN = 1;
    chains_wr.PioWord.B.BYPASS_COUNT = 0;
    chains_wr.PioWord.B.READ_WRITEB = 0; // 0:write mode  1:read mode

    chains_emitIRQ.pNext = NULL;
    chains_emitIRQ.DMA_Semaphore = 1;
    chains_emitIRQ.DMA_Command = BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);
    chains_emitIRQ.DMA_Chain = 0;
    chains_emitIRQ.DMA_IRQOnCompletion = 1;
    chains_emitIRQ.DMA_NANDLock = 0;
    chains_emitIRQ.DMA_NANDWaitForReady = 0;
    chains_emitIRQ.DMA_PIOWords = 0;
    chains_emitIRQ.pDMABuffer = 00000000;
    chains_emitIRQ.DMA_XferBytes = 0000;
}

static void LCDIF_WriteCMD(uint8_t *dat, uint32_t len) {

    // while((portBoardGetTime_us() - lastOpaTime) < defaultTiming.minOpaTime_us)
    //     ;
    // while(!LCDIF_checkReceiveFinish());
    /*
        while(BF_RD(LCDIF_STAT, TXFIFO_EMPTY) == false)
            ;*/

    // while(!opaFinish);

    driverWaitTrueF(LCDIF_checkSendFinish, 1000);
    driverWaitTrueF(LCDIF_checkDMAFin, 1000);

    // while ((HW_APBH_CHn_SEMA(0).B.INCREMENT_SEMA))
    //     ;

    // opaFinish = false;

    chains_wr.DMA_Command = BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);
    chains_wr.PioWord.B.DATA_SELECT = 0; // 0:command mode   1:data mode
    chains_wr.PioWord.B.READ_WRITEB = 0; // 0:write mode     1:read mode
    chains_wr.pDMABuffer = (uint32_t)dat;
    chains_wr.DMA_XferBytes = len;
    chains_wr.PioWord.B.COUNT = len;

    BF_WRn(APBH_CHn_NXTCMDAR, 0, CMD_ADDR, (reg32_t)&chains_wr);
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(0, 1);

    driverWaitTrueF(LCDIF_checkDMAFin, 1000);
    driverWaitTrueF(LCDIF_checkSendFinish, 1000);

    // INFO("wcs\n");
    // portDelayus(20);
    // INFO("wce\n");

    // lastOpaTime = portBoardGetTime_us();
    // INFO("LCDIF WR CMD Fin\n");
}

static void LCDIF_WriteDAT(uint8_t *dat, uint32_t len) {

    // while((portBoardGetTime_us() - lastOpaTime) < defaultTiming.minOpaTime_us)
    //     ;
    // while(!LCDIF_checkReceiveFinish());

    // while(BF_RD(LCDIF_STAT, TXFIFO_EMPTY) == false);
    // while(!opaFinish);
    driverWaitTrueF(LCDIF_checkSendFinish, 1000);
    driverWaitTrueF(LCDIF_checkDMAFin, 1000);
    // while ((HW_APBH_CHn_SEMA(0).B.INCREMENT_SEMA))
    //    ;

    // opaFinish = false;
    chains_wr.DMA_Command = BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);
    chains_wr.PioWord.B.DATA_SELECT = 1; // 0:command mode   1:data mode
    chains_wr.PioWord.B.READ_WRITEB = 0; // 0:write mode     1:read mode
    chains_wr.pDMABuffer = (uint32_t)dat;
    chains_wr.DMA_XferBytes = len;
    chains_wr.PioWord.B.COUNT = len;

    BF_WRn(APBH_CHn_NXTCMDAR, 0, CMD_ADDR, (reg32_t)&chains_wr);
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(0, 1);

    driverWaitTrueF(LCDIF_checkDMAFin, 1000);
    driverWaitTrueF(LCDIF_checkSendFinish, 1000);

    // INFO("wds\n");
    // portDelayus(20);
    // INFO("wde\n");
    // lastOpaTime = portBoardGetTime_us();
    // INFO("LCDIF WR DAT Fin\n");
}

static void LCDIF_ReadDAT(uint8_t *dat, uint32_t len) {
    // while((portBoardGetTime_us() - lastOpaTime) < defaultTiming.minOpaTime_us)
    //     ;
    // while(!LCDIF_checkSendFinish());

    // while(BF_RD(LCDIF_STAT, RXFIFO_EMPTY) == false);

    // while(!opaFinish);

    driverWaitTrueF(LCDIF_checkReceiveFinish, 1000);
    driverWaitTrueF(LCDIF_checkDMAFin, 1000);
    // while ((HW_APBH_CHn_SEMA(0).B.INCREMENT_SEMA))
    //     ;

    // opaFinish = false;
    chains_wr.DMA_Command = BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE);
    chains_wr.PioWord.B.DATA_SELECT = 1; // 0:command mode   1:data mode
    chains_wr.PioWord.B.READ_WRITEB = 1; // 0:write mode     1:read mode
    chains_wr.pDMABuffer = (uint32_t)dat;
    chains_wr.DMA_XferBytes = len;
    chains_wr.PioWord.B.COUNT = len;

    BF_WRn(APBH_CHn_NXTCMDAR, 0, CMD_ADDR, (reg32_t)&chains_wr);
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(0, 1);

    // while ((HW_APBH_CHn_SEMA(0).B.INCREMENT_SEMA))
    //     ;

    // while(!LCDIF_checkReceiveFinish());

    driverWaitTrueF(LCDIF_checkReceiveFinish, 10000);
    driverWaitTrueF(LCDIF_checkDMAFin, 10000);

    // portDelayus(500);

    // while(HW_APBH_CHn_DEBUG2(0).B.AHB_BYTES);
    // while(HW_APBH_CHn_DEBUG2(0).B.APB_BYTES);

    // INFO("rds\n");

    //

    // INFO("rde\n");
    // portDelayus(defaultTiming.minReadBackTime_us);

    // lastOpaTime = portBoardGetTime_us();
    // INFO("LCDIF RD DAT Fin\n");
}

#define LCDIF_CMD8(x)           \
    do {                        \
        uint8_t _a = (x);       \
        LCDIF_WriteCMD(&_a, 1); \
    } while (0)
#define LCDIF_DAT8(x)           \
    do {                        \
        uint8_t _a = (x);       \
        LCDIF_WriteDAT(&_a, 1); \
    } while (0)
#define LCDIF_DAT32(x)                     \
    do {                                   \
        uint32_t _a = (x);                 \
        LCDIF_WriteDAT((uint8_t *)&_a, 4); \
    } while (0)

#define BigEnd16(x) ((((x & 0xFFFF) << 8) | ((x & 0xFFFF) >> 8)))

void portDISP_ISR() {
    if (!BF_RD(APBH_CTRL1, CH0_CMDCMPLT_IRQ)) {
        INFO("LCDIF ERR IRQ, Overflow:%d, Underflow:%d\n",
             BF_RD(LCDIF_CTRL1, OVERFLOW_IRQ),
             BF_RD(LCDIF_CTRL1, UNDERFLOW_IRQ));
        BF_CLR(LCDIF_CTRL1, OVERFLOW_IRQ);
        BF_CLR(LCDIF_CTRL1, UNDERFLOW_IRQ);
    }

    BF_CLR(APBH_CTRL1, CH0_CMDCMPLT_IRQ);

    opaFinish = true;
}

void portDispClean() {
    uint32_t zeros[8];

    uint16_t start_x = SCREEN_START_X;
    uint16_t start_y = 0;
    uint16_t end_x = SCREEN_END_X;
    uint16_t end_y = SCREEN_END_Y;

    LCDIF_CMD8(0x2A);
    LCDIF_DAT32(BigEnd16(start_x) | BigEnd16(end_x) << 16);
    LCDIF_CMD8(0x2B);
    LCDIF_DAT32(BigEnd16(start_y) | BigEnd16(end_y) << 16);
    LCDIF_CMD8(0x2C);

    for (int i = 0; i < sizeof(zeros) / sizeof(uint32_t); i++) {
        zeros[i] = DISPLAY_INVERSE ? 0xFFFFFFFF : 0x00000000;
    }

    for (int i = 0; i < (((end_x - start_x + 1) * (end_y - start_y + 1)) * 3); i += sizeof(zeros)) {
        LCDIF_WriteDAT((uint8_t *)zeros, sizeof(zeros));
    }
}

int save_bat = 0;
int save_ind_bit = 0;
void portDispSetIndicate(int indicateBit, int batteryBit) {
    uint32_t sx, sy, ex, ey;
    uint8_t setbit, setBat;
    sx = 0;
    ex = 86;
    sy = 0;
    ey = 24;

    if (indicateBit == -1) {
        setbit = save_ind_bit;
    } else {
        setbit = indicateBit;
        save_ind_bit = indicateBit;
    }

    if (batteryBit == -1) {
        setBat = save_bat;
    } else {
        save_bat = batteryBit;
        setBat = batteryBit;
    }

    LCDIF_CMD8(0x2A);
    LCDIF_DAT32(BigEnd16(sx) | BigEnd16(ex) << 16);

    LCDIF_CMD8(0x2B);
    LCDIF_DAT32(BigEnd16(sy) | BigEnd16(ey) << 16);

    LCDIF_CMD8(0x2C);

    for (int y = sy; y < ey; y++) {
        for (int x = sx; x < ex; x++) {

            switch (x) {
            case 84: // Battery Box
                LCDIF_DAT8(0);
                break;
            case 76: // Battery 1st Indication
                LCDIF_DAT8((setBat >> 0) & 1 ? 0 : 0xFF);
                break;
            case 75: // Battery 3rd Indication
                LCDIF_DAT8((setBat >> 2) & 1 ? 0 : 0xFF);
                break;
            case 77: // Battery 2rd Indication
                LCDIF_DAT8((setBat >> 1) & 1 ? 0 : 0xFF);
                break;
            case 78: // Battery 4th Indication
                LCDIF_DAT8((setBat >> 3) & 1 ? 0 : 0xFF);
                break;
            case 10: // A..Z
                LCDIF_DAT8((setbit >> 2) & 1 ? 0 : 0xFF);
                break;
            case 28: // TX
                LCDIF_DAT8((setbit >> 5) & 1 ? 0 : 0xFF);
                break;
            case 37: // Left
                LCDIF_DAT8((setbit >> 0) & 1 ? 0 : 0xFF);
                break;
            case 44: // a..z
                LCDIF_DAT8((setbit >> 3) & 1 ? 0 : 0xFF);
                break;
            case 50: // RX
                LCDIF_DAT8((setbit >> 6) & 1 ? 0 : 0xFF);
                break;
            case 64: // Right
                LCDIF_DAT8((setbit >> 1) & 1 ? 0 : 0xFF);
                break;
            case 82: // Busy
                LCDIF_DAT8((setbit >> 4) & 1 ? 0 : 0xFF);
                break;

            default:
                LCDIF_DAT8(0xFF);
            }
        }
    }
}

void portDispReadBackVRAM(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf) {
    uint32_t xstart = x_start;
    uint32_t xend = x_end;
    uint32_t ystart = (y_start + SCREEN_START_Y);
    uint32_t yend = (y_end + SCREEN_START_Y);

    if ((xstart > xend) || (ystart > yend)) {
        return;
    }
    uint32_t p = 0;
    for (uint32_t line_i = ystart; line_i <= yend; line_i++) {

        LCDIF_CMD8(0x2A);
        LCDIF_DAT32(BigEnd16(xstart / 3) | (BigEnd16(xend / 3) << 16));
        LCDIF_CMD8(0x2B);
        LCDIF_DAT32(BigEnd16(line_i) | (BigEnd16(line_i) << 16));
        LCDIF_CMD8(0x2E);
        LCDIF_ReadDAT(lineBuffer, ((xend / 3) - (xstart / 3) + 1) * 3);

        memcpy(&buf[p], &lineBuffer[xstart % 3], xend - xstart + 1);
        p += xend - xstart + 1;
    }
}
uint32_t dummy;
void portDispFlushAreaBuf(uint32_t x_start, uint32_t y_start, uint32_t x_end, uint32_t y_end, uint8_t *buf) {

    uint32_t xstart = x_start;
    uint32_t xend = x_end;
    uint32_t ystart = (y_start + SCREEN_START_Y);
    uint32_t yend = (y_end + SCREEN_START_Y);

    if ((xstart > xend) || (ystart > yend)) {
        return;
    }

    if (
        (x_start > SCREEN_WIDTH + 1) ||
        (x_end > SCREEN_WIDTH + 1) ||
        (y_start > SCREEN_END_Y + 0) ||
        (y_end > SCREEN_END_Y + 0)

    ) {
        return;
    }

    LCDIF_CMD8(0x2A);
    LCDIF_DAT32(BigEnd16(xstart / 3) | (BigEnd16(xend / 3) << 16));

    uint32_t p = 0;
    lineBuffer[sizeof(lineBuffer) - 1] = 0x5A;
    for (uint32_t line_i = ystart; line_i <= yend; line_i++) {

        LCDIF_CMD8(0x2B);
        LCDIF_DAT32(BigEnd16(line_i) | ((BigEnd16(line_i) << 16)));

        if (xstart % 3) {
            LCDIF_CMD8(0x2E);
            LCDIF_ReadDAT(lineBuffer, 3);
        }
        memcpy(&lineBuffer[xstart % 3], &buf[p], xend - xstart + 1);
        p += xend - xstart + 1;

        LCDIF_CMD8(0x2C);

        LCDIF_WriteDAT(lineBuffer, xend - xstart + 1);
    }
    if (lineBuffer[sizeof(lineBuffer) - 1] != 0x5A) {
        INFO("LineBuffer Error.\n");
    }
    // INFO("ed\n");
}

void DisplayPrepareBatchIn(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) {
    LCDIF_CMD8(0x2A);
    LCDIF_DAT32(BigEnd16(x0 / 3) | (BigEnd16(x1 / 3) << 16));
    LCDIF_CMD8(0x2B);
    LCDIF_DAT32(BigEnd16(y0) | (BigEnd16(y1) << 16));
}

void DisplayBatchIn(uint8_t *dat, uint32_t len) {
    LCDIF_CMD8(0x2C);
    LCDIF_WriteDAT(dat, len);
}

void portDispSetContrast(uint8_t contrast) {
    LCDIF_CMD8(0x25);
    LCDIF_DAT8(contrast & 0x7F);
}

void portDispDeviceInit() {
    LCDIF_DMAChainsInit();
    LCDIF_EnableDMAChannel(true);
    LCDIF_ResetDMAChannel();
    LCDIF_SetTiming();

    portEnableIRQ(HW_IRQ_LCDIF_DMA, true);
    portEnableIRQ(HW_IRQ_LCDIF_ERROR, true);
    BF_CS1(APBH_CTRL1, CH0_CMDCMPLT_IRQ_EN, 1);
    ;

    BF_CLR(LCDIF_CTRL1, MODE86);
    BF_CLR(LCDIF_CTRL1, LCD_CS_CTRL);

    BF_SET(LCDIF_CTRL1, OVERFLOW_IRQ_EN);
    BF_SET(LCDIF_CTRL1, UNDERFLOW_IRQ_EN);

    BF_SET(LCDIF_CTRL1, BUSY_ENABLE);

    BF_CS1(LCDIF_CTRL1, BYTE_PACKING_FORMAT, 0xF);
    BF_CS1(LCDIF_CTRL1, READ_MODE_NUM_PACKED_SUBWORDS, 4);
    BF_CS1(LCDIF_CTRL1, FIRST_READ_DUMMY, 1);

    BF_CLR(LCDIF_CTRL1, RESET);
    portDelayus(120000);
    BF_SET(LCDIF_CTRL1, RESET);

    opaFinish = true;

    LCDIF_CMD8(0xD7); // Auto Load Set
    LCDIF_DAT8(0x9F);
    LCDIF_CMD8(0xE0); // EE Read/write mode
    LCDIF_DAT8(0x00); // Set read mode

    portDelayus(100000);

    LCDIF_CMD8(0xE3); // Read active

    portDelayus(100000);

    LCDIF_CMD8(0xE1); // Cancel control

    LCDIF_CMD8(0x11); // sleep out

    portDelayus(500000);

    LCDIF_CMD8(0x28); // Display off

    LCDIF_CMD8(0xC0); // Set Vop by initial Module
    LCDIF_DAT8(0x01);
    LCDIF_DAT8(0x01);

    // LCDIF_CMD8(0xF0); // Set Frame Rate
    // LCDIF_DAT32(0x0D0D0D0D);

    LCDIF_CMD8(0xC3); // Bias select
    LCDIF_DAT8(0x02);

    LCDIF_CMD8(0xC4); // Setting Booster times
    LCDIF_DAT8(0x07);

    LCDIF_CMD8(0xC5); // Booster Efficiency selection
    LCDIF_DAT8(0x02);

    LCDIF_CMD8(0xD0); // Analog circuit setting
    LCDIF_DAT8(0x1D);

    LCDIF_CMD8(0xB5); // N-Line
    LCDIF_DAT8(0x8C);
    LCDIF_DAT8(0x00);

    LCDIF_CMD8(0x38); // Idle mode off

    LCDIF_CMD8(0x3A); // pix format
    LCDIF_DAT8(7);

    LCDIF_CMD8(0x36); // Memory Access Control
    LCDIF_DAT8(0x48);

    LCDIF_CMD8(0xB0); // Set Duty
    LCDIF_DAT8(0x87);

    LCDIF_CMD8(0xB4); // Partial Saving Power Mode Selection
    LCDIF_DAT8(0xA0);

    LCDIF_CMD8(0x29); // Display on

    if (DISPLAY_INVERSE)
        LCDIF_CMD8(0x21); // Display inversion on

    uint8_t ID[3];

    LCDIF_CMD8(0xDA);
    LCDIF_ReadDAT(&ID[0], 1);

    LCDIF_CMD8(0xDB);
    LCDIF_ReadDAT(&ID[1], 1);

    LCDIF_CMD8(0xDC);
    LCDIF_ReadDAT(&ID[2], 1);

    for (int i = 0; i < 3; i++) {
        printf("LCD ID[%d]:%02x\n", i, ID[i]);
    }

    extern uint32_t g_lcd_contrast;
    portDispSetContrast(g_lcd_contrast);

    portDispClean();
}
