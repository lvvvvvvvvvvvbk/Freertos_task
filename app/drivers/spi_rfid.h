#ifndef SPI_RFID_H
#define SPI_RFID_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t card_id;
    bool authorized;
} card_info_t;

bool rfid_poll_card(card_info_t *card);

#endif
