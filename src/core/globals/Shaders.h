#ifndef SHADERS_H
#define SHADERS_H

#include <raylib/raylib.h>

namespace magique{

    struct Shaders final
    {
        Shader shadow;
        Shader light;
        Shader raytracing;

        int lightLightLoc;
        int lightColorLoc;
        int shadowLightLoc;
        int mvpLoc;
    };

    namespace global{
        inline Shaders SHADERS;
    }
}

#endif //SHADERS_H