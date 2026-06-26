#include <magique/core/Lighting.h>
#include <magique/gamedev/PathFinding.h>
#include <magique/core/Camera.h>
#include <magique/ecs/ECS.h>
#include <raylib/rlgl.h> // Needs to be here

#include "external/raylib-compat/rcore_compat.h"

namespace magique
{
    std::vector<Entity> entities{}; // Needed as stage need same order of entities
    Shader tracerShader{};
    Shader samplerShader{};
    RenderTexture rayTexture{};

    RenderTexture occlusionTex{};
    uint32_t lastTexUpdateTick = 0;

    int tracerOcclusionTexSlotLoc;
    int tracerOcclusionDimsLoc;
    int tracerLightPosLoc;

    int samplerIndexLoc;
    int samplerLightLoc;
    int samplerRayTexLoc;
    int samplerOcclusionTexLoc;
    int samplerLightTexSizeLoc;

    constexpr float RAY_TEX_SIZE = 64;
    constexpr int MAX_LIGHTS = MAGIQUE_MAX_TEXTURE_SIZE / RAY_TEX_SIZE;
    constexpr int RAY_TEX_SLOT = 4;
    constexpr int OCCLUSION_TEX_SLOT = 5;

    // Note: This can be further optimized to reduce draw calls by introduction of a lightInfoTexture
    // Then both shaders operator on this texture where they get light info from => 2 draw calls
    void LightingDrawRaytracing(RenderTexture texture)
    {
        const auto& camera = CameraGet();
        (void)LightingGetScreenOcclusion(); // call to build it
        const Point occlusionDims = {(float)occlusionTex.texture.width, (float)occlusionTex.texture.height};

        rlActiveTextureSlot(OCCLUSION_TEX_SLOT);
        rlEnableTexture(occlusionTex.texture.id);

        // Tracer Shader Locs - to set uniforms first activate a slot, enable a texture and set the slot id for the shader
        SetShaderValue(tracerShader, tracerOcclusionTexSlotLoc, &OCCLUSION_TEX_SLOT, SHADER_UNIFORM_SAMPLER2D);
        SetShaderValue(tracerShader, tracerOcclusionDimsLoc, &occlusionDims, SHADER_UNIFORM_VEC2);

        // Light Shader locs
        SetShaderValue(samplerShader, samplerLightTexSizeLoc, &occlusionDims, SHADER_UNIFORM_VEC2);
        SetShaderValue(samplerShader, samplerOcclusionTexLoc, &OCCLUSION_TEX_SLOT, SHADER_UNIFORM_SAMPLER2D);
        SetShaderValue(samplerShader, samplerRayTexLoc, &RAY_TEX_SLOT, SHADER_UNIFORM_SAMPLER2D);

        entities.clear();
        for (const auto e : EngineGetDrawEntities())
        {
            if (entities.size() >= MAX_LIGHTS) [[unlikely]]
                break;
            if (EntityHasAll<LightingC>(e)) [[unlikely]]
                entities.push_back(e);
        }


        // Phase 1 - draws all ray lengths of each light below each other
        {
            BeginTextureMode(rayTexture);
            ClearBackground(BLUE);
            BeginShaderMode(tracerShader);

            float lightIndex = 0;
            for (const auto e : entities)
            {
                const auto& light = ComponentGet<LightingC>(e);
                Point mid = GetWorldToScreen2D(CollisionC::GetMiddle(e), camera);
                mid += Point::Random(-light.randRangePosition, light.randRangePosition);
                const float lightData[3] = {mid.x, mid.y, light.radius};

                SetShaderValue(tracerShader, tracerLightPosLoc, lightData, SHADER_UNIFORM_VEC3);
                DrawRectangleRec(Rect{{0, lightIndex * RAY_TEX_SIZE}, {RAY_TEX_SIZE}}, WHITE);

                // Commit the draw call so that the shader is executed with the current uniform values
                rlDrawRenderBatchActive();
                lightIndex++;
            }
            EndShaderMode();
            EndTextureMode();
        }

        if (texture.texture.id != 0)
            BeginTextureMode(texture);

        // Phase 2 - use all the length information to draw the light value at each pixel
        {
            BeginShaderMode(samplerShader);
            BeginBlendMode(BLEND_ADDITIVE);

            float lightIndex = 1;
            for (const auto e : entities)
            {
                const auto& light = ComponentGet<LightingC>(e);
                Point mid = GetWorldToScreen2D(CollisionC::GetMiddle(e), camera);
                mid += Point::Random(-light.randRangePosition, light.randRangePosition);
                const float lightData[3] = {mid.x, mid.y, light.radius};

                SetShaderValue(samplerShader, samplerLightLoc, lightData, SHADER_UNIFORM_VEC3);
                SetShaderValue(samplerShader, samplerIndexLoc, &lightIndex, SHADER_UNIFORM_FLOAT);
                DrawRectangleRec(Rect{occlusionDims.x, occlusionDims.y}, light.color);

                rlDrawRenderBatchActive();
                lightIndex++;
            }

            EndBlendMode();
            EndShaderMode();
        }
    }

