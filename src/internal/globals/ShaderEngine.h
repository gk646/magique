#ifndef SHADERS_H
#define SHADERS_H

#include <raylib/raylib.h>
#include <raylib/rlgl.h>

#include "internal/headers/Shaders.h"

namespace magique
{
    struct Shaders final
    {
        vector<Vector3> shadowQuads; // Shadow segments
        Vector2 shadowResolution{};  //Shadow map resolution

        Shader shadow;
        Shader light;
        Shader raytracing;

        // shader locations
        int lightLightLoc;
        int lightColorLoc;
        int lightTypeLoc;
        int lightIntensityLoc;

        int shadowLightLoc;
        int mvpLoc;

        RenderTexture shadowTexture;

        unsigned int vao, vbo;
        unsigned int currentSize;

        void init(const size_t initialSize = 1024 * sizeof(Vector3))
        {
            shadowResolution = {1280, 960};
            shadowQuads.reserve(500);
            shadowTexture = LoadRenderTexture(1920, 1080);

            currentSize = initialSize;

            // Generate and bind a Vertex Array Object
            vao = rlLoadVertexArray();
            rlEnableVertexArray(vao);

            // Allocate the initial size for the Vertex Buffer Object
            vbo = rlLoadVertexBuffer(nullptr, currentSize, true); // true indicates dynamic usage
            rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
            rlEnableVertexAttribute(0);

            // Unbind the VAO
            rlDisableVertexArray();

            light = LoadShaderFromMemory(lightVert, lightFrag);
            shadow = LoadShaderFromMemory(shadowVert, shadowFrag);

            lightLightLoc = GetShaderLocation(light, "lightPos");
            lightColorLoc = GetShaderLocation(light, "lightColor");
            lightTypeLoc = GetShaderLocation(light, "lightType");
            lightIntensityLoc = GetShaderLocation(light, "intensity");

            shadowLightLoc = GetShaderLocation(shadow, "lightPosition");
            mvpLoc = GetShaderLocation(shadow, "mvp");
        }

        void updateObjectBuffer(const Vector3* vertices, const int vertexCount)
        {
            const size_t requiredSize = vertexCount * sizeof(Vector3);

            // Check if the current buffer size is sufficient
            if (requiredSize > currentSize)
            {
                // Reallocate buffer with larger size
                currentSize = requiredSize;
                rlUnloadVertexBuffer(vbo);
                vbo = rlLoadVertexBuffer(nullptr, currentSize, true);

                // Re-bind and re-enable vertex attribute
                rlEnableVertexArray(vao);
                rlSetVertexAttribute(0, 3, RL_FLOAT, false, 0, 0);
                rlEnableVertexAttribute(0);
                rlDisableVertexArray();
            }

            // Update the Vertex Buffer Object with the new data
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

#endif //SHADERS_H