#include <chrono>
#include <fstream>
#include <vector>
#if __has_include(<stacktrace>)
#include <stacktrace>
#endif

#include <magique/core/Game.h>

#include "util/headers/CrashLog.h"
#include "external/glad.h"

// Platform-specific headers
#ifdef __linux__
#include <csignal>
#include <sys/utsname.h>
#elif __APPLE__
#include <csignal>
#include <execinfo.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#elif _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#define M_CALL_STACK_SIZE 32
#define _CRT_SECURE_NO_WARNINGS

namespace magique
{
    const char* GetCrashLogFilename()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        const std::tm tm = *std::localtime(&now_time);
        const auto* game = GameGetInstance().getName();
        std::string date(64, '\0');
        // Cannot use TextFormat as we cant include raylib and windows in the same translation unit...
        snprintf(date.data(), 64, "%02d.%02d.%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        std::string time(64, '\0');
        snprintf(time.data(), 64, "%02d:%02d", tm.tm_hour, tm.tm_min);
        static std::string FILE_NAME(64, '\0');
        snprintf(FILE_NAME.data(), 64, "./crash-log-%s-%s-%s.txt", game, date.c_str(), time.c_str());
        return FILE_NAME.c_str();
    }

    void WriteCrashData(const std::string& crashData)
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
            fprintf(stderr, "Failed to write crashlog file: %s\n", fileName);
        }
    }

    std::string GetSystemInfoString()
    {
        std::string gpuRenderer;
        std::string gpuVersion;
        std::string gpuDriver;

        std::string cpuModel;
        int cpuCores = 0;
        float cpuSpeedMHz = 0.0f;

        uint64_t memTotal = 0;
        uint64_t memAvailable = 0;
        uint64_t memProcess = 0;

        std::string osName;
        std::string osVersion;
        std::string architecture;

        std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        gpuRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        gpuVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        gpuDriver = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

#if defined(_WIN32)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        cpuCores = sysInfo.dwNumberOfProcessors;
        cpuModel = "Not available";

        switch (sysInfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:
            architecture = "x86_64";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            architecture = "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            architecture = "ARM64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            architecture = "x86";
            break;
        default:
            architecture = "Unknown";
        }

        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        memTotal = memInfo.ullTotalPhys;
        memAvailable = memInfo.ullAvailPhys;

        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));
        memProcess = pmc.WorkingSetSize;

        OSVERSIONINFO osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osvi);
        osName = "Windows";
        osVersion = std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);

#elif defined(__linux__)
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (cpuinfo.is_open())
        {
            std::string line;
            while (std::getline(cpuinfo, line))
            {
                if (line.find("model name") != std::string::npos)
                    cpuModel = line.substr(line.find(':') + 2);
                else if (line.find("cpu cores") != std::string::npos)
                    cpuCores = std::stoi(line.substr(line.find(':') + 2));
                else if (line.find("cpu MHz") != std::string::npos)
                    cpuSpeedMHz = std::stof(line.substr(line.find(':') + 2));
            }
            cpuinfo.close();
        }

        std::ifstream meminfo("/proc/meminfo");
        if (meminfo.is_open())
        {
            std::string line;
            while (std::getline(meminfo, line))
            {
                if (line.find("MemTotal:") != std::string::npos)
                    memTotal = static_cast<uint64_t>(std::stof(line.substr(10)) * 1024);
                else if (line.find("MemAvailable:") != std::string::npos)
                    memAvailable = static_cast<uint64_t>(std::stof(line.substr(13)) * 1024);
            }
            meminfo.close();
        }

        std::ifstream statm("/proc/self/statm");
        if (statm.is_open())
        {
            std::string line;
            std::getline(statm, line);
            std::istringstream iss(line);
            std::string token;
            std::getline(iss, token, ' ');
            std::getline(iss, token, ' ');
            memProcess = static_cast<uint64_t>(std::stoull(token) * sysconf(_SC_PAGESIZE));
            statm.close();
        }

        std::ifstream osRelease("/etc/os-release");
        if (osRelease.is_open())
        {
            std::string line;
            while (std::getline(osRelease, line))
            {
                if (line.find("PRETTY_NAME=") != std::string::npos)
                {
                    osName = line.substr(line.find('=') + 2);
                    osName = osName.substr(0, osName.find('"'));
                }
            }
            osRelease.close();
        }

        struct utsname sysInfo;
        if (uname(&sysInfo) == 0)
            architecture = sysInfo.machine;
        else
            architecture = "Unknown";

