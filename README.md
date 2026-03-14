# STM32F407 + FreeRTOS 直流充电桩控制系统（标准 FreeRTOS 写法示例）

该仓库提供一个从 0 到 1 的工程化骨架，覆盖以下能力：

- FreeRTOS 任务划分、优先级与事件通信
- 充电流程状态机：插枪→绝缘→握手→参数配置→充电→结束
- 异常处理：过压/过流/过温/通信超时/急停
- CAN（GB/T 27930）报文收发与超时管理
- RS485（DL/T 645）电表轮询与校验
- SPI 射频 IC 卡支付流程接口
- 4G/WiFi 网络接入、MQTT/HTTP 上报任务
- OTA 双分区（A/B）升级流程、完整性校验与回滚
- Bootloader / App 职责边界示例

## 目录结构

```text
app/
  main.c
  config/
    app_config.h
  core/
    app_tasks.[ch]
    event_bus.[ch]
    charger_fsm.[ch]
    protection.[ch]
  drivers/
    bsp_time.[ch]
    can_if.[ch]
    rs485_if.[ch]
    spi_rfid.[ch]
    i2c_devices.[ch]
  protocol/
    gbt27930.[ch]
    dlt645.[ch]
  network/
    net_manager.[ch]
    telemetry.[ch]
  ota/
    ota_manager.[ch]
    partition_table.h
  bootloader/
    boot_decision.[ch]
docs/
  INTERVIEW_GUIDE.md
  FREERTOS_STANDARD_GUIDE.md
```

## 标准 FreeRTOS 写法（推荐）

本工程支持两种模式：

- `APP_USE_FREERTOS=1`：使用标准 FreeRTOS API（`xTaskCreateStatic`、`xQueueCreateStatic`、`xTaskNotifyGive`、`ulTaskNotifyTake`）。
- `APP_USE_FREERTOS=0`（默认）：主机环境回归测试模式，不依赖 FreeRTOS 源码。

你在 STM32F407 实机工程里应使用 `APP_USE_FREERTOS=1`。

## 推荐任务优先级（FreeRTOS：数值越大优先级越高）

- `TaskSafety`：6（保护逻辑、急停）
- `TaskCANRx`：5（高实时总线接收）
- `TaskFSM`：4（状态机消费事件）
- `TaskMeter`：3（RS485 电表）
- `TaskCloud`：2（MQTT/HTTP 上报）
- `TaskOTA`：2（升级）
- `TaskLog`：1（日志）

## 标准 API 映射

- ISR 到任务事件：`xTaskNotifyFromISR` / `xTaskNotifyGiveFromISR`
- 跨任务消息：`xQueueSend` / `xQueueReceive`
- 互斥资源：`xSemaphoreCreateMutex` + 优先级继承
- 周期调度：`vTaskDelay` 或 `vTaskDelayUntil`

## 编译（主机侧语法检查）

```bash
cmake -S . -B build
cmake --build build
./build/ev_charger
```

## STM32F407 实机接入建议

1. 用 CubeMX 建工程并启用 FreeRTOS（CMSIS_V2 或原生均可）。
2. 将 `APP_USE_FREERTOS` 设为 `1`。
3. 将 `drivers/*_if.c` 占位实现替换为 HAL 中断/DMA 版本。
4. 在 Bootloader 工程中完成 Flash 擦写、签名/CRC、升级标记持久化。
5. 按 docs 中用例做异常注入与回滚验证，确保掉电不变砖。
