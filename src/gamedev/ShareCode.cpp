#include <magique/gamedev/ShareCode.h>

#include "internal/utils/STLUtil.h"

// Format
// Version Header - 1 byte
// Format hash - to check that imported string has same format as import struct - 2 byte
// Data

// 1. Clean, easy-to-use software interface
// 2. Space efficiency
// 3. Fast

constexpr static uint8_t VERSION = 1;

namespace magique
{
    ShareCodeFormat::~ShareCodeFormat()
    {
        for (const auto& p : properties)
        {
            free(p.name);
        }
    }

    void ShareCodeFormat::addGenericProperty(const int bits, const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = bits;
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodeBlockType::GENERIC;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addIntegerProperty(const uint64_t amount, const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = internal::getBitSizeIntegral(amount);
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodeBlockType::INTEGER;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addFloatProperty(double amount, const char* name)
    {
        internal::ShareCodeProperty prop;
        if (amount > FLT_MAX)
        {
            prop.bits = 64;
        }
        else
        {
            prop.bits = 32;
        }
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodeBlockType::FLOATING;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addTextProperty(const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = -1;
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodeBlockType::STRING;
        properties.push_back(prop);
    }

    void ShareCodeFormat::removeProperty(const char* name)
    {
        const auto pred = [](const internal::ShareCodeProperty& p, const char* name)
        { return p.name != nullptr && strcmp(p.name, name) == 0; };
        UnorderedDelete(properties, name, pred);
    }

    void ShareCodeFormat::removeProperty(const int index)
    {
       properties.erase(properties.begin() + index);
    }

    ShareCodeData ShareCodeFormat::getFormatData() const
    {
        ShareCodeData data;
        for (const auto& p : properties)
        {
            internal::ShareCodePropertyData pData;
            pData.name = p.name;
            pData.bits = p.bits;
            pData.type = p.type;
            data.properties.push_back(pData);
        }
        return data;
    }

    std::string ShareCodeFormat::getShareCodeString(const ShareCodeData& data) { return {}; }

    ShareCodeData ShareCodeFormat::importFromString(const std::string& shareCode) { return {}; }

} // namespace magique