# tartarus_rgb - Razer Tartarus RGB control from the converter

Controls the backlight of a Razer Tartarus V2/Pro attached to the converter's
USB host port. The wire protocol comes from
[OpenRazer](https://github.com/openrazer/openrazer) (GPL-2.0), mainly
razerchromacommon.c and razercommon.c. This is a minimal reimplementation for
the embedded host case and is GPL-2.0-or-later like the rest of the tree.

## Things that cost me a lot of time

Razer commands are 90-byte HID FEATURE reports (bmRequestType 0x21, bRequest
0x09, wValue 0x0300) with an XOR checksum over bytes 2..87. Getting them to the
device from TinyUSB has three traps:

1. `tuh_hid_set_report()` does not work. It targets a HID instance, the device
   stalls every report, and the completion callback reports len=0. You have to
   send a raw control transfer to a specific interface number (wIndex), same as
   OpenRazer does. See `tartarus_send_iface()` in matrix.c. On this Tartarus
   the control interface is interface 2; the firmware finds it as the
   highest-numbered HID interface with protocol NONE. Interface numbers are
   stable across re-enumerations, TinyUSB instance indices are not.

2. The device needs about 15 ms to process each command after the transfer
   completes. Send the next one sooner and the previous one is silently
   dropped even though the wire ACKed it. `rz_send()` paces this, and also
   retries while the single TinyUSB control pipe is busy.

3. Do not put the device in Razer driver mode. Custom frames do NOT need it
   (they render fine in normal mode), and in driver mode the Tartarus stops
   sending normal key reports: every key goes dead while the LEDs keep
   working, and the mode sticks until the device loses power. The keymaps
   send device-mode NORMAL whenever the device appears, which also recovers a
   unit left stuck by other software.

## LED matrix

The Tartarus exposes its LEDs as one logical row (row 0) with columns mapping
1:1 onto the 20 physical keys: col 0 = key 01 ... col 19 = key 20. Rows 1+
address nothing.

## API

```c
bool tartarus_rgb_ready(void);          // device mounted, interface known

// Effects stored and run on the device:
void tartarus_rgb_static(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_brightness(uint8_t value);
void tartarus_rgb_effect_none(void);
void tartarus_rgb_effect_spectrum(void);
void tartarus_rgb_effect_breathing(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_effect_wave(uint8_t direction);
// plus breathing_dual/breathing_random, starlight (single/dual/random),
// reactive, wheel - see tartarus_rgb.h

// Per-key framebuffer:
void tartarus_rgb_set_all(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_set_key(uint8_t key, uint8_t r, uint8_t g, uint8_t b); // 0..19
void tartarus_rgb_flush(void);          // push buffer and display it
```

## Usage

Add to rules.mk:

```make
SRC += tartarus_rgb.c
```

The tartarus2_ahk and tartarus2_qmk keymaps are the reference consumers:
per-key reactive baseline, all keys red while dictating, and a pulse on the
active scroll key, driven from `housekeeping_task_user()`.
