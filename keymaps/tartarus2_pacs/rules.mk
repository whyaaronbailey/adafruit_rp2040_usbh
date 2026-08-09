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

TAP_DANCE_ENABLE = no
COMBO_ENABLE = NO
NKRO_ENABLE = yes
DEFERRED_EXEC_ENABLE = yes
