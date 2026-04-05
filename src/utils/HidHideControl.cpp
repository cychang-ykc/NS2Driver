#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winioctl.h>
#include "HidHideControl.hpp"
#include "Logger.hpp"

// HidHide IOCTL codes (from nefarius/HidHide source)
#define IOCTL_GET_ACTIVE CTL_CODE(0x8000, 2052, METHOD_BUFFERED, FILE_READ_DATA)
#define IOCTL_SET_ACTIVE CTL_CODE(0x8000, 2053, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

static HANDLE openHidHide() {
    return CreateFileW(
        L"\\\\.\\HidHide",
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
}

bool HidHideControl::setCloakingEnabled(bool enabled) {
    HANDLE h = openHidHide();
    if (h == INVALID_HANDLE_VALUE) {
        LOG("WARNING: Could not open HidHide device - is HidHide installed?");
        return false;
    }

    DWORD bytes = 0;
    BOOL result = DeviceIoControl(
        h, IOCTL_SET_ACTIVE,
        &enabled, sizeof(enabled),
        nullptr, 0,
        &bytes, nullptr
    );

    CloseHandle(h);

    if (!result) {
        LOG("WARNING: HidHide IOCTL_SET_ACTIVE failed");
        return false;
    }

    LOG(enabled ? "HidHide cloaking enabled" : "HidHide cloaking disabled");
    return true;
}

bool HidHideControl::isCloakingEnabled() {
    HANDLE h = openHidHide();
    if (h == INVALID_HANDLE_VALUE) return false;

    bool active = false;
    DWORD bytes = 0;
    DeviceIoControl(
        h, IOCTL_GET_ACTIVE,
        nullptr, 0,
        &active, sizeof(active),
        &bytes, nullptr
    );

    CloseHandle(h);
    return active;
}
