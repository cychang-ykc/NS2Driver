# Configures HidHide for NS2 Pro Controller Driver
# Run after HidHide installation, with the install path of ns2drv.exe as argument
# Usage: configure_hidhide.ps1 -InstallDir "C:\Program Files\NS2Driver"

param(
    [string]$InstallDir = "C:\Program Files\NS2Driver"
)

$cli = "C:\Program Files\Nefarius Software Solutions e.U\HidHide\x64\HidHideCLI.exe"
$exe = "$InstallDir\ns2drv.exe"

if (-not (Test-Path $cli)) {
    Write-Host "[ERROR] HidHideCLI.exe not found. Is HidHide installed?"
    exit 1
}

# 1. Add ns2drv.exe to the allowlist (so it can still see the hidden device)
Write-Host "[1/3] Adding ns2drv.exe to HidHide allowlist..."
& $cli --app-reg $exe

# 2. Find NS2 HID interface (VID_057E, PID_2069, MI_00) and hide it
Write-Host "[2/3] Hiding NS2 HID device..."
$devices = Get-PnpDevice -ErrorAction SilentlyContinue |
    Where-Object { $_.InstanceId -match "HID\\VID_057E&PID_2069" }

if ($devices) {
    foreach ($dev in $devices) {
        # Convert InstanceId to device path format HidHide expects
        $instanceId = $dev.InstanceId
        Write-Host "  Found: $instanceId"
        & $cli --dev-hide $instanceId
    }
} else {
    Write-Host "  NS2 controller not currently connected - will be hidden on next plug-in."
    Write-Host "  (HidHide hides by stored list, no action needed now)"
}

# 3. Enable cloaking
Write-Host "[3/3] Enabling HidHide cloaking..."
& $cli --cloak-on

Write-Host ""
Write-Host "HidHide configuration complete."
