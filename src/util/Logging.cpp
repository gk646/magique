// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <csignal>

#include <magique/util/Logging.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/ConsoleData.h"
#include "internal/globals/LoggingData.h"

namespace magique
{
    namespace internal
    {
        void LogInternal(const LogLevel level, const char* file, const int line, const char* function, const char* msg,
                         va_list args)
        {
            auto& log = global::LOG_DATA;
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
            char formatCache[LogData::CACHE_SIZE]{};

            int written = 0;
            if (level >= LEVEL_ERROR)
            {
                written = snprintf(formatCache, LogData::CACHE_SIZE, "[%s]: %s:%d %s: ", level_str, file, line, function);
            }
            else
            {
                written = snprintf(formatCache, LogData::CACHE_SIZE, "[%s]: ", level_str);
            }

            assert(written >= 0);
            written += vsnprintf(formatCache + written, LogData::CACHE_SIZE - written, msg, args);

            if (log.callback != nullptr)
            {
                log.callback(level, formatCache);
            }

            // Write to stdout
            fputs(formatCache, out);
            fputc('\n', out);

            // Write to console
            global::CONSOLE_DATA.addString(formatCache, consoleColor);

            // Write to log file
            if (log.file != nullptr)
            {
                fwrite(formatCache, written, 1, log.file);
                fputc('\n', log.file);
            }

            if (level >= LEVEL_ERROR) [[unlikely]]
            {
#ifdef MAGIQUE_DEBUG
#if defined(_MSC_VER)
                __debugbreak();
#elif defined(__GNUC__)
                raise(SIGTRAP);
#else
                std::abort();
#endif
#endif
                if (level == LEVEL_FATAL) [[unlikely]]
                {
                    fclose(log.file);
#if defined(__linux__) || defined(__APPLE__)
                    raise(SIGABRT);
#elif _WIN32
                    std::exit(EXIT_FAILURE);
#endif
                }
            }
        }

    } // namespace internal

    void Log(const LogLevel level, const char* msg, ...)
    {
        SpinLockGuard guard{global::LOG_DATA.lock};
        va_list args;
        va_start(args, msg);
        internal::LogInternal(level, "Unknown File", -1, "Unknown Func", msg, args);
        va_end(args);
    }

    void LogEx(const LogLevel level, const char* file, const int line, const char* function, const char* msg, ...)
    {
        SpinLockGuard guard{global::LOG_DATA.lock};
        va_list args;
        va_start(args, msg);
        internal::LogInternal(level, file, line, function, msg, args);
        va_end(args);
    }

    void LoggingSetLevel(const LogLevel level) { global::ENGINE_CONFIG.logLevel = level; }

    void LoggingSetCallback(const LogCallbackFunc func) { global::LOG_DATA.callback = func; }

    void LoggingEnableFile(bool value) { global::LOG_DATA.logToFile = value; }

    void LoggingEnableCrashLog(bool value) { global::LOG_DATA.crashLog = value; }

} // namespace magique
