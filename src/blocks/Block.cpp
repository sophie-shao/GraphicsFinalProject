#include "Block.h"
#include <algorithm>

Block::Block() {
    generateGeometry();
}

void Block::insertVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv,
                         glm::vec3 tangent, glm::vec3 bitangent) {
    // Position (3 floats)
    m_vertexData.push_back(pos.x);
    m_vertexData.push_back(pos.y);
    m_vertexData.push_back(pos.z);

    // Normal (3 floats)
    m_vertexData.push_back(normal.x);
    m_vertexData.push_back(normal.y);
    m_vertexData.push_back(normal.z);

    // Tangent (3 floats)
    m_vertexData.push_back(tangent.x);
    m_vertexData.push_back(tangent.y);
    m_vertexData.push_back(tangent.z);

    // Bitangent (3 floats)
    m_vertexData.push_back(bitangent.x);
    m_vertexData.push_back(bitangent.y);
    m_vertexData.push_back(bitangent.z);

    // UV (2 floats)
    m_vertexData.push_back(uv.x);
    m_vertexData.push_back(uv.y);
}

void Block::makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                     glm::vec3 bottomLeft, glm::vec3 bottomRight,
                     glm::vec3 normal, glm::vec2 uvTopLeft, glm::vec2 uvTopRight,
                     glm::vec2 uvBottomLeft, glm::vec2 uvBottomRight) {

    // Calculate tangent and bitangent for this face
    glm::vec3 edge1 = topRight - topLeft;
    glm::vec3 edge2 = bottomLeft - topLeft;
    glm::vec2 deltaUV1 = uvTopRight - uvTopLeft;
    glm::vec2 deltaUV2 = uvBottomLeft - uvTopLeft;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = glm::normalize(tangent);

    glm::vec3 bitangent;
    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent = glm::normalize(bitangent);

    // First triangle
    insertVertex(topLeft, normal, uvTopLeft, tangent, bitangent);
    insertVertex(bottomLeft, normal, uvBottomLeft, tangent, bitangent);
    insertVertex(topRight, normal, uvTopRight, tangent, bitangent);

    // Second triangle
    insertVertex(bottomLeft, normal, uvBottomLeft, tangent, bitangent);
    insertVertex(bottomRight, normal, uvBottomRight, tangent, bitangent);
    insertVertex(topRight, normal, uvTopRight, tangent, bitangent);
}

void Block::makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                     glm::vec3 bottomLeft, glm::vec3 bottomRight,
                     glm::vec3 normal) {

    glm::vec2 uvTopLeft(0.0f, 0.0f);
    glm::vec2 uvTopRight(1.0f, 0.0f);
    glm::vec2 uvBottomLeft(0.0f, 1.0f);
    glm::vec2 uvBottomRight(1.0f, 1.0f);

    makeTile(topLeft, topRight, bottomLeft, bottomRight,
             normal, uvTopLeft, uvTopRight, uvBottomLeft, uvBottomRight);
}

void Block::generateGeometry() {
    m_vertexData.clear();

    // Front face (+Z)
    makeFace(glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(0.0f, 0.0f, 1.0f));

    // Back face (-Z)
    makeFace(glm::vec3(0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(0.0f, 0.0f, -1.0f));

    // Left face (-X)
    makeFace(glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(-1.0f, 0.0f, 0.0f));

    // Right face (+X)
    makeFace(glm::vec3( 0.5f, 0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(1.0f, 0.0f, 0.0f));

    // Top face (+Y)
    makeFace(glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(0.5f, 0.5f, 0.5f),
             glm::vec3(0.0f, 1.0f, 0.0f));

    // Bottom face (-Y)
    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(0.0f, -1.0f, 0.0f));
}
