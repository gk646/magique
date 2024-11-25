// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COMPRESSION_H
#define MAGIQUE_COMPRESSION_H

#include <magique/core/Types.h>

//===============================================
// Compression Module
//===============================================

namespace magique
{
    // Compresses the given data
    // Does NOT take ownership of the passed in data or changes the given data
    // Returns: A pointer (and size) to newly allocated memory containing the compressed data - call .free() when done
    DataPointer<const unsigned char> Compress(const unsigned char* data, int size);

    // Decompresses data compressed by "Compress"
    // Does NOT take ownership of the passed in data or changes the given data
    // Returns: A pointer (and size) to newly allocated memory containing the decompressed data - call .free() when done
    DataPointer<const unsigned char> DeCompress(const unsigned char* data, int size);


} // namespace magique


#endif //MAGIQUE_COMPRESSION_H