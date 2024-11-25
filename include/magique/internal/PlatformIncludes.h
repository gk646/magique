// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PLATFORM_INCLUDES_H
#define MAGIQUE_PLATFORM_INCLUDES_H

#if defined(__MINGW32__) || defined(__MINGW64__) || defined(__GNUC__) || defined(__clang__) ||                          \
    !defined(MAGIQUE_FORWARD_FUNCTION)
#include <functional>
#endif

#endif //MAGIQUE_PLATFORM_INCLUDES_H