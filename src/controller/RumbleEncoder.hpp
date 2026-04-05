#pragma once
#include <cstdint>

struct RumbleData {
    uint8_t left[4];   // HD Rumble bytes for left motor
    uint8_t right[4];  // HD Rumble bytes for right motor
};

class RumbleEncoder {
public:
    // Encode XInput motor values (0-65535) into NS2 HD Rumble format.
    // TODO (Phase 4): Implement using NS1 format from dekuNukem/NS_RE,
    // or from USB captures if NS1 format doesn't work.
    static RumbleData encode(uint16_t left_motor, uint16_t right_motor);
};
