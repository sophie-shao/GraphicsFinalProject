#pragma once

#include <glm/glm.hpp>
#include <vector>

class Block {
public:
    Block();

    const std::vector<float>& getVertexData() const { return m_vertexData; }
    int getVertexCount() const { return static_cast<int>(m_vertexData.size() / 14); }

    void generateGeometry();

private:
    void makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                  glm::vec3 bottomLeft, glm::vec3 bottomRight,
                  glm::vec3 normal, glm::vec2 uvTopLeft, glm::vec2 uvTopRight,
                  glm::vec2 uvBottomLeft, glm::vec2 uvBottomRight);

    void makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                  glm::vec3 bottomLeft, glm::vec3 bottomRight,
                  glm::vec3 normal);

    void insertVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv,
                      glm::vec3 tangent, glm::vec3 bitangent);

    std::vector<float> m_vertexData;
};
