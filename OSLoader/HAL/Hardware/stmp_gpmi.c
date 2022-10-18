#include "mtd_up.h"
#include "interrupt_up.h"

#include "regsclkctrl.h"
#include "regsapbh.h"
#include "regsgpmi.h"
#include "regsecc8.h"
#include "regsdigctl.h"


#include "hw_irq.h"

#include "../debug.h"

#include "board_up.h"

#define CHIP_SEL  0
#define NAND_DMA_Channel  4

#define MASK_AND_REFERENCE_VALUE 0x0100

#define NAND_CMD_READ0      0
#define NAND_CMD_READSTART  0x30

#define NAND_CMD_ERASE1 0x60
#define NAND_CMD_ERASE2 0xd0

#define NAND_CMD_SEQIN 0x80
#define NAND_CMD_PAGEPROG 0x10

#define NAND_CMD_STATUS 0x70


typedef struct GPMI_DMA_Desc
{
    // DMA related fields
    uint32_t dma_nxtcmdar;
    uint32_t dma_cmd;
    uint32_t dma_bar;
    // GPMI related fields
    uint32_t gpmi_ctrl0;    //PIO 0
    uint32_t gpmi_compare;  //PIO 1
    uint32_t gpmi_eccctrl;  //PIO 2
    uint32_t gpmi_ecccount; //PIO 3
    uint32_t gpmi_data_ptr; //PIO 4
    uint32_t gpmi_aux_ptr;  //PIO 5
} GPMI_DMA_Desc;

typedef struct GPMI_Timing_t {
    uint32_t tPROG_us;
    uint32_t tBERS_us;
    uint32_t tREAD_us;
    unsigned char DataSetup_ns;
    unsigned char DataHold_ns;
    unsigned char AddressSetup_ns;
    float SampleDelay_cyc;
} GPMI_Timing_t;

static struct GPMI_Timing_t defaultTiming =
{
        .DataSetup_ns = 6 + 2,
        .DataHold_ns = 5 + 2,
        .AddressSetup_ns = 6 + 2,
        .SampleDelay_cyc = 3.0,
        .tREAD_us = 7,
        .tPROG_us = 15,
        .tBERS_us = 2000
};

typedef enum {
    GPMI_OPA_NONE,
    GPMI_OPA_WRITE,
    GPMI_OPA_READ,
    GPMI_OPA_ERASE,
    GPMI_OPA_COPY
}GPMI_Operation;

static uint64_t GPMIFreq;
static uint64_t DeviceTimeOutCycles;

static volatile GPMI_DMA_Desc chains_cmd[4];
static volatile GPMI_DMA_Desc chains_read[8 + 1];
static volatile GPMI_DMA_Desc chains_write[10 + 1];
static volatile GPMI_DMA_Desc chains_erase[8 + 1];


static uint32_t FlashSendCommandBuffer[8];
static uint32_t FlashSendParaBuffer[4];
static uint32_t FlashRecCommandBuffer[4];
static uint32_t  ECCResult;

static uint32_t GPMI_DataBuffer[ 2048 / 4 ] __attribute__((aligned(4)));
static uint32_t GPMI_AuxiliaryBuffer[ 512 / 4 ] __attribute__((aligned(4)));

static uint32_t ReserveBlock[32];


static volatile bool ECC_FIN;

static volatile GPMI_Operation GPMI_curOpa;
static volatile uint32_t GPMI_CopyState = 0;
static uint32_t CopyECCResult;

static uint32_t LastProgTime = 0;
static uint32_t LastEraseTime = 0;
static uint32_t LastReadTime = 0;
static uint32_t LastOpa = 0;
//#define PR_NAND_WR_TIMING_STATUS
#ifdef PR_NAND_WR_TIMING_STATUS
static uint32_t pgwdt;
static uint32_t pgrdt;
#endif
static void GPMI_EnableDMAChannel(bool enable)
{
    if(enable){
        BF_CLRV(APBH_CTRL0, CLKGATE_CHANNEL, 0x10);
    }else{
        BF_SETV(APBH_CTRL0, CLKGATE_CHANNEL, 0x10);
    }
}

static void GPMI_ResetDMAChannel()
{
    BF_SETV(APBH_CTRL0, RESET_CHANNEL, 0x10);
}

static void GPMI_SetAccessTiming(GPMI_Timing_t timing)
{

    uint32_t dh,ds,as;
    DeviceTimeOutCycles = nsToCycles(80000000, 1000000000ULL / (GPMIFreq / 4096ULL), 0);  //80ms
    //DeviceTimeOutCycles = 0xFFFF;
    ds = nsToCycles(timing.DataSetup_ns, 1000000000ULL / GPMIFreq, 1);
    dh = nsToCycles(timing.DataHold_ns, 1000000000ULL / GPMIFreq, 1);
    as = nsToCycles(timing.DataSetup_ns, 1000000000ULL / GPMIFreq, 0);
    INFO("DATA_SETUP:%ld\n", ds);
    INFO("DATA_HOLD:%ld\n", dh);
    INFO("ADDRESS_SETUP:%ld\n", as);
    
    BF_CS3(
        GPMI_TIMING0,
        DATA_SETUP, ds,
        DATA_HOLD, dh,
        ADDRESS_SETUP, as
    );
        
    BF_CS1(
        GPMI_TIMING1,
        DEVICE_BUSY_TIMEOUT, DeviceTimeOutCycles);

    BF_CS2(
        GPMI_CTRL1,
        DSAMPLE_TIME, (uint32_t)(timing.SampleDelay_cyc * 2),
        BURST_EN, 1);

}

static void GPMI_ClockConfigure()
{
    BF_CLR(CLKCTRL_GPMI, CLKGATE);
    BF_CLR(CLKCTRL_FRAC, CLKGATEIO);

    HW_CLKCTRL_GPMI_WR(BF_CLKCTRL_GPMI_DIV(2));    // 480 / 2 MHz
    INFO("GPMI CLK DIV:%d\n", BF_RD(CLKCTRL_GPMI, DIV));
    //BF_CS1(CLKCTRL_GPMI, DIV, 2);

    BF_CLR(CLKCTRL_CLKSEQ, BYPASS_GPMI);    //Set TO HF

    GPMIFreq = 480000000ULL / 2ULL;


}

