#ifndef GBT27930_H
#define GBT27930_H

#include <stdbool.h>
#include <stdint.h>

#include "event_bus.h"

typedef struct {
    uint32_t msg_id;
    uint32_t ts_ms;
    app_event_id_t mapped_event;
} gbt_frame_t;

bool gbt27930_poll(gbt_frame_t *out);

#endif
