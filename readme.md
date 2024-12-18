

# USB-to-USB convertor using [Adafruit RP2040 USB Host](https://www.adafruit.com/product/5723)

This is based on [Sekigon's Keyboard Quantizer mini-full branch](https://github.com/sekigon-gonnoc/qmk_firmware/tree/keyboard/sekigon/keyboard_quantizer/mini-full/keyboards/sekigon/keyboard_quantizer/mini) and [GongYiLiao's branch supporting the Adafruit RP2040 with USB Host](https://github.com/GongYiLiao/qmk_AdaFruitRp2040USBH) and incorporates updates from [raghur's audio-keys project](https://github.com/raghur/adafruit_rp2040_usbh/tree/audio-keys). 

This is for QMK 0.24. I have not updated it for newer builds. However, raghur's change's have removed the prior vendor dependences. This now works with the latest tinyUSB and PICO-PIO-USB distributions, with the tested distrubtions as submodules. 

## Available keymaps

This distribution is generic, and includes layouts and generic keymaps for :
* Generic ANSI 104 layout under `keymaps/ansi`
* Razer Tartarus V2 under `keymaps/tartarus`

## How to use this repository

Download Git 0.24! or 
```
git checkout 0.24.0 -b qmk-0.24.0
git submodule update --init --recursive
```

After [setup your qmk envorinment](https://github.com/qmk/qmk_firmware/blob/master/docs/newbs_getting_started.md), clone this repository to `keyboards/converter` then run

```
git clone https://github.com/whyaaronbailey/adafruitrp2040_usbh.git _your_qmk_repo/keyboards/converter/adafruit_rp2040_usbh
cd _your_qmk_repo/keyboards/converter/adafruit_rp2040_usbh
cd ../../..
make converter/adafruit_rp2040_usbh:_your_choice:uf2 
```

where `_your_choice` can be `ansi` for generic 104-key ANSI keyboard and `tartarus` for the Razer Tartarus V2


## TODO:
* Custom Tartarus info.json
* LED light control
