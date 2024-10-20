#ifndef GRAPHICS_SHADERS_H
#define GRAPHICS_SHADERS_H

namespace magique
{
    constexpr auto shadowFrag = R"(
#version 330 core
out vec4 FragColor;
void main() {
    gl_FragColor = vec4(1,0,0,1);
})";

    constexpr auto shadowVert = R"(
#version 330 core
layout (location = 0) in vec3 vertexPosition;     // Vertex input attribute: position

uniform vec2 lightPosition;
uniform mat4 mvp;

void main()
{
    vec2 pos = vertexPosition.xy;
    if(vertexPosition.z > 0.0){
        vec2 dist = pos - lightPosition;
        pos += normalize(dist) * 1000;
    }
    gl_Position = mvp * vec4(pos, 0., 1.);
}
)";

    constexpr auto lightFrag = R"(
#version 330 core

in vec2 pos;

out vec4 finalColor;

uniform vec4 lightColor;
uniform vec2 lightPos;
uniform int intensity;
uniform int lightStyle;

void main() {
    vec2 dis = pos - lightPos;
    float distance = length(dis);
    float str;

    if (lightStyle == 0) {                                  // Light style 0: point light with falloff
        str = 1.0 / (distance * distance / intensity + 1.0);
    } else if (lightStyle == 1) {                           // Light style 1: sunlight with minimal falloff
        str = 1.0 / (distance / intensity + 1.0);
    } else {
        str = 1.0;                                          // Default to no falloff if lightStyle is not recognized
    }
    finalColor = vec4(vec3(lightColor.xyz) * str, lightColor.w);
}
)";

    constexpr auto lightVert = R"(
#version 330 core
layout (location = 0) in vec3 vertexPosition;     // Vertex input attribute: position
in vec2 vertexTexCoord;     // Vertex input attribute: texture coordinate
in vec4 vertexColor;        // Vertex input attribute: color

out vec2 fragTexCoord;      // To-fragment attribute: texture coordinate
out vec4 fragColor;         // To-fragment attribute: color
out vec2 pos;

uniform mat4 mvp;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    pos = vertexPosition.xy;
    gl_Position = mvp * vec4(pos.x, pos.y, 0., 1.0);
}
)";

    constexpr auto shadowTextureFrag = R"(
#version 330 core
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
     vec4 texelColor = texture(texture0, fragTexCoord);
     finalColor = vec4(vec3(fragColor.a * texelColor.rgb + 1 - fragColor.a),1.);
}
)";

    constexpr auto shadowTextureVert = R"(
#version 330 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
)";


    constexpr auto rayVert = R"(
#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";


    constexpr auto rayFrag = R"(
#version 330 core

out vec4 color;

#define MAGIQUE_MAX_RAYTRACING_ENTITIES 50

// Lights
uniform vec2 lightPositions[MAGIQUE_MAX_RAYTRACING_ENTITIES];
uniform vec3 lightColors[MAGIQUE_MAX_RAYTRACING_ENTITIES];
uniform int lightStyle[MAGIQUE_MAX_RAYTRACING_ENTITIES];
uniform int lightCount;

// Occluders
uniform vec2 occPositions[MAGIQUE_MAX_RAYTRACING_ENTITIES];
uniform vec2 occDimensions[MAGIQUE_MAX_RAYTRACING_ENTITIES];
uniform int occShape[MAGIQUE_MAX_RAYTRACING_ENTITIES];
uniform int occCount;

float sphereSDF(vec2 p, float size) {
    return length(p) - size;
}

float boxSDF(vec2 p, vec2 size) {
    vec2 r = abs(p) - size;
    return min(max(r.x, r.y), 0.0) + length(max(r, vec2(0.0, 0.0)));
}

void addObj(inout float dist, inout vec3 color, float d, vec3 c) {
    if (dist > d) {
        dist = d;
        color = c;
    }
}

void scene(in vec2 pos, out vec3 color, out float dist) {
    dist = 1e9;
    color = vec3(0.0, 0.0, 0.0);
    addObj(dist, color, boxSDF(pos - vec2(-4, 1), vec2(1, 1)), vec3(-0.6, 0.8, 1.0));
    addObj(dist, color, sphereSDF(pos - vec2(4, -2), 0.5), vec3(1.0, 0.9, 2.8));

    addObj(dist, color, boxSDF(pos - vec2(0, 3.0 * sin(iTime)), vec2(1.0, 3.5)), vec3(0.4, 0.1, 0.1));
}

void trace(vec2 p, vec2 dir, out vec3 c) {
    for (int j = 0; j < 20; j++) {
        float d;
        scene(p, c, d);
        if (d < 1e-3) {
            return;
        }
        if (d > 1e1) break;
        p -= dir * d;
    }
    c = vec3(0.0, 0.0, 0.0);
}

float random(in vec2 _st) {
    return fract(sin(dot(_st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

#define SAMPLES 32

void main() {
    vec2 uv = (gl_FragCoord.xy - (resolution.xy / 2.0)) / resolution.y * 10.0;
    vec3 col = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < SAMPLES; i++) {
        float t = (float(i) + random(uv + float(i))) / float(SAMPLES) * 2.0 * 3.1415;
        vec3 c;
        trace(uv, vec2(cos(t), sin(t)), c);
        col += c;
    }
    col /= float(SAMPLES);

    color = vec4(col * 2.0, 3.0);
}
)";


} // namespace magique

#endif //GRAPHICS_SHADERS_H