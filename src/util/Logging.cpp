// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <magique/util/Logging.h>
#include <magique/gamedev/Console.h>
#include <magique/config.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/ConsoleData.h"

namespace magique
{
    struct LogData final
    {
        static constexpr int cacheSize = 255;
        static constexpr const char* fileName = "magique.log";
        FILE* file = nullptr;
        LogCallbackFunc callback = nullptr;

        LogData()
        {
#if MAGIQUE_LOG_FILE == 1
            file = fopen(fileName, "wb");
            if (!file)
            {
                LOG_ERROR("Failed to open log file");
            }
#endif
        }
        ~LogData()
        {
            if (file)
                fclose(file);
        }
    };

    static LogData LOG_DATA{};

    namespace internal
    {
        void LogInternal(const LogLevel level, const char* file, const int line, const char* msg, va_list args)
        {
            if (level < global::ENGINE_CONFIG.logLevel)
            {
                return;
            }

            const auto& theme = global::ENGINE_CONFIG.theme;
            Color consoleColor = theme.textPassive;

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
            }

            FILE* out = level >= LEVEL_ERROR ? stderr : stdout;
            constexpr int cacheSize = LOG_DATA.cacheSize;
            char FMT_CACHE[cacheSize]{};

            int written = 0;
            if (level >= LEVEL_ERROR)
            {
                written = snprintf(FMT_CACHE, cacheSize, "[%s]: %s:%d ", level_str, file, line);
            }
            else
            {
                written = snprintf(FMT_CACHE, cacheSize, "[%s]: ", level_str);
            }

            MAGIQUE_ASSERT(written >= 0, "Failed to format");
            written += vsnprintf(FMT_CACHE + written, cacheSize - written, msg, args);

            if (LOG_DATA.callback != nullptr)
            {
                LOG_DATA.callback(level, FMT_CACHE);
            }

            // Write to stdout
            fputs(FMT_CACHE, out);
            fputc('\n', out);

            // Write to console
            global::CONSOLE_DATA.addString(FMT_CACHE, consoleColor);

            // Write to log file
            if (LOG_DATA.file)
            {
                fwrite(FMT_CACHE, written, 1, LOG_DATA.file);
                fputc('\n', LOG_DATA.file);
            }

            if (level >= LEVEL_ERROR) [[unlikely]]
            {
#ifdef MAGIQUE_DEBUG
                fclose(LOG_DATA.file);
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
                    fclose(LOG_DATA.file);
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

    void SetLogCallback(const LogCallbackFunc func) { LOG_DATA.callback = func; }

} // namespace magique