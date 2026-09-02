# Context for Claude — customising the SpeechKey (SK) Tartarus firmware

**How to use this file:** open a Claude session in the folder containing this file
and say something like *"Read CLAUDE_PROMPT.md and help me change key 3 to open my
worklist."* Everything Claude needs is below.

---

## 1. What this device is

A **Razer Tartarus V2/Pro** connected to an **Adafruit RP2040 USB Host Feather**
running QMK. The PC never sees the Tartarus — it sees the adapter, which presents
three USB devices:

| Interface | What the PC sees |
|---|---|
| MI_00 | a keyboard |
| MI_01 | a mouse |
| MI_04 | **a Nuance PowerMic II** (VID `0x0554`, PID `0x1001`, "PowerMicII-NS") |

That third one is the whole point: PowerScribe binds to it as if a real PowerMic
handset were plugged in, so dictation is triggered **in firmware**, with **no host
software** and **regardless of window focus**.

The firmware also drives the Tartarus's per-key RGB over the adapter's USB *host*
side (core 1 of the RP2040).

Build name: `tartarus2_sk`, keyboard `converter/adafruit_rp2040_usbh`.

---

## 2. 🚨 Rules you must not break

These are load-bearing. Breaking any one of them silently kills dictation.

1. **Exactly ONE generic-HID collection may exist.** PowerScribe's binder treats
   *every* HID collection under `0554:1001` as a PowerMic candidate and binds the
   first one it meets, in an order that varies per machine. So in `rules.mk`,
   `RAW_ENABLE`, `VIA_ENABLE`, `CONSOLE_ENABLE`, `EXTRAKEY_ENABLE` **must stay
   `no`**. (Keyboard and mouse are class-exclusive and are never offered as
   candidates, so they're fine.) This is why standard VIA cannot be enabled here.
2. **Don't change the joystick report shape.** `JOYSTICK_AXIS_COUNT 1`,
   `JOYSTICK_AXIS_RESOLUTION 8`, `JOYSTICK_BUTTON_COUNT 14` make QMK's joystick
   report byte-identical to the PowerMic's 3-byte input report, and put Dictate on
   HID Button 3. There are `_Static_assert`s guarding this.
3. **Don't change the USB identity** (`VENDOR_ID 0x0554`, `PRODUCT_ID 0x1001`,
   `PRODUCT "PowerMicII-NS"`) — that's what PowerScribe matches on.
4. **If you edit `keymaps[]` in the source, bump `TRGB_CFG_VERSION`** (currently
   `10`; this build owns 10+). The stored keymap in EEPROM is only re-seeded when
   that number changes. Forget this and your edit appears to do nothing.
5. **Never write flash during early boot.** `dynamic_keymap_reset()` and EEPROM
   saves cut XIP under core 1 and boot-loop the board. Seeding happens in
   `housekeeping_task_user()` after `c1_ready`. Leave that structure alone.

---

## 3. The two ways to change things

| Change | Needs a rebuild? | How |
|---|---|---|
| Which key sends which keycode | **No** | Config channel (§4) or `tools/usevia.app.html` |
| Colours / brightness | **No** | Config channel (§4) |
| Lighting effect | **No** | Press thumb+18 / thumb+19 on the device, then SAVE |
| What a *macro* types (§6) | **Yes** | Edit source, rebuild, reflash |
| Adding new behaviour | **Yes** | Edit source, rebuild, reflash |

**Prefer the no-rebuild path.** It's instant, needs no toolchain, and can't brick
anything.

---

## 4. The config channel (no rebuild needed)

The PowerMic descriptor declares a 39-byte vendor **Feature report**. That is the
control channel: `SET_REPORT(Feature)` sends a request, the next
`GET_REPORT(Feature)` returns the response. On the wire reports are 39 bytes;
Windows/WebHID prepend a `0x00` report ID, so buffers are **40 bytes**.

Open the collection with **usagePage `0x01`, usage `0x00`** (VID `0x0554`, PID
`0x1001`).

