#include "NS2Controller.hpp"
#include "InputParser.hpp"
#include "../utils/Logger.hpp"
#include <hidapi.h>
#include <thread>
#include <chrono>

void NS2Controller::run() {
    running_ = true;

    LOG("Waiting for NS2 Pro Controller (plug in USB-C)...");

    while (running_) {
        if (isControllerPresent()) {
            LOG("Controller detected, initializing...");
            if (onStatusChange_) onStatusChange_(L"NS2 Driver - Initializing...");
            if (tryConnect()) {
                LOG("Controller ready. Reading input.");
                if (onStatusChange_) onStatusChange_(L"NS2 Driver - Controller connected");
                readLoop();
                disconnect();
                if (running_) {
                    LOG("Controller disconnected. Waiting for reconnect...");
                    if (onStatusChange_) onStatusChange_(L"NS2 Driver - Waiting for controller...");
                }
            } else {
                LOG("Init failed. Retrying in 2 seconds...");
                if (onStatusChange_) onStatusChange_(L"NS2 Driver - Init failed, retrying...");
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

void NS2Controller::stop() {
    running_ = false;
}

bool NS2Controller::isControllerPresent() {
    struct hid_device_info* devs = hid_enumerate(NINTENDO_VID, NS2_PRO_PID);
    bool found = (devs != nullptr);
    hid_free_enumeration(devs);
    return found;
}

bool NS2Controller::tryConnect() {
    // Step 1: libusb init sequence
    usb_ = new UsbDevice();
    if (!usb_->open(NINTENDO_VID, NS2_PRO_PID, 1)) {
        delete usb_; usb_ = nullptr;
        return false;
    }
    HidInitializer init(*usb_);
    if (!init.initialize()) {
        delete usb_; usb_ = nullptr;
        return false;
    }
    usb_->close();
    delete usb_; usb_ = nullptr;

    // Step 2: wait for HID device to settle
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Step 3: open HID for reading
    hid_ = new HidDevice();
    if (!hid_->open(NINTENDO_VID, NS2_PRO_PID)) {
        delete hid_; hid_ = nullptr;
        return false;
    }

    // Step 4: create virtual controller (appears in system now)
    if (!vigem_.connect()) {
        LOG("ERROR: Failed to connect ViGEmBus");
        delete hid_; hid_ = nullptr;
        return false;
    }

    return true;
}

void NS2Controller::disconnect() {
    vigem_.disconnect();   // remove virtual controller from system
    if (hid_) {
        delete hid_;
        hid_ = nullptr;
    }
}

void NS2Controller::readLoop() {
    uint8_t report[64] = {};
    while (running_) {
        int res = hid_->read(report, sizeof(report), 100);
        if (res > 0) {
            processReport(report, static_cast<size_t>(res));
        } else if (res < 0) {
            // Read error = controller disconnected
            break;
        }
    }
}

void NS2Controller::processReport(const uint8_t* report, size_t len) {
    ControllerState state{};
    if (!InputParser::parse(report, len, state)) return;
    vigem_.update(state);
}
