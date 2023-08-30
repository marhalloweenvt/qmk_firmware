/*
This is the c configuration file for the keymap

Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2015 Jack Humbert

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

/* Select hand configuration */

#define POINTING_DEVICE_DEBUG
#define DS1307_ADDR (0x68 << 1)

// OLED display
#define OLED_DISPLAY_128X64
#define OLED_DISPLAY_HEIGHT 64
#define OLED_DISPLAY_WIDTH 128
#define OLED_MATRIX_SIZE 1024
#define OLED_UPDATE_INTERVAL 50
#define OLED_FONT_H "./keymaps/default/glcdfont_jedi.c"
#define OLED_TIMEOUT 0
#define OLED_RESET -1

/* I2C Driver */
#define I2C1_SCL_PIN GP13
#define I2C1_SDA_PIN GP12
#define I2C_DRIVER I2CD1
#define I2C1_CLOCK_SPEED 90000
