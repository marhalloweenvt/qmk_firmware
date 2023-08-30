// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-3.0-or-later
#include QMK_KEYBOARD_H
#include "analog.h"
#include "spi_master.h"

void keyboard_post_init_kb(void) {
    // Enable RGB current limiter and wait for a bit before allowing RGB to continue
    setPinOutput(RGB_ENABLE_PIN);
    writePinHigh(RGB_ENABLE_PIN);
    wait_ms(20);

    // Offload to the user func
    keyboard_post_init_user();
}

void matrix_init_custom(void) {
    spi_init();

    // Encoder pushbutton
    setPinInputLow(ENCODER_PUSHBUTTON_PIN1);
    setPinInputLow(ENCODER_PUSHBUTTON_PIN2);
}
