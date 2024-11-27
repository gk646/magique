// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STRINGHASHMAP_H
#define MAGIQUE_STRINGHASHMAP_H

#include <string>
#include <cxutil/cxstring.h>
#include "internal/datastructures/HashTypes.h"

// Transparent lookup enabled string hashmap

namespace magique
{
    template <typename Value>
    using StringHashMap = HashMapEx<std::string, Value, cxstructs::StringCharHash, cxstructs::StringCharEquals>;
}

#endif //MAGIQUE_STRINGHASHMAP_H