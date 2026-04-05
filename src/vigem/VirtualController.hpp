#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../controller/InputParser.hpp"
#include <ViGEm/Client.h>

class VirtualController {
public:
    VirtualController() = default;
    ~VirtualController();

    bool connect();
    void disconnect();
    void update(const ControllerState& state);

private:
    PVIGEM_CLIENT client_ = nullptr;
    PVIGEM_TARGET target_ = nullptr;
};
