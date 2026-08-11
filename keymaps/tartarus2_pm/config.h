#pragma once
#define TAPPING_TERM 300

#ifdef CRT0_EXTRA_CORES_NUMBER
#undef CRT0_EXTRA_CORES_NUMBER
#endif
#define CRT0_EXTRA_CORES_NUMBER 1

// PowerMic II button HID: 1 axis (byte0 pad) + 14 buttons = exact 3-byte report.
#define JOYSTICK_AXIS_COUNT 1
#define JOYSTICK_AXIS_RESOLUTION 8
#define JOYSTICK_BUTTON_COUNT 14
#define POWERMIC_HID_DESC
