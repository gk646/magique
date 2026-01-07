#ifndef MAGEQUEST_CRASHLOG_H
#define MAGEQUEST_CRASHLOG_H

#include <string>

namespace magique
{
    const char* GetCrashLogFilename();

    void WriteCrashData(const std::string& crashData);

    std::string GetSystemInfoString();

    std::string GetStackTrace();

    void RegisterCrashLoggers();

} // namespace magique


#endif //MAGEQUEST_CRASHLOG_H
