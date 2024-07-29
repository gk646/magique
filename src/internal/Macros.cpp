#include <magique/internal/Macros.h>

#include <cstdio>

namespace magique
{

    void internal::AssertHandler(const char* expr, const char* file, int line, const char* message)
    {
        fprintf(stderr, "Assert failed: %s\nAt: %s:%d\nMessage: %s\n", expr, file, line, message);
#if defined(_MSC_VER)
        __debugbreak();
#elif defined(__GNUC__)
        __builtin_trap();
#else
        std::abort();
#endif
    }
} // namespace magique