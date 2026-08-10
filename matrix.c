// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "print.h"

#include "tusb.h"
#include "pio_usb_ll.h"
#include "report_descriptor_parser.h"
#include "report_parser.h"

matrix_row_t*           matrix_dest;
bool                    mouse_send_flag = false;
static uint8_t          kbd_addr;
static uint8_t          kbd_instance;
static int32_t          led_count = -1;
static uint8_t          hid_report_buffer[64];
static volatile uint8_t hid_report_size;
static uint8_t          hid_instance;
static bool             hid_disconnect_flag;
static uint8_t          pre_keyreport[8];
#define LED_BLINK_TIME_MS 50
#define KQ_PIN_LED 7
#define MATRIX_MSBTN_ROW 22

extern void busy_wait_us(uint64_t delay_us);
static bool send_led_report(uint8_t* leds);

void matrix_init_custom(void) {
    // Configure LED pin
    setPinOutput(KQ_PIN_LED);
    writePinHigh(KQ_PIN_LED);
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool matrix_has_changed = false;

    // If keyboard is disconnected, clear matrix
    if (hid_disconnect_flag) {
        for (uint8_t rowIdx = 0; rowIdx < MATRIX_ROWS; rowIdx++) {
            if (current_matrix[rowIdx] != 0) {
                matrix_has_changed     = true;
                current_matrix[rowIdx] = 0;
            }
        }

        hid_disconnect_flag = false;

        return matrix_has_changed;
    }

    // If keyboard report is received, apply it to matrix
    if (hid_report_size > 0) {
        matrix_dest        = current_matrix;
        matrix_has_changed = parse_report(hid_instance, hid_report_buffer, hid_report_size);
        hid_report_size    = 0;
        return matrix_has_changed;
    } else {
        return false;
    }

    return matrix_has_changed;
}

void housekeeping_task_kb(void) {
    // Control keyboard indicator LED
    static uint8_t keyboard_led;
    if (keyboard_led != host_keyboard_leds()) {
        uint8_t led_backup = keyboard_led;
        keyboard_led       = host_keyboard_leds();
        if (!send_led_report(&keyboard_led)) {
            keyboard_led = led_backup;
        }
    }

    // Blink LED when USB reports are received
    if (led_count >= 0) {
        if (timer_elapsed(led_count) < LED_BLINK_TIME_MS) {
            writePinLow(KQ_PIN_LED);
        } else if (timer_elapsed(led_count) < 2 * LED_BLINK_TIME_MS) {
            writePinHigh(KQ_PIN_LED);
        } else {
            led_count = -1;
        }
    }

    housekeeping_task_user();
}

static bool send_led_report(uint8_t* leds) {
    if (kbd_addr != 0) {
        return tuh_hid_set_report(kbd_addr, kbd_instance, 0, HID_REPORT_TYPE_OUTPUT, leds, sizeof(*leds));
    }

    return false;
}

// Push a HID FEATURE report to the mounted keyboard. Same call the lock-LED path
// uses, only with report type FEATURE (wValue high byte 0x03, matching OpenRazer's
// SET_REPORT value 0x0300) and a caller-supplied length. Gated to any Razer device
// (VID 0x1532) so a stray 90-byte vendor report never reaches an unrelated keyboard.
// Records diagnostics into tartarus_dbg_* because this build has no console.
volatile int16_t tartarus_dbg_pid      = -1;
volatile uint8_t tartarus_dbg_stage    = 0;
volatile uint8_t tartarus_dbg_instance = 0xFF; // which HID instance we send to
volatile uint8_t tartarus_dbg_icount   = 0;    // how many HID instances the device has
volatile uint8_t razer_cb_fired        = 0;    // FEATURE set_report completion callback ran
volatile uint16_t razer_cb_len         = 0;    // bytes the completed transfer reported

// The TinyUSB HID instance index of the Razer control interface is NOT stable
// across re-enumerations, so we can't hardcode it. Detect it at mount time as
// the interface with the largest report descriptor (on Razer keyboards the
// vendor/control interface is by far the most complex: e.g. Tartarus control
// 186 bytes vs keyboard 61, mouse 94). LED commands target this instance.
volatile uint8_t razer_led_instance = 0;
static uint16_t  razer_led_desc_len = 0;

// Per-instance report-descriptor length and interface protocol, captured at
// mount. The Razer control interface is the one with the LARGEST descriptor
// (~186 bytes) and interface protocol NONE (0). Exposed for host-side probing.
volatile uint16_t razer_inst_desclen[8] = {0};
volatile uint8_t  razer_inst_proto[8]   = {0};

uint8_t tartarus_led_instance(void) {
    return razer_led_instance;
}

// bInterfaceNumber of a given HID instance (0xFF if unavailable).
uint8_t tartarus_inst_itfnum(uint8_t instance) {
    if (kbd_addr == 0) {
        return 0xFF;
    }
    tuh_itf_info_t info;
    if (tuh_hid_itf_get_info(kbd_addr, instance, &info)) {
        return info.desc.bInterfaceNumber;
    }
    return 0xFF;
}

