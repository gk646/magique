#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <magique/util/Logging.h>

#include "internal/globals/Configuration.h"

namespace magique
{
    void SetLogLevel(const LogLevel level) { global::CONFIGURATION.logLevel = level; }

    void Log(const LogLevel level, const char* file, const int line, const char* msg, ...)
    {
        if (level < global::CONFIGURATION.logLevel)
        {
            return;
        }

        auto level_str = "";
        switch (level)
        {
        case LEVEL_INFO:
            level_str = "INFO";
            break;
        case LEVEL_WARNING:
            level_str = "WARNING";
            break;
        case LEVEL_ERROR:
            level_str = "ERROR";
            break;
        case LEVEL_FATAL:
            level_str = "FATAL";
            break;
        case LEVEL_ALLOCATION:
            level_str = "ALLOC";
            break;
        case LEVEL_NONE:
            return;
        }

        // Log to stdout or stderr based on log level
        FILE* out = level == LEVEL_ERROR || level == LEVEL_FATAL ? stderr : stdout;

        if (level >= LEVEL_ERROR)
        {
            // Only show level and message for warnings
            fprintf(out, "[%s]: %s:%d ", level_str, file, line);
        }
        else
        {
            fprintf(out, "[%s]: ", level_str);
        }


        // Handle the variable arguments
        va_list args;
        va_start(args, msg);
        vfprintf(out, msg, args);
        va_end(args);

        fprintf(out, "\n");

        if (level >= LEVEL_ERROR) [[unlikely]]
        {
#if MAGIQUE_DEBUG == 1
#if defined(_MSC_VER)
            __debugbreak();
#elif defined(__GNUC__)
            __builtin_trap();
#else
            std::abort();
#endif
#endif
            if (level == LEVEL_FATAL) [[unlikely]]
                exit(1);
        }
    }

} // namespace magique