| Cmd | Request | Response |
|---|---|---|
| `0x01` PROTO | `[1]` | `[1, ver, layers, rows, cols]` |
| `0x02` GET_KEY | `[2, layer, row, col]` | `[2, ok, kc_hi, kc_lo]` |
| `0x03` SET_KEY | `[3, layer, row, col, hi, lo]` | `[3, ok]` |
| `0x04` GET_COLOR | `[4, idx]` | `[4, ok, hue, sat, brightness]` |
| `0x05` SET_COLOR | `[5, idx, hue, sat]` | `[5, ok]` |
| `0x06` SET_BRIGHT | `[6, value]` | `[6, ok]` |
| `0x07` SAVE | `[7]` | `[7, ok]` |
| `0x08` RESET_KEYS | `[8]` | `[8, ok]` — restore compiled defaults |
| `0x09` DICT_SYNC | `[9, on]` | `[9, ok]` — force the dictate colour |

Colour `idx`: **0** = idle baseline, **1** = dictating, **2** = held-key / scroll
pulse. Hue and saturation are 0–255 (QMK HSV, *not* degrees).

> **SET_KEY, SET_COLOR and SET_BRIGHT take effect immediately but are lost on
> unplug until you send SAVE (`0x07`).** Always finish with SAVE.

### Ready-made helper

`tools/pmcfg.ps1` (PowerShell, pure Win32, no dependencies):

```powershell
. .\tools\pmcfg.ps1

PM-Proto                          # sanity check: Proto 1, Layers 2, Rows 36, Cols 8
PM-GetKey 0 5 4                   # read layer 0, row 5, col 4  -> "7E00"
PM-SetKey 0 4 0 0x7E00            # put DICTATE on key 03
PM-Save                           # persist
PM-ResetKeys                      # back to compiled defaults (then PM-Save)
PM-DictSync $true                 # force the dictating colour on
```

Bootloader from software (no button press): send the 8-byte magic
`B0 07 10 AD B0 07 10 AD` as a feature report — the board reboots into `RPI-RP2`.
This runs in USB interrupt context, so it works even if the firmware has hung.

---

## 5. Key positions and keycodes

### Physical key → matrix (row, col)

Use these with GET_KEY/SET_KEY. **Layer 0** is the base layer, **layer 1** is
thumb-held.

| Key | row,col | Key | row,col |
|---|---|---|---|
| 01 | 3,6 | 11 | 7,1 |
| 02 | 3,7 | 12 | 0,4 |
| 03 | 4,0 | 13 | 2,6 |
| 04 | 4,1 | 14 | 0,7 |
| 05 | 4,2 | 15 | 1,1 |
| 06 | 5,3 | 16 | 28,1 |
| 07 | 2,4 | 17 | 3,5 |
| 08 | 3,2 | 18 | 3,3 |
| 09 | 1,0 | 19 | 0,6 |
| 10 | 2,5 | **20** | **5,4** |
| wheel up (SW) | 13,1 | wheel down (SC) | 13,0 |
| **thumb** | **28,2** | | |
| D-pad ← | 10,0 | D-pad → | 9,7 |
| D-pad ↑ | 10,2 | D-pad ↓ | 10,1 |

### Custom keycodes

| Code | Name | Does |
|---|---|---|
| `0x7E00` | DICTATE | **PowerMic Record** — start/stop dictation |
| `0x7E01` | SCROLLUP | continuous scroll up (toggle) |
| `0x7E02` | SCROLLDOWN | continuous scroll down (toggle) |
| `0x7E03` | FAST_UP | continuous scroll up, fast |
| `0x7E04` | FAST_DOWN | continuous scroll down, fast |
| `0x7E05` / `0x7E06` | FX_NEXT / FX_PREV | next / previous lighting effect |
| `0x7E28`–`0x7E2B` | ASCROLLUP / ASCROLLDOWN / AFAST_UP / AFAST_DOWN | alternate-speed versions (used on layer 1) |
| `0x7E2C` | LED_RESYNC | flip the dictate colour only (no PowerMic press) |
| `0x7E1F` | PM_K_TRANSCRIBE | PowerMic Button 1 |
| `0x7E20` | PM_K_TABBACK | **previous field** (Button 2) |
| `0x7E21` | PM_K_TABFWD | **next field** (Button 4) |
| `0x7E22` | PM_K_REWIND | Button 5 |
| `0x7E23` | PM_K_FFWD | Button 6 |
| `0x7E24` | PM_K_STOPPLAY | Button 7 |
| `0x7E25` | PM_K_CUSTOML | Button 8 |
| `0x7E26` | PM_K_ENTERSEL | Button 9 |
| `0x7E27` | PM_K_CUSTOMR | Button 10 |

