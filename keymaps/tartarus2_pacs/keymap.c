#include QMK_KEYBOARD_H
#include "quantum.h"
#include "powermic.h"

// --- Tartarus V2 RGB via the OpenRazer protocol -----------------------------
// Proof-of-concept: turn the whole keyboard a solid colour by sending the
// device the same 90-byte vendor FEATURE report Razer Synapse would. Command
// decoded from notes/openrazer/ (razerkbd_driver.c + razerchromacommon.c):
//   extended-matrix "static" effect, transaction_id 0x1F (the Tartarus V2 tag),
//   command_class 0x0F, command_id 0x02, args = VARSTORE, BACKLIGHT_LED, STATIC,
//   then the RGB triple. CRC is XOR of report bytes [2..87].
// Sent via tartarus_send_feature_report() (matrix.c), the host-side path that
// already works for the keyboard lock LEDs. Async, so retry until it queues.

static bool    razer_led_pending = false;
static uint8_t razer_r, razer_g, razer_b;

static uint8_t razer_crc(const uint8_t *r) {
    uint8_t crc = 0;
    for (int i = 2; i < 88; i++) {
        crc ^= r[i];
    }
    return crc;
}

static void tartarus_request_color(uint8_t r, uint8_t g, uint8_t b) {
    razer_r = r; razer_g = g; razer_b = b;
    razer_led_pending = true;
}

void housekeeping_task_user(void) {
    if (!razer_led_pending) {
        return;
    }

    uint8_t report[90] = {0};
    report[1]  = 0x1F;    // transaction_id: Tartarus V2
    report[5]  = 0x09;    // data_size
    report[6]  = 0x0F;    // command_class: extended matrix
    report[7]  = 0x02;    // command_id: set effect
    report[8]  = 0x01;    // arg0: VARSTORE
    report[9]  = 0x05;    // arg1: BACKLIGHT_LED
    report[10] = 0x01;    // arg2: effect id = STATIC
    report[13] = 0x01;    // arg5: effect_static writes 0x01 here
    report[14] = razer_r; // arg6: R
    report[15] = razer_g; // arg7: G
    report[16] = razer_b; // arg8: B
    report[88] = razer_crc(report);

    if (tartarus_send_feature_report(report, sizeof(report))) {
        razer_led_pending = false;
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
    HANG
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
               OPENMCKESSON,   KC_TRNS,     KC_TRNS,     KC_TRNS,    
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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

	static deferred_token token = INVALID_DEFERRED_TOKEN;

    switch (keycode) {

		case DICTATE:
			// Was SS_TAP(X_F13): a keyboard scan code, so it only reached
			// PowerScribe when PowerScribe had focus, which is what forced the
			// AHK WinActivate helper. Now emits the PowerMic's own Button-page
			// report instead, which Raw Input delivers regardless of focus.
			if (record->event.pressed) {
				powermic_button_press(PM_DICTATE);
				tartarus_request_color(0xFF, 0x00, 0x00); // LED test: red on press
			} else {
				powermic_button_release(PM_DICTATE);
			}
		return false;

        case SCROLLUP:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token = INVALID_DEFERRED_TOKEN;
                } else {
                    tap_code(KC_MS_WH_UP);
                    token = defer_exec(REP_DELAY, wh_callback, (void*)true);
                }
            }
            return false;

        case SCROLLDOWN:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token = INVALID_DEFERRED_TOKEN;
                } else {
                    tap_code(KC_MS_WH_DOWN);
                    token = defer_exec(REP_DELAY, wh_callback, (void*)false);
                }
            }
            return false;

        case FAST_UP:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token = INVALID_DEFERRED_TOKEN;
                } else {
                    tap_code(KC_MS_WH_UP);
                    token = defer_exec(REP_DELAY_FAST, wh_callback_fast, (void*)true);
                }
            }
            return false;

        case FAST_DOWN:
            if (record->event.pressed) {
                if (token) {
                    cancel_deferred_exec(token);
                    token = INVALID_DEFERRED_TOKEN;
                } else {
                    tap_code(KC_MS_WH_DOWN);
                    token = defer_exec(REP_DELAY_FAST, wh_callback_fast, (void*)false);
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