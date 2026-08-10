#include QMK_KEYBOARD_H
#include "quantum.h"
#include "joystick.h"
#include "powermic.h"
#include "tartarus_rgb.h"

// --- Host-driven Tartarus control via RAW HID -------------------------------
// Keyboard input/output injection is blocked from the dev box, so the only
// reliable host->firmware channel is the RAW HID (vendor) interface. It carries
// both the reflash trigger and a relay for arbitrary Razer commands, so the
// whole LED protocol can be probed from the PC with no reflashing:
//   [0xB0]                      -> jump to UF2 bootloader (host reflash)
//   [0xB1, offset, <=30 bytes]  -> load bytes into the 90-byte staging buffer
//   [0xB2, instance]            -> send the staged 90 bytes to that HID instance
// The host builds the full 90-byte Razer report (including CRC), loads it in
// three chunks (offsets 0/30/60), then fires it at a chosen interface.
#include "raw_hid.h"

#define RZ_TXID     0x1F   // Tartarus transaction id

static uint8_t rz_crc(const uint8_t *b) {
    uint8_t c = 0;
    for (int i = 2; i < 88; i++) c ^= b[i];
    return c;
}
// zero a 90-byte report and set the fixed header (status/txid)
static void rz_zero(uint8_t *b) {
    for (int i = 0; i < 90; i++) b[i] = 0;
    b[1] = RZ_TXID;
}
static uint8_t rz_probe_instance = 0xFF;  // 0xFF = use dynamic detection
static uint8_t rz_iface          = 0xFF;  // 0xFF = send by HID instance; else wIndex
volatile uint8_t rz_last_ret     = 0xEE;  // return code of the last relay send

static uint8_t rz_target(void) {
    return (rz_probe_instance != 0xFF) ? rz_probe_instance : tartarus_led_instance();
}
static void rz_send(uint8_t *b) {
    b[88] = rz_crc(b);
    if (rz_iface != 0xFF) {
        // Send to an explicit USB interface number, as OpenRazer does.
        rz_last_ret = (uint8_t)tartarus_send_iface(rz_iface, b, 90);
    } else {
        rz_last_ret = (uint8_t)tartarus_send_to(rz_target(), b, 90);
    }
}

static void led_set_idle_fx(uint8_t idx);  // defined with the LED state block below
static uint32_t held_mask = 0;             // bit n = LED n's key is physically down

