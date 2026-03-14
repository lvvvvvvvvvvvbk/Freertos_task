#include "app_tasks.h"

#include <stdio.h>

#include "charger_fsm.h"
#include "event_bus.h"
#include "gbt27930.h"
#include "net_manager.h"
#include "ota_manager.h"
#include "protection.h"
#include "rs485_if.h"
#include "spi_rfid.h"

void app_tasks_init(void) {
    event_bus_init();
    charger_fsm_init();
    net_manager_init();
    ota_manager_init();
}

static void task_safety_tick(void) {
    protection_sample_t s = {.voltage_mv = 400000u, .current_ma = 32000u, .temp_mc = 45000, .emergency_stop = false};
    if (!protection_check(&s)) {
        (void)event_bus_publish((app_event_t){.id = EVT_EMERGENCY_STOP, .ts_ms = 0u, .data_u32 = 0u});
    }
}

static void task_can_rx_tick(void) {
    gbt_frame_t f;
    if (gbt27930_poll(&f)) {
        (void)event_bus_publish((app_event_t){.id = f.mapped_event, .ts_ms = f.ts_ms, .data_u32 = f.msg_id});
    }
}

static void task_meter_tick(void) {
    meter_sample_t sample;
    if (!rs485_meter_read(&sample)) {
        (void)event_bus_publish((app_event_t){.id = EVT_METER_TIMEOUT, .ts_ms = 0u, .data_u32 = 0u});
    }
}

static void task_cloud_tick(void) {
    telemetry_payload_t payload;
    telemetry_build(&payload, charger_fsm_get_state());
    net_manager_report(&payload);
}

static void task_card_tick(void) {
    card_info_t card;
    if (rfid_poll_card(&card)) {
        app_event_id_t id = card.authorized ? EVT_CARD_AUTH_OK : EVT_CARD_AUTH_FAIL;
        (void)event_bus_publish((app_event_t){.id = id, .ts_ms = 0u, .data_u32 = card.card_id});
    }
}

static void task_fsm_tick(void) {
    app_event_t evt;
    while (event_bus_wait(&evt, 0u)) {
        charger_fsm_dispatch(&evt);
    }
}

void app_tasks_run_once(void) {
    task_safety_tick();
    task_can_rx_tick();
    task_meter_tick();
    task_card_tick();
    task_cloud_tick();
    task_fsm_tick();
    ota_manager_process();
    printf("[APP] state=%s\n", charger_state_name(charger_fsm_get_state()));
}
