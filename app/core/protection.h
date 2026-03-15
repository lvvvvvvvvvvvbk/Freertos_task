#ifndef PROTECTION_H
#define PROTECTION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t voltage_mv;
    uint32_t current_ma;
    int32_t temp_mc;
    bool emergency_stop;
} protection_sample_t;

bool protection_check(const protection_sample_t *sample);

#endif
