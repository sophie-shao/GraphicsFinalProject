#include "Cube.h"

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    glm::vec3 norm = glm::normalize(glm::cross(bottomLeft - topLeft, topRight - topLeft));

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, norm);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, norm);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, norm);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, norm);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, norm);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, norm);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    float step = 1.0f / m_param1;

    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {

            float alpha1 = i * step;
            float alpha2 = (i + 1) * step;
            float alpha3 = j * step;
            float alpha4 = (j + 1) * step;

            glm::vec3 leftTop = (1 - alpha1) * topLeft + alpha1 * bottomLeft;
            glm::vec3 leftBottom = (1 - alpha2) * topLeft + alpha2 * bottomLeft;
            glm::vec3 rightTop = (1 - alpha1) * topRight + alpha1 * bottomRight;
            glm::vec3 rightBottom = (1 - alpha2) * topRight + alpha2 * bottomRight;

            glm::vec3 tileTopLeft = (1 - alpha3) * leftTop + alpha3 * rightTop;
            glm::vec3 tileTopRight = (1 - alpha4) * leftTop + alpha4 * rightTop;
            glm::vec3 tileBottomLeft = (1 - alpha3) * leftBottom + alpha3 * rightBottom;
            glm::vec3 tileBottomRight = (1 - alpha4) * leftBottom + alpha4 * rightBottom;

            makeTile(tileTopLeft, tileTopRight, tileBottomLeft, tileBottomRight);
        }
    }

}

void Cube::setVertexData() {

    m_vertexData.clear();

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));


    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f));

    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f));

    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));

    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f));

    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f));

}
