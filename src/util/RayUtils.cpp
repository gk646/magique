// SPDX-License-Identifier: zlib-acknowledgement

#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/RayUtils.h>

namespace magique
{
    float MeasureTextUpTo(const char* text, const int index, const Font& f, const float fontSize, const float spacing)
    {
        char* nonConstText = const_cast<char*>(text);
        const auto temp = text[index];
        nonConstText[index] = '\0';
        const float ret = MeasureTextEx(f, text, fontSize, spacing).x;
        nonConstText[index] = temp;
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

    Vector2 GetCenteredPos(const Rectangle& within, const float width, const float height)
    {
        return Vector2{within.x + ((within.width - width) / 2), within.y + ((within.height - height) / 2)};
    }

    Vector2 GetRectCenter(const Rectangle& rect)
    {
        return Vector2{rect.x + rect.width / 2.0F, rect.y + rect.height / 2.0F};
    }

    Rectangle GetCenteredRect(const Point& center, const float width, const float height)
    {
        return Rectangle{center.x - width / 2.0F, center.y - height / 2.0F, width, height};
    }


} // namespace magique