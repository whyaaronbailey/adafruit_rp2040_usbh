#include QMK_KEYBOARD_H
#include "quantum.h"

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
			if (record->event.pressed) {
				SEND_STRING(SS_TAP(X_F13));
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