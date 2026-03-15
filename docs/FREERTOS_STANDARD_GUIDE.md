# 我如果想使用标准 FreeRTOS，该如何写？

下面给出一套可直接落地到 `STM32F407 + FreeRTOS` 的写法，对应本仓库代码。

## 1) 任务划分（标准写法）

- `TaskSafety`：高优先级，负责过压/过流/过温/急停采样与故障上报。
- `TaskCANRx`：高优先级，处理 CAN 接收中断后的报文解析。
- `TaskFSM`：中高优先级，统一消费事件队列并驱动状态机。
- `TaskMeter`：中优先级，轮询 RS485 电表（DL/T645）。
- `TaskCloud`：中低优先级，MQTT/HTTP 上报，断网重连。
- `TaskOTA`：中低优先级，执行升级下载/校验/写入。

## 2) 事件总线（Queue）

标准写法建议：

- 业务事件统一进 `QueueHandle_t`。
- ISR 使用 `xQueueSendFromISR`。
- 任务使用 `xQueueSend` 与 `xQueueReceive`。

本工程实现位置：`app/core/event_bus.c`。

## 3) ISR 到任务通知（Task Notification）

推荐用于“轻量唤醒某个任务”而不是传复杂数据。

- ISR 里：`vTaskNotifyGiveFromISR(task, &hpw)`
- 任务里：`ulTaskNotifyTake(pdTRUE, timeout)`

本工程：CAN/保护任务发布事件后，会通知 FSM 任务尽快消费队列。

## 4) 标准创建方式（静态内存）

在量产桩控系统中建议优先静态分配，减少碎片：

- `xTaskCreateStatic`
- `xQueueCreateStatic`

本工程默认按静态方式创建任务与事件队列。

## 5) 充电状态机建议

状态：`IDLE -> GUN_DETECTED -> INSULATION_CHECK -> HANDSHAKE -> PARAM_CONFIG -> CHARGING -> FINISH`

异常事件（过压/过流/过温/急停）应具备最高抢占语义，直接转 `FAULT`。

## 6) 超时与可靠性

- CAN 心跳超时：推送 `EVT_CAN_TIMEOUT`
- 电表读失败：推送 `EVT_METER_TIMEOUT`
- 云链路断开：状态上报改为离线缓存
- 看门狗喂狗门控：仅关键任务心跳都正常才喂狗

## 7) OTA A/B 分区

建议流程：

1. 分片校验
2. 整包 CRC/Hash 校验
3. 写后读回校验
4. 写入 pending 分区标记并重启
5. 新镜像自检通过后写 boot_ok
6. 若失败，Bootloader 回滚到旧分区

对应实现参考：`app/ota/*` 与 `app/bootloader/*`。

## 8) 面试可直接说的“标准答案”

- “我们用 Queue 承载业务事件，用 Task Notification 做 ISR 到任务快速唤醒。”
- “关键任务（Safety/CANRx/FSM）高优先级，云和日志低优先级，避免阻塞控制链。”
- “OTA 采用 A/B + 回滚，保证升级失败不变砖。”
