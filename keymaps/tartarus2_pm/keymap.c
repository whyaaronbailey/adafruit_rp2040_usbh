// PowerMic-emulation keymap. Presents ONE clean PowerMic HID button interface so
// PowerScribe's HID manager binds it unambiguously (no raw-HID interface sharing
// our VID/PID as a second candidate). Keys are the production F-codes; key 20
// fires the real PowerMic Dictate button (Button 3). Reflash without BOOTSEL is
// still available two ways: the EP0 feature-report hatch (usb_main.c, on the
// PowerMic interface) and the 01+05+20 keypad chord below.
#include QMK_KEYBOARD_H
#include "joystick.h"
#include "powermic.h"

enum custom_keycodes {
    PM_DICT = QK_KB_0,  // PowerMic Dictate (Button 3)
};

// Bootloader chord: keys 01 + 05 + 20 held together drop to the UF2
// bootloader from the keypad itself, so a build with a broken raw-HID path
// can still be reflashed without touching the BOOTSEL button.
static uint8_t boot_chord = 0;

static void boot_chord_track(uint16_t keycode, bool pressed) {
    uint8_t bit;
    switch (keycode) {
        case KC_F13:  bit = 1 << 0; break;  // key 01
        case KC_F17:  bit = 1 << 1; break;  // key 05
        case PM_DICT: bit = 1 << 2; break;  // key 20
        default: return;
    }
    if (pressed) {
        boot_chord |= bit;
        if (boot_chord == 0b111) {
            bootloader_jump();
        }
    } else {
        boot_chord &= ~bit;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    boot_chord_track(keycode, record->event.pressed);
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
