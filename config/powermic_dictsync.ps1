# powermic_dictsync.ps1 - keep the Tartarus converter's dictate colour in sync
# with what PowerScribe is ACTUALLY doing.
#
# Ground truth: Windows Core Audio capture sessions. While PowerScribe records,
# its process (Nuance.PSOne.exe) holds an ACTIVE session on a microphone
# endpoint; when recording stops - by key 20, the on-screen mic button, a
# timeout, anything - the session goes inactive. That is the same signal the
# real PowerMic II uses (its own mic stream being opened), which this converter
# cannot see because it has no microphone.
#
# On every change, the state is pushed to the firmware over the PowerMic's
# feature-report config channel (cmd 0x09) via hidapitester.exe. The DICTATE
# key still toggles the colour locally for instant feedback; this corrects any
# drift within one poll period.
#
# Usage (from AHK, once at script start):
#   Run('powershell -NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "' . A_ScriptDir . '\powermic_dictsync.ps1"', , "Hide")
# Diagnostics:
#   powershell -File powermic_dictsync.ps1 -Verbose      (prints every session each poll)
#   powershell -File powermic_dictsync.ps1 -Test         (forces on, then off, and exits)
# Log: %TEMP%\powermic_dictsync.log

[CmdletBinding()]
param(
    [string]$HidTester = '',
    [string]$ProcessMatch = 'nuance|psone|powerscribe|dragon|natspeak',
    [int]$PollMs = 300,
    [switch]$Test
)

$ErrorActionPreference = 'Continue'
$log = Join-Path $env:TEMP 'powermic_dictsync.log'
function Log([string]$m) { ("{0:HH:mm:ss.fff} {1}" -f (Get-Date), $m) | Add-Content -Path $log }

# $PSScriptRoot is not populated for param defaults in Windows PowerShell 5.1.
if (-not $HidTester) { $HidTester = Join-Path (Split-Path -Parent $PSCommandPath) 'hidapitester.exe' }
if (-not (Test-Path $HidTester)) {
    $alt = 'J:\Projects\QMK\periloot\hidapitester-windows-x86_64\hidapitester.exe'
    if (Test-Path $alt) { $HidTester = $alt }
}
Log "start; hidtester=$HidTester match=/$ProcessMatch/ poll=${PollMs}ms"

