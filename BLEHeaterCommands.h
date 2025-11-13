#pragma once
#include <stdint.h>

struct Command {
    uint8_t cmd;
    uint8_t value;
};

// Example commands
const Command CMD_STATUS     = {0x01, 0x00};
const Command CMD_POWER_ON   = {0x03, 0x01};
const Command CMD_POWER_OFF  = {0x03, 0x00};