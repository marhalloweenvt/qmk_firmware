// Copyright 2018-2022 Nick Brassel (@tzarc)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include QMK_KEYBOARD_H
#include "analog.h"
#include "qp.h"
#include "qp_comms.h"
#include "qp_st77xx_opcodes.h"
#include "print.h"

bool qp_st7735_init(painter_device_t device, painter_rotation_t rotation) {
    // clang-format off
    const uint8_t st7735_init_sequence[] = {
        // Command,                 Delay, N, Data[N]
        ST77XX_CMD_RESET,            120,  0,
        ST77XX_CMD_SLEEP_OFF,          5,  0,
        ST77XX_SET_PIX_FMT,            0,  1, 0x55,
        ST77XX_CMD_INVERT_ON,          0,  0,
        ST77XX_CMD_NORMAL_ON,          0,  0,
        ST77XX_CMD_DISPLAY_ON,        20,  0
    };
    // clang-format on
    qp_comms_bulk_command_sequence(device, st7735_init_sequence, sizeof(st7735_init_sequence));

    // Configure the rotation (i.e. the ordering and direction of memory writes in GRAM)
    const uint8_t madctl[] = {
        [QP_ROTATION_0]   = ST77XX_MADCTL_BGR,
        [QP_ROTATION_90]  = ST77XX_MADCTL_BGR | ST77XX_MADCTL_MX | ST77XX_MADCTL_MV,
        [QP_ROTATION_180] = ST77XX_MADCTL_BGR | ST77XX_MADCTL_MX | ST77XX_MADCTL_MY,
        [QP_ROTATION_270] = ST77XX_MADCTL_BGR | ST77XX_MADCTL_MV | ST77XX_MADCTL_MY,
    };
    qp_comms_command_databyte(device, ST77XX_SET_MADCTL, madctl[rotation]);
    return true;
}

#define NUM_ADC_READS 32
#define btn_pin GP14
#define right_pin GP13
#define left_pin GP12
#define down_pin GP11
#define up_pin GP10
// #define bounce_interval 30
// #define base_move_pixels 5
// #define exponential_bound 15
// #define exponential_base 1.2

#include "graphics/thintel15.qff.c"

static painter_device_t       tft;
static painter_font_handle_t  font;
char wpm_str[10];
static lv_obj_t * btn1;
static lv_obj_t * btn2;
static lv_obj_t * label;
static int8_t rotations = 0;
bool lvgl_encoder = false;
static lv_group_t * g;
static uint32_t act_key = 0;

bool process_record_kb(uint16_t keycode, keyrecord_t* record) {
    if (!process_record_user(keycode, record)) {
        return false;
        // debug_emblem_config_to_console(&g_emblem_config);
    }
    switch (keycode) {
        case KC_1: {
            if (record->event.pressed) {
                lvgl_encoder = true;
                // dprintf("Key 1 is pressed");
            } else {
                lvgl_encoder = false;
                // dprintf("Key 1 is released");
            }
        }
        case KC_2: {
            if (record->event.pressed) {
                act_key = 1;
            } else {
                act_key = 0;
            }
            break;
        }
        case KC_3: {
            if (record->event.pressed) {
                act_key = 2;
            } else {
                act_key = 0;
            }
            break;
        }
        case KC_4: {
            if (record->event.pressed) {
                act_key = 3;
            } else {
                act_key = 0;
            }
            break;
        }
        case KC_5: {
            if (record->event.pressed) {
                act_key = 4;
            } else {
                act_key = 0;
            }
            break;
        }
        case KC_6: {
            if (record->event.pressed) {
                act_key = 5;
            } else {
                act_key = 0;
            }
            break;
        }
    }
    return true;
}

void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

void lv_example_btn_1(void)
{
    btn1 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn1, 100, 20);
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -20);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    btn2 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn2, 100, 20);
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);
}

// LVGL Encoder Control
lv_indev_t * indev_encoder;
lv_indev_t * indev_keypad;

void encoder_read_2(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    data->enc_diff = rotations;
    rotations = 0;
    if (lvgl_encoder) {
        data->state = LV_INDEV_STATE_PRESSED;
        // dprintf("pressed");
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
// LVGL Encoder Control

/*Will be called by the library to read the mouse*/
void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data){
    static uint32_t last_key = 0;
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
        case 5:
            act_key = LV_KEY_NEXT;
            break;
        case 2:
            act_key = LV_KEY_PREV;
            break;
        case 3:
            act_key = LV_KEY_LEFT;
            break;
        case 4:
            act_key = LV_KEY_RIGHT;
            break;
        case 1:
            act_key = LV_KEY_ENTER;
            break;
        }
        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    data->key = last_key;
}

void ui_init(void) {
    tft      = qp_st7735_make_spi_device(80, 160, TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN, 8, 0);
    font      = qp_load_font_mem(font_thintel15);

    qp_init(tft, QP_ROTATION_90);
    qp_set_viewport_offsets(tft, 1, 26);
    // qp_viewport(tft, 0, 0, 79, 159);
    // x = 160 -> Phương ngang, width
    // y = 80 -> Phương dọc, height
    // qp_drawimage(tft, 0, 0, image);
    qp_rect(tft, 0, 0, 159, 79, 0, 0, 0, true);
    qp_lvgl_attach(tft);
    lv_example_btn_1();
    // Register Encoder and create default group
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read_2;
    lv_indev_drv_register(&indev_drv);
    g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_t *cur_drv = NULL;
    for (;;) {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv) {
            break;
        }
        if (cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(cur_drv, g);
        }
        if (cur_drv->driver->type == LV_INDEV_TYPE_ENCODER) {
            lv_indev_set_group(cur_drv, g);
        }
    }
    wait_ms(50);
}

