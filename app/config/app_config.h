#ifndef APP_CONFIG_H //防止头文件被重复包含，避免重复定义
#define APP_CONFIG_H

#include <stdint.h> //引入标准整型类型定义，比如 uint8_t、uint16_t、uint32_t

#define APP_FW_VERSION_MAJOR 1u //应用固件版本号 这里的 u 表示无符号常量 unsigned int
#define APP_FW_VERSION_MINOR 0u
#define APP_FW_VERSION_PATCH 0u

#define APP_EVENT_QUEUE_LEN   64u //定义事件总线或系统事件队列的长度
#define APP_LOG_QUEUE_LEN     128u //定义日志队列长度

#define CAN_RX_TIMEOUT_MS     200u //定义 CAN 接收超时时间
#define METER_POLL_PERIOD_MS  1000u //定义电表轮询周期
#define CLOUD_REPORT_MS       2000u //定义云端上报周期

#define CHARGING_MAX_VOLTAGE_MV  750000u
#define CHARGING_MAX_CURRENT_MA  250000u
#define CHARGING_MAX_TEMP_MC     85000

#endif
