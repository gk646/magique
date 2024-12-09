// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOGGING_H
#define MAGIQUE_LOGGING_H

#include <cstdarg>

//===============================================
// Logging Module
//===============================================
// ................................................................................
// Use the provided macros for easier logging
// Note: Uses stdout and stderr for writing strings
// ................................................................................

namespace magique
{
    enum LogLevel : int
    {
        LEVEL_INFO,    // Something happened
        LEVEL_WARNING, // Something went wrong but it's okay
        LEVEL_ERROR,   // Something went wrong but it's NOT okay
        LEVEL_FATAL    // We have to shut down...
    };

    using LogCallbackFunc = void (*)(LogLevel level, const char* msg);

    //================= LOGGING =================//

    // Logs a message with the given level and message
    void Log(LogLevel level, const char* msg, ...);

    // Logs a message with extended parameters - use the macros for easier access
    void LogEx(LogLevel level, const char* file, int line, const char* msg, ...);

    // Macros that call LogEx internally with the given level
#define LOG_INFO(msg, ...)
#define LOG_WARNING(msg, ...)
#define LOG_ERROR(msg, ...)
#define LOG_FATAL(msg, ...)

    //================= CONTROL =================//

    // Sets the minimal allowed log level
    void SetLogLevel(LogLevel level);

    // Sets the callback func - called on each log
    void SetLogCallback(LogCallbackFunc func);

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique::internal
{
    // Internal log function
    void LogInternal(LogLevel level, const char* file, int line, const char* msg, va_list args);
} // namespace magique::internal

#undef LOG_INFO
#undef LOG_WARNING
#undef LOG_ERROR
#undef LOG_FATAL
#define LOG_INFO(msg, ...) magique::LogEx(magique::LEVEL_INFO, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) magique::LogEx(magique::LEVEL_WARNING, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) magique::LogEx(magique::LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) magique::LogEx(magique::LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#endif //MAGIQUE_LOGGING_H