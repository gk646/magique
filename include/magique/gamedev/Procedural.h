// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PROCEDURAL_H
#define MAGIQUE_PROCEDURAL_H

#include <magique/core/Types.h>

//===============================================
// Procedural module
//===============================================
// .....................................................................
// Generates different noises -> Uses FastNoiseLite
// .....................................................................

namespace magique
{
    //================= NOISE =================//

    // Returns the noise value [-1, 1] at the given position
    float ProceduralGet(Point pos);

    // Changes the generated noise type
    // Effective immediately
    void ProceduralSetType(NoiseType noise);

    // Sets a seed - is initialized randomly per default - can be set to deterministically generate noise
    // Effective immediately
    void ProceduralSetSeed(int seed);

} // namespace magique

#endif //MAGIQUE_PROCEDURAL_H