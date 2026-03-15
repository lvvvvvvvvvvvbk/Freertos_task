#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

void ota_manager_init(void);
void ota_manager_process(void);
bool ota_request_upgrade(const char *url, uint32_t expected_crc);

#endif
