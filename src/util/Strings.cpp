#include <magique/gamedev/Strings.h>

#include <cxutil/cxstring.h>


namespace magique
{
    struct Point
    {
        float x = 0.0F;
        float y = 0.0F;
    };

    struct PointI
    {
        uint8_t x;
        uint8_t y;
    };

    constexpr Point KEY_POSITIONS[4][10] = {
        {
            {
                {0.0, 0},
                {1.0, 0},
                {2.0, 0},
                {3.0, 0},
                {4.0, 0},
                {5.0, 0},
                {6.0, 0},
                {7.0, 0},
                {8.0, 0},
                {9.0, 0},
            }, // '1' to '0'
            {{0.5, 1},
             {1.5, 1},
             {2.5, 1},
             {3.5, 1},
             {4.5, 1},
             {5.5, 1},
             {6.5, 1},
             {7.5, 1},
             {8.5, 1},
             {9.5, 1}}, // 'q' to 'p'
            {{0.75, 2},
             {1.75, 2},
             {2.75, 2},
             {3.75, 2},
             {4.75, 2},
             {5.75, 2},
             {6.75, 2},
             {7.75, 2},
             {8.75, 2},
             {}}, // 'a' to 'l'
            {{1.25, 3},
             {2.25, 3},
             {3.25, 3},
             {4.25, 3},
             {5.25, 3},
             {6.25, 3},
             {7.25, 3},
             {},
             {},
             {}}, // 'z' to 'm' //////////
        },
    };

    constexpr PointI CHAR_LOOKUP[2][36] = {
        // QWERTY layout
        {
            {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, // '1' to '0'
            {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, // 'q' to 'p'
            {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8},         // 'a' to 'l'
            {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}                          // 'z' to 'm'
        },
        // QWERTZ layout
        {
            {0, 0}, {0, 1}, {0, 2}, {0, 3}, {3, 0}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, // '1' to '0'
            {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, // 'q' to 'p'
            {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8},         // 'a' to 'l'
            {0, 4}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}                          // 'y' to 'm'
        }};

    PointI getCharIndices(const KeyboardLayout layout, const char c)
    {
        const int num = static_cast<int>(layout);
        if (c >= '1' && c <= '9')
            return CHAR_LOOKUP[num][c - '1'];
        if (c == '0')
            return CHAR_LOOKUP[num][9];
        if (c >= 'a' && c <= 'z')
            return CHAR_LOOKUP[num][10 + (c - 'a')];
        return {UINT8_MAX, UINT8_MAX};
    }

    Point getCharPosition(const KeyboardLayout layout, const char c)
    {
        const auto [x, y] = getCharIndices(layout, c);
        if (x == UINT8_MAX)
            return {-1, -1};
        return KEY_POSITIONS[x][y];
    }


    int WordDistance(const char* s1, const char* s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1, s2, caseSensitive);
    }

    int WordDistance(const std::string& s1, const std::string& s2, const bool caseSensitive)
    {
        return cxstructs::str_sort_levenshtein_case<16>(s1.c_str(), s2.c_str(), caseSensitive);
    }
    float WordSimilarity(const char* s1, const char* s2, bool caseSensitive) {}
    float WordSimilarity(const std::string& s1, const std::string& s2, bool caseSensitive) {}
    float WordSimilarityKeyBoard(const char* s1, const char* s2, KeyboardLayout layout) {}
    float WordSimilarityKeyBoard(const std::string& s1, const std::string& s2, KeyboardLayout layout) {}


    const char* GetTimeString(int seconds) {}


} // namespace magique