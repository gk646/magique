#ifndef MAGIQUE_SHARECODE_H
#define MAGIQUE_SHARECODE_H

#include <string>
#include <vector>
#include <magique/internal/InternalTypes.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

//-------------------------------
// ShareCode Module
//-------------------------------
// ................................................................................
// This module allows to generate a space efficient string representation of configurable data
// It works by creating a format that specifies the different chunks (e.g. size and type) of the sharecode
// Then, by passing data values in the same format allows to generate the space efficient representation
// The data is packed on a bit level avoiding ANY unnecessary padding
// When importing
// Intended Workflow:
//          - 1. Create a format: Add the properties you want and specify the maximum range
//          - 2. Set the data   : Use getFormatData() to get the correct data object and set the property values
//          - 3. Get ShareCode  : Now you can generate the sharecode by passing the data to the format
//          - 4. Import         : Import the sharecode with the format that produced it to get the contained data
// Note: The format has to outlive all the data objects that it produces
// ................................................................................

namespace magique
{
    // Returns the version of the sharecode generator
    uint8_t GetShareCodeGeneratorVersion();

    struct ShareCode final
    {
        // Creates a share code by copying the given string
        // Failure: internal code will be null
        static ShareCode FromString(const char* str);

        //----------------- ACCESS -----------------//

        // Returns the version of the generator this sharecode was made with
        [[nodiscard]] uint8_t getVersion() const;

        // Returns the checksum of this ShareCode
        [[nodiscard]] uint16_t getCheckSum() const;

        // Returns the sharecode copied into a string
        [[nodiscard]] std::string getAsString() const;

        // Returns the sharecode directly without making a copy
        [[nodiscard]] const char* getCode() const;

        // Returns true if the given sharecode is valid
        [[nodiscard]] bool getIsValid() const;

        ~ShareCode();

    private:
        explicit ShareCode(const char* code);
        const char* code = nullptr;
        ShareCode() = default;
        friend struct ShareCodeFormat;
    };

    struct ShareCodeProperty final
    {
        // Returns true if the property has the given type
        [[nodiscard]] bool isInteger() const;
        [[nodiscard]] bool isFloat() const;
        [[nodiscard]] bool isString() const;

        // Returns the value
        // IMPORTANT: program will crash when you call the wrong type getter
        //            -> check if its integer first before calling getInteger()
        [[nodiscard]] int getInteger() const;
        [[nodiscard]] float getFloat() const;
        [[nodiscard]] const char* getString() const;

    private:
        internal::ShareCodePropertyType type{};
        union
        {
            char* string = nullptr;
            float floating;
            int32_t integer;
        };
        friend struct ShareCodeData;
    };

    struct ShareCodeData final
    {
        // Sets the data of the property with the given name
        template <typename T>
        void setData(const char* name, T data);

        // Sets the data of the property at the given index
        template <typename T>
        void setData(int index, T data);

        // Returns the data of the property with the given name
        // Note: The given type has to be larger and fit the underlying type (e.g. int for Integer)
        // Failure: returns null or the minimal value of the type
        [[nodiscard]] ShareCodeProperty getData(const char* name) const;

        // Returns the data of the property at the given index
        // Note: The given type has to be larger and fit the underlying type (e.g. int for Integer)
        // Failure: returns null or the minimal value of the type
        [[nodiscard]] ShareCodeProperty getData(int index) const;

        // Returns the total number of properties
        [[nodiscard]] int getSize() const;

    private:
        template <typename T>
        void setData(internal::ShareCodePropertyData& p, T data);

        std::vector<internal::ShareCodePropertyData> properties;
        friend struct ShareCodeFormat;
    };

    struct ShareCodeFormat final
    {
        ~ShareCodeFormat();

        //----------------- PROPERTIES -----------------//

        // Adds a new property with that stores an integer number - bit size is the minimal size that can hold the given amount
        //      - amount: maximum amount of different numbers the property can hold (e.g. 255, 4000, 65536)
        void addIntegerProperty(uint32_t amount, const char* name = nullptr);

        // Adds a new property with that stores floating point number - always 32 bits
        void addFloatProperty(const char* name = nullptr);