// Each opcode assembles a full Razer command from ONE 32-byte RAW HID report,
// so nothing depends on multi-report reassembly (which drops). The target HID
// instance is either the firmware's auto-detected control interface, or an
// explicit override set with 0xCF (used while probing which instance is right).
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length < 1) return;
    uint8_t b[90];
    switch (data[0]) {
        case 0xB0:  // reflash: drop to the UF2 bootloader
            bootloader_jump();
            break;

        case 0xCF:  // override target instance: [0xCF, instance] (0xFF = auto)
            rz_probe_instance = data[1];
            break;

        case 0xCE:  // override target INTERFACE number (wIndex): [0xCE, itf] (0xFF = off)
            rz_iface = data[1];
            break;

        case 0xCA:  // driver: static colour [0xCA, r, g, b] (auto interface)
            tartarus_rgb_static(data[1], data[2], data[3]);
            break;

        case 0xCB:  // driver: breathing effect [0xCB, r, g, b] (auto interface)
            tartarus_rgb_effect_breathing(data[1], data[2], data[3]);
            break;

        case 0xCC:  // driver: select idle effect [0xCC, index] (bench = same path as FX keys)
            led_set_idle_fx(data[1]);
            break;

        case 0xCD:  // bench: simulate key hold [0xCD, led, down] -> drives held_mask
            if (data[1] < TARTARUS_RGB_KEYS) {
                if (data[2]) {
                    held_mask |= (1UL << data[1]);
                } else {
                    held_mask &= ~(1UL << data[1]);
                }
            }
            break;

        case 0xD0: {  // probe: send driver-mode to instance data[1]; completion tracked async
            razer_cb_fired = 0;
            razer_cb_len   = 0;
            uint8_t save = rz_probe_instance;
            rz_probe_instance = data[1];
            rz_zero(b);
            b[5] = 0x02; b[6] = 0x00; b[7] = 0x04; b[8] = 0x03; b[9] = 0x00;
            rz_send(b);
            rz_probe_instance = save;
            break;
        }

        case 0xD1: {  // report last probe result + topology over RAW HID input
            uint8_t resp[32] = {0};
            resp[0] = 0xD1;
            resp[1] = razer_cb_fired;                 // did a FEATURE xfer complete
            resp[2] = (uint8_t)(razer_cb_len & 0xFF); // bytes it reported (90 = full)
            resp[3] = (uint8_t)(razer_cb_len >> 8);
            resp[4] = tartarus_instance_count();       // how many HID instances
            resp[5] = tartarus_led_instance();         // firmware's auto pick
            resp[6] = rz_last_ret;                     // last relay send result
            resp[7] = tartarus_led_itfnum();           // auto-detected control interface #
            resp[8] = (uint8_t)(razer_cb_count & 0xFF); // completed sends (wraps)
            resp[9] = (uint8_t)(razer_cb_count >> 8);
            raw_hid_send(resp, sizeof(resp));
            break;
        }

        case 0xD2: {  // report per-instance {proto, itf_num, desclen} for 0..6
            uint8_t resp[32] = {0};
            resp[0] = 0xD2;
            resp[1] = tartarus_instance_count();
            for (uint8_t i = 0; i < 6; i++) {
                resp[2 + i * 4] = razer_inst_proto[i];
                resp[3 + i * 4] = tartarus_inst_itfnum(i);
                resp[4 + i * 4] = (uint8_t)(razer_inst_desclen[i] & 0xFF);
                resp[5 + i * 4] = (uint8_t)(razer_inst_desclen[i] >> 8);
            }
            raw_hid_send(resp, sizeof(resp));
            break;
        }

        case 0xC4:  // set device mode: [0xC4, mode]   (0x03 = driver mode)
            rz_zero(b);
            b[5] = 0x02; b[6] = 0x00; b[7] = 0x04; b[8] = data[1]; b[9] = 0x00;
            rz_send(b);
            break;

        case 0xC1:  // static whole-device colour: [0xC1, r, g, b]
            rz_zero(b);
            b[5] = 0x09; b[6] = 0x0F; b[7] = 0x02;
            b[8] = 0x01; b[9] = 0x05; b[10] = 0x01; b[13] = 0x01;
            b[14] = data[1]; b[15] = data[2]; b[16] = data[3];
            rz_send(b);
            break;

        case 0xC2:  // native effect: [0xC2, effect_id, arg0, arg1]
            rz_zero(b);
            b[5] = 0x06; b[6] = 0x0F; b[7] = 0x02;
            b[8] = 0x01; b[9] = 0x05; b[10] = data[1]; b[11] = data[2]; b[12] = data[3];
            rz_send(b);
            break;

        case 0xC3:  // draw/commit the custom framebuffer (extended: effect id 0x08)
            // razer_chroma_extended_matrix_effect_custom_frame:
            // base(arg_size=0x0C, var=0x00, led=0x00, effect=0x08) -> class 0F cmd 02
            rz_zero(b);
            b[5] = 0x0C; b[6] = 0x0F; b[7] = 0x02;
            b[8] = 0x00; b[9] = 0x00; b[10] = 0x08;
            rz_send(b);
            break;

        case 0xC6:  // set brightness: [0xC6, led_id, value]  (class 0F cmd 04)
            rz_zero(b);
            b[5] = 0x03; b[6] = 0x0F; b[7] = 0x04;
            b[8] = 0x01; b[9] = data[1]; b[10] = data[2];
            rz_send(b);
            break;

        case 0xC7:  // reset to "none" effect: [0xC7, led_id]  (effect 0x00)
            rz_zero(b);
            b[5] = 0x06; b[6] = 0x0F; b[7] = 0x02;
            b[8] = 0x01; b[9] = data[1]; b[10] = 0x00;
            rz_send(b);
            break;

        case 0xC0: { // custom-frame solid span: [0xC0, row, start_col, stop_col, r, g, b]
            rz_zero(b);
            b[5] = 0x47; b[6] = 0x0F; b[7] = 0x03;
            b[10] = data[1]; b[11] = data[2]; b[12] = data[3];
            for (uint8_t col = data[2]; col <= data[3] && col < 20; col++) {
                uint8_t o = 13 + (col - data[2]) * 3;
                b[o] = data[4]; b[o + 1] = data[5]; b[o + 2] = data[6];
            }
            rz_send(b);
            break;
        }

        case 0xC5: { // custom-frame single key: [0xC5, row, col, r, g, b]
            rz_zero(b);
            b[5] = 0x47; b[6] = 0x0F; b[7] = 0x03;
            b[10] = data[1]; b[11] = data[2]; b[12] = data[2];
            b[13] = data[3]; b[14] = data[4]; b[15] = data[5];
            rz_send(b);
            break;
        }

        default:
            break;
    }
}

