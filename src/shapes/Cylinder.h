#pragma once

#include "Shape.h"
#include <glm/glm.hpp>

class Cylinder : public Shape {
public:
    Cylinder(): Shape() {}
    int minParam1() const override { return 1; }
    int minParam2() const override { return 2; }

    void setVertexData() override;

private:
    glm::vec3 calcSideNormal(glm::vec3 pt);

    void makeCapTile(glm::vec3 center, glm::vec3 edge1, glm::vec3 edge2, glm::vec3 normal);
    void makeCapSlice(float currentTheta, float nextTheta, bool top);

    void makeSideTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight);
    void makeSideSlice(float currentTheta, float nextTheta);

    void makeWedge(float currentTheta, float nextTheta);

    float m_radius = 0.5f;
};
