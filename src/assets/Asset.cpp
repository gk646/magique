// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/assets/types/Asset.h>

namespace magique
{
    const char* Asset::getPath() const { return path.data(); }

    const char* Asset::getData() const { return data.data(); }

    const unsigned char* Asset::getUData() const { return (const unsigned char*)data.data(); }

    int Asset::getSize() const { return data.size(); }

    bool Asset::startsWith(const char* prefix) const { return path.starts_with(prefix); }

    bool Asset::endsWith(const char* suffix) const { return path.ends_with(suffix); }

    bool Asset::contains(const char* str) const { return path.contains(str); }

    bool Asset::isValid() const { return !path.empty(); }

    std::string_view Asset::getFileName(const bool extension) const
    {
        const auto lastSlash = path.find_last_of('/');
        auto name = path;
        if (lastSlash != std::string_view::npos)
        {
            name = path.substr(lastSlash + 1);
        }
        const auto lastDot = name.find_last_of('.');
        if (!extension && lastDot != std::string_view::npos)
        {
            return name.substr(0, lastDot);
        }
        return name.data();
    }

    std::string_view Asset::getExtension() const
    {
        const auto lastDot = path.find_last_of('.');
        if (lastDot != std::string_view::npos)
        {
            return path.substr(lastDot);
        }
        else
        {
            return {};
        }
    }
} // namespace magique
