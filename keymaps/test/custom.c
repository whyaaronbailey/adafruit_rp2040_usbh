/* const key_override_t brightness_up_override = ko_make_with_layers_negmods_and_options(MOD_MASK_CA, KC_MPLY, */
/*                                             KC_BRIU, ~0, MOD_MASK_SHIFT, ko_option_no_reregister_trigger); */

/* const key_override_t brightness_down_override = ko_make_basic(MOD_MASK_CSA, KC_MPLY, KC_BRID); */
/* const key_override_t **key_overrides = (const key_override_t *[]){ */
/*     &brightness_up_override, */
/*     &brightness_down_override, */
/*     NULL */
/* }; */
#include "print.h"
#define MODS_SHIFT(v)  (v & MOD_MASK_SHIFT)
#define MODS_CTRL(v)   (v & MOD_MASK_CTRL)
#define MODS_ALT(v)    (v & MOD_MASK_ALT)
#define MODS_GUI(v)    (v & MOD_MASK_GUI)

#ifdef COMBO_ENABLE
const uint16_t PROGMEM jk_combo[] = {KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM esc_combo[] = {KC_ESC, KC_ESC, COMBO_END};
combo_t key_combos[] = {
    COMBO(jk_combo, KC_ESC),
    COMBO(esc_combo, QK_LEAD),
};
#endif

#ifdef TAP_DANCE_ENABLE
// Function to determine the current tapdance state
td_state_t cur_dance(tap_dance_state_t *state);

// `finished` and `reset` functions for each tapdance keycode
void bsls_finished(tap_dance_state_t *state, void *user_data);
void bsls_reset(tap_dance_state_t *state, void *user_data);

void shift_finished(tap_dance_state_t *state, void *user_data);
void shift_reset(tap_dance_state_t *state, void *user_data);
// Determine the current tap dance state
td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (!state->pressed) return TD_SINGLE_TAP;
        else return TD_SINGLE_HOLD;
    } else if (state->count >= 2) return TD_DOUBLE_TAP;
    else return TD_UNKNOWN;
}

// Initialize tap structure associated with example tap dance key
static td_tap_t bsls_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};
static td_tap_t shift_tap_state = {
    .is_press_action = true,
    .state = TD_NONE
};

// Functions that control what our tap dance key does
void bsls_finished(tap_dance_state_t *state, void *user_data) {
    bsls_tap_state.state = cur_dance(state);
    switch (bsls_tap_state.state) {
        case TD_SINGLE_TAP:
            tap_code(KC_BSLS);
            break;
        case TD_DOUBLE_TAP:
            // Check to see if the layer is already set
            if (layer_state_is(LYR_EXTRAKEYS)) {
                // If already set, then switch it off
                layer_off(LYR_EXTRAKEYS);
            } else {
                // If not already set, then switch the layer on
                set_oneshot_layer(LYR_EXTRAKEYS, ONESHOT_START);
            }
            break;
        default:
            break;
    }
}

void bsls_reset(tap_dance_state_t *state, void *user_data) {
    // If the key was held down and now is released then switch off the layer
    if (bsls_tap_state.state == TD_DOUBLE_TAP) {
        clear_oneshot_layer_state(ONESHOT_PRESSED);
    }
    bsls_tap_state.state = TD_NONE;
}

void shift_finished(tap_dance_state_t *state, void *user_data) {
    shift_tap_state.state = cur_dance(state);
    switch (shift_tap_state.state) {
        case TD_SINGLE_TAP:
#ifdef LEADER_ENABLE
        leader_start();
        break;
#endif
        case TD_SINGLE_HOLD:
            register_mods(MOD_RSFT);
            break;
        default:
            break;
    }
}

void shift_reset(tap_dance_state_t *state, void *user_data) {
    switch (shift_tap_state.state) {
        case TD_SINGLE_TAP:
            break;

        case TD_SINGLE_HOLD:
            unregister_mods(MOD_RSFT);
            break;
        default: break;
    }

    shift_tap_state.state = TD_NONE;
}
void td_bsls(tap_dance_state_t *state, void *user_data) {
    if (state->count > 1) {
        leader_start();
    } else {
        tap_code(KC_BSLS);
    }
    reset_tap_dance(state);
}
// Associate our tap dance key with its functionality
// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap PS once for print screen, double tap to toggle mousekeys
    [TD_PS_2] = ACTION_TAP_DANCE_LAYER_TOGGLE(KC_PSCR, 2),
    // [TD_COPY] = ACTION_TAP_DANCE_DOUBLE(KC_C, LCTL(KC_C)),
    // [TD_PASTE] = ACTION_TAP_DANCE_DOUBLE(KC_V, LCTL(KC_V)),
    [TD_BSLS] = ACTION_TAP_DANCE_FN(td_bsls),
    /* [TD_SHIFT] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, shift_finished, shift_reset), */
    [TD_F10] = ACTION_TAP_DANCE_LAYER_MOVE(KC_F10, 2),
    [TD_F11] = ACTION_TAP_DANCE_LAYER_MOVE(KC_F11, 1),
    [TD_F12] = ACTION_TAP_DANCE_LAYER_MOVE(KC_F12, 0),
    [TD_WIN_TAB] = ACTION_TAP_DANCE_DOUBLE(KC_TAB, LGUI(KC_TAB)),
};
#endif

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
}
bool oled_task_user(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("LYR: "), false);
    /* oled_write_P(get_u8_str(get_highest_layer(layer_state), ' '), false); */
    switch (get_highest_layer(layer_state)) {
        case 0:
            oled_write_P(PSTR("DFLT\n"), false);
            break;
        case 1:
            oled_write_P(PSTR("XTRA\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            // oled_write_ln_P(PSTR("NA"), false);
            break;
    }
    oled_write_P(PSTR("\n"), false);

    int mods =get_mods() || get_oneshot_mods();
    if (MODS_SHIFT(mods)) {
        oled_write_P(PSTR("SHIFT\n\n"), false);
    } else {
        oled_write_P(PSTR("\n\n"), false);
    }
    if (MODS_CTRL(mods)) {
        oled_write_P(PSTR("CONTROL\n\n"), false);
    } else {
        oled_write_P(PSTR("\n\n"), false);
    }
    if (MODS_ALT(mods)) {
        oled_write_P(PSTR("ALT\n\n"), false);
    } else {
        oled_write_P(PSTR("\n\n"), false);
    }

    return false;
}
#endif

#ifdef RGBLIGHT_ENABLE
const rgblight_segment_t PROGMEM extra_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {8, 4, HSV_BLUE}       // Light 4 LEDs, starting with LED 6
);
const rgblight_segment_t PROGMEM rgb_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {8, 4, HSV_RED}       // Light 4 LEDs, starting with LED 6
);

