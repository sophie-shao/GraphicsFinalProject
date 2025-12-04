#pragma once
#include <glm/glm.hpp>
#include <cmath>

namespace Transform {

inline glm::mat4 translateMatrix(const glm::vec3 &translation) {
    glm::mat4 M(1.0f);
    M[3][0] = translation.x;
    M[3][1] = translation.y;
    M[3][2] = translation.z;
    return M;
}

inline glm::mat4 scaleMatrix(const glm::vec3 &scale) {
    glm::mat4 M(1.0f);
    M[0][0] = scale.x;
    M[1][1] = scale.y;
    M[2][2] = scale.z;
    return M;
}

//using Rodrigues' formula from algo
inline glm::mat4 rotateMatrix(float radians, const glm::vec3 &axis) {
    glm::vec3 a = glm::normalize(axis);
    float cosine = std::cos(radians);
    float sine = std::sin(radians);
    float x = a.x;
    float y = a.y;
    float z = a.z;

    glm::mat4 M(1.0f);
    M[0][0] = cosine + (1 - cosine) * x*x;
    M[0][1] = (1 - cosine) * x*y + sine*z;
    M[0][2] = (1 - cosine) * x*z - sine*y;

    M[1][0] = (1 - cosine) * y*x - sine*z;
    M[1][1] = cosine + (1 - cosine) * y*y;
    M[1][2] = (1 - cosine) * y*z + sine*x;

    M[2][0] = (1 - cosine) * z*x + sine*y;
    M[2][1] = (1 - cosine) * z*y - sine*x;
    M[2][2] = cosine + (1 - cosine) * z*z;

    return M;
}

}
