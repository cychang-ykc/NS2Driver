#include "RumbleCallback.hpp"
#include "../controller/RumbleEncoder.hpp"
#include "../utils/Logger.hpp"

void RumbleCallback::onRumble(uint8_t left_motor, uint8_t right_motor) {
    // TODO (Phase 4): Encode and send rumble command via HidDevice
    auto data = RumbleEncoder::encode(
        static_cast<uint16_t>(left_motor  * 257),
        static_cast<uint16_t>(right_motor * 257)
    );
    (void)data;
    LOG("Rumble callback - stub, implement in Phase 4");
}
