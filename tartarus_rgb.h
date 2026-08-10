// Copyright 2026 whyaaronbailey
// SPDX-License-Identifier: GPL-2.0-or-later
//
// tartarus_rgb — a small, generalizable RGB driver for the Razer Tartarus
// V2/Pro (and other OpenRazer "extended matrix" keyboards) attached to an
// RP2040 USB-host converter running QMK + TinyUSB.
//
// The command wire-format and effect semantics are derived from the OpenRazer
// project (https://github.com/openrazer/openrazer, GPL-2.0), specifically
// razerchromacommon.c / razerkbd_driver.c. This driver reimplements a minimal
// subset for the embedded USB-host use case; all protocol credit is OpenRazer's.
//
// API shape intentionally mirrors QMK's RGB Matrix so it is familiar:
//   - tartarus_rgb_set_all() / tartarus_rgb_set_key() build a framebuffer,
//     tartarus_rgb_flush() commits it (per-key control).
//   - tartarus_rgb_static() / _brightness() / _effect_*() drive the device's
//     own hardware effects (no per-frame host traffic).
//
// Transport: everything goes out as an OpenRazer 90-byte FEATURE SET_REPORT to
// the device's control interface via tartarus_send_iface() (matrix.c), which
// performs a raw USB control transfer to the correct wIndex. Using the HID
// instance API (tuh_hid_set_report) does NOT work — the device stalls it.

#pragma once

#include <stdint.h>
#include <stdbool.h>

// The Tartarus exposes its per-key LEDs as a single logical matrix row whose
// columns map 1:1 to the 20 physical keys (col 0 == key 1 .. col 19 == thumb).
#define TARTARUS_RGB_KEYS 20

// True once a Razer device is mounted and its control interface is known.
bool tartarus_rgb_ready(void);

// Put the device into driver mode (required before custom-frame effects take
// effect). Called lazily by the driver; exposed for explicit control.
void tartarus_rgb_driver_mode(bool on);

// -------- Hardware effects (stored/executed on the device itself) -----------

// Whole-device solid colour (device "static" effect).
void tartarus_rgb_static(uint8_t r, uint8_t g, uint8_t b);

// Global brightness, 0..255.
void tartarus_rgb_brightness(uint8_t value);

// Clear to the device's "none" effect (LEDs off, no effect running).
void tartarus_rgb_effect_none(void);

// Built-in animated effects run by the device firmware — the full set of
// Synapse "lighting effects" the hardware supports (per OpenRazer):
//
//   Spectrum   – continuous colour cycle across the rainbow
//   Wave       – rainbow wave scrolling across the keys (direction 0/1)
//   Breathing  – fade in/out: one colour, two alternating colours, or random
//   Starlight  – random keys twinkle then fade (speed 1..3): colour/dual/random
//   Reactive   – a pressed key lights up and fades (speed 1..4)
//   Wheel      – colour wheel rotating across the pad (direction 1/2)
void tartarus_rgb_effect_spectrum(void);
void tartarus_rgb_effect_wave(uint8_t direction);                  // 0 or 1
void tartarus_rgb_effect_breathing(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_effect_breathing_dual(uint8_t r1, uint8_t g1, uint8_t b1,
                                        uint8_t r2, uint8_t g2, uint8_t b2);
void tartarus_rgb_effect_breathing_random(void);
void tartarus_rgb_effect_starlight(uint8_t speed, uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_effect_starlight_dual(uint8_t speed,
                                        uint8_t r1, uint8_t g1, uint8_t b1,
                                        uint8_t r2, uint8_t g2, uint8_t b2);
void tartarus_rgb_effect_starlight_random(uint8_t speed);          // speed 1..3
void tartarus_rgb_effect_reactive(uint8_t speed, uint8_t r, uint8_t g, uint8_t b); // speed 1..4
void tartarus_rgb_effect_wheel(uint8_t direction);                 // 1 or 2

// -------- Per-key framebuffer (QMK RGB-Matrix style) ------------------------

// Fill the whole framebuffer with one colour (does not send until flush()).
void tartarus_rgb_set_all(uint8_t r, uint8_t g, uint8_t b);

// Set a single key (0..TARTARUS_RGB_KEYS-1) in the framebuffer.
void tartarus_rgb_set_key(uint8_t key, uint8_t r, uint8_t g, uint8_t b);

// Push the framebuffer to the device and display it (device "custom" effect).
void tartarus_rgb_flush(void);
