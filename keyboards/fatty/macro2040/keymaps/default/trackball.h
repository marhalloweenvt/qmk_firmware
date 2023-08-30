#pragma once

#include "pointing_device.h"
#include "quantum.h"
#include "report.h"
#include "print.h"
// #include <util/delay.h>

// Trackpad speed adjustments
#define POINTER_SPEED_MULTIPLIER 2
#define SCROLL_SPEED_DIVIDER 6

// Pins on corresponding ports
#define TP_RESET     1
#define TP_SHUTDOWN  0
#define TP_CS        0
#define LVL_SHIFT_EN 7

void debug_emblem_config_to_console(void);
