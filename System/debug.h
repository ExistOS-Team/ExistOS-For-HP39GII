#pragma once

#include <stdio.h>
#include "CrashLog.h"

// 使用崩溃日志系统替换原有的PANIC宏
#define PANIC(...) \
    crash_handler(CRASH_TYPE_UNKNOWN, __FILE__, __LINE__, __VA_ARGS__)

// 特定类型的崩溃宏
#define ASSERT_FAIL(...) \
    crash_handler(CRASH_TYPE_ASSERT, __FILE__, __LINE__, __VA_ARGS__)

#define STACK_OVERFLOW(...) \
    crash_handler(CRASH_TYPE_STACK_OVERFLOW, __FILE__, __LINE__, __VA_ARGS__)

#define OUT_OF_MEMORY(...) \
    crash_handler(CRASH_TYPE_OUT_OF_MEMORY, __FILE__, __LINE__, __VA_ARGS__)

#define HARD_FAULT(...) \
    crash_handler(CRASH_TYPE_HARD_FAULT, __FILE__, __LINE__, __VA_ARGS__)

#define INFO(...)            \
    do {                     \
        printf(__VA_ARGS__); \
    } while (0)
