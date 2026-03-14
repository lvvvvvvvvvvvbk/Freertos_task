#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

#define APP_FW_VERSION_MAJOR 1u
#define APP_FW_VERSION_MINOR 1u
#define APP_FW_VERSION_PATCH 0u

/* 设为 1 时走标准 FreeRTOS API（需工程中已集成 FreeRTOS 头文件和源码） */
#ifndef APP_USE_FREERTOS
#define APP_USE_FREERTOS 0
#endif

#define APP_EVENT_QUEUE_LEN   64u
#define APP_LOG_QUEUE_LEN     128u

#define CAN_RX_TIMEOUT_MS     200u
#define METER_POLL_PERIOD_MS  1000u
#define CLOUD_REPORT_MS       2000u

#define CHARGING_MAX_VOLTAGE_MV  750000u
#define CHARGING_MAX_CURRENT_MA  250000u
#define CHARGING_MAX_TEMP_MC     85000

/* 标准 FreeRTOS 任务优先级建议（数值越大优先级越高） */
#define PRIO_TASK_SAFETY   6u
#define PRIO_TASK_CAN_RX   5u
#define PRIO_TASK_FSM      4u
#define PRIO_TASK_METER    3u
#define PRIO_TASK_CLOUD    2u
#define PRIO_TASK_OTA      2u
#define PRIO_TASK_LOG      1u

#define STACK_WORDS_SMALL  256u
#define STACK_WORDS_MEDIUM 384u
#define STACK_WORDS_LARGE  512u

#endif
