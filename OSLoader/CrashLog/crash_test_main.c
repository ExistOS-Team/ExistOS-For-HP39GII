#include "FreeRTOS.h"
#include "task.h"
#include "CrashLog.h"
#include "CrashSender.h"

// 测试任务，用于验证崩溃日志系统
void crash_test_task(void *pvParameters) {
    // 延迟一段时间以确保系统稳定
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 测试记录不同类型的崩溃
    crash_log_record(CRASH_TYPE_HARD_FAULT, "Test hard fault", __FILE__, __LINE__);
    
    // 删除任务
    vTaskDelete(NULL);
}

// 主函数，用于测试崩溃日志系统
int crash_test_main(void) {
    // 初始化崩溃日志系统
    crash_log_init();
    
    // 初始化崩溃发送器
    crash_sender_init();
    
    // 创建测试任务
    xTaskCreate(crash_test_task, "Crash Test", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 8, NULL);
    
    return 0;
}