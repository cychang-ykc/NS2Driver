#pragma once
#include <hidapi.h>
#include <cstdint>
#include <vector>

class HidDevice {
public:
    HidDevice() = default;
    ~HidDevice();

    bool open(uint16_t vendor_id, uint16_t product_id);
    void close();
    bool write(const uint8_t* data, size_t length);
    int read(uint8_t* buf, size_t length, int timeout_ms = 100);
    bool isOpen() const { return handle_ != nullptr; }

private:
    hid_device* handle_ = nullptr;
};