        // Adds a new property that can store text up to the given maximum length - variable length
        void addTextProperty(const char* name = nullptr);

        // Returns true if the property with the given name was successfully removed
        void removeProperty(const char* name);

        // Returns true if the property with at the given index was successfully removed
        void removeProperty(int index);

        //----------------- EXPORT -----------------//

        // Returns an empty data instance of this format
        // Note: You need (should) to set the value of each property in the data in order to export it
        [[nodiscard]] ShareCodeData getFormatData() const;

        // Returns the sharecode that holds the given data
        // Note: The sharecode can ONLY be imported by the format that generated it!
        // Failure: returns invalid sharecode
        [[nodiscard]] ShareCode getShareCode(const ShareCodeData& data) const;

        //----------------- IMPORT -----------------//

        // Returns the checksum of the format - useful to determine which format the sharecode comes from
        // The checksum of a sharecode matches the checksum of the format it was generated by!
        [[nodiscard]] uint16_t getCheckSum() const;

        // Returns the data for this format filled with the parsed values from the given string sharecode
        // Failure: returns empty data
        [[nodiscard]] ShareCodeData importFromString(const ShareCode& shareCode) const;

    private:
        std::vector<internal::ShareCodeProperty> properties;
    };

} // namespace magique

//----------------- IMPLEMENTATION -----------------//

namespace magique
{
    namespace internal
    {
        int GetShareCodeCharacterSize();

        template <typename T>
        int getBitSize(T num)
        {
            if constexpr (std::is_integral_v<T>)
            {
                if constexpr (!std::is_unsigned_v<T>)
                {
                    num = std::abs(num);
                }
                if (num == 0)
                    return 1;
                int bits = 0;
                while (num > 0)
                {
                    num >>= 1;
                    bits++;
                }
                return bits;
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                return sizeof(T) * 8;
            }
            else if constexpr (std::is_same_v<T, const char*>)
            {
                // +1 for the terminator
                return (strlen(num) + 1) * GetShareCodeCharacterSize();
            }
            else
            {
                static_assert(std::is_same_v<T, char*>, "Wrong type");
                return -1;
            }
        }
    } // namespace internal

    template <typename T>
    void ShareCodeData::setData(const char* name, T data)
    {
        MAGIQUE_ASSERT(name != nullptr, "passed nullptr");
        for (auto& p : properties)
        {
            if (p.name != nullptr && strcmp(p.name, name) == 0)
            {
                setData(p, data);
                return;
            }
        }
        LOG_WARNING("Property not found: %s", name);
    }
    template <typename T>
    void ShareCodeData::setData(const int index, T data)
    {
        if (index >= properties.size())
        {
            LOG_WARNING("Index out of range: %d", index);
            return;
        }
        setData(properties[index], data);
    }
    template <typename T>
    void ShareCodeData::setData(internal::ShareCodePropertyData& p, T data)
    {
        const int bits = internal::getBitSize(data);
        const char* fmt = "Trying to assign data that is bigger than specified: %s | Maximum/Actual: %d/%d";
        if constexpr (std::is_integral_v<T>)
        {
            M_SHARECODE_CHECKTYPE(internal::ShareCodePropertyType::INTEGER, "Property is not of integer type: %s")
            if (bits > p.bits)
            {
                LOG_WARNING(fmt, p.name, p.bits, bits);
                return;
            }
            p.integer = data;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            M_SHARECODE_CHECKTYPE(internal::ShareCodePropertyType::FLOATING, "Property is not of floating type: %s")
            if (bits > p.bits)
            {
                LOG_WARNING(fmt, p.name, p.bits, bits);
                return;
            }
            p.floating = data;
        }
        else if constexpr (std::is_same_v<const char*, T>)
        {
            M_SHARECODE_CHECKTYPE(internal::ShareCodePropertyType::STRING, "Property is not of string type: %s")
            free(p.string);
            p.bits = bits;
            p.string = data == nullptr ? nullptr : strdup(data);
        }
    }
} // namespace magique


#endif //MAGIQUE_SHARECODE_H