enum custom_keycodes {
    COPYACC = SAFE_RANGE,
    OPENGE,
    OPENEPIC,
    OPENMCKESSON,
    DICTATE,
    WL_SOFT,
    WL_BONE,
    WL_BRAIN,
    WL_STROKE,
    WL_LUNG,
    WL_VASCULAR,
    WL_SUBDURAL,
    WL_HARDWARE,
    ARROW,
    ZOOM,
    MEASURE,
    SCROLLUP,
    SCROLLDOWN,
    SPINE_C,
    SPINE_T,
    SPINE_L,
    ANNOTATION,
    FAST_UP,
    FAST_DOWN,
    ELLIPSE,
    ROI,
    INTERZOOM,
    HANG,
    FX_NEXT,   // cycle idle LED effect forward
    FX_PREV    // cycle idle LED effect back
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
            /*┌────────────┬────────────┬────────────┬────────────┬────────────┬ */
            /*│    01      │    02      │    03      │    04      │    05      │ */
                ANNOTATION,   WL_SOFT,     WL_LUNG,   WL_VASCULAR,    ZOOM,      
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    06      │    07      │    08      │    09      │    10      │*/  
                OPENEPIC,     WL_BONE,     SCROLLUP,   WL_BRAIN,    ARROW,       
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    11      │    12      │    13      │    14      │    15      │*/ 
                COPYACC,      FAST_UP,    SCROLLDOWN,  FAST_DOWN,   MEASURE,
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/  
            /*│    16      │    17      │    18      │    19      │*/ 
                  OPENGE,     SPINE_C,     SPINE_T,     SPINE_L,    
            /*├────────────┼────────────┼────────────┼────────────┼*/  
            /*│    SW      │    SC      │    THUMB   │     20     │*/
                  SCROLLUP,   SCROLLDOWN,   MO(1),      DICTATE,      
            /*├────────────┼────────────┼────────────┼────────────┼*/  
            /*│    LEFT    │    RIGHT        UP      │    DOWN   │ */
                KC_MS_LEFT, KC_MS_RIGHT,   KC_MS_UP,   KC_MS_DOWN
            /*└────────────┴────────────┴────────────┴────────────┘*/
    ),
    
    [1] = LAYOUT_tartarus2(
            /*┌────────────┬────────────┬────────────┬────────────┬────────────┬ */
            /*│    01      │    02      │    03      │    04      │    05      │ */
                  KC_H,        KC_TRNS,     KC_TRNS,     KC_TRNS,   INTERZOOM,      
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    06      │    07      │    08      │    09      │    10      │*/  
                  KC_P,     WL_HARDWARE, KC_MS_WH_UP,  WL_STROKE,    ELLIPSE,       
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/
            /*│    11      │    12      │    13      │    14      │    15      │*/ 
                  KC_R,     KC_MS_WH_UP, KC_MS_WH_DOWN, KC_MS_WH_DOWN,     ROI, 
            /*├────────────┼────────────┼────────────┼────────────┼────────────┼*/  
            /*│    16      │    17      │    18      │    19      │*/
               OPENMCKESSON,   KC_TRNS,     FX_PREV,     FX_NEXT,
            /*├────────────┼────────────┼────────────┼────────────┼*/  
            /*│    SW      │    SC      │    THUMB      │    20   │*/
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,      
            /*├────────────┼────────────┼────────────┼────────────┼*/  
            /*│    LEFT    │    RIGHT   │    UP       │    DOWN   │ */
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS
            /*└────────────┴────────────┴────────────┴────────────┘*/
        
    )
};

