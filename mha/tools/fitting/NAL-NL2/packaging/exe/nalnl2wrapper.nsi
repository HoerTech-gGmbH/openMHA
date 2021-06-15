!define PRODUCT_NAME nalnl2wrapper
!define PRODUCT_VERSION 2021.06
!define PRODUCT_PUBLISHER HoerTech
BrandingText "A command line wrapper for the NAL NL2 fitting rule"
RequestExecutionLevel admin
; The name of the installer
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
; The file to write
OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-installer.exe"
; The default installation directory
InstallDir $PROGRAMFILES\${PRODUCT_NAME}
LicenseData "../../COPYING"

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

; Add bin dir to path
EnVar::AddValue "path" "$INSTDIR\bin"
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

; Wrapper source code
Section "Wrapper source code"
  SetOutPath $INSTDIR\src
  File /r src\*
SectionEnd

Section "Uninstall"
  
  EnVar::DeleteValue "path" "$INSTDIR\bin"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
  DeleteRegKey HKLM "SOFTWARE\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}"

  ; Remove files and uninstaller
  Delete $INSTDIR\${PRODUCT_NAME}.lnk
  RMDir /r "$INSTDIR\bin"
  RMDir /r "$INSTDIR\src"
  Delete $INSTDIR\uninstall.exe

  ; Remove directories used
  RMDir "$INSTDIR"
SectionEnd
