#ifndef LIGHTINGSYSTEM_H
#define LIGHTINGSYSTEM_H

#include <raylib/raymath.h>

#include "internal/utils/OpenGLUtil.h"

namespace magique
{
    inline void RenderHardShadows(const entt::registry& registry)
    {
        auto& shader = global::SHADERS;
        auto& shadowShader = shader.shadow;
        auto& lightShader = shader.light;
        auto& shadowQuads = shader.shadowQuads;
        shadowQuads.clear();

        //TODO add missing shapes
        const auto occluders = registry.view<const PositionC, const OccluderC>();
        for (const auto e : occluders)
        {
            const auto& occ = occluders.get<OccluderC>(e);
            const auto& pos = occluders.get<PositionC>(e);
            switch (occ.shape)
            {
            case Shape::CIRCLE:
                break;
            case Shape::RECT:
                CreateQuadsFromRect(shadowQuads, {pos.x, pos.y, (float)occ.width, (float)occ.height});
                break;
            case Shape::TRIANGLE:
                break;
            case Shape::CAPSULE:
                break;
            }
        }

        const int size = shadowQuads.size();
        const int lightLightLoc = shader.lightLightLoc;
        const int lightColorLoc = shader.lightColorLoc;
        const int lightTypeLoc = shader.lightTypeLoc;
        const int lightIntensityLoc = shader.lightIntensityLoc;

        const int shadowLightLoc = shader.shadowLightLoc;
        const int mvpLoc = shader.mvpLoc;

        if (size > 0)
        {
            shader.updateObjectBuffer(shadowQuads.data(), size);
        }

        const Rectangle drawRect = {0, 0, shader.shadowResolution.x, shader.shadowResolution.y};
        Matrix projectionMatrix =
            MatrixOrtho(0.0f, CORE.Window.currentFbo.width, CORE.Window.currentFbo.height, 0.0f, 0.0f, 1.0f);
        Matrix matMVP = MatrixMultiply(GetCameraMatrix2D(global::ENGINE_DATA.camera), projectionMatrix);

        float matMVPfloat[16] = {matMVP.m0,  matMVP.m1,  matMVP.m2,  matMVP.m3, matMVP.m4,  matMVP.m5,
                                 matMVP.m6,  matMVP.m7,  matMVP.m8,  matMVP.m9, matMVP.m10, matMVP.m11,
                                 matMVP.m12, matMVP.m13, matMVP.m14, matMVP.m15};

        glUseProgram(shadowShader.id);
        glUniformMatrix4fv(mvpLoc, 1, false, matMVPfloat);
        glUseProgram(0);

        BeginTextureMode(shader.shadowTexture);
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
                    rlEnableVertexArray(shader.vao);
                    rlDrawVertexArray(0, size);
                    rlDisableVertexArray();
                }
                glUseProgram(0);

                rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
                BeginBlendMode(BLEND_CUSTOM_SEPARATE);
                {
                    BeginShaderMode(lightShader);
                    {
                        const auto screnPos = GetWorldToScreen2D(light, GetCamera());
                        SetShaderValue(lightShader, lightLightLoc, &screnPos, SHADER_UNIFORM_VEC2);
                        SetShaderValueV(lightShader, lightColorLoc, &color, SHADER_UNIFORM_VEC4, 1);
                        SetShaderValueV(lightShader, lightTypeLoc, &style, SHADER_UNIFORM_INT, 1);
                        SetShaderValueV(lightShader, lightIntensityLoc, &intensity, SHADER_UNIFORM_INT, 1);
                        DrawRectangleRec(drawRect, BLANK);
                    }
                    EndShaderMode();
                }
                EndBlendMode();
            }
        }
        EndTextureMode();

        BeginShaderMode(shader.texture);
        {
            rlSetBlendFactors(GL_ZERO, GL_SRC_COLOR, GL_FUNC_ADD);
            BeginBlendMode(BLEND_CUSTOM);

            const auto srcRect = Rectangle{0, 0, drawRect.width, -drawRect.height};
            const auto origin = Vector2{0, 0};
            DrawTexturePro(shader.shadowTexture.texture, srcRect, drawRect, origin, 0, ColorAlpha(WHITE, 0.8));

            EndBlendMode();
        }
        EndShaderMode();
    }

    inline void RenderRayTracing(const entt::registry& registry)
    {
        // Lights
        Vector2 lightPositions[MAGIQUE_RAYTRACING_ENTITIES];
        Vector3 lightColors[MAGIQUE_RAYTRACING_ENTITIES];
        int lightStyle[MAGIQUE_RAYTRACING_ENTITIES];

        // Occluders
        Vector2 occPositions[MAGIQUE_RAYTRACING_ENTITIES];
        Vector2 occDimensions[MAGIQUE_RAYTRACING_ENTITIES];
        Vector2 occShape[MAGIQUE_RAYTRACING_ENTITIES];

        int count = 0;
        {
            const auto view = registry.view<const PositionC, const EmitterC>();
            for (const auto e : view)
            {
                const auto& pos = view.get<const PositionC>(e);
                const auto& emit = view.get<const EmitterC>(e);
                lightPositions[count] = {pos.x, pos.y};
                lightColors[count] = Vector3{(float)emit.r, (float)emit.g, (float)emit.b};
                count++;
            }
        }

        const auto view = registry.view<const PositionC, const OccluderC>();
        count = 0;
        for (const auto e : view)
        {
            const auto& pos = view.get<const PositionC>(e);
            const auto& emit = view.get<const OccluderC>(e);
        }
    }

    inline void RenderLighting(const entt::registry& registry)
    {
        const auto model = global::ENGINE_CONFIG.lighting;
        if (model == LightingMode::STATIC_SHADOWS)
        {
            RenderHardShadows(registry);
        }
        else if (model == LightingMode::RAY_TRACING)
        {
            RenderRayTracing(registry);
        }
    }
} // namespace magique


#endif //LIGHTINGSYSTEM_H