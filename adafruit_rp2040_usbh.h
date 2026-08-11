#pragma once

#include <stdint.h>
#include <stdbool.h>

// Send a HID FEATURE report to the attached keyboard on the USB-host side.
// Used to push OpenRazer vendor commands (e.g. LED control) to a Razer keyboard.
// Returns: 0 = no keyboard mounted, 1 = mounted device is not Razer (VID != 0x1532),
// 2 = matched but the control pipe was busy (retry), 3 = queued successfully.
// Defined in matrix.c, which owns the host-side device handle. Same proven path
// as the keyboard-lock-LED reports.
int tartarus_send_feature_report(const uint8_t *buf, uint16_t len);

// Send a FEATURE report to a SPECIFIC HID instance of the attached keyboard,
// so we can probe which interface actually drives the LEDs. Returns the same
// codes as tartarus_send_feature_report (0 none, 1 not Razer / bad instance,
// 2 busy, 3 queued). tartarus_instance_count() reports how many HID instances
// the mounted device exposes (0 if none mounted).
int     tartarus_send_to(uint8_t instance, const uint8_t *buf, uint16_t len);
uint8_t tartarus_instance_count(void);

// Send a Razer SET_REPORT to an explicit USB interface number (wIndex),
// replicating OpenRazer exactly. The Tartarus V2/Pro control interface is
// interface 0x01. Returns the same codes as tartarus_send_to.
int     tartarus_send_iface(uint8_t itf_num, const uint8_t *buf, uint16_t len);

// HID instance of the Razer control interface, detected dynamically at mount
// (the instance index is not stable across re-enumerations). Target LED/Razer
// vendor commands here.
uint8_t tartarus_led_instance(void);

// USB interface number of the Razer control interface (the one to pass to
// tartarus_send_iface). Auto-detected as the largest-descriptor HID instance's
// bInterfaceNumber. 0xFF if no Razer device is mounted.
uint8_t tartarus_led_itfnum(void);

// bInterfaceNumber of a specific HID instance (0xFF if unavailable).
uint8_t tartarus_inst_itfnum(uint8_t instance);

// Diagnostics from the most recent tartarus_send_feature_report() call, so the
// keymap can surface them (we have no console on this build). dbg_pid is the
// actual PID read from the attached keyboard (-1 if none); dbg_stage is a
// bitmask: 1 = keyboard present, 2 = VID is Razer, 4 = PID is 0x02xx,
// 8 = set_report queued.
extern volatile int16_t  tartarus_dbg_pid;
extern volatile uint8_t  tartarus_dbg_stage;
extern volatile uint8_t  tartarus_dbg_instance; // HID instance the report is sent to
extern volatile uint8_t  tartarus_dbg_icount;   // number of HID instances on the device
extern volatile uint8_t  razer_cb_fired;        // FEATURE set_report completion fired
extern volatile uint16_t razer_cb_len;          // bytes the completed transfer reported
extern volatile uint16_t razer_cb_count;        // completed vendor SET_REPORTs since boot

// Per-instance report-descriptor length and interface protocol, captured at
// mount. Lets the host map which HID instance is keyboard/mouse/control by
// descriptor size (control interface has the largest, ~186 bytes).
extern volatile uint16_t razer_inst_desclen[8];
extern volatile uint8_t  razer_inst_proto[8];

// Input-path diagnostics (matrix.c): total HID reports received from the
// attached device, reports from the keyboard interface, and the first bytes
// of the most recent keyboard report.
extern volatile uint16_t diag_rx_all;
extern volatile uint16_t diag_rx_kbd;
extern volatile uint8_t  diag_last_kbd[4];