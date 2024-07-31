#include <ranges>
#include <string>
#include <raylib/raylib.h>
#include <cxutil/cxstring.h>

#include <magique/ui/TextFormat.h>
#include <magique/internal/DataStructures.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>
#include <magique/util/Defines.h>

// Note: This implementation uses separate vectors to store values without memory overhead
//       Inserting new values, and overwriting values from the same type is fast.
//       Changing the type of a value is slower as we need to shift indices of all existing values of that type

enum ValueType : uint8_t
{
    FLOAT,
    STRING,
    INT
};

struct ValueInfo final
{
    ValueType type = INT;
    uint8_t index = 0; // Index into the value vector
};

template <typename... ValueTypes>
struct ValueStorage final
{
    std::tuple<std::vector<ValueTypes>...> valueVectors; // A tuple to hold the vectors

    template <typename T, typename Tuple>
    struct Index;

    template <typename T, typename... Types>
    struct Index<T, std::tuple<T, Types...>>
    {
        static constexpr std::size_t value = 0;
    };

    template <typename T, typename U, typename... Types>
    struct Index<T, std::tuple<U, Types...>>
    {
        static constexpr std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
    };

    template <typename T>
    constexpr std::vector<T>& getValueVec()
    {
        return std::get<Index<T, std::tuple<ValueTypes...>>::value>(valueVectors);
    }
};

using namespace cxstructs;
using TransparentStringHashMap = magique::HashMapEx<std::string, ValueInfo, StringCharHash, StringCharEquals>;

//----------------- STATE DATA -----------------//
TransparentStringHashMap VALUES;                     // Placeholder to value mapping - transparent lookup enabled!
ValueStorage<std::string, float, int> VALUE_STORAGE; // All values stored by the placeholders
char FMT_PREFIX = '$';                               // The format prefix to seach for#
char FMT_ENCAP_START = '{';                          // Format encapsulator start
char FMT_ENCAP_END = '}';                            // Format encapsulator end
std::string FORMAT_CACHE(64, '\0');                  // Length of 64 to ensure it is large enough for most values
std::string STRING_BUILDER(512, '\0');               // Cache for the final formatted string

template <typename T>
constexpr ValueType getValueType()
{
    if constexpr (std::is_same_v<T, float>)
    {
        return FLOAT;
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        return INT;
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        return STRING;
    }
    else if constexpr (std::is_same_v<float, float>)
    {
        static_assert(std::is_same_v<T, std::string>, "Passed invalid type");
    }
    return INT;
} // Function to get the ValueType for a given type T

template <typename T>
void eraseValue(const ValueInfo info)
{
    std::vector<T>& vec = VALUE_STORAGE.getValueVec<T>();
    if (vec.empty())
        return;
    vec.erase(vec.begin() + info.index);
    for (auto& [type, index] : VALUES | std::views::values)
    {
        if (type == info.type && index > info.index)
        {
            index--;
        }
    }
}

template <typename T>
void SetFormatValueImpl(const char* ph, const T& val)
{
    M_ASSERT(strlen(ph) < MAGIQUE_MAX_FORMAT_LEN, "Given placholder is larger than configured max!");
    if constexpr (std::is_same_v<T, std::string>)
        M_ASSERT(val.size() < MAGIQUE_MAX_FORMAT_LEN, "Given value string is larger than configured max!");
    const auto it = VALUES.find(ph);
    auto& valueVec = VALUE_STORAGE.getValueVec<T>();
    if (it != VALUES.end())
    {
        if (it->second.type != getValueType<T>()) // placeholder has different type -> indices shifting
        {
            switch (it->second.type) // Erase old value and adjust existing indices
            {
            case FLOAT:
                eraseValue<float>(it->second);
                break;
            case STRING:
                eraseValue<std::string>(it->second);
                break;
            case INT:
                eraseValue<int>(it->second);
                break;
            }
            const auto size = static_cast<int>(valueVec.size());
            valueVec.emplace_back(std::move(val));
            VALUES.erase(it);
            VALUES.insert({ph, {getValueType<T>(), static_cast<uint8_t>(size)}});
        }
        else // Just overwrite the existing value
        {
            valueVec[it->second.index] = val;
            it->second.type = getValueType<T>();
        }
    }
    else // Just add a new value
    {
        const auto size = static_cast<int>(valueVec.size());
        valueVec.emplace_back(std::move(val));
        VALUES.insert({ph, {getValueType<T>(), static_cast<uint8_t>(size)}});
    }
}

const char* GetValueText(const ValueInfo info)
{
    switch (info.type)
    {
    case FLOAT:
        snprintf(FORMAT_CACHE.data(), 64, "%.3g", VALUE_STORAGE.getValueVec<float>()[info.index]);
        break;
    case STRING:
        {
            const std::string& str = VALUE_STORAGE.getValueVec<std::string>()[info.index];
            std::memcpy(FORMAT_CACHE.data(), str.c_str(), str.size() + 1);
            break;
        }
    case INT:
        {
            // Due to small buffer optimization doesnt cause allocation below 15 digits
            const std::string intStr = std::to_string(VALUE_STORAGE.getValueVec<int>()[info.index]);
            std::memcpy(FORMAT_CACHE.data(), intStr.c_str(), intStr.size() + 1);
            break;
        }
    default:
        return nullptr;
    }
    return FORMAT_CACHE.c_str();
}

