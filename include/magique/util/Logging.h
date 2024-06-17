#ifndef MAGIQUE_LOGGING_H
#define MAGIQUE_LOGGING_H

//-----------------------------------------------
// Logging Module
//-----------------------------------------------
// ................................................................................
// User the provided macros for easier logging
//
// ................................................................................

namespace magique
{
    enum LogLevel :  int
    {
        LEVEL_NONE,
        LEVEL_ALLOCATION, // Logs every allocation and deallocation
        LEVEL_INFO,
        LEVEL_WARNING,
        LEVEL_ERROR, // Only error and fatal are enabled in release builds
        LEVEL_FATAL
    };

    // Sets the minimum log level
    void SetLogLevel(LogLevel level);

    // Logs a message - use the macros for easier access
    // In release mode all expect errors and fatals will be defined to a null op for zero overhead
    void Log(LogLevel level, const char* file, int line, const char* msg, ...);

} // namespace magique

// Use these macros for logging

#define LOG_ALLOC(msg, ...) magique::Log(magique::LEVEL_ALLOCATION, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) magique::Log(magique::LEVEL_INFO, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) magique::Log(magique::LEVEL_WARNING, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) magique::Log(magique::LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) magique::Log(magique::LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)


#endif //MAGIQUE_LOGGING_H