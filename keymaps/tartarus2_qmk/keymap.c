#include QMK_KEYBOARD_H
#include "quantum.h"
#include "joystick.h"
#include "powermic.h"
#include "tartarus_rgb.h"
#include "via.h"
#include "color.h"
#include "c1.h"
#include "dynamic_keymap.h"

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
static volatile uint16_t key_event_count = 0;  // QMK key events since boot (diag)
static void led_force_reapply(void);       // resend current LED state to the device

// Host-driven LED state (AHK model): scrolling logic lives in AutoHotkey, so
// the firmware cannot infer it. The host mirrors its state via RAW HID opcodes
// 0xC8 (dictate) / 0xC9 (scroll pulse); see tartarus_led.ahk.
static bool     host_scroll = false;
static bool     dictating;    // defined with an initializer in the LED block
static uint8_t  scroll_led;   // defined with an initializer in the LED block
static uint16_t scroll_half;  // defined with an initializer in the LED block

// User-tunable colours (VIA "Lighting" menu writes these; HSV keeps VIA's
// colour picker round-trippable, RGB is what the wire wants).
static HSV     base_hsv   = {152, 255, 255};  // idle baseline: light blue
static HSV     dict_hsv   = {0, 255, 255};    // dictating: red
static HSV     scroll_hsv = {0, 255, 255};    // scroll pulse: red
static uint8_t led_bright = 255;              // device brightness

// Each opcode assembles a full Razer command from ONE 32-byte RAW HID report,
// so nothing depends on multi-report reassembly (which drops). The target HID
// instance is either the firmware's auto-detected control interface, or an
// explicit override set with 0xCF (used while probing which instance is right).
//
// With VIA enabled, VIA owns raw_hid_receive; the bench opcodes live at
// 0xB0..0xDF, far above VIA's command ids, and are intercepted here. Returning
// true tells VIA the report was fully handled.
bool via_command_kb(uint8_t *data, uint8_t length) {
    if (length < 1 || data[0] < 0xB0) {
        return false;  // not ours: let VIA process it
    }
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

        case 0xC8:  // host LED state: dictate [0xC8, on] -> all-red while on
            dictating = data[1] != 0;
            led_force_reapply();
            break;

        case 0xC9:  // host LED state: scroll pulse [0xC9, on, led, half_ms/10]
            if (data[1]) {
                host_scroll = true;
                scroll_led  = (data[2] < TARTARUS_RGB_KEYS) ? data[2] : 0xFF;
                if (data[3]) {
                    scroll_half = data[3] * 10;
                }
            } else {
                host_scroll = false;
                scroll_led  = 0xFF;
            }
            led_force_reapply();
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

        case 0xD3: {  // core-1 liveness: [0xD3] -> [0xD3, ready, heartbeat u32]
            uint8_t  resp[32] = {0};
            uint32_t hb       = c1_heartbeat;
            resp[0] = 0xD3;
            resp[1] = c1_ready;
            resp[2] = (uint8_t)(hb & 0xFF);
            resp[3] = (uint8_t)(hb >> 8);
            resp[4] = (uint8_t)(hb >> 16);
            resp[5] = (uint8_t)(hb >> 24);
            raw_hid_send(resp, sizeof(resp));
            break;
        }

        case 0xD4: {  // input-path diag: [rx_all u16, rx_kbd u16, qmk_events u16, last_kbd 4B, any_down]
            uint8_t resp[32] = {0};
            resp[0] = 0xD4;
            resp[1] = (uint8_t)(diag_rx_all & 0xFF);  resp[2] = (uint8_t)(diag_rx_all >> 8);
            resp[3] = (uint8_t)(diag_rx_kbd & 0xFF);  resp[4] = (uint8_t)(diag_rx_kbd >> 8);
            resp[5] = (uint8_t)(key_event_count & 0xFF); resp[6] = (uint8_t)(key_event_count >> 8);
            resp[7] = diag_last_kbd[0]; resp[8] = diag_last_kbd[1];
            resp[9] = diag_last_kbd[2]; resp[10] = diag_last_kbd[3];
            uint8_t any = 0;
            for (uint8_t r = 0; r < MATRIX_ROWS; r++) { if (matrix_get_row(r)) any = 1; }
            resp[11] = any;
            raw_hid_send(resp, sizeof(resp));
            break;
        }

        case 0xD5: {  // self-type: [0xD5, kc_lo, kc_hi] -> firmware taps the keycode
            uint16_t kc = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
            tap_code16(kc);
            break;
        }

        case 0xD6:  // PowerMic bench: [0xD6, button, down] -> press/release button
            if (data[2]) {
                powermic_button_press(data[1]);
            } else {
                powermic_button_release(data[1]);
            }
            break;

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
    return true;  // opcode was in our range; swallowed either way
}

