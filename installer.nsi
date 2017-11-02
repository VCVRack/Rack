
!include "MUI2.nsh"

Name "VCV Rack"
OutFile "Rack-setup.exe"
SetCompressor "bzip2"
CRCCheck On

;Default installation folder
InstallDir "$PROGRAMFILES\VCV"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\VCV Rack" ""

;Request application privileges for Windows Vista
RequestExecutionLevel admin



!define MUI_ICON "icon.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "installer-banner.bmp" ; 150x57
; !define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314
; !define MUI_UNWELCOMEFINISHPAGE_BITMAP  "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314
!define MUI_COMPONENTSPAGE_NODESC


; Pages

; !insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

;second directory selection
; Var DbInstDir
; !define MUI_PAGE_HEADER_SUBTEXT "Choose the folder in which to install the database."
; !define MUI_DIRECTORYPAGE_TEXT_TOP "The installer will install the database(s) in the following folder. To install in a differenct folder, click Browse and select another folder. Click Next to continue."
; !define MUI_DIRECTORYPAGE_VARIABLE $DbInstDir ; <= the other directory will be stored into that variable
; !insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"



Section "!VCV Rack" VCVRACK
	SetOutPath "$INSTDIR"

	File /r "dist\Rack"

	;Store installation folder
	WriteRegStr HKCU "Software\VCV Rack" "" $INSTDIR
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "DisplayName" "VCV Rack"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "UninstallString" "$\"$INSTDIR\UninstallRack.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "QuietUninstallString" "$\"$INSTDIR\UninstallRack.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack" "Publisher" "VCV"

	;Create uninstaller
	WriteUninstaller "$INSTDIR\UninstallRack.exe"

	;Create shortcuts
	CreateDirectory "$SMPROGRAMS"
	; Set working directory of shortcut
	SetOutPath "$INSTDIR\Rack"
	CreateShortcut "$SMPROGRAMS\VCV Rack.lnk" "$INSTDIR\Rack\Rack.exe"
SectionEnd


; Section "VST Plugin" VST
; SectionEnd


Section "Uninstall"
	RMDir /r "$INSTDIR\Rack"
	Delete "$INSTDIR\UninstallRack.exe"
	RMDir "$INSTDIR"

	Delete "$SMPROGRAMS\VCV Rack.lnk"

	DeleteRegKey /ifempty HKCU "Software\VCV Rack"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack"
SectionEnd
