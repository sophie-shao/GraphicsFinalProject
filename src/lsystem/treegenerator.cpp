#include "treegenerator.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
static std::uniform_real_distribution<float> angleDis(0.0f, 360.0f);

glm::vec3 rotateVector(glm::vec3 vec, glm::vec3 axis, float angle) {
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis));
    return glm::vec3(rotation * glm::vec4(vec, 1.0f));
}

AxialTree TreeGenerator::generateTree(const TreeParameters& params) {
    switch (params.treeType) {
        case TreeType::DEAD_PINE:
            return generateDeadPine(params);
        case TreeType::DEAD_OAK:
            return generateDeadOak(params);
        case TreeType::DEAD_BIRCH:
            return generateDeadBirch(params);
        default:
            return generateDeadPine(params);
    }
}

AxialTree TreeGenerator::generateDeadPine(const TreeParameters& params) {
    AxialTree tree;
    
    glm::vec3 trunkStart(0.0f, 0.0f, 0.0f);
    glm::vec3 trunkDirection(0.0f, 1.0f, 0.0f);
    
    int trunkSegments = params.numBranchLayers + 1;
    float segmentLength = params.trunkLength / trunkSegments;
    
    for (int i = 0; i < trunkSegments; ++i) {
        float heightRatio = static_cast<float>(i) / static_cast<float>(trunkSegments);
        float radiusReduction = 1.0f - (heightRatio * 0.8f);
        float currentRadius = params.trunkRadius * radiusReduction;
        currentRadius = glm::max(0.01f, currentRadius);
        
        auto trunkSegment = std::make_shared<Segment>(
            trunkStart,
            trunkDirection,
            segmentLength,
            currentRadius,
            0,
            true,
            false
        );
        
        tree.rootBranch->mainAxis->addSegment(trunkSegment);
        trunkStart = trunkSegment->end;
    }
    
    if (params.numBranchLayers > 0 && params.branchesPerLayer > 0) {
        float sectionLength = params.trunkLength / (params.numBranchLayers + 1);
        
        for (int layer = 1; layer <= params.numBranchLayers; ++layer) {
            float branchY = sectionLength * layer;
            float heightRatio = branchY / params.trunkLength;
            glm::vec3 branchPoint(0.0f, branchY, 0.0f);
            
            float lengthReduction = 1.0f - (heightRatio * 0.9f);
            float branchLength = params.branchLength * lengthReduction;
            
            for (int b = 0; b < params.branchesPerLayer; ++b) {
                float angle = (360.0f / params.branchesPerLayer) * b;
                float angleRad = glm::radians(angle);
                
                glm::vec3 branchDir(
                    std::cos(angleRad),
                    0.0f,
                    std::sin(angleRad)
                );
                
                float branchRadius = params.trunkRadius * 0.5f;
                
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
                
                if (params.branchDepth > 0) {
                    generateSubBranches(
                        branchPoint + branchDir * branchLength,
                        branchDir,
                        params,
                        1,
                        heightRatio,
                        tree.rootBranch
                    );
                }
            }
        }
    }
    
    tree.collectAllSegments();
    return tree;
}

