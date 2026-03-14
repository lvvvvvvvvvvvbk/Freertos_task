#ifndef BOOT_DECISION_H
#define BOOT_DECISION_H

#include "partition_table.h"

ota_partition_t boot_select_partition(const ota_boot_info_t *info);

#endif
