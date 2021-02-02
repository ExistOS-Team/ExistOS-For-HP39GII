#ifndef _PROCESSES_MAN_H
#define _PROCESSES_MAN_H

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum{
    STATUS_RUNNING,
    STATUS_SUSPEND
}task_status;

typedef int PID_t;
typedef int TID_t;

typedef struct ThreadsInfo
{
    struct ThreadsInfo *next_info;
    PID_t belong_PID;
    TID_t tid;
    task_status status;
    TaskHandle_t task_handle;
    
}threads_info;

typedef struct ProcessesInfo
{
    struct ProcessesInfo *next_info;
    PID_t PID;
    char process_name[configMAX_TASK_NAME_LEN];
    TaskHandle_t main_thread_task_handle;
    task_status status;
    struct ThreadsInfo *first_thread_info;
}processes_info;



PID_t get_pid_from_task_handle(TaskHandle_t task_handle);
int create_process(char *process_name, char *image_path);

PID_t get_current_running_task_pid();


#endif