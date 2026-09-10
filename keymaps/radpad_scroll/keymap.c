// RadPad - a trackball dedicated to scrolling CT/MRI slices.
//
// The ball drives the scroll wheel and never moves the cursor. A second
// trackball plugged straight into the PC handles pointing, so the two roles
// stay physically separated.
//
// Policy is matched per attached device, first hit wins, so this same firmware
// handles a board with two trackballs on a hub doing different jobs. To turn
// this board back into a normal pointer later, change one enum below.
#include QMK_KEYBOARD_H
#include "radpad_pointer.h"
#include "raw_hid.h"
#include "adafruit_rp2040_usbh.h"
#include "bootloader.h"

const radpad_policy_t radpad_policies[] = {
    // Kensington Orbit wireless receiver. PID confirmed by enumerating it
    // directly on the PC: 047D:80A6 (NOT 047D:2048 - that guess was wrong).
    //
    // This device is unusual in two ways, both verified from its HID caps:
    //   * it uses REPORT IDs - id 1 carries wheel + AC pan, id 2 carries X/Y
    //   * X and Y are 12-BIT, logical -2047..2047, not the usual 8-bit +/-127
    // The converter's parse_value() sign-extends arbitrary bit widths and
    // parse_report() demuxes report ids, so both are handled - but the 16x
    // larger delta range is why divisor starts much higher here than it would
    // for an 8-bit mouse. Expect to tune this against the real ball.
    { .vid = 0x047D, .pid = 0x80A6,
      .mode = PTR_SCROLL_V, .divisor = 96, .accel = 40, .max_gain = 800,
      .keep_buttons = true },

    // TEST ENTRY: any Razer device attached to this board also scrolls, so the
    // whole path can be proven with a Razer mouse while the Orbit's dongle is
    // missing. Delete once the Orbit is in use.
    { .vid = 0x1532,
      .mode = PTR_SCROLL_V, .divisor = 16, .accel = 40, .max_gain = 800,
      .keep_buttons = true },

    // Catch-all: anything else behaves as a normal pointer.
    { .mode = PTR_CURSOR },
};
const uint8_t radpad_policy_count = ARRAY_SIZE(radpad_policies);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

// --- raw HID diagnostics + reflash hatch ------------------------------------
// 0xB0                 -> jump to bootloader (software reflash, no BOOTSEL)
// 0xD0                 -> pointer telemetry, so a dongle that does not
//                         enumerate can be told apart from one that enumerates
//                         but whose reports never reach the policy layer.
extern volatile uint16_t radpad_vid, radpad_pid, radpad_reports;
extern volatile int16_t  radpad_last_x, radpad_last_y, radpad_last_v, radpad_last_clicks;
extern volatile uint8_t  radpad_last_mode;

void raw_hid_receive(uint8_t *data, uint8_t length) {
    uint8_t resp[32] = {0};
    switch (data[0]) {
        case 0xB0:
            bootloader_jump();
            break;

        case 0xD0:
            resp[0]  = 0xD0;
            resp[1]  = tartarus_instance_count();          // HID interfaces the adapter mounted
            resp[2]  = (uint8_t)(radpad_vid & 0xFF);       // VID/PID the policy layer saw
            resp[3]  = (uint8_t)(radpad_vid >> 8);
            resp[4]  = (uint8_t)(radpad_pid & 0xFF);
            resp[5]  = (uint8_t)(radpad_pid >> 8);
            resp[6]  = (uint8_t)(radpad_reports & 0xFF);   // mouse reports seen
            resp[7]  = (uint8_t)(radpad_reports >> 8);
            resp[8]  = (uint8_t)(radpad_last_x & 0xFF);
            resp[9]  = (uint8_t)(radpad_last_x >> 8);
            resp[10] = (uint8_t)(radpad_last_y & 0xFF);
            resp[11] = (uint8_t)(radpad_last_y >> 8);
            resp[12] = (uint8_t)(radpad_last_v & 0xFF);
            resp[13] = (uint8_t)(radpad_last_v >> 8);
            resp[14] = (uint8_t)(radpad_last_clicks & 0xFF);
            resp[15] = (uint8_t)(radpad_last_clicks >> 8);
            resp[16] = radpad_last_mode;                   // 0xFE = no policy matched
            resp[17] = (uint8_t)(diag_rx_all & 0xFF);      // ALL host HID reports
            resp[18] = (uint8_t)(diag_rx_all >> 8);
            resp[19] = host_mount_devs;                    // USB devices mounted
            resp[20] = host_mount_hids;                    // HID interfaces mounted
            resp[21] = (uint8_t)(host_last_vid & 0xFF);    // what actually attached
            resp[22] = (uint8_t)(host_last_vid >> 8);
            resp[23] = (uint8_t)(host_last_pid & 0xFF);
            resp[24] = (uint8_t)(host_last_pid >> 8);
            resp[25] = host_last_addr;
            raw_hid_send(resp, sizeof(resp));
            break;
    }
}
