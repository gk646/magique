#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <magique/util/Logging.h>

#include "internal/globals/EngineConfig.h"

namespace magique
{
    LogCallbackFunc CALL_BACK = nullptr;

    void SetLogLevel(const LogLevel level) { global::ENGINE_CONFIG.logLevel = level; }

    void LogEx(const LogLevel level, const char* file, const int line, const char* msg, ...)
    {
        if (level < global::ENGINE_CONFIG.logLevel)
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

        if (CALL_BACK)
            CALL_BACK(level, msg);

        // Handle the variable arguments
        va_list args;
        va_start(args, msg);
        vfprintf(out, msg, args);
        va_end(args);

        fputc('\n', out);

        if (level >= LEVEL_ERROR) [[unlikely]]
        {
#ifdef MAGIQUE_DEBUG
#if defined(_MSC_VER)
            __debugbreak();
#elif defined(__GNUC__)
            __builtin_trap();
#else
            std::abort();
#endif
#endif
            if (level == LEVEL_FATAL) [[unlikely]]
                std::exit(EXIT_FAILURE);
        }
    }

    void SetLogCallback(const LogCallbackFunc func) { CALL_BACK = func; }

} // namespace magique