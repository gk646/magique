#include <cmath>
#include <magique/gamedev/ShareCode.h>

#include "internal/utils/STLUtil.h"
#include "internal/utils/BitUtil.h"

//-------------------------------
// Share Code Format
//-------------------------------
// ................................................................................
// Format:
//      - Version Header: 1 byte
//      - Checksum      : 2 bytes
//      - Data          : Variable length
// ................................................................................

// 1. Clean, easy-to-use software interface
// 2. Space efficiency
// 3. Fast

constexpr static uint8_t VERSION = 1;
constexpr static uint16_t CHECKSUM_TYPE = 0;
constexpr static char STRING_LIMITER = 65;
constexpr static int BYTE_TO_BITS = 8;
constexpr static int CHARACTER_SIZE = 8;

bool IsVersionValid(const uint8_t version)
{
    if (version == 1)
    {
        return true;
    }
    return false;
}

namespace magique
{
    namespace internal
    {
        int GetShareCodeCharacterSize() { return CHARACTER_SIZE; }

    } // namespace internal

    uint8_t GetShareCodeGeneratorVersion() { return VERSION; }

    //----------------- SHARE CODE -----------------//

    ShareCode ShareCode::FromString(const char* str) // public version
    {
        if (str == nullptr)
        {
            LOG_WARNING("Passed null to ShareCode::FromString");
            return ShareCode{nullptr};
        }

        const auto len = static_cast<int>(strlen(str));
        if (len < sizeof(CHECKSUM_TYPE) + sizeof(VERSION))
        {
            LOG_WARNING("Passed string is not a ShareCode: %s", str);
            return ShareCode{nullptr};
        }

        ShareCode test{str};
        const auto version = test.getVersion();
        if (!IsVersionValid(version))
        {
            LOG_WARNING("Passed ShareCode has unsupported version: %d", static_cast<int>(version));
            return ShareCode{nullptr};
        }

        // Tests successful - return new sharecode with copied data
        test.code = nullptr; // Avoid deletion

        const auto data = new char[len + 1];
        std::memcpy(data, str, len);
        data[len] = '\0';

        return ShareCode{data};
    }

    uint8_t ShareCode::getVersion() const
    {
        uint8_t version = 0;
        std::memcpy(&version, &code[0], sizeof(uint8_t));
        return version;
    }

    uint16_t ShareCode::getCheckSum() const
    {
        uint16_t checkSum = 0;
        std::memcpy(&checkSum, &code[1], sizeof(uint16_t));
        return checkSum;
    }

    std::string ShareCode::getAsString() const { return {code}; }

    const char* ShareCode::getCode() const { return code; }

    bool ShareCode::getIsValid() const { return code != nullptr; }

    ShareCode::~ShareCode() { delete[] code; }

    ShareCode::ShareCode(const char* code) : code(code) {} // Internal version - no copy

    //----------------- PROPERTY DATA -----------------//

    int ShareCodeProperty::getInteger() const { return integer; }

    float ShareCodeProperty::getFloat() const { return floating; }

    const char* ShareCodeProperty::getString() const { return string; }

    bool ShareCodeProperty::isInteger() const { return type == internal::ShareCodePropertyType::INTEGER; }

    bool ShareCodeProperty::isFloat() const { return type == internal::ShareCodePropertyType::FLOATING; }

    bool ShareCodeProperty::isString() const { return type == internal::ShareCodePropertyType::STRING; }

    //----------------- DATA -----------------//

    ShareCodeProperty ShareCodeData::getData(const char* name) const
    {
        for (const auto& p : properties)
        {
            if (p.name != nullptr && strcmp(p.name, name) == 0)
            {
                ShareCodeProperty property;
                property.type = p.type;
                property.string = p.string;
                return property;
            }
        }
        LOG_WARNING("Property with name does not exist: %s", name);
        return {};
    }

    ShareCodeProperty ShareCodeData::getData(const int index) const
    {
        if (index < 0 || index >= properties.size())
        {
            LOG_WARNING("Property with index does not exist: %d", index);
            return {};
        }
        ShareCodeProperty property;
        property.type = properties[index].type;
        property.string = properties[index].string;
        return property;
    }

    int ShareCodeData::getSize() const { return static_cast<int>(properties.size()); }

    //----------------- FORMAT -----------------//

    ShareCodeFormat::~ShareCodeFormat()
    {
        for (const auto& p : properties)
        {
            free(p.name);
        }
    }

