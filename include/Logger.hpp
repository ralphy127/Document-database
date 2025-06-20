#pragma once

#include <iostream>
#include <string>

class Logger {
public:
    /// @brief Log information to console
    /// @param message Message to log
    static void logInfo(std::string_view message) {
        std::cout << "[INFO] " << message << std::endl;
    }

    /// @brief Log warning to console
    /// @param message Message to log
    static void logWarning(std::string_view message) {
        std::cout << "[WARNING] " << message << std::endl;
    }

    /// @brief Log error to console
    /// @param message Message to log
    static void logError(std::string_view message) {
        std::cout << "[ERROR] " << message << std::endl;
    }
};