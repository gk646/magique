#ifndef LIGHTINGSYSTEM_H
#define LIGHTINGSYSTEM_H

#include "raymath.h"
#include "external/raylib/src/external/glad.h"
#include "graphics/OpenGLUtil.h"

namespace magique
{
    inline void RenderHardShadows(entt::registry& registry)
    {
        ClearBackground(BLACK);
        rlDrawRenderBatchActive(); // Draw and Flush rest

        auto& shadowShader = SHADERS.shadow;
        auto& lightShader = SHADERS.light;
        auto& shadowQuads = LOGIC_TICK_DATA.shadowQuads;
        shadowQuads.clear();

        const auto occluders = registry.view<const PositionC, const OccluderC>();

        for (const auto e : occluders)
        {
            const auto& occ = occluders.get<OccluderC>(e);
            const auto& pos = occluders.get<PositionC>(e);
            switch (occ.shape)
            {
            case CIRCLE:
                break;
            case RECT:
                CreateQuadsFromRect(shadowQuads, {pos.x, pos.y, (float)occ.width, (float)occ.height});
            case POLYGON:
                break;
            }
        }
        int size = static_cast<int>(shadowQuads.size());


        int lightLightLoc = GetShaderLocation(lightShader, "lightPos");
        int shadowLightLoc = GetShaderLocation(shadowShader, "lightPosition");
        int mvpLoc = GetShaderLocation(shadowShader, "mvp");

        unsigned int vao, vbo;
        createSimpleObjectBuffer(shadowQuads.data(), size, &vao, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        const Vector2 lighe= {0, 0};

        rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
        BeginBlendMode(BLEND_CUSTOM_SEPARATE);
        BeginShaderMode(lightShader);
        {
            SetShaderValue(lightShader, lightLightLoc, &lighe, SHADER_UNIFORM_VEC2);
            DrawRectangle(0, 0, 1920, 1080, RED);
        }
        EndShaderMode();
        EndBlendMode();

        Matrix mvp = GetCameraMatrix2D(LOGIC_TICK_DATA.camera);
        SetShaderValueMatrix(shadowShader, mvpLoc, mvp);

        const auto lights = registry.view<const PositionC, const EmitterC>();
        for (const auto e : lights)
        {
            const auto& pos = lights.get<PositionC>(e);
            const Vector2 light = {pos.x, pos.y};

            rlEnableShader(shadowShader.id);
            {
                glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
                SetShaderValue(shadowShader, shadowLightLoc, &light, SHADER_UNIFORM_VEC2);
                SetShaderValueMatrix(shadowShader, mvpLoc, mvp);

                rlEnableVertexArray(vao);
                rlDrawVertexArray(0, shadowQuads.size());
                rlDisableVertexArray();
            }
            rlDisableShader();

            SetShaderValue(lightShader, lightLightLoc, &light, SHADER_UNIFORM_VEC2);
            rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
            BeginBlendMode(BLEND_CUSTOM_SEPARATE);
            BeginShaderMode(lightShader);
            {
                DrawRectangle(0, 0, 1920, 1080, RED);
            }
            EndShaderMode();
            EndBlendMode();
        }


        rlUnloadVertexArray(vao);
        rlUnloadVertexBuffer(vbo);
    }

    inline void RenderLighting(entt::registry& registry) { RenderHardShadows(registry); }


} // namespace magique


#endif //LIGHTINGSYSTEM_H