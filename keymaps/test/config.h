#pragma once
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET // Activates the double-tap behavior
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U // Timeout window in ms in which the double tap can occur.
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP17 // Specify a optional status led by GPIO number which blinks when entering the bootloader

#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_MAX_SPEED 10
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE

#define LAYER_STATE_8BIT
#define OLED_DISPLAY_128X64
#define OLED_TIMEOUT 10000
#define OLED_FADE_OUT
#define OLED_FADE_OUT_INTERVAL 2

#define COMBO_TERM 100
#define LEADER_PER_KEY_TIMING
#define LEADER_TIMEOUT 300
#define LEADER_NO_TIMEOUT

#define AUDIO_PIN B6
#define AUDIO_CLICKY
#define NO_MUSIC_MODE

// tap dance and related
/* #define TAPPING_TERM 175 */
#define RETRO_TAPPING

#ifdef RGBLIGHT_ENABLE

// rgb layers
// underglow related
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

#define RGBLIGHT_LAYERS
#define RGBLIGHT_LAYERS_OVERRIDE_RGB_OFF
#define RGBLIGHT_LAYERS_RETAIN_VAL

// GPIO PIN (GP2)2 (physical pin 4)
#define WS2812_DI_PIN 2
#define USE_GET_MILLISECOND_TIMER

//layer blink
#define RGBLIGHT_LAYER_BLINK
#define RGBLIGHT_BLINK_DURATION 500

// custom
#define ENABLE_LAYER_LED
#endif