static uint16_t REP_DELAY = 300; 
static uint16_t REP_DELAY_FAST = 140; 

static uint32_t wh_callback(uint32_t trigger_time, void* cb_arg) {
    bool is_up = (bool)cb_arg;
    if (is_up) {
        tap_code(KC_MS_WH_UP);
    } else {
        tap_code(KC_MS_WH_DOWN);
    }
    return REP_DELAY;
};

static uint32_t wh_callback_fast(uint32_t trigger_time, void* cb_arg) {
    bool is_up = (bool)cb_arg;
    if (is_up) {
        tap_code(KC_MS_WH_UP);
    } else {
        tap_code(KC_MS_WH_DOWN);
    }
    return REP_DELAY_FAST;
};

// --- Tartarus status LEDs (example consumer of the tartarus_rgb driver) -----
// Idle -> static light blue. Dictating -> static red. Continuous scroll active
// -> the device's own breathing effect in blue (a gentle, non-obtrusive pulse).
// All three are stored/native effects, so they need NO "driver mode" handshake
// and therefore never suppress the Tartarus's own key reports (which this
// converter depends on). Per-key custom frames are available in the driver but
// deliberately unused here for that reason.
static deferred_token token = INVALID_DEFERRED_TOKEN;  // continuous-scroll timer
static bool           dictating = false;               // toggled by DICTATE

typedef enum { LST_NONE, LST_IDLE, LST_DICT, LST_SCROLL } led_state_t;

// --- Idle effect table: every Synapse pattern the hardware runs natively ----
// The IDLE state applies whichever entry is selected; FX_NEXT/FX_PREV on layer
// 1 (or RAW HID 0xCC) cycle it live. Edit colours/speeds here to taste — this
// is the "settable from keymap.c" surface.
static void fx_static_blue(void)   { tartarus_rgb_static(0x00, 0x40, 0xFF); }
static void fx_spectrum(void)      { tartarus_rgb_effect_spectrum(); }
static void fx_wave(void)          { tartarus_rgb_effect_wave(1); }
static void fx_wheel(void)         { tartarus_rgb_effect_wheel(1); }
static void fx_breathe_blue(void)  { tartarus_rgb_effect_breathing(0x00, 0x40, 0xFF); }
static void fx_breathe_dual(void)  { tartarus_rgb_effect_breathing_dual(0x00, 0x40, 0xFF, 0x80, 0x00, 0xFF); }
static void fx_breathe_rand(void)  { tartarus_rgb_effect_breathing_random(); }
static void fx_star_blue(void)     { tartarus_rgb_effect_starlight(2, 0x00, 0x40, 0xFF); }
static void fx_star_dual(void)     { tartarus_rgb_effect_starlight_dual(2, 0x00, 0x40, 0xFF, 0xFF, 0xFF, 0xFF); }
static void fx_star_rand(void)     { tartarus_rgb_effect_starlight_random(2); }
static void fx_reactive_cyan(void) { tartarus_rgb_effect_reactive(2, 0x00, 0xFF, 0xFF); }

