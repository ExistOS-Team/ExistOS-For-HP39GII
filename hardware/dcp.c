#include "dcp.h"
#include "irq.h"
#include "hw_irq.h"
#include <stdio.h>

#define u32 reg32_t
// Reference: P477-P484 of the manual

DCP_DESCRIPTOR dcp0, dcp1, dcp2, dcp3;
DCP_CONTEXT dcpContextBuf[4];
/*
ch0: memcpy/memset sync
ch1: blit
ch2: hash
ch3: cipher
*/
void (*dcp_ch1_callback)(reg32_t) = 0x0;
void (*dcp_ch2_callback)(reg32_t) = 0x0;
void (*dcp_ch3_callback)(reg32_t) = 0x0;

void dcp_callback(){
    if (dcp_ch1_callback && (HW_DCP_STAT_RD() & 0x2))
    {
        dcp_ch1_callback(HW_DCP_CHnSTAT_RD(1));
    }
    if (dcp_ch2_callback && (HW_DCP_STAT_RD() & 0x4))
    {
        dcp_ch2_callback(HW_DCP_CHnSTAT_RD(2));
    }
    if (dcp_ch3_callback && (HW_DCP_STAT_RD() & 0x8))
    {
        dcp_ch3_callback(HW_DCP_CHnSTAT_RD(3));
    }
    HW_DCP_STAT_CLR(0xE);
}

void dcp_init()
{
    irq_install_service(HW_IRQ_DCP, dcp_callback);
    irq_set_enable(HW_IRQ_DCP, 1);

    HW_DCP_CTRL_CLR(BM_DCP_CTRL_SFTRST);
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_CLKGATE);
    HW_DCP_CTRL_SET(BM_DCP_CTRL_SFTRST);
    while (!HW_DCP_CTRL.B.CLKGATE)
        ;

    HW_DCP_CTRL_CLR(BM_DCP_CTRL_SFTRST);
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_CLKGATE);
    HW_DCP_CTRL_SET(BM_DCP_CTRL_ENABLE_CONTEXT_SWITCHING);
    BW_DCP_CTRL_CHANNEL_INTERRUPT_ENABLE(0xF);

    //BW_DCP_CHANNELCTRL_CH0_IRQ_MERGED(1);
    BW_DCP_CHANNELCTRL_ENABLE_CHANNEL(0xF);

    HW_DCP_CONTEXT_WR((reg32_t)dcpContextBuf);
}

reg32_t dcp_memset(void *dst, int constant, reg32_t size)
{
    reg32_t ret = 0;

    // set up control packet
    dcp0.next = 0;                   // single packet in chain
    dcp0.ctrl0.U = 0;                // clear ctrl0 field
    dcp0.ctrl0.B.ENABLE_MEMCOPY = 1; // enable memcopy
    dcp0.ctrl0.B.CONSTANT_FILL = 1;
    dcp0.ctrl0.B.DECR_SEMAPHORE = 1; // decrement semaphore
    dcp0.ctrl0.B.INTERRUPT = 1;      // interrupt
    dcp0.ctrl1.U = 0;                // clear ctrl1
    dcp0.constant = constant;        // the constant
    dcp0.dst = dst;                  // destination buffer
    dcp0.buf_size = size;            // size in bytes
    dcp0.payload = 0x0;              // not required
    dcp0.status = 0;                 // clear status

    // Enable channel 0
    HW_DCP_CHnCMDPTR_WR(0, (reg32_t)&dcp0); // write packet address to pointer register
    HW_DCP_CHnSEMA_WR(0, 1);                // increment semaphore by 1

    // now wait for interrupt or poll
    // polling code
    while ((HW_DCP_STAT_RD() & 0x1) == 0)
        ;

    // now check/clear channel status
    if ((HW_DCP_CHnSTAT_RD(0) & 0xFF) != 0)
    {
        // an error occurred
        ret = HW_DCP_CHnSTAT_RD(0); // return ch0 status
        HW_DCP_CHnSTAT_CLR(0, 0xFF);
    }

    // clear interrupt register
    HW_DCP_STAT_CLR(0x1);

    return ret;
}

