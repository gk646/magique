// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/util/Logging.h>
#include <magique/gamedev/Console.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/ConsoleData.h"

namespace magique
{
    static LogCallbackFunc CALL_BACK = nullptr;
    static constexpr int FMT_CACHE_SIZE = 255;

    namespace internal
    {
        void LogInternal(const LogLevel level, const char* file, const int line, const char* msg, va_list args)
        {
            if (level < global::ENGINE_CONFIG.logLevel)
            {
                return;
            }

            const auto& theme = global::ENGINE_CONFIG.theme;
            Color consoleColor = theme.txtPassive;

            const auto* level_str = "";
            switch (level)
            {
            case LEVEL_INFO:
                level_str = "INF";
                break;
            case LEVEL_WARNING:
                level_str = "WARN";
                consoleColor = theme.warning;
                break;
            case LEVEL_ERROR:
                level_str = "ERR";
                consoleColor = theme.error;
                break;
            case LEVEL_FATAL:
                level_str = "FATAL";
                consoleColor = theme.error;
                break;
            case LEVEL_ALLOCATION:
                level_str = "ALLOC";
                break;
            }

            FILE* out = level >= LEVEL_ERROR ? stderr : stdout;
            char FORMAT_CACHE[FMT_CACHE_SIZE]{};

            int written = 0;
            if (level >= LEVEL_ERROR)
            {
                written = snprintf(FORMAT_CACHE, FMT_CACHE_SIZE, "[%s]: %s:%d ", level_str, file, line);
            }
            else
            {
                written = snprintf(FORMAT_CACHE, FMT_CACHE_SIZE, "[%s]: ", level_str);
            }

            MAGIQUE_ASSERT(written >= 0, "Failed to format");
            written += vsnprintf(FORMAT_CACHE + written, FMT_CACHE_SIZE - written, msg, args);

            if (CALL_BACK != nullptr)
            {
                CALL_BACK(level, FORMAT_CACHE);
            }

            fputs(FORMAT_CACHE, out);
            fputc('\n', out);

            // Add to console
            global::CONSOLE_DATA.addString(FORMAT_CACHE, consoleColor);

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
                {
                    std::exit(EXIT_FAILURE);
                }
            }
        }

    } // namespace internal

    void SetLogLevel(const LogLevel level) { global::ENGINE_CONFIG.logLevel = level; }

    void Log(const LogLevel level, const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        internal::LogInternal(level, "(null)", -1, msg, args);
        va_end(args);
    }

    void LogEx(const LogLevel level, const char* file, const int line, const char* msg, ...)
    {
        va_list args;
        va_start(args, msg);
        internal::LogInternal(level, file, line, msg, args);
        va_end(args);
    }

    void SetLogCallback(const LogCallbackFunc func) { CALL_BACK = func; }

} // namespace magique