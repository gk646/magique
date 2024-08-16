#ifndef XMLUTIL_H
#define XMLUTIL_H

#include <cxutil/cxstring.h>

template <typename T>
T XMLGetValueInLine(const char* line, const char* name)
{
    const int nameLen = static_cast<int>(std::strlen(name));
    int i = 0;

    while (line[i] != '\n' && line[i] != '\0')
    {
        if (line[i] == name[0] && std::strncmp(&line[i], name, nameLen) == 0) [[unlikely]]
        {
            i += nameLen + 2; // Skip name + ="
            if constexpr (std::is_same_v<T, int>)
            {
                return cxstructs::str_parse_int(&line[i]);
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                return cxstructs::str_parse_float(&line[i]);
            }
            else if constexpr (std::is_same_v<T, const char*>)
            {
                return &line[i];
            }
        }
        ++i;
    }

    if constexpr (std::is_same_v<T, int>)
    {
        return INT32_MIN;
    }
    else if constexpr (std::is_same_v<T, float>)
    {
        return FLT_MIN;
    }
    else if constexpr (std::is_same_v<T, const char*>)
    {
        return nullptr;
    }
}

inline bool XMLLineContainsTag(const char* line, const char* tag)
{
    const auto tagLen = static_cast<int>(strlen(tag));
    int i = 0;
    while (line[i] != '\n' && line[i] != '\0')
    {
        if (line[i] == '<' && strncmp(&line[i + 1], tag, tagLen) == 0)
        {
            return true;
        }
        ++i;
    }
    return false;
}

#endif //XMLUTIL_H