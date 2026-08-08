// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "quantum.h"

enum custom_keycodes {
    CTRL_F13 = SAFE_RANGE,
    CTRL_F14,
    CTRL_F15,
    CTRL_F16,
    CTRL_F17,
    CTRL_F18,
    CTRL_F19,
    CTRL_F21,
    SHIFT_F13,
    SHIFT_F14,
    SHIFT_F15,
    SHIFT_F16
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
            /*┌────────────┬────────────┬────────────┬────────────┬────────────┬ */
            /*│    01      │    02      │    03      │    04      │    05      │ */
                  KC_F13,      KC_F14,      KC_F15,      KC_F16,      KC_F17,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    06      │    07      │    08      │    09      │    10      │*/
                  KC_F18,      KC_F19,      KC_F20,      KC_F21,      KC_F22,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    11      │    12      │    13      │    14      │    15      │*/
                  KC_F23,      KC_F24,      CTRL_F13,    CTRL_F14,    CTRL_F15,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    16      │    17      │    18      │    19      │*/
                  CTRL_F16,    CTRL_F17,    CTRL_F18,    CTRL_F19,
            /*├────────────┼────────────┼────────────┼────────────┼*/
            /*│    SW      │    SC      │    THUMB      │    20   │*/
                  KC_TRNS,     KC_TRNS,     KC_LALT,     CTRL_F21,
            /*├────────────┼────────────┼────────────┼────────────┼*/
            /*│    LEFT    │    RIGHT   │    UP       │    DOWN   │ */
            /*│  Shift+F13 │  Shift+F14 │  Shift+F15  │  Shift+F16│ */
                 SHIFT_F13,   SHIFT_F14,   SHIFT_F15,   SHIFT_F16
            /*└────────────┴────────────┴────────────┴────────────┘*/
    ),

    [1] = LAYOUT_tartarus2(
            /*┌────────────┬────────────┬────────────┬────────────┬────────────┬ */
            /*│    01      │    02      │    03      │    04      │    05      │ */
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    06      │    07      │    08      │    09      │    10      │*/
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    11      │    12      │    13      │    14      │    15      │*/
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    16      │    17      │    18      │    19      │*/
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
            /*├────────────┼────────────┼────────────┼────────────┼*/
            /*│    SW      │    SC      │    THUMB      │    20   │*/
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
            /*├────────────┼────────────┼────────────┼────────────┼*/
            /*│    LEFT    │    RIGHT   │    UP       │    DOWN   │ */
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS
            /*└────────────┴────────────┴────────────┴────────────┘*/

    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {

        case CTRL_F13:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F13)));
            }
            return false;

        case CTRL_F14:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F14)));
            }
            return false;

        case CTRL_F15:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F15)));
            }
            return false;

        case CTRL_F16:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F16)));
            }
            return false;

        case CTRL_F17:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F17)));
            }
            return false;

        case CTRL_F18:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F18)));
            }
            return false;

        case CTRL_F19:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F19)));
            }
            return false;

        case CTRL_F21:
            if (record->event.pressed) {
                SEND_STRING(SS_LCTL(SS_TAP(X_F21)));
            }
            return false;

        case SHIFT_F13:
            if (record->event.pressed) {
                SEND_STRING(SS_LSFT(SS_TAP(X_F13)));
            }
            return false;

        case SHIFT_F14:
            if (record->event.pressed) {
                SEND_STRING(SS_LSFT(SS_TAP(X_F14)));
            }
            return false;

        case SHIFT_F15:
            if (record->event.pressed) {
                SEND_STRING(SS_LSFT(SS_TAP(X_F15)));
            }
            return false;

        case SHIFT_F16:
            if (record->event.pressed) {
                SEND_STRING(SS_LSFT(SS_TAP(X_F16)));
            }
            return false;
    }
    return true;
}
