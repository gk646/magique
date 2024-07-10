#ifndef MAGIQUE_OSUTIL_H
#define MAGIQUE_OSUTIL_H

inline void WaitTime(const double destinationTime, double sleepSeconds)
{
    if (sleepSeconds < 0)
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
    while (glfwGetTime() < destinationTime)
    {
#if defined(_WIN32)
        YieldProcessor();
#endif
    }
}


#endif //MAGIQUE_OSUTIL_H