# --- Core Audio interop -------------------------------------------------------
$cs = @'
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
namespace MicWatch {
  [ComImport, Guid("BCDE0395-E52F-467C-8E3D-C4579291692E")] class MMDeviceEnumeratorCom {}
  [ComImport, Guid("A95664D2-9614-4F35-A746-DE8DB63617E6"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  interface IMMDeviceEnumerator { int EnumAudioEndpoints(int dataFlow, int stateMask, out IMMDeviceCollection devices); }
  [ComImport, Guid("0BD7A1BE-7A1A-44DB-8397-CC5392387B5E"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  interface IMMDeviceCollection { int GetCount(out int n); int Item(int i, out IMMDevice d); }
  [ComImport, Guid("D666063F-1587-4E43-81F1-B948E807363F"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  interface IMMDevice {
    int Activate(ref Guid iid, int clsCtx, IntPtr p, [MarshalAs(UnmanagedType.IUnknown)] out object o);
    int OpenPropertyStore(int a, out IntPtr ps); int GetId([MarshalAs(UnmanagedType.LPWStr)] out string id);
  }
  [ComImport, Guid("77AA99A0-1BD6-484F-8BC7-2C654C9A9B6F"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  interface IAudioSessionManager2 {
    int GetAudioSessionControl(IntPtr g, int f, out IntPtr c); int GetSimpleAudioVolume(IntPtr g, int f, out IntPtr v);
    int GetSessionEnumerator(out IAudioSessionEnumerator e);
  }
  [ComImport, Guid("E2F5BB11-0570-40CA-ACDD-3AA01277DEE8"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  interface IAudioSessionEnumerator { int GetCount(out int n); int GetSession(int i, out IAudioSessionControl2 s); }
  [ComImport, Guid("BFB7FF88-7239-4FC9-8FA2-07C950BE9C6D"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  interface IAudioSessionControl2 {
    int GetState(out int s); int GetDisplayName([MarshalAs(UnmanagedType.LPWStr)] out string n);
    int SetDisplayName(IntPtr a, IntPtr b); int GetIconPath(IntPtr a); int SetIconPath(IntPtr a, IntPtr b);
    int GetGroupingParam(IntPtr a); int SetGroupingParam(IntPtr a, IntPtr b);
    int RegisterAudioSessionNotification(IntPtr a); int UnregisterAudioSessionNotification(IntPtr a);
    int GetSessionIdentifier(IntPtr a); int GetSessionInstanceIdentifier(IntPtr a);
    int GetProcessId(out uint pid); int IsSystemSoundsSession(); int SetDuckingPreference(bool b);
  }
  public class Session { public bool Active; public uint Pid; public string Name; }
  public static class Watch {
    public static List<Session> Capture() {
      var list = new List<Session>();
      var en = (IMMDeviceEnumerator)new MMDeviceEnumeratorCom();
      IMMDeviceCollection col; en.EnumAudioEndpoints(1, 1, out col);   // eCapture, DEVICE_STATE_ACTIVE
      int n; col.GetCount(out n);
      for (int i = 0; i < n; i++) {
        IMMDevice d; col.Item(i, out d);
        Guid iid = typeof(IAudioSessionManager2).GUID; object o;
        d.Activate(ref iid, 23, IntPtr.Zero, out o);
        IAudioSessionEnumerator se; ((IAudioSessionManager2)o).GetSessionEnumerator(out se);
        int c; se.GetCount(out c);
        for (int j = 0; j < c; j++) {
          IAudioSessionControl2 s; se.GetSession(j, out s);
          int st; s.GetState(out st); uint pid; s.GetProcessId(out pid);
          string name = ""; try { name = System.Diagnostics.Process.GetProcessById((int)pid).ProcessName; } catch {}
          list.Add(new Session { Active = (st == 1), Pid = pid, Name = name });
        }
      }
      return list;
    }
  }
}
'@
if (-not ('MicWatch.Watch' -as [type])) { Add-Type -TypeDefinition $cs }

function Send-DictState([bool]$on) {
    if (-not (Test-Path $HidTester)) { Log "hidapitester missing: $HidTester"; return }
    $v = if ($on) { '1' } else { '0' }
    $args = @('--vidpid','0554:1001','--usagePage','0x01','--usage','0x00','--open','--length','40','--send-feature',"0x00,0x09,0x0$v",'--close')
    try {
        $p = Start-Process -FilePath $HidTester -ArgumentList $args -WindowStyle Hidden -PassThru -Wait
        Log ("sent dictate={0} (exit {1})" -f $v, $p.ExitCode)
    } catch { Log "send failed: $_" }
}

if ($Test) {
    Log "TEST: forcing on"; Send-DictState $true; Start-Sleep -Seconds 3
    Log "TEST: forcing off"; Send-DictState $false
    Write-Output "test done - see $log"; exit 0
}

$last = $null
while ($true) {
    $dictating = $false
    try {
        $sessions = [MicWatch.Watch]::Capture()
        foreach ($s in $sessions) {
            if ($VerbosePreference -eq 'Continue') { Write-Verbose ("{0} pid={1} {2}" -f ($(if($s.Active){'ACTIVE  '}else{'inactive'})), $s.Pid, $s.Name) }
            if ($s.Active -and $s.Name -match $ProcessMatch) { $dictating = $true }
        }
    } catch { Log "enumerate failed: $_" }

    if ($dictating -ne $last) {
        $names = ($sessions | Where-Object { $_.Active } | ForEach-Object { $_.Name }) -join ','
        Log ("state -> {0} (active capture: {1})" -f $(if($dictating){'DICTATING'}else{'idle'}), $names)
        Send-DictState $dictating
        $last = $dictating
    }
    Start-Sleep -Milliseconds $PollMs
}
