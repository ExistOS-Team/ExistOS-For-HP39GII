#include "./CrashSender.h"
#include "CrashLog.h"
#include "../debug.h"
#include "../HAL/board_up.h"
#include <stdio.h>

// 崩溃发送任务句柄
static TaskHandle_t crash_sender_task_handle = NULL;

// 崩溃发送任务
static void crash_sender_task(void* pvParameters) {
    (void)pvParameters; // 避免未使用参数警告
    
    // 初始化崩溃日志系统
    crash_log_init();
    
    printf("OSLoader Crash sender task started\n");
    
    // 每5秒检查并发送一次崩溃数据
    while (1) {
        // 检查是否有崩溃数据需要发送
        if (crash_log_has_data_to_send()) {
            printf("Sending crash data...\n");
            crash_log_send_data();
            printf("Crash data sent\n");
        }
        
        // 延迟5秒
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// 初始化崩溃发送系统
void crash_sender_init(void) {
    // 创建崩溃发送任务
    BaseType_t result = xTaskCreate(
        crash_sender_task,           // 任务函数
        "CrashSender",               // 任务名称
        configMINIMAL_STACK_SIZE * 4, // 栈大小
        NULL,                        // 任务参数
        tskIDLE_PRIORITY + 2,        // 任务优先级
        &crash_sender_task_handle    // 任务句柄
    );
    
    if (result != pdPASS) {
        printf("Failed to create crash sender task\n");
    } else {
        printf("Crash sender task created successfully\n");
    }
}

// 获取崩溃发送任务句柄
TaskHandle_t crash_sender_get_task_handle(void) {
    return crash_sender_task_handle;
}