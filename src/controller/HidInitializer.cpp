#include "HidInitializer.hpp"
#include "../utils/Logger.hpp"
#include <thread>
#include <chrono>

// Source: djedditt/nsw2usb-con.py (confirmed for PID 0x2069)
// Cross-referenced with ikz87/NSW2-controller-enabler (steps 1 and 17 match exactly)
static const uint8_t CMD_INIT[] = {
    0x03, 0x91, 0x00, 0x0D, 0x00, 0x08, 0x00, 0x00,
    0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// Player 1 LED command
static const uint8_t CMD_SET_PLAYER_LED[] = {
    0x09, 0x91, 0x00, 0x07, 0x00, 0x08, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

bool HidInitializer::initialize() {
    LOG("Sending USB init sequence via libusb (Interface 1)...");

    if (!sendCommand(CMD_INIT, sizeof(CMD_INIT))) {
        LOG("ERROR: Failed to send INIT command");
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (!sendCommand(CMD_SET_PLAYER_LED, sizeof(CMD_SET_PLAYER_LED))) {
        LOG("ERROR: Failed to send SET_PLAYER_LED command");
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    LOG("Init sequence complete - all 4 LEDs should now be lit");
    return true;
}

bool HidInitializer::sendCommand(const uint8_t* cmd, size_t len) {
    return usb_.bulkWrite(cmd, len);
}
