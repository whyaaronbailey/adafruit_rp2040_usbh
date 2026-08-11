

# USB-to-USB convertor using [Adafruit RP2040 USB Host](https://www.adafruit.com/product/5723)

This is based on [Sekigon's Keyboard Quantizer mini-full branch](https://github.com/sekigon-gonnoc/qmk_firmware/tree/keyboard/sekigon/keyboard_quantizer/mini-full/keyboards/sekigon/keyboard_quantizer/mini) and [GongYiLiao's branch supporting the Adafruit RP2040 with USB Host](https://github.com/GongYiLiao/qmk_AdaFruitRp2040USBH) and incorporates updates from [raghur's audio-keys project](https://github.com/raghur/adafruit_rp2040_usbh/tree/audio-keys). 

This is for QMK 0.24. I have not updated it for newer builds. However, raghur's change's have removed the prior vendor dependences. This now works with the latest tinyUSB and PICO-PIO-USB distributions, with the tested distrubtions as submodules. 

## Highlights

* **Tartarus per-key RGB control from the converter.** A generalizable,
  OpenRazer-derived driver (`tartarus_rgb.c/h`, GPL-2.0) gives the firmware a
  QMK-RGB-Matrix-style API over the Tartarus V2/Pro backlight: static colours,
  brightness, every native hardware effect (spectrum, wave, wheel, breathing,
  starlight, reactive), and a 20-key custom framebuffer — all in the device's
  normal mode, with the keyboard fully functional (Razer "driver mode" is
  never used; it kills key reporting). The default lighting is per-key
  reactive: idle colour baseline, pressed keys light in the accent colour,
  and the active scroll key pulses at the scroll speed. Protocol notes and
  the hard-won transport details are in `tartarus_rgb.README.md`.
* **VIA support.** The `tartarus2_ahk` and `tartarus2_qmk` keymaps are fully
  [VIA](https://usevia.app)-enabled: graphical key remapping (persists in
  EEPROM, no reflashing), GUI-editable dynamic macros for the PACS functions,
  and a custom **Lighting** menu (effect picker, three colour pickers,
  brightness and speed sliders) whose settings persist across power loss.
  Sideload `via_definition/adafruit_rp2040_usbh_tartarus.json` in usevia.app's
  Design tab.
* **Dual-core flash safety.** VIA EEPROM writes previously hard-faulted the
  PIO-USB host on core 1 (XIP disabled while core 1 executed flash-resident
  code). The wear-leveling backing store now parks core 1 across every flash
  operation.
* **AutoHotkey companion.** `tartarus.ahk` (+ `tools/hidapitester.exe`) is the
  workstation half of the AHK model: it maps the F-codes to Centricity /
  PowerScribe actions with window activation, and mirrors dictate/scroll state
  back into the LEDs over RAW HID.

## Available keymaps

This distribution is generic, and includes layouts and keymaps for:
* Generic Full Size ANSI 104 layout under `keymaps/default`
* Razer Tartarus V2/Pro:
  * `keymaps/tartarus2` — plain F-key emission (F13–F24, Ctrl/Shift combos);
    all mapping is done host-side. No VIA, no LEDs.
  * `keymaps/tartarus2_ahk` — the **AHK model** with everything on top: same
    F-code defaults as `tartarus2`, plus VIA, the LED engine (per-key reactive
    baseline, red-while-dictating, per-key scroll pulse driven by the AHK
    script), and GUI-editable PACS macros in the palette. Prebuilt UF2s live
    in `firmware/`.
  * `keymaps/tartarus2_qmk` — the **self-contained model**: the PACS functions
    live on the keys themselves as VIA dynamic macros (seeded with the original
    sequences), plus Dictate/PowerMic emulation, firmware scroll toggles, and
    the same LED engine. Works with no host-side software, but cannot activate
    target windows.
  * `keymaps/tartarus2_pacs` — the pre-VIA ancestor of `tartarus2_qmk`
    (hardcoded PACS sequences, RAW HID bench harness).

## How to use this repository

Install using the normal guide: [setup your qmk envorinment](https://github.com/qmk/qmk_firmware/blob/master/docs/newbs_getting_started.md). This will install the latest version of QMK. 

Then run the following:
```
git checkout 0.24.0 -b qmk-0.24.0
git submodule update --init --recursive
```
This will install QMK 0.24. This is a requisite step. Don't skip it.

After clone this repository to `keyboards/converter` then run

```
git clone https://github.com/whyaaronbailey/adafruitrp2040_usbh.git _your_qmk_repo/keyboards/converter/adafruit_rp2040_usbh
cd _your_qmk_repo/keyboards/converter/adafruit_rp2040_usbh
git submodule update --init --recursive
cd ../../..
make converter/adafruit_rp2040_usbh:_your_choice:uf2 
```

where `_your_choice` is one of the keymaps listed above (`default`,
`tartarus2`, `tartarus2_ahk`, `tartarus2_qmk`, `tartarus2_pacs`).

## Attribution

The Razer LED protocol is derived from
[OpenRazer](https://github.com/openrazer/openrazer) (GPL-2.0) — see
`tartarus_rgb.README.md` for specifics. `tools/hidapitester.exe` is
[todbot/hidapitester](https://github.com/todbot/hidapitester), bundled for the
AHK LED transport.

## TODO:
* Scroll wheel and scroll click customization (need to implement additional interfaces)
* Dynamic VIA layout detection for other host keyboards (see README_VIA.md groundwork in the _via tree)
