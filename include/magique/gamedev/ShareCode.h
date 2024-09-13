#ifndef MAGIQUE_SHARECODE_H
#define MAGIQUE_SHARECODE_H

#include <vector>
#include <string>
#include <magique/internal/InternalTypes.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

//-------------------------------
// Share Code
//-------------------------------
// ................................................................................
//
// Note: The format has to outlive all the data object that it produces
// ................................................................................

namespace magique
{
    struct ShareCodeData final
    {
        // Sets the data of the property with the given name
        template <typename T>
        void setData(const char* name, T data);

        // Sets the data of the property at the given index
        template <typename T>
        void setData(int index, T data);

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

        // Adds a new property with a maximum bit length of 'bits'
        void addGenericProperty(int bits, const char* name = nullptr);

        // Adds a new property with that stores an integer number
        //      - amount: maximum amount of different numbers the property can hold (e.g. 255, 4000, 65536)
        void addIntegerProperty(uint64_t amount, const char* name = nullptr);

        // Adds a new property with that stores floating point number
        //      - amount: maximum amount of different numbers the property can hold (e.g. 255, 4000, 65536)
        void addFloatProperty(double amount, const char* name = nullptr);

        // Adds a new property that can store text up to the given maximum length
        void addTextProperty(const char* name = nullptr);

        // Returns true if the property with the given name was successfully removed
        void removeProperty(const char* name);

        // Returns true if the property with at the given index was successfully removed
        void removeProperty(int index);

        //----------------- EXPORT -----------------//

        // Returns an empty data instance of this format
        // Note: You need to set the value of each property in the data in order to export it
        [[nodiscard]] ShareCodeData getFormatData() const;

        // Returns the string share code that holds the given data
        // Note: The generated string can ONLY be imported by the same format!l
        std::string getShareCodeString(const ShareCodeData& data);

        //----------------- IMPORT -----------------//

        // Returns the data for this format filled with the parsed values from the share code
        ShareCodeData importFromString(const std::string& shareCode);

    private:
        std::vector<internal::ShareCodeProperty> properties;
    };

} // namespace magique


//----------------- IMPLEMENTATION -----------------//

namespace magique
{
    namespace internal
    {
        template <typename T>
        int getBitSizeIntegral(T num)
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
        const char* fmt = "Size of the data is greater than specified: %s | Maximum/Actual: %d/%d";
        if constexpr (std::is_integral_v<T>)
        {
            M_SHARECODE_CHECKTYPE(internal::ShareCodeBlockType::INTEGER, "Property is not of integer type: %s")
            const int bits = internal::getBitSizeIntegral(data);
            if (bits > p.bits)
            {
                LOG_WARNING(fmt, p.name, p.bits, bits);
                return;
            }
            p.integer = data;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            M_SHARECODE_CHECKTYPE(internal::ShareCodeBlockType::FLOATING, "Property is not of floating type: %s")
            const int bits = internal::getBitSizeIntegral(data);
            if (bits > p.bits)
            {
                LOG_WARNING(fmt, p.name, p.bits, bits);
                return;
            }
            p.floating = data;
        }
        else if constexpr (std::is_same_v<const char*, T>)
        {
            M_SHARECODE_CHECKTYPE(internal::ShareCodeBlockType::STRING, "Property is not of string type: %s")
            free(p.string);
            p.string = data == nullptr ? nullptr : strdup(data);
        }
    }
} // namespace magique


#endif //MAGIQUE_SHARECODE_H