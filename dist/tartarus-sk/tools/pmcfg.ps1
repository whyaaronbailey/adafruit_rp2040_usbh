# pmcfg.ps1 - talk to the PowerMic feature-report config channel via pure Win32.
# Usage:  . .\pmcfg.ps1 ;  PM-Cmd 0x02,0x00,0x03,0x06     (returns byte[] response)
$script:pmcs = @'
using System; using System.Runtime.InteropServices; using Microsoft.Win32.SafeHandles;
public static class PmCfg {
  [DllImport("cfgmgr32.dll", CharSet=CharSet.Unicode)] static extern int CM_Get_Device_Interface_List_SizeW(out uint len, ref Guid g, IntPtr id, uint flags);
  [DllImport("cfgmgr32.dll", CharSet=CharSet.Unicode)] static extern int CM_Get_Device_Interface_ListW(ref Guid g, IntPtr id, char[] buf, uint len, uint flags);
  [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)] static extern SafeFileHandle CreateFileW(string n, uint a, uint sh, IntPtr sa, uint cd, uint fl, IntPtr t);
  [DllImport("hid.dll", SetLastError=true)] static extern bool HidD_SetFeature(SafeFileHandle h, byte[] b, uint len);
  [DllImport("hid.dll", SetLastError=true)] static extern bool HidD_GetFeature(SafeFileHandle h, byte[] b, uint len);
  [DllImport("hid.dll")] static extern bool HidD_GetPreparsedData(SafeFileHandle h, out IntPtr p);
  [DllImport("hid.dll")] static extern bool HidD_FreePreparsedData(IntPtr p);
  [DllImport("hid.dll")] static extern int HidP_GetCaps(IntPtr p, IntPtr c);
  static SafeFileHandle Open(out int featLen) {
    featLen = 0; Guid g = new Guid("4D1E55B2-F16F-11CF-88CB-001111000030");
    uint len; CM_Get_Device_Interface_List_SizeW(out len, ref g, IntPtr.Zero, 0);
    var buf = new char[len]; CM_Get_Device_Interface_ListW(ref g, IntPtr.Zero, buf, len, 0);
    foreach (var path in new string(buf).Split('\0')) {
      if (path.Length == 0 || path.IndexOf("vid_0554&pid_1001", StringComparison.OrdinalIgnoreCase) < 0) continue;
      var h = CreateFileW(path, 0xC0000000, 3, IntPtr.Zero, 3, 0, IntPtr.Zero);
      if (h.IsInvalid) continue;
      IntPtr pp; HidD_GetPreparsedData(h, out pp); IntPtr caps = Marshal.AllocHGlobal(64); HidP_GetCaps(pp, caps);
      ushort usage=(ushort)Marshal.ReadInt16(caps,0), page=(ushort)Marshal.ReadInt16(caps,2); featLen=(ushort)Marshal.ReadInt16(caps,8);
      Marshal.FreeHGlobal(caps); HidD_FreePreparsedData(pp);
      if (page==0x01 && usage==0x00) return h;
      h.Close();
    }
    return null;
  }
  public static byte[] Cmd(byte[] req) {
    int flen; var h = Open(out flen);
    if (h == null) throw new Exception("PowerMic config collection not found");
    try {
      var sb = new byte[flen]; sb[0] = 0x00;
      Array.Copy(req, 0, sb, 1, Math.Min(req.Length, flen - 1));
      if (!HidD_SetFeature(h, sb, (uint)flen)) throw new Exception("SetFeature failed err=" + Marshal.GetLastWin32Error());
      var gb = new byte[flen]; gb[0] = 0x00;
      if (!HidD_GetFeature(h, gb, (uint)flen)) throw new Exception("GetFeature failed err=" + Marshal.GetLastWin32Error());
      var res = new byte[flen - 1]; Array.Copy(gb, 1, res, 0, flen - 1); return res;
    } finally { h.Close(); }
  }
}
'@
if (-not ('PmCfg' -as [type])) { Add-Type -TypeDefinition $script:pmcs }

function PM-Cmd { param([byte[]]$Req) [PmCfg]::Cmd($Req) }

function PM-GetKey { param([int]$Layer, [int]$Row, [int]$Col)
    $r = [PmCfg]::Cmd([byte[]](0x02, $Layer, $Row, $Col))
    if ($r[1] -ne 1) { return 'ERR' }
    '{0:X2}{1:X2}' -f $r[2], $r[3]
}
function PM-SetKey { param([int]$Layer, [int]$Row, [int]$Col, [int]$Keycode)
    $r = [PmCfg]::Cmd([byte[]](0x03, $Layer, $Row, $Col, ($Keycode -shr 8), ($Keycode -band 0xFF)))
    $r[1] -eq 1
}
function PM-Save { $r = [PmCfg]::Cmd([byte[]](0x07)); $r[1] -eq 1 }
function PM-ResetKeys { $r = [PmCfg]::Cmd([byte[]](0x08)); $r[1] -eq 1 }
function PM-DictSync { param([bool]$On) $r = [PmCfg]::Cmd([byte[]](0x09, $(if($On){1}else{0}))); $r[1] -eq 1 }
function PM-Proto { $r = [PmCfg]::Cmd([byte[]](0x01)); [pscustomobject]@{ Proto=$r[1]; Layers=$r[2]; Rows=$r[3]; Cols=$r[4] } }
