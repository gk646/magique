#ifndef MAGIQUE_ASSETREGISTRY_H
#define MAGIQUE_ASSETREGISTRY_H

#include <magique/fwd.hpp>
#include <magique/internal/Hash.h>

//-----------------------------------------------
// Handle Registry (optional)
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
#define H(msg) magique::HashStringEval(msg, magique::HASH_SALT)
    // Example: GetHandle(H("player"));
    handle GetHandle(uint32_t hash);

    // Retrieves a direct handle - useful for tilemaps
    handle GetDirectHandle(int id);

} // namespace magique


#endif //MAGIQUE_ASSETREGISTRY_H