#include "rs485_if.h"

bool rs485_meter_read(meter_sample_t *sample) {
    if (!sample) return false;
    sample->energy_wh = 1234u;
    sample->voltage_mv = 380000u;
    sample->current_ma = 31500u;
    return true;
}
