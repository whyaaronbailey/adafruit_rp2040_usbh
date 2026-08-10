; tartarus_led.ahk — LED feedback for the Tartarus converter (AHK build)
; #Include this from tartarus.ahk, then call the three functions below from
; your handlers. Requires the converter to run the tartarus2_ahk (or _qmk)
; VIA firmware, which exposes RAW HID LED opcodes:
;   0xC8 [on]                 dictate state -> all keys red while on
;   0xC9 [on, led, half/10]   scroll pulse on the given key (led = key# - 1)
;
; Transport is hidapitester (https://github.com/todbot/hidapitester); set the
; path below to wherever it lives on this workstation.
#Requires AutoHotkey v2.0

global TartarusHidTester := "C:\Tools\hidapitester\hidapitester.exe"

; Fire a RAW HID report [0x00, opcodes...] at the converter, non-blocking.
_TartarusSend(bytes) {
    global TartarusHidTester
    if !FileExist(TartarusHidTester)
        return
    payload := "0"
    for b in bytes
        payload .= "," . b
    ; pad to the 33-byte report hidapitester expects
    loop 32 - bytes.Length
        payload .= ",0"
    Run(Format('"{1}" --vidpid 239A:0001 --usagePage 0xFF60 --usage 0x61 --open --length 33 --send-output {2}',
        TartarusHidTester, payload), , "Hide")
}

; Call with true when dictation starts (all keys red), false when it ends.
TartarusDictateLED(on) {
    _TartarusSend([0xC8, on ? 1 : 0])
}

; Call when toggling continuous scroll. keyNumber = physical key 1..20 that
; started the scroll (it pulses); halfMs = pulse half-period in ms (e.g. 350
; slow / 175 fast — match your wheel delays). Call TartarusScrollLED(false)
; from StopScroll().
TartarusScrollLED(on, keyNumber := 0, halfMs := 300) {
    if (on && keyNumber >= 1 && keyNumber <= 20)
        _TartarusSend([0xC9, 1, keyNumber - 1, Floor(halfMs / 10)])
    else
        _TartarusSend([0xC9, 0, 0, 0])
}

; ===== Integration cheat-sheet (add to tartarus.ahk) =====
; At the top:
;     #Include tartarus_led.ahk
; In StopScroll(), inside the "if (scrolling)" body:
;     TartarusScrollLED(false)
; In ToggleWheelScroll()/ToggleAltWheelScroll(), after "scrolling := true":
;     TartarusScrollLED(true, <key#>, delay)   ; e.g. 8 for F20, 12 for F24,
;                                              ; 13 for ^F13, 14 for ^F14
; In the ^F21 (PowerScribe) handler, if you track a dictate toggle:
;     TartarusDictateLED(dictating)
