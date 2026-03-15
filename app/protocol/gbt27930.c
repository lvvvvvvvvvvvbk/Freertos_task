#include "gbt27930.h"

#include "can_if.h"

bool gbt27930_poll(gbt_frame_t *out) {
    can_frame_t raw;
    if (!out || !can_if_receive(&raw)) {
        return false;
    }

    out->msg_id = raw.id;
    out->ts_ms = 0u;
    out->mapped_event = EVT_NONE;

    switch (raw.id) {
        case 0x1801F456: out->mapped_event = EVT_HANDSHAKE_OK; break;
        case 0x1802F456: out->mapped_event = EVT_PARAM_CONFIG_OK; break;
        case 0x1807F456: out->mapped_event = EVT_STOP_CHARGE; break;
        default: break;
    }

    return true;
}
