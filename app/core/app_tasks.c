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

#if APP_USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t g_task_fsm = NULL;

static void task_safety(void *arg) {
    (void)arg;
    for (;;) {
        protection_sample_t s = {.voltage_mv = 400000u, .current_ma = 32000u, .temp_mc = 45000, .emergency_stop = false};
        if (!protection_check(&s)) {
            (void)event_bus_publish((app_event_t){.id = EVT_EMERGENCY_STOP, .ts_ms = 0u, .data_u32 = 0u});
            if (g_task_fsm != NULL) {
                (void)xTaskNotifyGive(g_task_fsm);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void task_can_rx(void *arg) {
    (void)arg;
    for (;;) {
        gbt_frame_t f;
        if (gbt27930_poll(&f)) {
            (void)event_bus_publish((app_event_t){.id = f.mapped_event, .ts_ms = f.ts_ms, .data_u32 = f.msg_id});
            if (g_task_fsm != NULL) {
                (void)xTaskNotifyGive(g_task_fsm);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void task_meter(void *arg) {
    (void)arg;
    for (;;) {
        meter_sample_t sample;
        if (!rs485_meter_read(&sample)) {
            (void)event_bus_publish((app_event_t){.id = EVT_METER_TIMEOUT, .ts_ms = 0u, .data_u32 = 0u});
            if (g_task_fsm != NULL) {
                (void)xTaskNotifyGive(g_task_fsm);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(METER_POLL_PERIOD_MS));
    }
}

static void task_card(void *arg) {
    (void)arg;
    for (;;) {
        card_info_t card;
        if (rfid_poll_card(&card)) {
            app_event_id_t id = card.authorized ? EVT_CARD_AUTH_OK : EVT_CARD_AUTH_FAIL;
            (void)event_bus_publish((app_event_t){.id = id, .ts_ms = 0u, .data_u32 = card.card_id});
            if (g_task_fsm != NULL) {
                (void)xTaskNotifyGive(g_task_fsm);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void task_cloud(void *arg) {
    (void)arg;
    for (;;) {
        telemetry_payload_t payload;
        telemetry_build(&payload, charger_fsm_get_state());
        (void)net_manager_report(&payload);
        vTaskDelay(pdMS_TO_TICKS(CLOUD_REPORT_MS));
    }
}

static void task_ota(void *arg) {
    (void)arg;
    for (;;) {
        ota_manager_process();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void task_fsm(void *arg) {
    (void)arg;
    for (;;) {
        (void)ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));
        app_event_t evt;
        while (event_bus_wait(&evt, 0u)) {
            charger_fsm_dispatch(&evt);
        }
        printf("[APP] state=%s\n", charger_state_name(charger_fsm_get_state()));
    }
}

void app_tasks_init(void) {
    (void)event_bus_init();
    charger_fsm_init();
    net_manager_init();
    ota_manager_init();

    static StackType_t st_safety[STACK_WORDS_SMALL], st_can[STACK_WORDS_SMALL], st_fsm[STACK_WORDS_MEDIUM];
    static StackType_t st_meter[STACK_WORDS_SMALL], st_card[STACK_WORDS_SMALL], st_cloud[STACK_WORDS_MEDIUM], st_ota[STACK_WORDS_SMALL];
    static StaticTask_t tcb_safety, tcb_can, tcb_fsm, tcb_meter, tcb_card, tcb_cloud, tcb_ota;

    (void)xTaskCreateStatic(task_safety, "TaskSafety", STACK_WORDS_SMALL, NULL, PRIO_TASK_SAFETY, st_safety, &tcb_safety);
    (void)xTaskCreateStatic(task_can_rx, "TaskCANRx", STACK_WORDS_SMALL, NULL, PRIO_TASK_CAN_RX, st_can, &tcb_can);
    g_task_fsm = xTaskCreateStatic(task_fsm, "TaskFSM", STACK_WORDS_MEDIUM, NULL, PRIO_TASK_FSM, st_fsm, &tcb_fsm);
    (void)xTaskCreateStatic(task_meter, "TaskMeter", STACK_WORDS_SMALL, NULL, PRIO_TASK_METER, st_meter, &tcb_meter);
    (void)xTaskCreateStatic(task_card, "TaskCard", STACK_WORDS_SMALL, NULL, PRIO_TASK_METER, st_card, &tcb_card);
    (void)xTaskCreateStatic(task_cloud, "TaskCloud", STACK_WORDS_MEDIUM, NULL, PRIO_TASK_CLOUD, st_cloud, &tcb_cloud);
    (void)xTaskCreateStatic(task_ota, "TaskOTA", STACK_WORDS_SMALL, NULL, PRIO_TASK_OTA, st_ota, &tcb_ota);
}

#else

void app_tasks_init(void) {
    (void)event_bus_init();
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
    (void)net_manager_report(&payload);
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
#endif
