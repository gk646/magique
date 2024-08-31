#ifndef MAGIQUE_XML_UTIL_H
#define MAGIQUE_XML_UTIL_H

#include <cassert>
#include <cxutil/cxstring.h>

template <typename T>
T XMLGetValueInLine(const char* line, const char* name, T orElse)
{
    const int nameLen = static_cast<int>(std::strlen(name));
    int i = 0;
    while (line[i] != '\n' && line[i] != '\0')
    {
        if (line[i] == name[0] && std::strncmp(&line[i], name, nameLen) == 0) [[unlikely]]
        {
            i += nameLen;
            if (line[i] == '=' && line[i + 1] == '"')
            {
                i += +2; // Skip name + ="
                if constexpr (std::is_integral_v<T>)
                {
                    auto parsedValue = cxstructs::str_parse_int(&line[i]);
                    assert(parsedValue <= std::numeric_limits<T>::max() &&
                           "Parsed value exceeds the maximum value of the target type");
                    return static_cast<T>(parsedValue);
                }
                else if constexpr (std::is_floating_point_v<T>)
                {
                    auto parsedValue = cxstructs::str_parse_float(&line[i]);
                    assert(parsedValue <= std::numeric_limits<T>::max() &&
                           "Parsed value exceeds the maximum value of the target type");
                    return static_cast<T>(parsedValue);
                }
                else if constexpr (std::is_same_v<T, const char*>)
                {
                    return &line[i];
                }
                else
                {
                    static_assert(std::is_same_v<T, float>, "Unsupported type");
                }
            }
        }
        ++i;
    }
    return orElse;
}

inline bool XMLLineContainsTag(const char* line, const char* tag)
{
    const auto tagLen = static_cast<int>(strlen(tag));
    int i = 0;
    while (line[i] != '\n' && line[i] != '\0')
    {
        if (line[i] == '<' && strncmp(&line[i + 1], tag, tagLen) == 0)
        {
            const auto endSymbol = line[i + tagLen + 1];
            if (endSymbol == ' ' || endSymbol == '>' || endSymbol == '\n' || endSymbol == '\r')
            {
                return true;
            }
        }
        ++i;
    }
    return false;
}

inline bool XMLLineContainsCloseTag(const char* line)
{
    int i = 0;
    while (line[i] != '\n' && line[i] != '\0')
    {
        if (strncmp(&line[i], "/>", 2) == 0)
        {
            return true;
        }
        i++;
    }
    return false;
}


#endif //MAGIQUE_XML_UTIL_H