reg32_t dcp_memcpy(void *dst, void *src, reg32_t size)
{
    reg32_t ret = 0;

    // set up control packet
    dcp0.next = 0;                   // single packet in chain
    dcp0.ctrl0.U = 0;                // clear ctrl0 field
    dcp0.ctrl0.B.ENABLE_MEMCOPY = 1; // enable memcopy
    dcp0.ctrl0.B.DECR_SEMAPHORE = 1; // decrement semaphore
    dcp0.ctrl0.B.INTERRUPT = 1;      // interrupt
    dcp0.ctrl1.U = 0;                // clear ctrl1
    dcp0.src = src;                  // source buffer
    dcp0.dst = dst;                  // destination buffer
    dcp0.buf_size = size;            // size in bytes
    dcp0.payload = 0x0;              // not required
    dcp0.status = 0;                 // clear status

    // Enable channel 0
    HW_DCP_CHnCMDPTR_WR(0, (reg32_t)&dcp0); // write packet address to pointer register
    HW_DCP_CHnSEMA_WR(0, 1);                // increment semaphore by 1

    // now wait for interrupt or poll
    // polling code
    while ((HW_DCP_STAT_RD() & 0x1) == 0)
        ;

    // now check/clear channel status
    if ((HW_DCP_CHnSTAT_RD(0) & 0xFF) != 0)
    {
        // an error occurred
        ret = HW_DCP_CHnSTAT_RD(0); // return ch0 status
        HW_DCP_CHnSTAT_CLR(0, 0xFF);
    }

    // clear interrupt register
    HW_DCP_STAT_CLR(0x1);

    return ret;
}

reg32_t dcp_blit(void *dst, void *src, reg16_t frameWidth, reg16_t blitWidth, reg16_t numberLines, void (*callback)(reg32_t))
{
    reg32_t ret;

    // set up control packet
    dcp1.next = 0;                   // single packet in chain
    dcp1.ctrl0.U = 0;                // clear ctrl0 field
    dcp1.ctrl0.B.ENABLE_MEMCOPY = 1; // enable memcopy
    dcp1.ctrl0.B.ENABLE_BLIT = 1;
    dcp1.ctrl0.B.DECR_SEMAPHORE = 1; // decrement semaphore
    dcp1.ctrl0.B.INTERRUPT = 1;      // interrupt
    dcp1.ctrl1.U = 0;                // clear ctrl1
    dcp1.ctrl1.B.FRAMEBUFFER_LENGTH = frameWidth;
    dcp1.src = src;                  // source buffer
    dcp1.dst = dst;                  // destination buffer
    dcp1.blit_width = blitWidth;
    dcp1.number_lines = numberLines;
    dcp1.payload = 0x0;              // not required
    dcp1.status = 0;                 // clear status

    // Enable channel 1
    HW_DCP_CHnCMDPTR_WR(1, (reg32_t)&dcp1); // write packet address to pointer register
    dcp_ch1_callback = callback;            // set callback function
    HW_DCP_CHnSEMA_WR(1, 1);                // increment semaphore by 1

    if (callback == 0x0)
    {
        // now wait for interrupt or poll
        // polling code
        while ((HW_DCP_STAT_RD() & 0x2) == 0)
            ;

        // now check/clear channel status
        if ((HW_DCP_CHnSTAT_RD(1) & 0xFF) != 0)
        {
            // an error occurred
            ret = HW_DCP_CHnSTAT_RD(1); // return ch2 status
            HW_DCP_CHnSTAT_CLR(1, 0xFF);
        }

        // clear interrupt register
        HW_DCP_STAT_CLR(0x4);
    }
    return ret;
}

reg32_t dcp_hash(reg32_t *payload, void *src, reg32_t size) // payload is where the hash is written to and should be a length-5 array (20 bytes)
{
    reg32_t ret = 0;

    // set up control packet
    dcp2.next = 0;                   // single packet in chain
    dcp2.ctrl0.U = 0;                // clear ctrl0 field
    dcp2.ctrl0.B.HASH_INIT = 1;      // initialize hash with this block
    dcp2.ctrl0.B.HASH_TERM = 1;      // terminate hash with this block
    dcp2.ctrl0.B.ENABLE_HASH = 1;    // enable hash
    dcp2.ctrl0.B.DECR_SEMAPHORE = 1; // decrement semaphore
    dcp2.ctrl0.B.INTERRUPT = 1;      // interrupt
    dcp2.ctrl1.U = 0;                // clear ctrl1
    dcp2.src = src;                  // source buffer
    dcp2.dst = 0;                    // no destination buffer
    dcp2.buf_size = size;            // size in bytes
    dcp2.payload = payload;          // holds generated hash value
    dcp2.status = 0;                 // clear status

    // Enable channel 2
    HW_DCP_CHnCMDPTR_WR(2, (reg32_t)&dcp2); // write packet address to pointer register
    HW_DCP_CHnSEMA_WR(2, 1);                // increment semaphore by 1

    // now wait for interrupt or poll
    // polling code
    while ((HW_DCP_STAT_RD() & 0x4) == 0)
        ;

    // now check/clear channel status
    if ((HW_DCP_CHnSTAT_RD(2) & 0xFF) != 0)
    {
        // an error occurred
        ret = HW_DCP_CHnSTAT_RD(2); // return ch2 status
        HW_DCP_CHnSTAT_CLR(2, 0xFF);
    }

    // clear interrupt register
    HW_DCP_STAT_CLR(0x4);

    return ret;
}