#ifndef DLT645_H
#define DLT645_H

#include <stdbool.h>
#include <stdint.h>

bool dlt645_verify_checksum(const uint8_t *frame, uint16_t len);

#endif
