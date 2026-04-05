#include "InputParser.hpp"
#include <cstring>

// Source: ikz87/NSW2-controller-enabler enable_hid.py lines 289-339
// payload = report[1:]  (first byte is report ID)
// buttons = payload[2:5]  → report[3], report[4], report[5]
// stick1  = payload[5:8]  → report[6..8]
// stick2  = payload[8:11] → report[9..11]

static void unpack12bit(const uint8_t* data, uint16_t& a, uint16_t& b) {
    a = data[0] | ((data[1] & 0x0F) << 8);
    b = (data[1] >> 4) | (data[2] << 4);
}

static uint8_t remapTrigger(uint8_t raw) {
    // Raw range 36-240 → 0-255
    if (raw <= 36)  return 0;
    if (raw >= 240) return 255;
    return static_cast<uint8_t>(((raw - 36) / 204.0f) * 255.0f);
}

bool InputParser::parse(const uint8_t* report, size_t len, ControllerState& out) {
    if (len < 15) return false;

    // Button byte 0 → report[3]
    uint8_t b0 = report[3];
    out.btn_b      = (b0 & 0x01) != 0;
    out.btn_a      = (b0 & 0x02) != 0;
    out.btn_y      = (b0 & 0x04) != 0;
    out.btn_x      = (b0 & 0x08) != 0;
    out.btn_r      = (b0 & 0x10) != 0;
    out.btn_zr     = (b0 & 0x20) != 0;
    out.btn_plus   = (b0 & 0x40) != 0;
    out.btn_rstick = (b0 & 0x80) != 0;

    // Button byte 1 → report[4]
    uint8_t b1 = report[4];
    out.dpad_down  = (b1 & 0x01) != 0;
    out.dpad_right = (b1 & 0x02) != 0;
    out.dpad_left  = (b1 & 0x04) != 0;
    out.dpad_up    = (b1 & 0x08) != 0;
    out.btn_l      = (b1 & 0x10) != 0;
    out.btn_zl     = (b1 & 0x20) != 0;
    out.btn_minus  = (b1 & 0x40) != 0;
    out.btn_lstick = (b1 & 0x80) != 0;

    // Button byte 2 → report[5]
    uint8_t b2 = report[5];
    out.btn_home       = (b2 & 0x01) != 0;
    out.btn_capture    = (b2 & 0x02) != 0;
    out.btn_back_right = (b2 & 0x04) != 0;  // confirmed from raw data
    out.btn_back_left  = (b2 & 0x08) != 0;  // confirmed from raw data

    // Sticks: 12-bit packed, 3 bytes each, center = 2048
    uint16_t lx12, ly12, rx12, ry12;
    unpack12bit(&report[6], lx12, ly12);
    unpack12bit(&report[9], rx12, ry12);

    // Convert 12-bit (0-4095, center 2048) → 8-bit (0-255, center 128)
    out.lx = static_cast<uint8_t>(lx12 >> 4);
    out.ly = static_cast<uint8_t>(ly12 >> 4);
    out.rx = static_cast<uint8_t>(rx12 >> 4);
    out.ry = static_cast<uint8_t>(ry12 >> 4);

    // Triggers
    out.left_trigger  = remapTrigger(report[13]);
    out.right_trigger = remapTrigger(report[14]);

    return true;
}