// Custom keycodes start at QK_KB_0 so they line up with the customKeycodes
// table in the VIA definition (VIA numbers CUSTOM(n) from QK_KB_0). Keep this
// enum and the JSON "customKeycodes" array in the same order.
//
// Only functions that need real firmware logic live here. Everything that was
// a keystroke sequence (PACS tools, window/level, app-open macros) is a VIA
// dynamic macro (M0..M22) â€” viewable and editable in the GUI's Macros pane â€”
// seeded with the original sequences by macro_seed_defaults() below.
enum custom_keycodes {
    DICTATE = QK_KB_0,  // PowerMic dictate + all-red LEDs
    SCROLLUP,           // continuous scroll toggle (slow)
    SCROLLDOWN,
    FAST_UP,            // continuous scroll toggle (fast)
    FAST_DOWN,
    FX_NEXT,            // cycle idle LED effect forward
    FX_PREV,            // cycle idle LED effect back

    // Named macro-runner keys: each simply plays its VIA dynamic macro, so the
    // key shows a readable name ("01-Annotate") in the GUI while the behaviour
    // stays editable in the Macros pane. Named after the DEFAULT position on
    // the physical Tartarus caps (01..19, L = layer 1). Order must match the
    // JSON customKeycodes array and pacs_macro_for[] below.
    K01_ANNOT,          // M15
    K02_WLSOFT,         // M4
    K03_WLLUNG,         // M8
    K04_WLVASC,         // M10
    K05_ZOOM,           // M13
    K06_EPIC,           // M2
    K07_WLBONE,         // M5
    K09_WLBRAIN,        // M6
    K10_ARROW,          // M12
    K11_COPYACC,        // M0
    K15_MEASURE,        // M14
    K16_GE,             // M1
    K17_SPINEC,         // M20
    K18_SPINET,         // M21
    K19_SPINEL,         // M22
    L05_INTERZOOM,      // M18
    L07_WLHDWR,         // M9
    L09_WLSTROKE,       // M7
    L10_ELLIPSE,        // M16
    L15_ROI,            // M17
    L16_MCKESSON,       // M3
    KX_HANG,            // M19 (not on a key by default; assignable)
    KX_WLSUBDURAL,       // M11 (not on a key by default; assignable)
    DICT_AHK,           // sends Ctrl+F21 for AHK/PowerScribe AND toggles dictate LEDs

    // PowerMic buttons (see powermic.h). Each mirrors the physical handset
    // button: press on key-down, release on key-up. DICTATE above is
    // Button 3 and also toggles the red LEDs.
    PM_K_TRANSCRIBE,    // Button 1
    PM_K_TABBACK,       // Button 2
    PM_K_TABFWD,        // Button 4
    PM_K_REWIND,        // Button 5
    PM_K_FFWD,          // Button 6
    PM_K_STOPPLAY,      // Button 7
    PM_K_CUSTOML,       // Button 8
    PM_K_ENTERSEL,      // Button 9
    PM_K_CUSTOMR        // Button 10
};

// PowerMic button index for each PM_K_* keycode, in enum order.
static const uint8_t pm_button_for[] = {
    PM_TRANSCRIBE, PM_TAB_BACK, PM_TAB_FORWARD, PM_REWIND, PM_FFWD,
    PM_STOP_PLAY, PM_CUSTOM_LEFT, PM_ENTER_SELECT, PM_CUSTOM_RIGHT
};

// Macro slot for each runner key, indexed by (keycode - K01_ANNOT).
static const uint8_t pacs_macro_for[] = {
    15, 4, 8, 10, 13, 2, 5, 6, 12, 0, 14, 1, 20, 21, 22,  // K01..K19
    18, 9, 7, 16, 17, 3,                                   // L05..L16
    19, 11                                                 // hang, subdural
};

