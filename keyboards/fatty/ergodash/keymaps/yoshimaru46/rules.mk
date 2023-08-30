MOUSEKEY_ENABLE = yes       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
NKRO_ENABLE = yes            # Enable N-Key Rollover
BACKLIGHT_ENABLE = no      # Enable keyboard backlight functionality
AUDIO_ENABLE = yes           # Audio output
AUDIO_DRIVER = pwm_hardware
RGBLIGHT_ENABLE = yes       # Enable WS2812 RGB underlight.
WS2812_DRIVER = vendor
KEY_OVERRIDE_ENABLE = yes
OLED_ENABLE = yes
OLED_DRIVER = SSD1306
OLED_TRANSPORT = i2c
POINTING_DEVICE_ENABLE = yes # Generic Pointer, not as big as mouse keys hopefully.
POINTING_DEVICE_DRIVER = custom
QUANTUM_LIB_SRC += i2c_master.c
SRC += i2cJoystick.c

