
#include "FreeRTOS.h"
#include "task.h"

#include "ff.h"

#include "stdio.h"

#define LOAD_BLOCK_SIZE 65536

static void *rom_addr = NULL;

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

        fr = f_read(romf, &testread, 4, &btr);
        if ((fr) || (btr != 4)) {
            goto load_rom_fail_2;
        }
        if ((testread & 0xF0F0F0F0) == 0) {
            rom_isPacked = false;
            rom_addr = pvPortMalloc(rom_size);

        } else {
            rom_isPacked = true;
            rom_addr = pvPortMalloc(rom_size * 2);
        }

        if (!rom_addr) {
            goto load_rom_fail_1;
        }

        f_lseek(romf, 0);
        char rbyte;
        size_t rdcnt = 0;
        uint8_t *pch = rom_addr;
        uint8_t *u8pt = rdrom_buf;
        size_t wbcnt = 0;
        if (rom_isPacked) {
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
        } else {
            fr = f_read(romf, rom_addr, rom_size, &btr);
            if ((fr) || (btr != rom_size)) {
                goto load_rom_fail_2;
            }
        }

        vPortFree(rdrom_buf);

        return 0;
    }

load_rom_fail_2:
    vPortFree(rom_addr);

load_rom_fail_1:
    f_close(romf);
    vPortFree(romf);
    vPortFree(rdrom_buf);
    rom_size = 0;
    rom_addr = NULL;
    return -1;
}

void *get_rom_addr() {
    return rom_addr;
}

size_t get_rom_size() {
    return rom_size;
}
