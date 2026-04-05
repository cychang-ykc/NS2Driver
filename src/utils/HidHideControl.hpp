#pragma once

// Controls HidHide cloaking state via IOCTL.
// Enables hiding on app start, disables on app stop.
class HidHideControl {
public:
    static bool setCloakingEnabled(bool enabled);
    static bool isCloakingEnabled();
};
