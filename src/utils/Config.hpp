#pragma once

struct Config {
    float deadzone_left  = 0.1f;
    float deadzone_right = 0.1f;

    static Config load();  // Future: load from .ini file
};
