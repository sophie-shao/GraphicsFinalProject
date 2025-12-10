#include "Sphere.h"

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!
    glm::vec3 n1 = glm::normalize(topLeft);
    glm::vec3 n2 = glm::normalize(bottomLeft);
    glm::vec3 n3 = glm::normalize(topRight);
    glm::vec3 n4 = glm::normalize(bottomRight);

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

void Sphere::makeWedge(float currentTheta, float nextTheta) {

    float r = 0.5f;
    float phiStep = M_PI / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float currentPhi = i * phiStep;
        float nextPhi = (i + 1) * phiStep;

        glm::vec3 topLeft(
            r * glm::sin(currentPhi) * glm::cos(currentTheta),
            r * glm::cos(currentPhi),
            -r * glm::sin(currentPhi) * glm::sin(currentTheta)
            );
        glm::vec3 bottomLeft(
            r * glm::sin(nextPhi) * glm::cos(currentTheta),
            r * glm::cos(nextPhi),
            -r * glm::sin(nextPhi) * glm::sin(currentTheta)
            );

        glm::vec3 topRight(
            r * glm::sin(currentPhi) * glm::cos(nextTheta),
            r * glm::cos(currentPhi),
            -r * glm::sin(currentPhi) * glm::sin(nextTheta)
            );
        glm::vec3 bottomRight(
            r * glm::sin(nextPhi) * glm::cos(nextTheta),
            r * glm::cos(nextPhi),
            -r * glm::sin(nextPhi) * glm::sin(nextTheta)
            );

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}


void Sphere::makeSphere() {
    float thetaStep = glm::radians(360.f / m_param2);
    for (int i = 0; i < m_param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }

}

void Sphere::setVertexData() {
    m_vertexData.clear();

    // Uncomment these lines to make a wedge for Task 6, then comment them out for Task 7:

    // float thetaStep = glm::radians(360.f / m_param2);
    // float currentTheta = 0 * thetaStep;
    // float nextTheta = 1 * thetaStep;
    // makeWedge(currentTheta, nextTheta);

    // Uncomment these lines to make sphere for Task 7:

    makeSphere();
}