static void GPMI_DMAChainsInit()
{
    // ============================================Command Chains================================================
    //  Phase 1: Wait for ready;
    chains_cmd[0].dma_cmd       =   BF_APBH_CHn_CMD_CMDWORDS(1)         |          // 发送1个PIO命令到GPMI控制器
                                    BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |          // 完成当前命令之后再继续执行
                                    BF_APBH_CHn_CMD_NANDWAIT4READY(1)   |          // 等待NAND就绪后开始执行
                                    BF_APBH_CHn_CMD_NANDLOCK(0)         |          // 锁住NAND防止被其它DMA通道占用
                                    BF_APBH_CHn_CMD_CHAIN(1)            |          // 还有剩下的描述符链
                                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

    chains_cmd[0].dma_bar       =   0;

    chains_cmd[0].gpmi_ctrl0    =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)            | // 当前模式：等待NAND就绪
                                    BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(0)             |
                                    BF_GPMI_CTRL0_WORD_LENGTH(BV_GPMI_CTRL0_WORD_LENGTH__8_BIT) | // 8bit总线模式
                                    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)                      | // 数据模式
                                    BF_GPMI_CTRL0_CS(CHIP_SEL);                                   // 片选

    chains_cmd[0].dma_nxtcmdar  =   (uint32_t)&chains_cmd[1];

    //  Phase 2: Send command and address; Lock the nand flash
    chains_cmd[1].dma_cmd       =   BF_APBH_CHn_CMD_XFER_COUNT(1 + 00000000)    | // 1字节命令 和剩下的地址数据
                                    BF_APBH_CHn_CMD_CMDWORDS(3)                 | // 发送3个PIO命令到GPMI控制器
                                    BF_APBH_CHn_CMD_WAIT4ENDCMD(1)              | // 等待NAND就绪后开始执行
                                    BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                    BF_APBH_CHn_CMD_NANDLOCK(1)                 |
                                    BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                    BF_APBH_CHn_CMD_CHAIN(1)                    |
                                    BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);      // 从内存读取，发送到NAND

    chains_cmd[1].dma_bar       =   00000000;

    chains_cmd[1].gpmi_ctrl0    =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)     | // 写入NAND
                                    BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                    BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)       |
                                    BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(0)             |
                                    BF_GPMI_CTRL0_CS(CHIP_SEL)                  |
                                    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)       |
                                    BF_GPMI_CTRL0_ADDRESS_INCREMENT(1)          | // 1 byte CMD before multi-byte ADDRESS
                                    BF_GPMI_CTRL0_XFER_COUNT(1 + 00000000);
    
    chains_cmd[1].gpmi_compare = 0;
    chains_cmd[1].gpmi_eccctrl = 0;

    chains_cmd[1].dma_nxtcmdar  =   (uint32_t)&chains_cmd[2];   //
    //  Phase 3: Readback command result;    
    chains_cmd[2].dma_cmd       =   BF_APBH_CHn_CMD_XFER_COUNT(00000000)        |  //Readback bytes
                                    BF_APBH_CHn_CMD_CMDWORDS(3)                 |
                                    BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |
                                    BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                    BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                    BF_APBH_CHn_CMD_NANDLOCK(0)                 |
                                    BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                    BF_APBH_CHn_CMD_CHAIN(1)                    |
                                    BV_FLD(APBH_CHn_CMD, COMMAND, DMA_WRITE);

    chains_cmd[2].dma_bar       =   00000000;    //readback address

    chains_cmd[2].gpmi_ctrl0    =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ)      |
                                    BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                    BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)       |
                                    BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(0)             |
                                    BF_GPMI_CTRL0_CS(CHIP_SEL)                  |
                                    BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)      |
                                    BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)          |
                                    BF_GPMI_CTRL0_XFER_COUNT(00000000);             //Readback bytes

    chains_cmd[2].gpmi_compare = 0;
    chains_cmd[2].gpmi_eccctrl = 0;

    chains_cmd[2].dma_nxtcmdar = (reg32_t)&chains_cmd[3];

    //  Phase 4: Terminate;  

    chains_cmd[3].dma_cmd      =    BF_APBH_CHn_CMD_IRQONCMPLT(1)                   |
                                    BF_APBH_CHn_CMD_WAIT4ENDCMD(1)                  |
                                    BF_APBH_CHn_CMD_SEMAPHORE(1)                    |  
                                    BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

    chains_cmd[3].dma_bar = 0;
    chains_cmd[3].dma_nxtcmdar = 0;

    // ============================================Read Chains================================================
    //  Phase 1: issue NAND read setup command (CLE/ALE);
    chains_read[0].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(1 + 4)   |       // point to the next descriptor
                                        BF_APBH_CHn_CMD_CMDWORDS(3)         |       // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |       
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |       
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |       
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |       
                                        BF_APBH_CHn_CMD_CHAIN(1)            |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);    // read data from DMA, write to NAND

    chains_read[0].dma_bar          =   (uint32_t)FlashSendCommandBuffer;

    chains_read[0].gpmi_ctrl0       =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)  |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)    |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)              |
                                        BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(1)             |
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)   |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(1)      |
                                        BF_GPMI_CTRL0_XFER_COUNT(1 + 4);            

    chains_read[0].gpmi_compare     =   0;

    chains_read[0].gpmi_eccctrl     =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE); // disable the ECC

    chains_read[0].dma_nxtcmdar     =   (uint32_t)&chains_read[1];

    //  Phase 2: issue NAND read execute command (CLE);
    
    chains_read[1].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(1)       |       // 1 byte read command
                                        BF_APBH_CHn_CMD_CMDWORDS(1)         |       // send 1 word to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |       
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |       
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |       // prevent other DMA channels from taking over
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |       
                                        BF_APBH_CHn_CMD_CHAIN(1)            |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);    // read data from DMA, write to NAND

    chains_read[1].dma_bar          =   ((uint32_t)FlashSendCommandBuffer) + 5;     // point to byte 6, read execute command

    chains_read[1].gpmi_ctrl0       =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)     |   // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)       |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                  |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)       |
                                        BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(1)             |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)          |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);                      // 1 byte command

    chains_read[1].dma_nxtcmdar     =   (uint32_t)&chains_read[2];                // point to the next descriptor


    //  Phase 3: wait for ready (DATA);
    chains_read[2].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(0)       |       // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(1)         |       // send 1 word to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |       
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(1)   |       // wait for nand to be ready
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |       // relinquish nand lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |       
                                        BF_APBH_CHn_CMD_CHAIN(1)            |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER); // no dma transfer

    chains_read[2].dma_bar          =   0; // field not used
            // 1 word sent to the GPMI
    chains_read[2].gpmi_ctrl0       =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)    | // wait for NAND ready
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)               |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          | // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)              |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        BF_GPMI_CTRL0_TIMEOUT_IRQ_EN(1)             |
                                        BF_GPMI_CTRL0_XFER_COUNT(0);

    chains_read[2].dma_nxtcmdar     =   (uint32_t)&chains_read[3];    // point to the next descriptor


    //  Phase 4: psense compare (time out check);
    chains_read[3].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(0)               |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)                 |   // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |   // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)                 |
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                        BF_APBH_CHn_CMD_CHAIN(1)                    |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE);       // perform a sense check

    chains_read[3].dma_bar          =   (uint32_t)&chains_read[8];                      // if sense check fails, branch to error handler

    chains_read[3].dma_nxtcmdar     =   (uint32_t)&chains_read[4];                      // point to the next descriptor



    //  Phase 5: read 2K page plus 19 byte meta-data Nand data and send it to ECC block (DATA);
    chains_read[4].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(0)       |           // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(6)         |           // send 6 words to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |           // wait for command to finish beforecontinuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |           // prevent other DMA channels from taking over
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |           // ECC block generates ecc8 interrupt on completion
                                        BF_APBH_CHn_CMD_CHAIN(1)            |           // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);     // no DMA transfer, ECC block handlestransfer

    chains_read[4].dma_bar          =   0;                                              // field not used
    // 6 words sent to the GPMI
    chains_read[4].gpmi_ctrl0       =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ)      |   // read from the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)       |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                  |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)      |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)          |
                                        BF_GPMI_CTRL0_XFER_COUNT(4 * (512 + 9) + (19 + 9)); 
                                        // 2K PAGE SIZE four 512 byte data blocks (plusparity, t = 4) 
                                        // and one 19 byte aux block (plusparity, t = 4)

    chains_read[4].gpmi_compare     =   0; // field not used but necessary to seteccctrl

    // GPMI ECCCTRL PIO This launches the 2K byte transfer through ECC8’s
    // bus master. Setting the ECC_ENABLE bit redirects the data flow
    // within the GPMI so that read data flows to the ECC8 engine instead
    // of flowing to the GPMI’s DMA channel.

    chains_read[4].gpmi_eccctrl     =   BV_FLD(GPMI_ECCCTRL, ECC_CMD, DECODE_4_BIT) |   // specify t = 4 mode
                                        BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, ENABLE)    |   // enable ECC module
                                        BF_GPMI_ECCCTRL_BUFFER_MASK(0X10F);             // read all 4 data blocks and 1 aux block
    
    chains_read[4].gpmi_ecccount    =   BF_GPMI_ECCCOUNT_COUNT(4 * (512 + 9) + (19 + 9)); 
                                        // 2K PAGE SIZE specify number of bytes read from NAND

    chains_read[4].gpmi_aux_ptr     =   (uint32_t)GPMI_AuxiliaryBuffer; 
                                        // pointer for the 19 byte aux area + parity and syndrome bytes 
	                                    // for both data and aux blocks.

    chains_read[4].gpmi_data_ptr    =   00000000;  

    chains_read[4].dma_nxtcmdar     =   (uint32_t)&chains_read[5];                  // point to the next descriptor

    //  Phase 6: disable ECC block;
    chains_read[5].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(0)               |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(3)                 |   // send 3 words to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)              |   // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(1)           |   // wait for nand to be ready
                                        BF_APBH_CHn_CMD_NANDLOCK(1)                 |   // need nand lock to be thread safe while turnoff ECC8
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                        BF_APBH_CHn_CMD_CHAIN(1)                    |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);     // no dma transfer
    chains_read[5].dma_bar          =   0;                                              // field not used
    
    chains_read[5].gpmi_ctrl0       =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)    |
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)               |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          | // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)              |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        
                                        BF_GPMI_CTRL0_XFER_COUNT(0);

    chains_read[5].gpmi_compare     =   0;                                          // field not used but necessary to set eccctrl
    chains_read[5].gpmi_eccctrl     =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);  // disable the ECC block

    chains_read[5].dma_nxtcmdar     =   (uint32_t)&chains_read[6];                      // point to the next descriptor

    
    //  Phase 7: deassert nand lock;
    chains_read[6].dma_cmd          =   BF_APBH_CHn_CMD_XFER_COUNT(0)               |       // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)                 |       // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)              |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)                 |       // relinquish nand lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)               |       // ECC8 engine generates interrupt
                                        BF_APBH_CHn_CMD_CHAIN(1)                    |       // terminate DMA chain processing
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);         // no dma transfer
    chains_read[6].dma_bar          =   0;                                  // field not used
    chains_read[6].dma_nxtcmdar     =   (uint32_t)&chains_read[7];                      


    //  Phase 8: Terminate;
    chains_read[7].dma_cmd          =   BF_APBH_CHn_CMD_IRQONCMPLT(1)               |
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |
                                        BF_APBH_CHn_CMD_SEMAPHORE(1)                |
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

    chains_read[7].dma_bar          =   0;
    chains_read[7].dma_nxtcmdar     =   0;

    //  ERROR Brunch;
    chains_read[8].dma_cmd          =   BF_APBH_CHn_CMD_IRQONCMPLT(1)               |
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |
                                        BF_APBH_CHn_CMD_SEMAPHORE(1)                |
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

    chains_read[8].dma_bar          =   0;
    chains_read[8].dma_nxtcmdar     =   0;




    // ============================================Write Chains================================================
    //  Phase 1: issue NAND write setup command (CLE/ALE);

    chains_write[0].dma_nxtcmdar    =   (uint32_t)&chains_write[1];
    chains_write[0].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(1 + 4)       |       // 1 byte command, 4 byte address
                                        BF_APBH_CHn_CMD_CMDWORDS(3)             |       // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)          |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)            |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)       |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)             |       // prevent other DMA channels from taking over
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)           |
                                        BF_APBH_CHn_CMD_CHAIN(1)                |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);        // read data from DMA, write to NAND

    chains_write[0].dma_bar         =   (uint32_t)FlashSendCommandBuffer;               // byte 0 write setup, bytes 1 - 4 NAND address
    chains_write[0].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |       // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)  |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)    |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)              |       // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)   |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(1)      |       // send command and address
                                        BF_GPMI_CTRL0_XFER_COUNT(1 + 4);                // 1 byte command, 4 byte address

    chains_write[0].gpmi_compare    =   0;                             // field not used but necessary to set eccctrl
    chains_write[0].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);      // disable the ECC block


    //  Phase 2: write the data payload (DATA)
    
    chains_write[1].dma_nxtcmdar    =   (uint32_t)&chains_write[2];                 // point to the next descriptor
    chains_write[1].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(4 * 512)     |       // NOTE: DMA transfer only the data payload
                                        BF_APBH_CHn_CMD_CMDWORDS(4)             |       // send 4 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)          |       // DON’T wait to end, wait in the next descriptor
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)            |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)       |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)             |       // maintain resource lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)           |
                                        BF_APBH_CHn_CMD_CHAIN(1)                |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);        // read data from DMA, write to NAND

    chains_write[1].dma_bar         =   00000000;                               //DATA               

    chains_write[1].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE) |       // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)  |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)    |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)              |       // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)  |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)      |
                                        BF_GPMI_CTRL0_XFER_COUNT(4 * 512 + 19);         
                                        // NOTE: this field contains the total amount                      
                                        // DMA transferred (4 data and 1 aux blocks)
                                        // to GPMI
    chains_write[1].gpmi_compare    =   0;                                              // field not used but necessary to set eccctrl
    chains_write[1].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ECC_CMD, ENCODE_4_BIT) |   // specify t = 4 mode
                                        BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, ENABLE)    |   // enable ECC module
                                        BF_GPMI_ECCCTRL_BUFFER_MASK(0x10F);             // write all 8 data blocks and 1 aux block
                                                                                        
                                                                                        
    chains_write[1].gpmi_ecccount   =   BF_GPMI_ECCCOUNT_COUNT(4 * (512 + 9) + (19 + 9)); // specify number of bytes written to NAND
    // NOTE: the extra 4*(9)+9 bytes are parity
    // bytes generated by the ECC block.



    // Phase 3: write the aux payload (DATA)
    
    chains_write[2].dma_nxtcmdar    =   (uint32_t)&chains_write[3];             // point to the next descriptor
    chains_write[2].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(19)      |       // NOTE: DMA transfer only the aux block
                                        BF_APBH_CHn_CMD_CMDWORDS(0)         |       // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |       // maintain resource lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |
                                        BF_APBH_CHn_CMD_CHAIN(1)            |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);    // read data from DMA, write to NAND

    chains_write[2].dma_bar         =   00000000;                               //PAYLOAD DATA   

    // Phase 4: issue NAND write execute command (CLE)
    
    chains_write[3].dma_nxtcmdar    =   (uint32_t)&chains_write[4];                 // point to the next descriptor
    chains_write[3].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(1)       |           // 1 byte command
                                        BF_APBH_CHn_CMD_CMDWORDS(3)         |           // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |           // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |   
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |   
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |           // maintain resource lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |   
                                        BF_APBH_CHn_CMD_CHAIN(1)            |           // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);        // read data from DMA, write to NAND
    chains_write[3].dma_bar         =   ((uint32_t)FlashSendCommandBuffer) + 5;         // point to byte 6, write execute command
                                                                                        // 3 words sent to the GPMI

    chains_write[3].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)     |   // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)        |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                  |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)       |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)          |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);                    // 1 byte command

    chains_write[3].gpmi_compare    =   0;                                              // field not used but necessary to set eccctrl
    chains_write[3].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);      // disable the ECC block


    // Phase 5: wait for ready (CLE);
    chains_write[4].dma_nxtcmdar    =   (uint32_t)&chains_write[5];                 // point to the next descriptor
    chains_write[4].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)           |       // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(1)             |       // send 1 word to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)          |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)            |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(1)       |       // wait for nand to be ready
                                        BF_APBH_CHn_CMD_NANDLOCK(0)             |       // relinquish nand lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)           |
                                        BF_APBH_CHn_CMD_CHAIN(1)                |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);     // no dma transfer
    chains_write[4].dma_bar         =   0;                         // field not used

    // 1 word sent to the GPMI
    chains_write[4].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)    | // wait for NAND ready
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)               |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          | // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)              |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        BF_GPMI_CTRL0_XFER_COUNT(0);


    // Phase 6: psense compare (time out check)
    chains_write[5].dma_nxtcmdar    =   (uint32_t)&chains_write[6];                     // point to the next descriptor
    chains_write[5].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)       |           // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)         |           // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)      |           // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)         |
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |
                                        BF_APBH_CHn_CMD_CHAIN(1)            |           // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE);       // perform a sense check
    chains_write[5].dma_bar = (uint32_t)&chains_write[10];          // if sense check fails, branch to error handler
    

    // Phase 7: issue NAND status command (CLE)
    chains_write[6].dma_nxtcmdar    =   (uint32_t)&chains_write[7];                 // point to the next descriptor
    chains_write[6].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(1)       |       // 1 byte command
                                        BF_APBH_CHn_CMD_CMDWORDS(3)         |       // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |       // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |   
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |   
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |       // prevent other DMA channels from taking over
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |   
                                        BF_APBH_CHn_CMD_CHAIN(1)            |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);    // read data from DMA, write to NAND

    chains_write[6].dma_bar = ((uint32_t)FlashSendCommandBuffer) + 6; // point to byte 6, status command
    chains_write[6].gpmi_compare    =   0;                                              // field not used but necessary to set eccctrl
    chains_write[6].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);      // disable the ECC block
                                                                                        // 3 words sent to the GPMI
    chains_write[6].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)     |   // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)        |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                  |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)       |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)          |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);                    // 1 byte command


    // Phase 8: read status and compare (DATA);
    chains_write[7].dma_nxtcmdar    =   (uint32_t)&chains_write[8];                     // point to the next descriptor
    chains_write[7].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)       |           // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(2)         |           // send 2 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)      |           // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)         |           // maintain resource lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |
                                        BF_APBH_CHn_CMD_CHAIN(1)            |           // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);     // no dma transfer
    chains_write[7].dma_bar         =   0;                                  // field not used
    // 2 word sent to the GPMI
    chains_write[7].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ_AND_COMPARE)  |   // read from the NAND and compare to expect
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)               |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          | // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)              |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);

    chains_write[7].gpmi_compare    =   MASK_AND_REFERENCE_VALUE;   // NOTE: mask and reference values are NAND
                                                                    //       SPECIFIC to evaluate the NAND status

    // Phase 9: psense compare (time out check);
    chains_write[8].dma_nxtcmdar    =   (uint32_t)&chains_write[9];                 // point to the next descriptor
    chains_write[8].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)       |       // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)         |       // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)      |       // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)   |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)         |       // relinquish nand lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)       |
                                        BF_APBH_CHn_CMD_CHAIN(1)            |       // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE);   // perform a sense check
    chains_write[8].dma_bar         =   (uint32_t)&chains_write[10];                // if sense check fails, branch to error handler


    // Phase 10: emit GPMI interrupt
    chains_write[9].dma_nxtcmdar    =   0;          // not used since this is last descriptor
    chains_write[9].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)               |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)                 |   // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |   // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(1)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)                 |
                                        BF_APBH_CHn_CMD_IRQONCMPLT(1)               |   // emit GPMI interrupt
                                        BF_APBH_CHn_CMD_CHAIN(0)                    |   // terminate DMA chain processing
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);     // no dma transfer


    //  ERROR Brunch;
    chains_write[10].dma_cmd          =   BF_APBH_CHn_CMD_IRQONCMPLT(1)               |
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |
                                        BF_APBH_CHn_CMD_SEMAPHORE(1)                |
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

    chains_write[10].dma_bar          =   0;
    chains_write[10].dma_nxtcmdar     =   0;

    // ============================================Erase Chains================================================
    //  Phase 1: issue NAND erase setup command (CLE/ALE);
    chains_erase[0].dma_nxtcmdar    =   (uint32_t)&chains_erase[1];
    chains_erase[0].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(1 + 3)           |   // 1 byte command, 3 byte block address
                                        BF_APBH_CHn_CMD_CMDWORDS(3)                 |   // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)              |   // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)                 |   // prevent other DMA channels from taking over
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                        BF_APBH_CHn_CMD_CHAIN(1)                    |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);        // read data from DMA, write to NAND

    chains_erase[0].dma_bar         =   ((uint32_t)FlashSendCommandBuffer) + 0;       // byte 0 write setup, bytes 1 - 3 NAND address
                                                                                        // 3 words sent to the GPMI
    chains_erase[0].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)     |   // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)        |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                  |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)       |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(1)          |   // send command and address
                                        BF_GPMI_CTRL0_XFER_COUNT(1 + 3);                // 1 byte command, 2 byte address
    chains_erase[0].gpmi_compare    =   (uint32_t)NULL;                             // field not used but necessary to set eccctrl
    chains_erase[0].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);      // disable the ECC block


    //  Phase 2: issue NAND erase conform command (CLE/ALE);


    chains_erase[1].dma_nxtcmdar    =   (uint32_t)&chains_erase[2];                 // point to the next descriptor
    chains_erase[1].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(1)               |   // 1 byte command
                                        BF_APBH_CHn_CMD_CMDWORDS(3)                 |   // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)              |   // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)           |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)                 |   // maintain resource lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                        BF_APBH_CHn_CMD_CHAIN(1)                    |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);        // read data from DMA, write to NAND
    chains_erase[1].dma_bar         =   ((uint32_t)FlashSendCommandBuffer) + 4;       // point to byte 4, write execute command
                                                                                        // 3 words sent to the GPMI
    chains_erase[1].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)     |   // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)      |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)        |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                  |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)       |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)          |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);                    // 1 byte command
    chains_erase[1].gpmi_compare    =   (uint32_t)NULL;                             // field not used but necessary to set eccctrl
    chains_erase[1].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);      // disable the ECC block



    //  Phase 3: wait for ready (CLE)
    chains_erase[2].dma_nxtcmdar    =   (uint32_t)&chains_erase[3];                 // point to the next descriptor
    chains_erase[2].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)               |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(1)                 |   // send 1 word to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)              |   // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(1)           |   // wait for nand to be ready
                                        BF_APBH_CHn_CMD_NANDLOCK(0)                 |   // relinquish nand lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)               |
                                        BF_APBH_CHn_CMD_CHAIN(1)                    |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);     // no dma transfer
    chains_erase[2].dma_bar         =   (uint32_t)NULL;                             // field not used

    // 1 word sent to the GPMI
    chains_erase[2].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WAIT_FOR_READY)    |   // wait for NAND ready
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)               |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)              |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        BF_GPMI_CTRL0_XFER_COUNT(0);


    //  Phase 4: psense compare (time out check)
    chains_erase[3].dma_nxtcmdar    =   (uint32_t)&chains_erase[4];                         // point to the next descriptor
    chains_erase[3].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)                       |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)                         |   // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)                      |   // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)                   |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)                         |
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)                       |
                                        BF_APBH_CHn_CMD_CHAIN(1)                            |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE);               // perform a sense check
    
    chains_erase[3].dma_bar         =   (uint32_t)&chains_erase[8];            // if sense check fails, branch to error handler


    //  Phase 5: setup read flash status command

    chains_erase[4].dma_nxtcmdar    =   (uint32_t)&chains_erase[5];                         // point to the next descriptor
    chains_erase[4].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(1)                       |   // 1 byte command
                                        BF_APBH_CHn_CMD_CMDWORDS(3)                         |   // send 3 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)                      |   // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)                   |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)                         |   // prevent other DMA channels from taking over
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)                       |
                                        BF_APBH_CHn_CMD_CHAIN(1)                            |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_READ);                // read data from DMA, write to NAND
    chains_erase[4].dma_bar         =   ((uint32_t)FlashSendCommandBuffer) + 5;               // status command
    chains_erase[4].gpmi_compare    =   (uint32_t)NULL;                                     // field not used but necessary to set eccctrl
    chains_erase[4].gpmi_eccctrl    =   BV_FLD(GPMI_ECCCTRL, ENABLE_ECC, DISABLE);              // disable the ECC block
                                                                                                // 3 words sent to the GPMI
    chains_erase[4].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, WRITE)             |   // write to the NAND
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, ENABLED)                |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_CLE)               |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);                            // 1 byte command

    //  Phase 6: read status and compare (DATA)

    chains_erase[5].dma_nxtcmdar    =   (uint32_t)&chains_erase[6];                         // point to the next descriptor
    chains_erase[5].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)                       |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(2)                         |   // send 2 words to the GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(1)                      |   // wait for command to finish before continuing
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)                   |
                                        BF_APBH_CHn_CMD_NANDLOCK(1)                         |   // maintain resource lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)                       |
                                        BF_APBH_CHn_CMD_CHAIN(1)                            |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);             // no dma transfer
    chains_erase[5].dma_bar         =   (uint32_t)NULL;                                     // field not used
    // 2 word sent to the GPMI
    chains_erase[5].gpmi_ctrl0      =   BV_FLD(GPMI_CTRL0, COMMAND_MODE, READ_AND_COMPARE)  |   // read from the NAND and compare to expect
                                        BV_FLD(GPMI_CTRL0, WORD_LENGTH, 8_BIT)              |
                                        BV_FLD(GPMI_CTRL0, LOCK_CS, DISABLED)               |
                                        BF_GPMI_CTRL0_CS(CHIP_SEL)                          |   // must correspond to NAND CS used
                                        BV_FLD(GPMI_CTRL0, ADDRESS, NAND_DATA)              |
                                        BF_GPMI_CTRL0_ADDRESS_INCREMENT(0)                  |
                                        BF_GPMI_CTRL0_XFER_COUNT(1);
    chains_erase[5].gpmi_compare    =   MASK_AND_REFERENCE_VALUE;                           // NOTE: mask and reference values are NAND
                                                                                            // SPECIFIC to evaluate the NAND status
    //  Phase 7: psense compare (time out check)

    chains_erase[6].dma_nxtcmdar    =   (uint32_t)&chains_erase[7];                         // point to the next descriptor
    chains_erase[6].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)                       |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)                         |   // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)                      |   // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(0)                        |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)                   |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)                         |   // relinquish nand lock
                                        BF_APBH_CHn_CMD_IRQONCMPLT(0)                       |
                                        BF_APBH_CHn_CMD_CHAIN(1)                            |   // follow chain to next command
                                        BV_FLD(APBH_CHn_CMD, COMMAND, DMA_SENSE);               // perform a sense check
    chains_erase[6].dma_bar         =   (uint32_t)&chains_erase[8];                      // if sense check fails, branch to error handler

    //  Phase 8: emit GPMI interrupt
    chains_erase[7].dma_nxtcmdar    =   0;           // not used since this is last descriptor
    chains_erase[7].dma_cmd         =   BF_APBH_CHn_CMD_XFER_COUNT(0)                   |   // no dma transfer
                                        BF_APBH_CHn_CMD_CMDWORDS(0)                     |   // no words sent to GPMI
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)                  |   // do not wait to continue
                                        BF_APBH_CHn_CMD_SEMAPHORE(1)                    |
                                        BF_APBH_CHn_CMD_NANDWAIT4READY(0)               |
                                        BF_APBH_CHn_CMD_NANDLOCK(0)                     |
                                        BF_APBH_CHn_CMD_IRQONCMPLT(1)                   |   // emit GPMI interrupt
                                        BF_APBH_CHn_CMD_CHAIN(0)                        |   // terminate DMA chain processing
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);         // no dma transfer

    //  ERROR Brunch;
    chains_erase[8].dma_cmd          =  BF_APBH_CHn_CMD_IRQONCMPLT(1)               |
                                        BF_APBH_CHn_CMD_WAIT4ENDCMD(0)              |
                                        BF_APBH_CHn_CMD_SEMAPHORE(1)                |
                                        BV_FLD(APBH_CHn_CMD, COMMAND, NO_DMA_XFER);

    chains_erase[8].dma_bar          =   0;
    chains_erase[8].dma_nxtcmdar     =   0;

    

}

