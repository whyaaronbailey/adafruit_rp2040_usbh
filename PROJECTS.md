# Projects built on adafruit_rp2040_usbh

**Read this first if you've been away from the code for a while.**

Everything here descends from the published repo
[`whyaaronbailey/adafruit_rp2040_usbh`](https://github.com/whyaaronbailey/adafruit_rp2040_usbh).
Over 2024–2026 six *different* projects grew out of it (P1–P6). Historically each one
was started by **copying the directory** rather than branching, which is why the disk
ended up with a dozen near-identical trees and no record of what diverged or why.

As of 2026-08-08 every project is committed to its own branch. The directories stay
separate (they're genuinely separate QMK keyboard definitions with their own
`KEYBOARD_PATH`), but each directory now tracks one named branch.

**Quick index:** P1 Tartarus converter (shipping) · P2 Tartarus LEDs (stalled) ·
P3 VIA visual map (stalled) · P4 Kensington dual-trackball (active) ·
P5 PowerMic emulation (dead end) · **P6 PowerMic over QMK's HID stack (works, unflashed)**.
P6 supersedes P5 and is the one to build on for the dictation-trigger goal.

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

## P5 · PowerMic II emulation (old attempt) — SUPERSEDED BY P6

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_usbh_pm` |
| Branch | `p5-powermic` (`e027579`) |
| Status | Dead end. Kept for its notes; the working approach is P6. |

The first attempt at emulating the Nuance PowerMic II dictation handset's buttons —
the other half of the PACS workflow alongside P4.

Key files: `keymaps/powermic/` behind a `POWERMIC_ENABLE` flag, a `core0/` / `core1/`
split, and `.notes/` containing the PowerMic II technical specification, a custom
`usb_descriptors.c`, and `powermic.c` / `.h`.

**Why it could never work (diagnosed 2026-08-08).** It called TinyUSB's *device*
API (`tud_hid_n_report`) and enabled `CFG_TUD_ENABLED` + compiled in `dcd_rp2040.c`.
But this firmware runs TinyUSB in **host mode only** — the device side (what the PC
sees) is **ChibiOS USB** via QMK. That put two USB device stacks on one RP2040
peripheral. On top of that, `POWERMIC_INTERFACE` was 3 while `CFG_TUD_HID` was 1
(only instance 0 exists), the report struct was 2 bytes not 3, and `powermic.c` was
never even added to `rules.mk`'s `SRC`. Four independent blockers. This is the
"we never got QMK to emit that code" from the notes.

Its `.notes/` are still the best written record of the goal and the captured button
codes — read them, but build on P6.

---

## P6 · PowerMic II over QMK's own HID stack — WORKS (unflashed)

| | |
|---|---|
| Directory | `keyboards/converter/adafruit_rp2040_usbh` (keymap `tartarus2_pacs`) |
| Branch | `p6-powermic-hid` (from P1) |
| Status | Builds clean, descriptor verified byte-exact. **Not yet flashed.** |

The approach that actually works: instead of a second USB stack, carry the PowerMic's
3-byte Button-page report over **QMK's joystick HID interface** as a transport. QMK on
ChibiOS already enumerates it alongside the keyboard, so the Tartarus keeps working and
the PowerMic report is just one more interface.

**The daily-driver keyboard is untouched** — this branch only replaces what the
`DICTATE` key emits. It used to send `SS_TAP(X_F13)` (a keyboard scan code, hence
focus-gated, hence the AHK dependency); it now emits the PowerMic's own button report,
which Raw Input delivers off-focus.

Files added to `keymaps/tartarus2_pacs/`:
- `powermic.c` / `powermic.h` — the button enum and press/release, with the full
  captured descriptor documented in the header
- edits to `config.h` (`JOYSTICK_AXIS_COUNT 1`, `JOYSTICK_AXIS_RESOLUTION 8`,
  `JOYSTICK_BUTTON_COUNT 14`), `rules.mk` (`JOYSTICK_ENABLE`, `JOYSTICK_OWN_EP`,
  `MOUSE_SHARED_EP = no`), and `keymap.c` (the `DICTATE` case)

Depends on a one-line core opt-out: **`JOYSTICK_OWN_EP`** in
`tmk_core/protocol.mk` (committed on `qmk-0.24.0` as `e15dbc1002`). Without it, QMK
folds the joystick into the shared endpoint behind a REPORT_ID and the raw bytes never
reach the wire. `MOUSE_SHARED_EP = no` is also required — otherwise `SharedReport[]`
opens at the mouse block and a standalone `JoystickReport[]` won't compile.

**Report layout, matched to the real device:** byte 0 is a constant padding byte (an
8-bit axis reproduces it), Buttons 1–14 sit at bits 8–21, then 2 bits of tail padding.
So Dictate is HID **Button 3** on the wire *and* by usage — the same as the real
handset. Verified against the linked ELF.

**Every button verified** against real per-button USBPcap captures (see References):
all 10 enum codes match byte-for-byte, and `rear` = `01 00 00` confirms Trigger lives
in byte 0 (the scanner — correctly excluded from the button enum).

**Open question — only hardware answers it:** does PowerScribe react? The descriptor
still differs from a real PowerMic in top-level Usage (`0x04` Joystick vs `0x00`
Undefined), an extra Physical collection, an 8-bit LED OUTPUT report, and a 39-byte
vendor FEATURE report. The **feature-handshake risk is retired** — bus captures show
the host never reads that feature report (only `SET_IDLE`). The remaining unknown is
whether PowerScribe pins to the PowerMic VID/PID. To test: open PowerScribe, move focus
to another window, press Tartarus key 20.

```bash
qmk compile -kb converter/adafruit_rp2040_usbh -km tartarus2_pacs
# revert to the plain daily driver at any time:
git -C keyboards/converter/adafruit_rp2040_usbh checkout p1-tartarus-converter
```

---

## Reference assets (PowerMic / PowerScribe) — scattered across drives

These are the source-of-truth files for the P6 work. They live on removable/portable
drives, so **drive letters may differ when a drive is remounted** — the Envoy Pro was
`J:` and the DUOLINK32 flash drive was `G:` on 2026-08-08.

| Asset | Location (2026-08-08) | What it is |
|---|---|---|
| **Real per-button captures** | `J:\Projects\HID Remapper\hidremapper\*.pcapng` + `*.txt` | USBPcap traces of the genuine PowerMic (VID `0x0554` PID `0x1001`). `2024-0420_powermic_only.pcapng` frame 3613 = the 64-byte report descriptor. Per-button files (`Button Transcribe`, `record`, `ffwd`, `custom_left.txt`, `enter_select.txt`, …) each capture one button. This is what P6's mapping was verified against. **The most valuable asset — do not lose this folder.** |
| Full session capture | `D:\full_powerscribe_powermic.txt` | 33 MB Wireshark text export, PowerMic + PowerScribe. Tree-only (no raw descriptor bytes), so the `.pcapng` above is better, but this has the button-timing analysis. Also `D:\Downloads\1PowerMic_Capture_Analysis.txt`. |
| Nuance SDK reference | `J:\Projects\HID Remapper\Nuance PowerMic API_unlocked.pdf` (copy also in `H:\Projects\Powerscribe Documentation\`) | The `PMII_MICBUTTONS` table (p.39). Its 16-bit mask is the button-*number* bitmap — `BTN_DICTATE 0x0004` = Button 3. Confirms the capture. |
| Wireshark/tshark | `J:\PortableApps\WiresharkPortable64\App\Wireshark\tshark.exe` | On the same drive as the captures — use it to re-parse the `.pcapng`s. |
| API Monitor | `J:\Downloads\api-monitor-v2r13-x86-x64` | The tool to watch how `Nuance.PSOne.exe` binds its HID reader, if the VID/PID-pinning question ever needs a definitive answer. |
| pm_probe rig | `H:\Projects\Shared with Claude\pm_probe\` | A throwaway device-mode Arduino HID probe (a VM session's work) to validate the report hypothesis on a spare board. Superseded by P6 but the experiment protocol in its README is sound. Its transcript cites a `C:\Projects\qmk_firmware\...\converter\converter\...` path that does not exist here — that's an isolated VM copy, not this tree. |
| SpeechMike capture | `J:\Projects\HID Remapper\hidremapper\SpeechMike-device30.pcapng` | Device 30 (VID `0xdf04`) enumerates as a **plain boot mouse** — Philips routes its buttons through the SpeechControl driver, not readable HID. Not a useful PowerMic comparison. (This capture also contains a second real PowerMic at device 29.) |

**radhub** (`H:\Projects\radhub*`, and on `G:`) is the *other* half of the PACS story:
a DLL-injection interface into PowerScribe, developed in a separate Claude session. It
is a **discovery/runtime tool, not a dependency of this firmware.** The `KSHAH`/`KSJAJ`
`*.pcapng.gz` files next to pm_probe are radhub **network** captures, not USB.

## Earlier, pre-QMK history

Before any of the QMK work, PowerMic emulation was attempted in AutoHotkey:
`C:\Users\Kamran\OneDrive\Desktop\USB\` (Dec 2023) — `powermic_control.ahk`/`.exe` in
~15 copies, plus `Nuance PowerScribe360 Integration Component (x64)` with
`Nuance.SectraCOM.tlb`. This is the ancestor of the whole PACS-input effort; kept for
history, not active.

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
5. The P6 core change lives in a **different repo** than the keyboard: `tmk_core/protocol.mk`
   on branch `qmk-0.24.0` (commit `e15dbc1002`). If that outer tree is ever reset or
   re-cloned, P6 fails to build with a confusing `SharedReport` error — re-apply `JOYSTICK_OWN_EP`.
6. The PowerMic captures and `tshark` are on the **Envoy Pro** portable drive (`J:` when
   mounted). If it's not plugged in, the References table above still records what's where.
