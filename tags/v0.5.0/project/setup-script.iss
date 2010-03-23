#define SourceFileDir "C:\temp\free\Release"
#define NfoProgId "iNFEKT.NFOFile.1"

[Setup]
AppId={{B1AC8E6A-6C47-4B6D-A853-B4BF5C83421C}
AppName=iNFekt NFO Viewer
AppVerName=iNFekt 0.5
AppPublisher=cxxjoe & Contributors
AppPublisherURL=http://infekt.googlecode.com/
AppSupportURL=http://infekt.googlecode.com/
AppUpdatesURL=http://infekt.googlecode.com/
DefaultDirName={pf}\iNFEKT
DefaultGroupName=iNFekt NFO Viewer
AllowNoIcons=yes
LicenseFile=C:\temp\free\LICENSE.txt
OutputDir=C:\temp\free
OutputBaseFilename=infekt-setup
Compression=lzma
SolidCompression=yes
WizardSmallImageFile=setup-bmp-small.bmp
WizardImageFile=setup-bmp-left.bmp
WizardImageStretch=yes
ChangesAssociations=yes
PrivilegesRequired=admin

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "nfoassoc"; Description: "Make iNFekt the default viewer for .nfo files"; GroupDescription: "Shell Integration"; Flags: checkedonce

[Files]
Source: "{#SourceFileDir}\infekt-win32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\infekt-cmd.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\cairo.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\libpng14.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\pcre.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\cuda-blur.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\cudart.dll"; DestDir: "{app}"; Flags: ignoreversion
;Source: "C:\temp\vcredist_x86.exe"; DestDir: "{tmp}"; Flags: ignoreversion

[Icons]
Name: "{group}\iNFekt NFO Viewer"; Filename: "{app}\infekt-win32.exe"
Name: "{group}\{cm:ProgramOnTheWeb,iNFekt NFO Viewer}"; Filename: "http://infekt.googlecode.com/"
Name: "{group}\{cm:UninstallProgram,iNFekt NFO Viewer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\iNFekt NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\iNFekt NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Tasks: quicklaunchicon

[Run]
;Filename: "{tmp}\vcredist_x86.exe"; Parameters: "/q:a /c:""install /l /q"""; WorkingDir: {tmp}; Flags: skipifdoesntexist; StatusMsg: "Checking for and installing ""Microsoft Visual C++ 2008 SP1 Redistributable Package"" if needed. This can take several minutes..."
Filename: "{app}\infekt-win32.exe"; Description: "{cm:LaunchProgram,iNFekt NFO Viewer}"; Flags: nowait postinstall skipifsilent

[Registry]
; Set up ProgId for file associations:
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}"; ValueType: string; ValueName: ""; ValueData: "NFO File"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: """{app}\infekt-win32.exe"",0"
Root: HKLM; Subkey: "SOFTWARE\Classes\{#NfoProgId}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\infekt-win32.exe"" ""%1"""
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
Root: HKCU; Subkey: "Software\Classes\Applications\infekt-win32.exe"; Flags: dontcreatekey uninsdeletekey




