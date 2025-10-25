#ifndef SERVICES_H
#define SERVICES_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// USB服务
void vTaskTinyUSB(void *pvParameters);

// MTD服务
void vMTDSvc(void *pvParameters);

// FTL服务
void vFTLSvc(void *pvParameters);

// 键盘服务
void vKeysSvc(void *pvParameters);

// 虚拟内存管理服务
void vVMMgrSvc(void *pvParameters);

// LLAPI服务
void vLLAPISvc(void *pvParameters);

// 显示服务
void vDispSvc(void *pvParameters);

// 外部变量声明
extern bool g_vm_inited;

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_H */