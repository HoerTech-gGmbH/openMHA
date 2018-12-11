!define PRODUCT_NAME openMHA 
!define PRODUCT_VERSION 4.7.0
!define PRODUCT_PUBLISHER HoerTech
BrandingText "${PRODUCT_NAME} (c) ${PRODUCT_PUBLISHER}"
RequestExecutionLevel admin
; The name of the installer
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
; The file to write
OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-installer.exe"
; The default installation directory
InstallDir $PROGRAMFILES\${PRODUCT_NAME}
LicenseData "..\..\..\..\COPYING"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}" "Install_Dir"

;Pages
Page license
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

; Core Components
Section "${PRODUCT_NAME} (required)"
SectionIn RO
SetOutPath $INSTDIR\bin
File bin\*
SetOutPath $INSTDIR
File /oname=README.txt "..\..\..\..\README.md"
CreateShortcut $INSTDIR\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\mha.exe"
; Write the installation path into the registry
WriteRegStr HKLM SOFTWARE\${PRODUCT_PUBLISHER}\${PRODUCT_NAME} "Install_Dir" "$INSTDIR"
  
; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" \ 
  "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"  
SectionEnd

; Matlab Tools
Section "Matlab tools"
  SetOutPath $INSTDIR\mfiles
  File /r mfiles\*
SectionEnd

; Examples
Section "Examples"
  SetOutPath $INSTDIR\examples
  File /r examples\*
SectionEnd

; Manuals
Section "Documentation"
  SetOutPath $INSTDIR\doc
  File doc\*
SectionEnd

; Start Menu Shortcuts
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" 
  CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\mha.exe"
SectionEnd

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
  DeleteRegKey HKLM "SOFTWARE\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}"

  ; Remove files and uninstaller
  Delete $INSTDIR\${PRODUCT_NAME}.lnk
  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\mfiles"
  RMDir /r "$INSTDIR\examples"
  RMDir /r "$INSTDIR\doc"
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\README.txt
  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir "$INSTDIR"
SectionEnd