// Trackball interrupts accumulate over here. Processed on scan
// uint8_t trkBtnState = 0;

// volatile uint8_t tbUpCnt  = 0;
// volatile uint8_t tbDnCnt  = 0;
// volatile uint8_t tbLtCnt  = 0;
// volatile uint8_t tbRtCnt  = 0;

void ui_task(void) {


    // tbUpCnt = readPin(up_pin);
    // tbDnCnt = readPin(down_pin);
    // tbLtCnt = readPin(left_pin);
    // tbRtCnt = readPin(right_pin);
    // tbBtnCnt = readPin(btn_pin);

    // uint8_t count = 0;
    // uint32_t cur_state = readPin(up_pin);
    // uint8_t last_state = 0;
    // if (tbUpCnt == 1) {
    //     count++;
    // }
    // xprintf("U: %d Count: %d\n", tbUpCnt, count);

    // uprintf("Button 1 focus: %s\n", lv_obj_has_state(btn1, LV_STATE_FOCUSED) ? "true" : "false");
    // qp_rect(tft, 0, 0, 29, 29,       rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val(), true);
    // qp_line(tft, 0, 0, 0, 79, curr_hue, curr_sat, curr_val);
    // qp_line(tft, 0, 0, 159, 79, curr_hue, curr_sat, curr_val);
    // Draw a text message on the bottom-right of the 80x160 display on initialisation
    // qp_drawtext(tft, (160 - qp_textwidth(font, "Hello from QMK!")), (80 - font->line_height * 1) , font, "Hello from QMK!");

    // bool            hue_redraw = false;
    // static uint16_t last_hue   = 0xFFFF;
    // uint8_t         curr_hue   = rgb_matrix_get_hue();

    // static uint32_t last_read = 0;
    // if (timer_elapsed32(last_read) >= 1) {
    //     uprintf("Button 1 focus: %lu\n", readPin(GP10));
    //     last_read = timer_read32();
    // }

    // if (last_hue != curr_hue) {
    //     last_hue   = curr_hue;
    //     hue_redraw = true;
    // }

    // static led_t last_led_state = {0};
    // if (hue_redraw || last_led_state.raw != host_keyboard_led_state().raw) {
    //     last_led_state.raw = host_keyboard_led_state().raw;
    //     qp_drawimage_recolor(tft, 0, lock_caps->height * 0, lock_caps, curr_hue, 255, last_led_state.caps_lock ? 255 : 32, curr_hue, 255, 0);
    //     qp_drawimage_recolor(tft, 0, 79 / 2 - (lock_caps->height / 2), lock_num, curr_hue, 255, last_led_state.num_lock ? 255 : 32, curr_hue, 255, 0);
    //     qp_drawimage_recolor(tft, 0, (79 - lock_caps->height), lock_scrl, curr_hue, 255, last_led_state.scroll_lock ? 255 : 32, curr_hue, 255, 0);

    //     qp_rect(tft, 0, lock_caps->height + 1, lock_caps->width * 1, lock_caps->height + 3, curr_hue, 255, last_led_state.caps_lock ? 255 : 0, true);
    //     // qp_rect(tft, lock_caps->width * 1 + 1, lock_caps->height + 2, lock_caps->width * 2 - 1, lock_caps->height + 3, curr_hue, 255, last_led_state.num_lock ? 255 : 0, true);
    //     qp_rect(tft, 0, (79 - lock_caps->height) - 1, lock_caps->width * 1, (79 - lock_caps->height) - 3, curr_hue, 255, last_led_state.scroll_lock ? 255 : 0, true);
    // }
}

void keyboard_pre_init_user(void) {
  // Call the keyboard pre init code.

//   Set pins as input
//   setPinInput(up_pin);
//   setPinInput(down_pin);
//   setPinInput(left_pin);
//   setPinInput(right_pin);
//   setPinInput(btn_pin);
}

// #include <stdlib.h>

// struct Direction {
//     int pins[2];
//     int current_actions[2];
//     int last_actions[2];
//     int exponential;
//     double move_multiply;
//     unsigned long current_action_times[2];
//     unsigned long last_action_times[2];
// };

// void Direction_init(struct Direction* this, int pin1, int pin2) {
//     this->pins[0] = pin1;
//     this->pins[1] = pin2;
//     setPinInput(this->pins[0]);
//     setPinInput(this->pins[1]);
// }

// int Direction_read_action(struct Direction* this) {
//     for(int i = 0; i < 2; ++i) {
//         this->current_actions[i] = readPin(this->pins[i]);
//         this->current_action_times[i] = timer_read32();
//         if(this->current_actions[i] != this->last_actions[i]) {
//             this->last_actions[i] = this->current_actions[i];
//             this->exponential = (exponential_bound - (this->current_action_times[i] - this->last_action_times[i]));
//             this->exponential = (this->exponential > 0) ? this->exponential : 1;
//             this->move_multiply = exponential_base;
//             for(int j = 0; j < this->exponential; ++j) {
//                 this->move_multiply *= exponential_base;
//             }
//             this->last_action_times[i] = this->current_action_times[i];
//             if(i == 0) {
//                 return (-1) * base_move_pixels * this->move_multiply;
//             } else {
//                 return base_move_pixels * this->move_multiply;
//             }
//         }
//     }
//     return 0;
// }

// // button and debounce
// int btn_state;
// int btn_read_state;
// unsigned long btn_current_action_time;
// unsigned long btn_last_action_time;
// // mouse move
// void Direction
// struct Direction x_direction(left_pin, right_pin);
// struct Direction y_direction(up_pin, down_pin);
