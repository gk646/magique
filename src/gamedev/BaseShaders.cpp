#include <magique/gamedev/BaseShaders.h>

namespace magique
{
    VignetteShader::operator Shader() const { return shader; }

    VignetteShader& VignetteShader::setColor(Color color)
    {
        const auto normal = ColorNormalize(color);
        SetShaderValue(shader, colorLoc, &normal, SHADER_UNIFORM_VEC4);
        return *this;
    }

    VignetteShader& VignetteShader::setFalloff(float falloff)
    {
        SetShaderValue(shader, fallOffLoc, &falloff, SHADER_UNIFORM_FLOAT);
        return *this;
    }

    VignetteShader& VignetteShader::setSize(float size)
    {
        SetShaderValue(shader, sizeLoc, &size, SHADER_UNIFORM_FLOAT);
        return *this;
    }

    void VignetteShader::Init()
    {
        // https://www.shadertoy.com/view/lsKSWR - Created by Ippokratis in 2016-05-21
        shader = LoadShaderFromMemory(nullptr, R"(
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 vignetteColor = vec4(1., 0., 0., 1.);
uniform float vignetteSize = 0.50;
uniform float vignetteFalloff = 0.15;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * fragColor;
    vec2 uv = fragTexCoord;

    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * (vignetteSize);
    vig = 1. - pow(vig, vignetteFalloff);

    finalColor = mix(texel, vignetteColor, vig);
}
)");
        colorLoc = GetShaderLocation(shader, "vignetteColor");
        sizeLoc = GetShaderLocation(shader, "vignetteSize");
        fallOffLoc = GetShaderLocation(shader, "vignetteFalloff");
    }

    int VignetteShader::colorLoc = 0;

    int VignetteShader::fallOffLoc = 0;

    int VignetteShader::sizeLoc = 0;

    Shader VignetteShader::shader = {};

    OutlineShader::operator Shader() const { return shader; }

    OutlineShader& OutlineShader::setColor(Color color)
    {
        const auto normal = ColorNormalize(color);
        SetShaderValue(shader, colorLoc, &normal, SHADER_UNIFORM_VEC4);
        return *this;
    }
    OutlineShader& OutlineShader::setSize(float size)
    {
        SetShaderValue(shader, sizeLoc, &size, SHADER_UNIFORM_FLOAT);
        return *this;
    }

    void OutlineShader::Init()
    {
        shader = LoadShaderFromMemory(nullptr, R"(
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float outlineSize = 1;
uniform vec4 outlineColor = vec4(1., 1., 1., 0.9);

void main() {
    vec2 textureSize = vec2(2048, 2048);
    vec4 texel = texture(texture0, fragTexCoord) * fragColor * colDiffuse;
    vec2 texelScale = outlineSize / textureSize;

    vec4 neighbors = vec4(0.0);
    neighbors.x = texture(texture0, fragTexCoord + vec2(texelScale.x, 0)).a;
    neighbors.y = texture(texture0, fragTexCoord + vec2(0, -texelScale.y)).a;
    neighbors.z = texture(texture0, fragTexCoord + vec2(-texelScale.x, 0)).a;
    neighbors.w = texture(texture0, fragTexCoord + vec2(0, texelScale.y)).a;

    float hasSolidNeighbor = max(max(neighbors.x, neighbors.y), max(neighbors.z, neighbors.w));

    if (hasSolidNeighbor > 0.0) {
        finalColor = mix(outlineColor, texel, texel.a);
    } else {
        finalColor = texel;
    }
}
)");
        colorLoc = GetShaderLocation(shader, "outlineColor");
        sizeLoc = GetShaderLocation(shader, "outlineSize");
    }
    int OutlineShader::colorLoc = 0;
    int OutlineShader::sizeLoc = 0;
    Shader OutlineShader::shader = {};

} // namespace magique
