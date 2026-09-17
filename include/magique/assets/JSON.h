// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CSVREADER_H
#define MAGIQUE_CSVREADER_H

#include <magique/internal/glaze/json/write.hpp>
#include <magique/assets/types/Asset.h>
#include <magique/util/Datastructures.h>

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

    // Deserializes the given JSON string into the given c++ type
    // Refer to https://stephenberry.github.io/glaze/json/
    //      - append: appends the data instead of replacing if T==std::vector (or others that support it)
    template <bool append = false, typename T>
    bool JSONImport(Asset asset, T& obj);

    template <bool append = false, typename T>
    bool JSONImport(std::string_view json, T& data);

    // Serialized the given data into the buffer (will be cleared and sized appropriately)
    template <bool prettify = true, typename T>
    bool JSONExport(const T& data, std::string& buffer);

} // namespace magique

// IMPLEMENTATION

namespace glz
{

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

    template <typename K, typename V, typename HashFunc, typename EqualsFunc>
    struct from<JSON, magique::HashMapEx<K, V, HashFunc, EqualsFunc>>
    {
        template <auto Opts>
        static void op(magique::HashMapEx<K, V, HashFunc, EqualsFunc>& value, auto&&... args)
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

    template <typename K, typename V, typename HashFunc, typename EqualsFunc>
    struct to<JSON, magique::HashMapEx<K, V, HashFunc, EqualsFunc>>
    {
        template <auto Opts>
        static void op(const magique::HashMapEx<K, V, HashFunc, EqualsFunc>& value, auto&&... args) noexcept
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
        static void op(const magique::HashSet<V>& value, auto&&... args) noexcept
        {
            const std::vector<V>& vec = value.values();
            serialize<JSON>::op<Opts>(vec, args...);
        }
    };

} // namespace glz

namespace magique
{
    struct CustomOptions : glz::opts
    {
        bool reflect_enums = true;
        bool append_arrays;
        bool linear_search = true;
        constexpr CustomOptions(bool append_arrays = true, bool prettify = true) : append_arrays(append_arrays)
        {
            comments = true;
            this->prettify = prettify;
        }
    };

    template <bool append, typename T>
    bool JSONImport(const Asset asset, T& obj)
    {
        std::string_view data = asset;
        auto ec = glz::read<CustomOptions{append}>(obj, data);
        if (ec)
        {
            LOG_ERROR("Failed to import JSON asset %s:%s", asset.getPath().data(), glz::format_error(ec, data).c_str());
            return false;
        }
        return true;
    }

    template <bool append, typename T>
    bool JSONImport(std::string_view json, T& data)
    {
        glz::context ctx{};
        auto ec = glz::read<CustomOptions{append}>(data, json, ctx);
        if (ec)
        {
            LOG_ERROR("Failed to import JSON:%s", glz::format_error(ec, json).c_str());
            return false;
        }
        return true;
    }

    template <bool prettify, typename T>
    bool JSONExport(const T& data, std::string& buffer)
    {
        const auto ec = glz::write<CustomOptions{false, prettify}>(data, buffer);
        if (ec)
        {
            LOG_ERROR("Failed to export JSON: %s", glz::format_error(ec, buffer).c_str());
            return false;
        }
        return true;
    }

} // namespace magique


#endif // MAGIQUE_CSVREADER_H