namespace magique
{
    void SetFormatValue(const char* placeholder, const char* val) { SetFormatValueImpl(placeholder, std::string(val)); }

    void SetFormatValue(const char* placeholder, const float val) { SetFormatValueImpl(placeholder, val); }

    void SetFormatValue(const char* placeholder, const int val) { SetFormatValueImpl(placeholder, val); }

    void SetFormatValue(const char* placeholder, const std::string& val)
    {
        SetFormatValueImpl(placeholder, std::string(val));
    }

    template <typename T>
    T& GetFormatValue(const char* placeholder)
    {
        const auto it = VALUES.find(placeholder);
        if (it == VALUES.end())
        {
            LOG_FATAL("Value for given placholder does not exist: %s", placeholder);
        }
        if constexpr (std::is_same_v<T, const char*>)
        {
            return VALUE_STORAGE.getValueVec<std::string>()[it->second.index].c_str();
        }
        return VALUE_STORAGE.getValueVec<T>()[it->second.index];
    }

    void DrawTextFmt(const Font& font, const char* text, const Vector2 pos, const float size, const float spacing,
                     const Color tint)
    {
        auto* fmtText = GetFormattedText(text);
        DrawTextEx(font, fmtText, pos, size, spacing, tint);
    }

    const char* GetFormattedText(const char* text)
    {
        if (text == nullptr) [[unlikely]]
            return nullptr;

        int i = 0;
        STRING_BUILDER.clear();
        char c;

        while ((c = text[i]) != '\0')
        {
            if (c == FMT_PREFIX && text[i + 1] == FMT_ENCAP_START)
            {
                const int placeStart = i + 2;
                int j = placeStart;
                while (text[j] != '\0' && text[j] != FMT_ENCAP_END)
                {
                    ++j;
                }
                if (text[j] == FMT_ENCAP_END)
                {
                    FORMAT_CACHE.assign(text + placeStart, j - placeStart);
                    auto it = VALUES.find(FORMAT_CACHE);
                    if (it != VALUES.end())
                    {
                        auto* valueText = GetValueText(it->second);
                        STRING_BUILDER.append(valueText);
                        i = j;
                    }
                    else
                    {
                        STRING_BUILDER.push_back(c);
                    }
                }
                else
                {
                    STRING_BUILDER.push_back(c);
                }
            }
            else
            {
                STRING_BUILDER.push_back(c);
            }
            ++i;
        }
        return STRING_BUILDER.c_str();
    }


    void SetFormatPrefix(const char prefix) { FMT_PREFIX = prefix; }


    template float& GetFormatValue(const char*);
    template std::string& GetFormatValue(const char*);
    template int& GetFormatValue(const char*);

    // Old implementation - bit slower
    /*
    const char* GetFormattedText(const char* text)
    {
        if (text == nullptr) [[unlikely]]
            return nullptr;

        int i = 0;
        const int maxLen = MAX_TEXT_BUFFER_LENGTH - 1;
        char* work = const_cast<char*>(TextFormat("%s", text)); // Is valid up to maxLen
        int placeStart = -1;
        int placeEnd = -1;

        while (i < maxLen && work[i] != '\0')
        {
            if (placeStart == -1)
            {
                if (work[i] == FMT_PREFIX && work[i + 1] == FMT_ENCAP_START)
                {
                    placeStart = i + 2;
                }
            }
            else
            {
                while (i < maxLen)
                {
                    if (work[i] == FMT_ENCAP_END)
                    {
                        placeEnd = i - 1; // encap end -1 - thats why below +4 instead of +3
                        auto placeHolder = work + placeStart;
                        work[i] = '\0';
                        auto it = VALUES.find(placeHolder);
                        work[i] = FMT_ENCAP_END;
                        if (it == VALUES.end())
                        {
                            placeStart = -1;
                            placeEnd = -1;
                            break;
                        }
                        const auto valueText = GetValueText(it->second);
                        const auto valueLen = static_cast<int>(strlen(valueText));
                        const auto placeLen = placeEnd - placeStart + 4; // Including prefix, encapstart + encap end

                        if (placeLen > valueLen) [[likely]]
                        {
                            std::memcpy(work + (placeStart - 2), valueText, valueLen);
                            std::memcpy(work + (placeStart - 2 + valueLen), work + (i + 1), maxLen - i);
                            i = placeStart - 3 + valueLen;
                        }
                        else
                        {
                            const int newLen = i + valueLen - placeLen;
                            if (newLen >= MAX_TEXT_BUFFER_LENGTH)
                            {
                                return nullptr;
                            }
                            std::memmove(work + (placeStart - 2 + valueLen), work + (i + 1), maxLen - i);
                            std::memcpy(work + (placeStart - 2), valueText, valueLen);
                            i = placeStart - 3 + valueLen;
                        }
                        placeStart = -1;
                        placeEnd = -1;
                        break;
                    }
                    ++i;
                }
            }
            ++i;
        }
        return work;
    }
    */

} // namespace magique