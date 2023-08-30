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

// parameters for reading the joystick
const int   range         = 10;        // output range of X or Y movement (zero to range)
int         responseDelay = 5;         // response delay of the mouse, in ms
int         threshold     = range / 4; // resting threshold
int         center        = range / 2; // resting position value
const float powerValue    = 1.8;       // for exponential behavior, 1 < value < 2

// Variables will change:
bool buttonState;             // the current reading from the input pin
bool lastButtonState = false; // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
uint8_t lastDebounceTime = 0;  // the last time the output pin was toggled
uint8_t debounceDelay    = 50; // the debounce time; increase if the output flickers

static int16_t convertDeltaToInt(uint8_t high, uint8_t low) {
    // join bytes into twos compliment
    uint16_t twos_comp = ((high << 8) | low) >> 6;

    // convert twos comp to int
    if (twos_comp & 0x8000) return -1 * (~twos_comp + 1);

    return twos_comp;
}

static int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int16_t readAxis(int16_t thisAxisValue) {
    int16_t reading = thisAxisValue; // read the analog input

    // map the reading from the analog input range to the output range
    int16_t readingMap = map(reading, 100, 850, 0, range);

    // if the output reading is outside from the rest position threshold, use it
    int8_t distance = center - readingMap;

    if (abs(distance) < threshold) { // if distance not to threshold, no move
        distance = 0;                // prevents tiny jitters due to readings
    }

    // static uint16_t d_timer;
    // if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
    //     dprintf("Axis value X: %d, readingMap: %d, distance: %d\n", reading, readingMap,distance);
    //     d_timer = timer_read();
    // }

    return distance; // return the distance for this axis
}

void joystiic_device_init(uint8_t address) {
    i2c_start(address << 1);
    // Account for drift
    uint8_t joystiic_data[4] = {};
    // joystiic_status = read_i2c_joystiic(&joystiic_data, address);
    i2c_status_t status = i2c_readReg(address << 1, JOYSTIIC_COMMAND_MSB_HORIZONTAL, joystiic_data, 4, I2C_READ_TIMEOUT);
    while (status != I2C_STATUS_SUCCESS) {
        status = i2c_readReg(address << 1, JOYSTIIC_COMMAND_MSB_HORIZONTAL, joystiic_data, 4, I2C_READ_TIMEOUT);
    }
    xOrigin = convertDeltaToInt(joystiic_data[0], joystiic_data[1]);
    yOrigin = convertDeltaToInt(joystiic_data[2], joystiic_data[3]);
    while (xOrigin > 1000 || yOrigin > 1000) {
        status  = i2c_readReg(address << 1, JOYSTIIC_COMMAND_MSB_HORIZONTAL, joystiic_data, 6, I2C_READ_TIMEOUT);
        xOrigin = convertDeltaToInt(joystiic_data[0], joystiic_data[1]);
        yOrigin = convertDeltaToInt(joystiic_data[2], joystiic_data[3]);
    }
    // dprintf("xOrigin: %d, yOrigin: %d\n", xOrigin, yOrigin);
}

i2c_status_t read_i2c_joystiic(joystiic_data_t *data, uint8_t address) {
    // i2c_status_t status = i2c_receive(JOYSTIIC_DEFAULT_ADDR << 1, (uint8_t*)data, sizeof(*data), I2C_READ_TIMEOUT);
    i2c_status_t status = i2c_readReg(address << 1, JOYSTIIC_COMMAND_MSB_HORIZONTAL, (uint8_t *)data, sizeof(*data), I2C_READ_TIMEOUT);
    // #ifdef POINTING_DEVICE_DEBUG
    //     static uint16_t d_timer;
    //     if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
    //         dprintf("stick_id: %d, stick_fwMajor: %d, stick_fwMinor: %d, stick_X_MSB: %d, stick_X_LSB: %d, stick_Y_MSB: %d, stick_Y_LSB: %d, stick_button_state: %d, stick_button_status: %d, stick_i2cLock: %d, stick_i2cAddress: %d\n", data->id, data->fwMajor, data->fwMinor, data->X_MSB, data->X_LSB, data->Y_MSB, data->Y_LSB, data->button_state, data->button_status, data->i2cLock, data->i2cAddress);
    //         dprintf("stick_X_MSB: %d, stick_X_LSB: %d, stick_Y_MSB: %d, stick_Y_LSB: %d, stick_button_state: %d, stick_button_status: %d\n", data->X_MSB, data->X_LSB, data->Y_MSB, data->Y_LSB, data->button_state, data->button_status);
    //         dprintf("xOrigin: %d, yOrigin: %d\n", xOrigin, yOrigin);
    //         d_timer = timer_read();
    //     }
    // #endif
    return status;
}

