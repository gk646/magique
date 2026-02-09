// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_BITUTIL_H
#define MAGIQUE_BITUTIL_H

// TODO rewrite sharecode and remove
namespace magique
{
    // Writes 'bitsToWrite' many bits from val to 'byte' starting at the given bit offset
    template <typename T>
    int WriteBits(T val, const int bitsToWrite, char& byte, const int offset)
    {
        const int bitsAvailableInByte = 8 - offset;
        const int bitsToWriteNow = std::min(bitsToWrite, bitsAvailableInByte);
        T mask = (1 << bitsToWriteNow) - 1;
        byte &= ~(mask << offset);
        byte |= ((val & mask) << offset);
        return bitsToWriteNow;
    }

    // Read 'bitsToRead' many bits into val from 'byte' starting at the given bit offset
    template <typename T>
    int ReadBits(T& val, const int bitsToRead, const char byte, const int offset)
    {
        const int bitsAvailableInByte = 8 - offset;
        const int bitsToReadNow = std::min(bitsToRead, bitsAvailableInByte);
        T mask = (1 << bitsToReadNow) - 1;
        T extractedBits = (byte >> offset) & mask;
        val |= extractedBits;
        return bitsToReadNow;
    }
} // namespace magique

#endif //MAGIQUE_BITUTIL_H