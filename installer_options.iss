#define AppVer "0.0.1"

#define AppName "AviSynth as DirectShow Capture"
; AppId === AppName by default BTW

[Run]
Filename: {app}\vendor\vcredist_x86.exe; Parameters: "/passive /Q:a /c:""msiexec /qb /i vcredist.msi"" "; StatusMsg: Installing 2010 RunTime...
Filename: regsvr32; WorkingDir: {app}; Parameters: /s avisynth-as-dshow.dll

[UninstallRun]
Filename: regsvr32; WorkingDir: {app}; Parameters: /s /u avisynth-as-dshow.dll

[Files]
Source: source_code\Win32\Release\PushDesktop.ax; DestDir: {app}
Source: README.TXT; DestDir: {app}; Flags: isreadme
Source: ChangeLog.txt; DestDir: {app}
Source: configuration_setup_utility\*.*; DestDir: {app}\configuration_setup_utility; Flags: recursesubdirs
Source: vendor\troubleshooting_benchmarker\BltTest\Release\BltTest.exe; DestDir: {app}
Source: vendor\vcredist_x86.exe; DestDir: {app}\vendor

[Setup]
AppName={#AppName}
AppVerName={#AppVer}
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#AppName}
UninstallDisplayName={#AppName} uninstall
OutputBaseFilename=setup {#AppName} v{#AppVer}
OutputDir=releases

[Icons]
Name: {group}\Readme; Filename: {app}\README.TXT
Name: {group}\configure by setting the input filename to use; Filename: {app}\configuration_setup_utility\setup_filename_to_use.bat; WorkingDir: {app}\configuration_setup_utility
Name: {group}\Release Notes; Filename: {app}\ChangeLog.txt
Name: {group}\Uninstall {#AppName}; Filename: {uninstallexe}
Name: {group}\test current configuration; Filename: {app}\configuration_setup_utility\setup_filename_to_use.bat; WorkingDir: {app}\configuration_setup_utility
