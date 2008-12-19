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

  Set updateSession = CreateObject("Microsoft.Update.Session")
  Set updateSearcher = updateSession.CreateupdateSearcher()

  Set searchResult = updateSearcher.Search("IsInstalled=0 and Type='Software'")

  For I = 0 To searchResult.Updates.Count-1
    Set update = searchResult.Updates.Item(I)
    WScript.Echo "STATUS: " & update.Title & "|0|u=1"
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
