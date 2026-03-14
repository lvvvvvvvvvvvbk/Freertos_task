cmake_minimum_required(VERSION 3.16)
project(ev_charger_f407 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(ev_charger
    app/main.c
    app/core/app_tasks.c
    app/core/event_bus.c
    app/core/charger_fsm.c
    app/core/protection.c
    app/drivers/bsp_time.c
    app/drivers/can_if.c
    app/drivers/rs485_if.c
    app/drivers/spi_rfid.c
    app/drivers/i2c_devices.c
    app/protocol/gbt27930.c
    app/protocol/dlt645.c
    app/network/net_manager.c
    app/network/telemetry.c
    app/ota/ota_manager.c
    app/bootloader/boot_decision.c
)

target_include_directories(ev_charger PRIVATE
    app
    app/config
    app/core
    app/drivers
    app/protocol
    app/network
    app/ota
    app/bootloader
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ev_charger PRIVATE -Wall -Wextra -Wpedantic)
endif()
