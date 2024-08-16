#ifndef XMLUTIL_H
#define XMLUTIL_H

#include <cxutil/cxstring.h>

inline bool FindXMLValueInLine(const char* line, const char* name, float& val)
{
    int i = 0;
    while (line[i] != '\n')
    {
        if (line[i] == name[0] && strcmp(&line[i], name) == 0)
        {
            if (line[i] == '=')
            {
                ++i;
                if (line[i] == '"')
                {
                    ++i;
                    val = cxstructs::str_parse_float(&line[i]);
                    return true;
                }
            }
        }
        ++i;
    }

    return false;
}

#endif //XMLUTIL_H