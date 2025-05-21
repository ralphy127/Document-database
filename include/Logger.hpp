#pragma once

#include <iostream>
#include <string>

class Logger {
public:
    static void logInfo(std::string_view message) {
        std::cout << "[INFO] " << message << std::endl;
    }

    static void logWarning(std::string_view message) {
        std::cout << "[WARNING] " << message << std::endl;
    }

    static void logError(std::string_view message) {
        std::cout << "[ERROR] " << message << std::endl;
    }
};