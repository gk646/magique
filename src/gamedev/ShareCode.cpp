// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <string>

#include <magique/gamedev/ShareCode.h>
#include <magique/util/Strings.h>

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

constexpr static uint8_t VERSION = 1;
constexpr static uint16_t CHECKSUM_TYPE = 0;
constexpr static char STRING_LIMITER = 65;
constexpr static int BYTE_TO_BITS = 8;
constexpr static int CHARACTER_SIZE = 8;

namespace magique
{
    namespace internal
    {
        int GetShareCodeCharacterSize() { return CHARACTER_SIZE; }

    } // namespace internal

    uint8_t GetShareCodeGeneratorVersion() { return VERSION; }

    bool IsShareCodeVersionSupported(const uint8_t version)
    {
        if (version == 1)
        {
            return true;
        }
        return false;
    }

    //----------------- SHARE CODE -----------------//

    ShareCode::ShareCode(const char* str) // public version
    {
        if (str == nullptr)
        {
            LOG_WARNING("Passed null to ShareCode::FromString");
            return;
        }

        const auto len = static_cast<int>(strlen(str));
        if (len < (int)sizeof(CHECKSUM_TYPE) +  (int)sizeof(VERSION))
        {
            LOG_WARNING("Passed string is not a ShareCode: %s", str);
            return;
        }

        // Decode only the header
        char header[5];
        std::memcpy(header, str, sizeof(VERSION) + sizeof(CHECKSUM_TYPE));
        header[3] = '=';
        header[4] = '\0';
        DecodeBase64(header);

        ShareCode test{header, nullptr};
        const auto version = test.getVersion();
        if (!IsShareCodeVersionSupported(version))
        {
            LOG_WARNING("Passed ShareCode has unsupported version: %d", static_cast<int>(version));
            return;
        }

        // Tests successful - return new sharecode with copied data
        test.binaryCode = nullptr; // Avoid deletion

        // Copy base64 input string
        const auto base64Str = new char[len + 1];
        std::memcpy(base64Str, str, len);
        base64Str[len] = '\0';

        // Get binary length and copy it
        DecodeBase64(base64Str);
        int binaryLength = 0;
        for (int i = len - 1; i > -1; --i)
        {
            if (base64Str[i] == '\0')
            {
                binaryLength = i;
                break;
            }
        }

        // Copy binary part
        const auto binary = new char[binaryLength + 1];
        std::memcpy(binary, base64Str, binaryLength);
        binary[binaryLength] = '\0';

        // Encode base64 back
        EncodeBase64(base64Str, binaryLength, base64Str, len + 1);
        base64 = base64Str;
        binaryCode = binary;
    }

    uint8_t ShareCode::getVersion() const
    {
        uint8_t version = 0;
        std::memcpy(&version, &binaryCode[0], sizeof(uint8_t));
        return version;
    }

    uint16_t ShareCode::getCheckSum() const
    {
        uint16_t checkSum = 0;
        std::memcpy(&checkSum, &binaryCode[1], sizeof(uint16_t));
        return checkSum;
    }

    const char* ShareCode::getCode() const { return base64; }

    bool ShareCode::getIsValid() const { return binaryCode != nullptr && base64 != nullptr; }

    bool ShareCode::operator==(const ShareCode& o) const
    {
        const bool binaryEqual =
            (binaryCode == o.binaryCode) || (binaryCode && o.binaryCode && std::strcmp(binaryCode, o.binaryCode) == 0);
        const bool base64Equal = (base64 == o.base64) || (base64 && o.base64 && std::strcmp(base64, o.base64) == 0);
        return binaryEqual && base64Equal;
    }

    ShareCode::~ShareCode()
    {
        delete[] binaryCode; // Compiler warning is not correct - pointer is replaced before deletion
        binaryCode = nullptr;
        delete[] base64;
        base64 = nullptr;
    }

    ShareCode::ShareCode(const char* code, const char* base64) :
        binaryCode(code), base64(base64) {} // Internal version - no copy

    //----------------- PROPERTY DATA -----------------//

