#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board_up.h"
#include "interrupt_up.h"

#include "regsaudioout.h"
#include "regsclkctrl.h"
#include "regsclkctrl.h"
#include "regsapbx.h"
#include "hw_irq.h"

#include "SystemConfig.h"

#ifdef ENABLE_AUIDIOOUT

struct apb_dma_command_t
{
    struct apb_dma_command_t *next;
    uint32_t cmd;
    void *buffer;
    /* PIO words follow */
} __attribute__((packed));

struct pcm_dma_command_t
{
    struct apb_dma_command_t dma;
    /* padded to next multiple of cache line size (32 bytes) */
    uint32_t pad[5];
} __attribute__((packed));

uint32_t pcm_buffer1[4096] __attribute__((aligned(4)));
uint32_t pcm_buffer2[4096] __attribute__((aligned(4)));

volatile bool pcm_buffer_1_fin = true;
volatile bool pcm_buffer_2_fin = true; 

volatile bool pcm_playing = false;

volatile void *cur_pcm_buffer;

static struct pcm_dma_command_t dac_dma;


inline static void pcm_dma_load()
{
    dac_dma.dma.buffer = (void *)cur_pcm_buffer;

    HW_APBX_CHn_NXTCMDAR(1).B.CMD_ADDR = (uint32_t)&dac_dma;
    HW_APBX_CHn_SEMA(1).B.INCREMENT_SEMA = 1;
    

}

void portDAC_IRQ(uint32_t IRQn)
{
    //printf("DAC_IRQ\n");
    BF_CLR(APBX_CTRL1, CH1_CMDCMPLT_IRQ);
    if(IRQn == HW_IRQ_ADC_ERROR)
    {
        printf("DAC ERROR!\n");

    }

    if(pcm_buffer_1_fin)
    {
        if(!pcm_buffer_2_fin)
        {
            pcm_buffer_2_fin = true;
            cur_pcm_buffer = pcm_buffer2;
            pcm_dma_load();
        }else{
            //all buffers played.
            pcm_playing = false;

        }
    }else{
            pcm_buffer_1_fin = true;
            cur_pcm_buffer = pcm_buffer1;
            pcm_dma_load();
    }
    
}

bool is_pcm_buffer_idle()
{
    return pcm_buffer_1_fin || pcm_buffer_2_fin;
}

void pcm_buffer_load(void *pcmdat)
{
    if(pcm_buffer_1_fin)
    {
        memcpy(pcm_buffer1, pcmdat, sizeof(pcm_buffer1));
        pcm_buffer_1_fin = false;
    }else if(pcm_buffer_2_fin)
    {
        memcpy(pcm_buffer2, pcmdat, sizeof(pcm_buffer2));
        pcm_buffer_2_fin = false;
    }else{
        return;
    }
    
    if(!pcm_playing)
    {
        pcm_playing = true;
        portDAC_IRQ(HW_IRQ_ADC_DMA);
    }
}


