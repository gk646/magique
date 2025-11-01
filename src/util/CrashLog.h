#ifndef MAGEQUEST_CRASHLOG_H
#define MAGEQUEST_CRASHLOG_H

#include <chrono>
#include <fstream>
#include <stacktrace>
#include <cxxabi.h>

// Platform-specific headers
#ifdef __linux__
#include <csignal>
#include <execinfo.h>
#include <sys/utsname.h>
#elif __APPLE__
#include <csignal>
#include <execinfo.h>
#include <sys/utsname.h>
#elif _WIN32
#include <windows.h>
#include <dbghelp.h>
#endif

#define M_CALL_STACK_SIZE 128

inline const char* GetCrashLogFilename()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    const std::tm tm = *std::localtime(&now_time);
    const auto* game = magique::GetGame().getName();
    const auto* date = TextFormat("%02d.%02d.%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    const auto* time = TextFormat("%02d:%02d", tm.tm_hour, tm.tm_min);
    return TextFormat("./crash-log-%s-%s-%s.txt", game, date, time);
}

inline void WriteCrashData(const std::string& crashData)
{
    const auto* fileName = GetCrashLogFilename();
    FILE* crashFile = fopen(fileName, "wb+");
    if (crashFile != nullptr)
    {
        fputs(crashData.c_str(), crashFile);
        fclose(crashFile);
    }
    else
    {
        fprintf(stderr, "Failed to open file: %s\n", fileName);
    }
}

inline std::string GetSystemInfoString()
{
    std::string systemInfo;

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    systemInfo += "Current Time: " + std::string(std::ctime(&now_time));

#ifdef __linux__
    systemInfo += "\nRAM Information:\n";
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open())
    {
        std::string line;
        while (getline(meminfo, line))
        {
            if (line.find("MemTotal:") != std::string::npos || line.find("MemFree:") != std::string::npos ||
                line.find("MemAvailable:") != std::string::npos)
            {
                systemInfo += "  " + line + "\n";
            }
        }
        meminfo.close();
    }

    systemInfo += "\nCPU Information:\n";
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open())
    {
        std::string line;
        std::string model;
        std::string cores;
        std::string MHz;
        std::string fpu;
        while (getline(cpuinfo, line))
        {
            if (line.find("model name") != std::string::npos)
            {
                model = line.substr(line.find(':') + 2);
            }
            else if (line.find("cpu cores") != std::string::npos)
            {
                cores = line.substr(line.find(':') + 2);
            }
            else if (line.find("MHz") != std::string::npos)
            {
                MHz = line.substr(line.find(':') + 2);
            }
            else if (line.find("fpu") != std::string::npos)
            {
                fpu = line.substr(line.find(':') + 2);
                break;
            }
        }
        cpuinfo.close();
        systemInfo += "  Model: " + model + "\n";
        systemInfo += "  Cores: " + cores + "\n";
        systemInfo += "  Speed: " + MHz + " MHz\n";
        systemInfo += "  FPU: " + fpu + "\n";
    }

    systemInfo += "\nGPU Information:\n";
    FILE* lspci = popen("lspci | grep -i vga", "r");
    if (lspci != nullptr)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), lspci) != nullptr)
        {
            systemInfo += "  " + std::string(buffer);
        }
        pclose(lspci);
    }
    else
    {
        systemInfo += "  Not available\n";
    }
    systemInfo += "\nPlatform Information:\n";
    std::ifstream osRelease("/etc/os-release");
    if (osRelease.is_open())
    {
        std::string line;
        while (getline(osRelease, line))
        {
            if (line.find("PRETTY_NAME=") != std::string::npos)
            {
                std::string prettyName = line.substr(line.find("\"") + 1);
                prettyName = prettyName.substr(0, prettyName.find("\""));
                systemInfo += "  OS: " + prettyName + "\n";
            }
        }
        osRelease.close();
    }