void GPMIConfigure()
{
    GPMI_ClockConfigure();
    GPMI_EnableDMAChannel(true);
    GPMI_ResetDMAChannel();

    BF_CS1(GPMI_CTRL1, DEV_RESET, BV_GPMI_CTRL1_DEV_RESET__DISABLED);
    BF_CS1(GPMI_CTRL1, ATA_IRQRDY_POLARITY, BV_GPMI_CTRL1_ATA_IRQRDY_POLARITY__ACTIVEHIGH);
    BF_CS1(GPMI_CTRL1, GPMI_MODE, BV_GPMI_CTRL1_GPMI_MODE__NAND);

    GPMI_SetAccessTiming(defaultTiming);

    GPMI_DMAChainsInit();

    GPMI_curOpa = GPMI_OPA_NONE;

    portEnableIRQ(HW_IRQ_GPMI, true);
    portEnableIRQ(HW_IRQ_GPMI_DMA, true);
    portEnableIRQ(HW_IRQ_ECC8_IRQ, true);

    BF_SET(APBH_CTRL1, CH4_CMDCMPLT_IRQ_EN);
    BF_SET(ECC8_CTRL, COMPLETE_IRQ_EN);
    BF_SET(GPMI_CTRL0, DEV_IRQ_EN);
    BF_SET(GPMI_CTRL0, TIMEOUT_IRQ_EN);

    

}

