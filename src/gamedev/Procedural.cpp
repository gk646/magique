// SPDX-License-Identifier: zlib-acknowledgement
#include <ctime>

#include <magique/gamedev/Procedural.h>

#include "external/fastnoise/FastNoiseLite.h"

namespace magique
{
    FastNoiseLite NOISE_GEN{static_cast<int>(time(nullptr))};

    float GenerateNoise(const float x, const float y) { return NOISE_GEN.GetNoise(x, y); }

    void SetNoiseType(NoiseType noise) { NOISE_GEN.SetNoiseType(static_cast<FastNoiseLite::NoiseType>(noise)); }

    void SetSeed(const int seed) { NOISE_GEN.SetSeed(seed); }

} // namespace magique