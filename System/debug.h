#pragma once

#include <stdio.h>

#define PANIC(...)          \
    do {                     \
        printf(__VA_ARGS__); \
        while (1)            \
            ;                \
    } while (0)

#define INFO(...)            \
    do {                     \
        printf(__VA_ARGS__); \
    } while (0)
