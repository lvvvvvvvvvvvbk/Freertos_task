#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <stdbool.h>

#include "telemetry.h"

void net_manager_init(void);
bool net_manager_report(const telemetry_payload_t *payload);

#endif
