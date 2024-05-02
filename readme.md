

# USB-to-USB convertor using [Adafruit RP2040 USB Host](https://www.adafruit.com/product/5723)

This is based on [Sekigon's Keyboard Quantizer mini-full branch](https://github.com/sekigon-gonnoc/qmk_firmware/tree/keyboard/sekigon/keyboard_quantizer/mini-full/keyboards/sekigon/keyboard_quantizer/mini) and [GongYiLiao's branch supporting the Adafruit RP2040 with USB Host](https://github.com/GongYiLiao/qmk_AdaFruitRp2040USBH)

As GongYiLiao mentioned, the change from Sekigon's original code was to specify `DP+` Pin as 16 (thus `DP-` is 17) and the 5V pin (18) in `c1_usbh.c`:

```
// Initialize USB host stack on core1
void c1_usbh(void) {
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp                  = 16;
    // pio_cfg.extra_error_retry_count = 10;
    pio_cfg.skip_alarm_pool         = true;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);
    gpio_put(18, 1);

    tuh_init(1);
    c1_start_timer();
}

```

## Available keymaps

GongYiLiao included layouts for: 

* [Kinesis Advantage MPC (KB500)](https://www.kinesis-ergo.com/wp-content/uploads/kb500-user_manual.pdf) 
* Generic ANSI 104 layout
* [Pok3r](https://mechanicalkeyboards.com/shop/index.php?l=product_detail&p=3633) as an example for 60% keyboard 

and added custom DVORAK and other keymaps for those devices.

This distribution is generic, and includes layouts and generic keymaps for :
* Generic ANSI 104 layout under `keymaps/ansi`
* Razer Tartarus V2 under `keymaps/tartarus`

## How to use this repository

After [setup your qmk envorinment](https://github.com/qmk/qmk_firmware/blob/master/docs/newbs_getting_started.md), clone this repository to `keyboards/converter` then run

```
git clone https://github.com/whyaaronbailey/adafruitrp2040_usbh.git _your_qmk_repo/keyboards/converter/adafruit_rp2040_usbh
cd _your_qmk_repo/keyboards/converter/adafruit_rp2040_usbh
cd ../../..
make converter/adafruit_rp2040_usbh:_your_choice:uf2 
```

where `_your_choice` can be `ansi` for generic 104-key ANSI keyboard and `tartarus` for the Razer Tartarus V2


## TODO:
* I removed the line "git submodule update --init --recursive" from instructions above because this currently depends on an older fork of [Hathach's TinyUSB](https://github.com/hathach/tinyusb) prior to 2/23/24 commits. The older distribution is included here. Need to update to using current TinyHub commits.
* Add back support for the KA500 and pokr
* Is LED light control possible?
