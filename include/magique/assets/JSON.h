// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CSVREADER_H
#define MAGIQUE_CSVREADER_H

#include <glaze/json/write.hpp>
#include <enchantum/enchantum.hpp>
#include <raylib/raylib.h>
#include <magique/assets/types/Asset.h>
#include <magique/gamedev/VirtualClock.h>
#include <magique/util/Datastructures.h>
#include <magique/internal/InternalTypes.h>

//===============================================
// JSON Import/Exports
//===============================================
// ................................................................................
// magique uses glaze for JSON importing and exporting
// glaze allows to import/export C++ types (and also generic JSON with glz::generic)
// For standard types it's as easy as ExportJSON(myType, buffer); the serialization is generated automatically
// All standard type (std::array, std::vector, ...) and your own structs that use only those type work automatically!
// Refer to https://stephenberry.github.io/glaze/json/ for more info on how to use the JSON library
//
// IMPORTANT: If you define custom parse/serialization rules etc. they need to be included BEFORE you call Import/Export
// Notes: This header also contains the serialization specializations for magique (and raylib) structs
// ................................................................................

namespace magique
{

    // Deserializes the given JSON into the given c++ type
    // Refer to https://stephenberry.github.io/glaze/json/
    template <typename T>
    bool JSONImport(Asset asset, T& data);
    template <typename T>
    bool JSONImport(std::string_view json, T& data);

    // Serialized the given data into the buffer (will be cleared and sized appropriately)
    template <typename T, bool prettify = true>
    bool JSONExport(const T& data, std::string& buffer);

    // Useful to reflect an enum - allows to import/export from JSON (with value names)
    // Note: static analysis may show errors but code compiles (clang-tidy)
#define MQ_REFLECT_ENUM(Enum)                                                                                           \
    template <>                                                                                                         \
    struct glz::meta<Enum>                                                                                              \
    {                                                                                                                   \
        using enum Enum;                                                                                                \
        static constexpr auto value = enchantum::values<Enum>;                                                          \
        static constexpr auto keys = enchantum::names<Enum>;                                                            \
    };

} // namespace magique


// IMPLEMENTATION


MQ_REFLECT_ENUM(magique::KeyBindType)
MQ_REFLECT_ENUM(magique::StorageType)

namespace glz
{
    template <>
    struct meta<magique::Point>
    {
        using T = magique::Point;
        static constexpr auto value = object(&T::x, &T::y);
    };

    template <>
    struct meta<Vector2>
    {
        using T = Vector2;
        static constexpr auto value = object(&T::x, &T::y);
    };

    template <>
    struct meta<magique::Rect>
    {
        using T = magique::Rect;
        static constexpr auto value = object(&T::x, &T::y, &T::width, &T::height);
    };

    template <>
    struct meta<magique::Keybind>
    {
        using T = magique::Keybind;
        static constexpr auto value = object(&T::key, &T::type, &T::layered, &T::shift, &T::ctrl, &T::alt);
    };

    template <>
    struct meta<magique::VirtualClock>
    {
        using T = magique::VirtualClock;
        static constexpr auto value = object(&T::realSecondSeconds, &T::ticks, &T::timeScale, &T::isPaused);
    };

    template <>
    struct meta<Color>
    {
        using T = Color;
        static constexpr auto value = object(&T::r, &T::g, &T::b, &T::a);
    };

    template <>
    struct meta<magique::internal::StorageCell>
    {
        using T = magique::internal::StorageCell;
        static constexpr auto values = object(&T::name, &T::type, &T::data);
    };

    template <typename K, typename V, int maxSize>
    struct from<JSON, magique::EnumArray<K, V, maxSize>>
    {
        template <auto Opts>
        static void op(magique::EnumArray<K, V, maxSize>& value, auto&&... args)
        {
            std::vector<typename magique::EnumArray<K, V, maxSize>::ValueHolder> vec;
            parse<JSON>::op<Opts>(vec, args...);
            value = magique::EnumArray<K, V, maxSize>{vec};
        }
    };

    template <typename K, typename V, int maxSize>
    struct to<JSON, magique::EnumArray<K, V, maxSize>>
    {
        template <auto Opts>
        static void op(const magique::EnumArray<K, V, maxSize>& value, auto&&... args) noexcept
        {
            std::vector<typename magique::EnumArray<K, V, maxSize>::ValueHolder> vec;
            for (const auto& [key, val] : value)
            {
                vec.emplace_back(key, val);
            }
            serialize<JSON>::op<Opts>(vec, args...);
        }
    };

    template <typename K, typename V>
    struct from<JSON, magique::HashMap<K, V>>
    {
        template <auto Opts>
        static void op(magique::HashMap<K, V>& value, auto&&... args)
        {
            struct ValueHolder
            {
                K key;
                V value;
            };
            std::vector<ValueHolder> vec;
            parse<JSON>::op<Opts>(vec, args...);
            for (auto& [key, val] : vec)
            {
                value[key] = std::move(val);
            }
        }
    };

    template <typename K, typename V>
    struct to<JSON, magique::HashMap<K, V>>
    {
        template <auto Opts>
        static void op(const magique::HashMap<K, V>& value, auto&&... args) noexcept
        {
            struct ValueHolder
            {
                K key;
                V value;
            };
            std::vector<ValueHolder> vec;
            for (const auto& [key, val] : value)
            {
                vec.emplace_back(key, val);
            }
            serialize<JSON>::op<Opts>(vec, args...);
        }
    };

    template <typename V>
    struct from<JSON, magique::HashSet<V>>
    {
        template <auto Opts>
        static void op(magique::HashSet<V>& value, auto&&... args)
        {
            std::vector<V> vec;
            parse<JSON>::op<Opts>(vec, args...);
            for (auto& val : vec)
            {
                value.insert(std::move(val));
            }
        }
    };

    template <typename V>
    struct to<JSON, magique::HashSet<V>>
    {
        template <auto Opts>
        static void op(const magique::HashSet< V>& value, auto&&... args) noexcept
        {
            const std::vector<V>& vec = value.values();
            serialize<JSON>::op<Opts>(vec, args...);
        }
    };

} // namespace glz

namespace magique
{
    template <typename T>
    bool JSONImport(Asset asset, T& data)
    {
        std::string_view buff{asset.getData(), static_cast<size_t>(asset.getSize())};
        auto ec = glz::read_jsonc(data, buff);
        if (ec)
        {
            LOG_ERROR("Failed to import JSON asset %s:%s", asset.getPath(), glz::format_error(ec, buff).c_str());
            return false;
        }
        return true;
    }

    template <typename T>
    bool JSONImport(std::string_view json, T& data)
    {
        const auto ec = glz::read_jsonc(data, json);
        if (ec)
        {
            LOG_ERROR("Failed to import JSON:%s", glz::format_error(ec, json).c_str());
            return false;
        }
        return true;
    }

    template <typename T, bool prettify>
    bool JSONExport(const T& data, std::string& buffer)
    {
        const auto ec = glz::write<glz::opts{.prettify = prettify, .new_lines_in_arrays = false}>(data, buffer);
        if (ec)
        {
            LOG_ERROR("Failed to export JSON: %s", glz::format_error(ec, buffer).c_str());
            return false;
        }
        return true;
    }

} // namespace magique


#endif // MAGIQUE_CSVREADER_H
