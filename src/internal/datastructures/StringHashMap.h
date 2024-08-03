#ifndef STRINGHASHMAP_H
#define STRINGHASHMAP_H

#include <cxutil/cxstring.h>
#include "internal/datastructures/HashTypes.h"

using namespace cxstructs;

namespace magique
{
    template <typename Value>
    using StringHashMap = HashMapEx<std::string, Value, StringCharHash, StringCharEquals>;
}

#endif //STRINGHASHMAP_H