// Macro slot assignments (fixed; VIA shows these as M0..M22):
//  M0 CopyAccession  M1 OpenGE      M2 OpenEpic    M3 OpenMcKesson
//  M4 W/L Soft(KP1)  M5 Bone(KP2)   M6 Brain(KP3)  M7 Stroke(KP4)
//  M8 Lung(KP5)      M9 Hardware(KP6) M10 Vascular(KP7) M11 Subdural(KP8)
//  M12 Arrow(a) M13 Zoom(z) M14 Measure(m) M15 Annotation(y)
//  M16 Ellipse(e) M17 ROI(E) M18 Interzoom(Z) M19 Hang(h)
//  M20 Spine C  M21 Spine T  M22 Spine L

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_tartarus2(
            /*â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬ */
            /*â”‚    01      â”‚    02      â”‚    03      â”‚    04      â”‚    05      â”‚ */
                K01_ANNOT,   K02_WLSOFT,  K03_WLLUNG,  K04_WLVASC,  K05_ZOOM,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/
            /*â”‚    06      â”‚    07      â”‚    08      â”‚    09      â”‚    10      â”‚*/  
                K06_EPIC,    K07_WLBONE,  SCROLLUP,   K09_WLBRAIN,  K10_ARROW,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/
            /*â”‚    11      â”‚    12      â”‚    13      â”‚    14      â”‚    15      â”‚*/ 
               K11_COPYACC,   FAST_UP,    SCROLLDOWN,  FAST_DOWN,  K15_MEASURE,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/  
            /*â”‚    16      â”‚    17      â”‚    18      â”‚    19      â”‚*/ 
                  K16_GE,     K17_SPINEC,  K18_SPINET,  K19_SPINEL,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/  
            /*â”‚    SW      â”‚    SC      â”‚    THUMB   â”‚     20     â”‚*/
                  SCROLLUP,   SCROLLDOWN,   MO(1),      DICTATE,      
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/  
            /*â”‚    LEFT    â”‚    RIGHT        UP      â”‚    DOWN   â”‚ */
                KC_MS_LEFT, KC_MS_RIGHT,   KC_MS_UP,   KC_MS_DOWN
            /*â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜*/
    ),
    
    [1] = LAYOUT_tartarus2(
            /*â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬ */
            /*â”‚    01      â”‚    02      â”‚    03      â”‚    04      â”‚    05      â”‚ */
                  KC_H,        KC_TRNS,     KC_TRNS,     KC_TRNS,  L05_INTERZOOM,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/
            /*â”‚    06      â”‚    07      â”‚    08      â”‚    09      â”‚    10      â”‚*/  
                  KC_P,      L07_WLHDWR, KC_MS_WH_UP, L09_WLSTROKE, L10_ELLIPSE,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/
            /*â”‚    11      â”‚    12      â”‚    13      â”‚    14      â”‚    15      â”‚*/ 
                  KC_R,     KC_MS_WH_UP, KC_MS_WH_DOWN, KC_MS_WH_DOWN, L15_ROI,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/  
            /*â”‚    16      â”‚    17      â”‚    18      â”‚    19      â”‚*/
               L16_MCKESSON,   KC_TRNS,     FX_PREV,     FX_NEXT,
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/  
            /*â”‚    SW      â”‚    SC      â”‚    THUMB      â”‚    20   â”‚*/
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,      
            /*â”œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¼*/  
            /*â”‚    LEFT    â”‚    RIGHT   â”‚    UP       â”‚    DOWN   â”‚ */
                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS
            /*â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜*/
        
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
// 1 (or RAW HID 0xCC) cycle it live. Edit colours/speeds here to taste â€” this
// is the "settable from keymap.c" surface.
static void fx_static_base(void)   { RGB b = hsv_to_rgb(base_hsv); tartarus_rgb_static(b.r, b.g, b.b); }
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
    fx_static_base,    // 0  static, idle colour (default)
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

// Colours come from the VIA-tunable HSV variables declared near the top.

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
    bool    scrolling = (token != INVALID_DEFERRED_TOKEN) || host_scroll;
    bool    pulse_red = scrolling && ((timer_read32() / scroll_half) & 1);
    uint8_t want[TARTARUS_RGB_KEYS][3];
    RGB     base   = hsv_to_rgb(base_hsv);
    RGB     accent = hsv_to_rgb(scroll_hsv);

    for (uint8_t k = 0; k < TARTARUS_RGB_KEYS; k++) {
        bool red = (held_mask >> k) & 1;
        if (scrolling && k == scroll_led) {
            red = pulse_red;
        }
        want[k][0] = red ? accent.r : base.r;
        want[k][1] = red ? accent.g : base.g;
        want[k][2] = red ? accent.b : base.b;
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
        // Clear driver mode (it kills key reporting and sticks while the
        // device is powered), then re-push brightness, which is device-side
        // state that a frame reapply would not restore.
        tartarus_rgb_driver_mode(false);
        tartarus_rgb_brightness(led_bright);
    }
    was_ready = ready;
    if (!ready) {
        led_applied = LST_NONE;
        frame_valid = false;
        return;
    }

    // DICTATE overrides everything: all keys in the dictate colour.
    if (dictating) {
        if (led_applied != LST_DICT) {
            led_applied = LST_DICT;
            frame_valid = false;
            RGB d = hsv_to_rgb(dict_hsv);
            tartarus_rgb_static(d.r, d.g, d.b);
        }
        return;
    }

    // Default mode (idle_fx == 0): per-key reactive frames. Baseline in the
    // idle colour, held keys in the accent colour, pulse on the scroll key.
    if (idle_fx == 0) {
        led_applied = LST_IDLE;
        led_render_frame();
        return;
    }

    // A native idle effect is selected: edge-triggered; scrolling shows a
    // whole-pad breathing pulse in the accent colour.
    bool        scrolling = (token != INVALID_DEFERRED_TOKEN) || host_scroll;
    led_state_t want      = scrolling ? LST_SCROLL : LST_IDLE;
    if (want == led_applied) {
        return;
    }
    led_applied = want;
    frame_valid = false;
    if (want == LST_SCROLL) {
        RGB s = hsv_to_rgb(scroll_hsv);
        tartarus_rgb_effect_breathing(s.r, s.g, s.b);
    } else {
        idle_fx_table[idle_fx]();
    }
}