typedef void (*idle_fx_fn)(void);
static const idle_fx_fn idle_fx_table[] = {
    fx_static_blue,    // 0  static light blue (default)
    fx_spectrum,       // 1  spectrum cycle
    fx_wave,           // 2  rainbow wave
    fx_wheel,          // 3  colour wheel
    fx_breathe_blue,   // 4  breathing, blue
    fx_breathe_dual,   // 5  breathing, blue<->purple
    fx_breathe_rand,   // 6  breathing, random colours
    fx_star_blue,      // 7  starlight, blue twinkle
    fx_star_dual,      // 8  starlight, blue+white twinkle
    fx_star_rand,      // 9  starlight, random colours
    fx_reactive_cyan,  // 10 reactive: pressed key flashes cyan
};
#define IDLE_FX_COUNT (sizeof(idle_fx_table) / sizeof(idle_fx_table[0]))

static uint8_t     idle_fx     = 0;         // selected idle effect index
static led_state_t led_applied = LST_NONE;  // last state actually sent

// --- Per-key reactive frame mode (idle_fx == 0, the default) ----------------
// Baseline all light blue; a physically held key turns red and reverts on
// release; while continuous scroll runs, the key that started it pulses
// blue<->red (fast scroll = fast pulse, slow = slow). DICTATE overrides
// everything with all-red. Implemented with the driver's per-key framebuffer.

#define LED_BASE_R 0x00
#define LED_BASE_G 0x40
#define LED_BASE_B 0xFF

// matrix (row,col) -> LED index 0..19, from LAYOUT_tartarus2 in info.json.
typedef struct { uint8_t row, col, led; } led_map_t;
static const led_map_t led_map[] = {
    {3, 6, 0},  {3, 7, 1},  {4, 0, 2},  {4, 1, 3},  {4, 2, 4},   // keys 01-05
    {5, 3, 5},  {2, 4, 6},  {3, 2, 7},  {1, 0, 8},  {2, 5, 9},   // keys 06-10
    {7, 1, 10}, {0, 4, 11}, {2, 6, 12}, {0, 7, 13}, {1, 1, 14},  // keys 11-15
    {28, 1, 15}, {3, 5, 16}, {3, 3, 17}, {0, 6, 18},             // keys 16-19
    {5, 4, 19},                                                  // key 20 (space/thumb)
};

static uint8_t led_from_key(keypos_t key) {
    for (uint8_t i = 0; i < sizeof(led_map) / sizeof(led_map[0]); i++) {
        if (led_map[i].row == key.row && led_map[i].col == key.col) {
            return led_map[i].led;
        }
    }
    return 0xFF;  // key has no LED (scroll wheel, dpad, ...)
}

static uint8_t  scroll_led   = 0xFF;  // LED of the key that started the scroll
static uint16_t scroll_half  = 300;   // pulse half-period ms (fast scroll = shorter)
static uint8_t  frame_shadow[TARTARUS_RGB_KEYS][3];  // last frame sent
static bool     frame_valid  = false; // shadow holds what the device shows

static void led_set_idle_fx(uint8_t idx) {
    idle_fx     = idx % IDLE_FX_COUNT;
    led_applied = LST_NONE;  // force led_apply() to resend
    frame_valid = false;     // the device no longer shows our last frame
}

static void led_render_frame(void) {
    bool    scrolling = (token != INVALID_DEFERRED_TOKEN);
    bool    pulse_red = scrolling && ((timer_read32() / scroll_half) & 1);
    uint8_t want[TARTARUS_RGB_KEYS][3];

    for (uint8_t k = 0; k < TARTARUS_RGB_KEYS; k++) {
        bool red = (held_mask >> k) & 1;
        if (scrolling && k == scroll_led) {
            red = pulse_red;
        }
        want[k][0] = red ? 0xFF : LED_BASE_R;
        want[k][1] = red ? 0x00 : LED_BASE_G;
        want[k][2] = red ? 0x00 : LED_BASE_B;
    }

    if (frame_valid && memcmp(want, frame_shadow, sizeof(want)) == 0) {
        return;  // nothing changed since the last flush
    }
    memcpy(frame_shadow, want, sizeof(want));
    frame_valid = true;

    for (uint8_t k = 0; k < TARTARUS_RGB_KEYS; k++) {
        tartarus_rgb_set_key(k, want[k][0], want[k][1], want[k][2]);
    }
    tartarus_rgb_flush();
}

