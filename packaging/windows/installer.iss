; =============================================================================
;  TEAL 1176 -- Windows installer (Inno Setup)
;
;  Installs the VST3 plugin into the standard shared VST3 folder
;  (C:\Program Files\Common Files\VST3) and, optionally, the standalone app
;  into Program Files. Build with:
;
;      ISCC.exe /DAppVersion=1.0.0 packaging\windows\installer.iss
;
;  Expects the Release build to already exist under build\TEAL1176_artefacts.
; =============================================================================

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif

#define AppName "TEAL 1176"
#define AppPublisher "T.E.A.L."

[Setup]
AppId={{8F4C1176-1176-4A11-9C0E-7E41504C5341}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
DisableDirPage=auto
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\..\dist
OutputBaseFilename=TEAL-1176-{#AppVersion}-Windows
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName} {#AppVersion}

[Types]
Name: "full";   Description: "Full installation (VST3 + Standalone)"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3";       Description: "VST3 plugin";          Types: full custom
Name: "standalone"; Description: "Standalone application"; Types: full custom

[Files]
; VST3 is a bundle (folder) -- install the whole thing into the shared VST3 dir.
Source: "..\..\build\TEAL1176_artefacts\Release\VST3\{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Components: vst3; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\..\build\TEAL1176_artefacts\Release\Standalone\{#AppName}.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Components: standalone
Name: "{commondesktop}\{#AppName}"; Filename: "{app}\{#AppName}.exe"; Components: standalone; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Components: standalone; Flags: unchecked