AxialTree TreeGenerator::generateDeadOak(const TreeParameters& params) {
    AxialTree tree;
    
    glm::vec3 trunkStart(0.0f, 0.0f, 0.0f);
    glm::vec3 trunkDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 currentTrunkDir = trunkDirection;
    
    if (params.numBranchLayers > 0 && params.branchesPerLayer > 0) {
        float sectionLength = params.trunkLength / (params.numBranchLayers + 1);
        
        for (int layer = 1; layer <= params.numBranchLayers; ++layer) {
            float heightRatio = static_cast<float>(layer - 1) / static_cast<float>(params.numBranchLayers);
            float radiusReduction = 1.0f - (heightRatio * 0.7f);
            float currentRadius = params.trunkRadius * radiusReduction;
            currentRadius = glm::max(0.01f, currentRadius);
            
            auto trunkSegment = std::make_shared<Segment>(
                trunkStart,
                currentTrunkDir,
                sectionLength,
                currentRadius,
                0,
                true,
                false
            );
            
            tree.rootBranch->mainAxis->addSegment(trunkSegment);
            trunkStart = trunkSegment->end;
            
            float branchHeightRatio = static_cast<float>(layer) / static_cast<float>(params.numBranchLayers + 1);
            float lengthReduction = 1.0f - (branchHeightRatio * 0.85f);
            float branchLength = params.branchLength * lengthReduction * 1.2f;
            
            for (int b = 0; b < params.branchesPerLayer; ++b) {
                float baseAngle = (360.0f / params.branchesPerLayer) * b;
                float angleVariation = dis(gen) * 25.0f;
                float angle = baseAngle + angleVariation;
                float angleRad = glm::radians(angle);
                
                float verticalAngle = 30.0f + dis(gen) * 20.0f;
                float verticalRad = glm::radians(verticalAngle);
                
                glm::vec3 branchDir(
                    std::cos(angleRad) * std::cos(verticalRad),
                    std::sin(verticalRad),
                    std::sin(angleRad) * std::cos(verticalRad)
                );
                branchDir = glm::normalize(branchDir);
                
                float branchRadius = params.trunkRadius * 0.6f;
                
                auto branchSegment = std::make_shared<Segment>(
                    trunkStart,
                    branchDir,
                    branchLength,
                    branchRadius,
                    1,
                    false,
                    true
                );
                
                tree.rootBranch->mainAxis->addSegment(branchSegment);
                
                if (params.branchDepth > 0) {
                    generateOakSubBranches(
                        trunkStart + branchDir * branchLength,
                        branchDir,
                        params,
                        1,
                        branchHeightRatio,
                        tree.rootBranch
                    );
                }
            }
            
            if (layer < params.numBranchLayers) {
                float trunkConeAngle = 3.0f + dis(gen) * 8.0f;
                float trunkAzimuthAngle = angleDis(gen);
                float trunkConeRad = glm::radians(trunkConeAngle);
                float trunkAzimuthRad = glm::radians(trunkAzimuthAngle);
                
                glm::vec3 trunkPerpAxis = glm::normalize(glm::cross(currentTrunkDir, glm::vec3(0.0f, 1.0f, 0.0f)));
                if (glm::length(trunkPerpAxis) < 0.001f) {
                    trunkPerpAxis = glm::vec3(1.0f, 0.0f, 0.0f);
                }
                
                currentTrunkDir = rotateVector(currentTrunkDir, trunkPerpAxis, trunkConeRad);
                glm::vec3 trunkUpAxis = glm::normalize(glm::cross(trunkPerpAxis, currentTrunkDir));
                currentTrunkDir = rotateVector(currentTrunkDir, trunkUpAxis, trunkAzimuthRad);
                currentTrunkDir = glm::normalize(currentTrunkDir);
            }
        }
        
        float finalHeightRatio = static_cast<float>(params.numBranchLayers) / static_cast<float>(params.numBranchLayers + 1);
        float finalRadius = params.trunkRadius * (1.0f - (finalHeightRatio * 0.7f));
        finalRadius = glm::max(0.01f, finalRadius);
        
        auto finalTrunkSegment = std::make_shared<Segment>(
            trunkStart,
            currentTrunkDir,
            sectionLength,
            finalRadius,
            0,
            true,
            false
        );
        
        tree.rootBranch->mainAxis->addSegment(finalTrunkSegment);
    } else {
        int trunkSegments = params.numBranchLayers + 1;
        if (trunkSegments == 0) {
            trunkSegments = 1;
        }
        float segmentLength = params.trunkLength / trunkSegments;
        
        for (int i = 0; i < trunkSegments; ++i) {
            float heightRatio = static_cast<float>(i) / static_cast<float>(trunkSegments);
            float radiusReduction = 1.0f - (heightRatio * 0.7f);
            float currentRadius = params.trunkRadius * radiusReduction;
            currentRadius = glm::max(0.01f, currentRadius);
            
            auto trunkSegment = std::make_shared<Segment>(
                trunkStart,
                trunkDirection,
                segmentLength,
                currentRadius,
                0,
                true,
                false
            );
            
            tree.rootBranch->mainAxis->addSegment(trunkSegment);
            trunkStart = trunkSegment->end;
        }
    }
    
    tree.collectAllSegments();
    return tree;
}

AxialTree TreeGenerator::generateDeadBirch(const TreeParameters& params) {
    AxialTree tree;
    
    glm::vec3 trunkStart(0.0f, 0.0f, 0.0f);
    glm::vec3 trunkDirection(0.0f, 1.0f, 0.0f);
    
    int trunkSegments = params.numBranchLayers + 1;
    float segmentLength = params.trunkLength / trunkSegments;
    
    for (int i = 0; i < trunkSegments; ++i) {
        float heightRatio = static_cast<float>(i) / static_cast<float>(trunkSegments);
        float radiusReduction = 1.0f - (heightRatio * 0.9f);
        float currentRadius = params.trunkRadius * radiusReduction * 0.7f;
        currentRadius = glm::max(0.01f, currentRadius);
        
        auto trunkSegment = std::make_shared<Segment>(
            trunkStart,
            trunkDirection,
            segmentLength,
            currentRadius,
            0,
            true,
            false
        );
        
        tree.rootBranch->mainAxis->addSegment(trunkSegment);
        trunkStart = trunkSegment->end;
    }
    
    if (params.numBranchLayers > 0 && params.branchesPerLayer > 0) {
        float sectionLength = params.trunkLength / (params.numBranchLayers + 1);
        
        for (int layer = 1; layer <= params.numBranchLayers; ++layer) {
            float branchY = sectionLength * layer;
            float heightRatio = branchY / params.trunkLength;
            glm::vec3 branchPoint(0.0f, branchY, 0.0f);
            
            float lengthReduction = 1.0f - (heightRatio * 0.95f);
            float branchLength = params.branchLength * lengthReduction * 0.8f;
            
            for (int b = 0; b < params.branchesPerLayer; ++b) {
                float angle = (360.0f / params.branchesPerLayer) * b;
                float angleRad = glm::radians(angle);
                
                float verticalAngle = 10.0f + dis(gen) * 15.0f;
                float verticalRad = glm::radians(verticalAngle);
                
                glm::vec3 branchDir(
                    std::cos(angleRad) * std::cos(verticalRad),
                    std::sin(verticalRad),
                    std::sin(angleRad) * std::cos(verticalRad)
                );
                branchDir = glm::normalize(branchDir);
                
                float branchRadius = params.trunkRadius * 0.4f;
                
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
                
                if (params.branchDepth > 0) {
                    generateSubBranches(
                        branchPoint + branchDir * branchLength,
                        branchDir,
                        params,
                        1,
                        heightRatio,
                        tree.rootBranch
                    );
                }
            }
        }
    }
    
    tree.collectAllSegments();
    return tree;
}

