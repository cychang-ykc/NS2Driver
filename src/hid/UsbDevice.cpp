#include "UsbDevice.hpp"
#include "../utils/Logger.hpp"
#include <thread>
#include <chrono>

UsbDevice::~UsbDevice() {
    close();
}

bool UsbDevice::open(uint16_t vendor_id, uint16_t product_id, int interface_num) {
    interface_ = interface_num;

    if (libusb_init(&ctx_) < 0) {
        LOG("ERROR: libusb_init() failed");
        return false;
    }

    handle_ = libusb_open_device_with_vid_pid(ctx_, vendor_id, product_id);
    if (!handle_) {
        LOG("ERROR: libusb could not find device - is controller plugged in?");
        return false;
    }

    // Detach kernel driver from Interface 1 if active (mainly for Linux,
    // on Windows this is a no-op but safe to call)
    if (libusb_kernel_driver_active(handle_, interface_) == 1) {
        if (libusb_detach_kernel_driver(handle_, interface_) < 0) {
            LOG("WARNING: Could not detach kernel driver from interface");
        }
    }

    // set_configuration may fail if device is already configured (e.g. plugged in at boot) - non-fatal
    if (libusb_set_configuration(handle_, 1) < 0) {
        LOG("WARNING: libusb_set_configuration() failed (device may already be configured, continuing)");
    }

    if (libusb_claim_interface(handle_, interface_) < 0) {
        // Interface busy: reset the device to force Windows to release it, then retry once
        LOG("WARNING: libusb_claim_interface() failed - resetting device and retrying...");
        libusb_reset_device(handle_);
        libusb_close(handle_);
        handle_ = nullptr;
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));

        handle_ = libusb_open_device_with_vid_pid(ctx_, vendor_id, product_id);
        if (!handle_) {
            LOG("ERROR: Device lost after reset");
            return false;
        }
        if (libusb_claim_interface(handle_, interface_) < 0) {
            LOG("ERROR: libusb_claim_interface() failed after reset");
            return false;
        }
    }

    // Find bulk OUT endpoint on Interface 1
    libusb_config_descriptor* config = nullptr;
    libusb_get_active_config_descriptor(
        libusb_get_device(handle_), &config);

    if (config && config->bNumInterfaces > interface_) {
        const libusb_interface& iface = config->interface[interface_];
        if (iface.num_altsetting > 0) {
            const libusb_interface_descriptor& desc = iface.altsetting[0];
            for (int i = 0; i < desc.bNumEndpoints; i++) {
                const libusb_endpoint_descriptor& ep = desc.endpoint[i];
                bool is_out = (ep.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
                              == LIBUSB_ENDPOINT_OUT;
                bool is_bulk = (ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
                               == LIBUSB_TRANSFER_TYPE_BULK;
                if (is_out && is_bulk) {
                    ep_out_ = ep.bEndpointAddress;
                    break;
                }
            }
        }
        libusb_free_config_descriptor(config);
    }

    if (ep_out_ == 0) {
        LOG("ERROR: Could not find bulk OUT endpoint on Interface 1");
        return false;
    }

    LOG("UsbDevice: bulk OUT endpoint found, Interface 1 claimed");
    return true;
}

void UsbDevice::close() {
    if (handle_) {
        libusb_release_interface(handle_, interface_);
        libusb_close(handle_);
        handle_ = nullptr;
    }
    if (ctx_) {
        libusb_exit(ctx_);
        ctx_ = nullptr;
    }
}

bool UsbDevice::bulkWrite(const uint8_t* data, size_t length, unsigned int timeout_ms) {
    int transferred = 0;
    int res = libusb_bulk_transfer(
        handle_, ep_out_,
        const_cast<uint8_t*>(data),
        static_cast<int>(length),
        &transferred,
        timeout_ms
    );
    if (res < 0) {
        LOG(std::string("ERROR: libusb_bulk_transfer failed: ") + libusb_error_name(res));
        return false;
    }
    return true;
}