static inline void waitLastOpa()
{
    switch (LastOpa)
    {
    case GPMI_OPA_READ:
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
/*
        while(HW_DIGCTL_MICROSECONDS_RD() - LastReadTime < defaultTiming.tREAD_us)
        {
            ;
        }*/

        break;
    case GPMI_OPA_WRITE:
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
/*
        while(HW_DIGCTL_MICROSECONDS_RD() - LastProgTime < defaultTiming.tPROG_us)
        {
            ;
        }*/

        break;
    case GPMI_OPA_ERASE:


        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        
        while(HW_DIGCTL_MICROSECONDS_RD() - LastEraseTime < defaultTiming.tBERS_us)
        {
            ;
        }

        break;

    default:
        break;
    }
}

static inline void    GPMI_sendCommand(uint32_t *cmd, uint32_t *para, uint16_t paraLen, uint32_t *buf, uint32_t RBlen, bool block)
{
    
    while (HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA)
            ;

    chains_cmd[1].dma_cmd &= 0x0000FFFF;
    chains_cmd[1].dma_cmd |= (1 + paraLen) << 16;
    chains_cmd[1].dma_bar = (uint32_t)cmd;

    chains_cmd[1].gpmi_ctrl0 &= 0xFFFF0000;
    chains_cmd[1].gpmi_ctrl0 |= ((1 + paraLen) & 0xFFFF);

    if(paraLen){
        chains_cmd[1].dma_nxtcmdar  =   (uint32_t)&chains_cmd[2];

        chains_cmd[2].dma_cmd &= 0x0000FFFF;
        chains_cmd[2].dma_cmd |= (1 + RBlen) << 16;
        chains_cmd[2].gpmi_ctrl0 &= 0xFFFF0000;
        chains_cmd[2].gpmi_ctrl0 |= ((1 + RBlen) & 0xFFFF);
        chains_cmd[2].dma_bar = (uint32_t)buf;

    }else{
        chains_cmd[1].dma_nxtcmdar  =   (uint32_t)&chains_cmd[3];
    }

    BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains_cmd); // 填写DMA寄存器下个描述符地址
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1);   

    if(block){
        while (HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA)
            ;
    }

}

