#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <magique/fwd.hpp>
#include <ankerl/unordered_dense.h>


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
    using HashMap = ankerl::unordered_dense::map<K, V>;

    template <typename K, typename V, typename Hash, typename Equals>
    using HashMapEx = ankerl::unordered_dense::map<K, V, Hash, Equals>;

    template <typename K, typename Hash = std::hash<K>>
    using HashSet = ankerl::unordered_dense::set<K, Hash>;

} // namespace magique


#endif //DATASTRUCTURES_H