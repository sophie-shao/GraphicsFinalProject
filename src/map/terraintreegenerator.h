#pragma once

#include "lsystem/axialtree.h"
#include "Tree.h"
#include <glm/glm.hpp>
#include <memory>

class TerrainTreeGenerator {
public:
    static constexpr int TREE_DEPTH = 3;
    static constexpr float TRUNK_HEIGHT = 5.0f;
    static constexpr float TRUNK_RADIUS = 0.3f;
    static constexpr float BRANCH_LENGTH_REDUCTION = 0.6f;
    static constexpr float BRANCH_RADIUS_REDUCTION = 0.7f;
    static constexpr float BRANCH_CONE_ANGLE = 90.0f;
    
    static Tree generateTerrainTree(glm::vec3 basePosition);
    
private:
    static AxialTree generateLSystemTree();
    static void generateTrunk(std::shared_ptr<Branch> rootBranch, float height, float radius);
    static void generateBranches(glm::vec3 startPos, glm::vec3 parentDir, 
                                 float parentLength, float parentRadius, 
                                 int currentDepth, std::shared_ptr<Branch> parentBranch);
    static Tree convertAxialTreeToTreePieces(const AxialTree& tree, glm::vec3 basePosition);
    static glm::vec3 getRandomDirectionInCone(glm::vec3 parentDir, float coneAngle);
    static glm::vec3 directionToRotation(glm::vec3 direction);
};

