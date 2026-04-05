# NS2 Pro Controller Windows Driver - Installer
# Run as Administrator

#Requires -RunAsAdministrator

$ErrorActionPreference = "Stop"

$InstallDir  = "C:\Program Files\NS2Driver"
$TaskName    = "NS2Driver"
$ExeName     = "ns2drv.exe"
$SourceDir   = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host ""
Write-Host "======================================"
Write-Host " NS2 Pro Controller Driver Installer"
Write-Host "======================================"
Write-Host ""

# --- 1. Check prerequisites ---
Write-Host "[1/4] Checking prerequisites..."

$vigemOk = Get-PnpDevice -ErrorAction SilentlyContinue | Where-Object { $_.FriendlyName -like "*Virtual Gamepad*" }
if (-not $vigemOk) {
    Write-Warning "ViGEmBus not detected. Please install it from: https://github.com/nefarius/ViGEmBus/releases"
    exit 1
}
Write-Host "  ViGEmBus: OK"

$hidhideOk = Get-PnpDevice -ErrorAction SilentlyContinue | Where-Object { $_.FriendlyName -like "*HidHide*" }
if (-not $hidhideOk) {
    Write-Warning "HidHide not detected. Please install it from: https://github.com/nefarius/HidHide/releases"
    exit 1
}
Write-Host "  HidHide:  OK"

# --- 2. Copy files ---
Write-Host "[2/4] Installing to $InstallDir ..."

if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir | Out-Null
}

$filesToCopy = @($ExeName, "hidapi.dll", "libusb-1.0.dll")
foreach ($f in $filesToCopy) {
    $src = Join-Path $SourceDir $f
    if (-not (Test-Path $src)) {
        Write-Error "Missing file: $src`nRun build first, then copy files to installer\ folder."
    }
    Copy-Item $src $InstallDir -Force
    Write-Host "  Copied: $f"
}

$ExePath = Join-Path $InstallDir $ExeName

# --- 3. Register with HidHide allowlist ---
Write-Host "[3/4] Registering with HidHide allowlist..."
Write-Host "  NOTE: Open HidHide Configuration Client and add:"
Write-Host "    Applications tab: $ExePath"
Write-Host "  (This cannot be automated without HidHide SDK)"

# --- 4. Create Task Scheduler task (auto-start on login) ---
Write-Host "[4/4] Creating startup task..."

# Remove existing task if present
schtasks /delete /tn $TaskName /f 2>$null | Out-Null

# Create task: run at user logon with highest privileges, hidden window
$xml = @"
<?xml version="1.0" encoding="UTF-16"?>
<Task version="1.2" xmlns="http://schemas.microsoft.com/windows/2004/02/mit/task">
  <RegistrationInfo>
    <Description>NS2 Pro Controller Windows Driver</Description>
  </RegistrationInfo>
  <Triggers>
    <LogonTrigger>
      <Enabled>true</Enabled>
      <UserId>$env:USERDOMAIN\$env:USERNAME</UserId>
    </LogonTrigger>
  </Triggers>
  <Principals>
    <Principal id="Author">
      <UserId>$env:USERDOMAIN\$env:USERNAME</UserId>
      <LogonType>InteractiveToken</LogonType>
      <RunLevel>HighestAvailable</RunLevel>
    </Principal>
  </Principals>
  <Settings>
    <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>
    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>
    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>
    <ExecutionTimeLimit>PT0S</ExecutionTimeLimit>
    <Priority>7</Priority>
  </Settings>
  <Actions>
    <Exec>
      <Command>$ExePath</Command>
    </Exec>
  </Actions>
</Task>
"@

$xmlPath = "$env:TEMP\ns2driver_task.xml"
$xml | Out-File -FilePath $xmlPath -Encoding Unicode
schtasks /create /tn $TaskName /xml $xmlPath /f | Out-Null
Remove-Item $xmlPath

Write-Host "  Task '$TaskName' created (runs at login)"

# --- Done ---
Write-Host ""
Write-Host "======================================"
Write-Host " Installation complete!"
Write-Host "======================================"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Open HidHide Configuration Client"
Write-Host "  2. Applications tab -> Add: $ExePath"
Write-Host "  3. Plug in your NS2 Pro Controller"
Write-Host "  4. Driver will auto-start on next login, or run now:"
Write-Host "     Start-ScheduledTask -TaskName '$TaskName'"
Write-Host ""
