cmake_minimum_required(VERSION 3.20)
project(ns2-win-driver VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg toolchain (set via -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(hidapi CONFIG REQUIRED)
find_path(LIBUSB_INCLUDE_DIR libusb-1.0/libusb.h REQUIRED)
find_library(LIBUSB_LIBRARY NAMES libusb-1.0 libusb REQUIRED)

# ViGEmClient (built from source in third_party/)
add_subdirectory(third_party/ViGEmClient)

set(SOURCES
    src/ns2drv.rc
    src/main.cpp
    src/ui/TrayIcon.cpp
    src/controller/NS2Controller.cpp
    src/controller/HidInitializer.cpp
    src/controller/InputParser.cpp
    src/controller/RumbleEncoder.cpp
    src/vigem/VirtualController.cpp
    src/vigem/RumbleCallback.cpp
    src/hid/HidDevice.cpp
    src/hid/UsbDevice.cpp
    src/utils/Logger.cpp
    src/utils/Config.cpp
)

add_executable(ns2drv WIN32 ${SOURCES})

target_link_libraries(ns2drv PRIVATE
    hidapi::hidapi
    ViGEmClient::ViGEmClient
    setupapi
)

target_include_directories(ns2drv PRIVATE ${LIBUSB_INCLUDE_DIR})
target_link_libraries(ns2drv PRIVATE ${LIBUSB_LIBRARY})
