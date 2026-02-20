// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>
#include <utility>
#include <algorithm>

#include <magique/assets/types/Asset.h>


namespace magique
{
    const char* Asset::getPath() const { return path; }

    const char* Asset::getData() const { return data; }

    const unsigned char* Asset::getUData() const { return (const unsigned char*)data; }

    int Asset::getSize() const { return size; }

    bool Asset::hasExtension(const char* extension) const
    {
        MAGIQUE_ASSERT(extension != nullptr, "Passing nullptr");
        if (extension == nullptr)
            return false;
        const auto* ext = strrchr(path, '.');
        if (ext == nullptr || ext == path)
            return false;
        return strcmp(extension, ext) == 0;
    }

    bool Asset::startsWith(const char* prefix) const
    {
        MAGIQUE_ASSERT(prefix != nullptr, "Passing nullptr");
        if (prefix == nullptr)
            return false;
        const auto len = strlen(prefix);
        return strncmp(path, prefix, len) == 0;
    }

    bool Asset::endsWith(const char* suffix) const
    {
        MAGIQUE_ASSERT(suffix != nullptr, "Passing nullptr");
        if (suffix == nullptr)
            return false;
        const size_t pathLen = strlen(path);
        const size_t suffixLen = strlen(suffix);
        if (suffixLen > pathLen)
        {
            return false;
        }
        return strcmp(path + pathLen - suffixLen, suffix) == 0;
    }

    bool Asset::contains(const char* str) const
    {
        if (str == nullptr)
        {
            return false;
        }
        const char* found = strstr(path, str);
        return found != nullptr;
    }

    bool Asset::isValid() const { return data != nullptr && path != nullptr; }

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

        memcpy(stringBuffer, lastSep, std::min(64, len));
        stringBuffer[std::min(64, len)] = '\0';
        return stringBuffer;
    }

    const char* Asset::getExtension() const
    {
        MAGIQUE_ASSERT(path != nullptr, "No path");
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

        memcpy(stringBuffer, lastDot, std::min(64, len));
        stringBuffer[std::min(64, len)] = '\0';
        return stringBuffer;
    }
} // namespace magique
