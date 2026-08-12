# Clean PowerMic-emulation build: present exactly ONE PowerMic HID interface so
# PowerScribe's HID manager binds it unambiguously. RAW_ENABLE is OFF - a raw-HID
# interface shares our 0554:1001 VID/PID and PowerScribe's scanner grabbed it as a
# second "PowerMic" candidate (verified via its own HIDManager). The PowerMic HID
# still claims EP 0x81 (usb_descriptor.h reassigns JOYSTICK_IN_EPNUM ahead of the
# keyboard when POWERMIC_HID_DESC is defined).
EXTRAKEY_ENABLE = no
MOUSEKEY_ENABLE = yes
NKRO_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
VIA_ENABLE = no
VIRTSER_ENABLE = no
POINTING_DEVICE_ENABLE = yes

JOYSTICK_ENABLE = yes
JOYSTICK_OWN_EP = yes
MOUSE_SHARED_EP = no

RAW_ENABLE = no
DEFERRED_EXEC_ENABLE = yes
SRC += powermic.c

# No VIA and nothing to persist, so keep EEPROM in RAM. This also sidesteps the
# core-1 flash-park handshake entirely: the wear-leveling driver's first-boot
# EEPROM write enters an unbounded core-0<->core-1 spin (c1_main.c) that can
# hang the whole main loop while enumeration stays up - which is exactly what
# bricked the raw-HID reflash path on earlier pm builds.
EEPROM_DRIVER = transient

# NOTE: build with info.json usb set to vid 0x0554 pid 0x1001 device_version 1.4.3
# manufacturer Nuance keyboard_name PowerMicII-NS. The POWERMIC_HID_DESC override
# in tmk_core/protocol/usb_descriptor.c supplies the real PowerMic HID descriptor.
