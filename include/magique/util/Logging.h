#ifndef MAGIQUE_LOGGING_H
#define MAGIQUE_LOGGING_H

// Use these macros for logging

#define LOG(level, msg, ...) magique::util::Log(level, __FILE__, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ALLOC(msg, ...) LOG(magique::util::LEVEL_ALLOCATION, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) LOG(magique::util::LEVEL_INFO, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) LOG(magique::util::LEVEL_WARNING, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) LOG(magique::util::LEVEL_ERROR, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...) LOG(magique::util::LEVEL_FATAL, msg, ##__VA_ARGS__)

namespace magique::util
{
    enum LogLevel
    {
        LEVEL_NONE,
        LEVEL_ALLOCATION, // Logs every allocation and deallocation
        LEVEL_INFO,
        LEVEL_WARNING,
        LEVEL_ERROR,
        LEVEL_FATAL
    };

    // Sets the minimum log level
    void SetLogLevel(LogLevel level);

    // Logs a message - use the macros for easier access
    // In release mode all expect errors and fatals will be defined to a null op for zero overhead
    void Log(LogLevel level, const char* file, int line, const char* msg, ...);

} // namespace magique::util


#endif //MAGIQUE_LOGGING_H