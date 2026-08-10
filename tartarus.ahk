; Tartarus V2 Medical Imaging Mapping
; Based on ahkplan.md - Centricity and PowerScribe workflow
;
; ===== KEY LAYOUT DIAGRAM =====
;
; BASE LAYER (Primary Functions - Standard Scroll Speeds):
;┌────────────┬────────────┬────────────┬────────────┬────────────┐
;│    01      │    02      │    03      │    04      │    05      │
;│    F13     │    F14     │    F15     │    F16     │    F17     │
;│Navigation  │ WL Bone    │ WL Lung    │WL Vascular │   Zoom     │
;│  Ctrl+N    │  Numpad2   │  Numpad5   │  Numpad7   │ Ctrl+Sh+Z │
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│    06      │    07      │    08      │    09      │    10      │
;│    F18     │    F19     │    F20     │    F21     │    F22     │
;│   WL Tool  │  WL Soft    │Scroll Back │ WL Brain   │Arrow Annot │
;│ Ctrl+Sh+W  │  Numpad1   │Slow Up     │  Numpad3   │Ctrl+Sh+A  │
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│    11      │    12      │    13      │    14      │    15      │
;│    F23     │    F24     │  Ctrl+F13  │  Ctrl+F14  │  Ctrl+F15  │
;│  Select    │Scroll Back │Scroll Fwd  │Scroll Fwd  │  Measure   │
;│ Ctrl+Sh+S  │Fast Up     │Slow Up     │Fast Down   │ Ctrl+Alt+D│
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│    16      │    17      │    18      │    19      │    20      │
;│  Ctrl+F16  │  Ctrl+F17  │  Ctrl+F18  │  Ctrl+F19  │  Ctrl+F21  │
;│ Cine Scoll │  Lt Click  │Scroll Click│Rt Mouse Clk│PowerScribe │
;│ Ctrl+Sh+N  │ L Mouse Clk│ Toggle     │R Mouse Clk │    F4      │
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│   SW       │    SC      │   THUMB    │   D-PAD    │            │
;│  TRNS      │   TRNS     │ Left Alt   │Shift+F13-16│            │
;│            │            │            │Alt+M to    │            │
;│            │            │            │Centricity (all 4)│      │
;└────────────┴────────────┴────────────┴────────────┴────────────┘
;
; CHANGE LOG:
; 2026-07-15 - D-PAD: all 4 directions changed to send Alt+M to
;              Centricity RA1000. Previously: L/R = Fast Scroll
;              Up/Down toggle, U/D = Slow Scroll Up/Down toggle.
;              Old hotkeys are commented out in the DPAD KEYS
;              section below for easy reverting.
;
; ALTERNATE LAYER (Alt + Key - Different Scroll Speeds):
;┌────────────┬────────────┬────────────┬────────────┬────────────┐
;│  Alt+01    │  Alt+02    │  Alt+03    │  Alt+04    │  Alt+05    │
;│  Alt+F13   │  Alt+F14   │  Alt+F15   │  Alt+F16   │  Alt+F17   │
;│    TBD     │WL Hardware │    TBD     │ Subdural   │    Pan     │
;│            │  Numpad8   │            │  Numpad6   │ Ctrl+Sh+P │
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│  Alt+06    │  Alt+07    │  Alt+08    │  Alt+09    │  Alt+10    │
;│  Alt+F18   │  Alt+F19   │  Alt+F20   │  Alt+F21   │  Alt+F22   │
;│  Reset WL  │    TBD     │Scroll Back │ WL Stroke  │ Oval ROI   │
;│  Click+R   │            │Slower Up   │  Numpad4   │ Ctrl+Alt+O│
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│  Alt+11    │  Alt+12    │  Alt+13    │  Alt+14    │  Alt+15    │
;│  Alt+F23   │  Alt+F24   │Alt+Ctrl+F13│Alt+Ctrl+F14│Alt+Ctrl+F15│
;│   Delete   │Scroll Back │Scroll Fwd  │Scroll Fwd  │ Cine Scoll │
;│   {Delete} │Faster Up   │Slower Up   │Faster Down │  Ctrl+Sh+N │
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│  Alt+16    │  Alt+17    │  Alt+18    │  Alt+19    │  Alt+20    │
;│Alt+Ctrl+F16│Alt+Ctrl+F17│Alt+Ctrl+F18│Alt+Ctrl+F19│Alt+Ctrl+F21│
;│ Trpl Click │ Dbl Click  │   Copy     │PasteSpecial│    TBD     │
;│ Click(3x)  │ Click(2x)  │  Ctrl+C    │ Ctrl+Sh+V  │            │
;├────────────┼────────────┼────────────┼────────────┼────────────┤
;│   SW       │    SC      │   THUMB    │   D-PAD    │            │
;│  TRNS      │   TRNS     │   TRNS     │(see base)  │            │
;│            │            │            │            │            │
;└────────────┴────────────┴────────────┴────────────┴────────────┘
;
#Requires AutoHotkey v2.0
#SingleInstance Force
#UseHook
Persistent

