#include "charger_fsm.h"

#include <stdio.h>

static charger_state_t g_state = ST_IDLE;

void charger_fsm_init(void) {
    g_state = ST_IDLE;
}

charger_state_t charger_fsm_get_state(void) {
    return g_state;
}

const char *charger_state_name(charger_state_t st) {
    switch (st) {
        case ST_IDLE: return "IDLE";
        case ST_GUN_DETECTED: return "GUN_DETECTED";
        case ST_INSULATION_CHECK: return "INSULATION_CHECK";
        case ST_HANDSHAKE: return "HANDSHAKE";
        case ST_PARAM_CONFIG: return "PARAM_CONFIG";
        case ST_CHARGING: return "CHARGING";
        case ST_FINISH: return "FINISH";
        case ST_FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

static void enter_state(charger_state_t next, const app_event_t *evt) {
    int event_id = evt ? (int)evt->id : -1;
    printf("[FSM] %s --(%d)--> %s\n", charger_state_name(g_state), event_id, charger_state_name(next));
    g_state = next;
}

void charger_fsm_dispatch(const app_event_t *evt) {
    if (evt == NULL) {
        return;
    }

    if ((evt->id == EVT_OVERVOLTAGE) || (evt->id == EVT_OVERCURRENT) ||
        (evt->id == EVT_OVERTEMP) || (evt->id == EVT_EMERGENCY_STOP)) {
        enter_state(ST_FAULT, evt);
        return;
    }

    switch (g_state) {
        case ST_IDLE:
            if (evt->id == EVT_GUN_INSERTED) enter_state(ST_GUN_DETECTED, evt);
            break;
        case ST_GUN_DETECTED:
            if (evt->id == EVT_INSULATION_OK) enter_state(ST_INSULATION_CHECK, evt);
            if (evt->id == EVT_GUN_REMOVED) enter_state(ST_IDLE, evt);
            break;
        case ST_INSULATION_CHECK:
            if (evt->id == EVT_HANDSHAKE_OK) enter_state(ST_HANDSHAKE, evt);
            break;
        case ST_HANDSHAKE:
            if (evt->id == EVT_PARAM_CONFIG_OK) enter_state(ST_PARAM_CONFIG, evt);
            break;
        case ST_PARAM_CONFIG:
            if ((evt->id == EVT_CARD_AUTH_OK) || (evt->id == EVT_START_CHARGE)) {
                enter_state(ST_CHARGING, evt);
            }
            break;
        case ST_CHARGING:
            if ((evt->id == EVT_STOP_CHARGE) || (evt->id == EVT_GUN_REMOVED)) {
                enter_state(ST_FINISH, evt);
            }
            break;
        case ST_FINISH:
            if (evt->id == EVT_GUN_REMOVED) enter_state(ST_IDLE, evt);
            break;
        case ST_FAULT:
            if (evt->id == EVT_STOP_CHARGE) enter_state(ST_FINISH, evt);
            break;
        default:
            break;
    }
}
