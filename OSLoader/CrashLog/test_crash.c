#include "CrashLog.h"
#include "CrashSender.h"
#include "FreeRTOS.h"
#include "task.h"

// 测试任务，用于模拟系统崩溃
void test_crash_task(void *pvParameters) {
    // 初始化崩溃日志系统
    crash_log_init();
    
    // 初始化崩溃发送器
    crash_sender_init();
    
    // 延迟一段时间以确保系统稳定
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 模拟一个系统崩溃
    crash_log_record(CRASH_TYPE_HARD_FAULT, "Test crash simulation", __FILE__, __LINE__);
    
    // 这里应该永远不会执行到
    vTaskDelete(NULL);
}

// 创建测试崩溃任务
void create_test_crash_task(void) {
    xTaskCreate(test_crash_task, "Test Crash", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 8, NULL);
}