; ===== TRAY ICON SETUP =====
; Set tray icon (16x16 is standard for Windows tray)
try {
    iconFile := "tartarus-16.png"
    TraySetIcon(iconFile)
} catch {
    ; If icon file not found, use default
    TraySetIcon()
}

; Set tray tooltip
A_IconTip := "Tartarus Controller"


; ===== SCROLLING CONFIGURATION (EASY TO ADJUST) =====
; Base Layer (Version 1 - Wheel Notches)
wheelSlowDelay := 100        ; Milliseconds between slow wheel events
wheelFastDelay := 50         ; Milliseconds between fast wheel events

; Alt Layer (Version 2 - Smooth Pixel Scrolling)
pixelSlowDelay := 20         ; Milliseconds between slow pixel scroll (~50fps)
pixelFastDelay := 10         ; Milliseconds between fast pixel scroll (~100fps)
pixelSlowAmount := 3         ; Pixels per scroll (slow)
pixelFastAmount := 8         ; Pixels per scroll (fast)

; Global scroll state
scrolling := false
scrollTimer := ""
currentWheelDir := ""
currentScrollAmount := 0

; Global mouse toggle state
Toggle := false

; ===== HELPER FUNCTIONS =====

; Activate Centricity
ActivateCentricity() {
    try {
        WinActivate("ahk_class SunAwtFrame ahk_exe java.exe")
    } catch {
        try {
            WinActivate("ahk_class SunAwtFrame ahk_exe javaw.exe")
        } catch {
            MsgBox("Centricity not found!", "Error", "OK")
        }
    }
}

; Activate PowerScribe
 ActivatePowerScribe() {
    try {
        WinActivate("PowerScribe One")
    } catch {
        MsgBox("PowerScribe not found!", "Error", "OK")
    }
}

#Requires AutoHotkey v2.0
#SingleInstance Force
#UseHook
Persistent

; ===== SCROLLING CONFIGURATION (EASY TO ADJUST) =====
wheelSlowDelay := 350        ; ms between slow wheel events (base layer)
wheelFastDelay := 175        ; ms between fast wheel events (base layer)

altWheelSlowDelay := 400     ; ms between slow wheel events (alt layer)
altWheelFastDelay := 125     ; ms between fast wheel events (alt layer)

; Global scroll state
scrolling := false
currentScrollDirection := ""
currentScrollSpeed := ""

; ===== SCROLLING FUNCTIONS =====

; Dedicated timer functions that can be referenced by name
WheelScrollTimer() {
    global currentScrollDirection
    if (currentScrollDirection == "back") {
        Send("{WheelUp}")      ; Back = scroll UP
    } else {
        Send("{WheelDown}")    ; Forward = scroll DOWN
    }
}

AltWheelScrollTimer() {
    global currentScrollDirection
    if (currentScrollDirection == "back") {
        Send("{WheelUp}")      ; Back = scroll UP
    } else {
        Send("{WheelDown}")    ; Forward = scroll DOWN
    }
}

; Stop all scrolling by referencing function names directly
StopScroll() {
    global scrolling
    if (scrolling) {
        scrolling := false
        ; Stop both timer types using function names directly
        SetTimer(WheelScrollTimer, 0)
        SetTimer(AltWheelScrollTimer, 0)
    }
}

; Start wheel scrolling
ToggleWheelScroll(direction, speed) {
    global scrolling, currentScrollDirection, currentScrollSpeed, wheelSlowDelay, wheelFastDelay

    ; If already scrolling, stop it (toggle behavior)
    if (scrolling) {
        StopScroll()
        return
    }

    ; Set scroll parameters
    currentScrollDirection := direction
    currentScrollSpeed := speed
    scrolling := true

    ; Set delay and start timer using function name directly
    delay := (speed == "slow") ? wheelSlowDelay : wheelFastDelay
    SetTimer(WheelScrollTimer, delay)
}

; Start alt wheel scrolling (different speeds)
ToggleAltWheelScroll(direction, speed) {
    global scrolling, currentScrollDirection, currentScrollSpeed, altWheelSlowDelay, altWheelFastDelay

    ; If already scrolling, stop it (toggle behavior)
    if (scrolling) {
        StopScroll()
        return
    }

    ; Set scroll parameters
    currentScrollDirection := direction
    currentScrollSpeed := speed
    scrolling := true

    ; Set delay and start timer using function name directly
    delay := (speed == "slow") ? altWheelSlowDelay : altWheelFastDelay
    SetTimer(AltWheelScrollTimer, delay)
}

