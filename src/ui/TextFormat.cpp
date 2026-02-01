// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/config.h>
#include <magique/ui/TextFormat.h>
#include <magique/util/Datastructures.h>
#include <magique/internal/Macros.h>

// Note: This implementation uses separate vectors to store values without memory overhead
//       Inserting new values, and overwriting values from the same type is fast.
//       Changing the type of a value is a bit slower - but very rare!

namespace magique
{
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

    struct ValueStorage final
    {
        std::vector<std::string> strings;
        std::vector<int> ints;
        std::vector<float> floats;

        template <typename T>
        constexpr std::vector<T>& getValueVec()
        {
            if constexpr (std::is_same_v<std::string, T>)
            {
                return strings;
            }
            else if constexpr (std::is_same_v<int, T>)
            {
                return ints;
            }
            else if constexpr (std::is_same_v<float, T>)
            {
                return floats;
            }
        }
    };

    //----------------- STATE DATA -----------------//

    inline StringHashMap<ValueInfo> VALUES{};     // Placeholder to value mapping - transparent lookup enabled!
    inline ValueStorage VALUE_STORAGE{};          // All values stored by the placeholders
    inline char FMT_PREFIX = '$';                 // The format prefix to each for#
    inline char FMT_ENCAP_START = '{';            // Format encapsulator start
    inline char FMT_ENCAP_END = '}';              // Format encapsulator end
    inline std::string FORMAT_CACHE(64, '\0');    // Length of 64 to ensure it is large enough for most values
    inline std::string STRING_BUILDER(512, '\0'); // Cache for the final formatted string

    template <typename T>
    static constexpr ValueType getValueType() // Function to get the ValueType for a given type T
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
    }

    static void eraseValue(const ValueInfo info)
    {
        const auto erase = []<typename T>(const ValueInfo info)
        {
            // Where we have to delete
            auto& vec = VALUE_STORAGE.getValueVec<T>();

            if (vec.empty())
                return;
            if (vec.size() == 1)
            {
                vec.pop_back();
                return;
            }

            // The info to the value which we have to move
            const ValueInfo replacement{getValueType<T>(), static_cast<uint8_t>(vec.size() - 1)};

            // Move and pop
            vec[info.index] = vec.back();
            vec.pop_back();

            // Find the info to the moved value and set its index to where we just moved it
            for (auto& pair : VALUES)
            {
                if (pair.second.index == replacement.index && pair.second.type == replacement.type)
                {
                    pair.second.index = info.index;
                    break;
                }
            }
        };
        switch (info.type)
        {
        case FLOAT:
            erase.operator()<float>(info);
            break;
        case STRING:
            erase.operator()<std::string>(info);
            break;
        case INT:
            erase.operator()<int>(info);
            break;
        }
    }

    template <typename T>
    static void SetFormatValueImpl(const char* key, const auto& val)
    {
        MAGIQUE_ASSERT(strlen(key) < MAGIQUE_MAX_FORMAT_LEN, "Given placeholder is larger than configured max!");
        if constexpr (std::is_same_v<T, const char*>)
        {
            MAGIQUE_ASSERT(strlen(val) < MAGIQUE_MAX_FORMAT_LEN, "Given value string is larger than configured max!");
        }
        const auto it = VALUES.find(key);
        auto& valueVec = VALUE_STORAGE.getValueVec<T>(); // where to insert new value
        if (it != VALUES.end())                          // placeholder exists
        {
            if (it->second.type != getValueType<T>()) [[unlikely]] // placeholder has different type -> erase old
            {
                // Erase old value and adjust existing indices
                // Can use T because T is the new type not the existing one!
                eraseValue(it->second);
                const auto size = static_cast<int>(valueVec.size());
                valueVec.emplace_back(val);
                it->second.index = static_cast<uint8_t>(size);
                it->second.type = getValueType<T>();
            }
            else // placeholder has same type -> just overwrite old
            {
                valueVec[it->second.index] = val;
            }
        }
        else // Just add a new value
        {
            const auto size = static_cast<int>(valueVec.size());
            valueVec.emplace_back(val);
            VALUES.insert({key, {getValueType<T>(), static_cast<uint8_t>(size)}});
        }
    }

    static const char* GetValueText(const ValueInfo info)
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
                snprintf(FORMAT_CACHE.data(), 64, "%d", VALUE_STORAGE.getValueVec<int>()[info.index]);
                break;
            }
        }
        return FORMAT_CACHE.c_str();
    }

    //----------------- IMPLEMENTATION -----------------//

    void FormatSetValue(const char* placeholder, const std::string_view& val)
    {
        SetFormatValueImpl<std::string>(placeholder, val);
    }

    void FormatSetValue(const char* placeholder, const float val) { SetFormatValueImpl<float>(placeholder, val); }

    void FormatSetValue(const char* placeholder, const int val) { SetFormatValueImpl<int>(placeholder, val); }

    void DrawTextFmt(const Font& f, const char* t, const Vector2 p, const float s, const float sp, const Color c)
    {
        auto* fmtText = FormatGetText(t);
        DrawTextEx(f, fmtText, p, s, sp, c);
    }

    const char* FormatGetText(const char* text)
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
                    const auto it = VALUES.find(FORMAT_CACHE);
                    if (it != VALUES.end())
                    {
                        const auto* valueText = GetValueText(it->second);
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

    void FormatSetPrefix(const char prefix) { FMT_PREFIX = prefix; }

    template <typename T>
    T* FormatGetValue(const std::string_view& placeholder)
    {
        const auto it = VALUES.find(placeholder);
        if (it == VALUES.end())
        {
            return nullptr;
        }

        if constexpr (std::is_same_v<T, std::string>)
        {
            if (it->second.type != STRING)
            {
                return nullptr;
            }
            return &VALUE_STORAGE.getValueVec<std::string>()[it->second.index];
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            if (it->second.type != INT)
            {
                return nullptr;
            }
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            if (it->second.type != FLOAT)
            {
                return nullptr;
            }
        }
        return &VALUE_STORAGE.getValueVec<T>()[it->second.index];
    }

    template float* FormatGetValue(const std::string_view&);
    template std::string* FormatGetValue(const std::string_view&);
    template int* FormatGetValue(const std::string_view&);

} // namespace magique
