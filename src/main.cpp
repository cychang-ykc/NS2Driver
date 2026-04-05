#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <thread>
#include <string>
#include "controller/NS2Controller.hpp"
#include "ui/TrayIcon.hpp"
#include "utils/Logger.hpp"

static NS2Controller* g_controller = nullptr;
static TrayIcon        g_tray;

static constexpr wchar_t TASK_NAME[] = L"NS2DriverAutoStart";

// Returns true if this process is already running with admin/elevated token
static bool isElevated() {
    BOOL elevated = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elev = {};
        DWORD size = 0;
        if (GetTokenInformation(token, TokenElevation, &elev, sizeof(elev), &size))
            elevated = elev.TokenIsElevated;
        CloseHandle(token);
    }
    return elevated != FALSE;
}

// Run schtasks.exe with given params.
// If already elevated: run directly (no UAC prompt).
// If not elevated: use runas to request elevation (shows UAC prompt once).
static bool runSchtasks(const std::wstring& params) {
    wchar_t sysDir[MAX_PATH] = {};
    GetSystemDirectoryW(sysDir, MAX_PATH);
    std::wstring exe = std::wstring(sysDir) + L"\\schtasks.exe";

    SHELLEXECUTEINFOW sei = {};
    sei.cbSize       = sizeof(sei);
    sei.fMask        = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb       = isElevated() ? L"open" : L"runas";
    sei.lpFile       = exe.c_str();
    sei.lpParameters = params.c_str();
    sei.nShow        = SW_HIDE;

    if (!ShellExecuteExW(&sei)) return false;
    if (sei.hProcess) {
        WaitForSingleObject(sei.hProcess, 10000);
        DWORD code = 1;
        GetExitCodeProcess(sei.hProcess, &code);
        CloseHandle(sei.hProcess);
        return code == 0;
    }
    return false;
}

static bool isAutostartEnabled() {
    // Tasks are stored as files in System32\Tasks — readable without elevation
    wchar_t sysDir[MAX_PATH] = {};
    GetSystemDirectoryW(sysDir, MAX_PATH);
    std::wstring taskFile = std::wstring(sysDir) + L"\\Tasks\\" + TASK_NAME;
    return GetFileAttributesW(taskFile.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static void setAutostart(bool enable) {
    if (enable) {
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::wstring params = std::wstring(L"/Create /F /TN \"") + TASK_NAME +
                              L"\" /TR \"" + exePath +
                              L"\" /SC ONLOGON /RL HIGHEST /IT";
        runSchtasks(params);
    } else {
        runSchtasks(std::wstring(L"/Delete /F /TN \"") + TASK_NAME + L"\"");
    }
}

// Hidden message-only window procedure
LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case TrayIcon::WM_TRAYICON:
        if (lp == WM_RBUTTONUP || lp == WM_LBUTTONUP) {
            g_tray.showContextMenu(hwnd, isAutostartEnabled());
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wp) == TrayIcon::IDM_AUTOSTART) {
            bool current = isAutostartEnabled();
            setAutostart(!current);
        } else if (LOWORD(wp) == TrayIcon::IDM_EXIT) {
            if (g_controller) g_controller->stop();
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        g_tray.destroy();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    Logger::init();
    LOG("ns2drv starting...");

    // Register a message-only window class (no visible window)
    WNDCLASSEXW wc  = {};
    wc.cbSize       = sizeof(wc);
    wc.lpfnWndProc  = wndProc;
    wc.hInstance    = hInst;
    wc.lpszClassName = L"NS2DrvTray";
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0, L"NS2DrvTray", L"NS2 Driver",
        0, 0, 0, 0, 0,
        HWND_MESSAGE,   // message-only window, never visible
        nullptr, hInst, nullptr
    );

    // Create tray icon
    g_tray.create(hwnd, L"NS2 Pro Controller Driver - Waiting for controller...");

    // Run controller on background thread
    NS2Controller controller;
    g_controller = &controller;

    controller.onStatusChange_ = [&](const wchar_t* status) {
        g_tray.setTooltip(status);
    };

    std::thread workerThread([&]() {
        controller.run();
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
    });

    // Attach a callback to update tray tooltip on connect/disconnect
    // (simple polling via a timer)
    SetTimer(hwnd, 1, 1000, [](HWND h, UINT, UINT_PTR, DWORD) {
        // Tooltip is updated by the controller via g_tray
    });

    // Windows message loop (blocks until PostQuitMessage)
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    controller.stop();
    workerThread.join();

    LOG("ns2drv exited.");
    return 0;
}
