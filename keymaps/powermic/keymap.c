// Reference keymap for the portable PowerMic II module (users/powermic).
//
// Shows the minimum a keymap needs: include powermic.h and place the PM_K_*
// keycodes. The module's weak process_record_user() handles them, so this
// file defines no handlers at all. Key 20 dictates, the d-pad L/R move between
// PowerScribe fields, thumb+20 resyncs the dictate flag.
#include QMK_KEYBOARD_H
#include "powermic.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
        /*   01        02        03        04        05    */
        KC_F13,   KC_F14,   KC_F15,   KC_F16,   KC_F17,
        /*   06        07        08        09        10    */
        KC_F18,   KC_F19,   KC_F20,   KC_F21,   KC_F22,
        /*   11        12        13        14        15    */
        KC_F23,   KC_F24,   PM_K_REWIND, PM_K_FFWD, PM_K_STOPPLAY,
        /*   16        17        18        19              */
        PM_K_TRANSCRIBE, PM_K_CUSTOML, PM_K_ENTERSEL, PM_K_CUSTOMR,
        /*   SW        SC        THUMB     20              */
        KC_MS_WH_UP, KC_MS_WH_DOWN, MO(1),  PM_K_DICTATE,
        /*   LEFT      RIGHT     UP        DOWN            */
        PM_K_TABBACK, PM_K_TABFWD, KC_MS_UP, KC_MS_DOWN
    ),
    [1] = LAYOUT_tartarus2(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, PM_K_LED_RESYNC,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

void housekeeping_task_user(void) {
    powermic_housekeeping();  // auto-revert of a drifted dictate flag
}