static inline void   GPMI_ReadPage(uint32_t ColumnAddress, uint32_t RowAddress, uint32_t *data, uint32_t *auxData, bool block)
{
    volatile uint8_t *probe;
    waitLastOpa();
    //INFO("Start READ\n");
    volatile uint8_t *cmdBuf = (uint8_t *)FlashSendCommandBuffer;

    while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && !ECC_FIN)
        ;

    cmdBuf[0] = NAND_CMD_READ0;
    cmdBuf[1] = ColumnAddress & 0xFF;
    cmdBuf[2] = (ColumnAddress >> 8) & 0xFF;
    cmdBuf[3] = RowAddress & 0xFF;
    cmdBuf[4] = (RowAddress >> 8) & 0xFF;
    cmdBuf[5] = NAND_CMD_READSTART;

    if(data){
        chains_read[4].gpmi_data_ptr    =   (uint32_t)data;
    }else{
        chains_read[4].gpmi_data_ptr    =   (uint32_t)GPMI_DataBuffer;
    }

    if(auxData){
        chains_read[4].gpmi_aux_ptr     =   (uint32_t)auxData; 
    }else{
        chains_read[4].gpmi_aux_ptr     =   (uint32_t)GPMI_AuxiliaryBuffer; 
    }

    MTD_INFO("WAIT MTD READ FIN\n");

    probe = (uint8_t *)chains_read[4].gpmi_aux_ptr;
    probe[16] = 0x23;

    ECC_FIN = false;
    ECCResult = 0x0E0E0E0E;

    GPMI_curOpa = GPMI_OPA_READ;
