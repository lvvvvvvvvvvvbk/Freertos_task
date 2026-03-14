#include "event_bus.h"

#include <stddef.h>

#define EVENT_BUF_CAP 128u

static app_event_t g_ring[EVENT_BUF_CAP];
static volatile uint32_t g_w = 0;
static volatile uint32_t g_r = 0;

void event_bus_init(void) {
    g_w = 0;
    g_r = 0;
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