; ===== TARTARUS KEY MAPPINGS (Positions 1-20) =====

; 01 Navigation (PACS Navigator + Escape)
F13:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^n")
    Sleep(50)
    Send("{Escape}")
}
; Ctrl+Shift+V: Instant Plain Text Paste (with newline trimming)
^+v:: {
    if (A_Clipboard != "") {
        A_Clipboard := RTrim(A_Clipboard, "`r`n")  ; Remove trailing newlines and strip formatting
        Send("^v")  ; Instant paste with no typing effect
    }
}

; ~~ (Double Tilde): Same as Ctrl+Shift+V paste special
::~~:: {
    if (A_Clipboard != "") {
        A_Clipboard := RTrim(A_Clipboard, "`r`n")  ; Remove trailing newlines and strip formatting
        Send("^v")  ; Instant paste with no typing effect
    }
}

; Alt-01: TBD
!F13:: {
    StopScroll()  ; Stop any active scrolling first
    MsgBox("Alt-F13 - TBD", "Info", "OK T1")
}

; 02 WL Bone
F14:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad2}")
}
; Alt-02 WL Hardware
!F14:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad8}")
}

; 03 WL Lung
F15:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad5}")
}
; Alt-03: TBD
!F15:: {
    StopScroll()  ; Stop any active scrolling first
    MsgBox("Alt-F15 - TBD", "Info", "OK T1")
}

; 04 WL Vascular
F16:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad7}")
}
; Alt-16: Triple Left Click (must come before !F16)
!^F16:: {
    StopScroll()  ; Stop any active scrolling first
    Click("Left", , 3)
}

; Alt-04: Subdural
!F16:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad6}")
}

; 05 Zoom
F17:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^+z")
}
; Alt-05: Pan
!F17:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^+p")
}

; 06 WL Tool
F18:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("^+w")
}
; Alt-18: Copy (must come before !F18)
!^F18:: {
    StopScroll()  ; Stop any active scrolling first
    SendInput("^c")
    Sleep(50)  ; Brief pause to ensure copy completes
}

; Alt-06: Reset WL
!F18:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("r")
}

; 07 WL Soft
F19:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad1}")
}
; Alt-07: TBD
!F19:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    MsgBox("Alt-F19 - TBD", "Info", "OK T1")
}

; 08 Scroll back slow (Base: Wheel Version 1)
F20:: {
    ToggleWheelScroll("back", "slow")
}
; Alt-08: Scroll back slower (Alt: 400ms)
!F20:: {
    ToggleAltWheelScroll("back", "slow")
}

; 09 WL Brain
F21:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad3}")
}
; Alt-09: WL Stroke
!F21:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Click("Left")
    Sleep(50)
    Send("{Numpad4}")
}

; 10 Arrow Annotation Tool
F22:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^+a")
}
; Alt-10: Oval ROI
!F22:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^!o")
}

; 11 Select
F23:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^+s")
}
; Alt-11: Delete
!F23:: {
    StopScroll()  ; Stop any active scrolling first
    Send("{Delete}")
}

; 12 Scroll back fast (Base: Wheel Version 1)
F24:: {
    ToggleWheelScroll("back", "fast")
}
; Alt-12: Scroll back faster (Alt: 125ms)
!F24:: {
    ToggleAltWheelScroll("back", "fast")
}

; 13: Scroll forward slow (Base: Wheel Version 1)
^F13:: {
    ToggleWheelScroll("forward", "slow")
}
; Alt-13: Scroll forward slower (Alt: 400ms)
!^F13:: {
    ToggleAltWheelScroll("forward", "slow")
}

; 14: Scroll forward fast (Base: Wheel Version 1)
^F14:: {
    ToggleWheelScroll("forward", "fast")
}
; Alt-14: Scroll forward faster (Alt: 125ms)
!^F14:: {
    ToggleAltWheelScroll("forward", "fast")
}

; 15: Measure
^F15:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^!d")
}
; Alt-15: Cine Scroll
!^F15:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^+n")
}

; 16: Cine Scroll
^F16:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("^+n")
}


; 17: Left Mouse Click
^F17:: {
    StopScroll()  ; Stop any active scrolling first
    Click("Left")
}
; Alt-17: Double Left Click
!^F17:: {
    StopScroll()  ; Stop any active scrolling first
    Click("Left", , 2)
}

