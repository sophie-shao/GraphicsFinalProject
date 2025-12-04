#pragma once
#include "Shape.h"
#include <glm/glm.hpp>

class Cube : public Shape {
public:
    Cube():Shape(){}
    int minParam1() const override { return 1; }
    int minParam2() const override { return 1; }

    void setVertexData() override;

private:
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
};
