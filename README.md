# STM32F407 + FreeRTOS 直流充电桩控制系统（项目骨架）

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

> 说明：该工程以“可面试讲解 + 可二次开发”为目标，接口与模块完整，硬件相关细节使用 HAL/板级适配层占位实现。

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
```

## 推荐任务优先级（数值越大优先级越高）

- `TaskSafety`：高优先级，保护逻辑与急停
- `TaskCANRx`：高优先级，CAN 接收和协议分发
- `TaskFSM`：中高优先级，状态机驱动
- `TaskMeter`：中优先级，RS485 电表读取
- `TaskCloud`：中优先级，MQTT/HTTP 上报
- `TaskOTA`：中低优先级，升级下载与写入
- `TaskLog`：低优先级，日志落盘/串口输出

## 编译（主机侧语法检查）

```bash
cmake -S . -B build
cmake --build build
```

## 后续接入建议

1. 使用 CubeMX 生成 `STM32F407` 工程后，将 `Core/Src` 与本工程模块合并。
2. 将 `drivers/*_if.c` 中占位实现替换为 `HAL + DMA + 中断` 实现。
3. 在 Bootloader 工程中实现 Flash 擦写、签名校验、镜像标记持久化。
4. 基于真实 BMS / 电表 / 读卡器链路做联调与异常注入测试。
