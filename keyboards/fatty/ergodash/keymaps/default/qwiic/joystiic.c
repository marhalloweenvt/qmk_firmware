/* Copyright 2018 Jack Humbert <jack.humb@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include "joystiic.h"
#include "print.h"
#include "action.h"
#include "i2c_master.h"

// Trackpad speed adjustments
#define POINTER_SPEED_MULTIPLIER 2
#define SCROLL_SPEED_DIVIDER 6

typedef struct {
    // uint8_t id;
    // uint8_t fwMajor;
    // uint8_t fwMinor;
    uint8_t X_MSB;
    uint8_t X_LSB;
    uint8_t Y_MSB;
    uint8_t Y_LSB;
    uint8_t button_state;
    uint8_t button_status;
    // uint8_t i2cLock;
    // uint8_t i2cAddress;
} joystiic_data_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  button;
} report_joystiic_t;

#define JOYSTIIC_DEFAULT_ADDR 0x22
#define JOYSTIIC1_DEFAULT_ADDR 0x22
#define JOYSTIIC2_DEFAULT_ADDR 0x20
#define JOYSTIIC_ID 0x00
#define JOYSTIIC_VERSION1 0x01
#define JOYSTIIC_VERSION2 0x02
#define JOYSTIIC_COMMAND_MSB_HORIZONTAL 0x03
#define JOYSTIIC_COMMAND_LSB_HORIZONTAL 0x04
#define JOYSTIIC_COMMAND_MSB_VERTICAL 0x05
#define JOYSTIIC_COMMAND_LSB_VERTICAL 0x06
#define JOYSTIIC_COMMAND_BUTTON 0x07
#define JOYSTIIC_COMMAND_BUTTON_STATE 0x08
#define JOYSTIIC_CENTER 512
#define JOYSTIIC_DEADZONE 50
#define JOYSTIIC_DEBUG_INTERVAL 250
#define JOYSTIIC_ERROR_COUNT 10

#ifndef ANALOG_JOYSTICK_AXIS_MIN
#    define ANALOG_JOYSTICK_AXIS_MIN 0
#endif
#ifndef ANALOG_JOYSTICK_AXIS_MAX
#    define ANALOG_JOYSTICK_AXIS_MAX 1023
#endif
#ifndef ANALOG_JOYSTICK_SPEED_REGULATOR
#    define ANALOG_JOYSTICK_SPEED_REGULATOR 20
#endif
#ifndef ANALOG_JOYSTICK_READ_INTERVAL
#    define ANALOG_JOYSTICK_READ_INTERVAL 5
#endif
#ifndef ANALOG_JOYSTICK_SPEED_MAX
#    define ANALOG_JOYSTICK_SPEED_MAX 2
#endif

#define I2C_ADDR_WRITE ((JOYSTIIC_DEFAULT_ADDR << 1) | I2C_WRITE)
#define I2C_ADDR_READ ((JOYSTIIC_DEFAULT_ADDR << 1) | I2C_READ)
#define I2C_READ_TIMEOUT 10

uint16_t joystiic_horizontal;
uint16_t joystiic_vertical;
bool     joystiic_button;

uint8_t joystiic_tx[1];
uint8_t joystiic_rx_horizontal[2];
uint8_t joystiic_rx_vertical[2];
uint8_t joystiic_rx_button[1];

// Returns the 10-bit ADC value of the joystick horizontal position
i2c_status_t JOYSTIIC_getHorizontal(void) {
    uint8_t  data_MSB[1] = {};
    uint8_t  data_LSB[1] = {};
    uint16_t X_MSB       = i2c_readReg(JOYSTIIC_DEFAULT_ADDR, JOYSTIIC_COMMAND_MSB_HORIZONTAL, data_MSB, sizeof(data_MSB), I2C_READ_TIMEOUT);
    uint16_t X_LSB       = i2c_readReg(JOYSTIIC_DEFAULT_ADDR, JOYSTIIC_COMMAND_LSB_HORIZONTAL, data_LSB, sizeof(data_LSB), I2C_READ_TIMEOUT);
    i2c_stop();
    return ((X_MSB << 8) | X_LSB) >> 6;
}

// Returns the 10-bit ADC value of the joystick vertical position
i2c_status_t JOYSTIIC_getVertical(void) {
    uint8_t  data_MSB[1] = {};
    uint8_t  data_LSB[1] = {};
    uint16_t Y_MSB       = i2c_readReg(JOYSTIIC_DEFAULT_ADDR, JOYSTIIC_COMMAND_MSB_VERTICAL, data_MSB, sizeof(data_MSB), I2C_READ_TIMEOUT);
    uint16_t Y_LSB       = i2c_readReg(JOYSTIIC_DEFAULT_ADDR, JOYSTIIC_COMMAND_LSB_VERTICAL, data_LSB, sizeof(data_LSB), I2C_READ_TIMEOUT);
    i2c_stop();
    return ((Y_MSB << 8) | Y_LSB) >> 6;
}

bool JOYSTIIC_checkButton(void) {
    bool status = i2c_readReg(JOYSTIIC_DEFAULT_ADDR, JOYSTIIC_COMMAND_BUTTON, joystiic_rx_button, 1, I2C_TIMEOUT);
    return (status);
}

bool joystiic_triggered[5] = {0};

void joystiic_init(void) {
    i2c_init();
    i2c_start(JOYSTIIC_DEFAULT_ADDR);
}

void joystiic_update(uint16_t horizontal, uint16_t vertical, bool button) {
    joystiic_update_kb(horizontal, vertical, button);
    joystiic_update_user(horizontal, vertical, button);
}

__attribute__((weak)) void joystiic_update_kb(uint16_t horizontal, uint16_t vertical, bool button) {}

__attribute__((weak)) void joystiic_update_user(uint16_t horizontal, uint16_t vertical, bool button) {}

void joystiic_trigger(uint8_t trigger, bool active) {
    joystiic_trigger_kb(trigger, active);
    joystiic_trigger_user(trigger, active);
}

__attribute__((weak)) void joystiic_trigger_kb(uint8_t trigger, bool active) {}

__attribute__((weak)) void joystiic_trigger_user(uint8_t trigger, bool active) {}

void joystiic_trigger_if_not(uint8_t trigger, bool active) {
    if (joystiic_triggered[trigger] != active) {
        joystiic_triggered[trigger] = active;
        joystiic_trigger(trigger, active);
    }
}

void joystiic_task(void) {
    joystiic_horizontal = JOYSTIIC_getHorizontal();

    joystiic_trigger_if_not(JOYSTIIC_LEFT, joystiic_horizontal > (JOYSTIIC_CENTER + JOYSTIIC_DEADZONE));
    joystiic_trigger_if_not(JOYSTIIC_RIGHT, joystiic_horizontal < (JOYSTIIC_CENTER - JOYSTIIC_DEADZONE));

    joystiic_vertical = JOYSTIIC_getVertical();

    joystiic_trigger_if_not(JOYSTIIC_UP, joystiic_vertical > (JOYSTIIC_CENTER + JOYSTIIC_DEADZONE));
    joystiic_trigger_if_not(JOYSTIIC_DOWN, joystiic_vertical < (JOYSTIIC_CENTER - JOYSTIIC_DEADZONE));

    joystiic_button = JOYSTIIC_checkButton();

    joystiic_trigger_if_not(JOYSTIIC_PRESS, joystiic_button);

    joystiic_update(joystiic_horizontal, joystiic_vertical, joystiic_button);

    // printf("%d\n", joystiic[0]);

    // SEND_STRING("H: ");
    // send_word(joystiic_rx_horizontal[0]);
    // tap_code(KC_SPACE);
    // send_word(joystiic_rx_horizontal[1]);
    // tap_code(KC_SPACE);
}
