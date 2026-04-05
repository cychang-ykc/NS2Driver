#Requires -RunAsAdministrator

$InstallDir = "C:\Program Files\NS2Driver"
$TaskName   = "NS2Driver"

Write-Host "Removing NS2 Driver..."

Stop-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
schtasks /delete /tn $TaskName /f 2>$null | Out-Null
Write-Host "  Startup task removed"

if (Test-Path $InstallDir) {
    Remove-Item $InstallDir -Recurse -Force
    Write-Host "  Files removed: $InstallDir"
}

Write-Host "Uninstall complete."
Write-Host "Note: HidHide and ViGEmBus are not removed (shared with other apps)."
