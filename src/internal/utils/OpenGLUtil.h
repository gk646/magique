#ifndef OPENGLUTIL_H
#define OPENGLUTIL_H

// Function to create a simple object buffer
inline void createSimpleObjectBuffer(const Vector3* vertices, int vertexCount, unsigned int* vao, unsigned int* vbo)
{
    // Generate and bind a Vertex Array Object
    *vao = rlLoadVertexArray();
    rlEnableVertexArray(*vao);

    // Generate and bind a Vertex Buffer Object
    *vbo = rlLoadVertexBuffer(vertices, vertexCount * sizeof(Vector3), false);
    rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
    rlEnableVertexAttribute(0);

    // Unbind the VAO
    rlDisableVertexArray();
}

inline void CreateShadowQuads(magique::vector<Vector3>& quads, Vector2 posA, Vector2 posB)
{
    // Triangle 1
    quads.push_back(Vector3{posA.x, posA.y, 0});
    quads.push_back(Vector3{posB.x, posB.y, 0});
    quads.push_back(Vector3{posA.x, posA.y, 1});

    // Triangle 2
    quads.push_back(Vector3{posA.x, posA.y, 1});
    quads.push_back(Vector3{posB.x, posB.y, 0});
    quads.push_back(Vector3{posB.x, posB.y, 1});
}

inline void CreateQuadsFromRect(magique::vector<Vector3> &quads, const Rectangle rect)
{
    // Bottom edge
    CreateShadowQuads(quads, {rect.x, rect.y}, {rect.x + rect.width, rect.y});
    // Right edge
    CreateShadowQuads(quads, {rect.x + rect.width, rect.y}, {rect.x + rect.width, rect.y + rect.height});
    // Top edge
    CreateShadowQuads(quads, {rect.x + rect.width, rect.y + rect.height}, {rect.x, rect.y + rect.height});
    // Left edge
    CreateShadowQuads(quads, {rect.x, rect.y + rect.height}, {rect.x, rect.y});
}


#endif //OPENGLUTIL_H