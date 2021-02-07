!include "MUI2.nsh"

Name "VCV Rack ${VERSION}"
OutFile "installer.exe"
SetCompressor /solid "lzma"
SetCompressorDictSize 8
CRCCheck On

; Default installation folder
InstallDir "$PROGRAMFILES\VCV\Rack2"

; Get installation folder from registry if available
InstallDirRegKey HKLM "Software\VCV\Rack2" ""

; Request admin permissions so we can install to Program Files and add a registry entry
RequestExecutionLevel admin


; MUI installer pages

!define MUI_ICON "icon.ico"
;!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "installer-banner.bmp" ; 150x57
;!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314
;!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314

!define MUI_COMPONENTSPAGE_NODESC
;!insertmacro MUI_PAGE_COMPONENTS

; Prevent user from choosing an installation directory that already exists, such as C:\Program Files.
; This is necessary because the uninstaller removes the installation directory, which is dangerous for directories that existed before Rack was installed.
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE directoryLeave
Function directoryLeave
	IfFileExists "$INSTDIR" directoryExists directoryNotExists
	directoryExists:
		MessageBox MB_OK|MB_ICONSTOP "Cannot install to $INSTDIR. Folder already exists."
		Abort
	directoryNotExists:
FunctionEnd
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\Rack.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch VCV Rack"
!insertmacro MUI_PAGE_FINISH

; MUI uninstaller pages

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"


; Sections

Section "Install" INSTALL_SECTION
	SectionIn RO
	SetOutPath "$INSTDIR"

	File /r "dist\Rack\*"

	; Store installation folder
	WriteRegStr HKLM "Software\VCV\Rack2" "" "$INSTDIR"

	; Write uninstaller info
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "DisplayName" "VCV Rack 2"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "DisplayIcon" '"$INSTDIR\Rack.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "InstallLocation" '"$INSTDIR"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "Publisher" "VCV"
	SectionGetSize ${INSTALL_SECTION} $0
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "EstimatedSize" $0
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2" "NoRepair" 1

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Associate file type
	WriteRegStr HKLM "Software\Classes\.vcv" "" "VCVRack.Patch"
	WriteRegStr HKLM "Software\Classes\VCVRack.Patch" "" "VCV Rack Patch"
	WriteRegStr HKLM "Software\Classes\VCVRack.Patch\shell\open\command" "" '"$INSTDIR\Rack.exe" "%1"'

	; Create shortcuts
	CreateShortcut "$DESKTOP\VCV Rack 2.lnk" "$INSTDIR\Rack.exe"
	CreateShortcut "$SMPROGRAMS\VCV Rack 2.lnk" "$INSTDIR\Rack.exe"
SectionEnd


Section "Uninstall"
	; directoryLeave above ensures that INSTDIR is safe to remove.
	RMDir /r "$INSTDIR"
	; Attempt to remove C:\Program Files\VCV if empty
	RMDir "$INSTDIR\.."

	Delete "$DESKTOP\VCV Rack 2.lnk"
	Delete "$SMPROGRAMS\VCV Rack 2.lnk"

	DeleteRegKey HKLM "Software\VCV\Rack2"
	DeleteRegKey /ifempty HKLM "Software\VCV"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack2"
	DeleteRegKey HKLM "Software\Classes\.vcv"
	DeleteRegKey HKLM "Software\Classes\VCVRack.Patch"
SectionEnd
