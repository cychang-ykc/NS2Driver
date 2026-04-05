#include "RumbleEncoder.hpp"

// TODO (Phase 4): Replace with actual HD Rumble encoding.
// Reference: dekuNukem/Nintendo_Switch_Reverse_Engineering/rumble_data_table.md
RumbleData RumbleEncoder::encode(uint16_t left_motor, uint16_t right_motor) {
    RumbleData data{};

    // Neutral (no rumble) pattern from NS1 format
    data.left[0]  = 0x00;
    data.left[1]  = 0x01;
    data.left[2]  = 0x40;
    data.left[3]  = 0x40;

    data.right[0] = 0x00;
    data.right[1] = 0x01;
    data.right[2] = 0x40;
    data.right[3] = 0x40;

    return data;
}