    Texture2D LightingGetScreenOcclusion()
    {
        if (EngineGetTicks() == lastTexUpdateTick)
            return occlusionTex.texture;
        lastTexUpdateTick = EngineGetTicks();

        const auto cam = CameraGetNativeBounds();
        if (occlusionTex.texture.width != cam.width || occlusionTex.texture.height != cam.height)
        {
            UnloadRenderTexture(occlusionTex);
            occlusionTex = LoadRenderTexture(cam.width, cam.height);
        }

        const auto prevRenderTarget = GetCurrentRenderTexture();

        EndTextureMode();
        BeginTextureMode(occlusionTex);
        ClearBackground(BLANK);

        const auto map = CameraGetMap();
        const auto bounds = CameraGetNativeBounds();
        constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
        const int startX = static_cast<int>(bounds.x) / cellSize;
        const int startY = static_cast<int>(bounds.y) / cellSize;
        const int width = static_cast<int>(bounds.width) / cellSize;
        const int height = static_cast<int>(bounds.height) / cellSize + 1;

        for (int currY = startY; currY < startY + height; ++currY)
        {
            for (int currX = startX; currX < startX + width; ++currX)
            {
                // Normalized to the screen - as its only screen texture not world space
                Point curr = Point{(float)currX, (float)currY} * cellSize;
                const bool isSolid = PathIsSolid(curr, map);
                if (isSolid) [[unlikely]]
                    DrawRectangleRec(
                        Rect{{curr.x - bounds.x, occlusionTex.texture.height - (curr.y - bounds.y) - cellSize},
                             cellSize},
                        BLACK);
            }
        }

        EndTextureMode();

        if (prevRenderTarget.texture.id != 0)
            BeginTextureMode(prevRenderTarget);

        return occlusionTex.texture;
    }

