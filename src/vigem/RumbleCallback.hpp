#pragma once
#include "../hid/HidDevice.hpp"

// Receives rumble notifications from ViGEmBus and forwards them to the controller.
// TODO (Phase 4): Wire up ViGEm rumble callback.
class RumbleCallback {
public:
    explicit RumbleCallback(HidDevice& device) : device_(device) {}
    void onRumble(uint8_t left_motor, uint8_t right_motor);

private:
    HidDevice& device_;
};
