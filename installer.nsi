
!include "MUI2.nsh"

Name "VCV Rack"
OutFile "Rack-setup.exe"
SetCompressor "bzip2"
CRCCheck On

;Default installation folder
InstallDir "$PROGRAMFILES\VCV"

;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\VCV Rack" ""

;Request admin permissions so we can install to Program Files and add a registry entry
RequestExecutionLevel admin



!define MUI_ICON "icon.ico"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "installer-banner.bmp" ; 150x57
; !define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314
; !define MUI_UNWELCOMEFINISHPAGE_BITMAP  "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp" ; 164x314
!define MUI_COMPONENTSPAGE_NODESC


; Pages

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

Var VST_64_DIR
!define MUI_DIRECTORYPAGE_VARIABLE $VST_64_DIR
!define MUI_DIRECTORYPAGE_TEXT_TOP "Bridge 64-bit VST plugin install directory"
!define MUI_PAGE_CUSTOMFUNCTION_PRE VST_64_DIR_PRE
!insertmacro MUI_PAGE_DIRECTORY

Var VST_32_DIR
!define MUI_DIRECTORYPAGE_VARIABLE $VST_32_DIR
!define MUI_DIRECTORYPAGE_TEXT_TOP "Bridge 32-bit VST plugin install directory"
!define MUI_PAGE_CUSTOMFUNCTION_PRE VST_32_DIR_PRE
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"


; Sections

Section "VCV Rack" VCVRACK
	SectionIn RO
	SetOutPath "$INSTDIR"

	CreateDirectory $OUTDIR
	File /r /x "Bridge" "dist\Rack"

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


Section "Bridge 64-bit VST plugin" VST_64
	StrCpy $OUTDIR $VST_64_DIR
	CreateDirectory $OUTDIR
	File "dist\Rack\Bridge\VCV-Bridge-64.dll"
	File "dist\Rack\Bridge\VCV-Bridge-fx-64.dll"
SectionEnd


Section "Bridge 32-bit VST plugin" VST_32
	StrCpy $OUTDIR $VST_32_DIR
	CreateDirectory $OUTDIR
	File "dist\Rack\Bridge\VCV-Bridge-32.dll"
	File "dist\Rack\Bridge\VCV-Bridge-fx-32.dll"
SectionEnd


Section "Uninstall"
	RMDir /r "$INSTDIR\Rack"
	Delete "$INSTDIR\UninstallRack.exe"
	RMDir "$INSTDIR"

	Delete "$SMPROGRAMS\VCV Rack.lnk"

	DeleteRegKey /ifempty HKCU "Software\VCV Rack"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCV Rack"
SectionEnd


; Functions

Function VST_64_DIR_PRE
	${Unless} ${SectionIsSelected} ${VST_64}
		Abort
	${EndUnless}
FunctionEnd

Function VST_32_DIR_PRE
	${Unless} ${SectionIsSelected} ${VST_32}
		Abort
	${EndUnless}
FunctionEnd

Function .onInit
	StrCpy $VST_64_DIR "$PROGRAMFILES\Steinberg\VSTPlugins"
	StrCpy $VST_32_DIR "$PROGRAMFILES (x86)\Steinberg\VSTPlugins"
FunctionEnd