static void led_force_reapply(void) {
    led_applied = LST_NONE;
    frame_valid = false;
}

// --- First-boot EEPROM crash auto-heal --------------------------------------
// The first boot after new firmware mass-initializes the VIA EEPROM. Those
// flash writes disable XIP while core 1 (the USB host) is still executing its
// flash-resident startup path, which kills the host stack for that boot only.
// Detect the fresh-EEPROM boot and soft-reset once; the second boot performs
// no mass writes and comes up clean.
static bool eeprom_was_reset = false;

void eeconfig_init_user(void) {
    eeprom_was_reset = true;  // only called when EEPROM is (re)initialized
}

// --- Default VIA macros (the PACS functions, GUI-editable) ------------------
// Dynamic macros replay through send_string, so their stored bytes use the
// exact same encoding SEND_STRING literals compile to. This buffer is the
// original hardcoded sequences, one macro per NUL-terminated entry, in the
// M0..M22 order documented at the enum above (M23 spare = empty).
static const char macro_defaults[] =
    // M0 Copy Accession
    SS_LSFT("`") SS_LSFT("`") SS_TAP(X_ENTER) SS_DELAY(100)
    SS_LSFT(SS_LCTL(SS_TAP(X_LEFT))) SS_LCTL("c") SS_LSFT(SS_TAP(X_HOME))
    SS_TAP(X_DEL) SS_TAP(X_DEL) "\0"
    // M1 Open GE
    SS_LSFT(SS_TAP(X_TAB)) SS_TAP(X_END) SS_DELAY(75) SS_TAP(X_TAB)
    SS_LCTL("v") SS_DELAY(150) SS_TAP(X_ENTER) "\0"
    // M2 Open Epic
    SS_LCTL("2") SS_DELAY(2500) SS_LCTL("v") SS_DELAY(500) SS_TAP(X_ENTER)
    SS_DELAY(2500) SS_LALT(SS_LSFT("a")) "\0"
    // M3 Open McKesson
    SS_LCTL("f") SS_DELAY(500) SS_TAP(X_BSPC) SS_LCTL("v") SS_TAP(X_ENTER)
    SS_DELAY(500) SS_TAP(X_ENTER) "\0"
    // M4..M11 window/level presets: click, settle, keypad digit
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_1) "\0"  // M4  soft tissue
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_2) "\0"  // M5  bone
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_3) "\0"  // M6  brain
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_4) "\0"  // M7  stroke
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_5) "\0"  // M8  lung
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_6) "\0"  // M9  hardware
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_7) "\0"  // M10 vascular
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_KP_8) "\0"  // M11 subdural
    // M12..M19 PACS tools: click, settle, tool letter
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_A) "\0"             // M12 arrow
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_Z) "\0"             // M13 zoom
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_M) "\0"             // M14 measure
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_Y) "\0"             // M15 annotation
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_E) "\0"             // M16 ellipse
    SS_TAP(X_BTN1) SS_DELAY(50) SS_LSFT(SS_TAP(X_E)) "\0"    // M17 ROI
    SS_TAP(X_BTN1) SS_DELAY(50) SS_LSFT(SS_TAP(X_Z)) "\0"    // M18 interzoom
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_H) "\0"             // M19 hang
    // M20..M22 spine labels
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_C) "\0"             // M20 spine C
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_T) "\0"             // M21 spine T
    SS_TAP(X_BTN1) SS_DELAY(50) SS_TAP(X_L);                 // M22 spine L
    // (implicit literal NUL terminates M22; M23 spare stays empty)

