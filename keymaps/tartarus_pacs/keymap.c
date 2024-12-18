// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

enum custom_keycodes {
	COPYACC = SAFE_RANGE,
	OPENGE,
	OPENEPIC,
    OPENMCKESSON,
	PASTE,
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
	WHEELUP,
	WHEELDOWN,
	SPINE_C,
	SPINE_T,
	SPINE_L,
	NEXTFIELD,
	DELETE,
	ANNOTATION,
	PREVFIELD,
	FAST_UP,
	FAST_DOWN,
	SIGNREPORT,
    ELLIPSE,
    ROI,
    INTERZOOM,
    HANG
};

enum {
	TD_PAN_CINE = 0,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
 /*
     * ┌───┐   ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┐
     * │Esc│   │F1 │F2 │F3 │F4 │ │F5 │F6 │F7 │F8 │ │F9 │F10│F11│F12│ │PSc│Scr│Pse│
     * └───┘   └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┘
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐ ┌───┬───┬───┐ ┌───┬───┬───┬───┐
     * │ ` │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp│ │Ins│Hom│PgU│ │Num│ / │ * │ - │
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤ ├───┼───┼───┤ ├───┼───┼───┼───┤
     * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  │ │Del│End│PgD│ │ 7 │ 8 │ 9 │   │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤ └───┴───┴───┘ ├───┼───┼───┤ + │
     * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │               │ 4 │ 5 │ 6 │   │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤     ┌───┐     ├───┼───┼───┼───┤
     * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │    Shift │     │ ↑ │     │ 1 │ 2 │ 3 │   │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬────┤ ┌───┼───┼───┐ ├───┴───┼───┤Ent│
     * │Ctrl│GUI │Alt │                        │ Alt│ GUI│Menu│Ctrl│ │ ← │ ↓ │ → │ │   0   │ . │   │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘ └───┴───┴───┘ └───────┴───┴───┘
     *[0] = LAYOUT_fullsize_ansi(

        KC_ESC,           KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,     KC_PSCR, KC_SCRL, KC_PAUS,

        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,    KC_INS,  KC_HOME, KC_PGUP,    KC_NUM,  KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,    KC_DEL,  KC_END,  KC_PGDN,    KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,                                   KC_P4,   KC_P5,   KC_P6,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,             KC_UP,               KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                             KC_RALT, KC_RGUI, KC_APP,  KC_RCTL,    KC_LEFT, KC_DOWN, KC_RGHT,    KC_P0,            KC_PDOT
    )


     * TARATARUS REPRESENTATION ON ANSI.
     * Numbers represent key labels on the Tartarus
     * Arrow keys correspond to direction pad
     * THMB is the thumb button
     *
     * ┌───┐   ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┐
     * │   │   │   │   │   │   │ │   │   │   │   │ │   │   │   │   │ │   │   │   │
     * └───┘   └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┘
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐ ┌───┬───┬───┐ ┌───┬───┬───┬───┐
     * │   │ 01│ 02│ 03│ 04│ 05│   │   │   │   │   │   │   │       │ │   │   │   │ │   │   │   │   │
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤ ├───┼───┼───┤ ├───┼───┼───┼───┤
     * │  06 │07 │08 │09 │10 │   │   │   │   │   │   │   │   │     │ │   │   │   │ │   │   │   │   │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤ └───┴───┴───┘ ├───┼───┼───┤   │
     * │  11  │12 │13 │14 │15 │   │   │   │   │   │   │   │        │  DIRECTIONPAD │   │   │   │   │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤     ┌───┐     ├───┼───┼───┼───┤
     * │  16    │17 │18 │19 │   │   │   │   │   │   │   │          │     │ ↑ │     │   │   │   │   │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬────┤ ┌───┼───┼───┐ ├───┴───┼───┤   │
     * │    │    │THMB│          20            │THMB│    │    │    │ │ ← │ ↓ │ → │ │       │   │   │
     * └────┴────┴────┴────────────────────────┴────┴────┴────┴────┘ └───┴───┴───┘ └───────┴───┴───┘
    * In the example layout below, I've assigned the keys as follows. Hopefully this will be uncessary once I'm able to correctly define the tartarus in the info.json file.
    *
    * TARTARUS KEY LABEL            LAYOUT 0                    LAYOUT 1
    *        01                     ANNOTATION (Y)              HANG (H)
    *        02                     WL_SOFT (1)
    *        03                     WL_LUNG (5)
    *        04                     WL_SUBDURAL (8) (275 90)    WL_VASCULAR (7) 684 45)
    *        05                     ZOOM (Z)                    INTERZOOM (+Z)
    *        06                     OPENEPIC
    *        07                     WL_BONE (2)                 WL_HARDWARE (6)
    *        08                     SCROLL_UP
    *        09                     WL_BRAIN (3)                WL_STROKE (4)
    *        10                     ARROW (A)                   ELLIPSE (E)
    *        11                     COPYACC
    *        12                     FAST_UP
    *        13                     SCROLLDOWN
    *        14                     FAST_DOWN
    *        15                     MEASURE (M)                 ROI (+E)
    *        16                     OPENGE                      OPENMCKESSON
    *        17                     SPINE LABEL (C)
    *        18                     SPINE LABEL (T)
    *        19                     SPINE LABEL (L)/NAVIGATOR
    *        20                     DICTATE
    *        THUMB BUTTON           MO(1) / LAYER SWITCH
    *        DIRECTION PAD
    *           UP                  FAST_UP
    *           DOWN                FAST_DOWN
    *           LEFT                KC_MS_WH_UP
    *           RIGHT               KC_MS_WH_DOWN
    */



[0] = LAYOUT(

        KC_NO,                        KC_NO,      KC_NO,     KC_NO,       KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,

        KC_NO,        ANNOTATION,     WL_SOFT,    WL_LUNG,   WL_VASCULAR, ZOOM,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,              KC_NO,   KC_NO,   KC_NO,   KC_NO,
        OPENEPIC,     WL_BONE,        SCROLLUP,   WL_BRAIN,  ARROW,       KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,              KC_NO,   KC_NO,   KC_NO,   KC_NO,
        COPYACC,      FAST_UP,        SCROLLDOWN, FAST_DOWN, MEASURE,     KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,            KC_NO,                                            KC_NO,   KC_NO,   KC_NO,
        OPENGE,                       SPINE_C,    SPINE_T,   SPINE_L,     KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,            KC_NO,               KC_MS_U,                     KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,        KC_NO,          MO(1),                             DICTATE,                                      KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_MS_L, KC_MS_D, KC_MS_R,            KC_NO,            KC_NO
    ),

[1] = LAYOUT(

        KC_NO,                        KC_NO,      KC_NO,     KC_NO,     KC_NO,     KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,     KC_NO,   KC_NO,   KC_NO,

        KC_NO,        HANG,           KC_TRNS,    WL_LUNG,   KC_TRNS,   INTERZOOM, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,              KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_TRNS,      WL_HARDWARE,    SCROLLUP,   WL_STROKE, ELLIPSE,   KC_NO,     KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,      KC_NO,   KC_NO,   KC_NO,              KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_TRNS,      KC_TRNS,        SCROLLDOWN, FAST_DOWN, ROI,       KC_NO,     KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,            KC_NO,                                            KC_NO,   KC_NO,   KC_NO,
        OPENMCKESSON,                 KC_TRNS,    KC_TRNS,   KC_TRNS,   KC_NO,     KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,            KC_TRNS,             KC_TRNS,                     KC_TRNS, KC_NO,   KC_NO,   KC_NO,
        KC_NO,        KC_NO,          KC_TRNS,                           KC_TRNS,                                      KC_NO,   KC_NO,   KC_NO,   KC_TRNS,   KC_TRNS, KC_TRNS, KC_TRNS,             KC_TRNS,          KC_NO
    )
};


