#pragma once
#include "lvgl.h"
#define INDICATE_LEFT      (1 << 0)
#define INDICATE_RIGHT     (1 << 1)
#define INDICATE_A__Z      (1 << 2)
#define INDICATE_a__z      (1 << 3)
#define INDICATE_BUSY      (1 << 4)
#define INDICATE_TX        (1 << 5)
#define INDICATE_RX        (1 << 6)

#define SYSTEMUI_MSGBOX_BUTTON_OK       (0)
#define SYSTEMUI_MSGBOX_BUTTON_CANCAL   (1 << 1)

#ifdef __cplusplus
extern "C" {
#endif


void SystemUIInit();

uint32_t SystemUIMsgBox(lv_obj_t *parent,char *msg, char *title, uint32_t button);


void SystemUISuspend();
void SystemUIResume();
void SystemUIEditing(bool edit);
lv_indev_t *SystemGetInKeypad();
void SystemUISetBusy(bool enable);


#ifdef __cplusplus
}
#endif
