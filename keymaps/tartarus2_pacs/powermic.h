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

// Bit positions taken from the real USBPcap capture of the device
// (D:\full_powerscribe_powermic.txt, idVendor 0x0554 "Dictaphone Corp.",
// interface 3, HID, 64-byte report descriptor, EP 0x81, bInterval 10ms).
//
// Wireshark prints "HID Data" as bytes in transmission order, so a captured
// value of 010000 means byte0 = 0x01 -- NOT a big-endian 24-bit integer. The
// 24-bit "codes" in .notes/requirements.txt read the other way round, which
// puts Trigger, Enter/Select and Custom Right in the wrong places.
//
// Cross-checked against Nuance's own PMII_MICBUTTONS table
// (Powerscribe Documentation/Nuance PowerMic API_unlocked.pdf p.39), whose
// 16-bit mask equals byte1 | (byte2 << 8). All nine documented buttons agree.
// Trigger sits in byte0, outside that mask, because it belongs to the
// scanner -- a separate device in Nuance's object model.
//
// Index below is the QMK joystick button index, which equals byte * 8 + bit.
enum powermic_button {
    PM_TRIGGER      = 0,  // byte0 bit0 -- wire 01 00 00 (scanner)
    PM_TRANSCRIBE   = 8,  // byte1 bit0 -- wire 00 01 00 -- SDK 0x0001
    PM_TAB_BACK     = 9,  // byte1 bit1 -- wire 00 02 00 -- SDK 0x0002
    PM_DICTATE      = 10, // byte1 bit2 -- wire 00 04 00 -- SDK 0x0004
    PM_TAB_FORWARD  = 11, // byte1 bit3 -- wire 00 08 00 -- SDK 0x0008
    PM_REWIND       = 12, // byte1 bit4 -- wire 00 10 00 -- SDK 0x0010
    PM_FFWD         = 13, // byte1 bit5 -- wire 00 20 00 -- SDK 0x0020
    PM_STOP_PLAY    = 14, // byte1 bit6 -- wire 00 40 00 -- SDK 0x0040
    PM_CUSTOM_LEFT  = 15, // byte1 bit7 -- wire 00 80 00 -- SDK 0x0080
    PM_ENTER_SELECT = 16, // byte2 bit0 -- wire 00 00 01 -- SDK 0x0100
    PM_CUSTOM_RIGHT = 17, // byte2 bit1 -- wire 00 00 02
};

// Mirror a physical PowerMic button. Press on key-down, release on key-up,
// so the host sees the same press/release pair a real handset produces.
void powermic_button_press(uint8_t button);
void powermic_button_release(uint8_t button);
