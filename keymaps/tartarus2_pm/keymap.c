// Minimal PowerMic-emulation test keymap. Goal: present the PowerMic HID button
// interface at USB interface number 3 (MI_03), matching a genuine PowerMic, so
// PowerScribe/usbmgr reads it. Keys are the production F-codes; key 20 fires the
// real PowerMic Dictate button (Button 3). RAW HID carries the reflash trigger
// and a bench opcode to fire any PowerMic button.
#include QMK_KEYBOARD_H
#include "joystick.h"
#include "powermic.h"
#include "raw_hid.h"

enum custom_keycodes {
    PM_DICT = QK_KB_0,  // PowerMic Dictate (Button 3)
};

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length < 1) return;
    switch (data[0]) {
        case 0xB0:  // reflash: drop to the UF2 bootloader
            bootloader_jump();
            break;
        case 0xD6:  // fire PowerMic button: [0xD6, button, down]
            if (data[2]) {
                powermic_button_press(data[1]);
            } else {
                powermic_button_release(data[1]);
            }
            break;
        default:
            break;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == PM_DICT) {
        if (record->event.pressed) {
            powermic_button_press(PM_DICTATE);
        } else {
            powermic_button_release(PM_DICTATE);
        }
        return false;
    }
    return true;
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
        KC_F13,      KC_F14,      KC_F15,      KC_F16,      KC_F17,
        KC_F18,      KC_F19,      KC_F20,      KC_F21,      KC_F22,
        KC_F23,      KC_F24,    C(KC_F13),   C(KC_F14),   C(KC_F15),
        C(KC_F16),   C(KC_F17),   C(KC_F18),   C(KC_F19),
        KC_TRNS,     KC_TRNS,     KC_LALT,     PM_DICT,
        S(KC_F13),   S(KC_F14),   S(KC_F15),   S(KC_F16)
    ),
};
