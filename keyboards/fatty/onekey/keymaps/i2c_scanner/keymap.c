#include QMK_KEYBOARD_H

#include "i2c_master.h"
#include "debug.h"
#include "print.h"
#include "trackpad.h"
extern int16_t xOrigin;
extern int16_t yOrigin;
extern bool    isScrollMode;
enum custom_keycodes { QWERTY = SAFE_RANGE, LOWER, RAISE, RGBRST, MBTN1, SCRL };
#define KC_MBTN1 MBTN1
#define KC_SCRL SCRL

// #define JOYSTIIC_DEFAULT_ADDR (0x20 << 1)
// // #define JOYSTIIC_COMMAND_HORIZONTAL 0x00
// // #define JOYSTIIC_COMMAND_VERTICAL   0x02
// // #define JOYSTIIC_COMMAND_BUTTON     0x04
// #define JOYSTIIC_VERSION1 0x01
// #define JOYSTIIC_VERSION2 0x02
// #define JOYSTIIC_COMMAND_MSB_HORIZONTAL 0x03
// #define JOYSTIIC_COMMAND_LSB_HORIZONTAL 0x04
// #define JOYSTIIC_COMMAND_MSB_VERTICAL 0x05
// #define JOYSTIIC_COMMAND_LSB_VERTICAL 0x06
// #define JOYSTIIC_COMMAND_BUTTON 0x07

// #define JOYSTIIC_CENTER 512
// #define JOYSTIIC_DEADZONE 200

// #define TIMEOUT 50
// i2c_status_t joystick_status;

// TODO: remove patch
// #ifdef PROTOCOL_CHIBIOS
// #    pragma message("ChibiOS is currently 'best effort' and might not report accurate results")

// i2c_status_t i2c_start_bodge(uint8_t address, uint16_t timeout) {
//     i2c_start(address);

//     // except on ChibiOS where the only way is do do "something"
//     uint8_t data = 0;
//     return i2c_readReg(address, 0, &data, sizeof(data), TIMEOUT);
// }

// #    define i2c_start i2c_start_bodge
// #endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {LAYOUT_ortho_1x1(MBTN1)};

// void do_scan(void) {
//     uint8_t nDevices = 0;

//     dprintf("Scanning...\n");

//     for (uint8_t address = 1; address < 127; address++) {
//         // The i2c_scanner uses the return value of
//         // i2c_start to see if a device did acknowledge to the address.
//         i2c_status_t error = i2c_start(address << 1, TIMEOUT);
//         if (error == I2C_STATUS_SUCCESS) {
//             i2c_stop();
//             dprintf("  I2C device found at address 0x%02X\n", address);
//             nDevices++;
//         } else {
//             // dprintf("  Unknown error (%u) at address 0x%02X\n", error, address);
//         }
//     }

//     if (nDevices == 0)
//         dprintf("No I2C devices found\n");
//     else
//         dprintf("done\n");
// }

uint16_t scan_timer = 0;

void matrix_scan_user(void) {
    if (timer_elapsed(scan_timer) > 2000) {
        // uint8_t stick_id            = 0;
        // uint8_t stick_fwMajor       = 0;
        // uint8_t stick_fwMinor       = 0;
        // uint8_t stick_X_MSB         = 0;
        // uint8_t stick_X_LSB         = 0;
        // uint8_t stick_Y_MSB         = 0;
        // uint8_t stick_Y_LSB         = 0;
        // uint8_t stick_button_state  = 0;
        // uint8_t stick_button_status = 0;
        // uint8_t stick_i2cLock       = 0;
        // uint8_t stick_i2cAddress    = 0;
        // uint8_t data[11]            = {0};

        // i2c_status_t joystick_status = i2c_receive(JOYSTIIC_DEFAULT_ADDR << 1, data, sizeof(data), I2C_TIMEOUT);
        // dprintf("Status: %d\n", joystick_status);
        // if (joystick_status == I2C_STATUS_SUCCESS) {
        //     stick_id            = data[0];
        //     stick_fwMajor       = data[1];
        //     stick_fwMinor       = data[2];
        //     stick_X_MSB         = data[3];
        //     stick_X_LSB         = data[4];
        //     stick_Y_MSB         = data[5];
        //     stick_Y_LSB         = data[6];
        //     stick_button_state  = data[7];
        //     stick_button_status = data[8];
        //     stick_i2cLock       = data[9];
        //     stick_i2cAddress    = data[10];
        //     dprintf("stick_id: %d, stick_fwMajor: %d, stick_fwMinor: %d, stick_X_MSB: %d, stick_X_LSB: %d, stick_Y_MSB: %d, stick_Y_LSB: %d, stick_button_state: %d, stick_button_status: %d, stick_i2cLock: %d, stick_i2cAddress: %d\n", stick_id, stick_fwMajor, stick_fwMinor, stick_X_MSB, stick_X_LSB, stick_Y_MSB, stick_Y_LSB, stick_button_state, stick_button_status, stick_i2cLock, stick_i2cAddress);
        // } else {
        //     return;
        // }
        // xOrigin = (data[3] << 8 | data[4]) >> 6;
        // yOrigin = (data[5] << 8 | data[6]) >> 6;
        // dprintf("xOrigin: %d, yOrigin: %d\n", xOrigin, yOrigin);

        //     i2c_stop();
        scan_timer = timer_read();
    }
}

void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = true;

    // joystiic_device_init();
    scan_timer = timer_read();
}

void housekeeping_task_user(void) {
    // if (timer_elapsed(scan_timer) > 5000) {
    //     uint8_t      X_MSB   = 0;
    //     uint8_t      X_LSB   = 0;
    //     uint8_t      Y_MSB   = 0;
    //     uint8_t      Y_LSB   = 0;
    //     uint8_t      data[5] = {0};
    //     i2c_status_t error   = i2c_readReg(JOYSTIIC_DEFAULT_ADDR, JOYSTIIC_COMMAND_MSB_HORIZONTAL, data, 5, TIMEOUT);
    //     if (error == I2C_STATUS_SUCCESS) {
    //         i2c_stop();
    //         X_MSB = data[0];
    //         X_LSB = data[1];
    //         Y_MSB = data[2];
    //         Y_LSB = data[3];
    //         uprintf("X_MSB: %d, X_LSB: %d, Y_MSB: %d, Y_LSB: %d\n", X_MSB, X_LSB, Y_MSB, Y_LSB);
    //     } else {
    //         // dprintf("  Unknown error (%u) at address 0x%02X\n", error, address);
    //     }
    //     scan_timer = timer_read();
    // }
}

// bool process_record_user(uint16_t keycode, keyrecord_t *record) {
//     switch (keycode) {
//         case SCRL:
//             if (record->event.pressed) {
//                 isScrollMode = true;
//             } else {
//                 isScrollMode = false;
//             }
//             return false;
//     }
//     return true;
// }
