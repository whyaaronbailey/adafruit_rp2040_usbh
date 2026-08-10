# tartarus_rgb — Razer Tartarus RGB driver for QMK USB-host converters

A small, self-contained driver that controls the per-key RGB LEDs of a **Razer
Tartarus V2 / Pro** (and, in principle, other OpenRazer "extended-matrix" Razer
keyboards) that is attached to a **QMK + TinyUSB USB-host converter** — here the
Adafruit RP2040 USB Host running `keyboards/converter/adafruit_rp2040_usbh`.

It exposes a small QMK-RGB-Matrix-style API so a keymap can drive whole-device
colours, the device's own hardware effects, or a per-key framebuffer, without
knowing anything about the Razer wire protocol.

## Why this is not trivial (the key discovery)

Razer LED control uses OpenRazer's vendor protocol: a 90-byte **HID FEATURE
SET_REPORT** (`bmRequestType 0x21`, `bRequest 0x09`, `wValue 0x0300`) carrying a
command whose CRC is the XOR of bytes 2..87.

The non-obvious parts that make it actually work through a TinyUSB **host**:

1. **You must send a raw control transfer to a specific USB _interface number_
   (`wIndex`), not via `tuh_hid_set_report(dev, instance, …)`.** The HID-instance
   API targets the wrong interface and the device **STALLs every report**
   (TinyUSB's completion callback then reports `len == 0`). See
   `tartarus_send_iface()` in `matrix.c`, which mirrors OpenRazer's
   `razer_send_control_msg` exactly.
2. **The control interface is not the HID instance with the biggest descriptor.**
   On the Tartarus the interfaces are: `itf0` = boot keyboard, `itf1` = 186-byte
   HID, `itf2` = 94-byte HID — and **LED control lives on `itf2`**. The driver
   auto-detects it as *the highest-numbered HID interface whose interface
   protocol is NONE* (`tartarus_led_itfnum()`). USB interface numbers are
   device-fixed, so this is stable across re-enumerations (unlike TinyUSB HID
   instance indices, which shift on every reflash).
3. **Transaction id `0x1F`** for the Tartarus V2/Pro (byte 1 of every report).

## LED matrix layout

The Tartarus exposes its 20 keys as a **single logical matrix row** (row 0);
column index maps 1:1 to the physical keys (`col 0` = key 1 … `col 19` = thumb).
Writing to matrix rows 1+ addresses no physical LED.

## API (`tartarus_rgb.h`)

```c
bool tartarus_rgb_ready(void);                         // device mounted & interface known

// Hardware effects (stored/run on the device — no per-frame host traffic):
void tartarus_rgb_static(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_brightness(uint8_t value);
void tartarus_rgb_effect_none(void);
void tartarus_rgb_effect_spectrum(void);
void tartarus_rgb_effect_breathing(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_effect_wave(uint8_t direction);

// Per-key framebuffer (QMK RGB-Matrix style):
void tartarus_rgb_set_all(uint8_t r, uint8_t g, uint8_t b);
void tartarus_rgb_set_key(uint8_t key, uint8_t r, uint8_t g, uint8_t b); // key 0..19
void tartarus_rgb_flush(void);                         // push buffer + display
```

### Static vs. per-key, and a converter caveat

`tartarus_rgb_static` / `_brightness` / `_effect_*` are **stored/native** effects:
they need no handshake and never interfere with the device's own key reports.

`tartarus_rgb_flush` (per-key custom frames) first puts the device into **driver
mode**, which on Razer hardware can **suppress the device's normal HID key
reports**. On a *converter* the host reads those key reports, so enabling driver
mode there can stop your keys from working. Prefer the static/native effects for
status indication on a converter; use the per-key framebuffer only if you don't
depend on the device's own keys (e.g. a direct-to-PC setup).

## Usage

`rules.mk`:

```make
SRC += tartarus_rgb.c
```

Example consumer (see `keymaps/tartarus2_pacs/keymap.c`): idle = light blue,
dictating = red, continuous-scroll active = breathing blue. The state is applied
edge-triggered from `housekeeping_task_user()`:

```c
void housekeeping_task_user(void) {
    if (!tartarus_rgb_ready()) return;
    if      (scrolling)  tartarus_rgb_effect_breathing(0x00, 0x50, 0xFF);
    else if (dictating)  tartarus_rgb_static(0xFF, 0x00, 0x00);
    else                 tartarus_rgb_static(0x00, 0x40, 0xFF);
}
```

## Attribution / licence

The Razer command wire-format and effect semantics are derived from
[**OpenRazer**](https://github.com/openrazer/openrazer) (`razerchromacommon.c`,
`razercommon.c`, `razerkbd_driver.c`), licensed **GPL-2.0**. This driver is a
minimal re-implementation for the embedded USB-host case and is distributed under
the same **GPL-2.0-or-later** licence. All protocol credit is OpenRazer's.
