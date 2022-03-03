;NSIS Modern User Interface
;Multilingual Example Script
;Written by Joost Verburg
;Modified by B. Parisse and Y. Duron for Xcas
;
;See http://nsis.sourceforge.net/Examples/Modern%20UI/MultiLanguage.nsi
;and http://nsis.sourceforge.net/Examples/Modern%20UI/StartMenu.nsi
;and http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
;and http://nsis.sourceforge.net/Docs/Chapter4.html#4.8.3 (Version Information)

;--------------------------------
;Definition of constants

  !define APPNAME       "Xcas"
  !define VERSION       "1.1.2"
  !define URL           "http://www-fourier.ujf-grenoble.fr/~parisse/"
  !define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "${APPNAME}"
  OutFile "c:\tmp\xcasinst.exe"
  
  ;Default installation folder
  InstallDir "c:\xcas"
  ; will enable when sufficiently tested
  ; InstallDir "$PROGRAMFILES\${APPNAME}\"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${APPNAME}" ""
  
  ;For removing Start Menu shortcuts in Windows Vista and Windows 7
  ;See  http://nsis.sourceforge.net/Shortcuts_removal_fails_on_Windows_Vista
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !define MUI_WELCOMEPAGE_TEXT $(WELCOME_TEXT)
  !define MUI_FINISHPAGE_TEXT $(FINISHPAGE_TEXT)
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPNAME}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  ;Start Finish Page Configuration
  !define MUI_FINISHPAGE_RUN "$INSTDIR\$(BAT_FILE)"
  
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "C:\xcas\COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "SimpChinese"

;--------------------------------
;Installer Sections

Section "Xcas" SecXcas

  ;Make this section required
  SectionIn RO
  
  SetOutPath "$INSTDIR"
  
  ;Files to install
  File /r C:\xcas\*.*
  
  ;Generate batch files (e.g. runxcas.fr) according to the folder installation
  Exec "$\"$INSTDIR\win2unix.exe$\" $\"$INSTDIR$\""
  
  ;File association
  WriteRegStr HKCR ".xws" "" "Xcas.Worksheet"
  WriteRegStr HKCR ".xws" "Content Type" "application/x-xcas"
  WriteRegStr HKCR "Xcas.Worksheet" "" "$(MIMETYPE)"
  WriteRegStr HKCR "Xcas.Worksheet\DefaultIcon" "" "$INSTDIR\x-xcas.ico"
  WriteRegStr HKCR "Xcas.Worksheet\shell" "" "open"
  WriteRegStr HKCR "Xcas.Worksheet\shell\open\command" "" "$\"$INSTDIR\$(BAT_FILE)$\" $\"%1$\""
  
  ;Store installation folder
  WriteRegStr HKCU "Software\${APPNAME}" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ;Create start menu shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk" "$INSTDIR\$(BAT_FILE)" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Xcas Documentation.lnk" "$INSTDIR\doc\index.html"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (English).lnk" "$INSTDIR\xcasen.bat" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (French).lnk" "$INSTDIR\xcasfr.bat" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (German).lnk" "$INSTDIR\xcasde.bat" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Spanish).lnk" "$INSTDIR\xcases.bat" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Greek).lnk" "$INSTDIR\xcasgre.bat" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Italian).lnk" "$INSTDIR\xcasit.bat" "" "$INSTDIR\xcas.ico"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Chinese).lnk" "$INSTDIR\xcaszh.bat" "" "$INSTDIR\xcas.ico"
  !insertmacro MUI_STARTMENU_WRITE_END
  
  ;Create desktop shortcut
  CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\$(BAT_FILE)" "" "$INSTDIR\xcas.ico"
  
  ;Add uninstall information to Add/Remove Programs
  WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayName" "${APPNAME} ${VERSION} - Computer Algebra System"
  WriteRegStr HKLM "${REG_UNINSTALL}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKLM "${REG_UNINSTALL}" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
  WriteRegStr HKLM "${REG_UNINSTALL}" "InstallLocation" "$\"$INSTDIR$\""
  WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayIcon" "$\"$INSTDIR\xcas.ico$\""
  WriteRegStr HKLM "${REG_UNINSTALL}" "Readme" "$\"$INSTDIR\README$\""
  WriteRegStr HKLM "${REG_UNINSTALL}" "URLUpdateInfo" "${URL}"
  WriteRegStr HKLM "${REG_UNINSTALL}" "URLInfoAbout" "${URL}"
  WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayVersion" "${VERSION}"
  WriteRegDWORD HKLM "${REG_UNINSTALL}" "NoModify" 1
  WriteRegDWORD HKLM "${REG_UNINSTALL}" "NoRepair" 1

