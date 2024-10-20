#ifndef MAGIQUE_SHADERS_H
#define MAGIQUE_SHADERS_H

#include <raylib/raylib.h>
#include <raylib/rlgl.h>

#include "internal/headers/Shaders.h"

namespace magique
{
    struct Shaders final
    {
        vector<Vector3> shadowQuads; // Shadow segments
        Vector2 shadowResolution{};  //Shadow map resolution

        Shader shadow{};
        Shader light{};
        Shader texture{};

        Shader raytracing{};

        // shader locations
        int lightLightLoc = 0;
        int lightColorLoc = 0;
        int lightTypeLoc = 0;
        int lightIntensityLoc = 0;

        int shadowLightLoc = 0;
        int mvpLoc = 0;

        RenderTexture shadowTexture{};

        unsigned int vao = 0, vbo = 0;
        int currentSize = 0; // Current size of the vertex buffer in bytes

        void init(const int vertices = 1024)
        {
            shadowResolution = {static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
            shadowQuads.reserve(vertices);
            shadowTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

            currentSize = vertices * static_cast<int>(sizeof(Vector3));

            vao = rlLoadVertexArray();
            rlEnableVertexArray(vao);

            vbo = rlLoadVertexBuffer(nullptr, currentSize, true);
            rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
            rlEnableVertexAttribute(0);

            rlDisableVertexArray(); // Has to be after loading the vbo

            // Load shaders
            light = LoadShaderFromMemory(lightVert, lightFrag);
            shadow = LoadShaderFromMemory(shadowVert, shadowFrag);
            texture = LoadShaderFromMemory(shadowTextureVert, shadowTextureFrag);

            //raytracing = LoadShaderFromMemory(rayVert, rayFrag);

            lightLightLoc = GetShaderLocation(light, "lightPos");
            lightColorLoc = GetShaderLocation(light, "lightColor");
            lightTypeLoc = GetShaderLocation(light, "lightType");
            lightIntensityLoc = GetShaderLocation(light, "intensity");
            shadowLightLoc = GetShaderLocation(shadow, "lightPosition");
            mvpLoc = GetShaderLocation(shadow, "mvp");
        }

        void updateObjectBuffer(const Vector3* vertices, const int vertexCount)
        {
            const int requiredSize = vertexCount * static_cast<int>(sizeof(Vector3));
            if (requiredSize > currentSize)
            {
                currentSize = requiredSize;
                rlUnloadVertexBuffer(vbo);
                vbo = rlLoadVertexBuffer(nullptr, currentSize, true);

                rlEnableVertexArray(vao);
                rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
                rlEnableVertexAttribute(0);
                rlDisableVertexArray();
            }
            rlEnableVertexArray(vao);
            rlUpdateVertexBuffer(vbo, vertices, requiredSize, 0);
            rlDisableVertexArray();
        }
    };

    namespace global
    {
        inline Shaders SHADERS;
    }
} // namespace magique

#endif //MAGIQUE_SHADERS_H