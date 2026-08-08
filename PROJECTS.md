# Projects built on adafruit_rp2040_usbh

**Read this first if you've been away from the code for a while.**

Everything here descends from the published repo
[`whyaaronbailey/adafruit_rp2040_usbh`](https://github.com/whyaaronbailey/adafruit_rp2040_usbh).
Over 2024–2026 five *different* projects grew out of it. Historically each one was
started by **copying the directory** rather than branching, which is why the disk
ended up with a dozen near-identical trees and no record of what diverged or why.

As of 2026-08-08 every project is committed to its own branch. The directories stay
separate (they're genuinely separate QMK keyboard definitions with their own
`KEYBOARD_PATH`), but each directory now tracks one named branch.

---

## The shared trick

This is **not** normal QMK keyboard firmware — it's a USB-to-USB converter.
A device plugs into the RP2040's USB-A host port; the board presents as a standard
keyboard/mouse to the PC over USB-C.

Because there's no real key matrix, `info.json` declares a **virtual pin matrix**:
each physical key on the attached device is mapped to the matrix position it would
occupy on a full-size ANSI layout. Tartarus key `01` → `[3,6]`, key `06` (TAB) →
`[5,3]`, and so on. That's what makes QMK keymaps work against a device QMK can't
actually scan.

Architecture:

```
Device <-> USB-A/Host (Core 1) <-> parsing/matrix <-> shared memory
       <-> Core 0 QMK/keymap <-> USB-C <-> Host PC
```

The practical consequence, quoted from `notes/LED prompt3.txt`: *"Cannot use QMK RGB
Matrix features (we're not directly controlling LEDs)."* Anything that drives
hardware on the attached device has to go out as HID reports through Core 1.

---

## P1 · Tartarus V2 Pro converter — SHIPPING

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_usbh` |
| Branch | `p1-tartarus-converter` (`280658e`) |
| Status | **Working. Daily driver.** |

The base converter. Lets a Razer Tartarus V2 Pro run QMK keymaps with no Razer
software, with settings living on the RP2040 board.

Keymaps: `default`, `tartarus2`, `tartarus2_pacs`.

```bash
qmk compile -kb converter/adafruit_rp2040_usbh -km tartarus2_pacs
```

> Every other project below reuses this directory's submodules
> (`lib/tinyusb`, `lib/Pico-PIO-USB`, `lib/pico-sdk`) **by path** rather than
> duplicating them. Do not delete or rename this directory.

---

## P2 · Tartarus LED control — STALLED, want to revive

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_usbh.old.working` |
| Branch | `p2-tartarus-leds` (`5ea440e`) |
| Status | **Never worked.** Still wanted. |

**The directory name is misleading — this is the LED project, not an old backup.**

Goal: drive the Tartarus V2 Pro's per-key RGB from the converter, so the keyboard
lights up without Razer Synapse.

Key files:
- `keymaps/tartarus2_pacs/led_control.c` / `.h` — the implementation attempt
- `keymaps/tartarus2_pacs/rules.mk` — wires it in via `SRC += led_control.c`
- `notes/LED prompt*.txt` — four design docs, the largest 30KB. **Start here.**
- `notes/razerchromacommon.c`, `razerkbd_driver.c`, `razeraccessory_driver.h` —
  OpenRazer driver sources, kept as protocol reference

This branch also bumps `lib/tinyusb` to **0.18.0**; every other project is on
0.17.0-229. That bump was part of this work.

### Two approaches were sketched, neither finished

1. **OpenRazer protocol over HID Interface 2.** The Tartarus (VID `1532`, PID
   `0244`) exposes three HID interfaces: 0 = keyboard (boot protocol), 1 =
   mouse/scroll, 2 = LED control. Send Razer's vendor command structure to
   interface 2 through Core 1's USB host stack.
2. **Direct IS31FL3731.** Talk to the LED controller chip over I²C.

**The Orbweaver connection:** `notes/LED prompt3.txt` cites *"IS31FL3731 Physical
Implementation (from Orbweaver diagram)"*. QMK's stock `keyboards/handwired/orbweaver`
has working IS31FL3731 code for a similar Razer device, and it was pulled in as the
wiring reference — VCC→R4, GND→R1, SDA→R2, SCL→R3, both with 4.7k pullups to 3.3V.
That directory is a reference donor, not a project.

An earlier, different attempt survives at
`E:\qmk_firmware - Copy\keyboards\converter\adafruit_rp2040_usbh_t2_led` — it sets
`RGBLIGHT_ENABLE = yes` and has a `tartarus.c`. Approach 1 above superseded it.

---

## P3 · VIA visual layout — STALLED

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_usbh_via` |
| Branch | `p3-via-layout` (`cf0fcc1`) |
| Status | Incomplete |

The attempt to get a **visual, remappable map of the Tartarus** — so layouts could be
edited in the VIA GUI instead of by recompiling.

Key files: `via_layout.c` / `.h`, `device_detection.c` / `.h`, `keymaps/via/`
(`VIA_ENABLE = yes`), and a `core/` directory.

Until today none of this was in git — it existed only as loose files on two disks.

**A second, parallel attempt** is preserved on branch `p3-via-layout-alt` (`fc678d5`),
from `E:\qmk-via-test\`. The core sources (`via_layout.c/.h`, `device_detection.c`)
are byte-identical to the main attempt; `rules.mk` and `info.json` differ. Worth
diffing before picking one to continue from:

```bash
git diff p3-via-layout p3-via-layout-alt -- rules.mk info.json
```

---

## P4 · Kensington Orbit dual-trackball for PACS — NEWEST, ACTIVE

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_orbit` |
| Branch | `p4-orbit-trackball` (`f021258`) |
| Status | Partly working, in progress |

**The vision:** a dual-trackball interface to PACS where one trackball is *always*
scroll and the other is *always* cursor movement, plus custom radial menus driven by
the scroll ring — using the Kensington Orbit Wireless and Kensington's four-button
scroll-ring trackball.

This is attempt #1 at that. It's the only tree that documented itself as it went —
see [`changelog.md`](../adafruit_rp2040_orbit/changelog.md) for the full delta.

`keymaps/kensington_orbit_wireless/` (VID `047d` PID `80a6`) overrides the weak
`mouse_report_hook()` in `matrix.c`:

| Input | Behaviour |
|---|---|
| Default at boot | Middle button held down → Windows native scroll mode |
| Left button | Toggles scroll ↔ cursor mode |
| Trackball X/Y | Passed through; scroll or cursor per middle-button state |
| Scroll ring | Vertical wheel ticks, independent of middle button |
| Right button | `Ctrl+F21`, routed by AHK to PowerScribe F4 |

Rather than converting scroll in firmware, it leans on Windows' own middle-button
scroll mode — simpler, and avoided crashes in the accumulator math.

Two real bug fixes live here and are worth porting back to P1:
- **Boot-protocol devices produced zero output.** TinyUSB leaves boot-class devices in
  boot protocol, whose 3-byte `[buttons, X, Y]` report has no Report ID — but
  `parse_report()` treats `report[0]` as one, so no collection ever matched. Fixed by
  calling `tuh_hid_set_protocol(..., HID_PROTOCOL_REPORT)` in `tuh_hid_mount_cb`.
  No-op for non-boot devices.
- **Composite descriptors were being truncated.** `CFG_TUH_ENUMERATION_BUFSIZE`
  256 → 512, and `HID_REPORT_MEMBER_COUNT` 32 → 64, because the Orbit is a composite
  device with two HID interfaces that overflowed both.

---

## P5 · PowerMic II emulation — STALLED

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_usbh_pm` |
| Branch | `p5-powermic` (`e027579`) |
| Status | Incomplete |

Emulating the Nuance PowerMic II dictation handset's buttons — the other half of the
PACS workflow alongside P4.

Key files: `keymaps/powermic/` behind a `POWERMIC_ENABLE` flag, a `core0/` / `core1/`
split, and `.notes/` containing the PowerMic II technical specification, a custom
`usb_descriptors.c`, and `powermic.c` / `.h`.

---

## Not projects

| Path | What it is |
|---|---|
| `keyboards/handwired/orbweaver` | Stock QMK. IS31FL3731 reference for P2. Keep. |
| `keyboards/converter/qmk_AdaFruitRp2040USBH_gongyi` | Upstream GongYiLiao original (keymaps `ansi`, `ka500`, `pok3r`). Ancestor, no git. |
| `E:\qmk_firmware - Copy\` | Archive of ~30 dead-end experiments (`_claude`…`_claude6`, `_perplexity2/3`, `_chatgpt`, `_google`, `_tinyusb`, `_raghur`, `_OG`). Contains `_t2_led`, relevant to P2. |
| `E:\qmk-archive-2026-08-08\` | Verified duplicates, moved out 2026-08-08: the old `E:\temp adafruit` snapshot, `orbweaver.zip`, a stray nested `matrix.c`, and a two-file P3 fragment from `C:\keyboards`. See the README in there. Delete when satisfied. |
| `E:\qmk-personal-notes-removed\` | Unraid/NordVPN notes that had been sitting in P2's `notes/` folder. **Live credentials — rotate, then delete.** |

---

## Rules going forward

1. **New idea → new branch, not a new directory.** That's the one habit that caused all of this.
2. Never commit `github_persona_access_token.txt`. It's in `.git/info/exclude`; move it out of the repo tree and rotate it.
3. `.claude/` is gitignored — it holds machine-local paths.
4. The `-dirty` marker on `lib/tinyusb` is a harmless Windows symlink typechange in two docs files. Don't commit it.
