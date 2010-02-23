#define SourceFileDir "C:\temp\free\Release"

[Setup]
AppId={{B1AC8E6A-6C47-4B6D-A853-B4BF5C83421C}
AppName=iNFEKT NFO Viewer
AppVerName=iNFEKT 0.2
AppPublisher=cxxjoe & Contributors
AppPublisherURL=http://infekt.googlecode.com/
AppSupportURL=http://infekt.googlecode.com/
AppUpdatesURL=http://infekt.googlecode.com/
DefaultDirName={pf}\iNFEKT
DefaultGroupName=iNFEKT NFO Viewer
AllowNoIcons=yes
LicenseFile=C:\temp\free\LICENSE.txt
OutputDir=C:\temp\free
OutputBaseFilename=infekt-setup
Compression=lzma
SolidCompression=yes
WizardSmallImageFile=setup-bmp-small.bmp

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SourceFileDir}\infekt-win32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\infekt-cmd.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\cairo.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\libpng14.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\pcre.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceFileDir}\make-default-app.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\temp\vcredist_x86.exe"; DestDir: "{tmp}"; Flags: ignoreversion

[Icons]
Name: "{group}\iNFEKT NFO Viewer"; Filename: "{app}\infekt-win32.exe"
Name: "{group}\{cm:ProgramOnTheWeb,iNFEKT NFO Viewer}"; Filename: "http://infekt.googlecode.com/"
Name: "{group}\{cm:UninstallProgram,iNFEKT NFO Viewer}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\iNFEKT NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\iNFEKT NFO Viewer"; Filename: "{app}\infekt-win32.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{tmp}\vcredist_x86.exe"; Parameters: "/q:a /c:""install /l /q"""; WorkingDir: {tmp}; Flags: skipifdoesntexist; StatusMsg: "Checking for and installing ""Microsoft Visual C++ 2008 SP1 Redistributable Package"" if needed. This can take several minutes..."
Filename: "{app}\infekt-win32.exe"; Description: "{cm:LaunchProgram,iNFEKT NFO Viewer}"; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKCU32; Subkey: "Software\cxxjoe\iNFEKT"; Flags: dontcreatekey uninsdeletekey
; :TODO: delete default viewer keys


