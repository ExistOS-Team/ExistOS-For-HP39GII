#pragma once
#include <stdint.h>
void rtc_init();
void portRTC_init();


uint32_t rtc_get_seconds();

void rtc_set_seconds(uint32_t s);



