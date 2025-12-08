#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = std::max(1, param1);
    m_param2 = std::max(3, param2);
    setVertexData();
}

void Cylinder::setVertexData() {
    float thetaStep = glm::radians(360.0f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Cylinder::makeCapTile(glm::vec3 topLeft,
                           glm::vec3 topRight,
                           glm::vec3 bottomLeft,
                           glm::vec3 bottomRight,
                           glm::vec3 normal) {

    auto calculateCapUV = [](glm::vec3 pos) -> glm::vec2 {
        return glm::vec2(pos.x + 0.5f, pos.z + 0.5f);
    };

    glm::vec2 uvTopLeft = calculateCapUV(topLeft);
    glm::vec2 uvTopRight = calculateCapUV(topRight);
    glm::vec2 uvBottomLeft = calculateCapUV(bottomLeft);
    glm::vec2 uvBottomRight = calculateCapUV(bottomRight);

    glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 bitangent = glm::vec3(0.0f, 0.0f, 1.0f);

    if (normal.y < 0) {

        bitangent = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvTopLeft);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvBottomLeft);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvBottomRight);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvTopLeft);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvBottomRight);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvTopRight);
}

void Cylinder::makeTopCapTile(glm::vec3 topLeft,
                              glm::vec3 topRight,
                              glm::vec3 bottomLeft,
                              glm::vec3 bottomRight,
                              glm::vec3 normal) {

    auto calculateCapUV = [](glm::vec3 pos) -> glm::vec2 {
        return glm::vec2(pos.x + 0.5f, pos.z + 0.5f);
    };

    glm::vec2 uvTopLeft = calculateCapUV(topLeft);
    glm::vec2 uvTopRight = calculateCapUV(topRight);
    glm::vec2 uvBottomLeft = calculateCapUV(bottomLeft);
    glm::vec2 uvBottomRight = calculateCapUV(bottomRight);

    glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 bitangent = glm::vec3(0.0f, 0.0f, -1.0f);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvTopLeft);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvBottomRight);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvBottomLeft);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvTopLeft);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvTopRight);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, tangent);
    insertVec3(m_vertexData, bitangent);
    insertVec2(m_vertexData, uvBottomRight);
}

void Cylinder::makeCapSlice(float currentTheta, float nextTheta, float y, glm::vec3 normal, bool isTop) {
    float radius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float innerRadius = i / (float)m_param1 * radius;
        float outerRadius = (i + 1) / (float)m_param1 * radius;

        glm::vec3 topLeft(
            innerRadius * glm::cos(currentTheta),
            y,
            innerRadius * glm::sin(currentTheta)
            );

        glm::vec3 topRight(
            innerRadius * glm::cos(nextTheta),
            y,
            innerRadius * glm::sin(nextTheta)
            );

        glm::vec3 bottomLeft(
            outerRadius * glm::cos(currentTheta),
            y,
            outerRadius * glm::sin(currentTheta)
            );

        glm::vec3 bottomRight(
            outerRadius * glm::cos(nextTheta),
            y,
            outerRadius * glm::sin(nextTheta)
            );

        if (isTop) {
            makeTopCapTile(topLeft, topRight, bottomLeft, bottomRight, normal);
        } else {
            makeCapTile(topLeft, topRight, bottomLeft, bottomRight, normal);
        }
    }
}

void Cylinder::makeSideTile(glm::vec3 topLeft,
                            glm::vec3 topRight,
                            glm::vec3 bottomLeft,
                            glm::vec3 bottomRight) {

    glm::vec3 normalLeft = glm::normalize(glm::vec3(topLeft.x, 0.0f, topLeft.z));
    glm::vec3 normalRight = glm::normalize(glm::vec3(topRight.x, 0.0f, topRight.z));

    auto calculateSideUV = [](glm::vec3 pos) -> glm::vec2 {
        float u = 0.5f + atan2(pos.z, pos.x) / (2.0f * M_PI);
        float v = pos.y + 0.5f;
        return glm::vec2(u, v);
    };

    glm::vec2 uvTopLeft = calculateSideUV(topLeft);
    glm::vec2 uvTopRight = calculateSideUV(topRight);
    glm::vec2 uvBottomLeft = calculateSideUV(bottomLeft);
    glm::vec2 uvBottomRight = calculateSideUV(bottomRight);


    auto calculateTangentBitangent = [](glm::vec3 normal) -> std::pair<glm::vec3, glm::vec3> {
        glm::vec3 tangent = glm::normalize(glm::vec3(-normal.z, 0.0f, normal.x));
        glm::vec3 bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        return {tangent, bitangent};
    };

    auto [tangentLeft, bitangentLeft] = calculateTangentBitangent(normalLeft);
    auto [tangentRight, bitangentRight] = calculateTangentBitangent(normalRight);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalLeft);
    insertVec3(m_vertexData, tangentLeft);
    insertVec3(m_vertexData, bitangentLeft);
    insertVec2(m_vertexData, uvTopLeft);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalRight);
    insertVec3(m_vertexData, tangentRight);
    insertVec3(m_vertexData, bitangentRight);
    insertVec2(m_vertexData, uvBottomRight);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normalLeft);
    insertVec3(m_vertexData, tangentLeft);
    insertVec3(m_vertexData, bitangentLeft);
    insertVec2(m_vertexData, uvBottomLeft);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalLeft);
    insertVec3(m_vertexData, tangentLeft);
    insertVec3(m_vertexData, bitangentLeft);
    insertVec2(m_vertexData, uvTopLeft);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normalRight);
    insertVec3(m_vertexData, tangentRight);
    insertVec3(m_vertexData, bitangentRight);
    insertVec2(m_vertexData, uvTopRight);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalRight);
    insertVec3(m_vertexData, tangentRight);
    insertVec3(m_vertexData, bitangentRight);
    insertVec2(m_vertexData, uvBottomRight);
}

void Cylinder::makeSideSlice(float currentTheta, float nextTheta) {
    float topY = 0.5f;
    float bottomY = -0.5f;
    float radius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float upperY = topY - i / (float)m_param1 * (topY - bottomY);
        float lowerY = topY - (i + 1) / (float)m_param1 * (topY - bottomY);

        glm::vec3 topLeft(
            radius * glm::cos(currentTheta),
            upperY,
            radius * glm::sin(currentTheta)
            );

        glm::vec3 topRight(
            radius * glm::cos(nextTheta),
            upperY,
            radius * glm::sin(nextTheta)
            );

        glm::vec3 bottomLeft(
            radius * glm::cos(currentTheta),
            lowerY,
            radius * glm::sin(currentTheta)
            );

        glm::vec3 bottomRight(
            radius * glm::cos(nextTheta),
            lowerY,
            radius * glm::sin(nextTheta)
            );

        makeSideTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    makeSideSlice(currentTheta, nextTheta);
    makeCapSlice(currentTheta, nextTheta, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f), true);
    makeCapSlice(currentTheta, nextTheta, -0.5f, glm::vec3(0.0f, -1.0f, 0.0f), false);
}

void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
