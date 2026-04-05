#pragma once
#include <libusb-1.0/libusb.h>
#include <cstdint>

// Handles raw USB bulk endpoint communication (Interface 1).
// Used only for the init sequence, same as pyusb in the reference scripts.
class UsbDevice {
public:
    UsbDevice() = default;
    ~UsbDevice();

    bool open(uint16_t vendor_id, uint16_t product_id, int interface_num = 1);
    void close();
    bool bulkWrite(const uint8_t* data, size_t length, unsigned int timeout_ms = 1000);

private:
    libusb_context*       ctx_      = nullptr;
    libusb_device_handle* handle_   = nullptr;
    int                   interface_ = 1;
    uint8_t               ep_out_   = 0;
};
