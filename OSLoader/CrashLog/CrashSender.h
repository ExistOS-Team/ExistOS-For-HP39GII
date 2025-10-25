#ifndef __CRASH_SENDER_H__
#define __CRASH_SENDER_H__

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初始化崩溃发送系统
void crash_sender_init(void);

// 获取崩溃发送任务句柄
TaskHandle_t crash_sender_get_task_handle(void);

#ifdef __cplusplus
}
#endif

#endif // __CRASH_SENDER_H__