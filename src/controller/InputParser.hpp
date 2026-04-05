#pragma once
#include <cstdint>

struct ControllerState {
    // Face buttons
    bool btn_a, btn_b, btn_x, btn_y;
    // Shoulder
    bool btn_l, btn_r, btn_zl, btn_zr;
    // Menu
    bool btn_plus, btn_minus;
    bool btn_home, btn_capture;
    // Stick clicks
    bool btn_lstick, btn_rstick;
    // D-pad (individual bits, easier to map to XInput POV)
    bool dpad_up, dpad_down, dpad_left, dpad_right;
    // Sticks (0-255, center=128)
    uint8_t lx, ly, rx, ry;
    // Triggers (0-255)
    uint8_t left_trigger, right_trigger;
    // Back buttons (NS2 Pro exclusive, byte[5])
    bool btn_back_left, btn_back_right;
};

class InputParser {
public:
    // Parses a 64-byte HID input report into ControllerState.
    // Returns false if report is too short or invalid.
    static bool parse(const uint8_t* report, size_t len, ControllerState& out);
};
