#include "Cone.h"

glm::vec3 Cone::calcNorm(glm::vec3 pt, glm::vec3 bettter) {

    if(pt.y > 0.4999f) {
        return bettter;
    }
    float xNorm = (2 * pt.x);
    float yNorm = -(1.f/4.f) * (2.f * pt.y - 1.f);
    float zNorm = (2 * pt.z);

    return glm::normalize(glm::vec3{ xNorm, yNorm, zNorm });
}

void Cone::makeCapTile(glm::vec3 center, glm::vec3 edge1, glm::vec3 edge2) {
    glm::vec3 normal(0.f, -1.0f, 0.f); //down

    insertVec3(m_vertexData, edge2);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, edge1);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, center);
    insertVec3(m_vertexData, normal);
}

void Cone::makeCapSlice(float currentTheta, float nextTheta) {
    float step = 0.5f / m_param1;
    for (int i = 0; i < m_param1; i++) {
        float inner = i * step;
        float outer = (i + 1) * step;

        glm::vec3 center(0.f, -0.5f, 0.f);
        glm::vec3 inner1(inner * cos(currentTheta), -0.5f, -inner * sin(currentTheta));
        glm::vec3 inner2(inner * cos(nextTheta), -0.5f, -inner * sin(nextTheta));
        glm::vec3 outer1(outer * cos(currentTheta), -0.5f, -outer * sin(currentTheta));
        glm::vec3 outer2(outer * cos(nextTheta), -0.5f, -outer * sin(nextTheta));

        makeCapTile(inner1, outer1, outer2);
        makeCapTile(inner1, outer2, inner2);
    }
}

void Cone::makeSlopeTile(glm::vec3 topLeft,
                         glm::vec3 topRight,
                         glm::vec3 bottomLeft,
                         glm::vec3 bottomRight) {

    glm::vec3 n0 = calcNorm(bottomLeft, glm::vec3(0,0,0));
    glm::vec3 n00 = calcNorm(bottomRight, glm::vec3(0,0,0));
    glm::vec3 n1 = calcNorm(topLeft, n0*0.5f + n00*0.5f);
    glm::vec3 n2 = calcNorm(bottomLeft, n0*0.5f + n00*0.5f);
    glm::vec3 n3 = calcNorm(topRight, n0*0.5f + n00*0.5f);
    glm::vec3 n4 = calcNorm(bottomRight, n0*0.5f + n00*0.5f);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, n1);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, n3);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, n2);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, n4);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, n3);
}

void Cone::makeSlopeSlice(float currentTheta, float nextTheta) {
    float step = 1.0f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float y1 = 0.5f - i * step;
        float y2 = 0.5f - (i + 1) * step;

        float r1 = 0.5f * (1 - (y1 + 0.5f));
        float r2 = 0.5f * (1 - (y2 + 0.5f));

        glm::vec3 topLeft(r1 * cos(currentTheta), y1, -r1 * sin(currentTheta));
        glm::vec3 topRight(r1 * cos(nextTheta), y1, -r1 * sin(nextTheta));
        glm::vec3 bottomLeft(r2 * cos(currentTheta), y2, -r2 * sin(currentTheta));
        glm::vec3 bottomRight(r2 * cos(nextTheta), y2, -r2 * sin(nextTheta));

        makeSlopeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    makeCapSlice(currentTheta, nextTheta);
        makeSlopeSlice(currentTheta, nextTheta);
}
void Cone::setVertexData() {
    m_vertexData.clear();

    float thetaStep = glm::radians(360.f / m_param2);

    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeWedge(currentTheta, nextTheta);
    }
}
