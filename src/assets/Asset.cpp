// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/assets/types/Asset.h>

namespace magique
{
    Asset::operator const char*() const { return data.data(); }

    Asset::operator const unsigned char*() const { return (const unsigned char*)data.data(); }

    Asset::operator std::string_view() const { return data; }

    std::string_view Asset::getData() const { return data.data(); }

    std::string_view Asset::getPath() const { return path.data(); }

    int Asset::getSize() const { return data.size(); }

    bool Asset::startsWith(std::string_view prefix) const { return path.starts_with(prefix); }

    bool Asset::endsWith(std::string_view suffix) const { return path.ends_with(suffix); }

    bool Asset::contains(std::string_view str) const { return path.contains(str); }

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
