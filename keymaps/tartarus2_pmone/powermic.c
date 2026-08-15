#include "powermic.h"
#include "joystick.h"
#ifdef RAW_ENABLE
#    include "raw_hid.h"
#    include <string.h>
#    include "usb_descriptor.h"  // RAW_EPSIZE
#endif

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

#ifdef RAW_ENABLE
// PowerScribe's PowermicCtrl binds the FIRST HID device matching 0554:1001, and
// its enumeration order is effectively arbitrary per machine (observed: our
// PowerMic collection ranked first on the dev box, raw first at the
// workstation). Rather than gamble on the order, mirror every button report
// onto the raw collection too: whichever collection PowerScribe binds, it
// receives the dictate bytes with the same leading layout as a real PowerMic
// ([pad][btn1-8][btn9-14], zero-padded to the raw report size). Nuance's parser
// reads the button mask at fixed offsets 1..2, which line up either way.
// usevia.app is request/response and ignores unsolicited frames, so mirroring
// does not disturb VIA programming.
static uint16_t pm_raw_mask = 0;

static void pm_raw_mirror(void) {
    uint8_t frame[RAW_EPSIZE] = {0};
    frame[0] = 0x00;                            // byte 0 padding, as the real device
    frame[1] = (uint8_t)(pm_raw_mask & 0xFF);   // buttons 1..8
    frame[2] = (uint8_t)(pm_raw_mask >> 8);     // buttons 9..14
    raw_hid_send(frame, sizeof(frame));
}
#endif

void powermic_button_press(uint8_t button) {
    // register_joystick_button() sets the bit and flushes the report itself.
    register_joystick_button(button);
#ifdef RAW_ENABLE
    pm_raw_mask |= (uint16_t)1 << button;
    pm_raw_mirror();
#endif
}

void powermic_button_release(uint8_t button) {
    unregister_joystick_button(button);
#ifdef RAW_ENABLE
    pm_raw_mask &= ~((uint16_t)1 << button);
    pm_raw_mirror();
#endif
}
