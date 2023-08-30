// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Encoder
#define ENCODER_PUSHBUTTON_PIN1 GP7
#define ENCODER_PUSHBUTTON_PIN2 GP8

// SPI Configuration
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP18
#define SPI_MOSI_PIN GP19
#define SPI_MISO_PIN GP4

// RGB configuration
#define RGB_ENABLE_PIN GP6
#define RGB_MATRIX_LED_COUNT 1
#define RGBLED_NUM 1
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CYCLE_ALL

// ADC Configuration
#define ADC_RESOLUTION 12
#define ADC_SATURATION ((1 << 12) - 1)
#define ADC_CURRENT_PIN GP26
#define ADC_VOLTAGE_PIN GP27

// Display Configuration
#define TFT_CS_PIN GP16
#define TFT_DC_PIN GP17
#define TFT_RST_PIN GP20

//EEPROM
#define TRANSIENT_EEPROM_SIZE 64
