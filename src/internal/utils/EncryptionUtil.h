#ifndef MAGIQUE_SECURITY_H
#define MAGIQUE_SECURITY_H

// Simple xor encryption with a 64 bit key - should generally be enough
// https://reverseengineering.stackexchange.com/questions/2062/what-is-the-most-efficient-way-to-detect-and-to-break-xor-encryption

inline void SymmetricEncrypt(char* data, const int size, const unsigned long long key)
{
    for (int i = 0; i < size; ++i)
    {
        data[i] ^= static_cast<char>(key >> i % 8 * 8);
    }
}


#endif //MAGIQUE_SECURITY_H