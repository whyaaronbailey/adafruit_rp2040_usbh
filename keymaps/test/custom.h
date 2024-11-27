enum LAYERS {
    LYR_DEFAULT,
    LYR_EXTRAKEYS,
    LYR_RGB
};

#ifdef TAP_DANCE_ENABLE
// Tap Dance keycodes
enum td_keycodes {
    TD_PS_2,
    TD_BSLS_ALTTAB,
    TD_SHIFT,
    // TD_COPY,
    // TD_PASTE,
    TD_WIN_TAB,
};
// Define a type containing as many tapdance states as you need
typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_TAP
} td_state_t;
typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;


#endif
