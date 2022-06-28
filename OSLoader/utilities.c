

#include <stdio.h>


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"


#include "tusb.h"

#include "board_up.h"
#include "mtd_up.h"
#include "FTL_up.h"

#include "../debug.h"


static uint32_t _randSeed = 1;
void _randSetSeed(uint32_t seed)
{
  _randSeed = seed;
}


uint32_t _rand(void)
{
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
}

/*

void memTest()
{
  volatile uint32_t *testaddr = (uint32_t *)0x00300000;
  volatile uint32_t chkVal = 0;
  uint32_t seedx = 2344;
  for(;;)
  {
    
  INFO("START RANDOM WRITE MEM\n");
  INFO("TEST 1\n");
  INFO("TEST 1\n");
  INFO("TEST 1\n");
  INFO("TEST 1\n");
  vTaskDelay(pdMS_TO_TICKS(5000));
  seedx++;


    _randSetSeed(seedx);
    for(int i=0; i<1*4*1024/4;i++){
      uint32_t p =  (_rand() & 0xFFFFF) * 2 / 4;
      //INFO("WR ADDR:%08x\n", &testaddr[p] );
      testaddr[p] = _rand();

      if((i % (1 * 1024 / 4)) == 0){ 
        INFO("Random WR:%d/%d\n", i, 1*4*1024/4);
      }

    }

    _randSetSeed(seedx);
    for(int i=0; i<1*4*1024/4;i++){
      uint32_t p =  (_rand() & 0xFFFFF) * 2 / 4;
      //INFO("RD ADDR:%08x\n", &testaddr[p] );
      chkVal = _rand();
      if(testaddr[p] != chkVal){
        INFO("RAND TestERRORAt:%08vlx  %08lx==%08vlx\n", &testaddr[p/4], testaddr[p/4],   chkVal  );
      }

      if((i % (1 * 1024 / 4)) == 0){ 
        INFO("Random RB:%d/%d\n", i, 1*4*1024/4);
      }

    }

  INFO("TEST FINISH 1\n");
  INFO("TEST FINISH 1\n");
  INFO("TEST FINISH 1\n");
  INFO("TEST FINISH 1\n");


  INFO("START WRITE MEM\n");
  _randSetSeed(seedx);
  for(int i = 0; i < (1*1024 * 1024 / 4) ; i+=(4096+1024)){
    testaddr[i] = _rand();
    if((i % (32 * 1024 / 4)) == 0){
      INFO("WR:%d/%d\n", i, (1 * 1024 * 1024 / 4));
    }
  }

  INFO("MEM WRITE Finish, readback test\n");

  _randSetSeed(seedx);
  for(int i = 0; i < (1*1024 * 1024 / 4) ; i+=(4096+1024)){
    chkVal =  _rand();
    if(testaddr[i] != chkVal){
      INFO("TestERRORAt:%08vlx  %08vlx==%08vlx\n", &testaddr[i], testaddr[i],   chkVal  );
    }
    if((i % (32 * 1024 / 4)) == 0){
      INFO("RB TEST:%d/%d\n", i, (1 * 1024 * 1024 / 4));
    }
  }


  INFO("TEST FINISH 2\n");
  INFO("TEST FINISH 2\n");
  INFO("TEST FINISH 2\n");
  INFO("TEST FINISH 2\n");
seedx++;

  INFO("START WRITE MEM\n");
  _randSetSeed(seedx);
  for(int i = 0; i < (1*4 * 1024 / 4) ; i++){
    testaddr[i] = _rand();
    if((i % (1 * 1024 / 4)) == 0){
      INFO("WR:%d/%d\n", i, (1 * 4 * 1024 / 4));
    }
  }

  INFO("MEM WRITE Finish, readback test\n");

  _randSetSeed(seedx);
  for(int i = 0; i < (1*4 * 1024 / 4) ; i++){
    chkVal =  _rand();
    if(testaddr[i] != chkVal){
      INFO("TestERRORAt:%08vlx  %08vlx==%08vlx\n", &testaddr[i], testaddr[i],   chkVal  );
    }
    if((i % (1 * 1024 / 4)) == 0){
      INFO("RB TEST:%d/%d\n", i, (1 * 4 * 1024 / 4));
    }
  }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void checkFlash()
{

    uint8_t *buf = pvPortMalloc(4096);
    printf("FLASH TEST START\n");
    for(int block = 0; block < 1024; block++){
      printf("ERASE BLOCK:%d\n", block);
      MTD_ErasePhyBlock(block);
      printf("START WRITE BLOCK...\n");
      
      _randSetSeed(1);
      uint32_t *datTest = (uint32_t *)buf;
      for(int ptr = 0; ptr < 2048/4; ptr++){
        datTest[ptr] = _rand();
      }

      uint32_t startTime = 0;
      uint32_t endTime = 0;
      uint32_t sumTime = 0;
      for(int page = 0; page < 64; page++)
      {
        startTime = portBoardGetTime_us();
        MTD_WritePhyPage(page + block*64, buf);
        endTime = portBoardGetTime_us();
        sumTime += (endTime - startTime);
      }

      printf("BLOCK %d Write Finish: %lu KB/s \n", block, 128 * 1024 * 1000/sumTime);

      printf("READ BACK TEST\n");
      volatile uint32_t RB_DAT;
      for(int page = 0; page < 64; page++)
      {
        
        _randSetSeed(1);
        for(int ptr = 0; ptr < 2048/4; ptr++){
          volatile uint32_t testVal = _rand();
          MTD_ReadPhyPage(page + block*64, ptr * 4, 4, (uint8_t *)&RB_DAT);
          if(RB_DAT != testVal){
            printf("   TEST ERROR:[block:%d, page:%d, off:%ld][%08lx != %08lx]\n",block, page, ptr*4, testVal, RB_DAT);
          }
        }
      }
      printf("CHECK PASSED.\n");

    }
}
*/
