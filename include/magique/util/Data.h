// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_DATA_H
#define MAGIQUE_DATA_H

#include <magique/core/Types.h>

//===============================================
// Data Operations
//===============================================
// .....................................................................
// This system is trimmed for speed by busy waiting during the tick to quickly pickup tasks.
// Between ticks, it's in hibernation, sleeping until woken up again (if not used).
// Allows to submit concurrent jobs to distribute compatible work across threads and await their completion.
// Per default has MAGIQUE_WORKER_THREADS many worker threads.
// Note: Don't forget to give the main thread work as well BEFORE waiting for the jobs to return!
// .....................................................................

namespace magique
{
    // Returns the compressed data or the original, whichever one is smaller and a bool indication if compression happened
    //      - minSize: compression is not attempted below
    std::pair<std::string_view, bool> DataCompress(std::string_view data, size_t minSize = 128);

    // Tries to uncompress data compressed by CompressData()
    //      - minOutBuffer: minimal (starting) size of the output buffer - decompression fails if data doesnt fit
    // Failure: Returns empty view
    std::string_view DataDecompress(std::string_view data, size_t minOutBuffer = 64'000);

    // Returns true if the data was [En/De]crypted inplace with the given key
    // Note: EncryptionKey{0} skips encryption
    // Note: Encryption adds a few bytes at the end - encryption removes them again (this is why std::string& is used)
    // Note: Uses AES128-CTR with a new (hardware) random 16 byte IV each call
    bool DataEncrypt(std::string& data, EncryptionKey key);
    bool DataDecrypt(std::string& data, EncryptionKey key);

    // Returns the hash of the given data
    // Note: Uses BLAKE3
    std::string_view DataHash(std::string_view data);

} // namespace magique
#endif // MAGIQUE_DATA_H
