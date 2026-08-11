

# USB-to-USB convertor using [Adafruit RP2040 USB Host](https://www.adafruit.com/product/5723)

This is based on [Sekigon's Keyboard Quantizer mini-full branch](https://github.com/sekigon-gonnoc/qmk_firmware/tree/keyboard/sekigon/keyboard_quantizer/mini-full/keyboards/sekigon/keyboard_quantizer/mini) and [GongYiLiao's branch supporting the Adafruit RP2040 with USB Host](https://github.com/GongYiLiao/qmk_AdaFruitRp2040USBH) and incorporates updates from [raghur's audio-keys project](https://github.com/raghur/adafruit_rp2040_usbh/tree/audio-keys). 

This is for QMK 0.24. I have not updated it for newer builds. However, raghur's change's have removed the prior vendor dependences. This now works with the latest tinyUSB and PICO-PIO-USB distributions, with the tested distrubtions as submodules. 

## LED control

The converter can now drive the Tartarus V2/Pro backlight. tartarus_rgb.c is a
small driver derived from OpenRazer (GPL-2.0) with static colors, brightness,
the built-in Razer effects, and a 20-key custom framebuffer. Everything runs
with the device in normal mode. Do not put the device in Razer driver mode; it
stops sending key reports while the LEDs keep working, and the mode sticks
until the device loses power. The default lighting is per-key reactive: idle
color baseline, pressed keys light up in the accent color, and the active
scroll key pulses at the scroll speed. Protocol details are in
tartarus_rgb.README.md.

## VIA

The tartarus2_ahk and tartarus2_qmk keymaps support [VIA](https://usevia.app).
Keys can be remapped in the browser and stored in EEPROM, the PACS macro
sequences are editable in the Macros pane, and a custom Lighting menu sets the
effect, colors, brightness and scroll speeds. Settings survive power loss.
Load `via_definition/adafruit_rp2040_usbh_tartarus.json` from the Design tab
(leave "Use V2 definitions" off).

EEPROM writes used to crash the USB host stack on core 1. The wear-leveling
backing store now parks core 1 during flash operations.

## AutoHotkey

tartarus.ahk is the workstation half of the ahk build. It maps the F-codes to
Centricity and PowerScribe actions with window activation, and reports
dictate/scroll state back to the converter so the LEDs follow. hidapitester.exe
(from todbot/hidapitester, a copy is in `tools/`) must sit next to the script.

## Available keymaps

This distribution is generic, and includes layouts and keymaps for:
* Generic Full Size ANSI 104 layout under `keymaps/default`
* Razer Tartarus V2/Pro:
  * `keymaps/tartarus2` - each key sends a unique F-key or modifier+F-key
    combo and all the real functions live host-side in tartarus.ahk. No VIA,
    no LEDs.
  * `keymaps/tartarus2_ahk` - same key assignments as tartarus2, plus VIA, the
    LED engine, and the PACS macros available in the VIA palette. This is what
    I use. Prebuilt UF2s are in `firmware/`.
  * `keymaps/tartarus2_qmk` - self-contained variant: the PACS functions are
    on the keys as VIA macros, with PowerMic dictate emulation and firmware
    scroll toggles. Needs no host-side software, but cannot activate windows.
  * `keymaps/tartarus2_pacs` - older pre-VIA version of the same idea.

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
[OpenRazer](https://github.com/openrazer/openrazer) (GPL-2.0); see
tartarus_rgb.README.md. `tools/hidapitester.exe` is
[todbot/hidapitester](https://github.com/todbot/hidapitester).

## TODO:
* Scroll wheel and scroll click customization (need to implement additional interfaces)
* Dynamic VIA layout detection for other host keyboards (see README_VIA.md groundwork in the _via tree)
