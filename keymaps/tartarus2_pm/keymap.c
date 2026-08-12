// PowerMic-emulation keymap. Presents ONE clean PowerMic HID button interface so
// PowerScribe's HID manager binds it unambiguously (no raw-HID interface sharing
// our VID/PID as a second candidate). Keys are the production F-codes; key 20
// fires the real PowerMic Dictate button (Button 3). Reflash without BOOTSEL is
// still available two ways: the EP0 feature-report hatch (usb_main.c, on the
// PowerMic interface) and the 01+05+20 keypad chord below.
#include QMK_KEYBOARD_H
#include "joystick.h"
#include "powermic.h"

// One keycode per PowerMic button. Drop any of these into the LAYOUT below to
// assign it to a Tartarus key. PM_DICT is latched (toggle); every other button
// is momentary (pressed while the key is held), exactly like the real handset.
enum custom_keycodes {
    PM_DICT = QK_KB_0,  // Dictate / Record   (toggle)
    PM_TRANS,           // Transcribe
    PM_TBACK,           // Tab backward
    PM_TFWD,            // Tab forward
    PM_REW,             // Rewind
    PM_FF,              // Fast forward
    PM_STOP,            // Stop / Play
    PM_CUSTL,           // Custom left
    PM_CUSTR,           // Custom right
    PM_ENT,             // Enter / Select
};

// keycode -> PowerMic button index (powermic.h). PM_DICT handled separately.
static uint8_t pm_button_for(uint16_t keycode) {
    switch (keycode) {
        case PM_TRANS: return PM_TRANSCRIBE;
        case PM_TBACK: return PM_TAB_BACK;
        case PM_TFWD:  return PM_TAB_FORWARD;
        case PM_REW:   return PM_REWIND;
        case PM_FF:    return PM_FFWD;
        case PM_STOP:  return PM_STOP_PLAY;
        case PM_CUSTL: return PM_CUSTOM_LEFT;
        case PM_CUSTR: return PM_CUSTOM_RIGHT;
        case PM_ENT:   return PM_ENTER_SELECT;
        default:       return 0xFF;  // not a PowerMic button
    }
}

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

// Dictate is latched (toggle): one tap starts dictation and HOLDS the PowerMic
// Dictate button down; the next tap releases it. PowerScribe is in press-to-hold
// mode, so a held button = continuous dictation - which makes a single tap
// start/stop, i.e. toggle, with nothing to configure host-side.
static bool dictate_latched = false;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    boot_chord_track(keycode, record->event.pressed);

    // Dictate: latched toggle (tap on / tap off).
    if (keycode == PM_DICT) {
        if (record->event.pressed) {
            dictate_latched = !dictate_latched;
            if (dictate_latched) powermic_button_press(PM_DICTATE);
            else                 powermic_button_release(PM_DICTATE);
        }
        return false;
    }

    // Every other PowerMic button: momentary (mirror the physical key).
    uint8_t btn = pm_button_for(keycode);
    if (btn != 0xFF) {
        if (record->event.pressed) powermic_button_press(btn);
        else                       powermic_button_release(btn);
        return false;
    }
    return true;
}

// To assign a PowerMic button to a key, just put its keycode in the slot below.
// Available: PM_DICT (toggle) PM_TRANS PM_TBACK PM_TFWD PM_REW PM_FF PM_STOP
//            PM_CUSTL PM_CUSTR PM_ENT. Anything else is a normal keycode.
// Default: 20 main keys stay F13-F24/Ctrl/Shift (PACS via AHK); the D-pad +
// scroll wheel carry the PowerMic transport, key 20 = Dictate.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
        // main 4x5 grid (keys 01-19) -- F-codes for PACS macros
        KC_F13,      KC_F14,      KC_F15,      KC_F16,      KC_F17,
        KC_F18,      KC_F19,      KC_F20,      KC_F21,      KC_F22,
        KC_F23,      KC_F24,    C(KC_F13),   C(KC_F14),   C(KC_F15),
        C(KC_F16),   C(KC_F17),   C(KC_F18),   C(KC_F19),
        // scroll-up   scroll-down   thumb        key 20 (big)
        PM_STOP,      PM_ENT,       KC_LALT,     PM_DICT,
        // D-pad: left        right        up           down
        PM_REW,       PM_FF,        PM_TFWD,     PM_TBACK
    ),
};