//Tap Dance FOR GE PAN / CINE
tap_dance_action_t tap_dance_actions[] = {
  [TD_PAN_CINE]  = ACTION_TAP_DANCE_DOUBLE(KC_P, KC_U)  // tap once for P, twice for U
};

static uint16_t REP_DELAY = 290; // Common delay for both scroll up and down
static uint16_t REP_DELAY_FAST = 120; // Common delay for both scroll up and down

uint32_t wh_callback(uint32_t trigger_time, void* cb_arg) {
    bool is_up = (bool)cb_arg;
    if (is_up) {
        tap_code(KC_MS_WH_UP);
    } else {
        tap_code(KC_MS_WH_DOWN);
    }
    return REP_DELAY;
};

uint32_t wh_callback_fast(uint32_t trigger_time, void* cb_arg) {
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

        case COPYACC:  // Copies the accession number to the clipboard
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

        case OPENGE: // Pastes the accession number in GE PACs
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

        case OPENEPIC: // Pastes the accession number in EPIC
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

        case OPENMCKESSON: // Pastes the accession number in GE PACs
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

        case PASTE: // Pastes the accession number in EPIC
            if (record->event.pressed) {
                SEND_STRING(
                    SS_LCTL("v")
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

		case DELETE:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_F15)
				);
			}
		return false;

		case WHEELUP:
			if (record->event.pressed) {
				SEND_STRING(
					SS_TAP(X_MS_WH_UP)
				);
			}
		return false;

			if (record->event.pressed) {
		case WHEELDOWN:
				SEND_STRING(
					SS_TAP(X_MS_WH_DOWN)
				);
			}
		return false;

		case DICTATE: // Pastes the accession number in EPIC SEND_STRING(SS_TAP(X_F13));
			if (record->event.pressed) {
				SEND_STRING(SS_TAP(X_F13));
			}
		return false;

		case PREVFIELD:
			if (record->event.pressed) {
				SEND_STRING(SS_TAP(X_F15));
			}
		return false;

		case NEXTFIELD: //SEND_STRING(SS_TAP(X_F14));
			if (record->event.pressed) {
				SEND_STRING(SS_TAP(X_F14));
			}
		return false;

		case SIGNREPORT: //SEND_STRING(SS_TAP(X_F16));
			if (record->event.pressed) {
				SEND_STRING(SS_TAP(X_F16));
			}
		return false;

    }
    return true; // Process all other keycodes normally.
}

