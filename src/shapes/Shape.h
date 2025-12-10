#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "../utils/scenedata.h"

class Shape {
public:
    Shape(){}

    virtual ~Shape() {}

    virtual int minParam1() const { return 1; }
    virtual int minParam2() const { return 1; }
    virtual void updateParams(int param1, int param2) {
        m_param1 = std::max(param1, minParam1());
        m_param2 = std::max(param2, minParam2());
        setVertexData();
    }

    virtual void setVertexData() = 0;

    const std::vector<float>& getVertexData() const { return m_vertexData; }

protected:
    void insertVec3(std::vector<float> &data, glm::vec3 v) {
        data.push_back(v.x);
        data.push_back(v.y);
        data.push_back(v.z);
    }

    glm::mat4 m_ctm;
    int m_param1 = 1;
    int m_param2 = 1;

    std::vector<float> m_vertexData;

};
