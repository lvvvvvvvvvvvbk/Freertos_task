#include "event_bus.h"

#include "app_config.h"

#include <stddef.h>

#if APP_USE_FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

static StaticQueue_t g_evt_queue_ctrl;
static uint8_t g_evt_queue_storage[APP_EVENT_QUEUE_LEN * sizeof(app_event_t)];
static QueueHandle_t g_evt_queue = NULL;

bool event_bus_init(void) {
    g_evt_queue = xQueueCreateStatic(APP_EVENT_QUEUE_LEN, sizeof(app_event_t), g_evt_queue_storage, &g_evt_queue_ctrl);
    return g_evt_queue != NULL;
}

bool event_bus_publish_from_isr(app_event_t evt) {
    if (g_evt_queue == NULL) {
        return false;
    }
    BaseType_t hpw = pdFALSE;
    BaseType_t ok = xQueueSendFromISR(g_evt_queue, &evt, &hpw);
    portYIELD_FROM_ISR(hpw);
    return ok == pdPASS;
}

bool event_bus_publish(app_event_t evt) {
    if (g_evt_queue == NULL) {
        return false;
    }
    return xQueueSend(g_evt_queue, &evt, 0) == pdPASS;
}

bool event_bus_wait(app_event_t *evt, uint32_t timeout_ms) {
    if ((g_evt_queue == NULL) || (evt == NULL)) {
        return false;
    }
    TickType_t ticks = pdMS_TO_TICKS(timeout_ms);
    return xQueueReceive(g_evt_queue, evt, ticks) == pdPASS;
}

#else

#define EVENT_BUF_CAP 128u

static app_event_t g_ring[EVENT_BUF_CAP];
static uint32_t g_w = 0;
static uint32_t g_r = 0;

bool event_bus_init(void) {
    g_w = 0;
    g_r = 0;
    return true;
}

static bool push(app_event_t evt) {
    uint32_t next = (g_w + 1u) % EVENT_BUF_CAP;
    if (next == g_r) {
        return false;
    }
    g_ring[g_w] = evt;
    g_w = next;
    return true;
}

bool event_bus_publish_from_isr(app_event_t evt) {
    return push(evt);
}

bool event_bus_publish(app_event_t evt) {
    return push(evt);
}

bool event_bus_wait(app_event_t *evt, uint32_t timeout_ms) {
    (void)timeout_ms;
    if ((evt == NULL) || (g_r == g_w)) {
        return false;
    }
    *evt = g_ring[g_r];
    g_r = (g_r + 1u) % EVENT_BUF_CAP;
    return true;
}
#endif
