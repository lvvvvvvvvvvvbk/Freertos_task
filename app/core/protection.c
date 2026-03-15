#include "protection.h"

#include "app_config.h"

bool protection_check(const protection_sample_t *sample) {
    if (sample == 0) return false;
    if (sample->emergency_stop) return false;
    if (sample->voltage_mv > CHARGING_MAX_VOLTAGE_MV) return false;
    if (sample->current_ma > CHARGING_MAX_CURRENT_MA) return false;
    if (sample->temp_mc > CHARGING_MAX_TEMP_MC) return false;
    return true;
}
