#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <magique/fwd.hpp>
#include <ankerl/unordered_dense.h>
#include <ankerl/emhash7.h>
#include <ankerl/emhash4.h>

//-----------------------------------------------
// Datastructures Modules
//-----------------------------------------------

// ................................................................................
// These are the internal datastructures used by the engine
// ................................................................................

namespace magique
{
    template <typename T>
    using vector = std::vector<T>;

    template <typename K, typename V>
    using HashMap = emhash7::HashMap<K, V>;

    template <typename K, typename V, typename Hash, typename Equals>
    using HashMapEx = ankerl::unordered_dense::map<K, V, Hash, Equals>;

    template <typename K, typename Hash = std::hash<K>>
    using HashSet = emhash9::HashSet<K, Hash>;

} // namespace magique


#endif //DATASTRUCTURES_H