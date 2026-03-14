#ifndef PARTITION_TABLE_H
#define PARTITION_TABLE_H

#include <stdint.h>

typedef enum {
    PART_A = 0,
    PART_B = 1,
} ota_partition_t;

typedef struct {
    ota_partition_t active;
    ota_partition_t pending;
    uint32_t image_crc;
    uint32_t image_size;
    uint32_t boot_ok_flag;
} ota_boot_info_t;

#endif
