#ifndef __CRASH_LOG_H__
#define __CRASH_LOG_H__

#include <stdint.h>
#include <stdbool.h>

// 定义崩溃类型
typedef enum {
    CRASH_TYPE_UNKNOWN = 0,     // 未知错误
    CRASH_TYPE_PANIC,           // PANIC错误
    CRASH_TYPE_ASSERT,          // 断言失败
    CRASH_TYPE_STACK_OVERFLOW,  // 栈溢出
    CRASH_TYPE_OUT_OF_MEMORY,   // 内存不足
    CRASH_TYPE_HARD_FAULT,      // 硬件故障
    CRASH_TYPE_PAGE_FAULT,      // 页面错误
    CRASH_TYPE_BUS_FAULT,       // 总线错误
    CRASH_TYPE_USAGE_FAULT,     // 使用错误
    CRASH_TYPE_MEM_FAULT        // 内存错误
} crash_type_t;

#ifdef __cplusplus
extern "C" {
#endif

// 崩溃日志信息结构
typedef struct {
    uint32_t timestamp;         // 崩溃时间戳
    crash_type_t type;          // 崩溃类型
    uint32_t pc;                // 程序计数器
    uint32_t lr;                // 链接寄存器
    uint32_t sp;                // 栈指针
    uint32_t psr;               // 程序状态寄存器
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

// 获取当前栈跟踪
uint32_t crash_log_get_stack_trace(uint8_t* buffer, uint32_t buffer_size);

// 记录崩溃日志（兼容旧接口）
void crash_log_record(crash_type_t type, const char* description, const char* file, int line);

// 添加栈跟踪信息
void crash_log_append_stack_trace(const char* info);

// 系统崩溃处理函数（替换原有的PANIC宏）
void crash_handler(crash_type_t type, const char* file, int line, const char* description, ...);

// 发送崩溃数据
void crash_log_send_data(void);

// 检查是否有崩溃数据需要发送
bool crash_log_has_data_to_send(void);

// 获取崩溃数据
bool crash_log_get_data_for_send(crash_log_t* log);

#ifdef __cplusplus
}
#endif

#endif // __CRASH_LOG_H__