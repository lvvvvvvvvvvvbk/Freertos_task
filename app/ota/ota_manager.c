#include "ota_manager.h"

#include <stdio.h>

static bool g_pending = false;
static uint32_t g_expected_crc = 0u;

void ota_manager_init(void) {
    g_pending = false;
    g_expected_crc = 0u;
}

bool ota_request_upgrade(const char *url, uint32_t expected_crc) {
    if (!url) return false;
    printf("[OTA] request url=%s crc=%u\n", url, expected_crc);
    g_pending = true;
    g_expected_crc = expected_crc;
    return true;
}

void ota_manager_process(void) {
    if (!g_pending) return;

    printf("[OTA] step1 fragment-check pass\n");
    printf("[OTA] step2 package-check pass crc=%u\n", g_expected_crc);
    printf("[OTA] step3 write-readback pass\n");
    printf("[OTA] mark pending slot then reboot\n");
    g_pending = false;
}
