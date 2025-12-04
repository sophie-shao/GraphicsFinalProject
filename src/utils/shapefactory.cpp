#include <iostream>
#include "shapefactory.h"
#include "../shapes/Cube.h"
#include "../shapes/Sphere.h"
#include "../shapes/Cylinder.h"
#include "../shapes/Cone.h"
#include <memory>


ShapeFactory::ShapeFactory() = default;
ShapeFactory::~ShapeFactory() {}

// Helper to build GPU data from a Shape
ShapeData ShapeFactory::createShapeData(Shape &shape, int param1, int param2) {
    shape.updateParams(param1, param2);
    const auto &verts = shape.getVertexData();

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);


    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    //position (3) + normal (3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return {vao, vbo, static_cast<int>(verts.size()/6)};
}

void ShapeFactory::initShapes(int param1, int param2) {
    m_param1 = param1;
    m_param2 = param2;

    Cube cube;
    Sphere sphere;
    Cylinder cylinder;
    Cone cone;

    m_shapeMap[PrimitiveType::PRIMITIVE_CUBE] =
        createShapeData(cube, param1, param2);

    m_shapeMap[PrimitiveType::PRIMITIVE_SPHERE] =
        createShapeData(sphere, param1, param2);

    m_shapeMap[PrimitiveType::PRIMITIVE_CYLINDER] =
        createShapeData(cylinder, param1, param2);

    m_shapeMap[PrimitiveType::PRIMITIVE_CONE] =
        createShapeData(cone, param1, param2);
}


void ShapeFactory::updateParams(int newParam1, int newParam2) {
    if (m_shapeMap.empty()) {
        return;
    }
    //check to not update every single settings change
    if (newParam1 == m_param1 && newParam2 == m_param2) return;
    initShapes(newParam1, newParam2);
}

void ShapeFactory::destroyShapes() {
    //had to look up syntax for
    if(m_shapeMap.empty()) return;
    for (auto &[type, data] : m_shapeMap) {
        glDeleteBuffers(1, &data.vbo);
        glDeleteVertexArrays(1, &data.vao);
    }
    m_shapeMap.clear();
}

ShapeData& ShapeFactory::getShapeData(PrimitiveType type) {
    return m_shapeMap.at(type);
}


//MAYBE USE FOR ADAPTIVE TESSELATION
// std::vector<std::unique_ptr<Shape>> ShapeFactory::buildShapes(const RenderData &renderData, int param1, int param2) {
//     std::vector<std::unique_ptr<Shape>> shapes;

//     for (const RenderShapeData &shapeData : renderData.shapes) {
//         const ScenePrimitive &prim = shapeData.primitive;
//         const SceneMaterial &mat = prim.material;
//         const glm::mat4 &ctm = shapeData.ctm;

//         std::unique_ptr<Shape> shape;

//         switch (prim.type) {
//         case PrimitiveType::PRIMITIVE_CUBE:
//             shape = std::make_unique<Cube>(ctm, mat);
//             break;
//         case PrimitiveType::PRIMITIVE_SPHERE:
//             shape = std::make_unique<Sphere>(ctm, mat);
//             break;
//         case PrimitiveType::PRIMITIVE_CONE:
//             shape = std::make_unique<Cone>(ctm, mat);
//             break;
//         case PrimitiveType::PRIMITIVE_CYLINDER:
//             shape = std::make_unique<Cylinder>(ctm, mat);
//             break;
//         default:
//             continue;
//         }

//         shape->updateParams(param1,param2);
//         shapes.push_back(std::move(shape));
//     }

//     return shapes;
// }
