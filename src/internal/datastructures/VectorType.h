#ifndef MAGIQUE_VECTORTYPE_H
#define MAGIQUE_VECTORTYPE_H

// Made for minimal compile time and binary bloat
// And to switch implementation easily

#include "internal/datastructures/fast_vector.h"

namespace magique
{
    template <typename T>
    using vector = fast_vector<T>;

}


#endif //MAGIQUE_VECTORTYPE_H