#ifndef MAGEQUEST_STL_H
#define MAGEQUEST_STL_H

#include <string_view>

namespace magique
{

    // Uses a static std::string to assign and returns the pointer to it
    // Note: Only valid until this method is called again
    const char* ToCStr(const std::string_view& view);

} // namespace magique


// IMPLEMENTATION


#endif //MAGEQUEST_STL_H