// USB interface number (bInterfaceNumber / wIndex) of the Razer control
// interface — the value to pass to tartarus_send_iface(). Empirically this is
// the HIGHEST-numbered non-boot (interface protocol NONE) HID interface on the
// Tartarus. Interface numbers are device-fixed, unlike HID instance indices.
uint8_t tartarus_led_itfnum(void) {
    if (kbd_addr == 0) {
        return 0xFF;
    }
    uint8_t cnt  = tuh_hid_instance_count(kbd_addr);
    uint8_t best = 0xFF;
    for (uint8_t i = 0; i < cnt && i < 8; i++) {
        if (razer_inst_proto[i] == 0) {  // HID_ITF_PROTOCOL_NONE = vendor/control
            uint8_t itf = tartarus_inst_itfnum(i);
            if (itf != 0xFF && (best == 0xFF || itf > best)) {
                best = itf;
            }
        }
    }
    return best;
}

// Fires when a SET_REPORT control transfer completes. Filtered to FEATURE so it
// reflects our Razer send, not the lock-LED (OUTPUT) path. Tells us the transfer
// actually reached the device, not merely that it was queued.
void tuh_hid_set_report_complete_cb(uint8_t dev_addr, uint8_t idx, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if (report_type == HID_REPORT_TYPE_FEATURE) {
        razer_cb_fired = 1;
        razer_cb_len   = len;
    }
}

int tartarus_send_feature_report(const uint8_t *buf, uint16_t len) {
    tartarus_dbg_stage = 0;
    tartarus_dbg_pid   = -1;

    if (kbd_addr == 0) {
        return 0;
    }
    tartarus_dbg_stage |= 1;
    tartarus_dbg_instance = kbd_instance;
    tartarus_dbg_icount   = tuh_hid_instance_count(kbd_addr);

    uint16_t vid = 0, pid = 0;
    tuh_vid_pid_get(kbd_addr, &vid, &pid);
    tartarus_dbg_pid = (int16_t)pid;
    if (vid == 0x1532) {
        tartarus_dbg_stage |= 2;
    }
    if ((pid >> 8) == 0x02) {
        tartarus_dbg_stage |= 4;
    }

    if (vid != 0x1532) {
        return 1;
    }

    bool ok = tuh_hid_set_report(kbd_addr, kbd_instance, 0, HID_REPORT_TYPE_FEATURE, (void*)buf, len);
    if (ok) {
        tartarus_dbg_stage |= 8;
        return 3;
    }
    return 2;
}

uint8_t tartarus_instance_count(void) {
    return (kbd_addr == 0) ? 0 : tuh_hid_instance_count(kbd_addr);
}

// Send a Razer SET_REPORT to an explicit USB *interface number* (wIndex),
// replicating OpenRazer's razer_send_control_msg exactly: bmRequestType 0x21,
// bRequest 0x09 (SET_REPORT), wValue 0x0300 (FEATURE, report id 0), wIndex =
// interface number, 90-byte payload. This bypasses the HID-instance->interface
// mapping so we can target the interface OpenRazer uses (Tartarus V2 = 0x01).
static uint8_t rz_ctrl_buf[90];
volatile uint16_t razer_cb_count = 0;  // completed vendor SET_REPORTs since boot
static void rz_ctrl_complete(tuh_xfer_t *xfer) {
    if (xfer->setup->bRequest == 0x09) {
        razer_cb_fired = 1;
        razer_cb_len   = (xfer->result == XFER_RESULT_SUCCESS) ? xfer->setup->wLength : 0;
        razer_cb_count++;
    }
}
int tartarus_send_iface(uint8_t itf_num, const uint8_t *buf, uint16_t len) {
    if (kbd_addr == 0) {
        return 0;
    }
    uint16_t vid = 0, pid = 0;
    tuh_vid_pid_get(kbd_addr, &vid, &pid);
    if (vid != 0x1532) {
        return 1;
    }
    if (len > sizeof(rz_ctrl_buf)) {
        len = sizeof(rz_ctrl_buf);
    }
    memcpy(rz_ctrl_buf, buf, len);

    tusb_control_request_t const request = {
        .bmRequestType_bit = {.recipient = TUSB_REQ_RCPT_INTERFACE, .type = TUSB_REQ_TYPE_CLASS, .direction = TUSB_DIR_OUT},
        .bRequest = 0x09,
        .wValue   = tu_htole16(0x0300),
        .wIndex   = tu_htole16((uint16_t)itf_num),
        .wLength  = len,
    };
    tuh_xfer_t xfer = {
        .daddr       = kbd_addr,
        .ep_addr     = 0,
        .setup       = &request,
        .buffer      = rz_ctrl_buf,
        .complete_cb = rz_ctrl_complete,
        .user_data   = 0,
    };
    return tuh_control_xfer(&xfer) ? 3 : 2;
}

