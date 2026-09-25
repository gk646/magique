// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_DATA_H
#define MAGIQUE_DATA_H

#include <magique/core/Types.h>

//===============================================
// Data Operations
//===============================================
// .....................................................................
// This module contains useful methods that operate on data
// .....................................................................

namespace magique
{

    // Returns true if the given file was read into data
    // Note: Usually when using magique as intended, reading files directly should not be necessary
    bool DataReadFile(std::string_view file, std::string& data);

    // Returns true if the given data was written to file
    // Note: Usually when using magique as intended, writing files directly should not be necessary
    bool DataWriteFile(std::string_view file, std::string_view data);

    // Returns the compressed data
    std::string_view DataCompress(std::string_view data);

    // Tries to uncompress data that was compressed with DataCompress()
    std::optional<std::string_view> DataDecompress(std::string_view data);

    // Returns true if the data was [En/De]crypted inplace with the given key
    // Note: EncryptionKey{0} skips encryption
    // Note: Encryption adds a few bytes at the end - encryption removes them again (this is why std::string& is used)
    // Note: Uses AES128-CTR with a new (hardware) random 16 byte IV each call
    bool DataEncrypt(std::string& data, EncryptionKey key);
    bool DataDecrypt(std::string& data, EncryptionKey key);

    // Returns the hash of the given data
    // Note: Uses BLAKE3
    Hash DataHash(std::string_view data);

} // namespace magique

// IMPLEMENTATION

namespace magique
{
    namespace internal
    {
        // Those function allow moving the returned data directly which avoids a copy operation
        std::string& DataCompressImpl(std::string_view data);
        std::optional<std::reference_wrapper<std::string>> DataDecompressImpl(std::string_view data);
    } // namespace internal
} // namespace magique
#endif // MAGIQUE_DATA_H
