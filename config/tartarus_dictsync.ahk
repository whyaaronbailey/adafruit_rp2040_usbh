; tartarus_dictsync.ahk - keep the converter's dictate colour in sync with what
; PowerScribe is ACTUALLY doing. Pure AutoHotkey v2: no PowerShell, no
; hidapitester, no helper process. Add ONE line to tartarus.ahk:
;
;     #Include tartarus_dictsync.ahk
;
; How it works
;   PowerScribe never tells the PowerMic whether it is recording (the USB
;   capture shows zero host->HID traffic). The genuine handset infers it from
;   its own microphone stream being opened - which this converter, having no
;   microphone, cannot see. So we read the same fact from Windows Core Audio:
;   while PowerScribe (Nuance.PSOne.exe) records, it holds an ACTIVE capture
;   session on a microphone endpoint; when recording stops - by key 20, the
;   on-screen mic button, a timeout, anything - the session goes inactive.
;
;   On every change the state is written to the firmware over the PowerMic's
;   own feature report (config cmd 0x09) with HidD_SetFeature. The DICTATE
;   key still toggles the colour locally for instant feedback; this corrects
;   any drift within one poll period (300 ms).
;
; Diagnostics: set DictSync_Debug := true and watch %USERPROFILE%\tartarus_dictsync.log

#Requires AutoHotkey v2.0

global DictSync_ProcMatch := "nuance|psone|powerscribe|dragon|natspeak"
global DictSync_PollMs    := 300
global DictSync_Debug     := false
global DictSync_Last      := -1

SetTimer(DictSync_Poll, DictSync_PollMs)

DictSync_Poll() {
    global DictSync_Last, DictSync_ProcMatch
    on := DictSync_MicActive(DictSync_ProcMatch)
    if (on = DictSync_Last)
        return
    if DictSync_HidSetFeature(0x0554, 0x1001, 0x01, 0x00, [0x00, 0x09, on ? 1 : 0]) {
        DictSync_Last := on
        DictSync_Log("state -> " . (on ? "DICTATING" : "idle"))
    } else {
        DictSync_Log("converter not found / write failed")
    }
}

; ---- Core Audio: is any ACTIVE capture session owned by a matching process? ----
DictSync_MicActive(procRegex) {
    static CLSID_Enum := "{BCDE0395-E52F-467C-8E3D-C4579291692E}"
    static IID_Enum   := "{A95664D2-9614-4F35-A746-DE8DB63617E6}"   ; IMMDeviceEnumerator
    static IID_Mgr2   := "{77AA99A0-1BD6-484F-8BC7-2C654C9A9B6F}"   ; IAudioSessionManager2
    static IID_Ctl2   := "{BFB7FF88-7239-4FC9-8FA2-07C950BE9C6D}"   ; IAudioSessionControl2
    static eCapture := 1, DEVICE_STATE_ACTIVE := 1, CLSCTX_ALL := 23
    try {
        en := ComObject(CLSID_Enum, IID_Enum)
        ComCall(3, en, "int", eCapture, "uint", DEVICE_STATE_ACTIVE, "ptr*", &pCol := 0)  ; EnumAudioEndpoints
        col := ComValue(13, pCol)
        ComCall(3, col, "uint*", &nDev := 0)                                              ; GetCount
        iid := Buffer(16), DllCall("ole32\CLSIDFromString", "wstr", IID_Mgr2, "ptr", iid)
        loop nDev {
            ComCall(4, col, "uint", A_Index - 1, "ptr*", &pDev := 0)                      ; Item
            dev := ComValue(13, pDev)
            ComCall(3, dev, "ptr", iid, "uint", CLSCTX_ALL, "ptr", 0, "ptr*", &pMgr := 0) ; Activate(IAudioSessionManager2)
            mgr := ComValue(13, pMgr)
            ComCall(5, mgr, "ptr*", &pSe := 0)                                            ; GetSessionEnumerator
            se := ComValue(13, pSe)
            ComCall(3, se, "int*", &nSes := 0)                                            ; GetCount
            loop nSes {
                ComCall(4, se, "int", A_Index - 1, "ptr*", &pS := 0)                      ; GetSession
                s  := ComValue(13, pS)
                s2 := ComObjQuery(s, IID_Ctl2)
                ComCall(3, s2, "int*", &state := 0)                                       ; GetState (1 = active)
                if (state != 1)
                    continue
                ComCall(14, s2, "uint*", &pid := 0)                                       ; GetProcessId
                name := ""
                try name := ProcessGetName(pid)
                DictSync_Log("active capture: pid=" . pid . " " . name)
                if RegExMatch(name, "i)" . procRegex)
                    return true
            }
        }
    } catch as e {
        DictSync_Log("core audio error: " . e.Message)
    }
    return false
}

; ---- HID: write a feature report to the first VID/PID collection with the given usage ----
DictSync_HidSetFeature(vid, pid, usagePage, usage, bytes) {
    static GUID_HID := "{4D1E55B2-F16F-11CF-88CB-001111000030}"
    static INVALID_HANDLE := -1, GENERIC_RW := 0xC0000000, SHARE_RW := 3, OPEN_EXISTING := 3
    guid := Buffer(16), DllCall("ole32\CLSIDFromString", "wstr", GUID_HID, "ptr", guid)
    if DllCall("cfgmgr32\CM_Get_Device_Interface_List_SizeW", "uint*", &len := 0, "ptr", guid, "ptr", 0, "uint", 0)
        return false
    list := Buffer(len * 2, 0)
    if DllCall("cfgmgr32\CM_Get_Device_Interface_ListW", "ptr", guid, "ptr", 0, "ptr", list, "uint", len, "uint", 0)
        return false
    tag := Format("vid_{:04x}&pid_{:04x}", vid, pid)
    off := 0
    while (off < len * 2) {
        path := StrGet(list.Ptr + off, "UTF-16")
        if (path = "")
            break
        off += (StrLen(path) + 1) * 2
        if !InStr(path, tag)
            continue
        h := DllCall("CreateFileW", "wstr", path, "uint", GENERIC_RW, "uint", SHARE_RW, "ptr", 0, "uint", OPEN_EXISTING, "uint", 0, "ptr", 0, "ptr")
        if (h = INVALID_HANDLE)
            continue
        ok := false
        if DllCall("hid\HidD_GetPreparsedData", "ptr", h, "ptr*", &pp := 0) {
            caps := Buffer(64, 0)                       ; HIDP_CAPS: Usage@0, UsagePage@2, FeatureReportByteLength@8
            DllCall("hid\HidP_GetCaps", "ptr", pp, "ptr", caps)
            DllCall("hid\HidD_FreePreparsedData", "ptr", pp)
            if (NumGet(caps, 2, "ushort") = usagePage && NumGet(caps, 0, "ushort") = usage) {
                flen := NumGet(caps, 8, "ushort")
                rep := Buffer(flen, 0)
                for i, b in bytes {
                    if (i <= flen)
                        NumPut("uchar", b, rep, i - 1)
                }
                ok := DllCall("hid\HidD_SetFeature", "ptr", h, "ptr", rep, "uint", flen)
            }
        }
        DllCall("CloseHandle", "ptr", h)
        if ok
            return true
    }
    return false
}

DictSync_Log(msg) {
    global DictSync_Debug
    if !DictSync_Debug
        return
    try FileAppend(FormatTime(, "HH:mm:ss") . " " . msg . "`n", EnvGet("USERPROFILE") . "\tartarus_dictsync.log")
}
