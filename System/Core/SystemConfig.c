#include "SystemConfig.h"
#include "UI_Config.h"
#include "filesystem/Fatfs/ff.h"
#include "sys_llapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局配置实例
static SystemConfig g_config = {0};

// 简单的JSON解析辅助函数
static int find_json_value(const char* json, const char* key, char* value, int max_len) {
    char search_key[64];
    sprintf(search_key, "\"%s\":", key);
    
    char* key_pos = strstr(json, search_key);
    if (!key_pos) return -1;
    
    char* value_start = key_pos + strlen(search_key);
    // 跳过空格
    while (*value_start == ' ') value_start++;
    
    // 处理字符串值
    if (*value_start == '"') {
        value_start++;
        char* value_end = strchr(value_start, '"');
        if (!value_end) return -1;
        
        int len = value_end - value_start;
        if (len >= max_len) len = max_len - 1;
        strncpy(value, value_start, len);
        value[len] = '\0';
        return 0;
    }
    // 处理数字值
    else {
        char* value_end = value_start;
        while (*value_end != '\0' && *value_end != ',' && *value_end != '}' && *value_end != ' ') value_end++;
        
        int len = value_end - value_start;
        if (len >= max_len) len = max_len - 1;
        strncpy(value, value_start, len);
        value[len] = '\0';
        return 0;
    }
}

// 初始化配置系统
void config_init(void) {
    // 设置默认配置
    config_reset_to_default();
    
    // 尝试加载配置文件
    config_load();
}

// 重置配置为默认值
void config_reset_to_default(void) {
    g_config.language = DEFAULT_LANGUAGE;
    g_config.power_save = DEFAULT_POWER_SAVE;
    g_config.enable_charge = DEFAULT_ENABLE_CHARGE;
    g_config.charging = false;
    g_config.enable_mem_swap = DEFAULT_ENABLE_MEM_SWAP;
    g_config.mem_swap = false;
    g_config.rtc_time = ll_rtc_get_sec();
    g_config.dirty = false;
}

// 加载配置文件
void config_load(void) {
    FIL file;
    FRESULT res;
    
    // 尝试打开配置文件
    res = f_open(&file, CONFIG_FILE_PATH, FA_READ);
    if (res != FR_OK) {
        // 文件不存在，使用默认配置
        return;
    }
    
    // 读取文件内容
    char* buffer = (char*)malloc(1024);
    if (!buffer) {
        f_close(&file);
        return;
    }
    
    UINT bytes_read;
    res = f_read(&file, buffer, 1023, &bytes_read);
    buffer[bytes_read] = '\0';
    f_close(&file);
    
    if (res != FR_OK) {
        free(buffer);
        return;
    }
    
    // 解析JSON
    char value[32];
    
    // 解析语言设置
    if (find_json_value(buffer, "language", value, sizeof(value)) == 0) {
        g_config.language = atoi(value);
        if (g_config.language != UI_LANG_EN && g_config.language != UI_LANG_CN) {
            g_config.language = DEFAULT_LANGUAGE;
        }
    }
    
    // 解析电源节省模式
    if (find_json_value(buffer, "power_save", value, sizeof(value)) == 0) {
        g_config.power_save = value[0];
        if (g_config.power_save != ' ' && g_config.power_save != 'A' && g_config.power_save != 'B') {
            g_config.power_save = DEFAULT_POWER_SAVE;
        }
    }
    
    // 解析充电设置
    if (find_json_value(buffer, "enable_charge", value, sizeof(value)) == 0) {
        g_config.enable_charge = (strcmp(value, "true") == 0);
    }
    
    // 解析内存交换设置
    if (find_json_value(buffer, "enable_mem_swap", value, sizeof(value)) == 0) {
        g_config.enable_mem_swap = (strcmp(value, "true") == 0);
    }
    
    // 解析RTC时间（注释掉这部分代码，不从配置文件读取RTC时间）
    /*
    if (find_json_value(buffer, "rtc_time", value, sizeof(value)) == 0) {
        g_config.rtc_time = (uint32_t)strtoul(value, NULL, 10);
        // 设置RTC时间
        ll_rtc_set_sec(g_config.rtc_time);
    }
    */
    
    // 从RTC硬件获取当前时间
    g_config.rtc_time = ll_rtc_get_sec();
    
    free(buffer);
}

// 保存配置到文件
void config_save(void) {
    FIL file;
    FRESULT res;
    
    // 获取当前RTC时间
    g_config.rtc_time = ll_rtc_get_sec();
    
    // 创建JSON字符串
    char json_buffer[512];
    sprintf(json_buffer, 
        "{\n"
        "  \"language\": %d,\n"
        "  \"power_save\": \"%c\",\n"
        "  \"enable_charge\": %s,\n"
        "  \"enable_mem_swap\": %s,\n"
        "  \"rtc_time\": %u\n"
        "}",
        g_config.language,
        g_config.power_save,
        g_config.enable_charge ? "true" : "false",
        g_config.enable_mem_swap ? "true" : "false",
        (unsigned int)g_config.rtc_time
    );
    
    // 打开文件进行写入
    res = f_open(&file, CONFIG_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        return;
    }
    
    // 写入JSON数据
    UINT bytes_written;
    res = f_write(&file, json_buffer, strlen(json_buffer), &bytes_written);
    f_close(&file);
    
    // 如果写入失败，尝试删除可能损坏的文件
    if (res != FR_OK || bytes_written != strlen(json_buffer)) {
        f_unlink(CONFIG_FILE_PATH);
    }
}

// 获取配置结构体指针
SystemConfig* config_get(void) {
    return &g_config;
}

// 获取和设置特定配置项的函数
int config_get_language(void) {
    return g_config.language;
}

void config_set_language(int lang) {
    if (lang == UI_LANG_EN || lang == UI_LANG_CN) {
        g_config.language = lang;
        config_save();
    }
}

char config_get_power_save(void) {
    return g_config.power_save;
}

void config_set_power_save(char mode) {
    if (mode == ' ' || mode == 'A' || mode == 'B') {
        g_config.power_save = mode;
        config_save();
    }
}

bool config_get_enable_charge(void) {
    return g_config.enable_charge;
}

void config_set_enable_charge(bool enable) {
    g_config.enable_charge = enable;
    config_save();
}

// 获取/设置充电状态
bool config_get_charging(void) {
    return g_config.charging;
}

void config_set_charging(bool charging) {
    g_config.charging = charging;
    config_save();
}

bool config_get_enable_mem_swap(void) {
    return g_config.enable_mem_swap;
}

void config_set_enable_mem_swap(bool enable) {
    g_config.enable_mem_swap = enable;
    config_save();
}