Macro-runner keys (each plays one stored macro — see §6):

| Code | Name | Macro |
|---|---|---|
| `0x7E07` | K01_ANNOT | M15 |
| `0x7E08` | K02_WLSOFT | M4 |
| `0x7E09` | K03_WLLUNG | M8 |
| `0x7E0A` | K04_WLVASC | M10 |
| `0x7E0B` | K05_ZOOM | M13 |
| `0x7E0C` | K06_EPIC | M2 |
| `0x7E0D` | K07_WLBONE | M5 |
| `0x7E0E` | K09_WLBRAIN | M6 |
| `0x7E0F` | K10_ARROW | M12 |
| `0x7E10` | K11_COPYACC | M0 |
| `0x7E11` | K15_MEASURE | M14 |
| `0x7E12` | K16_GE | M1 |
| `0x7E13` | K17_SPINEC | M20 |
| `0x7E14` | K18_SPINET | M21 |
| `0x7E15` | K19_SPINEL | M22 |
| `0x7E16` | L05_INTERZOOM | M18 |
| `0x7E17` | L07_WLHDWR | M9 |
| `0x7E18` | L09_WLSTROKE | M7 |
| `0x7E19` | L10_ELLIPSE | M16 |
| `0x7E1A` | L15_ROI | M17 |
| `0x7E1B` | L16_MCKESSON | M3 |
| `0x7E1C` | KX_HANG | M19 (not on a key by default) |
| `0x7E1D` | KX_WLSUBDURAL | M11 (not on a key by default) |

Standard QMK keycodes work too — `KC_F13` is `0x0068`, `MO(1)` is `0x5221`,
`C(KC_F15)` is `0x016A`, etc. Look them up in QMK's `quantum/keycodes.h`.

### Worked example — put "next field" on key 05

```powershell
. .\tools\pmcfg.ps1
PM-SetKey 0 4 2 0x7E21     # key 05 is row 4, col 2; 0x7E21 = PM_K_TABFWD
PM-Save
```

---

## 6. Macros (these need a rebuild)

Keys 01–19 mostly run **dynamic macros** M0–M22 — recorded click/keystroke
sequences for the *original owner's* PACS (Epic, GE, McKesson, window/level
presets). A new user will want different ones.

The defaults live in `macro_defaults[]` in `keymap.c`, in QMK send-string syntax:

```c
SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_1) "\0"   // M4  window/level: soft tissue
SS_LCTL("2") SS_DELAY(2500) SS_LCTL("v") ...      // M2  open Epic
```

`SS_TAP(X_BTN1)` = left click, `SS_DELAY(n)` = wait n ms, `SS_LCTL("c")` = Ctrl+C,
`"\0"` terminates each macro. Order in the array is M0, M1, M2, …

**To change what a macro does you must edit `keymap.c`, bump `TRGB_CFG_VERSION`,
rebuild and reflash.** (There is no macro-write command on the config channel.)

**Simplest alternative that needs no rebuild:** if the target action is a plain
shortcut, just map the key straight to that keycode instead of to a macro. E.g.
Ctrl+Shift+N → `C(S(KC_N))` = `0x1D11`. Only multi-step sequences with delays
actually need a macro.

---

## 7. Rebuilding from source (only if §6 applies)

