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
                //glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
                SetShaderValue(shadowShader, shadowLightLoc, &light, SHADER_UNIFORM_VEC2);
                SetShaderValueMatrix(shadowShader, mvpLoc, mvp);

                rlEnableVertexArray(vao);
                rlDrawVertexArray(0, size);
                rlDisableVertexArray();
            }
            rlDisableShader();

            SetShaderValue(lightShader, lightLightLoc, &light, SHADER_UNIFORM_VEC2);
           // rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
           // BeginBlendMode(BLEND_CUSTOM_SEPARATE);
            BeginShaderMode(lightShader);
            {
                //DrawRectangle(0, 0, 1920, 1080, RED);
            }
            EndShaderMode();
           // EndBlendMode();
        }


        rlUnloadVertexArray(vao);
        rlUnloadVertexBuffer(vbo);
    }

    inline void RenderLighting(entt::registry& registry) { RenderHardShadows(registry); }


} // namespace magique

#include <external/glad.h>
#include <raylib.h>

#include "OpenGLThings.h"
#include "raymath.h"

int main()
{
    InitWindow(500, 500, "light");
    auto lightShader = LoadShader("../light.vert", "../light.frag");
    int lightLightLoc = GetShaderLocation(lightShader, "lightPos");

    auto shadowShader = LoadShader("../shadow.vert", "../shadow.frag");
    int shadowLightLoc = GetShaderLocation(shadowShader, "lightPosition");
    int mvpLoc = GetShaderLocation(shadowShader, "mvp");

    std::vector<Vector3> shadowQuads;
    shadowQuads.reserve(100);

    std::vector<Rectangle> objects = {
        {50, 50, 50, 50},
        {50, 150, 50, 200},
        {300, 200, 100, 100}};


    std::vector<Vector2> lights = {{}};

    while (!WindowShouldClose())
    {
        //printf("%d\n", lights.size());

        BeginDrawing();
        Camera2D camera{};
        camera.offset = {150,150};
        camera.zoom = 1.0F;
        BeginMode2D(camera);
        ClearBackground(BLACK);

        rlDrawRenderBatchActive(); // Draw and Flush rest

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            lights.push_back(GetMousePosition());
        }

        shadowQuads.clear();

        for (auto rect : objects)
        {
            CreateQuadsFromRect(shadowQuads, rect);
        }

        unsigned int vao, vbo;
        createSimpleObjectBuffer(shadowQuads.data(), shadowQuads.size(), &vao, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        float screenWidth = (float)GetScreenWidth();
        float screenHeight = (float)GetScreenHeight();

        Matrix projectionMatrix = MatrixOrtho(0.0f, screenWidth, screenHeight, 0.0f, 0.0f, 1.0f);
        Matrix matMVP =  MatrixMultiply( GetCameraMatrix2D(camera), projectionMatrix);

        float matMVPfloat[16] = {
            matMVP.m0, matMVP.m1, matMVP.m2, matMVP.m3,
            matMVP.m4, matMVP.m5, matMVP.m6, matMVP.m7,
            matMVP.m8, matMVP.m9, matMVP.m10, matMVP.m11,
            matMVP.m12, matMVP.m13, matMVP.m14, matMVP.m15
        };

        for (const auto light : lights)
        {
            // Set up blending for shadows
            glUseProgram(shadowShader.id);
            {
                glUniformMatrix4fv(mvpLoc, 1, false, matMVPfloat);
                glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO);
                SetShaderValue(shadowShader, shadowLightLoc, &light, SHADER_UNIFORM_VEC2);

                rlEnableVertexArray(vao);
                rlDrawVertexArray(0, shadowQuads.size());
                rlDisableVertexArray();
            }
            rlDisableShader();

            rlSetBlendFactorsSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD, GL_FUNC_ADD);
            BeginBlendMode(BLEND_CUSTOM_SEPARATE);
            BeginShaderMode(lightShader);
            {
                SetShaderValue(lightShader, lightLightLoc, &light, SHADER_UNIFORM_VEC2);
                DrawRectangle(0, 0, 500, 500, RED);
            }
            EndShaderMode();
            EndBlendMode();
        }

        rlUnloadVertexArray(vao);
        rlUnloadVertexBuffer(vbo);


        for (const auto rect : objects)
        {
            DrawRectangleRec(rect, BLUE);
        }

        EndMode2D();
        DrawFPS(25, 25);

        EndDrawing();
    }

    CloseWindow();
}

#include <cstdio>
#include <external/glad.h>


#include "raylib.h"
#include "rlgl.h"

int main()
{
    InitWindow(1920, 1080, "light");
    auto rayShader = LoadShader(nullptr, "../ray.frag");
    int rayResLoc = GetShaderLocation(rayShader, "resolution");
    int rayTimeLoc = GetShaderLocation(rayShader, "iTime");

    Vector2 res{1024, 576};
    auto tex = LoadRenderTexture(res.x, res.y);
    int data = 0;
    glGetIntegerv( GL_MAX_UNIFORM_BLOCK_SIZE,&data);
    printf("%d\n",data);
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
        DrawTexturePro(
            tex.texture,
           {0, 0, (float)tex.texture.width, -(float)tex.texture.height},
            {0, 0, 1920, 1080},
           {0, 0}, 0.0f, WHITE
        );
        DrawFPS(25, 25);
        EndDrawing();


    }

    UnloadShader(rayShader);
    CloseWindow();
    return 0;
}
#endif //LIGHTINGSYSTEM_H