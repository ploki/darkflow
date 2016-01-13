; Script for Inno Setup Studio
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!


#define public PackageContent "C:\Users\Guillaume Gimenez\Desktop\DarkflowPackage-" + Arch
#define public ImageMagickInstallerExec "ImageMagick-6.9.3-0-Q16-" + Arch + "-dll.exe"
#define public ImageMagickInstaller PackageContent + "\" + ImageMagickInstallerExec
#define public DcrawExec PackageContent + "\dcraw.exe"
#define public HomeDir "C:\Users\Guillaume Gimenez"
#define public BuildDir HomeDir + "\Desktop\build-darkflow-Desktop_Qt_5_5_1_MSVC2013_"+ BuildArch + "-Release\release"
#define public SourceDir HomeDir + "\Desktop\darkflow"
#define public FFmpegBinDir "C:\ffmpeg-" + Arch + "\bin"
#define public MSVCCRTRedistDir "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\" + Arch + "\Microsoft.VC120.CRT"
#define public MSVCOMPRedistDir "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\" + Arch + "\Microsoft.VC120.OPENMP"



[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{#AppUUID}
AppName=Dark Flow ({#Arch})
AppVersion={#Version}.{#BuildArch}
;AppVerName={AppName} {AppVersion}
AppPublisher=http://darkflow.org/
AppPublisherURL=http://darkflow.org/
AppSupportURL=http://darkflow.org/
AppUpdatesURL=http://darkflow.org/
DefaultDirName={pf}\Dark Flow
DefaultGroupName=Dark Flow ({#BuildArch})
DisableProgramGroupPage=yes
OutputDir={#PackageContent}
OutputBaseFilename=setup-darkflow-{#Arch}
SetupIconFile={#SourceDir}\icons\darkflow-256x256.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
;main exe
Source: "{#BuildDir}\darkflow.exe"; DestDir: "{app}"; Flags: ignoreversion

;dcraw
Source: "{#DcrawExec}"; DestDir: "{app}"; Flags: ignoreversion

;imagemagick
Source: "{#ImageMagickInstaller}";  DestDir: "{app}"; AfterInstall: RunOtherInstaller

;ffmpeg
Source: "{#FFmpegBinDir}\avcodec-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FFmpegBinDir}\avformat-57.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FFmpegBinDir}\avutil-55.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#FFmpegBinDir}\swresample-2.dll"; DestDir: "{app}"; Flags: ignoreversion

;Qt
Source: "{#QtBinDir}\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinDir}\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinDir}\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinDir}\..\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion

;msvc redist
Source: "{#MSVCCRTRedistDir}\msvcp120.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MSVCCRTRedistDir}\msvcr120.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MSVCOMPRedistDir}\vcomp120.dll"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Code]
procedure RunOtherInstaller;
var
  ResultCode: Integer;
begin
  if not Exec(ExpandConstant('{app}\{#ImageMagickInstallerExec}'), '', '', SW_SHOWNORMAL,
    ewWaitUntilTerminated, ResultCode)
  then
    MsgBox('ImageMagick installer failed to run!' + #13#10 +
      SysErrorMessage(ResultCode), mbError, MB_OK);
end;

[Icons]
Name: "{group}\Dark Flow"; Filename: "{app}\darkflow.exe"
Name: "{group}\{cm:ProgramOnTheWeb,Dark Flow}"; Filename: "http://darkflow.org/"
Name: "{group}\{cm:UninstallProgram,Dark Flow}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Dark Flow"; Filename: "{app}\darkflow.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Dark Flow"; Filename: "{app}\darkflow.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\darkflow.exe"; Description: "{cm:LaunchProgram,Dark Flow}"; Flags: nowait postinstall skipifsilent
