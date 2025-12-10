#include "rendering.h"
#include "../realtime.h"
#include "map/Map.h"
#include "map/mapproperties.h"
#include "map/Tree.h"
#include "map/CompletionCube.h"
#include "blocks/Block.h"
#include "blocks/TreePiece.h"
#include "utils/scenedata.h"
#include "utils/debug.h"
#include "utils/audiomanager.h"
#include <GL/glew.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <QStandardPaths>
#include <QFile>

void Rendering::setupMapLights(Realtime* realtime, const glm::vec3& cameraPos, std::vector<SceneLightData>& lights) {
    float baseIntensity = 0.4f;
    float overheadIntensity = baseIntensity * static_cast<float>(realtime->m_overheadLightIntensity);
    
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
    
    if (realtime->m_blockShaderProgram != 0) {
        glUseProgram(realtime->m_blockShaderProgram);
        
        glm::mat4 proj = realtime->m_camera.getProjMatrix();
        glm::mat4 view = realtime->m_camera.getViewMatrix();
        
        if (realtime->m_blockProjLoc >= 0) {
            glUniformMatrix4fv(realtime->m_blockProjLoc, 1, GL_FALSE, &proj[0][0]);
        }
        if (realtime->m_blockViewLoc >= 0) {
            glUniformMatrix4fv(realtime->m_blockViewLoc, 1, GL_FALSE, &view[0][0]);
        }
        if (realtime->m_blockCameraPosLoc >= 0) {
            glUniform3fv(realtime->m_blockCameraPosLoc, 1, &cameraPos[0]);
        }
        
        realtime->addLightsToBlockShader(lights);
        
        if (realtime->m_normalMapTexture != 0) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, realtime->m_normalMapTexture);
            GLint normalMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "normalMap");
            GLint useNormalMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "useNormalMap");
            if (normalMapLoc >= 0) {
                glUniform1i(normalMapLoc, 2);
            }
            if (useNormalMapLoc >= 0) {
                glUniform1i(useNormalMapLoc, realtime->m_useNormalMapping ? 1 : 0);
            }
        }
        
        if (realtime->m_bumpMapTexture != 0) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, realtime->m_bumpMapTexture);
            GLint bumpMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "bumpMap");
            GLint useBumpMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "useBumpMap");
            GLint bumpStrengthLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "bumpStrength");
            if (bumpMapLoc >= 0) {
                glUniform1i(bumpMapLoc, 3);
            }
            if (useBumpMapLoc >= 0) {
                glUniform1i(useBumpMapLoc, realtime->m_useBumpMapping ? 1 : 0);
            }
            if (bumpStrengthLoc >= 0) {
                glUniform1f(bumpStrengthLoc, realtime->m_bumpStrength);
            }
        }
    } else {
        realtime->addLightsToShader(lights);
        glUseProgram(realtime->m_shaderProgram);
        
        glm::mat4 proj = realtime->m_camera.getProjMatrix();
        glm::mat4 view = realtime->m_camera.getViewMatrix();
        
        if (realtime->m_projLoc >= 0) {
            glUniformMatrix4fv(realtime->m_projLoc, 1, GL_FALSE, &proj[0][0]);
        }
        if (realtime->m_viewLoc >= 0) {
            glUniformMatrix4fv(realtime->m_viewLoc, 1, GL_FALSE, &view[0][0]);
        }
        if (realtime->m_cameraPosLoc >= 0) {
            glUniform3fv(realtime->m_cameraPosLoc, 1, &cameraPos[0]);
        }
    }
    
    int renderDistance = 4;
    auto blocks = realtime->m_activeMap->getBlocksInRenderDistance(cameraPos, renderDistance);
    
    if (blocks.empty()) {
        blocks = realtime->m_activeMap->getBlocksToRender();
        if (blocks.empty()) {
            return;
        }
    }
    
    if (realtime->m_blockShaderProgram != 0 && realtime->m_blockVAO == 0) {
        Block block;
        const auto& vertexData = block.getVertexData();
        
        glGenVertexArrays(1, &realtime->m_blockVAO);
        glGenBuffers(1, &realtime->m_blockVBO);
        
        glBindVertexArray(realtime->m_blockVAO);
        glBindBuffer(GL_ARRAY_BUFFER, realtime->m_blockVBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                     vertexData.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(3 * sizeof(float)));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(6 * sizeof(float)));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(9 * sizeof(float)));
        
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(12 * sizeof(float)));
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        realtime->m_blockVertexCount = block.getVertexCount();
    }
    
    // Use block VAO if available, otherwise fallback to cube
    GLuint targetVAO = 0;
    int vertexCount = 0;
    
    if (realtime->m_blockShaderProgram != 0 && realtime->m_blockVAO != 0) {
        targetVAO = realtime->m_blockVAO;
        vertexCount = realtime->m_blockVertexCount;
        glBindVertexArray(targetVAO);
    } else {
        const ShapeData &cubeData = realtime->m_shapeManager.getShapeData(PrimitiveType::PRIMITIVE_CUBE);
        targetVAO = cubeData.vao;
        vertexCount = cubeData.numVertices;
        glBindVertexArray(targetVAO);
    }
    
    for (const auto& block : blocks) {
        int x, y, z;
        BiomeType biome;
        std::tie(x, y, z, biome) = block;
        
        glm::mat4 ctm = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
        
        if (realtime->m_blockShaderProgram != 0) {
            // Bind texture based on biome
            glActiveTexture(GL_TEXTURE0);
            if (biome == BIOME_FIELD && realtime->m_sandTexture != 0) {
                glBindTexture(GL_TEXTURE_2D, realtime->m_sandTexture);
            } else if (realtime->m_colorTexture != 0) {
                glBindTexture(GL_TEXTURE_2D, realtime->m_colorTexture);
            }
            GLint colorTextureLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "colorTexture");
            GLint useColorTextureLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "useColorTexture");
            if (colorTextureLoc >= 0) {
                glUniform1i(colorTextureLoc, 0);
            }
            if (useColorTextureLoc >= 0) {
                glUniform1i(useColorTextureLoc, 1);
            }
            
            // Use block shader
            if (realtime->m_blockModelLoc >= 0) {
                glUniformMatrix4fv(realtime->m_blockModelLoc, 1, GL_FALSE, &ctm[0][0]);
            }
            
            SceneMaterial mat;
            unsigned char r, g, b;
            MapProperties::getBiomeColor(biome, r, g, b);
            
            glm::vec3 originalColor = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
            glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 pastelColor = originalColor * 0.5f + white * 0.5f;
            
            mat.cAmbient = glm::vec4(pastelColor, 1.0f) * 0.3f;
            mat.cDiffuse = glm::vec4(pastelColor, 1.0f);
            if (biome == BIOME_FIELD) {
                mat.cSpecular = glm::vec4(0.1f, 0.1f, 0.08f, 1.0f);
                mat.shininess = 8.0f;
            } else {
                mat.cSpecular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                mat.shininess = 0.0f;
            }
            
            GLint ambientLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cAmbient");
            GLint diffuseLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cDiffuse");
            GLint specularLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cSpecular");
            GLint shinyLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.shininess");
            
            if (ambientLoc >= 0) {
                glUniform4fv(ambientLoc, 1, &mat.cAmbient[0]);
            }
            if (diffuseLoc >= 0) {
                glUniform4fv(diffuseLoc, 1, &mat.cDiffuse[0]);
            }
            if (specularLoc >= 0) {
                glUniform4fv(specularLoc, 1, &mat.cSpecular[0]);
            }
            if (shinyLoc >= 0) {
                glUniform1f(shinyLoc, mat.shininess);
            }
        } else {
            // Fallback to regular shader
            if (realtime->m_modelLoc >= 0) {
                glUniformMatrix4fv(realtime->m_modelLoc, 1, GL_FALSE, &ctm[0][0]);
            }
            
            SceneMaterial mat;
            unsigned char r, g, b;
            MapProperties::getBiomeColor(biome, r, g, b);
            
            glm::vec3 originalColor = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
            glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 pastelColor = originalColor * 0.5f + white * 0.5f;
            
            mat.cAmbient = glm::vec4(pastelColor, 1.0f) * 0.3f;
            mat.cDiffuse = glm::vec4(pastelColor, 1.0f);
            if (biome == BIOME_FIELD) {
                mat.cSpecular = glm::vec4(0.1f, 0.1f, 0.08f, 1.0f);
                mat.shininess = 8.0f;
            } else {
                mat.cSpecular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                mat.shininess = 0.0f;
            }
            
            if (realtime->mat_cAmbientLoc >= 0) {
                glUniform4fv(realtime->mat_cAmbientLoc, 1, &mat.cAmbient[0]);
            }
            if (realtime->mat_cDiffuseLoc >= 0) {
                glUniform4fv(realtime->mat_cDiffuseLoc, 1, &mat.cDiffuse[0]);
            }
            if (realtime->mat_cSpecularLoc >= 0) {
                glUniform4fv(realtime->mat_cSpecularLoc, 1, &mat.cSpecular[0]);
            }
            if (realtime->mat_shinyLoc >= 0) {
                glUniform1f(realtime->mat_shinyLoc, mat.shininess);
            }
        }
        
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
    
    glBindVertexArray(0);
}