static void led_apply(void) {
    static bool was_ready = false;

    bool ready = tartarus_rgb_ready();
    if (ready && !was_ready) {
        led_applied = LST_NONE;  // device (re)appeared: resend current state
        frame_valid = false;
    }
    was_ready = ready;
    if (!ready) {
        led_applied = LST_NONE;
        frame_valid = false;
        return;
    }

    // DICTATE overrides everything: all keys red.
    if (dictating) {
        if (led_applied != LST_DICT) {
            led_applied = LST_DICT;
            frame_valid = false;
            tartarus_rgb_static(0xFF, 0x00, 0x00);
        }
        return;
    }

    // Default mode: per-key reactive frames (continuous, self-diffing).
    if (idle_fx == 0) {
        led_applied = LST_IDLE;
        led_render_frame();
        return;
    }

    // A native idle effect is selected: edge-triggered, scroll pulses whole pad.
    bool        scrolling = (token != INVALID_DEFERRED_TOKEN);
    led_state_t want      = scrolling ? LST_SCROLL : LST_IDLE;
    if (want == led_applied) {
        return;
    }
    led_applied = want;
    frame_valid = false;
    if (want == LST_SCROLL) {
        tartarus_rgb_effect_breathing(0x00, 0x50, 0xFF);
    } else {
        idle_fx_table[idle_fx]();
    }
}

