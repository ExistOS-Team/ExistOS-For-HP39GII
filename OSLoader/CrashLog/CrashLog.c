#include "CrashLog.h"
#include "../debug.h"
#include "../HAL/board_up.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// 崩溃日志存储
static crash_log_t crash_log;
static bool crash_log_available = false;
static bool crash_data_sent = false;

// 获取当前时间戳
static uint32_t get_timestamp(void) {
    // 使用系统运行时间作为时间戳
    extern uint32_t portBoardGetTime_ms(void);
    return portBoardGetTime_ms() / 1000; // 转换为秒
}

// 初始化崩溃日志系统
void crash_log_init(void) {
    crash_log_available = false;
    crash_data_sent = false;
    memset(&crash_log, 0, sizeof(crash_log_t));
    printf("OSLoader Crash log system initialized\n");
}

// 获取当前栈跟踪
uint32_t crash_log_get_stack_trace(uint8_t* buffer, uint32_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return 0;
    }
    
    // 获取当前栈指针
    uint32_t sp;
    __asm volatile ("mov %0, sp" : "=r" (sp));
    
    // 复制栈数据
    uint32_t stack_size = buffer_size;
    if (stack_size > 256) {
        stack_size = 256;
    }
    
    memcpy(buffer, (void*)sp, stack_size);
    return stack_size;
}

// 记录崩溃日志
void crash_log_save(crash_type_t type, const char* file, int line, const char* description, 
                   uint32_t pc, uint32_t lr, uint32_t sp, uint32_t psr, const char* task_name) {
    // 填充崩溃日志
    memset(&crash_log, 0, sizeof(crash_log_t));
    
    crash_log.timestamp = get_timestamp();
    crash_log.type = type;
    crash_log.pc = pc;
    crash_log.lr = lr;
    crash_log.sp = sp;
    crash_log.psr = psr;
    crash_log.line = line;
    
    if (file) {
        strncpy(crash_log.file, file, sizeof(crash_log.file) - 1);
        crash_log.file[sizeof(crash_log.file) - 1] = '\0';
    }
    
    if (description) {
        strncpy(crash_log.description, description, sizeof(crash_log.description) - 1);
        crash_log.description[sizeof(crash_log.description) - 1] = '\0';
    }
    
    if (task_name) {
        strncpy(crash_log.task_name, task_name, sizeof(crash_log.task_name) - 1);
        crash_log.task_name[sizeof(crash_log.task_name) - 1] = '\0';
    }
    
    // 获取栈跟踪
    crash_log.stack_trace_size = crash_log_get_stack_trace(crash_log.stack_trace, sizeof(crash_log.stack_trace));
    
    crash_log_available = true;
    crash_data_sent = false;
    
    printf("OSLoader Crash log saved: %s\n", description);
}

// 记录崩溃日志（兼容旧接口）
void crash_log_record(crash_type_t type, const char* description, const char* file, int line) {
    crash_handler(type, file, line, description);
}

// 添加栈跟踪信息
void crash_log_append_stack_trace(const char* info) {
    // 这个函数的实现可以简单地将信息打印到控制台
    // 或者可以将信息添加到崩溃日志结构中
    printf("Stack trace info: %s\n", info ? info : "NULL");
}

// 系统崩溃处理函数
void crash_handler(crash_type_t type, const char* file, int line, const char* description, ...) {
    // 获取当前寄存器值
    uint32_t pc, lr, sp, psr;
    
    __asm volatile ("mov %0, pc" : "=r" (pc));
    __asm volatile ("mov %0, lr" : "=r" (lr));
    __asm volatile ("mov %0, sp" : "=r" (sp));
    // ARM926EJ-S 不支持 PSR 寄存器，设为0
    psr = 0;
    
    // 获取当前任务名称
    char task_name[16] = "Unknown";
    extern TaskHandle_t xTaskGetCurrentTaskHandle(void);
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    if (current_task) {
        extern char* pcTaskGetName(TaskHandle_t xTaskToQuery);
        char* name = pcTaskGetName(current_task);
        if (name) {
            strncpy(task_name, name, sizeof(task_name) - 1);
            task_name[sizeof(task_name) - 1] = '\0';
        }
    }
    
    // 格式化描述信息
    char formatted_desc[128];
    if (description) {
        va_list args;
        va_start(args, description);
        vsnprintf(formatted_desc, sizeof(formatted_desc), description, args);
        va_end(args);
    } else {
        formatted_desc[0] = '\0';
    }
    
    // 保存崩溃日志
    crash_log_save(type, file, line, formatted_desc, pc, lr, sp, psr, task_name);
    
    // 打印崩溃信息
    printf("\n\n=== OSLOADER CRASH ===\n");
    printf("Type: %d\n", type);
    printf("Description: %s\n", formatted_desc);
    printf("File: %s:%d\n", file ? file : "Unknown", line);
    printf("Task: %s\n", task_name);
    printf("PC: 0x%08lX\n", pc);
    printf("LR: 0x%08lX\n", lr);
    printf("SP: 0x%08lX\n", sp);
    printf("PSR: 0x%08lX\n", psr);
    printf("===================\n\n");
}

// 检查是否有崩溃数据需要发送
bool crash_log_has_data_to_send(void) {
    return crash_log_available && !crash_data_sent;
}

// 获取崩溃数据
bool crash_log_get_data_for_send(crash_log_t* log) {
    if (!log || !crash_log_available || crash_data_sent) {
        return false;
    }
    
    memcpy(log, &crash_log, sizeof(crash_log_t));
    return true;
}

// 发送崩溃数据
void crash_log_send_data(void) {
    if (!crash_log_available || crash_data_sent) {
        return;
    }
    
    // 发送崩溃数据的实现
    // 这里通过调试串口发送崩溃信息
    printf("\n=== CRASH DATA ===\n");
    printf("Timestamp: %lu\n", crash_log.timestamp);
    printf("Type: %d\n", crash_log.type);
    printf("PC: 0x%08lX\n", crash_log.pc);
    printf("LR: 0x%08lX\n", crash_log.lr);
    printf("SP: 0x%08lX\n", crash_log.sp);
    printf("PSR: 0x%08lX\n", crash_log.psr);
    printf("Task: %s\n", crash_log.task_name);
    printf("File: %s\n", crash_log.file);
    printf("Line: %d\n", crash_log.line);
    printf("Description: %s\n", crash_log.description);
    printf("Stack Trace Size: %lu\n", crash_log.stack_trace_size);
    
    // 发送栈跟踪数据（十六进制格式）
    if (crash_log.stack_trace_size > 0) {
        printf("Stack Trace:\n");
        
        for (uint32_t i = 0; i < crash_log.stack_trace_size; i += 16) {
            printf("%08lX: ", (uint32_t)(crash_log.sp + i));
            
            for (uint32_t j = 0; j < 16 && (i + j) < crash_log.stack_trace_size; j++) {
                printf("%02X ", crash_log.stack_trace[i + j]);
            }
            printf("\n");
        }
    }
    printf("==================\n\n");
    
    // 标记数据已发送
    crash_data_sent = true;
}