void Rendering::renderTrees(Realtime* realtime) {

    if (realtime->m_activeMap == nullptr) {
        return;
    }

    glm::vec3 cameraPos = realtime->m_camera.getPosition();
    int renderDistance = 2;

    if (realtime->m_blockShaderProgram == 0) {
        return;
    }

    glUseProgram(realtime->m_blockShaderProgram);

    glm::mat4 proj = realtime->m_camera.getProjMatrix();
    glm::mat4 view = realtime->m_camera.getViewMatrix();

    if (realtime->m_blockProjLoc >= 0) {
        glUniformMatrix4fv(realtime->m_blockProjLoc, 1, GL_FALSE, &proj[0][0]);
    }
    if (realtime->m_blockViewLoc >= 0) {
        glUniformMatrix4fv(realtime->m_blockViewLoc, 1, GL_FALSE, &view[0][0]);
    }
    if (realtime->m_blockCameraPosLoc >= 0) {
        glUniform3fv(realtime->m_blockCameraPosLoc, 1, &cameraPos[0]);
    }

    std::vector<SceneLightData> lights;
    setupMapLights(realtime, cameraPos, lights);
    realtime->addLightsToBlockShader(lights);

    if (realtime->m_treeVAO == 0) {
        TreePiece treePiece;
        const auto& vertexData = treePiece.getVertexData();

        glGenVertexArrays(1, &realtime->m_treeVAO);
        glGenBuffers(1, &realtime->m_treeVBO);

        glBindVertexArray(realtime->m_treeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, realtime->m_treeVBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float),
                     vertexData.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(6 * sizeof(float)));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(9 * sizeof(float)));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                              (void*)(12 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        realtime->m_treeVertexCount = treePiece.getVertexCount();
    }
    
    if (realtime->m_treeVertexCount <= 0) {
        return;
    }

    if (realtime->m_woodColorTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, realtime->m_woodColorTexture);
        GLint colorTextureLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "colorTexture");
        GLint useColorTextureLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "useColorTexture");
        if (colorTextureLoc >= 0) {
            glUniform1i(colorTextureLoc, 0);
        }
        if (useColorTextureLoc >= 0) {
            glUniform1i(useColorTextureLoc, 1);
        }
    }

    if (realtime->m_woodNormalTexture != 0) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, realtime->m_woodNormalTexture);
        GLint normalMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "normalMap");
        GLint useNormalMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "useNormalMap");
        if (normalMapLoc >= 0) {
            glUniform1i(normalMapLoc, 2);
        }
        if (useNormalMapLoc >= 0) {
            glUniform1i(useNormalMapLoc, 1);
        }
    }

    if (realtime->m_woodBumpTexture != 0) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, realtime->m_woodBumpTexture);
        GLint bumpMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "bumpMap");
        GLint useBumpMapLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "useBumpMap");
        GLint bumpStrengthLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "bumpStrength");
        if (bumpMapLoc >= 0) {
            glUniform1i(bumpMapLoc, 3);
        }
        if (useBumpMapLoc >= 0) {
            glUniform1i(useBumpMapLoc, 1);
        }
        if (bumpStrengthLoc >= 0) {
            glUniform1f(bumpStrengthLoc, realtime->m_bumpStrength);
        }
    }
    

    glBindVertexArray(realtime->m_treeVAO);

    int chunkSize = realtime->m_activeMap->getChunkSize();
    bool endlessMode = realtime->m_activeMap->isEndlessMode();
    
    float cameraChunkXFloat, cameraChunkZFloat;
    if (endlessMode) {
        cameraChunkXFloat = cameraPos.x / static_cast<float>(chunkSize);
        cameraChunkZFloat = cameraPos.z / static_cast<float>(chunkSize);
    } else {
        int centerX = realtime->m_activeMap->getWidth() / 2;
        int centerZ = realtime->m_activeMap->getHeight() / 2;
        cameraChunkXFloat = (cameraPos.x + static_cast<float>(centerX)) / static_cast<float>(chunkSize);
        cameraChunkZFloat = (cameraPos.z + static_cast<float>(centerZ)) / static_cast<float>(chunkSize);
    }
    
    int cameraChunkX = static_cast<int>(std::floor(cameraChunkXFloat));
    int cameraChunkZ = static_cast<int>(std::floor(cameraChunkZFloat));

    for (int dz = -renderDistance; dz <= renderDistance; dz++) {
        for (int dx = -renderDistance; dx <= renderDistance; dx++) {
            int chunkX = cameraChunkX + dx;
            int chunkZ = cameraChunkZ + dz;

            realtime->m_activeMap->ensureChunkGenerated(chunkX, chunkZ);
            
            int chunkKey;
            try {
                long long chunkKeyLL = static_cast<long long>(chunkX) * 10000LL + static_cast<long long>(chunkZ);
                if (chunkKeyLL < std::numeric_limits<int>::min() || chunkKeyLL > std::numeric_limits<int>::max()) {
                    continue;
                }
                chunkKey = static_cast<int>(chunkKeyLL);
            } catch (...) {
                continue;
            }
            
            const auto& chunks = realtime->m_activeMap->getChunks();
            auto it = chunks.find(chunkKey);
            
            if (it == chunks.end() || it->second == nullptr || !it->second->isPopulated()) {
                continue;
            }

            const auto& trees = it->second->getTrees();
            for (const auto& tree : trees) {
                for (const auto& piece : tree.getPieces()) {
                    // piece.position is the center of the tree cylinder
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), piece.position);
                    model = glm::scale(model, piece.scale);

                    if (realtime->m_blockModelLoc >= 0) {
                        glUniformMatrix4fv(realtime->m_blockModelLoc, 1, GL_FALSE, &model[0][0]);
                    }

                    SceneMaterial mat;
                    mat.cAmbient = glm::vec4(0.5f, 0.3f, 0.15f, 1.0f) * 0.5f;
                    mat.cDiffuse = glm::vec4(0.6f, 0.4f, 0.2f, 1.0f);
                    mat.cSpecular = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
                    mat.shininess = 3.0f;

                    GLint ambientLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cAmbient");
                    GLint diffuseLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cDiffuse");
                    GLint specularLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.cSpecular");
                    GLint shinyLoc = glGetUniformLocation(realtime->m_blockShaderProgram, "material.shininess");

                    if (ambientLoc >= 0) {
                        glUniform4fv(ambientLoc, 1, &mat.cAmbient[0]);
                    }
                    if (diffuseLoc >= 0) {
                        glUniform4fv(diffuseLoc, 1, &mat.cDiffuse[0]);
                    }
                    if (specularLoc >= 0) {
                        glUniform4fv(specularLoc, 1, &mat.cSpecular[0]);
                    }
                    if (shinyLoc >= 0) {
                        glUniform1f(shinyLoc, mat.shininess);
                    }

                    glDrawArrays(GL_TRIANGLES, 0, realtime->m_treeVertexCount);
                }
            }
        }
    }

    glBindVertexArray(0);
    
    glUseProgram(realtime->m_shaderProgram);
}