    void internal::LightingInit()
    {
        tracerShader = LoadShaderFromMemory(nullptr, R"(
#version 330

uniform sampler2D occlusionTex;     // Occlusion tex a > 0 for obstacle

uniform vec3 lightInfo;             //  x = x, y = y, z = radius
uniform vec2 occlusionTexSize;      // Size of the occlusion map

in vec2 fragTexCoord;
out vec4 finalColor;

const float MAX_LIGHTS = 64;
const float RAY_TEX_SIZE = 64;
const float TAU = 6.2831853071795864769252867665590;

void main() {
    vec2 mirroredCoords = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);
    float rayCount = floor(TAU * lightInfo.z);
    vec2 coord = floor(mirroredCoords * RAY_TEX_SIZE);

    float index = (coord.y * RAY_TEX_SIZE) + coord.x;
    float Theta = TAU * (index / rayCount);
    vec2 Delta = vec2(cos(Theta), -sin(Theta));

    if (index >= rayCount) {
        finalColor = vec4(0., 1., 0.0, 1.);
        return;
    }

    float rayLength = lightInfo.z;
    for (float d = 0.0; d <= lightInfo.z; d += 1.0) {
        vec2 worldPos = lightInfo.xy + Delta * d;
        vec2 uv = worldPos / occlusionTexSize;

        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
            rayLength = d;
            break;
        }

        float collision = texture(occlusionTex, uv).a;
        if (collision > 0.0) {
            rayLength = d;
            break;
        }
    }

    // Store normalized ray length in red channel
    finalColor = vec4(rayLength / lightInfo.z, 0.0, 0.0, 1.0);
}
)");

        samplerShader = LoadShaderFromMemory(nullptr, R"(
#version 330

uniform sampler2D occlusionTex;      // Collision map (world coordinates)
uniform sampler2D rayTex;            // Ray texture (output from tracer)

uniform vec3 lightInfo;         // x = x, y = y, z = radius
uniform vec2 lightTexSize;   // Size of the light texture (where we draw to)
uniform float lightIndex;       // Offset in the raytexture
//uniform vec2 worldTexSize;      // Size of the collision map

in vec4 fragColor;
in vec2 fragTexCoord;
out vec4 finalColor;

const float MAX_LIGHTS = 64;
const float RAY_TEX_SIZE = 64;
const float TAU = 6.2831853071795864769252867665590;

float ToneMapFunc(float d, float m) {
    return clamp(1.0 - (d / m), 0.0, 1.0);
}

void main() {
    float rayCount = floor(TAU * lightInfo.z);
    vec2 coord = fragTexCoord * lightTexSize;
    vec2 Delta = round(coord - lightInfo.xy);

    float angle = atan(-Delta.y, Delta.x);
    float RayIndex = floor(rayCount * fract(angle / TAU));

    vec2 RayPos = vec2(
    mod(RayIndex, RAY_TEX_SIZE) / RAY_TEX_SIZE,
    (floor(RayIndex / RAY_TEX_SIZE) + (MAX_LIGHTS - lightIndex) * RAY_TEX_SIZE) / (RAY_TEX_SIZE * MAX_LIGHTS)
    );

    // Here we can use RayPos directly cause we flipped the ray texture
    // This avoids that we start lookup at a row thats potentially not fully filled (the last one)
    vec2 TexRay = texture(rayTex, RayPos).rg;
    float RayLength = TexRay.r * lightInfo.z;
    float Distance = length(Delta);

    // vec2 collisionUV = (in_Light.xy + Delta) / worldTexSize;
    // collisionUV.y = 1.0 - collisionUV.y;
    // float collision = texture(inColTex, collisionUV).a;

    float RayVisible = step(Distance, RayLength);// * 1.0 - collision;
    float ToneMap = ToneMapFunc(Distance, lightInfo.z);

    finalColor = vec4(fragColor.xyz * ToneMap, ToneMap * RayVisible * fragColor.a);
}
)");

        rayTexture = LoadRenderTexture(RAY_TEX_SIZE, RAY_TEX_SIZE * MAX_LIGHTS);
        rlActiveTextureSlot(RAY_TEX_SLOT);
        rlEnableTexture(rayTexture.texture.id);

        // Locs
        tracerOcclusionTexSlotLoc = GetShaderLocation(tracerShader, "occlusionTex");
        tracerOcclusionDimsLoc = GetShaderLocation(tracerShader, "occlusionTexSize");
        tracerLightPosLoc = GetShaderLocation(tracerShader, "lightInfo");

        samplerRayTexLoc = GetShaderLocation(samplerShader, "rayTex");
        samplerOcclusionTexLoc = GetShaderLocation(samplerShader, "occlusionTex");
        samplerLightLoc = GetShaderLocation(samplerShader, "lightInfo");
        samplerLightTexSizeLoc = GetShaderLocation(samplerShader, "lightTexSize");
        samplerIndexLoc = GetShaderLocation(samplerShader, "lightIndex");
    }

} // namespace magique
