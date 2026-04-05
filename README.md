# NS2 Pro Controller Driver for Windows

A user-space Windows driver that makes the **Nintendo Switch 2 Pro Controller** (VID `0x057E`, PID `0x2069`) appear as an **Xbox 360 controller** on PC, enabling compatibility with any game that supports XInput.

## Features

- Plug-and-play: connect the controller via USB and it appears as an Xbox 360 pad instantly
- Hot-plug support: automatically reconnects when the controller is unplugged and re-plugged
- System tray icon showing connection status
- Toggle launch-at-startup from the tray right-click menu
- One-click installer that handles all dependencies

## Button Mapping

| NS2 Pro Controller | Xbox 360 |
|--------------------|----------|
| B (bottom) | A |
| A (right) | B |
| Y (left) | X |
| X (top) | Y |
| L / R | LB / RB |
| ZL / ZR | LT / RT |
| − (Minus) | Back |
| + (Plus) | Start |
| Home | Guide |
| L-Stick click | LS |
| R-Stick click | RS |
| D-Pad | D-Pad |

> Face buttons use **positional mapping** (by location on the controller), not label matching, to match how the physical layout feels.

## Requirements

- Windows 10/11 64-bit
- USB-C cable connecting the NS2 Pro Controller to your PC
- [Zadig](https://zadig.akeo.ie/) — install **WinUSB** driver for `NS2 Pro Controller (Interface 1)` (one-time setup)

> The installer automatically handles ViGEmBus and HidHide. Zadig is the only manual step required.

## Installation

1. Run **Zadig**, select `NS2 Pro Controller (Interface 1)`, install **WinUSB** driver
2. Run `NS2DriverSetup.exe` from the [Releases](../../releases) page — it will:
   - Install [ViGEmBus](https://github.com/nefarius/ViGEmBus) (virtual gamepad bus driver)
   - Install [HidHide](https://github.com/nefarius/HidHide) (hides the raw HID device from other apps)
   - Configure HidHide automatically
   - Set up auto-start at login
   - Prompt for a reboot if required

## Building from Source

### Prerequisites

- Visual Studio 2022 Build Tools (MSVC)
- CMake 3.20+
- [vcpkg](https://github.com/microsoft/vcpkg) at `C:\vcpkg`

```powershell
vcpkg install hidapi:x64-windows libusb:x64-windows
```

### Build

```powershell
git clone --recurse-submodules https://github.com/YOUR_USERNAME/ns2_pro_con.git
cd ns2_pro_con
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

### Package Installer

1. Install [Inno Setup 6](https://jrsoftware.org/isdl.php)
2. Download [ViGEmBus installer](https://github.com/nefarius/ViGEmBus/releases/latest) → save as `installer/files/ViGEmBusSetup.exe`
3. Download [HidHide installer](https://github.com/nefarius/HidHide/releases/latest) → save as `installer/files/HidHideSetup.exe`
4. Run `installer/prepare.bat`

## How It Works

```
NS2 Controller (USB)
  │
  ├─ Interface 1 (WinUSB / libusb)
  │    └─ Send init commands → activates HID input mode
  │
  └─ Interface 0 (HID / hidapi)
       └─ Read 64-byte input reports → parse buttons/sticks/triggers
            └─ ViGEmClient → virtual Xbox 360 controller → games
```

HidHide hides the raw HID device so only this driver (and not games/Steam) sees it directly.

## Dependencies

| Library | Purpose |
|---------|---------|
| [hidapi](https://github.com/libusb/hidapi) | HID input reading |
| [libusb](https://libusb.info) | USB bulk write for init sequence |
| [ViGEmClient](https://github.com/nefarius/ViGEmClient) | Virtual Xbox 360 controller |
| [ViGEmBus](https://github.com/nefarius/ViGEmBus) | Virtual gamepad bus (system driver) |
| [HidHide](https://github.com/nefarius/HidHide) | Hide raw HID device from other apps |

## References

Protocol information was derived from these open-source projects:
- [ikz87/NSW2-controller-enabler](https://github.com/ikz87/NSW2-controller-enabler)
- [djedditt/nsw2usb-con](https://github.com/djedditt/nsw2usb-con)

## Disclaimer

This project is not affiliated with or endorsed by Nintendo Co., Ltd.
"Nintendo Switch" is a trademark of Nintendo. This software interacts with
the controller using publicly documented USB HID protocols for personal,
non-commercial use.
