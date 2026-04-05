#include "HidDevice.hpp"
#include "../utils/Logger.hpp"
#include <cstring>

HidDevice::~HidDevice() {
    close();
}

bool HidDevice::open(uint16_t vendor_id, uint16_t product_id) {
    if (hid_init() != 0) {
        LOG("ERROR: hid_init() failed");
        return false;
    }
    handle_ = hid_open(vendor_id, product_id, nullptr);
    if (!handle_) {
        LOG("ERROR: hid_open() failed - is the controller plugged in?");
        return false;
    }
    hid_set_nonblocking(handle_, 0);  // blocking: read() waits until data arrives
    LOG("HID device opened successfully");
    return true;
}

void HidDevice::close() {
    if (handle_) {
        hid_close(handle_);
        handle_ = nullptr;
    }
    hid_exit();
}

bool HidDevice::write(const uint8_t* data, size_t length) {
    // hidapi write requires a leading 0x00 report ID byte
    std::vector<uint8_t> buf(length + 1);
    buf[0] = 0x00;
    memcpy(buf.data() + 1, data, length);
    int res = hid_write(handle_, buf.data(), buf.size());
    return res >= 0;
}

int HidDevice::read(uint8_t* buf, size_t length, int timeout_ms) {
    // timeout_ms = -1: block until data arrives (lowest latency)
    return hid_read_timeout(handle_, buf, length, timeout_ms);
}
