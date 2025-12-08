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

    // Overhead directional light
    SceneLightData light1;
    light1.type = LightType::LIGHT_DIRECTIONAL;
    light1.color = glm::vec4(overheadIntensity, overheadIntensity, overheadIntensity, 1.0f);
    light1.dir = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    light1.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light1.angle = 0.0f;
    light1.penumbra = 0.0f;
    lights.push_back(light1);

    // Side directional light 1
    SceneLightData light2;
    light2.type = LightType::LIGHT_DIRECTIONAL;
    light2.color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
    glm::vec3 dir2 = glm::normalize(glm::vec3(1.0f, -0.6f, 0.5f));
    light2.dir = glm::vec4(dir2.x, dir2.y, dir2.z, 0.0f);
    light2.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light2.angle = 0.0f;
    light2.penumbra = 0.0f;
    lights.push_back(light2);

    // Side directional light 2
    SceneLightData light3;
    light3.type = LightType::LIGHT_DIRECTIONAL;
    light3.color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
    glm::vec3 dir3 = glm::normalize(glm::vec3(-1.0f, -0.6f, -0.5f));
    light3.dir = glm::vec4(dir3.x, dir3.y, dir3.z, 0.0f);
    light3.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light3.angle = 0.0f;
    light3.penumbra = 0.0f;
    lights.push_back(light3);

    // Player point light
    SceneLightData playerLight;
    playerLight.type = LightType::LIGHT_POINT;
    playerLight.color = glm::vec4(realtime->m_playerLightColor, 1.0f);
    playerLight.pos = glm::vec4(cameraPos, 1.0f);
    playerLight.function = glm::vec3(0.3f, 0.2f, 0.1f);
    playerLight.angle = 0.0f;
    playerLight.penumbra = 0.0f;
    lights.push_back(playerLight);
}

void Rendering::addLightsToBlockShader(Realtime* realtime, const std::vector<SceneLightData>& lights) {
    int numLights = std::min((int)lights.size(), 8);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "numLights"), numLights);

    for (int i = 0; i < numLights; i++) {
        const SceneLightData &L = lights[i];
        std::string base = "lights[" + std::to_string(i) + "]";

        int type;
        if (L.type == LightType::LIGHT_DIRECTIONAL) {
            type = 0;
        } else if (L.type == LightType::LIGHT_POINT) {
            type = 1;
        } else {
            type = 2;
        }
        glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".type").c_str()), type);
        glUniform4fv(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".color").c_str()), 1, &L.color[0]);
        glUniform3fv(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".function").c_str()), 1, &L.function[0]);
        glUniform4fv(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".pos").c_str()), 1, &L.pos[0]);
        glUniform4fv(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".dir").c_str()), 1, &L.dir[0]);
        glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".penumbra").c_str()), L.penumbra);
        glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, (base + ".angle").c_str()), L.angle);
    }
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

void Rendering::renderMapBlocksWithBumpMapping(Realtime* realtime) {
    if (realtime->m_activeMap == nullptr) {
        return;
    }

    // Check if block shader is valid
    if (realtime->m_blockShaderProgram == 0) {
        std::cerr << "Block shader program is invalid, skipping bump-mapped rendering" << std::endl;
        return;
    }

    glUseProgram(realtime->m_blockShaderProgram);
    glErrorCheck();

    glm::vec3 cameraPos = realtime->m_camera.getPosition();
    glm::mat4 proj = realtime->m_camera.getProjMatrix();
    glm::mat4 view = realtime->m_camera.getViewMatrix();

    // Set camera matrices
    glUniformMatrix4fv(realtime->m_blockProjLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(realtime->m_blockViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3fv(realtime->m_blockCameraPosLoc, 1, &cameraPos[0]);

    // Set lighting coefficients
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "ka"), realtime->m_globalData.ka);
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "kd"), realtime->m_globalData.kd);
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "ks"), realtime->m_globalData.ks);

    // Setup lights
    std::vector<SceneLightData> lights;
    setupMapLights(realtime, cameraPos, lights);
    addLightsToBlockShader(realtime, lights);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, realtime->m_colorTexture);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "colorTexture"), 0);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "useColorTexture"), 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, realtime->m_normalMapTexture);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "useNormalMap"), realtime->m_useNormalMapping ? 1 : 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, realtime->m_bumpMapTexture);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "bumpMap"), 2);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "useBumpMap"), realtime->m_useBumpMapping ? 1 : 0);
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "bumpStrength"), realtime->m_bumpStrength);

    // Get blocks to render
    int renderDistance = 4;
    auto blocks = realtime->m_activeMap->getBlocksInRenderDistance(cameraPos, renderDistance);

    if (blocks.empty()) {
        blocks = realtime->m_activeMap->getBlocksToRender();
        if (blocks.empty()) {
            return;
        }
    }

    const ShapeData &cubeData = realtime->m_shapeManager.getShapeData(PrimitiveType::PRIMITIVE_CUBE);

    // Render each block
    for (const auto& block : blocks) {
        int x, y, z;
        BiomeType biome;
        std::tie(x, y, z, biome) = block;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        glUniformMatrix4fv(realtime->m_blockModelLoc, 1, GL_FALSE, &model[0][0]);

        // Set material properties
        unsigned char r, g, b;
        MapProperties::getBiomeColor(biome, r, g, b);

        glm::vec3 originalColor = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
        glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 pastelColor = originalColor * 0.5f + white * 0.5f;

        glm::vec4 ambient = glm::vec4(pastelColor, 1.0f) * 0.3f;
        glm::vec4 diffuse = glm::vec4(pastelColor, 1.0f);
        glm::vec4 specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float shininess = 1.0f;

        glUniform4fv(glGetUniformLocation(realtime->m_blockShaderProgram, "materialAmbient"), 1, &ambient[0]);
        glUniform4fv(glGetUniformLocation(realtime->m_blockShaderProgram, "materialDiffuse"), 1, &diffuse[0]);
        glUniform4fv(glGetUniformLocation(realtime->m_blockShaderProgram, "materialSpecular"), 1, &specular[0]);
        glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "materialShininess"), shininess);

        glBindVertexArray(cubeData.vao);
        glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}