void housekeeping_task_user(void) {
    led_apply();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    // Track physical key state for the reactive frame mode: a held key shows
    // red, reverting on release. Applies to every key that has an LED.
    {
        uint8_t led = led_from_key(record->event.key);
        if (led != 0xFF) {
            if (record->event.pressed) {
                held_mask |= (1UL << led);
            } else {
                held_mask &= ~(1UL << led);
            }
        }
    }

    switch (keycode) {

		case DICTATE:
			// Was SS_TAP(X_F13): a keyboard scan code, so it only reached
			// PowerScribe when PowerScribe had focus, which is what forced the
			// AHK WinActivate helper. Now emits the PowerMic's own Button-page
			// report instead, which Raw Input delivers regardless of focus.
			if (record->event.pressed) {
				powermic_button_press(PM_DICTATE);
				dictating = !dictating;   // toggle dictation LED state
			} else {
				powermic_button_release(PM_DICTATE);
			}
		return false;

        case FX_NEXT:
            if (record->event.pressed) {
                led_set_idle_fx((uint8_t)(idle_fx + 1));
            }
            return false;

        case FX_PREV:
            if (record->event.pressed) {
                led_set_idle_fx((uint8_t)(idle_fx + IDLE_FX_COUNT - 1));
            }
            return false;

        case SCROLLUP:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token      = INVALID_DEFERRED_TOKEN;
                    scroll_led = 0xFF;
                } else {
                    tap_code(KC_MS_WH_UP);
                    token       = defer_exec(REP_DELAY, wh_callback, (void*)true);
                    scroll_led  = led_from_key(record->event.key);
                    scroll_half = REP_DELAY;      // slow scroll -> slow pulse
                }
            }
            return false;

        case SCROLLDOWN:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token      = INVALID_DEFERRED_TOKEN;
                    scroll_led = 0xFF;
                } else {
                    tap_code(KC_MS_WH_DOWN);
                    token       = defer_exec(REP_DELAY, wh_callback, (void*)false);
                    scroll_led  = led_from_key(record->event.key);
                    scroll_half = REP_DELAY;      // slow scroll -> slow pulse
                }
            }
            return false;

        case FAST_UP:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token      = INVALID_DEFERRED_TOKEN;
                    scroll_led = 0xFF;
                } else {
                    tap_code(KC_MS_WH_UP);
                    token       = defer_exec(REP_DELAY_FAST, wh_callback_fast, (void*)true);
                    scroll_led  = led_from_key(record->event.key);
                    scroll_half = REP_DELAY_FAST; // fast scroll -> fast pulse
                }
            }
            return false;

        case FAST_DOWN:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token      = INVALID_DEFERRED_TOKEN;
                    scroll_led = 0xFF;
                } else {
                    tap_code(KC_MS_WH_DOWN);
                    token       = defer_exec(REP_DELAY_FAST, wh_callback_fast, (void*)false);
                    scroll_led  = led_from_key(record->event.key);
                    scroll_half = REP_DELAY_FAST; // fast scroll -> fast pulse
                }
            }
            return false;

        case COPYACC: 
            if (record->event.pressed) {
                SEND_STRING(
                    SS_LSFT("`")
                    SS_LSFT("`")
                    SS_TAP(X_ENTER)
                    SS_DELAY(100)
                    SS_LSFT(SS_LCTL(SS_TAP(X_LEFT)))
                    SS_LCTL("c")
                    SS_LSFT(SS_TAP(X_HOME))
                    SS_TAP(X_DEL)
                    SS_TAP(X_DEL)
                );
            }
            return false;

        case OPENGE: 
            if (record->event.pressed) {
                SEND_STRING(
                    SS_LSFT(SS_TAP(X_TAB))
                    SS_TAP(X_END)
                    SS_DELAY(75)
                    SS_TAP(X_TAB)
                    SS_LCTL("v")
                    SS_DELAY(150)
                    SS_TAP(X_ENTER)
                );
            }
            return false;

        case OPENEPIC: 
            if (record->event.pressed) {
                SEND_STRING(
                    SS_LCTL("2")
                    SS_DELAY(2500)
                    SS_LCTL("v")
                    SS_DELAY(500)
                    SS_TAP(X_ENTER)
                    SS_DELAY(2500)
                    SS_LALT(SS_LSFT("a"))
                );
            }
            return false;

        case OPENMCKESSON: 
            if (record->event.pressed) {
                SEND_STRING(
                    SS_LCTL("f")
                    SS_DELAY(500)
                    SS_TAP(X_BSPC)
                    SS_LCTL("v")
                    SS_TAP(X_ENTER)
                    SS_DELAY(500)
                    SS_TAP(X_ENTER)
                );
            }
            return false;

		case WL_SOFT:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_1)
				);
			}
		return false;

		case WL_BONE:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_2)
				);
			}
		return false;

		case WL_BRAIN:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_3)
				);
			}
		return false;

		case WL_STROKE:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_4)
				);
			}
		return false;

		case WL_LUNG:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_5)
				);
			}
		return false;

		case WL_HARDWARE:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_6)
				);
			}
		return false;

		case WL_VASCULAR:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_7)
				);
			}
		return false;

		case WL_SUBDURAL:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_KP_8)
				);
			}
		return false;

		case ARROW:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_A)
				);
			}
		return false;

		case ELLIPSE:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_E)
				);
			}
		return false;

		case MEASURE:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_M)
				);
			}
		return false;

		case ROI:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_LSFT(SS_TAP(X_E))
				);
			}
		return false;

		case ZOOM:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_Z)
				);
			}
		return false;

        case INTERZOOM:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_LSFT(SS_TAP(X_Z))
				);
			}
		return false;

 		case ANNOTATION:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_Y)
				);
			}
		return false;

 		case HANG:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_H)
				);
			}
		return false;

		case SPINE_C:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_C)
				);
			}
		return false;

		case SPINE_T:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_T)
				);
			}
		return false;

		case SPINE_L:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_BTN1)
					SS_DELAY(50)
					SS_TAP(X_L)
				);
			}
		return false;

    }
    return true; 
}