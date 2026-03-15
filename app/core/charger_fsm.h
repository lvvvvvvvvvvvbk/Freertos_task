#ifndef CHARGER_FSM_H
#define CHARGER_FSM_H

#include <stdbool.h>

#include "event_bus.h"

typedef enum {
    ST_IDLE = 0,
    ST_GUN_DETECTED,
    ST_INSULATION_CHECK,
    ST_HANDSHAKE,
    ST_PARAM_CONFIG,
    ST_CHARGING,
    ST_FINISH,
    ST_FAULT,
} charger_state_t;

void charger_fsm_init(void);
void charger_fsm_dispatch(const app_event_t *evt);
charger_state_t charger_fsm_get_state(void);
const char *charger_state_name(charger_state_t st);

#endif
