EXTRAKEY_ENABLE = yes
MOUSEKEY_ENABLE = yes

# PowerMic II button emulation. QMK's joystick interface is used purely as a
# transport: with 0 axes and 24 buttons it produces a buttons-only 3-byte
# report on the Button usage page, on its own HID interface and endpoint, so
# the keyboard and mouse interfaces are untouched.
JOYSTICK_ENABLE = yes
# Required: EXTRAKEY/MOUSE already enable the shared endpoint, which would
# otherwise force the joystick onto it behind a REPORT_ID. The PowerMic report
# must reach the wire as bare bytes.
JOYSTICK_OWN_EP = yes
# Also required, for a structural reason in usb_descriptor.c: with the default
# MOUSE_SHARED_EP = yes, the SharedReport[] array is opened at the mouse block
# and stays open past the joystick block, so a standalone JoystickReport[]
# cannot be declared there. Giving the mouse its own endpoint defers
# SharedReport[] to the extrakey/NKRO section, after the joystick.
# The mouse keeps working; it just loses its REPORT_ID prefix and gains an EP.
MOUSE_SHARED_EP = no
SRC += powermic.c
# Generalizable Razer Tartarus RGB driver (OpenRazer-derived, GPL-2.0).
SRC += tartarus_rgb.c
# hsv_to_rgb for the VIA colour pickers (not pulled in without an RGB feature).
SRC += color.c

TAP_DANCE_ENABLE = no
COMBO_ENABLE = NO
NKRO_ENABLE = no
DEFERRED_EXEC_ENABLE = yes

# RAW HID: the only host->firmware channel that works from the dev box (keyboard
# injection is blocked there). Carries the reflash trigger and the Razer command
# relay for bench-testing LED control.
RAW_ENABLE = yes

# VIA: graphical remapping + custom "Lighting" menus in usevia.app. The bench
# opcodes coexist with the VIA protocol via via_command_kb (ids >= 0xB0).
VIA_ENABLE = yes

# PowerScribe's PowermicCtrl binds the FIRST HID device matching 0554:1001, so
# every extra generic-HID collection we expose is a decoy that steals the bind
# (proven with its own HIDManager: it enumerated 5 candidates and the PowerMic
# was last). Keyboard/mouse are class-exclusive and never offered, but console
# (usagePage 0xFF31) and extrakey (consumer/system) are - and neither is used by
# this keymap, which is all F13-F24 plus modifier combos. Dropping them removes
# two decoys; raw HID stays because VIA needs it.
CONSOLE_ENABLE = no
EXTRAKEY_ENABLE = no

# PowerScribe-only variant: NO VIA and NO raw HID. Nuance's own classifier
# (POWERMICCTRLLib.USBDeviceMgr) reports usbdevPowerMic2 for EVERY HID collection
# under 0554:1001 - so VIA's raw interface is seen as a SECOND PowerMic II and
# competes for the bind that PowermicCtrl makes. Exposing exactly one collection
# is what made the earlier build work at the workstation. LEDs are unaffected:
# they are driven over the USB HOST side (core 1), not raw HID.
VIA_ENABLE = no
RAW_ENABLE = no
