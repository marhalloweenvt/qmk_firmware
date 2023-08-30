#pragma once

#include "pointing_device.h"
#include "quantum.h"
#include "report.h"
#include "i2c_master.h"

// Trackpad speed adjustments
#define POINTER_SPEED_MULTIPLIER 2
#define SCROLL_SPEED_DIVIDER 6

typedef struct {
    uint8_t id;
    uint8_t fwMajor;
    uint8_t fwMinor;
    uint8_t X_MSB;
    uint8_t X_LSB;
    uint8_t Y_MSB;
    uint8_t Y_LSB;
    uint8_t button_state;
    uint8_t button_status;
    uint8_t i2cLock;
    uint8_t i2cAddress;
} joystiic_data_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t  button;
} report_joystiic_t;
#define JOYSTIIC_DEFAULT_ADDR 0x22
// #define JOYSTIIC_COMMAND_HORIZONTAL 0x00
// #define JOYSTIIC_COMMAND_VERTICAL   0x02
// #define JOYSTIIC_COMMAND_BUTTON     0x04
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
#define JOYSTIIC_DEADZONE 100
#define JOYSTIIC_DEBUG_INTERVAL 100

#ifndef ANALOG_JOYSTICK_AXIS_MIN
#    define ANALOG_JOYSTICK_AXIS_MIN 0
#endif
#ifndef ANALOG_JOYSTICK_AXIS_MAX
#    define ANALOG_JOYSTICK_AXIS_MAX 1023
#endif
#ifndef ANALOG_JOYSTICK_SPEED_REGULATOR
#    define ANALOG_JOYSTICK_SPEED_REGULATOR 25
#endif
#ifndef ANALOG_JOYSTICK_READ_INTERVAL
#    define ANALOG_JOYSTICK_READ_INTERVAL 5
#endif
#ifndef ANALOG_JOYSTICK_SPEED_MAX
#    define ANALOG_JOYSTICK_SPEED_MAX 2
#endif

#define I2C_READ 1
#define I2C_WRITE 0
#define I2C_ADDR_WRITE ((JOYSTIIC_DEFAULT_ADDR << 1) | I2C_WRITE)
#define I2C_ADDR_READ ((JOYSTIIC_DEFAULT_ADDR << 1) | I2C_READ)
#define I2C_READ_TIMEOUT 10
i2c_status_t      joystiic_status;
void              joystiic_device_init(void);
report_joystiic_t i2c_joystick_read(void);
// report_mouse_t    i2c_joystick_get_report(report_mouse_t mouse_report);
i2c_status_t read_i2c_joystiic(joystiic_data_t* data);