SectionEnd

;--------------------------------
;Descriptions

  LangString BAT_FILE ${LANG_ENGLISH} "xcasen.bat"
  LangString BAT_FILE ${LANG_FRENCH} "xcasfr.bat"
  LangString BAT_FILE ${LANG_GERMAN} "xcasde.bat"
  LangString BAT_FILE ${LANG_SPANISH} "xcases.bat"
  LangString BAT_FILE ${LANG_ITALIAN} "xcasit.bat"
  LangString BAT_FILE ${LANG_GREEK} "xcasgre.bat"
  LangString BAT_FILE ${LANG_SIMPCHINESE} "xcaszh.bat"
  
  LangString MIMETYPE ${LANG_ENGLISH} "Xcas Worksheet"
  LangString MIMETYPE ${LANG_FRENCH} "Feuille de travail Xcas"
  LangString MIMETYPE ${LANG_GERMAN} "Xcas Arbeitsblatt"
  LangString MIMETYPE ${LANG_SPANISH} "Xcas Worksheet"
  LangString MIMETYPE ${LANG_ITALIAN} "Xcas Worksheet"
  LangString MIMETYPE ${LANG_GREEK} "Xcas Worksheet"
  LangString MIMETYPE ${LANG_SIMPCHINESE} "Xcas Worksheet"
  
  LangString WELCOME_TEXT ${LANG_ENGLISH} "The installer will install ${APPNAME}, a free computer algebra system with 2-d/3-d geometry and spreadsheet. ${APPNAME} is available under the GNU GPL version 3 or later license."
  LangString WELCOME_TEXT ${LANG_FRENCH} "Cet assistant va installer ${APPNAME}, un logiciel libre de calcul formel, de géométrie 2-d/3-d et tableur. ${APPNAME} est publié sous la licence GNU GPL version 3 ou ultérieure."
  LangString WELCOME_TEXT ${LANG_GERMAN} "The installer will install ${APPNAME}, a free computer algebra system with 2-d/3-d geometry and spreadsheet. ${APPNAME} is available under the GNU GPL version 3 or later license."
  LangString WELCOME_TEXT ${LANG_SPANISH} "The installer will install ${APPNAME}, a free computer algebra system with 2-d/3-d geometry and spreadsheet. ${APPNAME} is available under the GNU GPL version 3 or later license."
  LangString WELCOME_TEXT ${LANG_ITALIAN} "The installer will install ${APPNAME}, a free computer algebra system with 2-d/3-d geometry and spreadsheet. ${APPNAME} is available under the GNU GPL version 3 or later license."
  LangString WELCOME_TEXT ${LANG_GREEK} "The installer will install ${APPNAME}, a free computer algebra system with 2-d/3-d geometry and spreadsheet. ${APPNAME} is available under the GNU GPL version 3 or later license."
  LangString WELCOME_TEXT ${LANG_SIMPCHINESE} "The installer will install ${APPNAME}, a free computer algebra system with 2-d/3-d geometry and spreadsheet. ${APPNAME} is available under the GNU GPL version 3 or later license."
  
  LangString FINISHPAGE_TEXT ${LANG_ENGLISH} "If you want to compile a C++ program using giac, please read the README file from the installation directory."
  LangString FINISHPAGE_TEXT ${LANG_FRENCH} "Si Xcas a été installé sur une clé USB, utiliser le fichier xcaskey.bat (modifiez le fichier runxcasp.fr prévu pour lecteur réseau et répertoire personnel P:).$\r$\nSi vous êtes administrateur, regardez xcasfrjp.bat et runxcasj.fr (changez la lettre du lecteur réseau personnel p dans export XCAS_HOME=/cygdrive/p et export XCAS_AUTOSAVE_FOLDER=/cygdrive/p)$\r$\nPour créer une application Python/C++/Java utilisant giac, lisez le fichier README présent dans le dossier d'installation."
  LangString FINISHPAGE_TEXT ${LANG_GERMAN} "If you want to create a Python/C++/Java program using giac, please read the README file from the installation directory."
  LangString FINISHPAGE_TEXT ${LANG_SPANISH} "If you want to compile a C++ program using giac, please read the README file from the installation directory."
  LangString FINISHPAGE_TEXT ${LANG_ITALIAN} "If you want to compile a C++ program using giac, please read the README file from the installation directory."
  LangString FINISHPAGE_TEXT ${LANG_GREEK} "If you want to compile a C++ program using giac, please read the README file from the installation directory."
  LangString FINISHPAGE_TEXT ${LANG_SIMPCHINESE} "If you want to compile a C++ program using giac, please read the README file from the installation directory."

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;We could use 'RMDir /r "$INSTDIR"' but it is not safe (see wiki)
  ;Just imagine the disaster if the user has selected the Program Files folder...
  ;So we try to limit it as possible
  RMDir /r "$INSTDIR\%APPDATA%"
  RMDir /r "$INSTDIR\AsTeX"
  RMDir /r "$INSTDIR\doc"
  RMDir /r "$INSTDIR\examples"
  RMDir /r "$INSTDIR\include"
  RMDir /r "$INSTDIR\locale"
  RMDir /r "$INSTDIR\python27"
  RMDir /r "$INSTDIR\src"
  RMDir /r "$INSTDIR\tmp"
  RMDir /r "$INSTDIR\var"
  Delete "$INSTDIR\*.bat"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\*.po"
  Delete "$INSTDIR\*.sh"
  Delete "$INSTDIR\runxcas*"
  Delete "$INSTDIR\aide"
  Delete "$INSTDIR\aide_cas"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\README"
  Delete "$INSTDIR\xcas_recent"
  Delete "$INSTDIR\wget.ini"
  Delete "$INSTDIR\xcas.ico"
  Delete "$INSTDIR\xcas.rc"
  Delete "$INSTDIR\x-xcas.ico"
  Delete "$INSTDIR\.gdbinit"
  Delete "$INSTDIR\Image.png"
  Delete "$INSTDIR\LISEZMOI.python"
  Delete "$INSTDIR\README.python"
  Delete "$INSTDIR\altair.pif"
  Delete "$INSTDIR\copydll"
  Delete "$INSTDIR\endxcas"
  Delete "$INSTDIR\epstopdf"
  Delete "$INSTDIR\fichier"
  Delete "$INSTDIR\giac_oo.cpp"
  Delete "$INSTDIR\gpsavediff.cmd"
  Delete "$INSTDIR\pgcd.cc"
  Delete "$INSTDIR\rungiac.pif"
  Delete "$INSTDIR\session.ps"
  Delete "$INSTDIR\test"
  Delete "$INSTDIR\test2"
  Delete "$INSTDIR\tmpeps"
  Delete "$INSTDIR\tmpeps.eps"
  Delete "$INSTDIR\tmpeps.pdf"
  Delete "$INSTDIR\xcasexe.zip"
  Delete "$INSTDIR\xcasnew.exe.lnk"
  Delete "$INSTDIR\xcas.exe.stackdump"
  Delete "$INSTDIR\xcasexe.exe"
  RMDir "$INSTDIR" ;remove the folder if empty
  
  ;Delete start menu shortcuts
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  SetShellVarContext all
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Xcas Documentation.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (English).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (French).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (German).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Spanish).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Greek).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Italian).lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (Chinese).lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  ;Delete desktop shortcuts
  Delete "$DESKTOP\${APPNAME}.lnk"
  
  ;Delete registry keys
  DeleteRegKey HKCU "Software\${APPNAME}"
  DeleteRegKey HKCR ".xws"
  DeleteRegKey HKCR "Xcas.Worksheet"
  DeleteRegKey HKLM "${REG_UNINSTALL}"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

FunctionEnd

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd
