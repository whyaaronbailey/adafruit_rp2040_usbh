#include "powermic.h"
#include "joystick.h"

// Trigger is bit 16, so anything less than 17 buttons silently drops it and
// truncates the report below the PowerMic's 3 bytes. The previous campaign's
// descriptor declared 14 buttons and hit exactly this.
_Static_assert(JOYSTICK_BUTTON_COUNT >= 17,
               "PowerMic needs JOYSTICK_BUTTON_COUNT >= 17 (Trigger is bit 16)");

// QMK sizes the report as buttons[(JOYSTICK_BUTTON_COUNT - 1) / 8 + 1].
// The PowerMic's report is 3 bytes; 24 buttons lands on exactly that with no
// padding item in the descriptor.
_Static_assert((JOYSTICK_BUTTON_COUNT - 1) / 8 + 1 == 3,
               "PowerMic report must be exactly 3 bytes (use JOYSTICK_BUTTON_COUNT 24)");

void powermic_button_press(uint8_t button) {
    // register_joystick_button() sets the bit and flushes the report itself.
    register_joystick_button(button);
}

void powermic_button_release(uint8_t button) {
    unregister_joystick_button(button);
}