#ifdef PR_NAND_WR_TIMING_STATUS
    pgrdt = HW_DIGCTL_MICROSECONDS_RD();
#endif
    BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains_read[0]); 
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1);  

    MTD_INFO("MTD READ OPA SENT\n");

    if(block){
        while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && !ECC_FIN)
            ;
    }

    MTD_INFO("MTD READ OPA SENT FIN\n");

    LastReadTime = HW_DIGCTL_MICROSECONDS_RD();
    LastOpa = GPMI_OPA_READ;

        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        //portDelayus(100);

        while(probe[16] == 0x23)
            ;
}

static inline void    GPMI_EraseBlock(uint32_t blockAddress, bool block)
{
    waitLastOpa();

    volatile uint8_t *cmdBuf = (uint8_t *)FlashSendCommandBuffer;

    while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && !ECC_FIN)
        ;

    cmdBuf[0] = NAND_CMD_ERASE1;
    cmdBuf[1] = (blockAddress << 6) & 0xFF;
    cmdBuf[2] = (blockAddress >> 2) & 0xFF;
    cmdBuf[3] = (blockAddress >> 10) & 0xFF;
    cmdBuf[4] = NAND_CMD_ERASE2;
    cmdBuf[5] = NAND_CMD_STATUS;




    
    LastEraseTime = HW_DIGCTL_MICROSECONDS_RD();

    GPMI_curOpa = GPMI_OPA_ERASE;

    BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains_erase[0]); 
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1); 

    if(block){
        while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && !ECC_FIN)
            ;
    }

    LastEraseTime = HW_DIGCTL_MICROSECONDS_RD();
    LastOpa = GPMI_OPA_ERASE;

        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        
        //portDelayms(4);
}

