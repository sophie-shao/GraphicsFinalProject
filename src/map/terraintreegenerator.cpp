#include "terraintreegenerator.h"
#include <random>
#include <cmath>
#include <memory>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
static std::uniform_real_distribution<float> coneDist(0.0f, 1.0f);
static std::uniform_int_distribution<int> branchCountDist(1, 3);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Tree TerrainTreeGenerator::generateTerrainTree(glm::vec3 basePosition) {
    AxialTree lSystemTree = generateLSystemTree();
    return convertAxialTreeToTreePieces(lSystemTree, basePosition);
}

AxialTree TerrainTreeGenerator::generateLSystemTree() {
    AxialTree tree;
    
    generateTrunk(tree.rootBranch, TRUNK_HEIGHT, TRUNK_RADIUS);
    
    float trunkSegmentLength = TRUNK_HEIGHT / 4.0f;
    glm::vec3 trunkDir(0.0f, 1.0f, 0.0f);
    
    for (int i = 1; i <= 3; ++i) {
        float branchY = trunkSegmentLength * i;
        glm::vec3 branchPoint(0.0f, branchY, 0.0f);
        
        float heightRatio = branchY / TRUNK_HEIGHT;
        float branchLength = TRUNK_HEIGHT * 0.4f * (1.0f - heightRatio * 0.5f);
        float branchRadius = TRUNK_RADIUS * 0.6f;
        
        int numBranches = branchCountDist(gen);
        for (int b = 0; b < numBranches; ++b) {
            glm::vec3 branchDir = getRandomDirectionInCone(trunkDir, BRANCH_CONE_ANGLE);
            
            auto branchSegment = std::make_shared<Segment>(
                branchPoint,
                branchDir,
                branchLength,
                branchRadius,
                1,
                false,
                true
            );
            
            tree.rootBranch->mainAxis->addSegment(branchSegment);
            
            if (TREE_DEPTH > 1) {
                generateBranches(
                    branchSegment->end,  // Start recursive branches where this branch ends
                    branchDir,
                    branchLength,
                    branchRadius,
                    1,
                    tree.rootBranch
                );
            }
        }
    }
    
    tree.collectAllSegments();
    return tree;
}

void TerrainTreeGenerator::generateTrunk(std::shared_ptr<Branch> rootBranch, float height, float radius) {
    int trunkSegments = 4;
    float segmentLength = height / static_cast<float>(trunkSegments);
    glm::vec3 trunkStart(0.0f, 0.0f, 0.0f);
    glm::vec3 trunkDir(0.0f, 1.0f, 0.0f);
    
    for (int i = 0; i < trunkSegments; ++i) {
        float heightRatio = static_cast<float>(i) / static_cast<float>(trunkSegments);
        float radiusReduction = 1.0f - (heightRatio * 0.3f);
        float currentRadius = radius * radiusReduction;
        currentRadius = glm::max(0.05f, currentRadius);
        
        auto trunkSegment = std::make_shared<Segment>(
            trunkStart,
            trunkDir,
            segmentLength,
            currentRadius,
            0,
            true,
            false
        );
        
        rootBranch->mainAxis->addSegment(trunkSegment);
        trunkStart = trunkSegment->end;
    }
}

void TerrainTreeGenerator::generateBranches(glm::vec3 startPos, glm::vec3 parentDir,
                                            float parentLength, float parentRadius,
                                            int currentDepth, std::shared_ptr<Branch> parentBranch) {
    if (currentDepth >= TREE_DEPTH) {
        return;
    }
    
    float depthLengthReduction = std::pow(BRANCH_LENGTH_REDUCTION, currentDepth);
    float depthRadiusReduction = std::pow(BRANCH_RADIUS_REDUCTION, currentDepth);
    
    float branchLength = parentLength * BRANCH_LENGTH_REDUCTION;
    float branchRadius = parentRadius * BRANCH_RADIUS_REDUCTION;
    
    branchLength *= depthLengthReduction;
    branchRadius *= depthRadiusReduction;
    
    branchLength = glm::max(0.3f, branchLength);
    branchRadius = glm::max(0.02f, branchRadius);
    
    int numBranches = (currentDepth < TREE_DEPTH - 1) ? branchCountDist(gen) : 1;
    
    for (int b = 0; b < numBranches; ++b) {
        glm::vec3 branchDir = getRandomDirectionInCone(parentDir, BRANCH_CONE_ANGLE);
        
        auto branchSegment = std::make_shared<Segment>(
            startPos,
            branchDir,
            branchLength,
            branchRadius,
            currentDepth + 1,
            false,
            true
        );
        
        parentBranch->mainAxis->addSegment(branchSegment);
        
        if (currentDepth + 1 < TREE_DEPTH) {
            generateBranches(
                branchSegment->end,  // Start next branch where this one ends
                branchDir,
                branchLength,
                branchRadius,
                currentDepth + 1,
                parentBranch
            );
        }
    }
}

