#ifndef INCLUDEWINDOWS_H
#define INCLUDEWINDOWS_H

#if defined(_WIN32)
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#endif

#endif //INCLUDEWINDOWS_H