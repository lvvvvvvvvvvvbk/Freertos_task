#ifndef RS485_IF_H
#define RS485_IF_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t energy_wh;
    uint32_t voltage_mv;
    uint32_t current_ma;
} meter_sample_t;

bool rs485_meter_read(meter_sample_t *sample);

#endif
