SRC += matrix.c c1_main.c c1_usbh.c tusb_os_custom.c
CUSTOM_MATRIX = lite
POINTING_DEVICE_DRIVER = custom

OPT_DEFS += -DCRT0_EXTRA_CORES_NUMBER=1

SRC += lib/Pico-PIO-USB/src/pio_usb.c
SRC += lib/Pico-PIO-USB/src/pio_usb_host.c
SRC += lib/Pico-PIO-USB/src/usb_crc.c
VPATH += keyboards/converter/adafruit_rp2040_usbh/lib/Pico-PIO-USB/src

SRC += lib/tinyusb/src/tusb.c
SRC += lib/tinyusb/src/common/tusb_fifo.c
SRC += lib/tinyusb/src/host/usbh.c
SRC += lib/tinyusb/src/host/hub.c
SRC += lib/tinyusb/src/class/hid/hid_host.c
SRC += lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
VPATH += keyboards/converter/adafruit_rp2040_usbh/lib/tinyusb/src

SRC += lib/pico-sdk/src/rp2_common/hardware_dma/dma.c
SRC += lib/pico-sdk/src/host/pico_stdlib/stdlib.c
VPATH += lib/pico-sdk/src/rp2_common/hardware_dma/include
VPATH += lib/pico-sdk/src/rp2_common/hardware_uart/include
VPATH += lib/pico-sdk/src/rp2_common/pico_stdio/include
VPATH += lib/pico-sdk/src/common/pico_stdlib/include
VPATH += lib/pico-sdk/src/common/pico_time/include
VPATH += lib/pico-sdk/src/common/pico_sync/include
VPATH += lib/pico-sdk/src/common/pico_util/include

# KEY_OVERRIDE_ENABLE = yes
# COMBO_ENABLE = yes
# TAP_DANCE_ENABLE = no
# LEADER_ENABLE = yes
# OS_DETECTION_ENABLE = yes
