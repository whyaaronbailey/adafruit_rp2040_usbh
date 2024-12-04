#pragma once
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET // Activates the double-tap behavior
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U // Timeout window in ms in which the double tap can occur.
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP25 // Specify a optional status led by GPIO number which blinks when entering the bootloader

#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_MAX_SPEED 7
#define MOUSEKEY_DELAY 0
#define MOUSEKEY_TIME_TO_MAX 40
#define MOUSEKEY_WHEEL_DELAY 0
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

#define TAPPING_TOGGLE 2
#define LAYER_STATE_8BIT
#define OLED_DISPLAY_128X64
#define OLED_TIMEOUT 10000
#define OLED_FADE_OUT
#define OLED_FADE_OUT_INTERVAL 2

#define COMBO_TERM 100

/// leader options - backslash mod_tap
#define LEADER_TIMEOUT 300
// time between each leader
#define LEADER_PER_KEY_TIMING
// no timeout after leader initialized
#define LEADER_NO_TIMEOUT

#ifdef AUDIO_ENABLE
#define AUDIO_PIN GP16
#define AUDIO_PWM_DRIVER PWMD0
#define AUDIO_PWM_CHANNEL RP2040_PWM_CHANNEL_A
#define AUDIO_ENABLE_TONE_MULTIPLEXING
#define AUDIO_TONE_MULTIPLEXING_RATE_DEFAULT 10
#define AUDIO_INIT_DELAY
#define STARTUP_SONG SONG(STARTUP_SOUND)
#endif
/* #define AUDIO_CLICKY */

// tap dance and related
#define TAPPING_TERM 300
#define RETRO_TAPPING

#ifdef RGBLIGHT_ENABLE
// #define WS2812_PIO_USE_PIO1

// rgb layers
// underglow related
#define RGBLIGHT_LAYERS
#define RGBLIGHT_SLEEP
#define RGBLIGHT_LED_COUNT 20
#define RGBLIGHT_LIMIT_VAL 180
#define RGBLIGHT_MAX_LAYERS 8

// effects
#define RGBLIGHT_EFFECT_ALTERNATING
/* #define RGBLIGHT_EFFECT_BREATHING */
/* #define RGBLIGHT_EFFECT_CHRISTMAS */
/* #define RGBLIGHT_EFFECT_KNIGHT */
/* #define RGBLIGHT_EFFECT_RAINBOW_MOOD */
/* #define RGBLIGHT_EFFECT_RAINBOW_SWIRL */
/* #define RGBLIGHT_EFFECT_RGB_TEST */
/* #define RGBLIGHT_EFFECT_SNAKE */
/* #define RGBLIGHT_EFFECT_STATIC_GRADIENT */
#define RGBLIGHT_EFFECT_TWINKLE

#define RGBLIGHT_LAYERS_OVERRIDE_RGB_OFF
#define RGBLIGHT_LAYERS_RETAIN_VAL

// GPIO PIN (GP2)2 (physical pin 4)
#define WS2812_DI_PIN 2
/* #define USE_GET_MILLISECOND_TIMER */

//layer blink
#define RGBLIGHT_LAYER_BLINK
#define RGBLIGHT_BLINK_DURATION 500

// custom
#define ENABLE_LAYER_LED
#endif
