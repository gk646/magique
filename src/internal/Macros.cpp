// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

namespace magique
{
    void internal::AssertHandler(const char* expr, const char* file, const int line, const char* message)
    {
        LOG_FATAL("Assert failed: %s\nAt: %s:%d\nMessage: %s\n", expr, file, line, message);
    }
} // namespace magique