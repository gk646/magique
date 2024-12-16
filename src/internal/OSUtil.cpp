#include <cstdint>


#include <raylib/raylib.h>
#include <magique/util/Logging.h>

#if defined(_WIN32)
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
#define NONLS             // All NLS defines and routines
#define NOMB              // MB_* and MessageBox()
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMINMAX          // Macros min(a,b) and max(a,b)
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <ctime>
#include <bits/types/struct_timespec.h>
#endif


void WaitTime(const double destinationTime, double sleepSeconds)
{
    if (sleepSeconds < 0) [[unlikely]]
        return; // Security check

    // System halt functions
#if defined(_WIN32)
    Sleep(static_cast<unsigned long>(sleepSeconds * 1000.0));
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
    timespec req;
    req.tv_sec = static_cast<time_t>(sleepSeconds);                                 // Seconds portion
    req.tv_nsec = static_cast<long>((sleepSeconds - (float)(int)req.tv_sec) * 1e9); // Nanoseconds portion
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


void SetupThreadPriority(const int thread, bool high = true)
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


void SetupProcessPriority()
{
#if defined(WIN32)
    HANDLE hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
    return;
    // Generally don't want to thread pin (at least not like this?)
    // Performance is generally a bit less on average but no lags or hitches in high intensive workload
    DWORD_PTR processAffinityMask = 0xF;
    if (!SetProcessAffinityMask(hProcess, processAffinityMask))
    {
        LOG_ERROR("Failed to setup process priority");
    }
#endif
}

uint64_t GetMemoryWorkingSet() { return 0; }