// Seed the factory macros whenever macro slot 0 is empty — i.e. on a fresh
// EEPROM or after a macro reset. A GUI-edited macro 0 is nonzero, so user
// edits are never overwritten. (Emptying M0 on purpose re-seeds on next boot.)
static void macro_seed_defaults(void) {
    if (sizeof(macro_defaults) > dynamic_keymap_macro_get_buffer_size()) {
        return;
    }
    uint8_t first = 0xFF;
    dynamic_keymap_macro_get_buffer(0, 1, &first);
    if (first != 0x00) {
        return;
    }
    dynamic_keymap_macro_set_buffer(0, (uint16_t)sizeof(macro_defaults), (uint8_t *)macro_defaults);
}

// --- VIA "Lighting" settings persistence (EEPROM kb datablock) --------------
// Saved on VIA "Save", loaded at post-init. The version byte tells a real
// save apart from a zeroed block, so compiled defaults survive a fresh
// EEPROM. Writes go through the wear-leveling backing store, which parks
// core 1 across the flash op (see matrix.c).
#define TRGB_CFG_VERSION 1

typedef struct PACKED {
    uint8_t version;   // TRGB_CFG_VERSION; anything else => treat as absent
    uint8_t idle_fx;
    uint8_t base_h, base_s;
    uint8_t dict_h, dict_s;
    uint8_t scroll_h, scroll_s;
    uint8_t bright;
    uint8_t rep_slow;  // REP_DELAY / 10        (VIA speed-slider units)
    uint8_t rep_fast;  // REP_DELAY_FAST / 10
} trgb_settings_t;
_Static_assert(sizeof(trgb_settings_t) <= EECONFIG_KB_DATA_SIZE,
               "trgb_settings_t larger than EECONFIG_KB_DATA_SIZE");

static void trgb_settings_save(void) {
    const trgb_settings_t s = {
        .version  = TRGB_CFG_VERSION,
        .idle_fx  = idle_fx,
        .base_h   = base_hsv.h,   .base_s   = base_hsv.s,
        .dict_h   = dict_hsv.h,   .dict_s   = dict_hsv.s,
        .scroll_h = scroll_hsv.h, .scroll_s = scroll_hsv.s,
        .bright   = led_bright,
        .rep_slow = (uint8_t)(REP_DELAY / 10),
        .rep_fast = (uint8_t)(REP_DELAY_FAST / 10),
    };
    eeconfig_update_kb_datablock(&s);  // parks core 1 via backing_store hooks
}

static void trgb_settings_load(void) {
    trgb_settings_t s;
    eeconfig_read_kb_datablock(&s);  // memsets to 0 if the block is invalid
    if (s.version != TRGB_CFG_VERSION) {
        return;  // no saved settings: keep the compiled-in defaults
    }
    base_hsv.h   = s.base_h;   base_hsv.s   = s.base_s;
    dict_hsv.h   = s.dict_h;   dict_hsv.s   = s.dict_s;
    scroll_hsv.h = s.scroll_h; scroll_hsv.s = s.scroll_s;
    led_bright     = s.bright;
    REP_DELAY      = (uint16_t)s.rep_slow * 10;
    REP_DELAY_FAST = (uint16_t)s.rep_fast * 10;
    led_set_idle_fx(s.idle_fx);  // sets idle_fx and forces a reapply
    led_force_reapply();         // colours (and brightness, via led_apply's
                                 // device-ready path) re-push to the device
}

void keyboard_post_init_user(void) {
    macro_seed_defaults();
    if (eeprom_was_reset) {
        soft_reset_keyboard();  // fresh-EEPROM heal: reboots, never returns
    }
    trgb_settings_load();
}

