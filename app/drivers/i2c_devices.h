#ifndef I2C_DEVICES_H
#define I2C_DEVICES_H

#include <stdbool.h>
#include <stdint.h>

bool i2c_read_temp_sensor(int32_t *temp_mc);

#endif
