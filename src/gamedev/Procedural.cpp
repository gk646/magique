// SPDX-License-Identifier: zlib-acknowledgement
#include <ctime>

#include <magique/gamedev/Procedural.h>

#include "external/fastnoise/FastNoiseLite.h"

namespace magique
{
    FastNoiseLite NOISE_GEN{static_cast<int>(time(nullptr))};

    float ProceduralGet(const Point p) { return NOISE_GEN.GetNoise(p.x, p.y); }

    void ProceduralSetType(NoiseType noise) { NOISE_GEN.SetNoiseType(static_cast<FastNoiseLite::NoiseType>(noise)); }

    void ProceduralSetSeed(const int seed) { NOISE_GEN.SetSeed(seed); }

} // namespace magique
