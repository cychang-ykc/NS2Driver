#include "VirtualController.hpp"
#include "../utils/Logger.hpp"

VirtualController::~VirtualController() {
    disconnect();
}

bool VirtualController::connect() {
    client_ = vigem_alloc();
    if (!client_) {
        LOG("ERROR: vigem_alloc() failed");
        return false;
    }

    VIGEM_ERROR err = vigem_connect(client_);
    if (!VIGEM_SUCCESS(err)) {
        LOG("ERROR: vigem_connect() failed - is ViGEmBus installed?");
        vigem_free(client_);
        client_ = nullptr;
        return false;
    }

    target_ = vigem_target_x360_alloc();
    if (!target_) {
        LOG("ERROR: vigem_target_x360_alloc() failed");
        return false;
    }

    err = vigem_target_add(client_, target_);
    if (!VIGEM_SUCCESS(err)) {
        LOG("ERROR: vigem_target_add() failed");
        vigem_target_free(target_);
        target_ = nullptr;
        return false;
    }

    LOG("Virtual Xbox 360 controller connected");
    return true;
}

void VirtualController::disconnect() {
    if (client_ && target_) {
        vigem_target_remove(client_, target_);
        vigem_target_free(target_);
        target_ = nullptr;
    }
    if (client_) {
        vigem_disconnect(client_);
        vigem_free(client_);
        client_ = nullptr;
    }
}

// Convert 8-bit stick (0-255, center=128) to XInput int16 (-32768 to 32767)
static int16_t stickToXInput(uint8_t val) {
    int32_t v = (static_cast<int32_t>(val) - 128) * 258;
    if (v >  32767) v =  32767;
    if (v < -32768) v = -32768;
    return static_cast<int16_t>(v);
}

void VirtualController::update(const ControllerState& state) {
    if (!client_ || !target_) return;

    XUSB_REPORT report = {};

    // Face buttons: positional mapping (Nintendo layout → Xbox layout)
    // NS2 right=A, bottom=B, top=X, left=Y
    // Xbox right=B, bottom=A, top=Y, left=X
    if (state.btn_a)      report.wButtons |= XUSB_GAMEPAD_B;       // right
    if (state.btn_b)      report.wButtons |= XUSB_GAMEPAD_A;       // bottom
    if (state.btn_x)      report.wButtons |= XUSB_GAMEPAD_Y;       // top
    if (state.btn_y)      report.wButtons |= XUSB_GAMEPAD_X;       // left
    if (state.btn_l)      report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
    if (state.btn_r)      report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
    if (state.btn_plus)   report.wButtons |= XUSB_GAMEPAD_START;
    if (state.btn_minus)  report.wButtons |= XUSB_GAMEPAD_BACK;
    if (state.btn_home)   report.wButtons |= XUSB_GAMEPAD_GUIDE;
    if (state.btn_lstick) report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
    if (state.btn_rstick) report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;

    // D-pad
    if (state.dpad_up)    report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
    if (state.dpad_down)  report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
    if (state.dpad_left)  report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
    if (state.dpad_right) report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;

    // ZL/ZR → XInput triggers (0-255)
    report.bLeftTrigger  = state.btn_zl ? 255 : state.left_trigger;
    report.bRightTrigger = state.btn_zr ? 255 : state.right_trigger;

    // Sticks
    report.sThumbLX = stickToXInput(state.lx);
    report.sThumbLY = stickToXInput(state.ly);  // UP=bigger raw value, matches XInput convention
    report.sThumbRX = stickToXInput(state.rx);
    report.sThumbRY = stickToXInput(state.ry);  // verify R stick direction in-game

    vigem_target_x360_update(client_, target_, report);
}
