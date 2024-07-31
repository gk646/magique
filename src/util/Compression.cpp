#include <cstring>

#include <magique/util/Compression.h>
#include <magique/util/Logging.h>

#define SINFL_IMPLEMENTATION
#define SINFL_NO_SIMD
#include "external/raylib/src/external/sinfl.h"
#define SDEFL_IMPLEMENTATION
#include "external/raylib/src/external/sdefl.h"

namespace magique
{
    DataPointer<const unsigned char> Compress(const unsigned char* data, const int size)
    {
        if (!data || size <= 0)
        {
            LOG_ERROR("Passed nullptr or invalid size");
            return {nullptr, 0};
        }

        sdefl state{};
        const int maxCompressedSize = sdefl_bound(size);

        auto* compressedData = new unsigned char[maxCompressedSize];
        int compressedSize = sdeflate(&state, compressedData, data, size, 3);

        if (compressedSize <= 0)
        {
            delete[] compressedData;
            LOG_ERROR("Failed to compress data");
            return {nullptr, 0};
        }

        // Allocate exacty data
        auto* result = new unsigned char[compressedSize];
        std::memcpy(result, compressedData, compressedSize);

        delete[] compressedData;
        return {result, compressedSize};
    }

    DataPointer<const unsigned char> DeCompress(const unsigned char* data, const int size)
    {
        if (!data || size <= 0)
        {
            LOG_ERROR("Passed nullptr or invalid size");
            return {nullptr, 0};
        }

        const int decompressedSizeEstimate = size * 7; // Guess the size
        auto* decompressedData = new unsigned char[decompressedSizeEstimate];

        // Decompress the data
        int decompressedSize = sinflate(decompressedData, decompressedSizeEstimate, data, size);

        if (decompressedSize <= 0)
        {
            delete[] decompressedData;
            LOG_ERROR("Failed to decompress data");
            return {nullptr, 0};
        }
        // Allocate exacty data
        auto* result = new unsigned char[decompressedSize];
        std::memcpy(result, decompressedData, decompressedSize);

        delete[] decompressedData;
        return {result, decompressedSize};
    }

} // namespace magique