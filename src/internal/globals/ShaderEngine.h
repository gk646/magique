#ifndef SHADERS_H
#define SHADERS_H

#include <raylib/raylib.h>
#include <raylib/rlgl.h>

namespace magique
{
    struct Shaders final
    {
        vector<Vector3> shadowQuads; // Shadow segments

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

        void init(size_t initialSize = 1024 * sizeof(Vector3))
        {
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
        }

        void updateObjectBuffer(const Vector3* vertices, const int vertexCount)
        {
            size_t requiredSize = vertexCount * sizeof(Vector3);

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