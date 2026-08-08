; ========================================================================
; TARTARUS V2 KEYMAP LAYOUT DOCUMENTATION
; ========================================================================
;
; Physical Layout with Key Assignments (QMK KC_CODE / AHK2 Reference):
;
;    ┌────────────┬────────────┬────────────┬────────────┬────────────┐
;    │    01      │    02      │    03      │    04      │    05      │
;    │   KC_F13   │   KC_F14   │   KC_F15   │   KC_F16   │   KC_F17   │
;    │    F13     │    F14     │    F15     │    F16     │    F17     │
;    ├────────────┼────────────┼────────────┼────────────┼────────────┤
;    │    06      │    07      │    08      │    09      │    10      │
;    │   KC_F18   │   KC_F19   │   KC_F20   │   KC_F21   │   KC_F22   │
;    │    F18     │    F19     │    F20     │    F21     │    F22     │
;    ├────────────┼────────────┼────────────┼────────────┼────────────┤
;    │    11      │    12      │    13      │    14      │    15      │
;    │   KC_F23   │   KC_F24   │LCTL(KC_F13)│LCTL(KC_F14)│LCTL(KC_F15)│
;    │    F23     │    F24     │   ^F13     │   ^F14     │   ^F15     │
;    ├────────────┼────────────┼────────────┼────────────┼────────────┤
;    │    16      │    17      │    18      │    19      │
;    │LCTL(KC_F16)│LCTL(KC_F17)│LCTL(KC_F18)│LCTL(KC_F19)│
;    │   ^F16     │   ^F17     │   ^F18     │   ^F19     │
;    ├────────────┼────────────┼────────────┼────────────┤
;    │    SW      │    SC      │   THUMB    │    20      │
;    │  KC_TRNS   │  KC_TRNS   │LCTL(KC_F20)│LCTL(KC_F21)│
;    │    N/A     │    N/A     │   ^F20     │   ^F21     │
;    ├────────────┼────────────┼────────────┼────────────┤
;    │    LEFT    │   RIGHT    │     UP     │    DOWN    │
;    │LALT(KC_F13)│LALT(KC_F14)│LALT(KC_F15)│LALT(KC_F16)│
;    │   !F13     │   !F14     │   !F15     │   !F16     │
;    └────────────┴────────────┴────────────┴────────────┘
;
; QMK KC_CODE TO AHK2 REFERENCE MAPPING:
; ┌─────────────┬─────────────┬──────────────────────────────────────┐
; │  QMK Code   │ AHK2 Syntax │            Description               │
; ├─────────────┼─────────────┼──────────────────────────────────────┤
; │   KC_F13    │     F13     │ Function Key 13                      │
; │   KC_F14    │     F14     │ Function Key 14                      │
; │   KC_F15    │     F15     │ Function Key 15                      │
; │   KC_F16    │     F16     │ Function Key 16                      │
; │   KC_F17    │     F17     │ Function Key 17                      │
; │   KC_F18    │     F18     │ Function Key 18                      │
; │   KC_F19    │     F19     │ Function Key 19                      │
; │   KC_F20    │     F20     │ Function Key 20                      │
; │   KC_F21    │     F21     │ Function Key 21                      │
; │   KC_F22    │     F22     │ Function Key 22                      │
; │   KC_F23    │     F23     │ Function Key 23                      │
; │   KC_F24    │     F24     │ Function Key 24                      │
; │LCTL(KC_F20) │    ^F20     │ Left Ctrl + F20 (Thumb)              │
; │LCTL(KC_F21) │    ^F21     │ Left Ctrl + F21 (Space)              │
; │LCTL(KC_F13) │    ^F13     │ Left Ctrl + F13 (Pos 13)             │
; │LCTL(KC_F14) │    ^F14     │ Left Ctrl + F14 (Pos 14)             │
; │LCTL(KC_F15) │    ^F15     │ Left Ctrl + F15 (Pos 15)             │
; │LCTL(KC_F16) │    ^F16     │ Left Ctrl + F16 (Pos 16)             │
; │LCTL(KC_F17) │    ^F17     │ Left Ctrl + F17 (Pos 17)             │
; │LCTL(KC_F18) │    ^F18     │ Left Ctrl + F18 (Pos 18)             │
; │LCTL(KC_F19) │    ^F19     │ Left Ctrl + F19 (Pos 19)             │
; │LALT(KC_F13) │    !F13     │ Left Alt + F13 (LEFT directional)   │
; │LALT(KC_F14) │    !F14     │ Left Alt + F14 (RIGHT directional)  │
; │LALT(KC_F15) │    !F15     │ Left Alt + F15 (UP directional)     │
; │LALT(KC_F16) │    !F16     │ Left Alt + F16 (DOWN directional)   │
; └─────────────┴─────────────┴──────────────────────────────────────┘
;
; CTRL+FUNCTION KEYS BEING DETECTED:
; - LCTL(KC_F20): Thumb button (position THUMB) → ^F20
; - LCTL(KC_F21): Space replacement (position 20) → ^F21
; - LCTL(KC_F13): Position 13 (row 3, leftmost) → ^F13
; - LCTL(KC_F14): Position 14 (row 3, second from left) → ^F14
; - LCTL(KC_F15): Position 15 (row 3, rightmost) → ^F15
; - LCTL(KC_F16): Position 16 (row 4, leftmost) → ^F16
; - LCTL(KC_F17): Position 17 (row 4, second from left) → ^F17
; - LCTL(KC_F18): Position 18 (row 4, third from left) → ^F18
; - LCTL(KC_F19): Position 19 (row 4, rightmost) → ^F19
;
; ALT+FUNCTION KEYS BEING DETECTED:
; - LALT(KC_F13): LEFT directional button → !F13
; - LALT(KC_F14): RIGHT directional button → !F14
; - LALT(KC_F15): UP directional button → !F15
; - LALT(KC_F16): DOWN directional button → !F16
;
; ========================================================================