static inline void    GPMI_WritePage(uint32_t ColumnAddress, uint32_t RowAddress, uint32_t *data, uint32_t *auxData, bool block)
{
    waitLastOpa();

    while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA))
        ;
   

    volatile uint8_t *cmdBuf = (uint8_t *)FlashSendCommandBuffer;
    cmdBuf[0] = NAND_CMD_SEQIN;
    cmdBuf[1] = ColumnAddress & 0xFF;
    cmdBuf[2] = (ColumnAddress >> 8) & 0xFF;
    cmdBuf[3] = RowAddress & 0xFF;
    cmdBuf[4] = (RowAddress >> 8) & 0xFF;
    cmdBuf[5] = NAND_CMD_PAGEPROG;
    cmdBuf[6] = NAND_CMD_STATUS;

    

    if(data == NULL){
        memset(GPMI_DataBuffer, 0xFF, sizeof(GPMI_DataBuffer));
        chains_write[1].dma_bar     =   (uint32_t)GPMI_DataBuffer;
    }else{
        chains_write[1].dma_bar     =   (uint32_t)data;
    }

    if(auxData == NULL)
    {
        memset(GPMI_AuxiliaryBuffer, 0xFF, sizeof(GPMI_AuxiliaryBuffer));
        chains_write[2].dma_bar     =   (uint32_t)GPMI_AuxiliaryBuffer;
    }else{
        chains_write[2].dma_bar     =   (uint32_t)auxData;
    }



    GPMI_curOpa = GPMI_OPA_WRITE;
#ifdef PR_NAND_WR_TIMING_STATUS
    pgwdt = HW_DIGCTL_MICROSECONDS_RD();
#endif
    BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains_write[0]); 
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1); 

    LastProgTime = HW_DIGCTL_MICROSECONDS_RD();

    if(block){
        while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA))
            ;
    }

    LastEraseTime = HW_DIGCTL_MICROSECONDS_RD();
    LastOpa = GPMI_OPA_WRITE;

        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        
        //portDelayus(200);
}


static inline void    GPMI_CopyPage(uint32_t srcPage, uint32_t dstPage)
{
    waitLastOpa();
    
    volatile uint8_t *cmdBuf = (uint8_t *)FlashSendCommandBuffer;
    while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && !ECC_FIN)
        ;

    cmdBuf[0] = NAND_CMD_READ0;
    cmdBuf[1] = 0;
    cmdBuf[2] = 0;
    cmdBuf[3] =  srcPage & 0xFF;
    cmdBuf[4] = (srcPage >> 8) & 0xFF;
    cmdBuf[5] = NAND_CMD_READSTART;

    chains_read[4].gpmi_data_ptr    =   (uint32_t)GPMI_DataBuffer;
    chains_read[4].gpmi_aux_ptr     =   (uint32_t)GPMI_AuxiliaryBuffer; 
    
    ECC_FIN = false;
    GPMI_curOpa = GPMI_OPA_COPY;
    GPMI_CopyState = 0;

    BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains_read[0]); 
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1);

    
    while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && (!ECC_FIN))
        ;

        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        /*
    portDelayus(defaultTiming.tREAD_us * 10);
    portDelayus(defaultTiming.tREAD_us * 10);
    portDelayus(defaultTiming.tREAD_us * 10);*/
    
        //portDelayus(100);

    while ((HW_APBH_CHn_SEMA(NAND_DMA_Channel).B.INCREMENT_SEMA) && (!ECC_FIN))
        ;

    GPMI_curOpa = GPMI_OPA_COPY;
    GPMI_CopyState = 1;

    cmdBuf[0] = NAND_CMD_SEQIN;
    cmdBuf[1] = 0;
    cmdBuf[2] = 0;
    cmdBuf[3] = dstPage & 0xFF;
    cmdBuf[4] = (dstPage >> 8) & 0xFF;
    cmdBuf[5] = NAND_CMD_PAGEPROG;
    cmdBuf[6] = NAND_CMD_STATUS;    

    chains_write[1].dma_bar     =   (uint32_t)GPMI_DataBuffer;
    chains_write[2].dma_bar     =   (uint32_t)GPMI_AuxiliaryBuffer;

    BF_WRn(APBH_CHn_NXTCMDAR, NAND_DMA_Channel, CMD_ADDR, (reg32_t)&chains_write[0]); 
    BW_APBH_CHn_SEMA_INCREMENT_SEMA(NAND_DMA_Channel, 1); 

    LastProgTime = HW_DIGCTL_MICROSECONDS_RD();
    LastOpa = GPMI_OPA_WRITE;

        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.AHB_BYTES);
        while(HW_APBH_CHn_DEBUG2(NAND_DMA_Channel).B.APB_BYTES);
        
        //portDelayus(200);
}

static void NAND_Reset()
{
    FlashSendCommandBuffer[0] = 0xFF;
    GPMI_sendCommand(FlashSendCommandBuffer, 0, 0, 0, 0, true);
    portDelayus(10);
}

static void GPMI_GetNANDInfo(mtdInfo_t *mtdinfo)
{

    FlashSendCommandBuffer[0] = 0x90;
    FlashSendParaBuffer[0] = 0;
    GPMI_sendCommand(FlashSendCommandBuffer, FlashSendParaBuffer, 1, FlashRecCommandBuffer, 6, true);

    INFO("Flash ID:\n");
    for(int i = 0; i < 6;i++){
        INFO("%02x ", ((uint8_t *)FlashRecCommandBuffer)[i] );
    }
    INFO("\n");

    char DIDesc4Rd = ((uint8_t *)FlashRecCommandBuffer)[3];

    mtdinfo->PageSize_B = (1 << ( DIDesc4Rd & 0x3 )) * 1024 ;
    mtdinfo->SpareSizePerPage_B = mtdinfo->PageSize_B / 512 * ((1 << ((DIDesc4Rd >> 2) & 1)) * 8);
    mtdinfo->BlockSize_KB = (1 << ((DIDesc4Rd >> 4) & 0x3)) * 64;
    mtdinfo->PagesPerBlock = mtdinfo->BlockSize_KB * 1024 / mtdinfo->PageSize_B;

    mtdinfo->MetaSize_B = 19;
    mtdinfo->Blocks = 1024;

    INFO("PageSize:%lu B\n", mtdinfo->PageSize_B);
    INFO("SpareSizePerPage:%lu B\n", mtdinfo->SpareSizePerPage_B);
    INFO("BlockSize:%lu KB\n", mtdinfo->BlockSize_KB);
    INFO("PagesPerBlock:%lu\n", mtdinfo->PagesPerBlock);
    INFO("Blocks:%lu\n", mtdinfo->Blocks);

}

