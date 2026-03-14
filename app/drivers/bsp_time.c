#include "bsp_time.h"

uint32_t bsp_millis(void) {
    static uint32_t mock = 0;
    return ++mock;
}
