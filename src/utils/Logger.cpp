#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>

void Logger::init() {
    // Future: open log file here
}

void Logger::log(const std::string& message, const char* file, int line) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char timebuf[20];
    std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", std::localtime(&t));
    std::cout << "[" << timebuf << "] " << message << "\n";
}
