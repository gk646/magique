#ifndef MAGIQUE_COMPRESSION_H
#define MAGIQUE_COMPRESSION_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Compression Module
//-----------------------------------------------

namespace magique
{
    // Compresses the given data with a custom algorithm
    // Returns: A pointer (and size) to newly allocated memory containing the compressed data
    DataPointer<const char> Compress(const char* data, int size);

    // Decompresses data compressed by "Compress"
    // Returns: A pointer (and size) to newly allocated memory containing the decompressed data
    DataPointer<const char> DeCompress(const char* data, int size);

} // namespace magique


#endif //MAGIQUE_COMPRESSION_H