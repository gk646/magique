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
        LEVEL_WARNING,    // Something went wrong but it's okay
        LEVEL_ERROR,      // Something went wrong but it's NOT okay
        LEVEL_FATAL       // We have to shut down...
    };

    using LogCallbackFunc = void (*)(LogLevel level, const char* msg);

    //----------------- LOGGING -----------------//

    // Logs a message in a simpler interface
    void Log(LogLevel level, const char* msg, ...);

    // Logs a message with extended parameters - use the macros for easier access
    void LogEx(LogLevel level, const char* file, int line, const char* msg, ...);

#define LOG_ALLOC(msg, ...) magique::LogEx(magique::LEVEL_ALLOCATION, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) magique::LogEx(magique::LEVEL_INFO, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) magique::LogEx(magique::LEVEL_WARNING, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) magique::LogEx(magique::LEVEL_ERROR, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) magique::LogEx(magique::LEVEL_FATAL, __FILE__, __LINE__, msg, ##__VA_ARGS__)

    //----------------- CONTROL -----------------//

    // Sets the minimal allowed log level
    void SetLogLevel(LogLevel level);

    // Sets the callback func - called on each log
    void SetLogCallback(LogCallbackFunc func);

} // namespace magique


//----------------- IMPLEMENTATION -----------------//

#if MAGIQUE_LOGGING == 0
#undef LOG_ALLOC
#undef LOG_INFO
#undef LOG_WARNING
#define LOG_ALLOC(msg, ...) ((void)0)
#define LOG_INFO(msg, ...) ((void)0)
#define LOG_WARNING(msg, ...) ((void)0)
#endif
#endif //MAGIQUE_LOGGING_H