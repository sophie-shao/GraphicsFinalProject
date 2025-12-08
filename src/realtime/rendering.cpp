#include "rendering.h"
#include "../realtime.h"
#include "map/Map.h"
#include "map/mapproperties.h"
#include "utils/scenedata.h"
#include "utils/debug.h"
#include <GL/glew.h>
#include <iostream>

void Rendering::setupMapLights(Realtime* realtime, const glm::vec3& cameraPos, std::vector<SceneLightData>& lights) {
    float baseIntensity = 0.4f;
    float overheadIntensity = baseIntensity * static_cast<float>(realtime->m_overheadLightIntensity);


    //THIS IS JUST FOR ILLUMINATING THE MAP
    SceneLightData light1;
    light1.type = LightType::LIGHT_DIRECTIONAL;
    light1.color = glm::vec4(overheadIntensity, overheadIntensity, overheadIntensity, 1.0f);
    light1.dir = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    light1.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light1.angle = 0.0f;
    light1.penumbra = 0.0f;
    lights.push_back(light1);

    SceneLightData light2;
    light2.type = LightType::LIGHT_DIRECTIONAL;
    light2.color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
    glm::vec3 dir2 = glm::normalize(glm::vec3(1.0f, -0.6f, 0.5f));
    light2.dir = glm::vec4(dir2.x, dir2.y, dir2.z, 0.0f);
    light2.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light2.angle = 0.0f;
    light2.penumbra = 0.0f;
    lights.push_back(light2);

    SceneLightData light3;
    light3.type = LightType::LIGHT_DIRECTIONAL;
    light3.color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
    glm::vec3 dir3 = glm::normalize(glm::vec3(-1.0f, -0.6f, -0.5f));
    light3.dir = glm::vec4(dir3.x, dir3.y, dir3.z, 0.0f);
    light3.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light3.angle = 0.0f;
    light3.penumbra = 0.0f;
    lights.push_back(light3);

    SceneLightData playerLight;
    playerLight.type = LightType::LIGHT_POINT;
    playerLight.color = glm::vec4(realtime->m_playerLightColor, 1.0f);
    playerLight.pos = glm::vec4(cameraPos, 1.0f);
    playerLight.function = glm::vec3(0.3f, 0.2f, 0.1f);
    playerLight.angle = 0.0f;
    playerLight.penumbra = 0.0f;
    lights.push_back(playerLight);
}

void Rendering::renderMapBlocks(Realtime* realtime) {
    if (realtime->m_activeMap == nullptr) {
        return;
    }

    glm::vec3 cameraPos = realtime->m_camera.getPosition();

    std::vector<SceneLightData> lights;
    setupMapLights(realtime, cameraPos, lights);
    realtime->addLightsToShader(lights);
    glUseProgram(realtime->m_shaderProgram);

    int renderDistance = 4;

    auto blocks = realtime->m_activeMap->getBlocksInRenderDistance(cameraPos, renderDistance);

    if (blocks.empty()) {
        blocks = realtime->m_activeMap->getBlocksToRender();

        if (blocks.empty()) {
            static bool errorPrinted = false;
            if (!errorPrinted) {
                std::cout << "Error: Map has no blocks to render! Block count: "
                          << realtime->m_activeMap->getBlockCount() << std::endl;
                errorPrinted = true;
            }
            return;
        }
    }

    const ShapeData &cubeData = realtime->m_shapeManager.getShapeData(PrimitiveType::PRIMITIVE_CUBE);

    for (const auto& block : blocks) {
        int x, y, z;
        BiomeType biome;
        std::tie(x, y, z, biome) = block;

        glm::mat4 ctm = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        glUniformMatrix4fv(realtime->m_modelLoc, 1, GL_FALSE, &ctm[0][0]);

        SceneMaterial mat;
        unsigned char r, g, b;
        MapProperties::getBiomeColor(biome, r, g, b);

        glm::vec3 originalColor = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
        glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 pastelColor = originalColor * 0.5f + white * 0.5f;

        mat.cAmbient = glm::vec4(pastelColor, 1.0f) * 0.3f;
        mat.cDiffuse = glm::vec4(pastelColor, 1.0f);
        mat.cSpecular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        mat.shininess = 0.0f;

        glUniform4fv(realtime->mat_cAmbientLoc, 1, &mat.cAmbient[0]);
        glUniform4fv(realtime->mat_cDiffuseLoc, 1, &mat.cDiffuse[0]);
        glUniform4fv(realtime->mat_cSpecularLoc, 1, &mat.cSpecular[0]);
        glUniform1f(realtime->mat_shinyLoc, mat.shininess);

        glErrorCheck();

        glBindVertexArray(cubeData.vao);
        glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
    }
}
