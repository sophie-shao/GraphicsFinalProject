#pragma once
#include "Shape.h"
#include <glm/glm.hpp>

class Cone : public Shape {
public:
    Cone():Shape(){}
    int minParam1() const override { return 1; }
    int minParam2() const override { return 2; }

    void setVertexData() override;

private:
    void makeCapTile(glm::vec3 center, glm::vec3 edge1, glm::vec3 edge2);
    void makeCapSlice(float currentTheta, float nextTheta);

    void makeSlopeTile(glm::vec3 topLeft,
                       glm::vec3 topRight,
                       glm::vec3 bottomLeft,
                       glm::vec3 bottomRight);

    void makeSlopeSlice(float currentTheta, float nextTheta);
    void makeWedge(float currentTheta, float nextTheta);

    glm::vec3 calcNorm(glm::vec3 pt, glm::vec3 better);

    float m_radius = 0.5f;
};
