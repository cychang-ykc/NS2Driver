#pragma once
#include "../hid/UsbDevice.hpp"
#include "../hid/HidDevice.hpp"
#include "../vigem/VirtualController.hpp"
#include "HidInitializer.hpp"
#include "InputParser.hpp"
#include <functional>

static constexpr uint16_t NINTENDO_VID = 0x057e;
static constexpr uint16_t NS2_PRO_PID  = 0x2069;

class NS2Controller {
public:
    NS2Controller() = default;
    void run();
    void stop();

    // Optional callback to update UI (e.g. tray tooltip)
    std::function<void(const wchar_t*)> onStatusChange_;

private:
    VirtualController vigem_;
    bool              running_ = false;

    bool tryConnect();
    void disconnect();
    bool isControllerPresent();
    void readLoop();
    void processReport(const uint8_t* report, size_t len);

    UsbDevice* usb_ = nullptr;
    HidDevice* hid_ = nullptr;
};