; 18: Scroll Click Toggle
^F18:: {
    global Toggle
    StopScroll()  ; Stop any active scrolling first
    if (!Toggle) {
        Toggle := true
        SendInput("{Blind}{MButton down}")
        ;ToolTip("[Scroll: ON]")
    } else {
        Toggle := false
        SendInput("{Blind}{MButton up}")
        ;ToolTip("[Scroll: OFF]")
        ;SetTimer(() => ToolTip(), -1000)
    }
}


; 19: Right Mouse Click
^F19:: {
    StopScroll()  ; Stop any active scrolling first
    Click("Right")
}
; Alt-19: Paste Special (Ctrl+Shift+V)
!^F19:: {
    StopScroll()  ; Stop any active scrolling first
    if (A_Clipboard != "") {
        A_Clipboard := RTrim(A_Clipboard, "`r`n")  ; Remove trailing newlines and strip formatting
        Send("^v")  ; Instant paste with no typing effect
        Send("{LAlt up}")  ; Ensure Alt key is released
    }
}

; 20: PowerScribe F4
^F21:: {
    StopScroll()  ; Stop any active scrolling first
    ActivatePowerScribe()
    Send("{F4}")
}

; Ctrl+Alt+D: Same as Position 20 (PowerScribe F4)
^!d:: {
    StopScroll()  ; Stop any active scrolling first
    ActivatePowerScribe()
    Send("{F4}")
}
; Alt-20: TBD
!^F21:: {
    StopScroll()  ; Stop any active scrolling first
    MsgBox("Alt-Position 20 detected: Alt+Control+F21", "Key Detection", "OK")
}

; ===== DPAD KEYS (Shift+F13-F16) =====
; CHANGED: All four directions now send Alt+M to Centricity RA1000.
; Previous scroll-toggle assignments are commented out below each
; hotkey for easy reverting (delete the new block, uncomment the old).

; DPAD Left - Alt+M to Centricity
; (was: Fast Scroll Up toggle)
+F13:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("!m")
}
; +F13:: {
;     ToggleWheelScroll("back", "fast")
; }

; DPAD Right - Alt+M to Centricity
; (was: Fast Scroll Down toggle)
+F14:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("!m")
}
; +F14:: {
;     ToggleWheelScroll("forward", "fast")
; }

; DPAD Up - Alt+M to Centricity
; (was: Slow Scroll Up toggle)
+F15:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("!m")
}
; +F15:: {
;     ToggleWheelScroll("back", "slow")
; }

; DPAD Down - Alt+M to Centricity
; (was: Slow Scroll Down toggle)
+F16:: {
    StopScroll()  ; Stop any active scrolling first
    ActivateCentricity()
    Send("!m")
}
; +F16:: {
;     ToggleWheelScroll("forward", "slow")
; }

; ===== UTILITY KEYS =====

; F1 for testing
F1:: {
    StopScroll()  ; Stop any active scrolling first
    MsgBox("Tartarus mapping script is running!", "Test", "OK T1")
}

; Escape key disabled (no longer exits script)
; Esc:: {
;     StopScroll()  ; Stop any active scrolling first
;     MsgBox("Exiting Tartarus mapping script", "Exit", "OK T1")
;     ExitApp()
; }

; ===== MOUSE BUTTON TOGGLE FUNCTIONALITY =====

; Middle mouse button also triggers scroll toggle
MButton:: {
    global Toggle
    if (!Toggle) {
        Toggle := true
        SendInput("{Blind}{MButton down}")
        ;ToolTip("[Scroll: ON]")
    } else {
        Toggle := false
        SendInput("{Blind}{MButton up}")
        ;ToolTip("[Scroll: OFF]")
        ;SetTimer(() => ToolTip(), -1000)
    }
}

~LButton Up:: {
    global Toggle
    if (Toggle) {
        Toggle := false
        SendInput("{Blind}{MButton up}")
        ;ToolTip("[Scroll: OFF]")
        ;SetTimer(() => ToolTip(), -1000)
    }
}

~RButton Up:: {
    global Toggle
    if (Toggle) {
        Toggle := false
        SendInput("{Blind}{MButton up}")
        ;ToolTip("[Scroll: OFF]")
        ;SetTimer(() => ToolTip(), -1000)
    }
}

~XButton1 Up:: {
    global Toggle
    if (Toggle) {
        Toggle := false
        SendInput("{Blind}{MButton up}")
        ;ToolTip("[Scroll: OFF]")
        ;SetTimer(() => ToolTip(), -1000)
    }
}

~XButton2 Up:: {
    global Toggle
    if (Toggle) {
        Toggle := false
        SendInput("{Blind}{MButton up}")
        ;ToolTip("[Scroll: OFF]")
        ;SetTimer(() => ToolTip(), -1000)
    }
}

