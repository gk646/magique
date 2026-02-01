#include <magique/util/STL.h>
#include <string>

namespace magique
{
    static std::string CACHE{'0', 64};

    const char* to_cstr(const std::string_view& view)
    {
        CACHE = view;
        return CACHE.c_str();
    }

} // namespace magique
