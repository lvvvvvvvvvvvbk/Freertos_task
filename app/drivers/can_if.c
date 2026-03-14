#include "can_if.h"

bool can_if_receive(can_frame_t *frame) {
    (void)frame;
    return false;
}

bool can_if_send(const can_frame_t *frame) {
    (void)frame;
    return true;
}
