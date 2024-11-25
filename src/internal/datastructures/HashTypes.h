// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_HASHMAPTYPE_H
#define MAGIQUE_HASHMAPTYPE_H

// Made for minimal compile time and binary bloat
// And to switch implementation easily

#include <ankerl/unordered_dense.h>

namespace magique
{
    template <typename K, typename V>
    using HashMap = ankerl::unordered_dense::map<K, V>;

    template <typename K, typename V, typename Hash, typename Equals>
    using HashMapEx = ankerl::unordered_dense::map<K, V, Hash, Equals>;

    template <typename K>
    using HashSet = ankerl::unordered_dense::set<K>;
} // namespace magique

#endif //MAGIQUE_HASHMAPTYPE_H