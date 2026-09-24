// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SHARECODE_H
#define MAGIQUE_SHARECODE_H

#include <magique/util/Logging.h>
#include <magique/util/Strings.h>
#include <magique/util/Reflection.h>

//===============================
// ShareCode
//===============================
// ................................................................................
// This module allows to generate base64 encoded strings that efficiently pack the given data
// This is use in: WoW Talent Export, Loadout Export, ...
// The advantage is that the format is not encoded into the data => as small as possible
// Note: ShareCodeType can be ignored OR used to conditionally decided which struct to use for importing
// ................................................................................

enum class ShareCodeType : uint8_t; // User implemented to identify different sharecodes

namespace magique
{
    // Generates a sharecode from the given struct that encodes its current values
    template <class T>
    ShareCode ShareCodeExport(ShareCodeType type, const T& val);

    // Returns a new object of the given struct with the decoded values assigned
    template <class T>
    std::optional<T> ShareCodeImport(ShareCode code);

} // namespace magique

//================= IMPLEMENTATION =================//

namespace magique
{

    template <typename T>
    ShareCode ShareCodeExport(ShareCodeType type, const T& val)
    {
        ShareCode code;
        code.data.reserve(16);
        code.data.push_back((char)type);

        static constexpr auto members = std::define_static_array(ReflectClassMembers<T>());
        template for (constexpr auto member : members)
        {
            if  constexpr (std::is_convertible_v<typename[:std::meta::type_of(member):], std::string>)
            {
                code.data.push_back((char)val.[:member:].size());
                code.data.append(val.[:member:]);
            }
            else if constexpr (std::is_arithmetic_v<typename[:std::meta::type_of(member):]>)
            {
                char buff[sizeof(typename[:std::meta::type_of(member):])];
                std::memcpy(buff, &val.[:member:], sizeof(buff));
                code.data.append(std::string_view{buff, sizeof(buff)});
            } else
            {
                static_assert(std::is_same_v<typename[:std::meta::type_of(member):], int>, "Unsupported sharecode type");
            }
        }
        code.data = StringToBase64(code.data);
        return code;
    }

    template <typename T>
    std::optional<T> ShareCodeImport(ShareCode code)
    {
        std::string_view decoded = StringFromBase64(code);
        static constexpr auto members = std::define_static_array(ReflectClassMembers<T>());

        T val{};
        int offset = 1; // type is first byte
        template for (constexpr auto member : members)
        {
            if (offset >= decoded.size())
            {
                LOG_WARNING("Failed to parse sharecode into: %s", std::meta::identifier_of(^^T).data());
                return {};
            }

            if constexpr (std::is_convertible_v<typename[:std::meta::type_of(member):], std::string_view>)
            {
                auto size = (int)decoded[offset];
                if (size >= 255) // Sanity check
                {
                    LOG_WARNING("Failed to parse sharecode into: %s", std::meta::identifier_of(^^T).data());
                    return {};
                }
                offset++;
                val.[:member:] = decoded.subview(offset , size);
                offset += size;
            }
            else if constexpr (std::is_arithmetic_v<typename[:std::meta::type_of(member):]>)
            {
                constexpr auto size = sizeof(typename[:std::meta::type_of(member):]);
                std::memcpy(&val.[:member:], decoded.subview(offset, size).data(), size);
                offset += size;
            }
            else
            {
                static_assert(std::is_same_v<typename[:std::meta::type_of(member):], int>, "Unsupported sharecode type");
                return {};
            }
        }
        return val;
    }

} // namespace magique

#endif // MAGIQUE_SHARECODE_H
