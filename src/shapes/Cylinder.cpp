#include "Cylinder.h"

glm::vec3 Cylinder::calcSideNormal(glm::vec3 pt) {
    glm::vec3 n(pt.x, 0.f, pt.z);
    return glm::normalize(n);
}

void Cylinder::makeCapTile(glm::vec3 center, glm::vec3 edge1, glm::vec3 edge2, glm::vec3 normal) {
    insertVec3(m_vertexData, edge2);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, edge1);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, center);
    insertVec3(m_vertexData, normal);
}

void Cylinder::makeCapSlice(float currentTheta, float nextTheta, bool top) {
    float step = 0.5f / m_param1;
    float y = top ? 0.5f : -0.5f;
    glm::vec3 normal = top ? glm::vec3(0.f, 1.f, 0.f) : glm::vec3(0.f, -1.f, 0.f);
    glm::vec3 center(0.f, y, 0.f);

    for (int i = 0; i < m_param1; i++) {
        float inner = i * step;
        float outer = (i + 1) * step;

        glm::vec3 inner1(inner * cos(currentTheta), y, -inner * sin(currentTheta));
        glm::vec3 inner2(inner * cos(nextTheta), y, -inner * sin(nextTheta));
        glm::vec3 outer1(outer * cos(currentTheta), y, -outer * sin(currentTheta));
        glm::vec3 outer2(outer * cos(nextTheta), y, -outer * sin(nextTheta));

        if (top) {
            makeCapTile(inner1, outer2, outer1, normal);
            makeCapTile(inner1, inner2, outer2, normal);
        } else {
            makeCapTile(inner1, outer1, outer2, normal);
            makeCapTile(inner1, outer2, inner2, normal);
        }
    }
}

void Cylinder::makeSideTile(glm::vec3 topLeft,
                            glm::vec3 topRight,
                            glm::vec3 bottomLeft,
                            glm::vec3 bottomRight) {

    glm::vec3 n0 = calcSideNormal(bottomLeft);
    glm::vec3 n1 = calcSideNormal(bottomRight);
    glm::vec3 n2 = calcSideNormal(topLeft);
    glm::vec3 n3 = calcSideNormal(topRight);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, n0);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, n3);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, n0);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, n3);
}

void Cylinder::makeSideSlice(float currentTheta, float nextTheta) {
    float step = 1.0f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float y1 = 0.5f - i * step;
        float y2 = 0.5f - (i + 1) * step;

        glm::vec3 topLeft(0.5f * cos(currentTheta), y1, -0.5f * sin(currentTheta));
        glm::vec3 topRight(0.5f * cos(nextTheta), y1, -0.5f * sin(nextTheta));
        glm::vec3 bottomLeft(0.5f * cos(currentTheta), y2, -0.5f * sin(currentTheta));
        glm::vec3 bottomRight(0.5f * cos(nextTheta), y2, -0.5f * sin(nextTheta));

        makeSideTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}


void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    makeCapSlice(currentTheta, nextTheta, true);
    makeCapSlice(currentTheta, nextTheta, false);
    makeSideSlice(currentTheta, nextTheta);
}

void Cylinder::setVertexData() {
    m_vertexData.clear();
    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}
