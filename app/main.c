#include <stdio.h>

#include "app_config.h"
#include "app_tasks.h"
#include "event_bus.h"

#if APP_USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

int main(void) {
    printf("EV Charger Controller Start\n");
    app_tasks_init();

    (void)event_bus_publish((app_event_t){.id = EVT_GUN_INSERTED, .ts_ms = 1u, .data_u32 = 0u});
    (void)event_bus_publish((app_event_t){.id = EVT_INSULATION_OK, .ts_ms = 2u, .data_u32 = 0u});
    (void)event_bus_publish((app_event_t){.id = EVT_HANDSHAKE_OK, .ts_ms = 3u, .data_u32 = 0u});
    (void)event_bus_publish((app_event_t){.id = EVT_PARAM_CONFIG_OK, .ts_ms = 4u, .data_u32 = 0u});
    (void)event_bus_publish((app_event_t){.id = EVT_START_CHARGE, .ts_ms = 5u, .data_u32 = 0u});

#if APP_USE_FREERTOS
    vTaskStartScheduler();
    for (;;) {
    }
#else
    for (int i = 0; i < 3; ++i) {
        app_tasks_run_once();
    }

    (void)event_bus_publish((app_event_t){.id = EVT_STOP_CHARGE, .ts_ms = 6u, .data_u32 = 0u});
    app_tasks_run_once();
#endif

    return 0;
}