void TreeGenerator::generateSubBranches(glm::vec3 startPos,
                                         glm::vec3 parentDir,
                                         const TreeParameters& params,
                                         int currentDepth,
                                         float heightRatio,
                                         std::shared_ptr<Branch> parentBranch) {
    if (currentDepth >= params.branchDepth) {
        return;
    }
    
    float heightReduction = 1.0f - (heightRatio * 0.9f);
    float depthReduction = std::pow(0.7f, currentDepth);
    float branchLength = params.branchLength * heightReduction * depthReduction;
    float branchRadius = params.trunkRadius * 0.5f * depthReduction;
    
    int numSubBranches = 2;
    if (currentDepth == 0) {
        numSubBranches = 2;
    }
    
    for (int b = 0; b < numSubBranches; ++b) {
        float coneAngle = params.subBranchAngle * (0.3f + dis(gen) * 0.4f);
        float azimuthAngle = angleDis(gen);
        
        glm::vec3 perpAxis = glm::normalize(glm::cross(parentDir, glm::vec3(0.0f, 1.0f, 0.0f)));
        if (glm::length(perpAxis) < 0.001f) {
            perpAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        
        glm::vec3 branchDir = rotateVector(parentDir, perpAxis, glm::radians(coneAngle));
        
        float tiltAngle = dis(gen) * 15.0f;
        branchDir = rotateVector(branchDir, parentDir, glm::radians(tiltAngle));
        branchDir = glm::normalize(branchDir);
        
        branchDir.y *= 0.2f;
        branchDir = glm::normalize(branchDir);
        
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
        
        generateSubBranches(
            startPos + branchDir * branchLength,
            branchDir,
            params,
            currentDepth + 1,
            heightRatio,
            parentBranch
        );
    }
}

void TreeGenerator::generateOakSubBranches(glm::vec3 startPos,
                                         glm::vec3 parentDir,
                                         const TreeParameters& params,
                                         int currentDepth,
                                         float heightRatio,
                                         std::shared_ptr<Branch> parentBranch) {
    if (currentDepth >= params.branchDepth) {
        return;
    }
    
    float heightReduction = 1.0f - (heightRatio * 0.85f);
    float depthReduction = std::pow(0.7f, currentDepth);
    float branchLength = params.branchLength * heightReduction * depthReduction * 1.2f;
    float branchRadius = params.trunkRadius * 0.6f * depthReduction;
    
    int numSubBranches = 2;
    
    float upwardAngle = 5.0f + (static_cast<float>(currentDepth) * 5.0f);
    upwardAngle = glm::min(upwardAngle, 35.0f);
    float upwardAngleRad = glm::radians(upwardAngle);
    
    for (int b = 0; b < numSubBranches; ++b) {
        float coneAngle = params.subBranchAngle * (0.3f + dis(gen) * 0.4f);
        
        glm::vec3 perpAxis = glm::normalize(glm::cross(parentDir, glm::vec3(0.0f, 1.0f, 0.0f)));
        if (glm::length(perpAxis) < 0.001f) {
            perpAxis = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        
        glm::vec3 branchDir = rotateVector(parentDir, perpAxis, glm::radians(coneAngle));
        
        float tiltAngle = dis(gen) * 15.0f;
        branchDir = rotateVector(branchDir, parentDir, glm::radians(tiltAngle));
        branchDir = glm::normalize(branchDir);
        
        glm::vec3 upVector(0.0f, 1.0f, 0.0f);
        glm::vec3 rightVector = glm::normalize(glm::cross(branchDir, upVector));
        if (glm::length(rightVector) < 0.001f) {
            rightVector = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        
        glm::mat4 upwardRotation = glm::rotate(glm::mat4(1.0f), upwardAngleRad, rightVector);
        branchDir = glm::normalize(glm::vec3(upwardRotation * glm::vec4(branchDir, 0.0f)));
        
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
        
        generateOakSubBranches(
            startPos + branchDir * branchLength,
            branchDir,
            params,
            currentDepth + 1,
            heightRatio,
            parentBranch
        );
    }
}