    void ShareCodeFormat::addIntegerProperty(const uint32_t amount, const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = internal::getBitSize(amount);
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodePropertyType::INTEGER;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addFloatProperty(const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = 32; // Fixed with
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodePropertyType::FLOATING;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addTextProperty(const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = 0;
        prop.name = name == nullptr ? nullptr : _strdup(name);
        prop.type = internal::ShareCodePropertyType::STRING;
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

    ShareCode ShareCodeFormat::getShareCode(const ShareCodeData& data) const
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
            if ((fp.type != internal::ShareCodePropertyType::STRING && dp.bits != fp.bits) || dp.type != fp.type)
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
        totalDataBits += sizeof(VERSION) * BYTE_TO_BITS;       // Version
        totalDataBits += sizeof(CHECKSUM_TYPE) * BYTE_TO_BITS; // Checksum

        const int totalBytes = std::ceil(static_cast<float>(totalDataBits) / BYTE_TO_BITS);
        auto* strData = new char[totalBytes + 1];

        // Write statics
        const auto checkSum = getCheckSum();
        strData[0] = static_cast<char>(VERSION);
        std::memcpy(&strData[1], &checkSum, sizeof(uint16_t));

        int currentBits = 24;
        for (const auto& p : data.properties)
        {
            if (p.type < internal::ShareCodePropertyType::STRING)
            {
                uint32_t value = 0;
                if (p.type == internal::ShareCodePropertyType::INTEGER)
                {
                    std::memcpy(&value, &p.integer, sizeof(uint32_t));
                }
                else if (p.type == internal::ShareCodePropertyType::FLOATING)
                {
                    std::memcpy(&value, &p.floating, sizeof(uint32_t));
                }
                else
                {
                    LOG_FATAL("Wrong type");
                }

                int bitsLeftToWrite = p.bits;
                while (bitsLeftToWrite > 0)
                {
                    const int byteIndex = currentBits / BYTE_TO_BITS;
                    const int bitIndexInByte = currentBits % BYTE_TO_BITS;
                    auto& currByte = strData[byteIndex];
                    const auto written = WriteBits(value, bitsLeftToWrite, currByte, bitIndexInByte);
                    currentBits += written;
                    bitsLeftToWrite -= written;
                    value >>= written;
                }
            }
            else if (p.type == internal::ShareCodePropertyType::STRING && p.string != nullptr)
            {
                const int len = static_cast<int>(strlen(p.string));
                const char* strPtr = p.string;
                for (int i = 0; i < len + 1; ++i)
                {
                    unsigned char value = i == len ? STRING_LIMITER : strPtr[i];
                    int bitsLeftToWrite = CHARACTER_SIZE;
                    while (bitsLeftToWrite > 0)
                    {
                        const int byteIndex = currentBits / BYTE_TO_BITS;
                        const int bitIndexInByte = currentBits % BYTE_TO_BITS;
                        auto& currByte = strData[byteIndex];
                        const auto written = WriteBits(value, bitsLeftToWrite, currByte, bitIndexInByte);
                        currentBits += written;
                        bitsLeftToWrite -= written;
                        value >>= written;
                    }
                }
            }
        }
        strData[totalBytes] = '\0';
        return ShareCode{strData};
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

    ShareCodeData ShareCodeFormat::importFromString(const ShareCode& shareCode) const
    {
        MAGIQUE_ASSERT(shareCode.getIsValid(), "passed nullptr");

        const auto version = shareCode.getVersion();
        if (!IsVersionValid(version))
        {
            LOG_WARNING("Passed ShareCode has unsupported version: %d", static_cast<int>(version));
            return {};
        }

        const auto formatChecksum = getCheckSum();
        const auto codeCheckSum = shareCode.getCheckSum();
        if (formatChecksum != codeCheckSum)
        {
            LOG_WARNING("Format of the ShareCode does not match this format! Your using the wrong format for importing");
            return {};
        }

        const auto strData = shareCode.getCode();
        ShareCodeData data = getFormatData();
        int currentBit = 24;
        for (int i = 0; i < properties.size(); ++i)
        {
            const auto& fp = properties[i];
            auto& dp = data.properties[i];
            if (fp.type < internal::ShareCodePropertyType::STRING)
            {
                uint32_t value = 0;
                int bitsLeftToRead = fp.bits;
                while (bitsLeftToRead > 0)
                {
                    uint32_t tempValue = 0;
                    const int byteIndex = currentBit / BYTE_TO_BITS;
                    const int bitIndexInByte = currentBit % BYTE_TO_BITS;
                    const auto currByte = strData[byteIndex];
                    const auto read = ReadBits(tempValue, bitsLeftToRead, currByte, bitIndexInByte);
                    tempValue <<= fp.bits - bitsLeftToRead;
                    currentBit += read;
                    bitsLeftToRead -= read;
                    value |= tempValue;
                }
                if (fp.type == internal::ShareCodePropertyType::INTEGER)
                {
                    std::memcpy(&dp.integer, &value, sizeof(value));
                }
                else if (fp.type == internal::ShareCodePropertyType::FLOATING)
                {
                    std::memcpy(&dp.floating, &value, sizeof(value));
                }
                else
                {
                    LOG_FATAL("Wrong type");
                }
            }
            else if (fp.type == internal::ShareCodePropertyType::STRING)
            {
                std::string str;
                while (true)
                {
                    char value = 0;
                    int bitsLeftToRead = CHARACTER_SIZE;
                    while (bitsLeftToRead > 0)
                    {
                        const int byteIndex = currentBit / BYTE_TO_BITS;
                        const int bitIndexInByte = currentBit % BYTE_TO_BITS;
                        const auto currByte = strData[byteIndex];
                        const auto read = ReadBits(value, bitsLeftToRead, currByte, bitIndexInByte);
                        currentBit += read;
                        bitsLeftToRead -= read;
                        if (bitsLeftToRead > 0)
                        {
                            value <<= read;
                        }
                    }
                    if (value == STRING_LIMITER)
                        break;
                    str.push_back(value);
                }
                const auto stringData = new char[str.size() + 1];
                std::memcpy(stringData, str.c_str(), str.size());
                stringData[str.size()] = '\0';
                dp.string = stringData;
            }
        }

        return data;
    }

} // namespace magique