const rgblight_segment_t PROGMEM my_alt_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {4, 2, HSV_GREEN},
    {14, 2, HSV_GREEN}
);
const rgblight_segment_t PROGMEM my_shift_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {0, 2, HSV_ORANGE},
    {18, 2, HSV_ORANGE}
);
const rgblight_segment_t PROGMEM my_ctrl_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {2, 2,  HSV_PINK},
    {16, 2,  HSV_PINK}
);
const rgblight_segment_t PROGMEM my_gui_layer[] = RGBLIGHT_LAYER_SEGMENTS(
    {6, 2,  HSV_MAGENTA},
    {12, 2,  HSV_MAGENTA}
);
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
    extra_layer,    // Overrides other layers
    rgb_layer, // Overrides other layers
    my_alt_layer,
    my_shift_layer,
    my_ctrl_layer,
    my_gui_layer
);

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    int mods =get_mods() | get_oneshot_mods();
    if ( (mods  & MOD_MASK_SHIFT)
        || (mods  & MOD_MASK_ALT)
        || (mods && MOD_MASK_GUI)
        || (mods && MOD_MASK_CTRL)){
        rgblight_set_layer_state(2, MODS_ALT(mods));
        rgblight_set_layer_state(3, MODS_SHIFT(mods));
        rgblight_set_layer_state(4, MODS_CTRL(mods));
        rgblight_set_layer_state(5, MODS_GUI(mods));

    } else {
        rgblight_set_layer_state(2, 0);
        rgblight_set_layer_state(3, 0);
        rgblight_set_layer_state(4, 0);
        rgblight_set_layer_state(5, 0);
    }
}
layer_state_t default_layer_state_set_user(layer_state_t state) {
    rgblight_set_layer_state(0, layer_state_cmp(state, LYR_EXTRAKEYS));
    rgblight_set_layer_state(1, layer_state_cmp(state, LYR_RGB));
    return state;
}


#endif

#ifdef LEADER_ENABLE
uint8_t leaderCSFT = 0;
void toggleCSFT (void) {
    if (!leaderCSFT) {
        register_mods(MOD_LCTL);
        register_mods(MOD_LSFT);
    }else {
        unregister_mods(MOD_LSFT);
        unregister_mods(MOD_LCTL);
    }

    leaderCSFT = !leaderCSFT;
}

void leader_end_user(void) {
    if (leader_sequence_two_keys(KC_E, KC_E)) {
        SEND_STRING("raghu.rajagopalan@gmail.com");
    } else if (leader_sequence_two_keys(KC_E, KC_S)) {
        SEND_STRING("raghu.nospam@gmail.com");
    } else if (leader_sequence_two_keys(KC_E, KC_W)) {
        SEND_STRING("raghu.rajagopalan@rockwellautomation.com");
    }
    if (leader_sequence_one_key(KC_L)) {
        SEND_STRING("ra-int\\rrajagopala");
    }
    if (leader_sequence_one_key(KC_MINS)) {
        SEND_STRING("->");
    }
    if (leader_sequence_one_key(KC_EQL)) {
        SEND_STRING("=>");
    }
    if (leader_sequence_one_key(KC_M)) {
        register_code(KC_LGUI);
        register_code(KC_F12);
        unregister_code(KC_F12);
        unregister_code(KC_LGUI);
    }
    if (leader_sequence_one_key(KC_V)) {
        register_code(KC_LALT);
        register_code(KC_LCTL);
        register_code(KC_V);
        unregister_code(KC_V);
        unregister_code(KC_LCTL);
        unregister_code(KC_LALT);
    }
}

/* void leader_end(void) { */
/*     clear_oneshot_layer_state(ONESHOT_PRESSED); */
/* } */
#endif

void keyboard_post_init_user(void) {

#ifdef CONSOLE_ENABLE
    // Customise these values to desired behaviour
    debug_enable=true;
    debug_keyboard=true;
    //debug_mouse=true;
#endif

#ifdef RGBLIGHT_ENABLE
    // Enable the LED layers
    rgblight_layers = my_rgb_layers;
    rgblight_set_effect_range(0, RGBLIGHT_LED_COUNT);
#endif
    uprintf("completed post_init_user");
}

layer_state_t layer_state_set_user(layer_state_t state) {
    int layer = get_highest_layer(state);
    int keyCode = KC_F12 - layer;
    register_code(KC_LSFT);
    register_code(keyCode);
    unregister_code(keyCode);
    unregister_code(KC_LSFT);
    layer_debug();
    return state;
}
