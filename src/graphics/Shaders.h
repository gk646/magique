#ifndef SHADERS_H
#define SHADERS_H

namespace magique
{

    constexpr auto shadowFrag = R"(
#version 330 core
out vec4 FragColor;
void main() {
    gl_FragColor = vec4(1.);
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
        pos += normalize(dist) * 10.0;
    }
    gl_Position = mvp * vec4(pos, 0.0, 1.0);  // Correctly use pos.xy and add 0.0 for z coordinate
}

)";
    constexpr auto lightFrag = R"(
#version 330 core

in vec2 pos;

out vec4 finalColor;

uniform vec2 lightPos;

const float strengthFactor = 1024.;

void main() {
    vec2 dis = pos - lightPos;
    float str = 1. / (sqrt(dis.x * dis.x + dis.y * dis.y + strengthFactor * strengthFactor) - strengthFactor);
    finalColor = vec4(vec3(str), 0.5);
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
} // namespace magique

#endif //SHADERS_H