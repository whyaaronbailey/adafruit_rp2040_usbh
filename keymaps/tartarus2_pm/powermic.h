// PowerMic II button emulation over QMK's own HID stack.
//
// Background: earlier attempts used TinyUSB's device API (tud_hid_n_report).
// That could never work here -- this firmware runs TinyUSB in HOST mode only
// (CFG_TUD_ENABLED 0) for the PIO port the Tartarus plugs into, while the
// DEVICE side, i.e. what the PC sees, is ChibiOS USB via QMK. Enabling
// TinyUSB device mode put two USB device stacks on one RP2040 peripheral.
//
// This rides QMK's joystick HID interface purely as a transport, because
// ChibiOS already knows how to enumerate it alongside the keyboard.
//
// ---------------------------------------------------------------------------
// The real device's report descriptor
// ---------------------------------------------------------------------------
// Extracted with tshark from the genuine USBPcap capture at
//   J:\Projects\HID Remapper\hidremapper\2024-0420_powermic_only.pcapng
// frame 3613 (device 1.19, VID 0x0554 Dictaphone Corp., interface 3, HID,
// wDescriptorLength 64, EP 0x81 IN, bInterval 10ms). All 64 bytes:
//
//   05 01        Usage Page (Generic Desktop)
//   09 00        Usage (Undefined)
//   A1 01        Collection (Application)
//     05 08        Usage Page (LED)
//     09 4B        Usage (Generic Indicator)
//     15 00 25 01 95 08 75 01
//     91 02        OUTPUT (Data,Var,Abs)     -- 8-bit LED output report
//     05 09        Usage Page (Button)
//     15 00 25 01 75 01 95 08
//     81 01        INPUT  (Constant)         -- byte 0 is PADDING
//     19 01        Usage Minimum (1)
//     29 0E        Usage Maximum (14)
//     15 00 25 01 95 0E
//     81 02        INPUT  (Data,Var,Abs)     -- Buttons 1..14, bits 8..21
//     95 02 81 01  INPUT  (Constant)         -- 2 bits tail padding
//     05 FF 09 00 15 00 26 FF 00 75 08 95 27
//     B1 00        FEATURE (39 bytes, vendor-defined)
//   C0           End Collection
//
// So the 3-byte input report is: [padding byte][buttons 1..8][buttons 9..14 + 2 pad].
//
// ---------------------------------------------------------------------------
// How that maps onto QMK
// ---------------------------------------------------------------------------
// With JOYSTICK_AXIS_COUNT 1 / JOYSTICK_AXIS_RESOLUTION 8 / BUTTON_COUNT 14,
// report_joystick_t is { int8_t axes[1]; uint8_t buttons[2]; } PACKED -- the
// axis reproduces the device's padding byte and the buttons land on bits
// 8..21, exactly where the device puts them.
//
// Therefore: QMK joystick index n == HID Button n+1 == wire bit 8+n.
//
// Cross-checked against Nuance's PMII_MICBUTTONS table (Microphone SDK API
// Reference, p.39): its 16-bit mask is the button-NUMBER bitmap, so
// BTN_DICTATE 0x0004 is bit 2, i.e. Button 3. Agrees with the capture on all
// nine documented buttons.
//
// Note on Trigger: the capture shows it as wire 01 00 00, i.e. byte 0 -- which
// this descriptor declares Constant. It therefore does not belong to this
// interface at all; it comes from the scanner, a separate device in Nuance's
// object model (and correspondingly absent from the SDK button table). It is
// deliberately not defined below.
//
// Why this is focus-independent: the Windows focus rule applies to the
// keyboard usage page (0x07). A Button-page collection is delivered by Raw
// Input to whoever registered for that usage, foreground or not.

#pragma once

#include <stdint.h>

// QMK joystick button index. Add 1 for the HID Button number the host sees.
enum powermic_button {
    PM_TRANSCRIBE   = 0,  // Button 1  -- wire 00 01 00 -- SDK 0x0001
    PM_TAB_BACK     = 1,  // Button 2  -- wire 00 02 00 -- SDK 0x0002
    PM_DICTATE      = 2,  // Button 3  -- wire 00 04 00 -- SDK 0x0004
    PM_TAB_FORWARD  = 3,  // Button 4  -- wire 00 08 00 -- SDK 0x0008
    PM_REWIND       = 4,  // Button 5  -- wire 00 10 00 -- SDK 0x0010
    PM_FFWD         = 5,  // Button 6  -- wire 00 20 00 -- SDK 0x0020
    PM_STOP_PLAY    = 6,  // Button 7  -- wire 00 40 00 -- SDK 0x0040
    PM_CUSTOM_LEFT  = 7,  // Button 8  -- wire 00 80 00 -- SDK 0x0080
    PM_ENTER_SELECT = 8,  // Button 9  -- wire 00 00 01 -- SDK 0x0100
    PM_CUSTOM_RIGHT = 9,  // Button 10 -- wire 00 00 02
};

// Mirror a physical PowerMic button: press on key-down, release on key-up, so
// the host sees the same press/release pair a real handset produces.
void powermic_button_press(uint8_t button);
void powermic_button_release(uint8_t button);
