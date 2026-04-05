#pragma once
#include <string>

class Logger {
public:
    static void init();
    static void log(const std::string& message, const char* file, int line);
};

#define LOG(msg) Logger::log(msg, __FILE__, __LINE__)