#elif defined(__APPLE__)
        size_t size;
        size = 0;
        sysctlbyname("machdep.cpu.brand_string", nullptr, &size, nullptr, 0);
        if (size > 0)
        {
            std::vector<char> buffer(size);
            sysctlbyname("machdep.cpu.brand_string", buffer.data(), &size, nullptr, 0);
            cpuModel = std::string(buffer.data(), size - 1);
        }
        size = sizeof(cpuCores);
        sysctlbyname("hw.ncpu", &cpuCores, &size, nullptr, 0);

        size = sizeof(memTotal);
        sysctlbyname("hw.memsize", &memTotal, &size, nullptr, 0);
        memAvailable = 0;

        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
        if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &infoCount) ==
            KERN_SUCCESS)
            memProcess = info.resident_size;

        osName = "macOS";
        size = 0;
        sysctlbyname("kern.osproductversion", nullptr, &size, nullptr, 0);
        if (size > 0)
        {
            std::vector<char> buffer(size);
            sysctlbyname("kern.osproductversion", buffer.data(), &size, nullptr, 0);
            osVersion = std::string(buffer.data(), size - 1);
        }

        size = 0;
        sysctlbyname("machdep.cpu.arch", nullptr, &size, nullptr, 0);
        if (size > 0)
        {
            std::vector<char> buffer(size);
            sysctlbyname("machdep.cpu.arch", buffer.data(), &size, nullptr, 0);
            architecture = std::string(buffer.data(), size - 1);
        }
        else
        {
            size = 0;
            sysctlbyname("hw.machine", nullptr, &size, nullptr, 0);
            if (size > 0)
            {
                std::vector<char> buffer(size);
                sysctlbyname("hw.machine", buffer.data(), &size, nullptr, 0);
                architecture = std::string(buffer.data(), size - 1);
            }
            else
            {
                architecture = "Unknown";
            }
        }
#endif

        std::ostringstream oss;
        oss << "Current Time: " << std::ctime(&now_time);

        oss << "\nGPU Information:\n";
        oss << "  Renderer: " << (gpuRenderer.empty() ? "Not available" : gpuRenderer) << "\n";
        oss << "  Version: " << (gpuVersion.empty() ? "Not available" : gpuVersion) << "\n";
        oss << "  Driver: " << (gpuDriver.empty() ? "Not available" : gpuDriver) << "\n";

        oss << "\nCPU Information:\n";
        oss << "  Model: " << (cpuModel.empty() ? "Not available" : cpuModel) << "\n";
        oss << "  Cores: " << cpuCores << "\n";
        oss << "  Speed: " << (cpuSpeedMHz > 0 ? std::to_string(cpuSpeedMHz) + " MHz" : "Not available") << "\n";

        oss << "\nMemory Information:\n";
        oss << "  Total: " << (memTotal > 0 ? std::to_string(memTotal / (1024 * 1024)) + " MB" : "Not available")
            << "\n";
        oss << "  Available: "
            << (memAvailable > 0 ? std::to_string(memAvailable / (1024 * 1024)) + " MB" : "Not available") << "\n";
        oss << "  Process Usage: "
            << (memProcess > 0 ? std::to_string(memProcess / (1024 * 1024)) + " MB" : "Not available") << "\n";

        oss << "\nPlatform Information:\n";
        oss << "  OS: " << (osName.empty() ? "Not available" : osName) << "\n";
        oss << "  Version: " << (osVersion.empty() ? "Not available" : osVersion) << "\n";
        oss << "  Architecture: " << (architecture.empty() ? "Not available" : architecture) << "\n";

        return oss.str();
    }

    std::string GetStackTrace()
    {
        std::string stackTrace;
        stackTrace.reserve(128);

#if __has_include(<stacktrace>)
        auto stacktrace = std::stacktrace::current();
        int lineNumber = 0;
        for (const auto& entry : stacktrace)
        {
            stackTrace += std::to_string(lineNumber++);
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
#else
        stackTrace += "     Not available\n";
#endif
        return stackTrace;
    }

#if defined(__linux__) || defined(__APPLE__)
    inline std::string GetSignalName(int signal)
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
            return "Unknown signal" + std::to_string(signal);
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
        const auto& game = GameGetInstance();
        crashData += "magique CrashLog File\n";
        crashData += "Exception received: " + signalName + "\n\n";
        crashData += "magique: " + std::string{MAGIQUE_VERSION} + "\n";
        crashData += std::string{game.getName()} + ": " + std::string{game.getVersion()} + "\n\n";
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
    } // namespace magique


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