You need the full QMK tree with this keyboard and the PowerMic core patch — it is
**not** just `keymap.c`. Ask the sender for the repository
(`github.com/whyaaronbailey/adafruit_rp2040_usbh`, branch `via2`, which sits inside
a QMK checkout at `keyboards/converter/adafruit_rp2040_usbh`), or for a copy of the
whole tree.

```bash
qmk compile -kb converter/adafruit_rp2040_usbh -km tartarus2_sk
```

- **Always `rm -rf .build/obj_converter_adafruit_rp2040_usbh_tartarus2_sk` first.**
  QMK will happily link a stale `keymap.o` and you'll flash your *old* code while
  believing you changed something. This has burned us repeatedly.
- Output: `.build/converter_adafruit_rp2040_usbh_tartarus2_sk.uf2`
- Flash: hold BOOT while plugging in → drag the `.uf2` onto the `RPI-RP2` drive.
  Or send the bootloader magic from §4 and skip the button.

A portable, documented version of just the PowerMic emulation (for putting it on a
*different* QMK keyboard) lives in the same QMK tree at `users/powermic/`, with its
own README and a `core-patch/` that applies to stock QMK.

---

## 8. Lighting

Defaults: idle **light blue** `HSV(152,255,255)`, dictating **red** `HSV(0,255,255)`,
held-key/scroll-pulse **yellow** `HSV(43,255,255)`, brightness 255.

Change without rebuilding:

```powershell
. .\tools\pmcfg.ps1
PM-Cmd ([byte[]](0x05, 0, 85, 255))   # idx 0 (idle) -> green (hue 85)
PM-Cmd ([byte[]](0x06, 180))          # brightness 180
PM-Save
```

Eleven idle effects (index 0–10): static, spectrum cycle, rainbow wave, colour
wheel, three breathing modes, three starlight modes, reactive-cyan. Cycle them with
thumb+18 / thumb+19 on the device, then SAVE. Effect 0 is the reactive per-key mode
described above and is the sensible default.

**Dictate colour drift — explain this if the user reports "the colour is
backwards":** PowerScribe never tells the PowerMic whether it is recording (a full
USB capture shows zero host→HID traffic; the real handset infers it from its own
microphone stream, which this adapter does not have). So `dictating` is a local
toggle and drifts if recording starts/stops any other way. Remedies: **thumb+20**
(`LED_RESYNC`), the 15-minute auto-revert (`DICT_AUTO_REVERT_MS`), or a host helper
pushing `0x09 DICT_SYNC` (see `users/powermic/host/` in the repo — an AutoHotkey
include that reads Windows audio sessions). The *keys* are never wrong; only the
colour can drift.

---

## 9. Diagnosing

```powershell
. .\tools\pmcfg.ps1
PM-Proto        # expect: Proto 1, Layers 2, Rows 36, Cols 8
```

If that errors, the device isn't enumerating as a PowerMic — check it's plugged in
and that Device Manager shows **PowerMicII-NS**.

| Symptom | Cause |
|---|---|
| PowerScribe ignores key 20 | Another PowerMic (real or a second adapter) won the bind. Unplug it and restart PowerScribe. |
| Keys do nothing, LEDs fine | Tartarus not enumerated on the host port — unplug/replug the Tartarus. |
| LEDs dead, keys fine | Core 1 didn't bring the RGB up — replug the Tartarus. |
| Edited `keymaps[]`, no change | Forgot to bump `TRGB_CFG_VERSION`, or a stale `.build` dir. |
| `hidapitester --read-feature` returns `read -1` | A known quirk of that tool on this device; the channel is fine. Use `tools/pmcfg.ps1`. |

---

## 10. Quick reference

- USB: `0554:1001` "PowerMicII-NS", config collection at usagePage `0x01` / usage `0x00`, 40-byte feature buffer
- Layers: 2 (0 = base, 1 = thumb held). Matrix 36×8.
- Dictate = `0x7E00` on key 20 (row 5, col 4)
- Always `SAVE` (`0x07`) after `SET_*`
- Bump `TRGB_CFG_VERSION` after editing `keymaps[]` in source
- Bootloader magic: `B0 07 10 AD B0 07 10 AD`
