#include QMK_KEYBOARD_H
#include "hardware_id.h"
#include <stdio.h>
#include <print.h>
#include "quantum.h"

enum custom_keycodes {
    DUMP_ID = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    LAYOUT_normal(
        DUMP_ID
    ),
};

#define right_pin GP12
#define left_pin GP13
#define down_pin GP10
#define up_pin GP11
char buffer[100];
uint8_t lastState;
uint16_t count = 0;
uint16_t currPinState;

void keyboard_pre_init_user(void) {
  // Set our LED pins as output
  setPinInput(up_pin);
  setPinInput(down_pin);
  setPinInput(left_pin);
  setPinInput(right_pin);
  lastState = readPin(up_pin);
}

int countChangeState(void){
    currPinState = readPin(up_pin);
    if (currPinState != lastState){
        count++;
        lastState = currPinState;
    }
    return count;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    countChangeState();
    uprintf("State Up pin: %d", count);
    return false;
};
