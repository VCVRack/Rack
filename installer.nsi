!include "MUI2.nsh"

!define NAME_FULL "VCV Rack Free ${VERSION}"
!define NAME "VCV Rack ${VERSION_MAJOR} Free"
!define RACK_DIR "Rack${VERSION_MAJOR}Free"
!define INSTALL_REG "Software\VCV\Rack${VERSION_MAJOR}Free"
!define UNINSTALL_REG "Software\Microsoft\Windows\CurrentVersion\Uninstall\VCVRack${VERSION_MAJOR}Free"

Name "${NAME_FULL}"
Unicode True
SetCompressor /solid "lzma"
SetCompressorDictSize 8
CRCCheck On

; Default installation folder
InstallDir "$PROGRAMFILES\VCV\${RACK_DIR}"
; Get installation folder from registry if available
InstallDirRegKey HKLM "${INSTALL_REG}" ""

; Request admin permissions so we can install to Program Files and add a registry entry
RequestExecutionLevel admin


; MUI installer pages

!define MUI_ICON "icon.ico"
;!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "installer-banner.bmp" ; 150x57
;!define MUI_WELCOMEFINISHPAGE_BITMAP "$NSISDIR\Contrib\Graphics\Wizard\win.bmp" ; 164x314
;!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "$NSISDIR\Contrib\Graphics\Wizard\win.bmp" ; 164x314

!define MUI_COMPONENTSPAGE_NODESC
;!insertmacro MUI_PAGE_COMPONENTS

; Prevent user from choosing an installation directory that already exists, such as C:\Program Files.
; This is necessary because the uninstaller removes the installation directory, which is dangerous for directories that existed before Rack was installed.
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE directoryLeave
Function directoryLeave
	StrCmp "$INSTDIR" "$PROGRAMFILES" directoryBad
	StrCmp "$INSTDIR" "$PROGRAMFILES32" directoryBad
	StrCmp "$INSTDIR" "$PROGRAMFILES64" directoryBad
	StrCmp "$INSTDIR" "$COMMONFILES" directoryBad
	StrCmp "$INSTDIR" "$COMMONFILES32" directoryBad
	StrCmp "$INSTDIR" "$COMMONFILES64" directoryBad
	StrCmp "$INSTDIR" "$DESKTOP" directoryBad
	StrCmp "$INSTDIR" "$WINDIR" directoryBad
	StrCmp "$INSTDIR" "$SYSDIR" directoryBad
	StrCmp "$INSTDIR" "$DOCUMENTS" directoryBad
	StrCmp "$INSTDIR" "$MUSIC" directoryBad
	StrCmp "$INSTDIR" "$PICTURES" directoryBad
	StrCmp "$INSTDIR" "$VIDEOS" directoryBad
	StrCmp "$INSTDIR" "$APPDATA" directoryBad
	StrCmp "$INSTDIR" "$LOCALAPPDATA" directoryBad
	Return
	directoryBad:
		MessageBox MB_OK|MB_ICONSTOP "Cannot install to $INSTDIR."
		Abort
FunctionEnd
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN "$INSTDIR\Rack.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${NAME}"
!insertmacro MUI_PAGE_FINISH

; MUI uninstaller pages

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"


; Sections

Section "${NAME}" INSTALL_SECTION
	SectionIn RO
	SetOutPath "$INSTDIR"

	# Uninstall existing version silently before installing.
	# This is needed because the VST3 adapter used to be a file, and now it is a bundle (folder), and NSIS can't overwrite files with folders.
	# "_?=" makes the uninstaller block until exit. Explanation at bottom of page:
	# https://nsis.sourceforge.io/Docs/Chapter3.html
	# Fails gracefully if uninstaller does not exist.
	DetailPrint "Uninstalling existing version"
	ExecWait '"$INSTDIR\Uninstall.exe" /S _?=$INSTDIR'

	File /r "dist\${RACK_DIR}\*"

	; Store installation folder
	WriteRegStr HKLM "${INSTALL_REG}" "" "$INSTDIR"

	; Write uninstaller info
	WriteRegStr HKLM "${UNINSTALL_REG}" "DisplayName" "${NAME}"
	WriteRegStr HKLM "${UNINSTALL_REG}" "DisplayIcon" '"$INSTDIR\Rack.exe"'
	WriteRegStr HKLM "${UNINSTALL_REG}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "${UNINSTALL_REG}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegStr HKLM "${UNINSTALL_REG}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
	WriteRegStr HKLM "${UNINSTALL_REG}" "InstallLocation" '"$INSTDIR"'
	WriteRegStr HKLM "${UNINSTALL_REG}" "Publisher" "VCV"
	SectionGetSize ${INSTALL_SECTION} $0
	WriteRegDWORD HKLM "${UNINSTALL_REG}" "EstimatedSize" $0
	WriteRegDWORD HKLM "${UNINSTALL_REG}" "NoModify" 1
	WriteRegDWORD HKLM "${UNINSTALL_REG}" "NoRepair" 1

	; Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	; Associate file type
	WriteRegStr HKLM "Software\Classes\.vcv" "" "VCVRack.Patch"
	WriteRegStr HKLM "Software\Classes\VCVRack.Patch" "" "VCV Rack patch"
	WriteRegStr HKLM "Software\Classes\VCVRack.Patch\shell\open\command" "" '"$INSTDIR\Rack.exe" "%1"'

	; Create shortcuts
	CreateShortcut "$DESKTOP\${NAME}.lnk" "$INSTDIR\Rack.exe"
	CreateShortcut "$SMPROGRAMS\${NAME}.lnk" "$INSTDIR\Rack.exe"

	; Add allowed app to Controlled Folder Access
	ExpandEnvStrings $0 "%COMSPEC%"
	ExecShellWait "" '"$0"' "/C powershell -ExecutionPolicy Bypass -WindowStyle Hidden $\"Add-MpPreference -ControlledFolderAccessAllowedApplications '$INSTDIR\Rack.exe'$\"" SW_HIDE
SectionEnd


Section "Uninstall"
	; directoryLeave above ensures that INSTDIR is safe to remove.
	RMDir /r "$INSTDIR"
	; Attempt to remove C:\Program Files\VCV if empty
	RMDir "$INSTDIR\.."

	Delete "$DESKTOP\${NAME}.lnk"
	Delete "$SMPROGRAMS\${NAME}.lnk"

	DeleteRegKey HKLM "${INSTALL_REG}"
	DeleteRegKey /ifempty HKLM "Software\VCV"
	DeleteRegKey HKLM "${UNINSTALL_REG}"
	DeleteRegKey HKLM "Software\Classes\.vcv"
	DeleteRegKey HKLM "Software\Classes\VCVRack.Patch"
SectionEnd
