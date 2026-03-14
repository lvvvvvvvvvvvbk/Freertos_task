#include "i2c_devices.h"

bool i2c_read_temp_sensor(int32_t *temp_mc) {
    if (!temp_mc) return false;
    *temp_mc = 42000;
    return true;
}
