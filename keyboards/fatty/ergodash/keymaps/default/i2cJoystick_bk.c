#include "i2cJoystick.h"
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

int16_t xOrigin, yOrigin;

uint16_t lastCursor = 0;

void joystiic_device_init(uint8_t address) {
    i2c_init();
    i2c_start(address << 1);
    // Account for drift
    joystiic_data_t joystiic_data = {0};
    joystiic_status = read_i2c_joystiic(&joystiic_data, address);
    xOrigin         = (joystiic_data.X_MSB << 8 | joystiic_data.X_LSB) >> 6;
    yOrigin         = (joystiic_data.Y_MSB << 8 | joystiic_data.Y_LSB) >> 6;

    // dprintf("xOrigin: %d, yOrigin: %d\n", xOrigin, yOrigin);

}

i2c_status_t read_i2c_joystiic(joystiic_data_t* data, uint8_t address) {
    // i2c_status_t status = i2c_receive(JOYSTIIC_DEFAULT_ADDR << 1, (uint8_t*)data, sizeof(*data), I2C_READ_TIMEOUT);
    i2c_status_t status = i2c_readReg(address << 1, JOYSTIIC_COMMAND_MSB_HORIZONTAL, (uint8_t*)data, sizeof(*data), I2C_READ_TIMEOUT);
#ifdef POINTING_DEVICE_DEBUG
    static uint16_t d_timer;
    if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
        // dprintf("stick_id: %d, stick_fwMajor: %d, stick_fwMinor: %d, stick_X_MSB: %d, stick_X_LSB: %d, stick_Y_MSB: %d, stick_Y_LSB: %d, stick_button_state: %d, stick_button_status: %d, stick_i2cLock: %d, stick_i2cAddress: %d\n", data->id, data->fwMajor, data->fwMinor, data->X_MSB, data->X_LSB, data->Y_MSB, data->Y_LSB, data->button_state, data->button_status, data->i2cLock, data->i2cAddress);
        dprintf("stick_X_MSB: %d, stick_X_LSB: %d, stick_Y_MSB: %d, stick_Y_LSB: %d, stick_button_state: %d, stick_button_status: %d\n", data->X_MSB, data->X_LSB, data->Y_MSB, data->Y_LSB, data->button_state, data->button_status);
        dprintf("xOrigin: %d, yOrigin: %d\n", xOrigin, yOrigin);
        d_timer = timer_read();
    }
#endif
    i2c_stop();
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

int8_t axisToMouseComponent(uint16_t data, int16_t origin, uint8_t maxSpeed) {
    int16_t coordinate = axisCoordinate(data, origin);
    if (coordinate != 0) {
        float percent = (float)coordinate / 100;
        return percent * maxCursorSpeed * (abs(coordinate) / speedRegulator);
    } else {
        return 0;
    }
}

bool isInRange(int lowerLimit, int upperLimit, int no)
{
    return (lowerLimit < no && no < upperLimit);
}

report_joystiic_t i2c_joystick_read(uint8_t address) {
    report_joystiic_t report        = {0};
    joystiic_data_t   joystiic_data = {0};
    joystiic_status = read_i2c_joystiic(&joystiic_data, address);
    if (timer_elapsed(lastCursor) > ANALOG_JOYSTICK_READ_INTERVAL) {
        lastCursor = timer_read();
        uint16_t xPosition = (joystiic_data.X_MSB << 8 | joystiic_data.X_LSB) >> 6;
        uint16_t yPosition = (joystiic_data.Y_MSB << 8 | joystiic_data.Y_LSB) >> 6;
        report.x   = axisToMouseComponent(xPosition, xOrigin, maxCursorSpeed);
        report.y   = axisToMouseComponent(yPosition, yOrigin, maxCursorSpeed);
        // dprintf("button state: %d, button status: %d\n", joystiic_data.button_state,joystiic_data.button_status);
        // bool xInRange = isInRange((JOYSTIIC_CENTER - JOYSTIIC_DEADZONE), (JOYSTIIC_CENTER + JOYSTIIC_DEADZONE), xPosition);
        // if (!xInRange)
        // bool yInRange = isInRange((JOYSTIIC_CENTER - JOYSTIIC_DEADZONE), (JOYSTIIC_CENTER + JOYSTIIC_DEADZONE), yPosition);
        // if (!yInRange)

        // dprintf("xOrigin: %ls, yOrigin: %ls\n", xOrigin, yOrigin);
        // dprintf("xPosition: %d, yPosition: %d\n", xPosition, yPosition);
        // dprintf("Report X: %d, Report Y: %d\n", report.x, report.y);

    }
    report.button = !joystiic_data.button_state;


    return report;
}

void pointing_device_driver_init(void) {
};

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {

    report_joystiic_t data1 = i2c_joystick_read(JOYSTIIC1_DEFAULT_ADDR);
    // report_joystiic_t data2 = i2c_joystick_read(JOYSTIIC2_DEFAULT_ADDR);

    #ifdef POINTING_DEVICE_DEBUG
        static uint16_t d_timer;
        if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
            // dprintf("Raw X: %d, Y: %d\n", data1.x, data1.y);
            // dprintf("Raw X: %d, Y: %d\n", data2.x, data2.y);
            d_timer = timer_read();
        }
    #endif

    if (isScrollMode) {
        mouse_report.h = data1.y/8;
        mouse_report.v = -(data1.x/8);
        // mouse_report.h = data2.y/8;
        // mouse_report.v = -(data2.x/8);
    } else {
        mouse_report.x = data1.x;
        mouse_report.y = data1.y;
        // mouse_report.x = data2.x;
        // mouse_report.y = data2.y;
    }

    mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, data1.button, POINTING_DEVICE_BUTTON1);

    return mouse_report;
 }
