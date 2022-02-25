#ifndef __BOARD_UP_H__
#define __BOARD_UP_H__

#include <stdint.h>
#include <stdbool.h>

uint32_t nsToCycles(uint32_t nstime, uint32_t period, uint32_t min);


void portBoardInit(void);
void portDelayus(uint32_t us);


void boardInit(void);


uint32_t portBoardGetTime_us(void);
void portBoardPowerOff(void);
void portBoardReset(void);

#endif