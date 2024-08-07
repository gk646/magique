#ifndef MAGIQUE_LOGGING_H
#define MAGIQUE_LOGGING_H

//-----------------------------------------------
// Logging Module
//-----------------------------------------------
// ................................................................................
// Use the provided macros for easier logging
// ................................................................................

namespace magique
{
    enum LogLevel : int
    {
        LEVEL_ALLOCATION, // Logs every allocation and deallocation
        LEVEL_INFO,       // Something happened
        LEVEL_WARNING,    // Something went wrong but its okay
        LEVEL_ERROR,      // Something went wrong but its NOT okay
        LEVEL_FATAL       // We have to shut down...
    };

    using LogCallbackFunc = void (*)(LogLevel level, const char* msg);

    //----------------- LOGGING -----------------//

    // Logs a message - use the macros for easier access
    void LogEx(LogLevel level, const char* file, int line, const char* msg, ...);

    //----------------- CONTROL -----------------//

    // Sets the minimal allowed log level
    void SetLogLevel(LogLevel level);

    // Sets the callback func - called on each log
    void SetLogCallback(LogCallbackFunc func);

} // namespace magique

// Use these macros for logging
#define LOG_ALLOC(msg, ...) magique::LogEx(magique::LEVEL_ALLOCATION, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) magique::LogEx(magique::LEVEL_INFO, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) magique::LogEx(magique::LEVEL_WARNING, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) magique::LogEx(magique::LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) magique::LogEx(magique::LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#endif //MAGIQUE_LOGGING_H