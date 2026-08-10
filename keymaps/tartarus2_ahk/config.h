
#pragma once

#pragma weak backing_store_lock
#pragma weak backing_store_unlock

#define NO_DEBUG
#define NO_PRINT
#define FORCE_NKRO
#define TAPPING_TERM 300

// VIA dynamic keymaps: the virtual matrix is 32x8 = 256 positions, so each
// layer costs 512 bytes of EEPROM. Two layers (all this keymap uses) keeps the
// dynamic keymap well inside the RP2040 wear-leveling EEPROM.
#define DYNAMIC_KEYMAP_LAYER_COUNT 2

// The PACS functions live in VIA dynamic macros (M0..M22) so they are
// viewable and editable in the GUI. 24 slots: 23 seeded + 1 spare.
#define DYNAMIC_KEYMAP_MACRO_COUNT 24

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