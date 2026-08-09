#pragma once

#include <stdint.h>
#include <stdbool.h>

// Send a HID FEATURE report to the attached keyboard on the USB-host side.
// Used to push OpenRazer vendor commands (e.g. LED control) to a Razer Tartarus.
// Returns false if no keyboard is mounted, the mounted device is not the
// Tartarus V2 (VID 0x1532 / PID 0x0244), or the control pipe is momentarily
// busy (caller should retry). Defined in matrix.c, which owns the host-side
// device handle. Same proven path as the keyboard-lock-LED reports.
bool tartarus_send_feature_report(const uint8_t *buf, uint16_t len);