int16_t axisCoordinate(int16_t data, int16_t origin) {
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

bool isInRange(int lowerLimit, int upperLimit, int no) {
    return (lowerLimit < no && no < upperLimit);
}

int8_t mouseLinearAcc(int16_t data) {
    if (data > 0)
        data = (int8_t)pow(powerValue, data);
    else if (data < 0)
        data = -(int8_t)pow(powerValue, abs(data));
    return data;
}

report_joystiic_t i2c_joystick_read(uint8_t address) {
    report_joystiic_t report        = {0};
    joystiic_data_t   joystiic_data = {0};
    static uint8_t    error_count   = 0;
    if (error_count < 10) {
        joystiic_status = read_i2c_joystiic(&joystiic_data, address);
        if (joystiic_status == I2C_STATUS_SUCCESS) {
            error_count       = 0;
            lastCursor        = timer_read();
            int16_t xPosition = convertDeltaToInt(joystiic_data.X_MSB, joystiic_data.X_LSB);
            int16_t yPosition = convertDeltaToInt(joystiic_data.Y_MSB, joystiic_data.Y_LSB);

            report.x = axisToMouseComponent(xPosition, xOrigin, maxCursorSpeed);
            report.y = axisToMouseComponent(yPosition, yOrigin, maxCursorSpeed);

            // report.x = mouseLinearAcc(readAxis(xPosition));
            // report.y = mouseLinearAcc(readAxis(yPosition));

            bool     reading = !joystiic_data.button_state;
            uint32_t now     = timer_read32();
            if (reading != lastButtonState) {
                lastDebounceTime = timer_read();
            }
            if ((now - lastDebounceTime) > debounceDelay) {
                if (reading != buttonState) {
                    buttonState = reading;
                    if (buttonState == true) {
                        report.button = !joystiic_data.button_state;
                    }
                }
            }
            now              = 0;
            lastDebounceTime = 0;
            lastButtonState  = reading;

            // report.button = !joystiic_data.button_state;

            static uint16_t d_timer;
            if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
                // dprintf("Position X: %d, Y: %d\nReport X: %d, Y: %d\nOrigin X: %d, Y:%d\n", xPosition, yPosition, report.x, report.y, xOrigin, yOrigin);
                // dprintf("report.button: %d\n", report.button);
                // dprintf("Report X: %d, Y: %d\n", report.x, report.y);
                d_timer = timer_read();
            }

        } else {
            error_count++;
        }
    }
    // if (timer_elapsed(lastCursor) > ANALOG_JOYSTICK_READ_INTERVAL) {
    //     if (joystiic_status)
    //     lastCursor = timer_read();
    //     uint16_t xPosition = (joystiic_data.X_MSB << 8 | joystiic_data.X_LSB) >> 6;
    //     uint16_t yPosition = (joystiic_data.Y_MSB << 8 | joystiic_data.Y_LSB) >> 6;
    //     report.x   = axisToMouseComponent(xPosition, xOrigin, maxCursorSpeed);
    //     report.y   = axisToMouseComponent(yPosition, yOrigin, maxCursorSpeed);
    //     // dprintf("button state: %d, button status: %d\n", joystiic_data.button_state,joystiic_data.button_status);
    //     // bool xInRange = isInRange((JOYSTIIC_CENTER - JOYSTIIC_DEADZONE), (JOYSTIIC_CENTER + JOYSTIIC_DEADZONE), xPosition);
    //     // if (!xInRange)
    //     // bool yInRange = isInRange((JOYSTIIC_CENTER - JOYSTIIC_DEADZONE), (JOYSTIIC_CENTER + JOYSTIIC_DEADZONE), yPosition);
    //     // if (!yInRange)

    //     // dprintf("xOrigin: %ls, yOrigin: %ls\n", xOrigin, yOrigin);
    //     // dprintf("xPosition: %d, yPosition: %d\n", xPosition, yPosition);
    //     // dprintf("Report X: %d, Report Y: %d\n", report.x, report.y);

    // }
    // report.button = !joystiic_data.button_state;

    return report;
}

void pointing_device_driver_init(void){};

// report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {

//     report_joystiic_t data1 = i2c_joystick_read(JOYSTIIC1_DEFAULT_ADDR);
//     // report_joystiic_t data2 = i2c_joystick_read(JOYSTIIC2_DEFAULT_ADDR);

//     #ifdef POINTING_DEVICE_DEBUG
//         static uint16_t d_timer;
//         if (timer_elapsed(d_timer) > JOYSTIIC_DEBUG_INTERVAL) {
//             // dprintf("Raw X: %d, Y: %d\n", data1.x, data1.y);
//             // dprintf("Raw X: %d, Y: %d\n", data2.x, data2.y);
//             d_timer = timer_read();
//         }
//     #endif

//     mouse_report.x = data1.x;
//     mouse_report.y = data1.y;

//     if (isScrollMode) {
//         // mouse_report.h = data1.y/8;
//         // mouse_report.v = -(data1.x/8);
//         // mouse_report.h = data2.y/8;
//         // mouse_report.v = -(data2.x/8);
//     } else {
//         // mouse_report.x = data1.x;
//         // mouse_report.y = data1.y;
//         // mouse_report.x = data2.x;
//         // mouse_report.y = data2.y;
//     }

//     mouse_report.buttons = pointing_device_handle_buttons(mouse_report.buttons, data1.button, POINTING_DEVICE_BUTTON1);

//     return mouse_report;
//  }
bool dpl_adv;

bool pointing_device_task(void) {
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
    dpl_adv                      = data1.button;
    report_mouse_t currentReport = pointing_device_get_report();

    if (is_keyboard_master()) {
        if (isScrollMode) {
            currentReport.h = -data1.x / SCROLL_SPEED_DIVIDER;
            currentReport.v = data1.y / SCROLL_SPEED_DIVIDER;
        } else {
            currentReport.x = -data1.x;
            currentReport.y = data1.y;
        }
    } else {
        if (isScrollMode) {
            currentReport.h = -data1.y / SCROLL_SPEED_DIVIDER;
            currentReport.v = data1.x / SCROLL_SPEED_DIVIDER;
        } else {
            currentReport.x = -data1.y;
            currentReport.y = data1.x;
        }
    }
    // currentReport.buttons = pointing_device_handle_buttons(currentReport.buttons, data1.button, POINTING_DEVICE_BUTTON1);
    pointing_device_set_report(currentReport);
    wait_ms(responseDelay);
    return pointing_device_send();
}
