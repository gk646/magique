#include <cmath>
#include <magique/gamedev/ShareCode.h>

#include "internal/utils/STLUtil.h"

//-------------------------------
// Share Code Format
//-------------------------------
// ................................................................................
// Format:
//      - Version Header: 1 byte
//      - Version Header: 1 byte
// Format hash - to check that imported string has same format as import struct - 2 byte
// Data
// ................................................................................

// 1. Clean, easy-to-use software interface
// 2. Space efficiency
// 3. Fast

constexpr static uint8_t VERSION = 1;

namespace magique
{
    uint8_t GetShareCodeGeneratorVersion() { return VERSION; }

    ShareCode::~ShareCode() { delete[] code; }

    //----------------- FORMAT -----------------//

    ShareCodeFormat::~ShareCodeFormat()
    {
        for (const auto& p : properties)
        {
            free(p.name);
        }
    }

    void ShareCodeFormat::addIntegerProperty(const uint64_t amount, const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = internal::getBitSize(amount);
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodeBlockType::INTEGER;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addFloatProperty(const double amount, const char* name)
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
        prop.bits = 0;
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

    void ShareCodeFormat::removeProperty(const int index) { properties.erase(properties.begin() + index); }

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

    ShareCode ShareCodeFormat::getShareCodeString(const ShareCodeData& data) const
    {
        const int dataSize = static_cast<int>(data.properties.size());
        const int formatSize = static_cast<int>(properties.size());

        if (dataSize != formatSize)
        {
            LOG_WARNING("Given data does not match the format! Use getFormatData() to get the current empty data");
            return {};
        }
        for (int i = 0; i < dataSize; ++i)
        {
            const auto& dp = data.properties[i];
            const auto& fp = properties[i];
            if ((fp.type != internal::ShareCodeBlockType::STRING && dp.bits != fp.bits) || dp.type != fp.type)
            {
                LOG_WARNING("Given data does not match the format! Use getFormatData() to get the current empty data");
                return {};
            }
            if (dp.name == nullptr && fp.name || fp.name == nullptr && dp.name)
            {
                LOG_WARNING("Given data does not match the format! Use getFormatData() to get the current empty data");
                return {};
            }
            if (dp.name && strcmp(dp.name, fp.name) != 0)
            {
                LOG_WARNING("Given data does not match the format! Use getFormatData() to get the current empty data");
                return {};
            }
        }

        // Accumulate size
        int totalDataBits = 0;
        for (const auto& p : data.properties)
        {
            totalDataBits += p.bits;
        }

        if (totalDataBits == 0)
            return {};

        // Add static size
        totalDataBits += 8;  // Version
        totalDataBits += 16; // Checksum

        const int totalBytes = std::ceil(static_cast<float>(totalDataBits) / 8.0F);
        auto* strData = new char[totalBytes];

        // Write statics
        const auto checkSum = getCheckSum();
        strData[0] = static_cast<char>(VERSION);
        std::memcpy(&strData[1], &checkSum, sizeof(uint16_t));

        int currentBits = 25;
        for (const auto& p : data.properties)
        {
            switch (p.type)
            {
            case internal::ShareCodeBlockType::INTEGER:
                {
                    const int64_t value = p.integer;
                    int bitsLeftToWrite = p.bits;
                    while (bitsLeftToWrite > 0)
                    {
                        const int byteIndex = currentBits / 8;
                        const int bitIndexInByte = currentBits % 8;

                        int availableBitsInCurrentByte = 8 - bitIndexInByte;
                        const int bitsToWriteNow = std::min(bitsLeftToWrite, availableBitsInCurrentByte);

                        const auto bitsToWrite =
                            (value >> (bitsLeftToWrite - bitsToWriteNow)) & ((1 << bitsToWriteNow) - 1);

                        strData[byteIndex] &= ~(((1 << bitsToWriteNow) - 1) << bitIndexInByte);

                        strData[byteIndex] |= (bitsToWrite << bitIndexInByte);

                        currentBits += bitsToWriteNow;
                        bitsLeftToWrite -= bitsToWriteNow;
                    }
                }
                break;
            case internal::ShareCodeBlockType::FLOATING:
                break;
            case internal::ShareCodeBlockType::STRING:
                break;
            }
        }

        ShareCode shareCode;
        shareCode.code = strData;
        return shareCode;
    }

    uint16_t ShareCodeFormat::getCheckSum() const
    {
        uint64_t sum = properties.size() * 35'938'1123;
        for (const auto& p : properties)
        {
            sum ^= p.bits ^ static_cast<int>(p.type);

            if (p.name == nullptr)
            {
                sum += 123'456'789;
            }
            else
            {
                const int len = static_cast<int>(strlen(p.name));
                uint64_t name_sum = 0;
                for (int i = 0; i < len; ++i)
                {
                    name_sum += p.name[i] * (i + 1) * 31;
                }
                sum += name_sum;
            }
        }
        const uint32_t lower32 = sum & 0xFFFFFFFF;
        const uint32_t upper32 = sum >> 32;
        const uint32_t folded32 = lower32 ^ upper32;

        const uint16_t lower16 = folded32 & 0xFFFF;
        const uint16_t upper16 = folded32 >> 16;
        return lower16 ^ upper16;
    }

    bool IsVersionValid(const uint8_t version)
    {
        if (version == 1)
        {
            return true;
        }
        return false;
    }

    ShareCodeData ShareCodeFormat::importFromString(const ShareCode& shareCode) const { return {}; }

} // namespace magique