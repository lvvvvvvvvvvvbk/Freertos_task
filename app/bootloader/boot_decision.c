#include "boot_decision.h"

ota_partition_t boot_select_partition(const ota_boot_info_t *info) {
    if (!info) {
        return PART_A;
    }

    if ((info->pending != info->active) && (info->boot_ok_flag == 0u)) {
        return info->pending;
    }

    return info->active;
}
