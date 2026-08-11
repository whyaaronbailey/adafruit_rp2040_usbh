// Copyright 2026 whyaaronbailey
// SPDX-License-Identifier: GPL-2.0-or-later
//
// tartarus_rgb — see tartarus_rgb.h. Protocol derived from OpenRazer (GPL-2.0):
// https://github.com/openrazer/openrazer  (razerchromacommon.c, razercommon.c).

#include "tartarus_rgb.h"
#include "adafruit_rp2040_usbh.h" // tartarus_send_iface(), tartarus_led_itfnum()
#include "wait.h"                 // wait_ms for the busy-pipe retry

#include <string.h>

// OpenRazer 90-byte report field offsets (byte 0 = status, byte 1 = transaction
// id, bytes 2-3 remaining, byte 4 protocol, byte 5 data_size, byte 6 class,
// byte 7 command_id, bytes 8..87 arguments, byte 88 crc, byte 89 reserved).
#define RZ_TXID       0x1F // Tartarus V2/Pro transaction id
#define RZ_LEN        90
#define RZ_LED_BACKLT 0x05 // BACKLIGHT_LED
#define RZ_VARSTORE   0x01
#define RZ_NOSTORE    0x00

// Local framebuffer: one colour per physical key.
static uint8_t fb[TARTARUS_RGB_KEYS][3];
static bool    driver_mode_set = false;

static uint8_t rz_crc(const uint8_t *b) {
    uint8_t c = 0;
    for (int i = 2; i < 88; i++) {
        c ^= b[i];
    }
    return c;
}

static void rz_zero(uint8_t *b) {
    memset(b, 0, RZ_LEN);
    b[1] = RZ_TXID;
}

// Send an assembled report to the auto-detected control interface. The control
// pipe admits one transfer at a time; back-to-back commands (e.g. flush's
// set-frame + draw) would otherwise drop the second, so retry while busy.
static int rz_send(uint8_t *b) {
    uint8_t itf = tartarus_led_itfnum();
    if (itf == 0xFF) {
        return 0;
    }
    b[88] = rz_crc(b);
    int ret = 2;
    for (uint8_t tries = 0; tries < 100; tries++) {
        ret = tartarus_send_iface(itf, b, RZ_LEN);
        if (ret != 2) {  // 2 = control pipe busy, anything else is final
            break;
        }
        wait_ms(1);
    }
    // The device firmware needs time to PROCESS a command after the USB
    // transfer completes (OpenRazer sleeps after every control message).
    // Without this, a following command overwrites the internal buffer and
    // the earlier one is silently dropped even though the wire ACKed it.
    if (ret == 3) {
        wait_ms(15);
    }
    return ret;
}

bool tartarus_rgb_ready(void) {
    return tartarus_led_itfnum() != 0xFF;
}

void tartarus_rgb_driver_mode(bool on) {
    uint8_t b[RZ_LEN];
    rz_zero(b);
    // Device mode: class 0x00, cmd 0x04, args[0]=mode (0x03 driver / 0x00 normal)
    b[5] = 0x02;
    b[6] = 0x00;
    b[7] = 0x04;
    b[8] = on ? 0x03 : 0x00;
    b[9] = 0x00;
    rz_send(b);
    driver_mode_set = on;
}

void tartarus_rgb_static(uint8_t r, uint8_t g, uint8_t b_) {
    uint8_t b[RZ_LEN];
    rz_zero(b);
    // extended matrix static: class 0x0F cmd 0x02, data_size 0x09
    b[5]  = 0x09;
    b[6]  = 0x0F;
    b[7]  = 0x02;
    b[8]  = RZ_VARSTORE;   // args[0]
    b[9]  = RZ_LED_BACKLT; // args[1]
    b[10] = 0x01;          // args[2] effect id = STATIC
    b[13] = 0x01;          // args[5]
    b[14] = r;             // args[6]
    b[15] = g;             // args[7]
    b[16] = b_;            // args[8]
    rz_send(b);
}

void tartarus_rgb_brightness(uint8_t value) {
    uint8_t b[RZ_LEN];
    rz_zero(b);
    // extended matrix brightness: class 0x0F cmd 0x04, data_size 0x03
    b[5]  = 0x03;
    b[6]  = 0x0F;
    b[7]  = 0x04;
    b[8]  = RZ_VARSTORE;
    b[9]  = RZ_LED_BACKLT;
    b[10] = value;
    rz_send(b);
}

void tartarus_rgb_effect_none(void) {
    uint8_t b[RZ_LEN];
    rz_zero(b);
    b[5]  = 0x06;
    b[6]  = 0x0F;
    b[7]  = 0x02;
    b[8]  = RZ_VARSTORE;
    b[9]  = RZ_LED_BACKLT;
    b[10] = 0x00; // effect NONE
    rz_send(b);
}

// Shared preamble for extended-matrix effect commands (class 0x0F cmd 0x02).
// args[0]=VARSTORE args[1]=led args[2]=effect id; caller fills args[3..].
static void rz_effect_base(uint8_t *b, uint8_t arg_size, uint8_t effect_id) {
    rz_zero(b);
    b[5]  = arg_size;
    b[6]  = 0x0F;
    b[7]  = 0x02;
    b[8]  = RZ_VARSTORE;
    b[9]  = RZ_LED_BACKLT;
    b[10] = effect_id;
}

void tartarus_rgb_effect_spectrum(void) {
    uint8_t b[RZ_LEN];
    rz_effect_base(b, 0x06, 0x03); // SPECTRUM
    rz_send(b);
}

