#pragma once

#include <glm/glm.hpp>
#include <vector>

class TreePiece {
public:
    TreePiece();

    const std::vector<float>& getVertexData() const { return m_vertexData; }
    int getVertexCount() const { return static_cast<int>(m_vertexData.size() / 14); }

    void generateGeometry();

private:
    void makeSideSlice(float currentTheta, float nextTheta, int segments);
    void makeCapSlice(float currentTheta, float nextTheta, bool top, int segments);
    void makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                  glm::vec3 bottomLeft, glm::vec3 bottomRight,
                  glm::vec3 normal, glm::vec2 uvTopLeft, glm::vec2 uvTopRight,
                  glm::vec2 uvBottomLeft, glm::vec2 uvBottomRight);
    void insertVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv,
                      glm::vec3 tangent, glm::vec3 bitangent);

    std::vector<float> m_vertexData;
    static constexpr float m_radius = 0.3f;
    static constexpr float m_height = 25.0f;
    static constexpr int m_segments = 16;
};


