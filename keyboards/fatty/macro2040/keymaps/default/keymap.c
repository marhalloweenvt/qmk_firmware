// Copyright 2022 Jimmy Lye (@satorusaka)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "trackball.h"
#include "print.h"
// extern void ui_init(void);
// extern void ui_task(void);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_default(
        // KC_MS_UP, KC_MS_DOWN, KC_MS_LEFT, KC_MS_RIGHT, KC_MS_BTN1, KC_1, KC_2
        KC_1, KC_2
    ),
    [1] = LAYOUT_default(
        // KC_MS_UP, KC_MS_DOWN, KC_MS_LEFT, KC_MS_RIGHT, KC_MS_BTN1, KC_1, KC_2
        KC_1, KC_2
    )
};

const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [0] = {ENCODER_CCW_CW(RGB_HUI, RGB_HUD)},
    [1]  = {ENCODER_CCW_CW(RGB_HUI, RGB_HUD)},
};

void keyboard_post_init_user(void) {
    // Init the display
    // ui_init();
    debug_enable=true;
    debug_matrix=true;
}

void housekeeping_task_user(void) {
    // Draw the display
    // ui_task();
    debug_emblem_config_to_console();
    dprintf("keymap");
}
