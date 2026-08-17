#ifndef MAGEQUEST_LOGGINGDATA_H
#define MAGEQUEST_LOGGINGDATA_H

#include "internal/types/SpinLock.h"

namespace magique
{
    void RegisterCrashLoggers();

    struct LogData final
    {
        static constexpr int CACHE_SIZE = 1024;
        LogCallbackFunc callback = nullptr;
        FILE* file = nullptr;
        SpinLock lock{};
        bool crashLog = false;
        bool logToFile = false;

        void init()
        {
            SetTraceLogCallback(
                [](int logLevel, const char* text, va_list args)
                {
                    logLevel = std::max(logLevel - 3, 0);
                    internal::LogInternal(static_cast<LogLevel>(logLevel), "(unknown)", 0, "(unknown)", text, args);
                });

#ifndef MAGIQUE_DEBUG
            LoggingEnableFile();
            LoggingEnableCrashLog();
#endif
        }

        ~LogData()
        {
            if (file != nullptr)
            {
                fclose(file);
                file = nullptr;
            }
        }
    };

    namespace global
    {
        inline LogData LOG_DATA{};
    }

} // namespace magique


#endif // MAGEQUEST_LOGGINGDATA_H
