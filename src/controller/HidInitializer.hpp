#pragma once
#include "../hid/UsbDevice.hpp"

// Sends the USB init sequence via libusb bulk write to Interface 1.
// Mirrors the pyusb approach in ikz87/NSW2-controller-enabler and djedditt/nsw2usb-con.
class HidInitializer {
public:
    explicit HidInitializer(UsbDevice& usb) : usb_(usb) {}
    bool initialize();

private:
    UsbDevice& usb_;
    bool sendCommand(const uint8_t* cmd, size_t len);
};
