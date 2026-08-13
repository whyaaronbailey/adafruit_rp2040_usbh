
#pragma once

#pragma weak backing_store_lock
#pragma weak backing_store_unlock

// Guarded: with CONSOLE_ENABLE = no, QMK already defines both, and a bare
// redefine trips -Werror.
#ifndef NO_DEBUG
#define NO_DEBUG
#endif
#ifndef NO_PRINT
#define NO_PRINT
#endif
// #define FORCE_NKRO  // disabled: NKRO-over-shared-EP dropped keycodes on Windows
#define TAPPING_TERM 300

// VIA dynamic keymaps: the virtual matrix is 32x8 = 256 positions, so each
// layer costs 512 bytes of EEPROM. Two layers (all this keymap uses) keeps the
// dynamic keymap well inside the RP2040 wear-leveling EEPROM.
#define DYNAMIC_KEYMAP_LAYER_COUNT 2

// The PACS functions live in VIA dynamic macros (M0..M22) so they are
// viewable and editable in the GUI. 24 slots: 23 seeded + 1 spare.
#define DYNAMIC_KEYMAP_MACRO_COUNT 24

// Reserve a keyboard EEPROM datablock for the VIA "Lighting" settings so they
// survive a reboot (see trgb_settings_* in keymap.c). 16 bytes leaves headroom
// past the current struct; growing this bumps EECONFIG_KB_DATA_VERSION, which
// invalidates any older block automatically.
#define EECONFIG_KB_DATA_SIZE 16

#ifdef CRT0_EXTRA_CORES_NUMBER
#undef CRT0_EXTRA_CORES_NUMBER
#endif
#define CRT0_EXTRA_CORES_NUMBER 1


// PowerMic II emulation, shaped against the real 64-byte report descriptor
// pulled from 2024-0420_powermic_only.pcapng frame 3613. See powermic.h for
// the full byte listing. Its input report is laid out as:
//
//   8 bits  INPUT (Constant)      -> byte 0 is padding, NOT buttons
//   14 bits INPUT (Data,Var,Abs)  -> Buttons 1..14 at bits 8..21
//   2 bits  INPUT (Constant)      -> tail padding
//
// One 8-bit axis reproduces byte 0, so QMK's buttons then start at bit 8 just
// like the device. That matters beyond byte layout: it makes Dictate HID
// *Button 3* - the usage the real device reports, and the bit Nuance's
// BTN_DICTATE 0x0004 names. Starting buttons at bit 0 produced identical wire
// bytes but numbered every button 8 higher, which a usage-based reader
// (HidP_GetUsages, or the PowerMic COM SDK) would read wrong.
//
// 14 % 8 != 0, so QMK appends the 2-bit constant tail on its own.
#define JOYSTICK_AXIS_COUNT 1
#define JOYSTICK_AXIS_RESOLUTION 8
#define JOYSTICK_BUTTON_COUNT 14

#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_DELAY 0
#define MOUSEKEY_TIME_TO_MAX 60
#define MOUSEKEY_MAX_SPEED 8
#define MOUSEKEY_WHEEL_DELAY 0
// ---- PowerMic emulation delta (this keymap = tartarus2_ahk + these lines) ----
// Present the joystick interface with the genuine 64-byte PowerMic II report
// descriptor. Everything else (interfaces, EEPROM, LEDs, VIA) stays exactly as
// the production ahk build, which is known to bring core 1 up reliably.
// NOTE: deliberately NO POWERMIC_FORCE_EP1 and no interface trimming - Nuance
// matches on VID/PID + usagePage 1/usage 0 only, never endpoint or interface
// number (proven against PowerScribe's own HIDManager and the dictation_support
// SDK), so the shape that keeps the converter healthy wins.
#define POWERMIC_HID_DESC

// PowerMic II identity. Self-contained here so it does not depend on the shared
// keyboard info.json (which stays 239A:0001 for the other builds). #undef first:
// the info.json-generated defines land earlier in the TU and would -Werror.
#undef VENDOR_ID
#undef PRODUCT_ID
#undef DEVICE_VER
#undef MANUFACTURER
#undef PRODUCT
#define VENDOR_ID    0x0554
#define PRODUCT_ID   0x1001
#define DEVICE_VER   0x0143
#define MANUFACTURER "Nuance"
#define PRODUCT      "PowerMicII-NS"

// Put the PowerMic HID on endpoint 0x81, as the genuine device does (its itf3
// HID is EP 0x81 IN). The build that PowerScribe actually accepted at the
// workstation had this; the earlier reason for dropping it (suspected TX
// breakage on multi-endpoint builds) was a misdiagnosis - that was the missing
// 0x00 report-ID prefix on raw-HID output reports.
#define POWERMIC_FORCE_EP1
