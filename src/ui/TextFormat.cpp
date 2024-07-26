#include <vector>
#include <raylib/raylib.h>

#include <magique/ui/TextFormat.h>
#include <magique/internal/DataStructures.h>

// Note: This implementation uses separate vectors to store values without memory overhead
//       Inserting new values, and overwriting values from the same type is fast.
//       Changing the type of a value is slower as we need to shift indices of all existing values

enum ValueType : uint8_t
{
    FLOAT,
    STRING,
    INT
};

struct ValueInfo final
{
    ValueType type = INT;
    uint8_t index; // Index into the value vector
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

// State data
ValueStorage<std::string, float, int> VALUE_STORAGE;
magique::HashMap<std::string, ValueInfo> VALUES;
std::vector<const char*> FORMATS{};
auto* FMT_PREFIX = "${";
auto* FMT_SUFFIX = "}";


// Function to get the ValueType for a given type T
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
}

template <typename T>
void eraseValue(const ValueInfo info)
{
    std::vector<T>& vec = VALUE_STORAGE.getValueVec<T>();
    if (vec.empty())
        return;
    vec.erase(vec.begin() + info.index);
    for (auto& [key, val] : VALUES)
    {
        if (val.type == info.type && val.index > info.index)
        {
            val.index--;
        }
    }
}

template <typename T>
void SetFormatValueImpl(const char* ph, const T& val)
{
    auto it = VALUES.find(ph);
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
            valueVec.emplace_back(val);
            VALUES.insert({ph, {STRING, static_cast<uint8_t>(size)}});
        }
        else // Just overwrite the existing value
        {
            valueVec[it->second.index] = val;
        }
    }
    else // Just add a new value
    {
        const auto size = static_cast<int>(valueVec.size());
        valueVec.emplace_back(val);
        VALUES.insert({ph, {STRING, static_cast<uint8_t>(size)}});
    }
}

namespace magique
{
    void SetFormatMask(const char* prefix, const char* suffix)
    {
        // Copied for safety - method shouldnt be called often
        static bool first = true;
        if (!first)
        {
            free((void*)FMT_PREFIX);
            free((void*)FMT_SUFFIX);
        }
        FMT_PREFIX = strdup(prefix);
        FMT_SUFFIX = strdup(suffix);
        first = false;
    }

    void SetFormatValue(const char* placeholder, const char* val) { SetFormatValueImpl(placeholder, std::string(val)); }

    void SetFormatValue(const char* placeholder, float val) { SetFormatValueImpl(placeholder, val); }

    void SetFormatValue(const char* placeholder, int val) { SetFormatValueImpl(placeholder, val); }

    void DrawTextFmt(const char* text, const Font& font, Vector2 pos, float size, float spacing, Color tint)
    {
        auto* fmtText = GetFormattedText(text);
        DrawTextEx(font, fmtText, pos, size, spacing, tint);
    }

    const char* GetFormattedText(const char* text) { return nullptr; }

} // namespace magique