// --- VIA custom "Lighting" menu ---------------------------------------------
// Channel 6, value ids below. usevia.app reads/writes these through the JSON
// definition's menus; every set takes effect immediately.
enum {
    TRGB_VAL_IDLE_FX      = 1,  // dropdown: idle effect index (0 = per-key reactive)
    TRGB_VAL_BASE_COLOR   = 2,  // color picker: idle baseline (hue, sat)
    TRGB_VAL_DICT_COLOR   = 3,  // color picker: dictate override
    TRGB_VAL_SCROLL_COLOR = 4,  // color picker: held-key / scroll pulse accent
    TRGB_VAL_BRIGHTNESS   = 5,  // slider 0..255
    TRGB_VAL_SPEED_SLOW   = 6,  // slider: slow scroll repeat/pulse, ms/10
    TRGB_VAL_SPEED_FAST   = 7,  // slider: fast scroll repeat/pulse, ms/10
};
#define TRGB_CHANNEL 6

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [command_id, channel_id, value_id, value_data...]
    uint8_t *command_id = &(data[0]);
    uint8_t *channel_id = &(data[1]);
    uint8_t *value_id   = &(data[2]);
    uint8_t *value_data = &(data[3]);

    if (*channel_id != TRGB_CHANNEL) {
        *command_id = id_unhandled;
        return;
    }

    switch (*command_id) {
        case id_custom_set_value:
            switch (*value_id) {
                case TRGB_VAL_IDLE_FX:      led_set_idle_fx(value_data[0]); break;
                case TRGB_VAL_BASE_COLOR:   base_hsv.h = value_data[0]; base_hsv.s = value_data[1]; break;
                case TRGB_VAL_DICT_COLOR:   dict_hsv.h = value_data[0]; dict_hsv.s = value_data[1]; break;
                case TRGB_VAL_SCROLL_COLOR: scroll_hsv.h = value_data[0]; scroll_hsv.s = value_data[1]; break;
                case TRGB_VAL_BRIGHTNESS:   led_bright = value_data[0]; tartarus_rgb_brightness(led_bright); break;
                case TRGB_VAL_SPEED_SLOW:   REP_DELAY = value_data[0] * 10; break;
                case TRGB_VAL_SPEED_FAST:   REP_DELAY_FAST = value_data[0] * 10; break;
            }
            led_force_reapply();
            break;

        case id_custom_get_value:
            switch (*value_id) {
                case TRGB_VAL_IDLE_FX:      value_data[0] = idle_fx; break;
                case TRGB_VAL_BASE_COLOR:   value_data[0] = base_hsv.h; value_data[1] = base_hsv.s; break;
                case TRGB_VAL_DICT_COLOR:   value_data[0] = dict_hsv.h; value_data[1] = dict_hsv.s; break;
                case TRGB_VAL_SCROLL_COLOR: value_data[0] = scroll_hsv.h; value_data[1] = scroll_hsv.s; break;
                case TRGB_VAL_BRIGHTNESS:   value_data[0] = led_bright; break;
                case TRGB_VAL_SPEED_SLOW:   value_data[0] = REP_DELAY / 10; break;
                case TRGB_VAL_SPEED_FAST:   value_data[0] = REP_DELAY_FAST / 10; break;
            }
            break;

        case id_custom_save:
            trgb_settings_save();  // persist the current settings to EEPROM
            break;

        default:
            *command_id = id_unhandled;
            break;
    }
}

void housekeeping_task_user(void) {
    led_apply();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    key_event_count++;  // input-path diagnostic (0xD4)

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

    // Named macro-runner keys: play the mapped VIA dynamic macro. The name is
    // for the GUI; the behaviour is whatever the (editable) macro contains.
    // PowerMic buttons: mirror press and release.
    if (keycode >= PM_K_TRANSCRIBE && keycode <= PM_K_CUSTOMR) {
        if (record->event.pressed) {
            powermic_button_press(pm_button_for[keycode - PM_K_TRANSCRIBE]);
        } else {
            powermic_button_release(pm_button_for[keycode - PM_K_TRANSCRIBE]);
        }
        return false;
    }

    if (keycode >= K01_ANNOT && keycode <= KX_WLSUBDURAL) {
        if (record->event.pressed) {
            dynamic_keymap_macro_send(pacs_macro_for[keycode - K01_ANNOT]);
        }
        return false;
    }

    switch (keycode) {

        case DICT_AHK:
            // AHK-model dictate: emit the production Ctrl+F21 (AutoHotkey activates
            // PowerScribe and triggers dictation) while the firmware runs the same
            // red LED toggle as the native DICTATE key.
            if (record->event.pressed) {
                tap_code16(C(KC_F21));
                dictating = !dictating;
            }
            return false;

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



    }
    return true; 
}
