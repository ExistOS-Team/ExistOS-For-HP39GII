#ifndef __BOARD_UP_H__
#define __BOARD_UP_H__

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"

uint64_t nsToCycles(uint64_t nstime, uint64_t period, uint64_t min) ;

bool driverWaitTrueF(bool (*f)(), TickType_t timeout);
bool driverWaitFalseF(bool (*f)(), TickType_t timeout);

void portBoardInit(void);
void portDelayus(uint32_t us);
void portDelayms(uint32_t ms);

void boardInit(void);

void portPowerInit();
uint32_t portGetBatterVoltage_mv();
uint32_t portLRADCConvCh(uint32_t ch, uint32_t samples);
uint32_t portGetBatteryMode();
uint32_t portGetPWRSpeed();
void portChargeEnable(bool enable);


uint32_t portBoardGetTime_us(void);
uint32_t portBoardGetTime_ms(void);
uint32_t portBoardGetTime_s(void);

void portBoardPowerOff(void);
void portBoardReset(void);
void portLRADCEnable(bool enable ,uint32_t ch);
void portLRADC_init();

void setCPUDivider(uint32_t div);
void setCPUFracDivider(uint32_t div);
void setHCLKDivider(uint32_t div);
void portGetCoreFreqDIV(uint32_t *CPU_DIV, uint32_t *CPU_Frac, uint32_t *HCLK_DIV);


void enterSlowDown();
void exitSlowDown();
void slowDownEnable(bool enable);
void setSlowDownMinCpuFrac(uint8_t frac);

void stmp_audio_init();
void pcm_play();


#endif