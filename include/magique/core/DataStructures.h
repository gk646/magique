#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H


//-----------------------------------------------
// Datastructures Modules
//-----------------------------------------------

// .....................................................................
// These are the internal datastructures used by the engine
// Include this header with care, its rather heavy!
// .....................................................................

#include "fastvector/fast_vector.h"
#include "tsl/robin_map.h"
#include "tsl/robin_set.h"

namespace magique
{
    template <typename T>
    using vector = fast_vector<T>;

    template <typename K, typename V, typename Hash = std::hash<K>>
    using HashMap = tsl::robin_map<K, V, Hash>;

    template <typename K, typename Hash = std::hash<K>>
    using HashSet = tsl::robin_set<K, Hash>;


} // namespace magique


#endif //DATASTRUCTURES_H