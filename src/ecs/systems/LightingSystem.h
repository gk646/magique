#ifndef LIGHTINGSYSTEM_H
#define LIGHTINGSYSTEM_H

#include "raymath.h"
#include "external/raylib/src/external/glad.h"
#include "graphics/OpenGLUtil.h"

namespace magique
{
    inline void RenderHardShadows(entt::registry& registry)
    {

        auto& shaders = global::SHADERS;
        auto& shadowShader = shaders.shadow;
        auto& lightShader = shaders.light;
        auto& shadowQuads = global::LOGIC_TICK_DATA.shadowQuads;
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
        int lightLightLoc = shaders.lightLightLoc;
        int lightColorLoc = shaders.lightColorLoc;
        int lightTypeLoc = shaders.lightTypeLoc;
        int lightIntensityLoc = shaders.lightIntensityLoc;

        int shadowLightLoc = shaders.shadowLightLoc;
        int mvpLoc = shaders.mvpLoc;

        if (size > 0)
        {
            shaders.updateObjectBuffer(shadowQuads.data(), size);
        }
        printf("%d\n", size);

        const Rectangle backRect = GetCameraNativeBounds();

        Matrix projectionMatrix =
            MatrixOrtho(0.0f, CORE.Window.currentFbo.width, CORE.Window.currentFbo.height, 0.0f, 0.0f, 1.0f);
        Matrix matMVP = MatrixMultiply(GetCameraMatrix2D(global::DRAW_TICK_DATA.camera), projectionMatrix);

        float matMVPfloat[16] = {matMVP.m0,  matMVP.m1,  matMVP.m2,  matMVP.m3, matMVP.m4,  matMVP.m5,
                                 matMVP.m6,  matMVP.m7,  matMVP.m8,  matMVP.m9, matMVP.m10, matMVP.m11,
                                 matMVP.m12, matMVP.m13, matMVP.m14, matMVP.m15};

        glUseProgram(shadowShader.id);
        glUniformMatrix4fv(mvpLoc, 1, false, matMVPfloat);
        glUseProgram(0);

        BeginTextureMode(shaders.shadowTexture);
        {
            ClearBackground({0, 0, 0, 0});

            const auto lights = registry.view<const PositionC, const EmitterC>();
            for (const auto e : lights)
            {
                const auto& pos = lights.get<PositionC>(e);
                const auto& emit = lights.get<EmitterC>(e);
                int intensity = emit.intensity;
                int style = emit.style;
                const Vector2 light = {pos.x, pos.y};
                const Vector4 color = {(float)emit.r / 255.0F, (float)emit.g / 255.0F, (float)emit.b / 255.0F,
                                       (float)emit.a / 255.0F};

                glUseProgram(shadowShader.id);
                {
                    glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
                    SetShaderValue(shadowShader, shadowLightLoc, &light, SHADER_UNIFORM_VEC2);
                    rlEnableVertexArray(shaders.vao);
                    rlDrawVertexArray(0, size);
                    rlDisableVertexArray();
                }
                glUseProgram(0);

                rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
                BeginBlendMode(BLEND_CUSTOM_SEPARATE);
                {
                    BeginShaderMode(lightShader);
                    {
                        SetShaderValue(lightShader, lightLightLoc, &light, SHADER_UNIFORM_VEC2);
                        SetShaderValueV(lightShader, lightColorLoc, &color, SHADER_UNIFORM_VEC4, 1);
                        SetShaderValueV(lightShader, lightTypeLoc, &style, SHADER_UNIFORM_INT, 1);
                        SetShaderValueV(lightShader, lightIntensityLoc, &intensity, SHADER_UNIFORM_INT, 1);
                        DrawRectangleRec(backRect, BLANK);
                    }
                    EndShaderMode();
                }
                EndBlendMode();
            }
        }
        EndTextureMode();


        rlSetBlendFactors(GL_ZERO, GL_SRC_COLOR, GL_FUNC_ADD);
        BeginBlendMode(BLEND_CUSTOM);

        DrawTexturePro(shaders.shadowTexture.texture,
                       {0, 0, 1920, -1080},
                       {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                       {0,0}, 0, {255, 255, 255, 200});

        EndBlendMode();
    }

    inline void RenderRayTracing(entt::registry& registry)
    {
        // Lights
        Vector2 lightPositions[MAGIQUE_MAX_RAYTRACING_ENTITIES];
        Vector3 lightColors[MAGIQUE_MAX_RAYTRACING_ENTITIES];
        int lightStyle[MAGIQUE_MAX_RAYTRACING_ENTITIES];

        // Occluders
        Vector2 occPositions[MAGIQUE_MAX_RAYTRACING_ENTITIES];
        Vector2 occDimensions[MAGIQUE_MAX_RAYTRACING_ENTITIES];
        Vector2 occShape[MAGIQUE_MAX_RAYTRACING_ENTITIES];

        const auto lights = registry.view<const PositionC, const EmitterC>();
        int count = 0;
        for (const auto e : lights)
        {
            const auto& pos = registry.get<const PositionC>(e);
            const auto& emit = registry.get<const EmitterC>(e);
            lightPositions[count] = {pos.x, pos.y};
            lightColors[count] = Vector3{(float)emit.r, (float)emit.g, (float)emit.b};
            count++;
        }

        const auto occluders = registry.view<const PositionC, const OccluderC>();
        count = 0;
        for (const auto e : occluders)
        {
            const auto& pos = registry.get<const PositionC>(e);
            const auto& emit = registry.get<const EmitterC>(e);
        }
    }

    inline void RenderLighting(entt::registry& registry)
    {
        const auto model = global::CONFIGURATION.lighting;
        if (model == LightingModel::STATIC_SHADOWS)
        {
            RenderHardShadows(registry);
        }
        else if (model == LightingModel::RAY_TRACING)
        {
            RenderRayTracing(registry);
        }
    }
} // namespace magique



#endif //LIGHTINGSYSTEM_H