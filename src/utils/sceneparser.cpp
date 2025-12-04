#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    SceneNode* root = fileReader.getRootNode();
    traverseSceneGraph(root, glm::mat4(1.0f), renderData);

    // TODO: Use your Lab 5 code here

    return true;
}


void SceneParser::traverseSceneGraph(SceneNode* node, const glm::mat4 &parentCTM,
                                     RenderData &renderData) {
    glm::mat4 currentCTM = parentCTM;

    for (const SceneTransformation* trans : node->transformations) {
        glm::mat4 M(1.0f);
        switch (trans->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            M = glm::translate(glm::mat4(1.0f), trans->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            M = glm::scale(glm::mat4(1.0f), trans->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            M = glm::rotate(glm::mat4(1.0f), trans->angle, trans->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            M = trans->matrix;
            break;
        }
        currentCTM = currentCTM * M;
    }

    for (const ScenePrimitive* prim : node->primitives) {
        RenderShapeData shape;
        shape.primitive = *prim;
        shape.ctm = currentCTM;
        renderData.shapes.push_back(shape);
    }

    for (const SceneLight* light : node->lights) {
        SceneLightData newLight;

        newLight.id = light->id;
        newLight.type = light->type;
        newLight.color = light->color;
        newLight.function = light->function;
        newLight.penumbra = light->penumbra;
        newLight.angle = light->angle;
        newLight.width = light->width;
        newLight.height = light->height;

        switch (light->type) {

        case LightType::LIGHT_DIRECTIONAL: {
            glm::vec4 transformedDir = currentCTM * glm::vec4(light->dir);
            newLight.dir = glm::normalize(transformedDir);
            break;
        }

        case LightType::LIGHT_POINT: {
            glm::vec4 transformedPos = currentCTM * glm::vec4(0,0,0,1);
            newLight.pos = transformedPos;
            break;
        }

        case LightType::LIGHT_SPOT: {
            glm::vec4 transformedPos = currentCTM * glm::vec4(0,0,0,1);
            glm::vec4 transformedDir = currentCTM * glm::vec4(light->dir);
            newLight.pos = transformedPos;
            newLight.dir = glm::normalize(transformedDir);
            break;
        }

        default:
            continue;
        }

        renderData.lights.push_back(newLight);


    }

    for (SceneNode* child : node->children) {
        traverseSceneGraph(child, currentCTM, renderData);
    }
}

