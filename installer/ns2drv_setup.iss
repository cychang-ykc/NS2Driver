; NS2 Pro Controller Driver - Inno Setup Script
; Build with: iscc ns2drv_setup.iss
; Output: dist\NS2DriverSetup.exe

#define AppName      "NS2 Pro Controller Driver"
#define AppVersion   "1.0.0"
#define AppPublisher "ns2drv"
#define AppExeName   "ns2drv.exe"
#define TaskName     "NS2DriverAutoStart"
#define InstallDir   "{autopf}\NS2Driver"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={#InstallDir}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir=dist
OutputBaseFilename=NS2DriverSetup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\{#AppExeName}
SetupIconFile=files\NS2Driver.ico
WizardSmallImageFile=files\NS2Driver_small.bmp
DisableFinishedPage=no
CloseApplications=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Main executable and runtime DLLs
Source: "files\ns2drv.exe";      DestDir: "{app}"; Flags: ignoreversion
Source: "files\hidapi.dll";      DestDir: "{app}"; Flags: ignoreversion
Source: "files\libusb-1.0.dll";  DestDir: "{app}"; Flags: ignoreversion

; Bundled dependency installers (extracted to temp, deleted after install)
Source: "files\ViGEmBusSetup.exe";  DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall
Source: "files\HidHideSetup.exe";   DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall

; HidHide configuration script
Source: "configure_hidhide.ps1"; DestDir: "{tmp}"; Flags: ignoreversion deleteafterinstall

[Icons]
Name: "{group}\{#AppName}";          Filename: "{app}\{#AppExeName}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Run]
; 1. Install ViGEmBus silently if not already present
Filename: "{tmp}\ViGEmBusSetup.exe"; Parameters: "/quiet /norestart"; \
    StatusMsg: "Installing ViGEmBus driver..."; \
    Check: NeedsViGEmBus; Flags: waituntilterminated

; 2. Install HidHide silently if not already present (suppress auto-reboot)
Filename: "{tmp}\HidHideSetup.exe"; Parameters: "/quiet /norestart"; \
    StatusMsg: "Installing HidHide driver..."; \
    Check: NeedsHidHide; Flags: waituntilterminated

; 3. Configure HidHide (allowlist + hide NS2 device + enable cloaking)
Filename: "powershell.exe"; \
    Parameters: "-ExecutionPolicy Bypass -File ""{tmp}\configure_hidhide.ps1"" -InstallDir ""{app}"""; \
    StatusMsg: "Configuring HidHide..."; \
    Flags: runhidden waituntilterminated

; 4. Register Task Scheduler task for auto-start at login (no UAC popup)
Filename: "schtasks.exe"; \
    Parameters: "/Create /F /TN ""{#TaskName}"" /TR ""{app}\{#AppExeName}"" /SC ONLOGON /RL HIGHEST /IT"; \
    Flags: runhidden waituntilterminated; \
    StatusMsg: "Setting up auto-start..."

; 5. Launch the driver right now
Filename: "{app}\{#AppExeName}"; Description: "Start NS2 Driver now"; \
    Flags: nowait postinstall skipifsilent

[UninstallRun]
; Stop the running process
Filename: "taskkill.exe"; Parameters: "/F /IM {#AppExeName}"; \
    Flags: runhidden waituntilterminated; RunOnceId: "KillDriver"

; Remove Task Scheduler task
Filename: "schtasks.exe"; Parameters: "/Delete /F /TN ""{#TaskName}"""; \
    Flags: runhidden waituntilterminated; RunOnceId: "RemoveTask"

[Code]
var
  HidHideWasInstalled: Boolean;
  // Uninstall options chosen by user
  UninstallViGEm:   Boolean;
  UninstallHidHide: Boolean;

// ---------- Install-time helpers ----------

function NeedRestart: Boolean;
begin
  Result := HidHideWasInstalled;
end;

function NeedsViGEmBus: Boolean;
var
  SubkeyNames: TArrayOfString;
  I: Integer;
  DisplayName: String;
begin
  Result := True;
  if RegGetSubkeyNames(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall', SubkeyNames) then
    for I := 0 to GetArrayLength(SubkeyNames) - 1 do
      if RegQueryStringValue(HKLM,
          'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\' + SubkeyNames[I],
          'DisplayName', DisplayName) then
        if Pos('ViGEm', DisplayName) > 0 then
        begin
          Result := False;
          Exit;
        end;
end;

function NeedsHidHide: Boolean;
var
  SubkeyNames: TArrayOfString;
  I: Integer;
  DisplayName: String;
begin
  Result := True;
  if RegGetSubkeyNames(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall', SubkeyNames) then
    for I := 0 to GetArrayLength(SubkeyNames) - 1 do
      if RegQueryStringValue(HKLM,
          'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\' + SubkeyNames[I],
          'DisplayName', DisplayName) then
        if Pos('HidHide', DisplayName) > 0 then
        begin
          Result := False;
          Exit;
        end;
  if Result then
    HidHideWasInstalled := True;
end;

function InitializeSetup: Boolean;
begin
  Result := True;
  if not Is64BitInstallMode then
  begin
    MsgBox('NS2 Driver requires a 64-bit version of Windows.', mbError, MB_OK);
    Result := False;
  end;
end;

// ---------- Uninstall-time helpers ----------

// Look up the quiet uninstall command for a given product name substring
function GetUninstallCmd(NameSubstr: String): String;
var
  SubkeyNames: TArrayOfString;
  I: Integer;
  DisplayName, Cmd: String;
begin
  Result := '';
  if not RegGetSubkeyNames(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall', SubkeyNames) then
    Exit;
  for I := 0 to GetArrayLength(SubkeyNames) - 1 do
    if RegQueryStringValue(HKLM,
        'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\' + SubkeyNames[I],
        'DisplayName', DisplayName) then
      if Pos(NameSubstr, DisplayName) > 0 then
      begin
        // Prefer QuietUninstallString; fall back to UninstallString
        if not RegQueryStringValue(HKLM,
            'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\' + SubkeyNames[I],
            'QuietUninstallString', Cmd) then
          RegQueryStringValue(HKLM,
            'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\' + SubkeyNames[I],
            'UninstallString', Cmd);
        Result := Cmd;
        Exit;
      end;
end;

// Run an uninstall command (handles both MsiExec and direct-exe formats)
procedure RunUninstallCmd(Cmd: String);
var
  ResultCode: Integer;
begin
  if Cmd = '' then Exit;
  // MSI-based: "MsiExec.exe /X{GUID}" → run msiexec with quiet flags
  if Pos('MsiExec', Cmd) > 0 then
    Exec(ExpandConstant('{sys}\msiexec.exe'),
         Copy(Cmd, Pos('/X', Cmd), MaxInt) + ' /quiet /norestart',
         '', SW_HIDE, ewWaitUntilTerminated, ResultCode)
  else
    // Direct exe: run as-is with quiet flags appended
    Exec(Cmd, '/quiet /norestart', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
end;

function InitializeUninstall: Boolean;
begin
  UninstallViGEm := MsgBox(
    'Remove ViGEmBus driver?' + #13#10 +
    '(Only remove if no other gamepad tools need it)',
    mbConfirmation, MB_YESNO) = IDYES;

  UninstallHidHide := MsgBox(
    'Remove HidHide driver?' + #13#10 +
    '(Only remove if no other tools need it)',
    mbConfirmation, MB_YESNO) = IDYES;

  Result := True;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Cmd: String;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    if UninstallViGEm then
    begin
      Cmd := GetUninstallCmd('ViGEm');
      RunUninstallCmd(Cmd);
    end;
    if UninstallHidHide then
    begin
      Cmd := GetUninstallCmd('HidHide');
      RunUninstallCmd(Cmd);
    end;
  end;
end;
