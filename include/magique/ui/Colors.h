#ifndef COLORS_H
#define COLORS_H

#include <raylib/raylib.h>

namespace magique
{
    namespace colors
    {
        inline constexpr Color LightGrey = {192, 203, 220, 255};
        inline constexpr Color LightGreyAlpha = {192, 203, 220, 190};
        inline constexpr Color LightGreyTransparent = {150, 150, 150, 77};
        inline constexpr Color MediumGrey = {139, 155, 180, 255};
        inline constexpr Color MediumLightGrey = {155, 169, 194, 255};
        inline constexpr Color MediumLightGreyAlpha = {155, 169, 194, 255};
        inline constexpr Color DarkerLightGrey = {135, 151, 176, 255};
        inline constexpr Color DarkGrey = {90, 105, 136, 255};
        inline constexpr Color DarkGreyAlpha = {90, 105, 136, 170};
        inline constexpr Color DarkDarkGrey = {58, 68, 102, 255};
    } // namespace colors
} // namespace magique

#endif //COLORS_H