#ifndef MAGIQUE_HASH_H
#define MAGIQUE_HASH_H


namespace magique::util
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

    inline uint32_t HashString(char const* s, const int salt) noexcept
    {
        uint32_t hash = 2166136261U + salt;
        while (*s != 0)
        {
            hash ^= static_cast<uint32_t>(*s++);
            hash *= 16777619U;
        }
        return hash;
    }


} // namespace magique::util

#endif //MAGIQUE_HASH_H