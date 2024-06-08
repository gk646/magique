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
        int shadowLightLoc = shaders.shadowLightLoc;
        int mvpLoc = shaders.mvpLoc;

        unsigned int vao, vbo;
        createSimpleObjectBuffer(shadowQuads.data(), size, &vao, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        const Rectangle backRect = GetCameraBounds();

        Matrix projectionMatrix =
            MatrixOrtho(0.0f, CORE.Window.currentFbo.width, CORE.Window.currentFbo.height, 0.0f, 0.0f, 1.0f);
        Matrix matMVP = MatrixMultiply(GetCameraMatrix2D(global::DRAW_TICK_DATA.camera), projectionMatrix);

        float matMVPfloat[16] = {matMVP.m0,  matMVP.m1,  matMVP.m2,  matMVP.m3, matMVP.m4,  matMVP.m5,
                                 matMVP.m6,  matMVP.m7,  matMVP.m8,  matMVP.m9, matMVP.m10, matMVP.m11,
                                 matMVP.m12, matMVP.m13, matMVP.m14, matMVP.m15};

        glUseProgram(shadowShader.id);
        glUniformMatrix4fv(mvpLoc, 1, false, matMVPfloat);
        glUseProgram(0);

        const auto lights = registry.view<const PositionC, const EmitterC>();

        const Vector2 lighe = {0, 0};
        BeginBlendMode(BLEND_CUSTOM_SEPARATE);
        BeginShaderMode(lightShader);
        {
            SetShaderValue(lightShader, lightLightLoc, &lighe, SHADER_UNIFORM_VEC2);
            DrawRectangleRec(backRect, BLANK);
        }
        EndShaderMode();
        EndBlendMode();

        for (const auto e : lights)
        {
            const auto& pos = lights.get<PositionC>(e);
            const auto& emit = lights.get<EmitterC>(e);
            const Vector2 light = {pos.x, pos.y};
            const Vector4 color = {(float)emit.r / 255.0F, (float)emit.g / 255.0F, (float)emit.b / 255.0F,
                                   (float)emit.a / 255.0F};

            glUseProgram(shadowShader.id);
            {
                glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
                SetShaderValue(shadowShader, shadowLightLoc, &light, SHADER_UNIFORM_VEC2);

                rlEnableVertexArray(vao);
                rlDrawVertexArray(0, size);
                rlDisableVertexArray();
            }
            glUseProgram(0);

            SetShaderValue(lightShader, lightLightLoc, &light, SHADER_UNIFORM_VEC2);
            rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
            BeginBlendMode(BLEND_CUSTOM_SEPARATE);
            BeginShaderMode(lightShader);
            SetShaderValueV(lightShader, lightColorLoc, &color, SHADER_UNIFORM_VEC4, 1);
            {
                DrawRectangleRec(backRect, BLANK);
            }
            EndShaderMode();
            EndBlendMode();
        }

        rlUnloadVertexArray(vao);
        rlUnloadVertexBuffer(vbo);
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
        for(const auto e : occluders)
        {
            const auto& pos = registry.get<const PositionC>(e);
            const auto& emit = registry.get<const EmitterC>(e);
        }
    }

    inline void RenderLighting(entt::registry& registry)
    {
        if (global::CONFIGURATION.lighting == LightingModel::STATIC_SHADOWS)
        {
            RenderHardShadows(registry);
        }
        else
        {
            RenderRayTracing(registry);
        }
    }
} // namespace magique


inline int asdf()
{
    InitWindow(1920, 1080, "light");
    auto rayShader = LoadShader(nullptr, "../ray.frag");
    int rayResLoc = GetShaderLocation(rayShader, "resolution");
    int rayTimeLoc = GetShaderLocation(rayShader, "iTime");

    Vector2 res{1024, 576};
    auto tex = LoadRenderTexture(res.x, res.y);
    int data = 0;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &data);
    printf("%d\n", data);
    while (!WindowShouldClose())
    {
        float time = GetTime();

        BeginTextureMode(tex);
        ClearBackground(BLUE);
        BeginShaderMode(rayShader);
        SetShaderValue(rayShader, rayResLoc, &res, RL_SHADER_UNIFORM_VEC2);
        SetShaderValue(rayShader, rayTimeLoc, &time, RL_SHADER_UNIFORM_FLOAT);
        DrawRectangle(0, 0, res.x, res.y, BLACK);
        EndShaderMode();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLUE);
        DrawTexturePro(tex.texture, {0, 0, (float)tex.texture.width, -(float)tex.texture.height}, {0, 0, 1920, 1080},
                       {0, 0}, 0.0f, WHITE);
        DrawFPS(25, 25);
        EndDrawing();
    }

    UnloadShader(rayShader);
    CloseWindow();
    return 0;
}
#endif //LIGHTINGSYSTEM_H