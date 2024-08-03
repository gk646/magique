#ifndef MAGIQUE_HANDLE_REGISTRY_H
#define MAGIQUE_HANDLE_REGISTRY_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Handle Registry
//-----------------------------------------------
// ................................................................................
// This module helps organizing, storing and retrieving handles
// Your free to manage and store handles on your own aswell!
// ................................................................................

enum class HandleID : int; // User implementable to store handles by enum

namespace magique
{
    // Customizable parameter to handle collisions
    // https://en.wikipedia.org/wiki/Salt_(cryptography)
    inline constexpr int HASH_SALT = 0;

    //----------------- REGISTER -----------------//

    // Saves a handle by the given type - enum ordinal value will be used as index
    void RegisterHandle(handle handle, HandleID type);

    // Stores a handle by the given name - hashed
    void RegisterHandle(handle handle, const char* name);

    // Saves an unnamed handle directly by number
    void RegisterDirectHandle(handle handle, int id);

    //----------------- GET -----------------//

    // Retrieves a handle identified by a type - enum ordinal value will be used as index
    handle GetHandle(HandleID type);

    // Retrieves a handle based on string hashing
    // IMPORTANT: You have to use this macro to get compile time hashing - sadly you can pass constexpr strings
#define H(msg) magique::internal::HashStringEval(msg, magique::HASH_SALT)
    // Example: GetHandle(H("player"));
    handle GetHandle(uint32_t hash);

    // Retrieves a direct handle - useful for tilemaps
    handle GetDirectHandle(int id);

} // namespace magique


//----------------- IMPLEMENTATION -----------------//

namespace magique::internal
{
    // Compile time string hashing function
    // Takes an optional salt parameter (arbitrary defined value) to make it customizable in terms of collision handling
    consteval uint32_t HashStringEval(char const* s, const int salt) noexcept
    {
        uint32_t hash = 2166136261U + salt;
        while (*s != 0)
        {
            hash ^= static_cast<uint32_t>(*s++);
            hash *= 16777619U;
        }
        return hash;
    }

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
} // namespace magique::interal

#endif //MAGIQUE_HANDLE_REGISTRY_H