#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

class TrayIcon {
public:
    // Message posted to hwnd when user interacts with tray icon
    static const UINT WM_TRAYICON  = WM_APP + 1;
    // Menu item IDs
    static const UINT IDM_EXIT      = 1001;
    static const UINT IDM_AUTOSTART = 1002;
    // Tray icon UID
    static const UINT ICON_UID      = 1;

    bool create(HWND hwnd, const wchar_t* tooltip);
    void destroy();
    void setTooltip(const wchar_t* tooltip);
    void showContextMenu(HWND hwnd, bool autostartEnabled);

private:
    NOTIFYICONDATAW nid_ = {};
    bool created_ = false;
};
