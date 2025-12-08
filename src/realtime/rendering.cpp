#include "rendering.h"
#include "../realtime.h"
#include "map/Map.h"
#include "blocks/Block.h"
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

    // Secondary directional light
    SceneLightData light2;
    light2.type = LightType::LIGHT_DIRECTIONAL;
    light2.color = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);
    glm::vec3 dir2 = glm::normalize(glm::vec3(1.0f, -0.6f, 0.5f));
    light2.dir = glm::vec4(dir2.x, dir2.y, dir2.z, 0.0f);
    light2.function = glm::vec3(1.0f, 0.0f, 0.0f);
    light2.angle = 0.0f;
    light2.penumbra = 0.0f;
    lights.push_back(light2);

    // Tertiary directional light
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

void Rendering::renderMapBlocks(Realtime* realtime) {
    if (realtime->m_activeMap == nullptr) {
        return;
    }

    glm::vec3 cameraPos = realtime->m_camera.getPosition();

    std::vector<SceneLightData> lights;
    setupMapLights(realtime, cameraPos, lights);

    glUseProgram(realtime->m_blockShaderProgram);

    glm::mat4 proj = realtime->m_camera.getProjMatrix();
    glm::mat4 view = realtime->m_camera.getViewMatrix();

    glUniformMatrix4fv(realtime->m_blockProjLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(realtime->m_blockViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3fv(realtime->m_blockCameraPosLoc, 1, &cameraPos[0]);

    realtime->addLightsToBlockShader(lights);

    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "k_a"), realtime->m_globalData.ka);
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "k_d"), realtime->m_globalData.kd);
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "k_s"), realtime->m_globalData.ks);

    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "useNormalMap"), realtime->m_useNormalMapping ? 1 : 0);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "useBumpMap"), realtime->m_useBumpMapping ? 1 : 0);
    glUniform1f(glGetUniformLocation(realtime->m_blockShaderProgram, "bumpStrength"), realtime->m_bumpStrength);
    glUniform1i(glGetUniformLocation(realtime->m_blockShaderProgram, "textureType"), 1); // 1 = dirt

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, realtime->m_dirtNormalMap);
    GLint normalMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "normalMap");
    if (normalMapLoc == -1) {
        std::cerr << "ERROR: 'normalMap' uniform not found in shader!" << std::endl;
    } else {
        glUniform1i(normalMapLoc, 0);
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, realtime->m_dirtBumpMap);
    GLint bumpMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "bumpMap");
    if (bumpMapLoc == -1) {
        std::cerr << "ERROR: 'bumpMap' uniform not found in shader!" << std::endl;
    } else {
        glUniform1i(bumpMapLoc, 1);
    }

    int renderDistance = 4;
    auto blocks = realtime->m_activeMap->getBlocksInRenderDistance(cameraPos, renderDistance);

    if (blocks.empty()) {
        blocks = realtime->m_activeMap->getBlocksToRender();
        if (blocks.empty()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(0);
            return;
        }
    }

    if (realtime->m_blockVAO == 0) {

        Block block;
        const auto& vertexData = block.getVertexData();

        glGenVertexArrays(1, &realtime->m_blockVAO);
        glGenBuffers(1, &realtime->m_blockVBO);

        glBindVertexArray(realtime->m_blockVAO);
        glBindBuffer(GL_ARRAY_BUFFER, realtime->m_blockVBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                     vertexData.data(), GL_STATIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(3 * sizeof(float)));

        // Tangent
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(6 * sizeof(float)));

        // Bitangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(9 * sizeof(float)));

        // UV
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(12 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        realtime->m_blockVertexCount = block.getVertexCount();
    }

    glBindVertexArray(realtime->m_blockVAO);

    for (const auto& block : blocks) {
        int x, y, z;
        BiomeType biome;
        std::tie(x, y, z, biome) = block;

        glm::mat4 ctm = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        glUniformMatrix4fv(realtime->m_blockModelLoc, 1, GL_FALSE, &ctm[0][0]);

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

        GLint ambientLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cAmbient");
        GLint diffuseLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cDiffuse");
        GLint specularLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cSpecular");
        GLint shinyLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.shininess");

        if (ambientLoc != -1) glUniform4fv(ambientLoc, 1, &mat.cAmbient[0]);
        if (diffuseLoc != -1) glUniform4fv(diffuseLoc, 1, &mat.cDiffuse[0]);
        if (specularLoc != -1) glUniform4fv(specularLoc, 1, &mat.cSpecular[0]);
        if (shinyLoc != -1) glUniform1f(shinyLoc, mat.shininess);

        glDrawArrays(GL_TRIANGLES, 0, realtime->m_blockVertexCount);
    }

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
    glErrorCheck();
}