void portMTD_ISR()
{

    if(BF_RD(GPMI_CTRL1, TIMEOUT_IRQ))
        INFO("GPMI_TIMEOUT_IRQ\n");
    if(BF_RD(GPMI_CTRL1, DEV_IRQ))
        INFO("GPMI_DEV_IRQ\n");

    BF_CS1(GPMI_CTRL1, DEV_IRQ, 0);
    BF_CS1(GPMI_CTRL1, TIMEOUT_IRQ, 0);
}

void portMTD_DMA_ISR()
{
    uint32_t error = BF_RD(APBH_CTRL1, CH4_AHB_ERROR_IRQ);
    //INFO("\nportMTD_DMA_ISR\n");
    
    BF_CLR(APBH_CTRL1, CH4_CMDCMPLT_IRQ);
    BF_CLR(APBH_CTRL1, CH4_AHB_ERROR_IRQ);

    if(error)
    {
        INFO("GPMI_DMA_ERR\n");
    }   



    switch (GPMI_curOpa)
    {
    case GPMI_OPA_READ:
#ifdef PR_NAND_WR_TIMING_STATUS
        INFO("prd=%ld\n", HW_DIGCTL_MICROSECONDS_RD() - pgrdt);
#endif
        if(BF_RDn(APBH_CHn_CURCMDAR, NAND_DMA_Channel, CMD_ADDR) == (uint32_t)&chains_read[8]){
            INFO("GPMI_OPA_READ psense compare ERROR\n");
        }

        break;
    case GPMI_OPA_WRITE:
#ifdef PR_NAND_WR_TIMING_STATUS
        INFO("pwr=%ld\n", HW_DIGCTL_MICROSECONDS_RD() - pgwdt);
#endif
        if(BF_RDn(APBH_CHn_CURCMDAR, NAND_DMA_Channel, CMD_ADDR) == (uint32_t)&chains_read[8]){
            INFO("GPMI_OPA_WRITE psense compare ERROR\n");
            MTD_upOpaFin(1);
        }else{
            MTD_upOpaFin(0);
        }
        break;
    case GPMI_OPA_ERASE:
        if(BF_RDn(APBH_CHn_CURCMDAR, NAND_DMA_Channel, CMD_ADDR) == (uint32_t)&chains_read[8]){
            INFO("GPMI_OPA_ERASE psense compare ERROR\n");
            MTD_upOpaFin(1);
        }else{
            MTD_upOpaFin(0);
        }
        break;  

    case GPMI_OPA_COPY:
        if(GPMI_CopyState == 0)
        {
            if(BF_RDn(APBH_CHn_CURCMDAR, NAND_DMA_Channel, CMD_ADDR) == (uint32_t)&chains_read[8]){
                INFO("GPMI_OPA_COPY_READ psense compare ERROR\n");
            }
            return;
        }
        if(GPMI_CopyState == 1){
            if(BF_RDn(APBH_CHn_CURCMDAR, NAND_DMA_Channel, CMD_ADDR) == (uint32_t)&chains_read[8]){
                INFO("GPMI_OPA_COPY_WRITE psense compare ERROR\n");
                MTD_upOpaFin(0x0E0E0E0E);
            }else{
                MTD_upOpaFin(CopyECCResult);
            }
        }

        break;

    default:
        break;
    }
    
    GPMI_curOpa = GPMI_OPA_NONE;
    
}

bool portMTD_ECC_ISR()
{


    ECCResult = BF_RD(ECC8_STATUS1, STATUS_PAYLOAD0)            |
                (BF_RD(ECC8_STATUS1, STATUS_PAYLOAD1) << 8)     |
                (BF_RD(ECC8_STATUS1, STATUS_PAYLOAD2) << 16)    |
                (BF_RD(ECC8_STATUS1, STATUS_PAYLOAD3) << 24)    ;
    

    BF_CLR(ECC8_CTRL, COMPLETE_IRQ);
    BF_CLR(ECC8_CTRL, BM_ERROR_IRQ);
    
    //INFO("portMTD_ECC_ISR\n");

    if((GPMI_curOpa == GPMI_OPA_COPY) && (GPMI_CopyState == 0)){
        CopyECCResult = ECCResult;
        ECC_FIN = true;
        return false;
    }

    ECC_FIN = true;

    return MTD_upOpaFin(ECCResult);

}


void portMTDInterfaceInit()
{
    for(int i =0; i < 32; i++)
    {
        ReserveBlock[i] = 0xFFFFFFFF;
    }
    GPMIConfigure();
    ECC_FIN = true;
    
}

void portMTDDeviceInit(mtdInfo_t *mtdinfo)
{
    NAND_Reset();
    GPMI_GetNANDInfo(mtdinfo);


    //GPMI_GetNANDInfo(mtdinfo);

}
//static uint32_t lastRDPage = 0xFFFFFFFF;

void portMTDReadPage(uint32_t page, uint8_t *buf)
{
    /*
    if((LastOpa == GPMI_OPA_READ) && (lastRDPage == page)){
        //INFO("RD:%d\n", page);
        MTD_upOpaFin(ECCResult);
        return;
    }*/
    
    GPMI_ReadPage(0, page, (uint32_t *)buf, NULL, false);

    //lastRDPage = page;
}

void portMTDWritePage(uint32_t page, uint8_t *buf)
{
    //NFO("WR:%d\n", page);
    GPMI_WritePage(0, page, (uint32_t *)buf, NULL, false);
}

void portMTDWritePageMeta(uint32_t page, uint8_t *buf, uint8_t *metaBuf)
{
    //INFO("WRM:%d\n", page);
    GPMI_WritePage(0, page, (uint32_t *)buf, (uint32_t *)metaBuf, false);
}

uint8_t *portMTDGetMetaData()
{
    return (uint8_t *)GPMI_AuxiliaryBuffer;
}


void portMTDEraseBlock(uint32_t block)
{
    GPMI_EraseBlock(block, false);
}

void portMTDCopyPage(uint32_t src, uint32_t dst)
{
    GPMI_CopyPage(src, dst);
}