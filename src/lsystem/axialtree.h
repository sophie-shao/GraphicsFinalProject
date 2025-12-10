#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

struct Segment {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 direction;
    float length;
    float radius;
    int order;
    bool isStraight;
    bool isLateral;
    
    Segment(glm::vec3 start, glm::vec3 direction, float length, float radius, int order, bool isStraight, bool isLateral)
        : start(start), direction(glm::normalize(direction)), length(length), radius(radius), order(order), isStraight(isStraight), isLateral(isLateral)
    {
        end = start + this->direction * length;
    }
};

struct Node {
    glm::vec3 position;
    int order;
    std::vector<std::shared_ptr<Segment>> outgoingSegments;
    std::shared_ptr<Segment> straightSegment;
    
    Node(glm::vec3 pos, int ord) : position(pos), order(ord) {}
};

class Axis {
public:
    std::vector<std::shared_ptr<Segment>> segments;
    int order;
    std::shared_ptr<Node> rootNode;
    
    Axis(int ord) : order(ord) {}
    
    void addSegment(std::shared_ptr<Segment> segment) {
        segments.push_back(segment);
    }
    
    std::shared_ptr<Node> getTerminalNode() {
        if (segments.empty()) return rootNode;
        return std::make_shared<Node>(segments.back()->end, order);
    }
};

class Branch {
public:
    std::shared_ptr<Axis> mainAxis;
    std::vector<std::shared_ptr<Branch>> subBranches;
    int order;
    
    Branch(int ord) : order(ord) {
        mainAxis = std::make_shared<Axis>(ord);
    }
};

class AxialTree {
public:
    std::shared_ptr<Branch> rootBranch;
    std::vector<std::shared_ptr<Segment>> allSegments;
    
    AxialTree() {
        rootBranch = std::make_shared<Branch>(0);
        rootBranch->mainAxis->rootNode = std::make_shared<Node>(glm::vec3(0.0f), 0);
    }
    
    void collectAllSegments() {
        allSegments.clear();
        collectSegmentsFromBranch(rootBranch);
    }
    
private:
    void collectSegmentsFromBranch(std::shared_ptr<Branch> branch) {
        if (!branch || !branch->mainAxis) return;
        
        for (auto& segment : branch->mainAxis->segments) {
            allSegments.push_back(segment);
        }
        
        if (branch->mainAxis->rootNode) {
            for (auto& lateralSegment : branch->mainAxis->rootNode->outgoingSegments) {
                allSegments.push_back(lateralSegment);
            }
        }
        
        for (auto& subBranch : branch->subBranches) {
            collectSegmentsFromBranch(subBranch);
        }
    }
};

