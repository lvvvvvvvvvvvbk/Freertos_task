#include "net_manager.h"

#include <stdio.h>

void net_manager_init(void) {
    printf("[NET] init 4G/WiFi + MQTT/HTTP\n");
}

bool net_manager_report(const telemetry_payload_t *payload) {
    if (!payload) {
        return false;
    }
    printf("[NET] report state=%u energy=%u\n", payload->state, payload->energy_wh);
    return true;
}