void tartarus_rgb_effect_breathing(uint8_t r, uint8_t g, uint8_t b_) {
    uint8_t b[RZ_LEN];
    rz_effect_base(b, 0x09, 0x02); // BREATHING, 1 colour
    b[11] = 0x01; // args[3] colour count
    b[13] = 0x01; // args[5] colour count (repeated per protocol)
    b[14] = r;
    b[15] = g;
    b[16] = b_;
    rz_send(b);
}

void tartarus_rgb_effect_breathing_dual(uint8_t r1, uint8_t g1, uint8_t b1,
                                        uint8_t r2, uint8_t g2, uint8_t b2) {
    uint8_t b[RZ_LEN];
    rz_effect_base(b, 0x0C, 0x02); // BREATHING, 2 colours
    b[11] = 0x02; // args[3]
    b[13] = 0x02; // args[5]
    b[14] = r1;
    b[15] = g1;
    b[16] = b1;
    b[17] = r2;
    b[18] = g2;
    b[19] = b2;
    rz_send(b);
}

void tartarus_rgb_effect_breathing_random(void) {
    uint8_t b[RZ_LEN];
    rz_effect_base(b, 0x06, 0x02); // BREATHING, random colours
    rz_send(b);
}

void tartarus_rgb_effect_wave(uint8_t direction) {
    uint8_t b[RZ_LEN];
    rz_effect_base(b, 0x06, 0x04); // WAVE
    b[11] = direction ? 0x01 : 0x00; // args[3] direction
    b[12] = 0x28;                    // args[4] speed (lower = faster)
    rz_send(b);
}

void tartarus_rgb_effect_starlight(uint8_t speed, uint8_t r, uint8_t g, uint8_t b_) {
    uint8_t b[RZ_LEN];
    if (speed < 1 || speed > 3) speed = 2;
    rz_effect_base(b, 0x09, 0x07); // STARLIGHT, 1 colour
    b[12] = speed; // args[4]
    b[13] = 0x01;  // args[5] colour count
    b[14] = r;
    b[15] = g;
    b[16] = b_;
    rz_send(b);
}

void tartarus_rgb_effect_starlight_dual(uint8_t speed,
                                        uint8_t r1, uint8_t g1, uint8_t b1,
                                        uint8_t r2, uint8_t g2, uint8_t b2) {
    uint8_t b[RZ_LEN];
    if (speed < 1 || speed > 3) speed = 2;
    rz_effect_base(b, 0x0C, 0x07); // STARLIGHT, 2 colours
    b[12] = speed;
    b[13] = 0x02;
    b[14] = r1;
    b[15] = g1;
    b[16] = b1;
    b[17] = r2;
    b[18] = g2;
    b[19] = b2;
    rz_send(b);
}

void tartarus_rgb_effect_starlight_random(uint8_t speed) {
    uint8_t b[RZ_LEN];
    if (speed < 1 || speed > 3) speed = 2;
    rz_effect_base(b, 0x06, 0x07); // STARLIGHT, random colours
    b[12] = speed;
    rz_send(b);
}

void tartarus_rgb_effect_reactive(uint8_t speed, uint8_t r, uint8_t g, uint8_t b_) {
    uint8_t b[RZ_LEN];
    if (speed < 1 || speed > 4) speed = 2;
    rz_effect_base(b, 0x09, 0x05); // REACTIVE
    b[12] = speed; // args[4]
    b[13] = 0x01;  // args[5] colour count
    b[14] = r;
    b[15] = g;
    b[16] = b_;
    rz_send(b);
}

void tartarus_rgb_effect_wheel(uint8_t direction) {
    uint8_t b[RZ_LEN];
    if (direction < 1 || direction > 2) direction = 1;
    rz_effect_base(b, 0x06, 0x0A); // WHEEL
    b[11] = direction; // args[3] (0 is ignored by the device)
    b[12] = 0x28;      // args[4] speed
    rz_send(b);
}

void tartarus_rgb_set_all(uint8_t r, uint8_t g, uint8_t b_) {
    for (uint8_t k = 0; k < TARTARUS_RGB_KEYS; k++) {
        fb[k][0] = r;
        fb[k][1] = g;
        fb[k][2] = b_;
    }
}

void tartarus_rgb_set_key(uint8_t key, uint8_t r, uint8_t g, uint8_t b_) {
    if (key >= TARTARUS_RGB_KEYS) {
        return;
    }
    fb[key][0] = r;
    fb[key][1] = g;
    fb[key][2] = b_;
}

void tartarus_rgb_flush(void) {
    uint8_t b[RZ_LEN];

    // Custom frames work in normal device mode. Never enter driver mode
    // here: it stops the Tartarus sending key reports.

    // 1) Load the custom frame: row 0, columns 0..KEYS-1.
    // extended set_custom_frame: class 0x0F cmd 0x03, data_size 0x47,
    // args[2]=row, args[3]=start_col, args[4]=stop_col, args[5..]=RGB per col.
    rz_zero(b);
    b[5]  = 0x47;
    b[6]  = 0x0F;
    b[7]  = 0x03;
    b[10] = 0x00;                    // row 0
    b[11] = 0x00;                    // start col
    b[12] = TARTARUS_RGB_KEYS - 1;   // stop col
    for (uint8_t k = 0; k < TARTARUS_RGB_KEYS; k++) {
        uint8_t o = 13 + k * 3;      // args[5] == byte 13
        b[o]     = fb[k][0];
        b[o + 1] = fb[k][1];
        b[o + 2] = fb[k][2];
    }
    rz_send(b);

    // 2) Draw/commit the custom frame (extended custom effect id 0x08).
    rz_zero(b);
    b[5]  = 0x0C;
    b[6]  = 0x0F;
    b[7]  = 0x02;
    b[8]  = 0x00;
    b[9]  = 0x00;
    b[10] = 0x08;
    rz_send(b);
}
