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

    // Convenience wrapper to quickly get the JSON of a type
    // Note: This is useful for logging as well
    template <typename T>
    std::string_view ToJSON(const T& value);

} // namespace magique

// IMPLEMENTATION

namespace glz
{

    template <typename Type>
    struct meta<DynamicGrid<Type>>
    {
        using T = DynamicGrid<Type>;
        static constexpr auto value = object(&T::cols, &T::rows, &T::data);
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
    struct json_opts
    {
        uint32_t format = glz::JSON;
        bool bools_as_numbers = true;
        bool reflect_enums = true;
        bool null_terminated = GLZ_NULL_TERMINATED;
        bool comments = true;
        bool error_on_unknown_keys = true;
        bool skip_null_members = true;
        bool prettify = true;
        bool minified = false;
        bool partial_read = false;
        bool error_on_missing_keys = false;
        bool append_arrays = false;
        uint32_t internal{};
    };

    template <bool append, typename T>
    bool JSONImport(const Asset asset, T& obj)
    {
        std::string_view data = asset;
        auto ec = glz::read<json_opts{.append_arrays = append}>(obj, data);
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
        auto ec = glz::read<json_opts{.append_arrays = append}>(data, json, ctx);
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
        const auto ec = glz::write<json_opts{.prettify = prettify}>(data, buffer);
        if (ec)
        {
            LOG_ERROR("Failed to export JSON: %s", glz::format_error(ec, buffer).c_str());
            return false;
        }
        return true;
    }

    template <typename T>
    std::string_view ToJSON(const T& value)
    {
        thread_local std::string CACHE;
        JSONExport<true>(value, CACHE);
        return CACHE;
    }

} // namespace magique


#endif // MAGIQUE_CSVREADER_H
