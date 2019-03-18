!include "MUI2.nsh"

Name "VCV Rack ${VERSION}"
OutFile "installer.exe"
SetCompressor /solid "lzma"
CRCCheck On

; Default installation folder
InstallDir "$PROGRAMFILES\VCV\Rack"

; Get installation folder from registry if available
InstallDirRegKey HKLM "Software\VCV\Rack" ""

; Request admin permissions so we can install to Program Files and add a registry entry
RequestExecutionLevel admin


; MUI pages

!define MUI_ICON "icon.ico"
!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "installer-banner.bmp" ; 150x57
;!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314
;!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314

!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\Rack.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch VCV Rack"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"


; Sections

Section "VCV Rack" VCV_RACK_SECTION
	SectionIn RO
	SetOutPath "$INSTDIR"

	File /r "dist\Rack\*"

	; Store installation folder
	WriteRegStr HKLM "Software\VCV\Rack" "" "$INSTDIR"

	; Write uninstaller info
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "DisplayName" "VCV Rack"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "DisplayIcon" '"$INSTDIR\Rack.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "InstallLocation" '"$INSTDIR"'
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "Publisher" "VCV"
	SectionGetSize ${VCV_RACK_SECTION} $0
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "EstimatedSize" $0
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "NoRepair" 1

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Associate file type
	WriteRegStr HKLM "Software\Classes\.vcv" "" "VCVRack.Patch"
	WriteRegStr HKLM "Software\Classes\VCVRack.Patch" "" "VCV Rack Patch"
	WriteRegStr HKLM "Software\Classes\VCVRack.Patch\shell\open\command" "" '"$INSTDIR\Rack.exe" "%1"'

	; Create shortcuts
	CreateDirectory "$SMPROGRAMS"
	CreateShortcut "$SMPROGRAMS\VCV Rack.lnk" "$INSTDIR\Rack.exe"
SectionEnd


Section "Uninstall"
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\*"
	RMDir /r "$INSTDIR\res"
	RMDir "$INSTDIR"
	RMDir "$INSTDIR\.."

	Delete "$SMPROGRAMS\VCV Rack.lnk"

	DeleteRegKey HKLM "Software\VCV\Rack"
	DeleteRegKey /ifempty HKLM "Software\VCV"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack"
	DeleteRegKey HKLM "Software\Classes\.vcv"
	DeleteRegKey HKLM "Software\Classes\VCVRack.Patch"
SectionEnd


; Functions
