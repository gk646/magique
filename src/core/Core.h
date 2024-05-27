#ifndef CORE_H
#define CORE_H

// STL
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <new>

// Replce with cxallocator
#include "Allocator.h"

#include <cxutil/cxassert.h>


namespace magique
{
    // Keep internal types flexible
    template <typename T>
    using vector = std::vector<T>;
    template <typename K, typename V>
    using HashMap = std::unordered_map<K, V>;

} // namespace magique
#endif // CORE_H