void stmp_audio_init()
{
    BF_CLR(AUDIOOUT_CTRL, SFTRST);
    BF_CLR(AUDIOOUT_CTRL, CLKGATE);

    BF_SET(AUDIOOUT_CTRL, SFTRST);
    while(BF_RD(AUDIOOUT_CTRL, CLKGATE) == 0)
    {
        ;
    }
    BF_CLR(AUDIOOUT_CTRL, SFTRST);
    BF_CLR(AUDIOOUT_CTRL, CLKGATE);

    BF_CLR(CLKCTRL_XTAL, FILT_CLK24M_GATE);



    /* Enable DAC */
    BF_CLR(AUDIOOUT_ANACLKCTRL, CLKGATE);

    BF_CLR(AUDIOOUT_PWRDN, CAPLESS);

    /* Set word-length to 16-bit */
    BF_SET(AUDIOOUT_CTRL, WORD_LENGTH);

    /* Power up DAC */
    BF_CLR(AUDIOOUT_PWRDN, DAC);
    /* Hold HP to ground to avoid pop, then release and power up HP */
    BF_SET(AUDIOOUT_ANACTRL, HP_HOLD_GND);
    BF_CLR(AUDIOOUT_PWRDN, HEADPHONE);


    /* Set HP mode to AB */
    BF_SET(AUDIOOUT_ANACTRL, HP_CLASSAB);
    
    /* change bias to -50% */
    
    BF_WR(AUDIOOUT_TEST, HP_I1_ADJ, (1));
    BF_WR(AUDIOOUT_REFCTRL, BIAS_CTRL, (1));
    BF_SET(AUDIOOUT_REFCTRL, RAISE_REF);
    BF_SET(AUDIOOUT_REFCTRL, XTAL_BGR_BIAS);




    /* Stop holding to ground */
    BF_CLR(AUDIOOUT_ANACTRL, HP_HOLD_GND);

    
    /* Set dmawait count to 31 (see errata, workaround random stop) */
    BF_WR(AUDIOOUT_CTRL, DMAWAIT_COUNT, (31));
    /* start converting audio */
    BF_SET(AUDIOOUT_CTRL, RUN);
    /* unmute DAC */
    BF_CLR(AUDIOOUT_DACVOLUME, MUTE_LEFT);
    BF_CLR(AUDIOOUT_DACVOLUME, MUTE_RIGHT);

    HW_AUDIOOUT_HPVOL.B.MUTE = 0;
    
    HW_AUDIOOUT_HPVOL.B.VOL_LEFT = 0x31;
    HW_AUDIOOUT_HPVOL.B.VOL_RIGHT = 0x31;
    /*send a few samples to avoid pop*/

    HW_AUDIOOUT_DATA.U = 0;
    HW_AUDIOOUT_DATA.U = 0;
    HW_AUDIOOUT_DATA.U = 0;
    HW_AUDIOOUT_DATA.U = 0;

    printf("stmp_audio_init \n");

/*
    //44100 Hz
    HW_AUDIOOUT_DACSRR.B.BASEMULT = 0x1; // quad-rate
    HW_AUDIOOUT_DACSRR.B.SRC_HOLD = 0x0; // 0 for full- double- quad-rates
    HW_AUDIOOUT_DACSRR.B.SRC_INT = 0x11; // 15 for the integer portion
    HW_AUDIOOUT_DACSRR.B.SRC_FRAC = 0x0037; // the fractional portion
*/
/*
    //32000 Hz
    HW_AUDIOOUT_DACSRR.B.BASEMULT = 0x1; // quad-rate
    HW_AUDIOOUT_DACSRR.B.SRC_HOLD = 0x0; // 0 for full- double- quad-rates
    HW_AUDIOOUT_DACSRR.B.SRC_INT = 0x17; 
    HW_AUDIOOUT_DACSRR.B.SRC_FRAC = 0x0E00; // the fractional portion
*/
    //22050 Hz
    HW_AUDIOOUT_DACSRR.B.BASEMULT = 0x1; // quad-rate
    HW_AUDIOOUT_DACSRR.B.SRC_HOLD = 0x1; // 0 for full- double- quad-rates
    HW_AUDIOOUT_DACSRR.B.SRC_INT = 0x11; 
    HW_AUDIOOUT_DACSRR.B.SRC_FRAC = 0x0037; // the fractional portion


    HW_APBX_CTRL1.B.CH1_CMDCMPLT_IRQ_EN = 1;
    HW_APBX_CTRL1.B.CH1_ERROR_IRQ = 1;


    
    dac_dma.dma.next = NULL;
    dac_dma.dma.cmd = BF_APBX_CHn_CMD_XFER_COUNT(sizeof(pcm_buffer1)) | 
                      BF_APBX_CHn_CMD_IRQONCMPLT(1) |
                      BF_APBX_CHn_CMD_SEMAPHORE(1) |
                      BV_FLD(APBX_CHn_CMD, COMMAND, DMA_READ);

    portEnableIRQ(HW_IRQ_DAC_DMA, 1);
    portEnableIRQ(HW_IRQ_DAC_ERROR, 1);


    HW_AUDIOOUT_DACVOLUME.U = 0x00ff00ff;

    


}

#endif


