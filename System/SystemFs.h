#pragma once


#ifdef __cplusplus
extern "C" {
#endif
void SystemFSInit() ;
void *GetFsObj();
#ifdef __cplusplus
}
#endif

#define FS_FLASH_PATH        "/"
#define FS_LVGL_FLASH_PATH   "A:"

#define FS_SYSTEM_PATH       "System"
#define FS_FONTS_PATH        "fonts"

