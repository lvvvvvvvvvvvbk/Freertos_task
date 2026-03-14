#include "dlt645.h"

bool dlt645_verify_checksum(const uint8_t *frame, uint16_t len) {
    if (!frame || len < 2u) {
        return false;
    }

    uint8_t sum = 0u;
    for (uint16_t i = 0; i < len - 1u; ++i) {
        sum = (uint8_t)(sum + frame[i]);
    }

    return sum == frame[len - 1u];
}
