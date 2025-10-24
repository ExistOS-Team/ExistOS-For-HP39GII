#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 崩溃日志类型定义
typedef enum {
    CRASH_TYPE_ASSERT = 0,      // 断言失败
    CRASH_TYPE_STACK_OVERFLOW,  // 栈溢出
    CRASH_TYPE_OUT_OF_MEMORY,   // 内存不足
    CRASH_TYPE_HARD_FAULT,      // 硬件故障
    CRASH_TYPE_UNKNOWN          // 未知错误
} crash_type_t;

// 崩溃日志信息结构
typedef struct {
    uint32_t timestamp;         // 崩溃时间戳
    crash_type_t type;          // 崩溃类型
    uint32_t pc;                // 程序计数器
    uint32_t lr;                // 链接寄存器
    uint32_t sp;                // 栈指针
    uint32_t psr;               // 程序状态寄存器
    uint32_t task_name_addr;    // 任务名称地址
    char task_name[16];         // 任务名称
    char file[64];              // 文件名
    int line;                   // 行号
    char description[128];      // 崩溃描述
    uint8_t stack_trace[256];   // 栈跟踪数据
    uint32_t stack_trace_size;  // 栈跟踪数据大小
} crash_log_t;

// 初始化崩溃日志系统
void crash_log_init(void);

// 记录崩溃日志
void crash_log_save(crash_type_t type, const char* file, int line, const char* description, 
                   uint32_t pc, uint32_t lr, uint32_t sp, uint32_t psr, const char* task_name);

// 获取崩溃日志数量
int crash_log_get_count(void);

// 获取崩溃日志
bool crash_log_get(int index, crash_log_t* log);

// 清除所有崩溃日志
void crash_log_clear_all(void);

// 创建崩溃日志文件
bool crash_log_create_file(const crash_log_t* log);

// 系统崩溃处理函数（替换原有的PANIC宏）
void crash_handler(crash_type_t type, const char* file, int line, const char* description, ...);

// 获取当前栈跟踪
uint32_t crash_log_get_stack_trace(uint8_t* buffer, uint32_t buffer_size);

#ifdef __cplusplus
}
#endif