    int ShareCodeProperty::getInt() const
    {
        MAGIQUE_ASSERT(type == internal::ShareCodePropertyType::INTEGER, "Property is not an integer");
        return integer;
    }

    float ShareCodeProperty::getFloat() const
    {
        MAGIQUE_ASSERT(type == internal::ShareCodePropertyType::FLOATING, "Property is not an float");
        return floating;
    }

    const char* ShareCodeProperty::getString() const
    {
        MAGIQUE_ASSERT(type == internal::ShareCodePropertyType::STRING, "Property is not an string");
        return string;
    }

    bool ShareCodeProperty::getIsInt() const { return type == internal::ShareCodePropertyType::INTEGER; }

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
        if (index < 0 || index >= static_cast<int>(properties.size()))
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

    ShareCodeData& ShareCodeData::operator=(ShareCodeData&& other) noexcept
    {
        if (this != &other)
        {
            properties = std::move(other.properties);
        }
        return *this;
    }

    ShareCodeData::ShareCodeData(ShareCodeData&& other) noexcept : properties(std::move(other.properties)) {}

    ShareCodeData::~ShareCodeData()
    {
        for (auto& p : properties)
        {
            if (p.type == internal::ShareCodePropertyType::STRING)
            {
                delete[] p.string;
                p.string = nullptr;
            }
        }
    }

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
        prop.name = name == nullptr ? nullptr : strdup(name);
        prop.type = internal::ShareCodePropertyType::INTEGER;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addFloatProperty(const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = 32; // Fixed with
        prop.name = name == nullptr ? nullptr : strdup(name);
        prop.type = internal::ShareCodePropertyType::FLOATING;
        properties.push_back(prop);
    }

    void ShareCodeFormat::addTextProperty(const char* name)
    {
        internal::ShareCodeProperty prop;
        prop.bits = 0;
        prop.name = name == nullptr ? nullptr : strdup(name);
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
        data.properties.reserve(properties.size() + 1);
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
            if ((dp.name == nullptr && fp.name != nullptr) || (fp.name == nullptr && dp.name != nullptr))
            {
                LOG_WARNING("Given data does not match the format! Use getFormatData() to get the current empty data");
                return {};
            }
            if (dp.name != nullptr && strcmp(dp.name, fp.name) != 0)
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

        // Allocate the base64 length already
        const int binaryBytes = static_cast<int>(std::ceil(static_cast<float>(totalDataBits) / BYTE_TO_BITS));
        const int base64Bytes = GetBase64EncodedLength(binaryBytes);
        auto* binary = new char[binaryBytes + 1];
        auto* base64 = new char[base64Bytes + 1];
        binary[binaryBytes] = '\0';
        base64[base64Bytes] = '\0';

        // Write header
        const auto checkSum = getCheckSum();
        binary[0] = static_cast<char>(VERSION);
        std::memcpy(&binary[1], &checkSum, sizeof(uint16_t));

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
                    auto& currByte = binary[byteIndex];
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
                        auto& currByte = binary[byteIndex];
                        const auto written = WriteBits(value, bitsLeftToWrite, currByte, bitIndexInByte);
                        currentBits += written;
                        bitsLeftToWrite -= written;
                        value >>= written;
                    }
                }
            }
        }

        // Copy binary to base64 and encode
        std::memcpy(base64, binary, binaryBytes);
        EncodeBase64(base64, binaryBytes, base64, base64Bytes + 1);
        return ShareCode{binary, base64};
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

    ShareCodeData ShareCodeFormat::getShareCodeData(const ShareCode& shareCode) const
    {
        // Checks
        MAGIQUE_ASSERT(shareCode.getIsValid(), "passed nullptr");
        const auto version = shareCode.getVersion();
        if (!IsShareCodeVersionSupported(version))
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

        const auto strData = shareCode.binaryCode;
        ShareCodeData data = getFormatData();
        int currentBit = 24;
        for (int i = 0; i < static_cast<int>(properties.size()); ++i)
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
                    uint32_t value = 0;
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
                    str.push_back(static_cast<char>(value));
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