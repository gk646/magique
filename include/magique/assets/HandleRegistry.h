#ifndef MAGIQUE_ASSETREGISTRY_H
#define MAGIQUE_ASSETREGISTRY_H

#include <magique/fwd.hpp>
#include <magique/util/Hash.h>

//-----------------------------------------------
// Asset Registry
//-----------------------------------------------
// .....................................................................
// This module helps organizes storing and retrieving handles
// Your free to manage and store handles on your own aswell!
// .....................................................................

enum HandleType : int; // User implementable to store handles by enum

namespace magique
{
    // Customizable parameter to handle collisions
    // https://en.wikipedia.org/wiki/Salt_(cryptography)
    inline constexpr int HASH_SALT = 0;

    //----------------- REGISTER -----------------//

    // Saves a handle by the given type - not hashed -> enum ordinal value will be used as index
    void RegisterHandle(handle handle, HandleType type);

    // Stores a handle by the given name - hashed
    void RegisterHandle(handle handle, const char* name);

    // Saves an unnamed handle directly by number - useful for tilemaps
    void RegisterDirectHandle(handle handle, int id);

    //----------------- GET -----------------//

    // Retrieves a handle identified by a type - enum ordinal value will be used as index
    handle GetHandle(HandleType type);

    // Retrieves a handle based on string hashing
    // IMPORTANT: You have to use this macro to get compile time hash - sadly you can pass constexpr strings
#define H(msg) magique::util::HashStringEval(msg, HASH_SALT)
    // Example: GetHandle(H("player"));
    handle GetHandle(uint32_t hash);

    // Retrieves a direct handle - useful for tilemaps
    handle GetDirectHandle(int id);


} // namespace magique

#endif //MAGIQUE_ASSETREGISTRY_H