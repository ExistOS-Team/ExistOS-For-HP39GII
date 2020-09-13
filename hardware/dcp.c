#include "dcp.h"
#include <stdio.h>



// Reference: P477-P484 of the manual

DCP_DESCRIPTOR dcp1;
DCP_CONTEXT dcpContextBuf[4];

void dcp_init()
{
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_SFTRST);
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_CLKGATE);
    HW_DCP_CTRL_SET(BM_DCP_CTRL_SFTRST);
    while (!HW_DCP_CTRL.B.CLKGATE)
        ;

    HW_DCP_CTRL_CLR(BM_DCP_CTRL_SFTRST);
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_CLKGATE);
    HW_DCP_CTRL_SET(BM_DCP_CTRL_ENABLE_CONTEXT_SWITCHING);
    BW_DCP_CTRL_CHANNEL_INTERRUPT_ENABLE(0xF);

    BW_DCP_CHANNELCTRL_ENABLE_CHANNEL(0xF);

    HW_DCP_CONTEXT_WR((reg32_t)dcpContextBuf);
}

reg32_t dcp_memset(void *dst, int c, reg32_t size) // the original type is size_t
{
    reg32_t ret = 0;

    // set up control packet
    dcp1.next = 0;                   // single packet in chain
    dcp1.ctrl0.U = 0;                // clear ctrl0 field
    dcp1.ctrl0.B.ENABLE_MEMCOPY = 1; // enable memcopy
    dcp1.ctrl0.B.CONSTANT_FILL = 1;
    dcp1.ctrl0.B.DECR_SEMAPHORE = 1; // decrement semaphore
    dcp1.ctrl0.B.INTERRUPT = 1;      // interrupt
    dcp1.ctrl1.U = 0;                // clear ctrl1
    dcp1.src = (reg32_t *)c;         // the constant
    dcp1.dst = dst;                  // destination buffer
    dcp1.buf_size = size;            // size in bytes
    dcp1.payload = 0x0;              // not required
    dcp1.status = 0;                 // clear status

    // Enable channel 0
    HW_DCP_CHnCMDPTR_WR(0, (reg32_t)&dcp1); // write packet address to pointer register
    HW_DCP_CHnSEMA_WR(0, 1);                // use ch0. increment semaphore by 1

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

reg32_t dcp_memcpy(void *dst, void *src, reg32_t size) // the original type is size_t
{
    reg32_t ret = 0;

    // set up control packet
    dcp1.next = 0;                   // single packet in chain
    dcp1.ctrl0.U = 0;                // clear ctrl0 field
    dcp1.ctrl0.B.ENABLE_MEMCOPY = 1; // enable memcopy
    dcp1.ctrl0.B.DECR_SEMAPHORE = 1; // decrement semaphore
    dcp1.ctrl0.B.INTERRUPT = 1;      // interrupt
    dcp1.ctrl1.U = 0;                // clear ctrl1
    dcp1.src = src;                  // source buffer
    dcp1.dst = dst;                  // destination buffer
    dcp1.buf_size = size;            // size in bytes
    dcp1.payload = 0x0;              // not required
    dcp1.status = 0;                 // clear status

    // Enable channel 0
    HW_DCP_CHnCMDPTR_WR(0, (reg32_t)&dcp1); // write packet address to pointer register
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