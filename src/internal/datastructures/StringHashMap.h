#ifndef STRINGHASHMAP_H
#define STRINGHASHMAP_H

#include <string>
#include <cxutil/cxstring.h>
#include "internal/datastructures/HashTypes.h"

namespace magique
{
    template <typename Value>
    using StringHashMap = HashMapEx<std::string, Value, cxstructs::StringCharHash, cxstructs::StringCharEquals>;
}

#endif //STRINGHASHMAP_H