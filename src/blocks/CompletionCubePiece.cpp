#include "CompletionCubePiece.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CompletionCubePiece::CompletionCubePiece() {
    generateGeometry();
}

void CompletionCubePiece::insertVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv,
                            glm::vec3 tangent, glm::vec3 bitangent) {
    m_vertexData.push_back(pos.x); m_vertexData.push_back(pos.y); m_vertexData.push_back(pos.z);
    m_vertexData.push_back(normal.x); m_vertexData.push_back(normal.y); m_vertexData.push_back(normal.z);
    m_vertexData.push_back(tangent.x); m_vertexData.push_back(tangent.y); m_vertexData.push_back(tangent.z);
    m_vertexData.push_back(bitangent.x); m_vertexData.push_back(bitangent.y); m_vertexData.push_back(bitangent.z);
    m_vertexData.push_back(uv.x); m_vertexData.push_back(uv.y);
}

void CompletionCubePiece::makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                        glm::vec3 bottomLeft, glm::vec3 bottomRight,
                        glm::vec3 normalTopLeft, glm::vec3 normalTopRight,
                        glm::vec3 normalBottomLeft, glm::vec3 normalBottomRight,
                        glm::vec2 uvTopLeft, glm::vec2 uvTopRight,
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

    insertVertex(topLeft, normalTopLeft, uvTopLeft, tangent, bitangent);
    insertVertex(topRight, normalTopRight, uvTopRight, tangent, bitangent);
    insertVertex(bottomLeft, normalBottomLeft, uvBottomLeft, tangent, bitangent);

    insertVertex(bottomLeft, normalBottomLeft, uvBottomLeft, tangent, bitangent);
    insertVertex(topRight, normalTopRight, uvTopRight, tangent, bitangent);
    insertVertex(bottomRight, normalBottomRight, uvBottomRight, tangent, bitangent);
}

void CompletionCubePiece::makeSphereSlice(float currentTheta, float nextTheta, float currentPhi, float nextPhi, int segments) {
    glm::vec3 topLeft(
        m_radius * sin(currentPhi) * cos(currentTheta),
        m_radius * cos(currentPhi),
        -m_radius * sin(currentPhi) * sin(currentTheta)
    );
    glm::vec3 topRight(
        m_radius * sin(currentPhi) * cos(nextTheta),
        m_radius * cos(currentPhi),
        -m_radius * sin(currentPhi) * sin(nextTheta)
    );
    glm::vec3 bottomLeft(
        m_radius * sin(nextPhi) * cos(currentTheta),
        m_radius * cos(nextPhi),
        -m_radius * sin(nextPhi) * sin(currentTheta)
    );
    glm::vec3 bottomRight(
        m_radius * sin(nextPhi) * cos(nextTheta),
        m_radius * cos(nextPhi),
        -m_radius * sin(nextPhi) * sin(nextTheta)
    );

    glm::vec3 normalTopLeft = glm::normalize(topLeft);
    glm::vec3 normalTopRight = glm::normalize(topRight);
    glm::vec3 normalBottomLeft = glm::normalize(bottomLeft);
    glm::vec3 normalBottomRight = glm::normalize(bottomRight);

    float uCurrent = currentTheta / (2.0f * M_PI);
    float uNext = nextTheta / (2.0f * M_PI);
    float vCurrent = currentPhi / M_PI;
    float vNext = nextPhi / M_PI;

    glm::vec2 uvTopLeft(uCurrent, vCurrent);
    glm::vec2 uvTopRight(uNext, vCurrent);
    glm::vec2 uvBottomLeft(uCurrent, vNext);
    glm::vec2 uvBottomRight(uNext, vNext);

    makeTile(topLeft, topRight, bottomLeft, bottomRight,
             normalTopLeft, normalTopRight, normalBottomLeft, normalBottomRight,
             uvTopLeft, uvTopRight, uvBottomLeft, uvBottomRight);
}

void CompletionCubePiece::generateGeometry() {
    m_vertexData.clear();

    float thetaStep = 2.0f * M_PI / static_cast<float>(m_segments);
    float phiStep = M_PI / static_cast<float>(m_segments);

    for (int i = 0; i < m_segments; i++) {
        float currentPhi = i * phiStep;
        float nextPhi = (i + 1) * phiStep;

        for (int j = 0; j < m_segments; j++) {
            float currentTheta = j * thetaStep;
            float nextTheta = (j + 1) * thetaStep;

            makeSphereSlice(currentTheta, nextTheta, currentPhi, nextPhi, m_segments);
        }
    }
}



