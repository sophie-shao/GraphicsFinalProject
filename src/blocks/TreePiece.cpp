#include "TreePiece.h"
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

TreePiece::TreePiece() {
    generateGeometry();
}

void TreePiece::insertVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv,
                             glm::vec3 tangent, glm::vec3 bitangent) {
    m_vertexData.push_back(pos.x);
    m_vertexData.push_back(pos.y);
    m_vertexData.push_back(pos.z);

    m_vertexData.push_back(normal.x);
    m_vertexData.push_back(normal.y);
    m_vertexData.push_back(normal.z);

    m_vertexData.push_back(tangent.x);
    m_vertexData.push_back(tangent.y);
    m_vertexData.push_back(tangent.z);

    m_vertexData.push_back(bitangent.x);
    m_vertexData.push_back(bitangent.y);
    m_vertexData.push_back(bitangent.z);

    m_vertexData.push_back(uv.x);
    m_vertexData.push_back(uv.y);
}

void TreePiece::makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                         glm::vec3 bottomLeft, glm::vec3 bottomRight,
                         glm::vec3 normal, glm::vec2 uvTopLeft, glm::vec2 uvTopRight,
                         glm::vec2 uvBottomLeft, glm::vec2 uvBottomRight) {
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

    insertVertex(topLeft, normal, uvTopLeft, tangent, bitangent);
    insertVertex(bottomLeft, normal, uvBottomLeft, tangent, bitangent);
    insertVertex(topRight, normal, uvTopRight, tangent, bitangent);

    insertVertex(bottomLeft, normal, uvBottomLeft, tangent, bitangent);
    insertVertex(bottomRight, normal, uvBottomRight, tangent, bitangent);
    insertVertex(topRight, normal, uvTopRight, tangent, bitangent);
}

void TreePiece::makeSideSlice(float currentTheta, float nextTheta, int segments) {
    float yTop = m_height / 2.0f;
    float yBottom = -m_height / 2.0f;

    float uCurrent = currentTheta / (2.0f * M_PI);
    float uNext = nextTheta / (2.0f * M_PI);

    glm::vec3 topLeft(m_radius * cos(currentTheta), yTop, -m_radius * sin(currentTheta));
    glm::vec3 topRight(m_radius * cos(nextTheta), yTop, -m_radius * sin(nextTheta));
    glm::vec3 bottomLeft(m_radius * cos(currentTheta), yBottom, -m_radius * sin(currentTheta));
    glm::vec3 bottomRight(m_radius * cos(nextTheta), yBottom, -m_radius * sin(nextTheta));

    glm::vec3 normal = glm::normalize(glm::vec3(cos(currentTheta), 0.0f, -sin(currentTheta)));

    float circumference = 2.0f * M_PI * m_radius;
    float vRepeat = m_height / circumference;
    
    glm::vec2 uvTopLeft(uCurrent, vRepeat);
    glm::vec2 uvTopRight(uNext, vRepeat);
    glm::vec2 uvBottomLeft(uCurrent, 0.0f);
    glm::vec2 uvBottomRight(uNext, 0.0f);

    makeTile(topLeft, topRight, bottomLeft, bottomRight,
             normal, uvTopLeft, uvTopRight, uvBottomLeft, uvBottomRight);
}

void TreePiece::makeCapSlice(float currentTheta, float nextTheta, bool top, int segments) {
    float y = top ? m_height / 2.0f : -m_height / 2.0f;
    glm::vec3 normal = top ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f);

    glm::vec3 center(0.0f, y, 0.0f);
    glm::vec3 edge1(m_radius * cos(currentTheta), y, -m_radius * sin(currentTheta));
    glm::vec3 edge2(m_radius * cos(nextTheta), y, -m_radius * sin(nextTheta));

    glm::vec2 uvCenter(0.5f, 0.5f);
    glm::vec2 uvEdge1(0.5f + 0.5f * cos(currentTheta), 0.5f + 0.5f * sin(currentTheta));
    glm::vec2 uvEdge2(0.5f + 0.5f * cos(nextTheta), 0.5f + 0.5f * sin(nextTheta));

    glm::vec3 tangent = glm::normalize(edge1 - center);
    glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

    if (top) {
        insertVertex(center, normal, uvCenter, tangent, bitangent);
        insertVertex(edge1, normal, uvEdge1, tangent, bitangent);
        insertVertex(edge2, normal, uvEdge2, tangent, bitangent);
    } else {
        insertVertex(center, normal, uvCenter, tangent, bitangent);
        insertVertex(edge2, normal, uvEdge2, tangent, bitangent);
        insertVertex(edge1, normal, uvEdge1, tangent, bitangent);
    }
}

void TreePiece::generateGeometry() {
    m_vertexData.clear();

    float thetaStep = 2.0f * M_PI / static_cast<float>(m_segments);

    for (int i = 0; i < m_segments; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeSideSlice(currentTheta, nextTheta, m_segments);
        makeCapSlice(currentTheta, nextTheta, true, m_segments);
        makeCapSlice(currentTheta, nextTheta, false, m_segments);
    }
}

