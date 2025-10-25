#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 配置文件路径
#define CONFIG_FILE_PATH "/existos_config.json"

// 配置项默认值
#define DEFAULT_LANGUAGE UI_LANG_EN
#define DEFAULT_POWER_SAVE ' '  // ' ' = 关闭, 'A' = 低功耗, 'B' = 超低功耗
#define DEFAULT_ENABLE_CHARGE false
#define DEFAULT_ENABLE_MEM_SWAP false

// 系统配置结构体
typedef struct {
    int language;           // 语言设置 (0=英文, 1=中文)
    char power_save;        // 电源节省模式 (' ' = 关闭, 'A' = 低功耗, 'B' = 超低功耗)
    bool enable_charge;     // 是否启用充电
    bool charging;          // 充电状态
    bool enable_mem_swap;   // 是否启用内存交换
    bool mem_swap;          // 内存交换状态
    uint32_t rtc_time;      // RTC时间戳
    bool dirty;             // 配置是否已修改但未保存
} SystemConfig;

// 配置管理函数
void config_init(void);
void config_load(void);
void config_save(void);
void config_reset_to_default(void);
SystemConfig* config_get(void);

// 获取和设置特定配置项的函数
int config_get_language(void);
void config_set_language(int lang);
char config_get_power_save(void);
void config_set_power_save(char mode);
bool config_get_enable_charge(void);
void config_set_enable_charge(bool enable);
// 获取/设置充电状态
bool config_get_charging(void);
void config_set_charging(bool charging);

bool config_get_enable_mem_swap(void);
void config_set_enable_mem_swap(bool enable);

#ifdef __cplusplus
}
#endif