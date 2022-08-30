
#include "FreeRTOS.h"
#include "task.h"

#include "ff.h"

#include "string.h"
#include "stdio.h"

#define LOAD_BLOCK_SIZE 65536

void *emu_rom_addr = NULL;

static FIL *romf;

static size_t rom_size;

static bool rom_isPacked = true;

static uint8_t *rdrom_buf;

bool rom_is_packed()
{
    return rom_isPacked;
}

int load_rom(const char *romfile) {
    uint32_t testread;
    FRESULT fr;
    UINT btr;
    romf = pvPortMalloc(sizeof(FIL));
    if (romf) {

        rdrom_buf = pvPortMalloc(LOAD_BLOCK_SIZE);
        if (!rdrom_buf) {
            goto load_rom_fail_1;
        }

        fr = f_open(romf, romfile, FA_OPEN_EXISTING | FA_READ);
        if (fr) {
            goto load_rom_fail_1;
        }
        rom_size = f_size(romf);

/*
        fr = f_read(romf, &testread, 4, &btr);
        if ((fr) || (btr != 4)) {
            goto load_rom_fail_2;
        }
        if ((testread & 0xF0F0F0F0) == 0) {
            rom_isPacked = false;
            emu_rom_addr = pvPortMalloc(rom_size);

        } else {
            rom_isPacked = true;
            emu_rom_addr = pvPortMalloc(rom_size * 2);
        }
*/
        emu_rom_addr = pvPortMalloc(rom_size);
        if (!emu_rom_addr) {
            goto load_rom_fail_1;
        }
/*
        f_lseek(romf, 0);
        char rbyte;
        size_t rdcnt = 0;
        uint8_t *pch = emu_rom_addr;
        uint8_t *u8pt = rdrom_buf;
        size_t wbcnt = 0;*/
        /*
        if (rom_isPacked) 
        {

            while (rdcnt != rom_size) {

                if (rdcnt + LOAD_BLOCK_SIZE <= rom_size) {
                    fr = f_read(romf, rdrom_buf, LOAD_BLOCK_SIZE, &btr);
                    if ((fr) || (btr != LOAD_BLOCK_SIZE)) {
                        goto load_rom_fail_2;
                    }
                    wbcnt = 0;
                    u8pt = rdrom_buf;
                    while (wbcnt < LOAD_BLOCK_SIZE) {
                        *pch++ = *u8pt & 0xF;
                        *pch++ = *u8pt >> 4;
                        u8pt++;
                        wbcnt++;
                    }
                    rdcnt += LOAD_BLOCK_SIZE;

                } else 
                {

                    fr = f_read(romf, &rbyte, 1, &btr);
                    if ((fr) || (btr != 1)) {
                        goto load_rom_fail_2;
                    }

                    *pch++ = rbyte & 0xF;
                    *pch++ = rbyte >> 4;
                    rdcnt++;
                }

                if (rdcnt % 2048 == 0) {
                    printf("Load rom:%d/%d\n", rdcnt, rom_size);
                }
            }
        } else 
        */
        {
            fr = f_read(romf, emu_rom_addr, rom_size, &btr);
            if ((fr) || (btr != rom_size)) {
                goto load_rom_fail_2;
            }
        }

        vPortFree(rdrom_buf);

        return 0;
    }

load_rom_fail_2:
    vPortFree(emu_rom_addr);

load_rom_fail_1:
    f_close(romf);
    vPortFree(romf);
    vPortFree(rdrom_buf);
    rom_size = 0;
    emu_rom_addr = NULL;
    return -1;
}

//static uint32_t maxsz = 0;
static uint8_t data_packed[32];
static uint8_t data_unpacked[sizeof(data_packed) * 2];

uint8_t rom_read_nibbles(void *dst,uint32_t nibble_addr, uint32_t size)
{
    if(nibble_addr >= 0x70000000)
    {
        //if(nibble_addr < 0x70000000 + rom_size * 2)
        {
            uint32_t acc_addr = nibble_addr - 0x70000000;

            memcpy(data_packed, (void *)(acc_addr / 2 + emu_rom_addr) , sizeof(data_packed));

            for(int i =0, j =0; i < sizeof(data_packed); i++)
            {
                data_unpacked[j++] = data_packed[i] & 0xF;
                data_unpacked[j++] = data_packed[i] >> 4;
            }
            memcpy(dst, &data_unpacked[nibble_addr & 1] , size);

/*
            if(nibble_addr & 1)
            {
                memcpy(dst, &data_unpacked[1] , size);
            }else
            {
                memcpy(dst, data_unpacked , size);
            }
            */

            //memcpy(dst, (void *)(acc_addr + emu_rom_addr) , size);

/*
            if(size > maxsz)
            {
                maxsz = size;
                printf("rddmsz:%d\n", size);
            }
            */

            return 0;
        }
    }else{
        memcpy(dst, (void *)nibble_addr, size);
    }


    //printf("RD ERR:%08x %08x\n", nibble_addr, emu_rom_addr);
}





void *get_emu_rom_addr() {
    return emu_rom_addr;
}

size_t get_rom_size() {
    return rom_size;
}
