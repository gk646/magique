#include <cstring>
#include <utility> // Needed for std::move() in STLUtil.h

#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>

#include "internal/utils/STLUtil.h"

namespace magique
{
    bool Asset::hasExtension(const char* extension) const
    {
        MAGIQUE_ASSERT(extension != nullptr, "Passing nullptr");
        const auto* ext = strrchr(path, '.');
        if (ext == nullptr || ext == path)
            return false;
        return strcmp(extension, ext) == 0;
    }

    bool Asset::startsWith(const char* prefix) const
    {
        MAGIQUE_ASSERT(prefix != nullptr, "Passing nullptr");
        const auto len = strlen(prefix);
        return strncmp(path, prefix, len) == 0;
    }

    bool Asset::endsWith(const char* suffix) const
    {
        MAGIQUE_ASSERT(suffix != nullptr, "Passing nullptr");
        return false;
    }

    bool Asset::contains(const char* str) const { return false; }

    const char* Asset::getFileName(const bool extension) const
    {
        const char* lastSep = nullptr;

    beginning:
        const char* workPtr = path;
        int len = 0;

        while (*workPtr != 0)
        {
            if (lastSep != nullptr)
            {
                if (!extension && *workPtr == '.')
                    break;
                len++;
            }

            if (*workPtr == '/')
            {
                len = 0;
                lastSep = workPtr + 1;
            }
            workPtr++;
        }

        if (lastSep == nullptr)
        {
            lastSep = path;
            goto beginning;
        }

        memcpy(stringBuffer, lastSep, min(64, len));
        stringBuffer[min(64, len)] = '\0';
        return stringBuffer;
    }

    const char* Asset::getExtension() const
    {
        const char* workPtr = path;
        const char* lastDot = nullptr;

        while (*workPtr != 0)
        {
            if (*workPtr == '.')
            {
                lastDot = workPtr;
            }
            workPtr++;
        }

        if (lastDot == nullptr)
            return nullptr;

        int len = 0;
        workPtr = lastDot;
        while (*workPtr != 0)
        {
            len++;
            workPtr++;
        }

        memcpy(stringBuffer, lastDot, min(64, len));
        stringBuffer[min(64, len)] = '\0';
        return stringBuffer;
    }
} // namespace magique