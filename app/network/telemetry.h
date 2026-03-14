#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

#include "charger_fsm.h"

typedef struct {
    uint32_t ts_ms;
    uint32_t state;
    uint32_t energy_wh;
    uint32_t fault_bits;
} telemetry_payload_t;

void telemetry_build(telemetry_payload_t *payload, charger_state_t st);

#endif
