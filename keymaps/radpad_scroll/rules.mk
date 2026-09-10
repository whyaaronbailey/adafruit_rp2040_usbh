# RadPad - scroll-only trackball board.
# This board does NOT impersonate a PowerMic: two devices claiming 0554:1001
# on one PC would compete for PowerScribe's bind (it takes the first match it
# finds, in arbitrary per-machine order) and dictation would fail
# intermittently. Only the Tartarus board wears that disguise.
USER_NAME := radpad_pointer

# Raw HID: this board is NOT a PowerMic, so an extra HID collection costs
# nothing here (no PowerScribe binder to confuse). It carries the bootloader
# hatch and the pointer diagnostics - without it, a dongle that fails to
# enumerate would leave no way to see why and no software reflash path.
RAW_ENABLE = yes
