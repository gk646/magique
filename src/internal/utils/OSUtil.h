#ifndef MAGIQUE_OSUTIL_H
#define MAGIQUE_OSUTIL_H

inline void WaitTime(const double destinationTime, double sleepSeconds)
{
    if (sleepSeconds < 0) [[unlikely]]
        return; // Security check

        // System halt functions
#if defined(_WIN32)
    Sleep((unsigned long)(sleepSeconds * 1000.0));
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
    struct timespec req = {0};
    time_t sec = sleepSeconds;
    long nsec = (sleepSeconds - sec) * 1000000000L;
    req.tv_sec = sec;
    req.tv_nsec = nsec;
    while (nanosleep(&req, &req) == -1)
        continue;
#endif
#if defined(__APPLE__)
    usleep(sleepSeconds * 1000000.0);
#endif
    while (GetTime() < destinationTime)
    {
    }
}

inline void SetupThreadPriority(const int thread, bool high = true)
{
#if defined(WIN32)
    //printf("Setting up: %d\n", GetCurrentThreadId());
    HANDLE hThread = GetCurrentThread();
    SetThreadPriority(hThread, high ? THREAD_PRIORITY_HIGHEST : THREAD_PRIORITY_ABOVE_NORMAL);
    DWORD_PTR affinityMask = static_cast<DWORD_PTR>(1) << thread;
    auto res = SetThreadAffinityMask(hThread, affinityMask);
    //printf("Affinity: %d\n",affinityMask);
    if (res == 0)
    {
        LOG_ERROR("Failed to setup thread affinity for thread: %d", thread);
    }
#endif
}

inline void SetupProcessPriority()
{
#if defined(WIN32)
    HANDLE hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
    return;
    // Generally dont wanna thread pin (at least not like this?)
    // Performance is generally a bit less on average but no lags or hitches in high intensive workload
    DWORD_PTR processAffinityMask = 0xF;
    if (!SetProcessAffinityMask(hProcess, processAffinityMask))
    {
        LOG_ERROR("Failed to setup process priority");
    }
#endif
}

#endif //MAGIQUE_OSUTIL_H