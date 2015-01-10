# INCLUDES
!include MUI2.nsh ;Modern interface
!include LogicLib.nsh ;nsDialogs

# INIT
!define CM_VERSION "0.9"

Name "Cell Motus Puzzle Game"
InstallDir "$LOCALAPPDATA\CellMotus"
OutFile "cm-${CM_VERSION}-win32-x86_64.exe"
RequestExecutionLevel user

# REFS
!define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\CellMotus"
!define START_LINK_DIR "$STARTMENU\Programs\CellMotus"
!define START_LINK_RUN "$STARTMENU\Programs\CellMotus\CellMotus.lnk"
!define START_LINK_UNINSTALLER "$STARTMENU\Programs\CellMotus\Uninstall CellMotus.lnk"
!define UNINSTALLER_NAME "CellMotus-Uninstall.exe"
!define WEBSITE_LINK "http://www.time-flask.com/cellmotus.html"

# GRAPHICS
!define MUI_ICON "..\..\src\win32\main.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-r.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall-r.bmp"

# TEXT AND SETTINGS
!define MUI_PAGE_HEADER_TEXT "Cell Motus Puzzle Game"

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
	WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayName" "Cell Motus"
	WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayIcon" "$\"$INSTDIR\CellMotus.exe$\""
	WriteRegStr HKCU "${REG_UNINSTALL}" "Publisher" "TimeFlask"
	WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayVersion" "0.9"
	WriteRegDWord HKCU "${REG_UNINSTALL}" "EstimatedSize" 2048 ;KB
	WriteRegStr HKCU "${REG_UNINSTALL}" "HelpLink" "${WEBSITE_LINK}"
	WriteRegStr HKCU "${REG_UNINSTALL}" "URLInfoAbout" "${WEBSITE_LINK}"
	WriteRegStr HKCU "${REG_UNINSTALL}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKCU "${REG_UNINSTALL}" "InstallSource" "$\"$EXEDIR$\""
	WriteRegDWord HKCU "${REG_UNINSTALL}" "NoModify" 1
	WriteRegDWord HKCU "${REG_UNINSTALL}" "NoRepair" 1
	WriteRegStr HKCU "${REG_UNINSTALL}" "UninstallString" "$\"$INSTDIR\${UNINSTALLER_NAME}$\""
	WriteRegStr HKCU "${REG_UNINSTALL}" "Comments" "Cell Motus puzzle game Unistall, bcuz its hard"
	
	;Links
	SetShellVarContext current
	CreateDirectory "${START_LINK_DIR}"
	CreateShortCut "${START_LINK_RUN}" "$INSTDIR\CellMotus.exe"
	CreateShortCut "${START_LINK_UNINSTALLER}" "$INSTDIR\${UNINSTALLER_NAME}"
	
FunctionEnd

Function un.DeregisterApplication
	DeleteRegKey HKCU "${REG_UNINSTALL}"
	
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
	
	File "..\..\bin\win32\64-bit\CellMotus.exe"
	

        File /r "..\..\assets"

	WriteUninstaller "$INSTDIR\${UNINSTALLER_NAME}"
	Call RegisterApplication
SectionEnd

Section "Uninstall"
	RMDir /r "$INSTDIR"
	
	RMDir /r "$APPDATA\CellMotus"
	
	Call un.DeregisterApplication
SectionEnd
