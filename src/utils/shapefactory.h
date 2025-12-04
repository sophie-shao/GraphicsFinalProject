#pragma once
#include "../shapes/Shape.h"
// #include "../shapes/Cube.h"
// #include "../shapes/Sphere.h"
// #include "../shapes/Cone.h"
// #include "../shapes/Cylinder.h"
// #include "sceneparser.h"
// #include <memory>
// #include <vector>

#include <GL/glew.h>
#include <unordered_map>
#include "../utils/scenedata.h"

struct ShapeData {
    GLuint vao;
    GLuint vbo;
    int numVertices;
};

class ShapeFactory {
public:
    ShapeFactory();~ShapeFactory();

    void initShapes(int param1, int param2);
    void destroyShapes();

    //get VAO/VBO data for a specific primitive type
    ShapeData& getShapeData(PrimitiveType type);
    void updateParams(int param1, int param2);

private:
    ShapeData createShapeData(Shape &shape, int param1, int param2);
    std::unordered_map<PrimitiveType, ShapeData> m_shapeMap;
    int m_param1 = 1;
    int m_param2 = 1;
};



//FROM V1
// class ShapeFactory {
// public:
//     static std::vector<std::unique_ptr<Shape>> buildShapes(const RenderData &renderData, int param1, int param2);
// };
