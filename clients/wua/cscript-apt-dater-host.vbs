'~ apt-dater - terminal-based remote package update manager
'~
'~ $Id$
'~
'~ Authors:
'~   Andre Ellguth <ellguth@ibh.de>
'~   Thomas Liske <liske@ibh.de>
'~
'~ Copyright Holder:
'~   2008 (C) IBH IT-Service GmbH [http://www.ibh.de/apt-dater/]
'~
'~ License:
'~   This program is free software; you can redistribute it and/or modify
'~   it under the terms of the GNU General Public License as published by
'~   the Free Software Foundation; either version 2 of the License, or
'~   (at your option) any later version.
'~
'~   This program is distributed in the hope that it will be useful,
'~   but WITHOUT ANY WARRANTY; without even the implied warranty of
'~   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'~   GNU General Public License for more details.
'~
'~   You should have received a copy of the GNU General Public License
'~   along with this package; if not, write to the Free Software
'~   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
'~

Dim sh, adproto

adproto = "0.2"

If WScript.Arguments.Count < 1 Then
  WScript.Echo "Don't call this script directly!"
  WScript.Quit
End If

Set sh = CreateObject("WScript.Shell")
cmd = WScript.Arguments.Item(0)


If cmd = "refresh" Then
  WScript.Echo "ADPROTO: " & adproto
  WScript.Echo
	WScript.Echo "Sorry, apt-dater based refreshs on this host are disabled!"
  WScript.Echo
  do_status
  do_kernel
ElseIf cmd = "status" Then
  WScript.Echo "ADPROTO: " & adproto
  do_status
  do_kernel
ElseIf cmd = "upgrade" Then
  WScript.Echo
	WScript.Echo "** Sorry, apt-dater based upgrades on this host are disabled! **"
  WScript.Echo
ElseIf cmd = "install" Then
  WScript.Echo
	WScript.Echo "** Sorry, apt-dater based upgrades on this host are disabled! **"
  WScript.Echo
ElseIf cmd = "kernel" Then
  WScript.Echo "ADPROTO: " & adproto
  do_kernel
Else
  WScript.Echo "IInvalid command " & cmd & "!"
End If


Sub do_status()
  If sh.ExpandEnvironmentStrings("%OS%") = "Windows_NT" Then
    regkey = "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\"
    lsbrel = sh.regread(regkey  & "ProductName") & "|" & sh.regread(regkey & "CurrentVersion") & "|" & sh.regread(regkey & "CurrentBuildNumber")
  Else
    regkey = "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\"
    lsbrel = sh.regread(regkey & "ProductName") & "|" & sh.regread(regkey & "VersionNumber") & "|"
  End If

  WScript.Echo "LSBREL: " & lsbrel
  WScript.Echo "FORBID: 7"

  get_installed

  Set updateSession = CreateObject("Microsoft.Update.Session")
  Set updateSearcher = updateSession.CreateupdateSearcher()

  Set searchResult = updateSearcher.Search("IsInstalled=0 and Type='Software'")

  For I = 0 To searchResult.Updates.Count-1
    Set update = searchResult.Updates.Item(I)
    WScript.Echo "STATUS: " & update.Title & "|n/i|u=n/a"
  Next
End Sub


Sub do_kernel()
  If sh.ExpandEnvironmentStrings("%OS%") = "Windows_NT" Then
    kernelinfo = sh.regread("HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\BuildLab")
  Else
    kernelinfo = sh.regread("HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\BuildLab")
  End If

  Set updateSession = CreateObject("Microsoft.Update.Session")
  Set updateSearcher = updateSession.CreateupdateSearcher()
  Set searchResult = updateSearcher.Search("RebootRequired=1")

  If searchResult.Updates.Count > 0 then
    WScript.Echo "KERNELINFO: " & kernelinfo & " 1"
  Else
    WScript.Echo "KERNELINFO: " & kernelinfo & " 0"
  End If
End Sub

Sub get_installed()
  Const HKLM = &H80000002 'HKEY_LOCAL_MACHINE
  strComputer = "."
  strKey = "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\"
  strEntry1a = "DisplayName"
  strEntry1b = "QuietDisplayName"
  strEntry2 = "DisplayVersion"
  strEntry3 = "VersionMajor"
  strEntry4 = "VersionMinor"

  Set objReg = GetObject("winmgmts://" & strComputer & "/root/default:StdRegProv")
  objReg.EnumKey HKLM, strKey, arrSubkeys

  For Each strSubkey In arrSubkeys
    ret = objReg.GetStringValue(HKLM, strKey & strSubkey, strEntry1a, strProgName)
    If ret <> 0 Then
      objReg.GetStringValue HKLM, strKey & strSubkey, strEntry1b, strProgName
    End If
  
    If strProgName <> "" Then
      ret = objReg.GetStringValue(HKLM, strKey & strSubkey, strEntry2, strVersion)
      if ret <> 0 Then
        objReg.GetDWORDValue HKLM, strKey & strSubkey, strEntry3, intValue3
        objReg.GetDWORDValue HKLM, strKey & strSubkey, strEntry4, intValue4
        If intValue3 <> "" Then
          strVersion = intValue3 & "." & intValue4
        Else
          strVersion = "n/a"
        End If
      End If

      WScript.Echo "STATUS: " & strProgName & "|" & strVersion & "|i"
    End If
  Next
End Sub
