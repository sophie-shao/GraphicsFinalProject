#include "Block.h"
#include <algorithm>

Block::Block() {
    generateGeometry();
}

void Block::insertVertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 uv) {
    m_vertexData.push_back(pos.x);
    m_vertexData.push_back(pos.y);
    m_vertexData.push_back(pos.z);
    m_vertexData.push_back(normal.x);
    m_vertexData.push_back(normal.y);
    m_vertexData.push_back(normal.z);
    m_vertexData.push_back(uv.x);
    m_vertexData.push_back(uv.y);
}

void Block::makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                     glm::vec3 bottomLeft, glm::vec3 bottomRight,
                     glm::vec3 normal, glm::vec2 uvTopLeft, glm::vec2 uvTopRight,
                     glm::vec2 uvBottomLeft, glm::vec2 uvBottomRight) {
    
    insertVertex(topLeft, normal, uvTopLeft);
    insertVertex(bottomLeft, normal, uvBottomLeft);
    insertVertex(topRight, normal, uvTopRight);
    
    insertVertex(bottomLeft, normal, uvBottomLeft);
    insertVertex(bottomRight, normal, uvBottomRight);
    insertVertex(topRight, normal, uvTopRight);
}

void Block::makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                     glm::vec3 bottomLeft, glm::vec3 bottomRight,
                     glm::vec3 normal) {
    
    glm::vec2 uvTopLeft(0.0f, 0.0f);
    glm::vec2 uvTopRight(1.0f, 0.0f);
    glm::vec2 uvBottomLeft(0.0f, 1.0f);
    glm::vec2 uvBottomRight(1.0f, 1.0f);
    
    makeTile(topLeft, topRight, bottomLeft, bottomRight,
             normal, uvTopLeft, uvTopRight, uvBottomLeft, uvBottomRight);
}

void Block::generateGeometry() {
    m_vertexData.clear();
    
    makeFace(glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(0.0f, 0.0f, 1.0f));
    
    makeFace(glm::vec3(0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(0.0f, 0.0f, -1.0f));
    
    makeFace(glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(-1.0f, 0.0f, 0.0f));
    
    makeFace(glm::vec3( 0.5f, 0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(1.0f, 0.0f, 0.0f));
    
    makeFace(glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, 0.5f, 0.5f),
             glm::vec3(0.5f, 0.5f, 0.5f),
             glm::vec3(0.0f, 1.0f, 0.0f));
    
    makeFace(glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(0.0f, -1.0f, 0.0f));
}

