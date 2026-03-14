#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    EVT_NONE = 0,
    EVT_GUN_INSERTED,
    EVT_GUN_REMOVED,
    EVT_INSULATION_OK,
    EVT_HANDSHAKE_OK,
    EVT_PARAM_CONFIG_OK,
    EVT_START_CHARGE,
    EVT_STOP_CHARGE,
    EVT_CAN_TIMEOUT,
    EVT_METER_TIMEOUT,
    EVT_CARD_AUTH_OK,
    EVT_CARD_AUTH_FAIL,
    EVT_NET_CONNECTED,
    EVT_NET_DISCONNECTED,
    EVT_OTA_TRIGGER,
    EVT_OVERVOLTAGE,
    EVT_OVERCURRENT,
    EVT_OVERTEMP,
    EVT_EMERGENCY_STOP,
} app_event_id_t;

typedef struct {
    app_event_id_t id;
    uint32_t ts_ms;
    uint32_t data_u32;
} app_event_t;

void event_bus_init(void);
bool event_bus_publish_from_isr(app_event_t evt);
bool event_bus_publish(app_event_t evt);
bool event_bus_wait(app_event_t *evt, uint32_t timeout_ms);

#endif
