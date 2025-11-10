#ifndef MAGEQUEST_LOGGINGDATA_H
#define MAGEQUEST_LOGGINGDATA_H

namespace magique
{
    void RegisterCrashLoggers();

    struct LogData final
    {
        static constexpr int cacheSize = 255;
        LogCallbackFunc callback = nullptr;
        FILE* file = nullptr;
#ifdef MAGIQUE_DEBUG
        bool crashLog = false;
        bool logToFile = false;
#else
        bool crashLog = true;
        bool logToFile = true;
#endif

        void init()
        {
            if (crashLog)
            {
                RegisterCrashLoggers();
            }

            if (logToFile)
            {
                file = fopen(TextFormat("./%s.log", GetGame().getName()), "wb");
                if (file == nullptr)
                {
                    LOG_ERROR("Failed to open log file");
                }
            }
        }

        ~LogData()
        {
            if (file != nullptr)
            {
                fclose(file);
            }
        }
    };

    namespace global
    {
        inline LogData LOG_DATA{};
    }

} // namespace magique


#endif //MAGEQUEST_LOGGINGDATA_H
