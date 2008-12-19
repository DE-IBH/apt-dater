Dim sh, adproto, lsbrel, regkey, envos

adproto = "0.2"

Set sh = CreateObject("WScript.Shell")

envos = sh.ExpandEnvironmentStrings("%OS%")
if envos = "Windows_NT" Then
	regkey = "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\"
	lsbrel = sh.regread(regkey  & "ProductName") & " " & sh.regread(regkey & "CurrentVersion") & "." & sh.regread(regkey & "CurrentBuildNumber")
else
	regkey = "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\"
	lsbrel = sh.regread(regkey & "ProductName") & " " & sh.regread(regkey & "VersionNumber")
end if

WScript.Echo "ADPROTO: " & adproto
WScript.Echo "LSBREL: " & lsbrel & "||"

Set updateSession = CreateObject("Microsoft.Update.Session")
Set updateSearcher = updateSession.CreateupdateSearcher()

Set searchResult = _
	updateSearcher.Search("IsInstalled=0 and Type='Software'")


For I = 0 To searchResult.Updates.Count-1
	Set update = searchResult.Updates.Item(I)
	WScript.Echo "STATUS: " & update.Title & "|0|u=1"
Next
