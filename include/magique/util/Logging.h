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
        LEVEL_ALLOCATION, // Logs every allocation and deallocation
        LEVEL_INFO,       // Something happened
        LEVEL_WARNING,    // Something went wrong but it's okay
        LEVEL_ERROR,      // Something went wrong but it's NOT okay
        LEVEL_FATAL       // We have to shut down...
    };

    using LogCallbackFunc = void (*)(LogLevel level, const char* msg);

    //================= LOGGING =================//

    // Logs a message in a simpler interface
    void Log(LogLevel level, const char* msg, ...);

#define LOG_ALLOC(msg, ...) magique::Log(magique::LEVEL_ALLOCATION, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) magique::Log(magique::LEVEL_INFO, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) magique::Log(magique::LEVEL_WARNING, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) magique::Log(magique::LEVEL_ERROR, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) magique::Log(magique::LEVEL_FATAL, msg, ##__VA_ARGS__)

    //================= CONTROL =================//

    // Sets the minimal allowed log level
    void SetLogLevel(LogLevel level);

    // Sets the callback func - called on each log
    void SetLogCallback(LogCallbackFunc func);

} // namespace magique


//================= IMPLEMENTATION =================//

#if MAGIQUE_LOGGING == 0
#undef LOG_ALLOC
#undef LOG_INFO
#undef LOG_WARNING
#define LOG_ALLOC(msg, ...) ((void)0)
#define LOG_INFO(msg, ...) ((void)0)
#define LOG_WARNING(msg, ...) ((void)0)
#endif
#endif //MAGIQUE_LOGGING_H

namespace magique::internal
{
    // Internal log function
    void LogInternal(LogLevel level, const char* file, int line, const char* msg, va_list args);
} // namespace magique::internal