#elif __APPLE__
    // Get RAM information
    systemInfo += "\nRAM Information:\n";
    // Placeholder for Mac RAM info
    systemInfo += "  Not implemented for Mac\n";

    // Get CPU information
    systemInfo += "\nCPU Information:\n";
    // Placeholder for Mac CPU info
    systemInfo += "  Not implemented for Mac\n";

    // Get GPU information
    systemInfo += "\nGPU Information:\n";
    // Placeholder for Mac GPU info
    systemInfo += "  Not available\n";

    // Get Platform information
    systemInfo += "\nPlatform Information:\n";
    systemInfo += "  OS: Mac OS\n";
    // Placeholder for more platform info
    systemInfo += "  Not implemented for Mac\n";
#elif _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    systemInfo += "\nRAM Information:\n";
    systemInfo += "  Total Physical Memory: " + std::to_string(memInfo.ullTotalPhys / (1024 * 1024)) + " MB\n";
    systemInfo += "  Available Physical Memory: " + std::to_string(memInfo.ullAvailPhys / (1024 * 1024)) + " MB\n";

    // Get CPU information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    systemInfo += "\nCPU Information:\n";
    systemInfo += "  Number of Processors: " + std::to_string(sysInfo.dwNumberOfProcessors) + "\n";
    systemInfo += "  Processor Type: " + std::to_string(sysInfo.dwProcessorType) + "\n";

    // Get GPU information (simplified)
    systemInfo += "\nGPU Information:\n";
    systemInfo += "  Not available\n";

    systemInfo += "\nPlatform Information:\n";
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    systemInfo +=
        "  OS: Windows " + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion) + "\n";
    systemInfo += "  System Type: " +
        std::string(sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "64-bit" : "32-bit") + "\n";
#endif
    return systemInfo;
}

inline std::string GetStackTrace()
{
    std::string stackTrace;
    stackTrace.reserve(128);

    auto stacktrace = std::stacktrace::current();
    int lineNumber = 0;
    for (const auto& entry : stacktrace)
    {
        stackTrace += TextFormat("%2d", lineNumber++);
        stackTrace += ":";
        const auto file = entry.source_file();
        if (file.empty())
        {
            stackTrace += "Unknown File";
        }
        else
        {
            stackTrace += file;
        }
        stackTrace += ":";
        stackTrace += std::to_string(entry.source_line());
        stackTrace += " - ";
        stackTrace += entry.description();
        stackTrace += "\n";
    }
    return stackTrace;
}

#if defined(__linux__) || defined(__APPLE__)
inline const char* GetSignalName(int signal)
{
    switch (signal)
    {
    case SIGABRT:
        return "SIGABRT";
    case SIGFPE:
        return "SIGFPE";
    case SIGILL:
        return "SIGILL";
    case SIGINT:
        return "SIGINT";
    case SIGSEGV:
        return "SIGSEGV";
    case SIGTERM:
        return "SIGTERM";
    default:
        return TextFormat("%d Unknown signal", signal);
    }
}
inline void crashLogHandler(int signal)
{
    const auto signalName = GetSignalName(signal);
#elif _WIN32
inline LONG WINAPI crashLogHandler(EXCEPTION_POINTERS* exceptionPointers)
{
    const auto signalName = std::to_string(exceptionPointers->ExceptionRecord->ExceptionCode);
#endif
    std::string crashData;
    crashData.reserve(1024);
    crashData += "magique CrashLog File\n";
    crashData += "Exception received: ";
    crashData += signalName;
    crashData += "\n\n";
    crashData += "System Information:\n";
    crashData += GetSystemInfoString();
    crashData += "\n";
    crashData += "Trace Log:\n";
    crashData += GetStackTrace();
    WriteCrashData(crashData);
#if defined(__linux__) || defined(__APPLE__)
    exit(1);
#elif _WIN32
    return EXCEPTION_EXECUTE_HANDLER;
#endif
}

namespace magique
{
    void RegisterCrashLoggers()
    {

#if defined(__linux__) || defined(__APPLE__)
        signal(SIGSEGV, crashLogHandler);
        signal(SIGABRT, crashLogHandler);
#ifndef MAGIQUE_DEBUG
        signal(SIGTRAP, crashLogHandler);
#endif
#elif _WIN32
        SetUnhandledExceptionFilter(crashLogHandler);
#endif
    }

} // namespace magique


#endif //MAGEQUEST_CRASHLOG_H
