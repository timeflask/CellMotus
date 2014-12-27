# INCLUDES
!include MUI2.nsh ;Modern interface
!include LogicLib.nsh ;nsDialogs

# INIT
Name "CellMotus Puzzle Game"
InstallDir "$LOCALAPPDATA\CellMotus"
OutFile "CellMotus-Setup.exe"
RequestExecutionLevel user

# REFS
!define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\CellMotus"
!define START_LINK_DIR "$STARTMENU\Programs\CellMotus"
!define START_LINK_RUN "$STARTMENU\Programs\CellMotus\CellMotus.lnk"
!define START_LINK_UNINSTALLER "$STARTMENU\Programs\CellMotus\Uninstall CellMotus.lnk"
!define UNINSTALLER_NAME "CellMotus-Uninstall.exe"
!define WEBSITE_LINK "http://www.time-flask.com/cellmotus/"

# GRAPHICS
!define MUI_ICON "..\..\src\win32\main.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-r.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall-r.bmp"

# TEXT AND SETTINGS
!define MUI_PAGE_HEADER_TEXT "CellMotus Puzzle Game"

!define MUI_FINISHPAGE_RUN "$INSTDIR\CellMotus.exe"

;Do not skip to finish automatially
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

# PAGE DEFINITIONS
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# LANGUAGES
!insertmacro MUI_LANGUAGE "English"

# INITIALIZATION AND ERROR CHECKING
Function .onInit
FunctionEnd

# CALLBACKS
Function RegisterApplication
	;Register uninstaller into Add/Remove panel (for local user only)
	WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayName" "CellMotus"
	WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayIcon" "$\"$INSTDIR\CellMotus.exe$\""
	WriteRegStr HKCU "${REG_UNINSTALL}" "Publisher" "TimeFlask Co"
	WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayVersion" "0.9"
	WriteRegDWord HKCU "${REG_UNINSTALL}" "EstimatedSize" 2048 ;KB
	WriteRegStr HKCU "${REG_UNINSTALL}" "HelpLink" "${WEBSITE_LINK}"
	WriteRegStr HKCU "${REG_UNINSTALL}" "URLInfoAbout" "${WEBSITE_LINK}"
	WriteRegStr HKCU "${REG_UNINSTALL}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKCU "${REG_UNINSTALL}" "InstallSource" "$\"$EXEDIR$\""
	WriteRegDWord HKCU "${REG_UNINSTALL}" "NoModify" 1
	WriteRegDWord HKCU "${REG_UNINSTALL}" "NoRepair" 1
	WriteRegStr HKCU "${REG_UNINSTALL}" "UninstallString" "$\"$INSTDIR\${UNINSTALLER_NAME}$\""
	WriteRegStr HKCU "${REG_UNINSTALL}" "Comments" "CellMotus puzzle game Unistall, bcuz its hard"
	
	;Links
	SetShellVarContext current
	CreateDirectory "${START_LINK_DIR}"
	CreateShortCut "${START_LINK_RUN}" "$INSTDIR\CellMotus.exe"
	CreateShortCut "${START_LINK_UNINSTALLER}" "$INSTDIR\${UNINSTALLER_NAME}"
	
FunctionEnd

Function un.DeregisterApplication
	;Deregister uninstaller from Add/Remove panel
	DeleteRegKey HKCU "${REG_UNINSTALL}"
	
	;Start menu links
	SetShellVarContext current
	RMDir /r "${START_LINK_DIR}"
FunctionEnd

# INSTALL SECTIONS
Section "!CellMotus" CellMotus
	SectionIn RO
	
	SetOutPath $INSTDIR
	SetOverwrite on

	File ".desktop"
	File ".game"
	
	;Main installation
	File "..\..\bin\win32\CellMotus.exe"
	File "..\..\bin\win32\glew32.dll"
	File "..\..\bin\win32\lua51.dll"
	File "..\..\bin\win32\glfw3.dll"
	

        File /r "..\..\assets"

	;Uninstaller
	WriteUninstaller "$INSTDIR\${UNINSTALLER_NAME}"
	Call RegisterApplication
SectionEnd

Section "Uninstall"
	;Remove whole directory (no data is stored there anyway)
	RMDir /r "$INSTDIR"
	
	;Remove roaming AppData folder (settings and logs)
	RMDir /r "$APPDATA\CellMotus"
	
	;Remove uninstaller
	Call un.DeregisterApplication
SectionEnd
