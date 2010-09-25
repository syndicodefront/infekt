#define SourceFileDir32 "C:\temp\free\Release"
#define SourceFileDir64 "C:\temp\free\Release-x64"
#define NfoProgId "iNFEKT.NFOFile.1"

[Setup]
AppId={{B1AC8E6A-6C47-4B6D-A853-B4BF5C83421C}
AppName=iNFekt NFO Viewer
AppVerName=iNFekt 0.7.2
AppVersion=0.7.2
AppPublisher=cxxjoe & Contributors
AppPublisherURL=http://infekt.googlecode.com/
DefaultDirName={pf}\iNFekt
DefaultGroupName=iNFekt NFO Viewer
AllowNoIcons=yes
OutputDir=C:\temp\free
OutputBaseFilename=infekt-setup
Compression=lzma
SolidCompression=yes
WizardSmallImageFile=setup-bmp-small.bmp
WizardImageFile=setup-bmp-left.bmp
WizardImageStretch=yes
ChangesAssociations=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
AppMutex=iNFektNfoViewerOneInstanceMutex
UninstallDisplayIcon={app}\infekt-win64.exe
UninstallDisplayName=iNFekt NFO Viewer

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkedonce
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "nfoassoc"; Description: "Make iNFekt the default viewer for .nfo files"; GroupDescription: "Shell integration"
Name: "shellpreview"; Description: "Install Explorer preview pane and thumbnail integration for .nfo files"; GroupDescription: "Shell integration"; MinVersion: 0,6.0

[Files]
Source: "{#SourceFileDir32}\infekt-win32.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\infekt-win64.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\infekt-cmd.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\infekt-cmd.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\cairo.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\cairo.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\libpng14.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\libpng14.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\pcre.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\pcre.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\cuda-blur.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\cuda-blur.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\cudart32_31_9.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\cudart64_31_9.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#SourceFileDir32}\infekt-nfo-shell.dll"; DestDir: "{app}"; Flags: ignoreversion regserver; Tasks: shellpreview; Check: not Is64BitInstallMode
Source: "{#SourceFileDir64}\infekt-nfo-shell.dll"; DestDir: "{app}"; Flags: ignoreversion regserver; Tasks: shellpreview; Check: Is64BitInstallMode
;Source: "C:\temp\vcredist_x86.exe"; DestDir: "{tmp}"; Flags: ignoreversion; Flags: ignoreversion; Check: not Is64BitInstallMode
;Source: "C:\temp\vcredist_x64.exe"; DestDir: "{tmp}"; Flags: ignoreversion; Flags: ignoreversion; Check: Is64BitInstallMode

[InstallDelete]
Type: files; Name: "{app}\cudart.dll"
; cudart.dll was used up to v0.6.0

[UnInstallDelete]
Type: files; Name: "{app}\infekt-nfo-shell.dll"
; in case someone copied this file into the program folder manually (when it has NOT been installed using the "shellpreview" task)

[Icons]
Name: "{group}\iNFekt NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Check: not Is64BitInstallMode
Name: "{group}\iNFekt NFO Viewer"; Filename: "{app}\infekt-win64.exe"; Check: Is64BitInstallMode
Name: "{group}\{cm:ProgramOnTheWeb,iNFekt NFO Viewer}"; Filename: "http://infekt.googlecode.com/"
Name: "{group}\{cm:UninstallProgram,iNFekt NFO Viewer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\iNFekt NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Tasks: desktopicon; Check: not Is64BitInstallMode
Name: "{commondesktop}\iNFekt NFO Viewer"; Filename: "{app}\infekt-win64.exe"; Tasks: desktopicon; Check: Is64BitInstallMode
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\iNFekt NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Tasks: quicklaunchicon; Check: not Is64BitInstallMode
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\iNFekt NFO Viewer"; Filename: "{app}\infekt-win64.exe"; Tasks: quicklaunchicon; Check: Is64BitInstallMode

[Run]
;Filename: "{tmp}\vcredist_x86.exe"; Parameters: "/q:a /c:""install /l /q"""; WorkingDir: {tmp}; Flags: skipifdoesntexist; StatusMsg: "Checking for and installing ""Microsoft Visual C++ 2008 SP1 Redistributable Package"" if needed. This can take several minutes..."; Check: not Is64BitInstallMode
;Filename: "{tmp}\vcredist_x64.exe"; Parameters: "/q:a /c:""install /l /q"""; WorkingDir: {tmp}; Flags: skipifdoesntexist; StatusMsg: "Checking for and installing ""Microsoft Visual C++ 2008 SP1 Redistributable Package"" if needed. This can take several minutes..."; Check: Is64BitInstallMode
Filename: "{app}\infekt-win32.exe"; Description: "{cm:LaunchProgram,iNFekt NFO Viewer}"; Flags: nowait postinstall skipifsilent; Check: not Is64BitInstallMode
Filename: "{app}\infekt-win64.exe"; Description: "{cm:LaunchProgram,iNFekt NFO Viewer}"; Flags: nowait postinstall skipifsilent; Check: Is64BitInstallMode

[UninstallRun]
Filename: "{sys}\regsvr32.exe"; Parameters: "/s /u ""{app}\infekt-nfo-shell.dll"""; Flags: skipifdoesntexist runhidden
; in case someone manually registered the shell extension, make sure to clean up.

[Registry]
; Set up ProgId for file associations:
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}"; ValueType: string; ValueName: ""; ValueData: "NFO File"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: """{app}\infekt-win32.exe"",0"; Check: not Is64BitInstallMode
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: """{app}\infekt-win64.exe"",0"; Check: Is64BitInstallMode
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\infekt-win32.exe"" ""%1"""; Check: not Is64BitInstallMode
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\infekt-win64.exe"" ""%1"""; Check: Is64BitInstallMode
; Claim machine-level file associations:
Root: HKLM; Subkey: "SOFTWARE\Classes\.nfo"; ValueType: string; ValueName: ""; ValueData: "{#NfoProgId}"; Flags: uninsclearvalue; Tasks: "nfoassoc"
; Register the application with Default Programs
Root: HKLM; Subkey: "SOFTWARE\cxxjoe\iNFEKT"; ValueType: string; ValueName: ""; ValueData: ""; Flags: uninsdeletekey; MinVersion: 0,6.0
Root: HKLM; Subkey: "SOFTWARE\cxxjoe\iNFEKT\Capabilities"; ValueType: string; ValueName: "ApplicationName"; ValueData: "iNFekt NFO Viewer"; MinVersion: 0,6.0
Root: HKLM; Subkey: "SOFTWARE\cxxjoe\iNFEKT\Capabilities"; ValueType: string; ValueName: "ApplicationDescription"; ValueData: "iNFekt NFO Viewer is the next generation NFO Viewer Application!"; MinVersion: 0,6.0
Root: HKLM; Subkey: "SOFTWARE\cxxjoe\iNFEKT\Capabilities\FileAssociations"; ValueType: string; ValueName: ".nfo"; ValueData: "{#NfoProgId}"; MinVersion: 0,6.0
Root: HKLM; Subkey: "SOFTWARE\RegisteredApplications"; ValueType: string; ValueName: "iNFekt NFO Viewer"; ValueData: "SOFTWARE\cxxjoe\iNFEKT\Capabilities"; Flags: uninsdeletevalue; MinVersion: 0,6.0
; Clear user-overridden stuff on 5.x:
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.nfo"; ValueType: none; ValueName: "Progid"; Flags: deletevalue; Tasks: "nfoassoc"
; Clear user-overridden stuff on Vista/7:
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.nfo\UserChoice"; ValueType: none; Flags: deletekey; Tasks: "nfoassoc"; MinVersion: 0,6.0

; Uninstall-only stuff:

; Program settings:
Root: HKCU; Subkey: "Software\cxxjoe\iNFEKT"; Flags: dontcreatekey uninsdeletekey
; Association created by Windows:
Root: HKCU; Subkey: "Software\Classes\Applications\infekt-win32.exe"; Flags: dontcreatekey uninsdeletekey; Check: not Is64BitInstallMode
Root: HKCU; Subkey: "Software\Classes\Applications\infekt-win64.exe"; Flags: dontcreatekey uninsdeletekey; Check: Is64BitInstallMode

