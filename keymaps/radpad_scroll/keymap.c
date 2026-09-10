// RadPad - a trackball dedicated to scrolling CT/MRI slices.
//
// The ball drives the scroll wheel and never moves the cursor. A second
// trackball plugged straight into the PC handles pointing, so the two roles
// stay physically separated.
//
// Policy is matched per attached device, first hit wins, so this same firmware
// handles a board with two trackballs on a hub doing different jobs. To turn
// this board back into a normal pointer later, change one enum below.
#include QMK_KEYBOARD_H
#include "radpad_pointer.h"

const radpad_policy_t radpad_policies[] = {
    // Kensington Orbit wireless. Confirm the PID from the diagnostics before
    // trusting it - wireless receivers often report the dongle's id, not the
    // ball's.
    { .vid = 0x047D, .pid = 0x2048,
      .mode = PTR_SCROLL_V, .divisor = 12, .accel = 40, .max_gain = 800,
      .keep_buttons = true },

    // TEST ENTRY: any Razer device attached to this board also scrolls, so the
    // whole path can be proven with a Razer mouse while the Orbit's dongle is
    // missing. Delete once the Orbit is in use.
    { .vid = 0x1532,
      .mode = PTR_SCROLL_V, .divisor = 16, .accel = 40, .max_gain = 800,
      .keep_buttons = true },

    // Catch-all: anything else behaves as a normal pointer.
    { .mode = PTR_CURSOR },
};
const uint8_t radpad_policy_count = ARRAY_SIZE(radpad_policies);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};
