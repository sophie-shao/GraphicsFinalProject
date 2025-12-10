#pragma once

#include <glm/glm.hpp>
#include <vector>

struct TreePieceData {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    TreePieceData(glm::vec3 pos, glm::vec3 rot, glm::vec3 scl)
        : position(pos), rotation(rot), scale(scl) {}
};

class Tree {
public:
    Tree(glm::vec3 basePos) : basePosition(basePos) {}

    void addPiece(const TreePieceData& piece) {
        pieces.push_back(piece);
    }

    const std::vector<TreePieceData>& getPieces() const { return pieces; }
    glm::vec3 getBasePosition() const { return basePosition; }

private:
    std::vector<TreePieceData> pieces;
    glm::vec3 basePosition;
};



