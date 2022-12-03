
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


#define ALLOCATE_BLOCK_SIZE (32768)
#define MAX_BLOCK    (32)



static uint32_t _randSeed = 1;
static void testRandSetSeed(uint32_t seed)
{
  _randSeed = seed;
}

static uint32_t testRand(void)
{
    return (_randSeed++)& 0xF8;
    /*
    uint32_t value;
    //Use a linear congruential generator (LCG) to update the state of the PRNG
    _randSeed *= 1103515245;
    _randSeed += 12345;
    value = (_randSeed >> 16) & 0x07FF;
    _randSeed *= 1103515245;
    _randSeed += 12345;
    value <<= 10;
    value |= (_randSeed >> 16) & 0x03FF;
    _randSeed *= 1103515245;
    _randSeed += 12345;
    value <<= 10;
    value |= (_randSeed >> 16) & 0x03FF;
    //Return the random value
    return value;   
     */
}


static int blkTestWrite(void *adr)
{
    testRandSetSeed((uint32_t)adr);
    for(int i = 0; i < ALLOCATE_BLOCK_SIZE; i++)
    {
        ((uint8_t *)adr)[i] = testRand();
    }
    return 0;
}

static void *blkCheck(void *adr)
{
    uint8_t tst_val;
    testRandSetSeed((uint32_t)adr);
    for(int i = 0; i < ALLOCATE_BLOCK_SIZE; i++)
    {
        tst_val = testRand();
        if(((uint8_t *)adr)[i] != tst_val)
        {
            
            printf("ERROR Detected at: %08x, %02x == %02x\n", &((uint8_t *)adr)[i], tst_val, ((uint8_t *)adr)[i] );
            return &((uint8_t *)adr)[i];
        }
    }
    return NULL;
}


void memtest(uint32_t testSize)
{
    void *addr[MAX_BLOCK];

    uint32_t blks = testSize / ALLOCATE_BLOCK_SIZE;
    if(blks == 0)
    {
        blks++;
    }

    for(int i = 0 ; i < blks; i++)
    {
        addr[i] = malloc(ALLOCATE_BLOCK_SIZE);
        if(addr[i] == NULL)
        {
            printf("Failed to allocate mem block:%d\n", i);
            for(int j = i - 1; j >= 0; j--)
            {
                free(addr[j]);
            }
            return;
        }
    }

    for(int i = 0 ; i < blks; i++)
    {
        blkTestWrite(addr[i]);
    }

    void *fadr;
    for(int i = 0 ; i < blks; i++)
    {
        fadr = blkCheck(addr[i]);
        if(fadr)
        {
            
            break;
        }
    }

    for(int i = 0 ; i < blks; i++)
    {
        free(addr[i]);
    }

    printf("MEMTEST FINISH.\n");
    return;
}