glm::vec3 TerrainTreeGenerator::getRandomDirectionInCone(glm::vec3 parentDir, float coneAngle) {
    parentDir = glm::normalize(parentDir);
    float angleRad = glm::radians(coneAngle);
    
    float u = coneDist(gen);
    float v = angleDist(gen) * M_PI / 180.0f;
    
    float theta = std::acos(1.0f - u * (1.0f - std::cos(angleRad)));
    float phi = v;
    
    glm::vec3 perpAxis = glm::normalize(glm::cross(parentDir, glm::vec3(0.0f, 1.0f, 0.0f)));
    if (glm::length(perpAxis) < 0.001f) {
        perpAxis = glm::normalize(glm::cross(parentDir, glm::vec3(1.0f, 0.0f, 0.0f)));
        if (glm::length(perpAxis) < 0.001f) {
            perpAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        }
    }
    
    glm::vec3 upAxis = glm::normalize(glm::cross(perpAxis, parentDir));
    
    glm::vec3 localDir(
        std::sin(theta) * std::cos(phi),
        std::cos(theta),
        std::sin(theta) * std::sin(phi)
    );
    
    glm::vec3 worldDir = perpAxis * localDir.x + parentDir * localDir.y + upAxis * localDir.z;
    worldDir = glm::normalize(worldDir);
    
    if (worldDir.y < -0.1f) {
        worldDir.y = std::abs(worldDir.y) * 0.3f;
        worldDir = glm::normalize(worldDir);
    }
    
    return worldDir;
}

glm::vec3 TerrainTreeGenerator::directionToRotation(glm::vec3 direction) {
    direction = glm::normalize(direction);
    
    // Cylinder is along Y-axis, we need to rotate it to align with direction
    // Use axis-angle representation: rotate around cross product of Y and direction
    glm::vec3 yAxis(0.0f, 1.0f, 0.0f);
    glm::vec3 rotationAxis = glm::cross(yAxis, direction);
    
    // Handle case where direction is parallel to Y-axis
    if (glm::length(rotationAxis) < 0.0001f) {
        if (direction.y > 0.0f) {
            // Already aligned, no rotation
            return glm::vec3(0.0f);
        } else {
            // Opposite direction, rotate 180 degrees around X axis
            return glm::vec3(M_PI, 0.0f, 0.0f); // axis (1,0,0) * angle π
        }
    }
    
    rotationAxis = glm::normalize(rotationAxis);
    float angle = std::acos(glm::dot(yAxis, direction));
    
    // Return axis-angle representation (axis * angle)
    return rotationAxis * angle;
}

Tree TerrainTreeGenerator::convertAxialTreeToTreePieces(const AxialTree& tree, glm::vec3 basePosition) {
    Tree result(basePosition);
    
    for (const auto& segment : tree.allSegments) {
        glm::vec3 segmentCenter = (segment->start + segment->end) * 0.5f;
        glm::vec3 segmentDir = segment->direction;
        
        glm::vec3 position = basePosition + segmentCenter;
        glm::vec3 rotation = directionToRotation(segmentDir);
        glm::vec3 scale(segment->radius * 2.0f, segment->length, segment->radius * 2.0f);
        
        TreePieceData piece(position, rotation, scale);
        result.addPiece(piece);
    }
    
    if (result.getPieces().empty()) {
        glm::vec3 fallbackPos = basePosition + glm::vec3(0.0f, TRUNK_HEIGHT * 0.5f, 0.0f);
        TreePieceData fallback(fallbackPos, glm::vec3(0.0f), glm::vec3(TRUNK_RADIUS * 2.0f, TRUNK_HEIGHT, TRUNK_RADIUS * 2.0f));
        result.addPiece(fallback);
    }
    
    return result;
}

