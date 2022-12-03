#pragma once
//#include "lvgl.h"

#define LCD_PIX_W   256
#define LCD_PIX_H   127

#define INDICATE_LEFT      (1 << 0)
#define INDICATE_RIGHT     (1 << 1)
#define INDICATE_A__Z      (1 << 2)
#define INDICATE_a__z      (1 << 3)
#define INDICATE_BUSY      (1 << 4)
#define INDICATE_TX        (1 << 5)
#define INDICATE_RX        (1 << 6)


#ifdef __cplusplus
extern "C" {
#endif



void SystemUIRefresh() ;
void SystemUISuspend();
void SystemUIResume();

void UI_Task(void *_);


#ifdef __cplusplus
}
#endif
