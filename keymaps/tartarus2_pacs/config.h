
#pragma once

#pragma weak backing_store_lock
#pragma weak backing_store_unlock

#define NO_DEBUG
#define NO_PRINT
#define FORCE_NKRO
#define TAPPING_TERM 300

#ifdef CRT0_EXTRA_CORES_NUMBER
#undef CRT0_EXTRA_CORES_NUMBER
#endif
#define CRT0_EXTRA_CORES_NUMBER 1


// PowerMic II emulation. 0 axes keeps the report buttons-only; 24 buttons
// makes it exactly 3 bytes with no padding item, matching the captured
// PowerMic report. Must stay >= 17 — Trigger is bit 16.
#define JOYSTICK_AXIS_COUNT 0
#define JOYSTICK_BUTTON_COUNT 24

#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_DELAY 0
#define MOUSEKEY_TIME_TO_MAX 60
#define MOUSEKEY_MAX_SPEED 8
#define MOUSEKEY_WHEEL_DELAY 0