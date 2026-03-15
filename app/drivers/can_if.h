#ifndef CAN_IF_H
#define CAN_IF_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
} can_frame_t;

bool can_if_receive(can_frame_t *frame);
bool can_if_send(const can_frame_t *frame);

#endif
