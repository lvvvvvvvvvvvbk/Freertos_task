#include "telemetry.h"

#include "bsp_time.h"

void telemetry_build(telemetry_payload_t *payload, charger_state_t st) {
    if (!payload) {
        return;
    }
    payload->ts_ms = bsp_millis();
    payload->state = (uint32_t)st;
    payload->energy_wh = 1234u;
    payload->fault_bits = (st == ST_FAULT) ? 1u : 0u;
}
