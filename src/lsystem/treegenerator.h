#pragma once

#include "axialtree.h"
#include <glm/glm.hpp>

enum class TreeType {
    DEAD_PINE,
    DEAD_OAK,
    DEAD_BIRCH
};

struct TreeParameters {
    TreeType treeType;
    float trunkLength;
    float trunkRadius;
    float branchAngle;
    float subBranchAngle;
    float branchLength;
    int numBranchLayers;
    int branchesPerLayer;
    int branchDepth;
};

class TreeGenerator {
public:
    static AxialTree generateTree(const TreeParameters& params);
    
private:
    static AxialTree generateDeadPine(const TreeParameters& params);
    static AxialTree generateDeadOak(const TreeParameters& params);
    static AxialTree generateDeadBirch(const TreeParameters& params);
    
    static void generateSubBranches(glm::vec3 startPos,
                                    glm::vec3 parentDir,
                                    const TreeParameters& params,
                                    int currentDepth,
                                    float heightRatio,
                                    std::shared_ptr<Branch> parentBranch);
    
    static void generateOakSubBranches(glm::vec3 startPos,
                                      glm::vec3 parentDir,
                                      const TreeParameters& params,
                                      int currentDepth,
                                      float heightRatio,
                                      std::shared_ptr<Branch> parentBranch);
};
