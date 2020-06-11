; This file is part of the HörTech Open Master Hearing Aid (openMHA)
; Copyright © 2018 HörTech gGmbH
;
; openMHA is free software: you can redistribute it and/or modify
; it under the terms of the GNU Affero General Public License as published by
; the Free Software Foundation, version 3 of the License.
;
; openMHA is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU Affero General Public License, version 3 for more details.
;
; You should have received a copy of the GNU Affero General Public License, 
; version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

!define PRODUCT_NAME openMHA 
!define PRODUCT_VERSION 4.12.0
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

; Add mha dir to path
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

; Reference algorithms
Section "Reference algorithms"
  SetOutPath $INSTDIR\reference_algorithms
  File /r reference_algorithms\*
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
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Readme.lnk" "$INSTDIR\README.txt"
SectionEnd

Section "Uninstall"
  
  EnVar::DeleteValue "path" "$INSTDIR\bin"
  
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
