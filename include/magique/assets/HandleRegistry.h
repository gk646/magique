// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_HANDLE_REGISTRY_H
#define MAGIQUE_HANDLE_REGISTRY_H

#include <magique/fwd.hpp>

//===============================================
// Handle Registry
//===============================================
// ................................................................................
// This module helps to organize, storing and retrieving handles
// Your free to manage and store handles on your own as well!
// ................................................................................

enum class HandleID : int; // User implementable to store handles by enum

namespace magique
{
    // Customizable parameter to handle collisions - https://en.wikipedia.org/wiki/Salt_(cryptography)
    inline constexpr int HASH_SALT = 0;

    //================= REGISTER =================//

    // Saves a handle by the given type - enum ordinal value will be used as index
    void RegisterHandle(handle handle, HandleID type);

    // Stores a handle by the given name - hashed
    void RegisterHandle(handle handle, const char* name);

    // Saves an unnamed handle directly by number - other methods should be preferred
    void RegisterDirectHandle(handle handle, int id);

    //================= GET =================//

    // Retrieves a handle identified by a type - enum ordinal value will be used as index
    handle GetHandle(HandleID type);

    // Retrieves a handle based on string hashing
    // Note: Must use the GetHash() function for hash
    handle GetHandle(uint32_t hash);

    // Retrieves a direct handle
    handle GetDirectHandle(int id);

    //================= UTIL =================//

    // Use this to get the hash
    consteval uint32_t GetHash(char const* s) noexcept;

} // namespace magique

//================= IMPLEMENTATION =================//

namespace magique
{
    // Compile time string hashing function
    // Takes an optional salt parameter (arbitrary defined value) to make it customizable in terms of collision handling
    consteval uint32_t GetHash(char const* s) noexcept
    {
        uint32_t hash = 2166136261U + HASH_SALT;
        while (*s != 0)
        {
            hash ^= static_cast<uint32_t>(*s++);
            hash *= 16777619U;
        }
        return hash;
    }

    namespace internal
    {
        constexpr uint32_t HashString(char const* s, const int salt) noexcept
        {
            uint32_t hash = 2166136261U + salt;
            while (*s != 0)
            {
                hash ^= static_cast<uint32_t>(*s++);
                hash *= 16777619U;
            }
            return hash;
        }
    } // namespace internal
} // namespace magique

#endif //MAGIQUE_HANDLE_REGISTRY_H