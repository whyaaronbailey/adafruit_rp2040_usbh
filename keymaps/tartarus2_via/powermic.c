#include "powermic.h"
#include "joystick.h"

// The device declares Usage Maximum (14), so anything else changes the button
// numbering the host sees and stops matching the capture.
_Static_assert(JOYSTICK_BUTTON_COUNT == 14,
               "PowerMic declares Buttons 1..14 (Usage Maximum 0x0e)");

// The axis reproduces the device's constant byte 0. Without it, buttons start
// at bit 0 and every HID Button number comes out 8 too high.
_Static_assert(JOYSTICK_AXIS_COUNT == 1 && JOYSTICK_AXIS_RESOLUTION == 8,
               "PowerMic byte 0 is padding; one 8-bit axis reproduces it");

// report_joystick_t is { int8_t axes[1]; uint8_t buttons[2]; } PACKED.
_Static_assert(JOYSTICK_AXIS_COUNT + ((JOYSTICK_BUTTON_COUNT - 1) / 8 + 1) == 3,
               "PowerMic input report must be exactly 3 bytes");

void powermic_button_press(uint8_t button) {
    // register_joystick_button() sets the bit and flushes the report itself.
    register_joystick_button(button);
}

void powermic_button_release(uint8_t button) {
    unregister_joystick_button(button);
}
