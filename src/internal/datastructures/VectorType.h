// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_VECTOR_TYPE_H
#define MAGIQUE_VECTOR_TYPE_H

// Made for minimal compile time and less binary bloat
// And to switch implementation easily

#include "internal/datastructures/fast_vector.h"

namespace magique
{
    template <typename T>
    using vector = fast_vector<T>;

    template <typename T>
    struct AlignedVec final
    {
        // To prevent false sharing
        alignas(64) vector<T> vec;
    };

} // namespace magique


#endif //MAGIQUE_VECTOR_TYPE_H