// Copyright 2022 Stefan Kerkmann
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

/* Audio */
#undef RP_PWM_USE_PWM2
#define RP_PWM_USE_PWM2 TRUE

/* Backlight */
// #undef RP_PWM_USE_PWM4
// #define RP_PWM_USE_PWM4 TRUE

/* I2C Driver */
#undef RP_I2C_USE_I2C0
#undef RP_I2C_USE_I2C1
#define RP_I2C_USE_I2C0 FALSE
#define RP_I2C_USE_I2C1 TRUE
