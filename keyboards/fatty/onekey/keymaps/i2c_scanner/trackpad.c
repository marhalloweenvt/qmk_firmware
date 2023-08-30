#include "trackpad.h"
#include "i2c_master.h"
#include "wait.h"
#include "timer.h"
#include "print.h"
// joystiic_data_t joystiic_data = {0};

// Scroll Indicator
bool isScrollMode = false;

// Set Parameters
uint16_t minAxisValue = ANALOG_JOYSTICK_AXIS_MIN;
uint16_t maxAxisValue = ANALOG_JOYSTICK_AXIS_MAX;

uint8_t maxCursorSpeed = ANALOG_JOYSTICK_SPEED_MAX;
uint8_t speedRegulator = ANALOG_JOYSTICK_SPEED_REGULATOR; // Lower Values Create Faster Movement

int8_t xOrigin, yOrigin;

uint16_t lastCursor = 0;

void joystiic_device_init(void) {
    i2c_init();
    i2c_start(JOYSTIIC_DEFAULT_ADDR << 1);
    // Account for drift
    joystiic_data_t joystiic_data = {0};

    joystiic_status = read_i2c_joystiic(&joystiic_data);
    xOrigin         = joystiic_data.X_MSB << 8;
    yOrigin         = joystiic_data.Y_MSB << 8;
}

i2c_status_t read_i2c_joystiic(joystiic_data_t* data) {
    i2c_status_t status = i2c_receive(JOYSTIIC_DEFAULT_ADDR << 1, (uint8_t*)data, sizeof(*data), I2C_READ_TIMEOUT);
#ifdef POINTING_DEVICE_DEBUG
    static uint16_t d_timer;
    if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
        dprintf("stick_id: %d, stick_fwMajor: %d, stick_fwMinor: %d, stick_X_MSB: %d, stick_X_LSB: %d, stick_Y_MSB: %d, stick_Y_LSB: %d, stick_button_state: %d, stick_button_status: %d, stick_i2cLock: %d, stick_i2cAddress: %d\n", stick_id, stick_fwMajor, stick_fwMinor, stick_X_MSB, stick_X_LSB, stick_Y_MSB, stick_Y_LSB, stick_button_state, stick_button_status, stick_i2cLock, stick_i2cAddress);
        d_timer = timer_read();
    }
#endif
    return status;
}

int16_t axisCoordinate(uint16_t data, uint16_t origin) {
    int8_t  direction;
    int16_t distanceFromOrigin;
    int16_t range;

    int16_t position = data;

    if (origin == position) {
        return 0;
    } else if (origin > position) {
        distanceFromOrigin = origin - position;
        range              = origin - minAxisValue;
        direction          = -1;
    } else {
        distanceFromOrigin = position - origin;
        range              = maxAxisValue - origin;
        direction          = 1;
    }

    float   percent    = (float)distanceFromOrigin / range;
    int16_t coordinate = (int16_t)(percent * 100);
    if (coordinate < 0) {
        return 0;
    } else if (coordinate > 100) {
        return 100 * direction;
    } else {
        return coordinate * direction;
    }
}

int8_t axisToMouseComponent(uint16_t  data, int8_t origin, uint8_t maxSpeed) {
    int16_t coordinate = axisCoordinate(data, origin);
    if (coordinate != 0) {
        float percent = (float)coordinate / 100;
        return percent * maxCursorSpeed * (abs(coordinate) / speedRegulator);
    } else {
        return 0;
    }
}

report_joystiic_t i2c_joystick_read(void) {
    report_joystiic_t report        = {0};
    joystiic_data_t   joystiic_data = {0};
    joystiic_status                 = read_i2c_joystiic(&joystiic_data);
    if (timer_elapsed(lastCursor) > ANALOG_JOYSTICK_READ_INTERVAL) {
        lastCursor = timer_read();
        report.x   = axisToMouseComponent(((joystiic_data.X_MSB << 8 | joystiic_data.X_LSB) >> 6), xOrigin, maxCursorSpeed);
        report.y   = axisToMouseComponent(((joystiic_data.Y_MSB << 8 | joystiic_data.Y_LSB) >> 6), yOrigin, maxCursorSpeed);
    }
    report.button = !joystiic_data.button_state;

    return report;
}

// report_mouse_t i2c_joystick_get_report(report_mouse_t mouse_report) {
//     report_joystiic_t data = i2c_joystick_read();

//     dprintf("Raw ] X: %d, Y: %d\n", data.x, data.y);

//     if (isScrollMode) {
//         mouse_report.h = data.y;
//         mouse_report.v = data.x;
//     } else {
//         mouse_report.x = data.y;
//         mouse_report.y = data.x;
//     }

//     mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, data.button, POINTING_DEVICE_BUTTON1);

//     return mouse_report;
// }

// clang-format off
// const pointing_device_driver_t pointing_device_driver = {
//     .init       = joystiic_device_init,
//     .get_report = i2c_joystick_get_report,
//     .set_cpi    = NULL,
//     .get_cpi    = NULL
// };

// void pointing_device_init(void){joystiic_device_init()};

// bool pointing_device_task(void) {
//     report_mouse_t currentReport = i2c_joystick_get_report();
//     pointing_device_set_report(currentReport);

//     return pointing_device_send();
// }

void           pointing_device_driver_init(void) {joystiic_device_init();};

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {

    report_joystiic_t data = i2c_joystick_read();

    #ifdef POINTING_DEVICE_DEBUG
        static uint16_t d_timer;
        if (timer_elapsed(d_timer) > 1000) {
            dprintf("Raw X: %d, Y: %d\n", data.x, data.y);
            d_timer = timer_read();
        }
    #endif

    if (isScrollMode) {
        mouse_report.h = data.y/8;
        mouse_report.v = -(data.x/8);
    } else {
        mouse_report.x = data.y;
        mouse_report.y = data.x;
    }

    mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, data.button, POINTING_DEVICE_BUTTON1);

    return mouse_report;

 }
