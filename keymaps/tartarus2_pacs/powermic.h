// PowerMic II button emulation over QMK's own HID stack.
//
// Background: earlier attempts at this used TinyUSB's device API
// (tud_hid_n_report). That could never work here — this firmware runs
// TinyUSB in HOST mode only (CFG_TUD_ENABLED 0) for the PIO USB port the
// Tartarus plugs into. The DEVICE side, i.e. what the PC sees, is ChibiOS
// USB via QMK. Adding a TinyUSB device stack meant two drivers fighting
// over the same RP2040 USB peripheral.
//
// This implementation instead rides QMK's joystick HID interface, which
// ChibiOS already knows how to enumerate alongside the keyboard. With
// JOYSTICK_AXIS_COUNT 0 and JOYSTICK_BUTTON_COUNT 24, QMK emits a
// buttons-only, 3-byte input report on the Button usage page — the same
// shape as the PowerMic's. Because QMK packs buttons as
//
//     buttons[button / 8] |= 1 << (button % 8)
//
// the PowerMic's bit numbers ARE the joystick button indices, so the wire
// bytes come out identical. Dictate (bit 10) -> {0x00, 0x04, 0x00}.
//
// Why this is focus-independent: the Windows focus rule applies to the
// keyboard usage page (0x07). A Button-page collection is delivered by
// Raw Input to whoever registered for that usage, foreground or not.

#pragma once

#include <stdint.h>

// Bit positions from the USB capture recorded in
// keyboards/converter/adafruit_rp2040_usbh_pm/.notes/requirements.txt (L121-133).
// The comment on each line is the original captured 24-bit code.
enum powermic_button {
    PM_ENTER_SELECT = 0,  // 0x000001
    PM_CUSTOM_RIGHT = 1,  // 0x000002
    PM_TRANSCRIBE   = 8,  // 0x000100
    PM_TAB_BACK     = 9,  // 0x000200
    PM_DICTATE      = 10, // 0x000400
    PM_TAB_FORWARD  = 11, // 0x000800
    PM_REWIND       = 12, // 0x001000
    PM_FFWD         = 13, // 0x002000
    PM_STOP_PLAY    = 14, // 0x004000
    PM_CUSTOM_LEFT  = 15, // 0x008000
    PM_TRIGGER      = 16, // 0x010000
};

// Mirror a physical PowerMic button. Press on key-down, release on key-up,
// so the host sees the same press/release pair a real handset produces.
void powermic_button_press(uint8_t button);
void powermic_button_release(uint8_t button);
