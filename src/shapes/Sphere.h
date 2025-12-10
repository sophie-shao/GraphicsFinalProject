#pragma once
#include "Shape.h"
#include <glm/glm.hpp>

class Sphere : public Shape {
public:
    Sphere():Shape() {}
    int minParam1() const override { return 2; }
    int minParam2() const override { return 2; }

    void setVertexData() override;

private:
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeWedge(float currTheta, float nextTheta);
    void makeSphere();

    float m_radius = 0.5f;
};