// Send to an explicit HID instance rather than the auto-picked keyboard one.
int tartarus_send_to(uint8_t instance, const uint8_t *buf, uint16_t len) {
    if (kbd_addr == 0) {
        return 0;
    }
    uint16_t vid = 0, pid = 0;
    tuh_vid_pid_get(kbd_addr, &vid, &pid);
    if (vid != 0x1532 || instance >= tuh_hid_instance_count(kbd_addr)) {
        return 1;
    }
    return tuh_hid_set_report(kbd_addr, instance, 0, HID_REPORT_TYPE_FEATURE, (void*)buf, len) ? 3 : 2;
}

static volatile bool set_protocol_complete = false;
void                 tuh_hid_set_protocol_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t protocol) {
    set_protocol_complete = true;
}

void tuh_mount_cb(uint8_t dev_addr) {
    dprintf("USB device is mounted:%d\n", dev_addr);

    if (led_count < 0) {
        led_count = timer_read();
    }

    // Fresh device: re-detect the control interface as its HID interfaces mount.
    razer_led_desc_len = 0;
    razer_led_instance = 0;

    for (int instance = 0; instance < tuh_hid_instance_count(dev_addr); instance++) {
        uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
        if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
            kbd_addr     = dev_addr;
            kbd_instance = instance;
        }
    }
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    dprintf("HID is mounted:%d:%d\n", dev_addr, instance);
    parse_report_descriptor((dev_addr * 16) + instance, desc_report, desc_len);

    if (instance < 8) {
        razer_inst_desclen[instance] = desc_len;
        razer_inst_proto[instance]   = tuh_hid_interface_protocol(dev_addr, instance);
    }

    // Identify the Razer control interface as the one with the LARGEST report
    // descriptor. On the Tartarus the vendor/control interface is by far the
    // most complex (~186 bytes) vs keyboard (~61) and mouse (~94), and this is
    // stable across re-enumerations. Track the running max as interfaces mount.
    if (desc_len > razer_led_desc_len) {
        razer_led_desc_len = desc_len;
        razer_led_instance = instance;
    }

    tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    dprintf("HID unmounted:%d:%d\n", dev_addr, instance);
    memset(pre_keyreport, 0, sizeof(pre_keyreport));
    hid_disconnect_flag = true;
    kbd_addr            = 0;
    kbd_instance        = 0;
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    dprintf("Report received\n");
    if (led_count < 0) {
        led_count = timer_read();
    }

    if (len > 0) {
        int cnt = 0;
        while (hid_report_size > 0 && cnt++ < 50000) {
            busy_wait_us(1);
            continue;
        }

        hid_instance = (dev_addr * 16) + instance;
        memcpy(hid_report_buffer, report, len);
        __compiler_memory_barrier();
        // hid_report_size is used as trigger of report parser
        hid_report_size = len;
    }

    tuh_hid_receive_report(dev_addr, instance);
}

__attribute__((weak)) void keyboard_report_hook(keyboard_parse_result_t const* report) {
    if (debug_enable) {
        uprintf("Keyboard report\n");
        for (int idx = 0; idx < sizeof(report->bits); idx++) {
            uprintf("%02X ", report->bits[idx]);
        }
        uprintf("\n");
    }

    for (uint8_t rowIdx = 0; rowIdx < MATRIX_ROWS - 1; rowIdx++) {
        matrix_dest[rowIdx] = report->bits[rowIdx];
    }

    // copy modifier bits
    matrix_dest[KC_LEFT_CTRL / 8] = report->bits[28];
}

__attribute__((weak)) void mouse_report_hook(mouse_parse_result_t const* report) {
    if (debug_enable) {
        uprintf("Mouse report\n");
        uprintf("b:%d ", report->button);
        uprintf("x:%d ", report->x);
        uprintf("y:%d ", report->y);
        uprintf("v:%d ", report->v);
        uprintf("h:%d ", report->h);
        uprintf("undef:%u\n", report->undefined);
    }

    mouse_send_flag = true;

    report_mouse_t mouse = pointing_device_get_report();

    mouse.buttons = report->button;

    mouse.x += report->x;
    mouse.y += report->y;
    mouse.v += report->v;
    mouse.h += report->h;

    pointing_device_set_report(mouse);
}

bool pointing_device_task(void) {
    if (mouse_send_flag) {
        bool send_report = pointing_device_send();
        mouse_send_flag = false;
        return send_report;
    }

    return false;
}

void vendor_report_parser(uint16_t usage_id, hid_report_member_t const* member, uint8_t const* data, uint8_t len) {
    // For Lenovo thinkpad keyboard(17ef:6047)
    // TODO: restriction by VID:PID
    if (usage_id == 0xFFA1) {
        mouse_parse_result_t mouse = {0};
        mouse.h                    = (data[0] & 0x80 ? 0xFF00 : 0) | data[0];
        mouse_report_hook(&mouse);
    }
}

__attribute__((weak)) void system_report_hook(uint16_t report) {
    host_system_send(report);
    wait_ms(TAP_CODE_DELAY);
    host_system_send(0);
}

__attribute__((weak)) void consumer_report_hook(uint16_t report) {
    host_consumer_send(report);
    wait_ms(TAP_CODE_DELAY);
    host_consumer_send(0);
}