; Function keys (unchanged)
F13:: {
    MsgBox("KC_F13 - Function Key 13", "Tartarus V2")
}

F14:: {
    MsgBox("KC_F14 - Function Key 14", "Tartarus V2")
}

F15:: {
    MsgBox("KC_F15 - Function Key 15", "Tartarus V2")
}

F16:: {
    MsgBox("KC_F16 - Function Key 16", "Tartarus V2")
}

F17:: {
    MsgBox("KC_F17 - Function Key 17", "Tartarus V2")
}

F18:: {
    MsgBox("KC_F18 - Function Key 18", "Tartarus V2")
}

F19:: {
    MsgBox("KC_F19 - Function Key 19", "Tartarus V2")
}

F20:: {
    MsgBox("KC_F20 - Function Key 20", "Tartarus V2")
}

F21:: {
    MsgBox("KC_F21 - Function Key 21", "Tartarus V2")
}

F22:: {
    MsgBox("KC_F22 - Function Key 22", "Tartarus V2")
}

F23:: {
    MsgBox("KC_F23 - Function Key 23", "Tartarus V2")
}

F24:: {
    MsgBox("KC_F24 - Function Key 24", "Tartarus V2")
}

; Ctrl + Function keys
^F13:: {
    MsgBox("LCTL(KC_F13) - Position 13", "Tartarus V2")
}

^F14:: {
    MsgBox("LCTL(KC_F14) - Position 14", "Tartarus V2")
}

^F15:: {
    MsgBox("LCTL(KC_F15) - Position 15", "Tartarus V2")
}

^F16:: {
    MsgBox("LCTL(KC_F16) - Position 16", "Tartarus V2")
}

^F17:: {
    MsgBox("LCTL(KC_F17) - Position 17", "Tartarus V2")
}

^F18:: {
    MsgBox("LCTL(KC_F18) - Position 18", "Tartarus V2")
}

^F19:: {
    MsgBox("LCTL(KC_F19) - Position 19", "Tartarus V2")
}

^F20:: {
    MsgBox("LCTL(KC_F20) - Thumb Key", "Tartarus V2")
}

^F21:: {
    MsgBox("LCTL(KC_F21) - Space Key", "Tartarus V2")
}

; Alt + Function keys (directional pad)
!F13:: {
    MsgBox("LALT(KC_F13) - LEFT", "Tartarus V2")
}

!F14:: {
    MsgBox("LALT(KC_F14) - RIGHT", "Tartarus V2")
}

!F15:: {
    MsgBox("LALT(KC_F15) - UP", "Tartarus V2")
}

!F16:: {
    MsgBox("LALT(KC_F16) - DOWN", "Tartarus V2")
}

; Exit script hotkey
^!q:: {
    MsgBox("Exiting Tartarus Key Detector", "Exit")
    ExitApp()
}

; Startup message
MsgBox("Tartarus V2 Key Detector Started`nPress Ctrl+Alt+Q to exit", "Started") 