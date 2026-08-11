# Minimal PowerMic-emulation build: get the PowerMic HID onto interface 3.
# Interface order (QMK): keyboard(0), RAW(1), MOUSE(2), JOYSTICK(3).
EXTRAKEY_ENABLE = no
MOUSEKEY_ENABLE = yes
NKRO_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
VIA_ENABLE = no

JOYSTICK_ENABLE = yes
JOYSTICK_OWN_EP = yes
MOUSE_SHARED_EP = no

RAW_ENABLE = yes
DEFERRED_EXEC_ENABLE = yes
SRC += powermic.c

# NOTE: build with info.json usb set to vid 0x0554 pid 0x1001 device_version 1.4.3
# manufacturer Nuance keyboard_name PowerMicII-NS. The POWERMIC_HID_DESC override
# in tmk_core/protocol/usb_descriptor.c supplies the real PowerMic HID descriptor.
