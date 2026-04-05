#include "TrayIcon.hpp"
#include <shellapi.h>

bool TrayIcon::create(HWND hwnd, const wchar_t* tooltip) {
    nid_.cbSize           = sizeof(NOTIFYICONDATAW);
    nid_.hWnd             = hwnd;
    nid_.uID              = ICON_UID;
    nid_.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAYICON;
    nid_.hIcon            = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
    wcsncpy_s(nid_.szTip, ARRAYSIZE(nid_.szTip), tooltip, _TRUNCATE);

    created_ = Shell_NotifyIconW(NIM_ADD, &nid_) != FALSE;
    return created_;
}

void TrayIcon::destroy() {
    if (created_) {
        Shell_NotifyIconW(NIM_DELETE, &nid_);
        created_ = false;
    }
}

void TrayIcon::setTooltip(const wchar_t* tooltip) {
    wcsncpy_s(nid_.szTip, ARRAYSIZE(nid_.szTip), tooltip, _TRUNCATE);
    nid_.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &nid_);
}

void TrayIcon::showContextMenu(HWND hwnd, bool autostartEnabled) {
    HMENU menu = CreatePopupMenu();

    UINT autostartFlags = MF_STRING | (autostartEnabled ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(menu, autostartFlags, IDM_AUTOSTART, L"Launch at startup");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, IDM_EXIT, L"Exit NS2 Driver");

    // Must set foreground window for menu to dismiss correctly
    SetForegroundWindow(hwnd);

    POINT pt;
    GetCursorPos(&pt);
    TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}