void Rendering::renderCompletionCubes(Realtime* realtime, float currentTime) {
    if (!realtime) return;
    
    if (realtime->m_activeMap == nullptr) {
        return;
    }
    
    if (realtime->m_shaderProgram == 0) {
        std::cout << "ERROR: m_shaderProgram is 0 (invalid)!" << std::endl;
        return;
    }
    
    if (!glIsProgram(realtime->m_shaderProgram)) {
        std::cout << "ERROR: m_shaderProgram (" << realtime->m_shaderProgram << ") is not a valid program!" << std::endl;
        return;
    }
    
    glBindVertexArray(0);
    
    glUseProgram(realtime->m_shaderProgram);
    
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    if (currentProgram != realtime->m_shaderProgram) {
        std::cout << "ERROR: Shader not bound! Expected: " << realtime->m_shaderProgram << ", Got: " << currentProgram << std::endl;
        std::cout << "  Block shader: " << realtime->m_blockShaderProgram << std::endl;
        std::cout << "  Is program valid? " << (glIsProgram(realtime->m_shaderProgram) ? "YES" : "NO") << std::endl;
        
        glUseProgram(realtime->m_shaderProgram);
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    }
    
    glm::mat4 proj = realtime->m_camera.getProjMatrix();
    glm::mat4 view = realtime->m_camera.getViewMatrix();
    glm::vec3 cameraPos = realtime->m_camera.getPosition();
    
    if (realtime->m_projLoc >= 0) {
        glUniformMatrix4fv(realtime->m_projLoc, 1, GL_FALSE, &proj[0][0]);
    }
    if (realtime->m_viewLoc >= 0) {
        glUniformMatrix4fv(realtime->m_viewLoc, 1, GL_FALSE, &view[0][0]);
    }
    if (realtime->m_cameraPosLoc >= 0) {
        glUniform3fv(realtime->m_cameraPosLoc, 1, &cameraPos[0]);
    }
    
    std::vector<SceneLightData> lights;
    setupMapLights(realtime, cameraPos, lights);
    realtime->addLightsToShader(lights);
    
    if (realtime->m_k_aLoc >= 0) {
        glUniform1f(realtime->m_k_aLoc, realtime->m_globalData.ka);
    }
    if (realtime->m_k_dLoc >= 0) {
        glUniform1f(realtime->m_k_dLoc, realtime->m_globalData.kd);
    }
    if (realtime->m_k_sLoc >= 0) {
        glUniform1f(realtime->m_k_sLoc, realtime->m_globalData.ks);
    }
    
    const ShapeData& cubeData = realtime->m_shapeManager.getShapeData(PrimitiveType::PRIMITIVE_CUBE);
    
    if (cubeData.vao == 0 || cubeData.numVertices == 0) {
        std::cout << "ERROR: Invalid cube data! VAO: " << cubeData.vao << ", Vertices: " << cubeData.numVertices << std::endl;
        return;
    }
    
    GLint mat_cAmbientLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.cAmbient");
    GLint mat_cDiffuseLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.cDiffuse");
    GLint mat_cSpecularLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.cSpecular");
    GLint mat_shinyLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.shininess");
    
    int renderDistance = 4;

    int chunkSize = realtime->m_activeMap->getChunkSize();
    bool endlessMode = realtime->m_activeMap->isEndlessMode();
    
    float cameraChunkXFloat, cameraChunkZFloat;
    if (endlessMode) {
        cameraChunkXFloat = cameraPos.x / static_cast<float>(chunkSize);
        cameraChunkZFloat = cameraPos.z / static_cast<float>(chunkSize);
    } else {
        int centerX = realtime->m_activeMap->getWidth() / 2;
        int centerZ = realtime->m_activeMap->getHeight() / 2;
        cameraChunkXFloat = (cameraPos.x + static_cast<float>(centerX)) / static_cast<float>(chunkSize);
        cameraChunkZFloat = (cameraPos.z + static_cast<float>(centerZ)) / static_cast<float>(chunkSize);
    }
    
    int cameraChunkX = static_cast<int>(std::floor(cameraChunkXFloat));
    int cameraChunkZ = static_cast<int>(std::floor(cameraChunkZFloat));

    const float PICKUP_RANGE = 1.8f;
    const float COMPLETION_CUBE_SIZE = 1.0f;

    static bool debugChecked = false;
    static int totalCubesInWorld = 0;
    if (!debugChecked) {
        for (const auto& chunkPair : realtime->m_activeMap->getChunks()) {
            if (chunkPair.second && chunkPair.second->isPopulated()) {
                totalCubesInWorld += chunkPair.second->getCompletionCubes().size();
            }
        }
        debugChecked = true;
    }

    int cubesFoundThisFrame = 0;
    int cubesRenderedThisFrame = 0;

    for (int dz = -renderDistance; dz <= renderDistance; dz++) {
        for (int dx = -renderDistance; dx <= renderDistance; dx++) {
            int chunkX = cameraChunkX + dx;
            int chunkZ = cameraChunkZ + dz;

            realtime->m_activeMap->ensureChunkGenerated(chunkX, chunkZ);
            
            int chunkKey;
            try {
                long long chunkKeyLL = static_cast<long long>(chunkX) * 10000LL + static_cast<long long>(chunkZ);
                if (chunkKeyLL < std::numeric_limits<int>::min() || chunkKeyLL > std::numeric_limits<int>::max()) {
                    continue;
                }
                chunkKey = static_cast<int>(chunkKeyLL);
            } catch (...) {
                continue;
            }
            
            const auto& chunks = realtime->m_activeMap->getChunks();
            auto it = chunks.find(chunkKey);
            
            if (it == chunks.end() || it->second == nullptr || !it->second->isPopulated()) {
                continue;
            }

            auto& completionCubes = it->second->getCompletionCubesMutable();
            
            for (auto cubeIt = completionCubes.begin(); cubeIt != completionCubes.end();) {
                if (cubeIt->isCollected()) {
                    cubeIt = completionCubes.erase(cubeIt);
                    continue;
                }

                glm::vec3 cubePos = cubeIt->getPosition();
                glm::vec3 cubeColor = cubeIt->getColor();
                cubesFoundThisFrame++;
                
                //check pickup distance for collection
                float distToCube = glm::length(cameraPos - cubePos);
                if (distToCube < PICKUP_RANGE) {
                    BiomeType biome = cubeIt->getBiome();
                    realtime->m_activeMap->markCompletionCubeCollected(biome);
                    
                    switch (biome) {
                        case BIOME_FIELD:
                            if (realtime->m_fieldPenaltyTimer <= 0.0f) {
                                realtime->m_fieldPenaltyTimer = Realtime::PENALTY_INCREASE_DURATION;
                            }
                            break;
                        case BIOME_MOUNTAINS:
                            if (realtime->m_mountainPenaltyTimer <= 0.0f) {
                                realtime->m_mountainPenaltyTimer = Realtime::PENALTY_INCREASE_DURATION;
                            }
                            break;
                        case BIOME_FOREST:
                            if (realtime->m_forestPenaltyTimer <= 0.0f) {
                                realtime->m_forestPenaltyTimer = Realtime::PENALTY_INCREASE_DURATION;
                            }
                            break;
                    }
                    
                    if (realtime->m_audioManager != nullptr) {
                        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                        QString cubeGrabPath = tempDir + "/cube_grab.wav";
                        
                        QString resourcePath = ":/resources/soundeffects/cube_grab.wav";
                        if (QFile::exists(resourcePath) && !QFile::exists(cubeGrabPath)) {
                            QFile::copy(resourcePath, cubeGrabPath);
                        }
                        
                        if (QFile::exists(cubeGrabPath)) {
                            realtime->m_audioManager->playSound(cubeGrabPath.toUtf8().constData());
                        }
                    }
                    
                    cubeIt->collect();
                    cubeIt = completionCubes.erase(cubeIt);
                    
                    auto& allChunks = realtime->m_activeMap->getChunks();
                    for (auto& chunkPair : allChunks) {
                        if (chunkPair.second != nullptr && chunkPair.second->isPopulated()) {
                            auto& allCubes = chunkPair.second->getCompletionCubesMutable();
                            for (auto it = allCubes.begin(); it != allCubes.end();) {
                                if (it->getBiome() == biome) {
                                    it = allCubes.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                        }
                    }
                    
                    int cubesCollected = 0;
                    if (realtime->hasFieldCompletionCube()) cubesCollected++;
                    if (realtime->hasMountainCompletionCube()) cubesCollected++;
                    if (realtime->hasForestCompletionCube()) cubesCollected++;
                    
                    int enemiesToSpawn = 2 * cubesCollected;
                    if (enemiesToSpawn > 0) {
                        realtime->m_enemyManager.spawnEnemiesOnRing(cameraPos, enemiesToSpawn);
                    }
                    
                    continue;
                }

                glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePos);
                model = glm::scale(model, glm::vec3(COMPLETION_CUBE_SIZE));
                
                glUniformMatrix4fv(realtime->m_modelLoc, 1, GL_FALSE, &model[0][0]);
                
                SceneMaterial mat;
                mat.cAmbient = glm::vec4(cubeColor * 2.0f, 1.0f);
                mat.cDiffuse = glm::vec4(cubeColor * 2.0f, 1.0f);
                mat.cSpecular = glm::vec4(cubeColor, 1.0f);
                mat.shininess = 39.0f;
                
                glUseProgram(realtime->m_shaderProgram);
                
                glUniform4fv(mat_cAmbientLoc, 1, &mat.cAmbient[0]);
                glUniform4fv(mat_cDiffuseLoc, 1, &mat.cDiffuse[0]);
                glUniform4fv(mat_cSpecularLoc, 1, &mat.cSpecular[0]);
                glUniform1f(mat_shinyLoc, mat.shininess);
                
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
                
                glBindVertexArray(cubeData.vao);
                
                static bool debugRendering = false;
                if (!debugRendering && cubesRenderedThisFrame == 0) {
                    glUseProgram(realtime->m_shaderProgram);
                    GLenum bindErr = glGetError();
                    
                    GLint currentProgram = 0;
                    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
                    GLint currentVAO = 0;
                    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
                    GLenum err = glGetError();
                    debugRendering = true;
                }
                
                glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
                cubesRenderedThisFrame++;
                
                ++cubeIt;
            }
        }
    }
    
}

void Rendering::renderEnemies(Realtime* realtime, float currentTime) {
    if (!realtime) return;
    
    glUseProgram(realtime->m_shaderProgram);
    
    const ShapeData& cubeData = realtime->m_shapeManager.getShapeData(PrimitiveType::PRIMITIVE_CUBE);
    
    GLint mat_cAmbientLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.cAmbient");
    GLint mat_cDiffuseLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.cDiffuse");
    GLint mat_cSpecularLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.cSpecular");
    GLint mat_shinyLoc = glGetUniformLocation(realtime->m_shaderProgram, "material.shininess");
    
    for (int i = 0; i < realtime->m_enemyManager.getEnemyCount(); ++i) {
        const Enemy* enemy = realtime->m_enemyManager.getEnemy(i);
        if (!enemy || (enemy->shouldBeRemoved())) continue;
        
        glm::vec3 pos = enemy->getPosition();
        if (enemy->isDying()) {
            pos = enemy->getDeathStartPosition();
        }
        float sizeMultiplier = enemy->getSizeMultiplier();
        bool isIlluminated = enemy->isIlluminated();
        bool isDying = enemy->isDying();
        float deathProgress = isDying ? enemy->getDeathProgress() : 0.0f;
        
        glm::vec4 cube1Color, cube2Color;
        bool isRecentlyDamaged = enemy->isRecentlyDamaged();
        if (isIlluminated) {
            float illumTime = enemy->getIlluminationTime();
            cube1Color = enemy->getFlashColor(illumTime);
            float flashSpeed = isRecentlyDamaged ? 180.0f : 35.0f;
            float colorOffset = 3.0f / flashSpeed;
            cube2Color = enemy->getFlashColor(illumTime + colorOffset);
        } else {
            //z fight effect by flashing black and white
            cube1Color = enemy->getRandomFlashColor(currentTime);
            cube2Color = enemy->getRandomFlashColor(currentTime + 0.15f);
        }
        
        float scaleFactor = 1.0f;
        glm::vec3 splitOffset1(0.0f, 0.0f, 0.0f);
        glm::vec3 splitOffset2(0.0f, 0.0f, 0.0f);
        
        if (isDying) {
            scaleFactor = 1.0f - (deathProgress * deathProgress);
            
            float splitDistance = 2.0f * deathProgress * deathProgress;

            float angle1 = deathProgress * 3.0f;
            float angle2 = deathProgress * 3.0f;
            splitOffset1 = glm::vec3(
                splitDistance * std::cos(angle1),
                splitDistance * 1.5f + deathProgress * 1.0f,
                splitDistance * std::sin(angle1)
            );
            splitOffset2 = glm::vec3(
                splitDistance * std::cos(angle2),
                splitDistance * 1.5f + deathProgress * 1.0f,
                splitDistance * std::sin(angle2)
            );
        }
        
        glm::vec3 renderPos1 = pos;
        if (isDying) {
            renderPos1 = pos + splitOffset1;
        } else if (isIlluminated) {
            glm::vec3 vibrationOffset = enemy->getVibrationOffset(enemy->getIlluminationTime());
            renderPos1 = pos + vibrationOffset;
        }
        
        glm::mat4 model1 = glm::translate(glm::mat4(1.0f), renderPos1);

        if (isDying) {
            float rotationAngle = deathProgress * 6.28f * 2.0f;
            model1 = glm::rotate(model1, rotationAngle, glm::vec3(0.5f, 1.0f, 0.3f));
        }
        model1 = glm::scale(model1, glm::vec3(sizeMultiplier * scaleFactor, sizeMultiplier * scaleFactor, sizeMultiplier * scaleFactor));
        
        glUniformMatrix4fv(realtime->m_modelLoc, 1, GL_FALSE, &model1[0][0]);
        
        SceneMaterial cube1Mat;
        if (isIlluminated) {
            cube1Mat.cAmbient = cube1Color;
        } else {
            cube1Mat.cAmbient = cube1Color;
        }
        cube1Mat.cDiffuse = cube1Color;
        cube1Mat.cSpecular = glm::vec4(glm::vec3(cube1Color) * 0.5f, 1.0f);
        cube1Mat.shininess = 32.0f;
        
        glUniform4fv(mat_cAmbientLoc, 1, &cube1Mat.cAmbient[0]);
        glUniform4fv(mat_cDiffuseLoc, 1, &cube1Mat.cDiffuse[0]);
        glUniform4fv(mat_cSpecularLoc, 1, &cube1Mat.cSpecular[0]);
        glUniform1f(mat_shinyLoc, cube1Mat.shininess);
        
        glBindVertexArray(cubeData.vao);
        glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
        
        glm::vec3 renderPos2 = pos;
        if (isDying) {
            renderPos2 = pos + splitOffset2;
        } else if (isIlluminated) {
            glm::vec3 vibrationOffset2 = enemy->getVibrationOffset(enemy->getIlluminationTime() + 0.1f);
            renderPos2 = pos + vibrationOffset2;
        }
        
        glm::mat4 model2 = glm::translate(glm::mat4(1.0f), renderPos2);

        if (isDying) {
            float rotationAngle = deathProgress * 6.28f * -2.0f;
            model2 = glm::rotate(model2, rotationAngle, glm::vec3(-0.5f, 1.0f, -0.3f)); // Different chaotic rotation axis
        }
        model2 = glm::scale(model2, glm::vec3(sizeMultiplier * scaleFactor, sizeMultiplier * scaleFactor, sizeMultiplier * scaleFactor));
        
        glUniformMatrix4fv(realtime->m_modelLoc, 1, GL_FALSE, &model2[0][0]);
        
        SceneMaterial cube2Mat;
        if (isIlluminated) {
            cube2Mat.cAmbient = cube2Color;
        } else {
            cube2Mat.cAmbient = cube2Color;
        }
        cube2Mat.cDiffuse = cube2Color;
        cube2Mat.cSpecular = glm::vec4(glm::vec3(cube2Color) * 0.5f, 1.0f);
        cube2Mat.shininess = 32.0f;
        
        glUniform4fv(mat_cAmbientLoc, 1, &cube2Mat.cAmbient[0]);
        glUniform4fv(mat_cDiffuseLoc, 1, &cube2Mat.cDiffuse[0]);
        glUniform4fv(mat_cSpecularLoc, 1, &cube2Mat.cSpecular[0]);
        glUniform1f(mat_shinyLoc, cube2Mat.shininess);
        
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        
        glBindVertexArray(cubeData.vao);
        glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
        
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
}

