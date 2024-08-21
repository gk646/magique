#ifndef MAGIQUE_PROCEDURAL_H
#define MAGIQUE_PROCEDURAL_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Procedural module
//-----------------------------------------------
// .....................................................................
// Generates different noises -> Uses FastNoiseLite
// .....................................................................

namespace magique
{
    //----------------- NOISE -----------------//

    // Returns the noise value at this position
    float GenerateNoise(float x, float y);

    // Changes the generated noise type
    // Effective immediately
    void SetNoiseType(NoiseType noise);

    // Sets a seed - is initialized randomly per default - can be set to deterministically generate noise
    // Effective immediately
    void SetSeed(int seed);




} // namespace magique

#endif //MAGIQUE_PROCEDURAL_H