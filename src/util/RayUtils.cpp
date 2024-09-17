#include <raylib/raylib.h>

#include <magique/util/RayUtils.h>

namespace magique
{
    float MeasureTextUpTo(char* text, const int index, const Font& f, const float fontSize, const float spacing)
    {
        const auto temp = text[index];
        text[index] = '\0';
        const float ret = MeasureTextEx(f, text, fontSize, spacing).x;
        text[index] = temp;
        return ret;
    }

    float GetRandomFloat(const float min, const float max)
    {
        constexpr float ACCURACY = 100'000.0F;
        const int minI = static_cast<int>(min * ACCURACY);
        const int maxI = static_cast<int>(max * ACCURACY);
        const auto val = static_cast<float>(GetRandomValue(minI, maxI));
        return val / ACCURACY;
    }

} // namespace magique