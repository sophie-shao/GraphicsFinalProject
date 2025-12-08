#include "realtime.h"
#include "realtime/physics.h"
#include "realtime/rendering.h"
#include "utils/sceneparser.h"
#include "utils/shapefactory.h"
#include "utils/shaderloader.h"
#include "utils/debug.h"
#include <glm/gtx/transform.hpp>
#include <algorithm>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImage>
#include <iostream>
#include "settings.h"


Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
    m_keyMap[Qt::Key_Shift]   = false;

    m_activeMap = nullptr;

    m_globalData.ka = 0.5f;
    m_globalData.kd = 0.5f;
    m_globalData.ks = 0.5f;
    m_globalData.kt = 0.0f;

    m_flyingMode = true;
    m_velocity = glm::vec3(0.0f);
    m_onGround = false;
    m_movementSpeedMultiplier = 1.0;
    m_jumpHeightMultiplier = 1.0;
    m_cameraHeightMultiplier = 1.0;
    m_gravityMultiplier = 1.0;
    m_overheadLightIntensity = 1.0;
    m_playerLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    m_baseFOV = 70.0f;
    m_currentFOV = 70.0f;
    m_baseFOV = 70.0f;
    m_currentFOV = 70.0f;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    m_shapeManager.destroyShapes();
    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    glErrorCheck();

    // Create regular phong shader
    try {
        m_shaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/phong.vert",
            ":/resources/shaders/phong.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "Phong shader error: " << e.what() << std::endl;
    }

    // Create block shader with normal/bump mapping support
    try {
        m_blockShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/default.vert",
            ":/resources/shaders/default.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "Block shader error: " << e.what() << std::endl;
    }

    // Get uniform locations for phong shader
    m_modelLoc = glGetUniformLocation(m_shaderProgram, "modelMatrix");
    m_projLoc = glGetUniformLocation(m_shaderProgram, "projMatrix");
    m_viewLoc = glGetUniformLocation(m_shaderProgram, "viewMatrix");
    m_cameraPosLoc = glGetUniformLocation(m_shaderProgram, "cameraPos");
    m_numLightsLoc = glGetUniformLocation(m_shaderProgram, "numLights");

    mat_cAmbientLoc = glGetUniformLocation(m_shaderProgram, "material.cAmbient");
    mat_cDiffuseLoc = glGetUniformLocation(m_shaderProgram, "material.cDiffuse");
    mat_cSpecularLoc = glGetUniformLocation(m_shaderProgram, "material.cSpecular");
    mat_shinyLoc = glGetUniformLocation(m_shaderProgram, "material.shininess");

    m_k_aLoc = glGetUniformLocation(m_shaderProgram, "k_a");
    m_k_dLoc = glGetUniformLocation(m_shaderProgram, "k_d");
    m_k_sLoc = glGetUniformLocation(m_shaderProgram, "k_s");
    m_shininessLoc = glGetUniformLocation(m_shaderProgram, "shininess");

    // Get uniform locations for block shader
    m_blockModelLoc = glGetUniformLocation(m_blockShaderProgram, "modelMatrix");
    m_blockProjLoc = glGetUniformLocation(m_blockShaderProgram, "projMatrix");
    m_blockViewLoc = glGetUniformLocation(m_blockShaderProgram, "viewMatrix");
    m_blockCameraPosLoc = glGetUniformLocation(m_blockShaderProgram, "cameraPos");
    m_blockNumLightsLoc = glGetUniformLocation(m_blockShaderProgram, "numLights");

    // Load dirt texture for blocks
    glGenTextures(1, &m_dirtTexture);
    glBindTexture(GL_TEXTURE_2D, m_dirtTexture);

    QImage dirtImage(":/scenefiles/maps/dirt.png");
    if (dirtImage.isNull()) {
        std::cerr << "Failed to load dirt texture" << std::endl;
        // Create a simple brown texture as fallback
        unsigned char dirtColor[4] = {139, 69, 19, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, dirtColor);
    } else {
        dirtImage = dirtImage.convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dirtImage.width(), dirtImage.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, dirtImage.bits());
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Load wood texture for cylinders
    glGenTextures(1, &m_woodTexture);
    glBindTexture(GL_TEXTURE_2D, m_woodTexture);

    QImage woodImage(":/scenefiles/maps/wood_bump.png");
    if (woodImage.isNull()) {
        std::cerr << "Failed to load wood texture" << std::endl;
        // Create a simple wood-colored texture as fallback
        unsigned char woodColor[4] = {150, 111, 51, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, woodColor);
    } else {
        woodImage = woodImage.convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, woodImage.width(), woodImage.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, woodImage.bits());
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_shapeManager.initShapes(settings.shapeParameter1, settings.shapeParameter2);
    glErrorCheck();
}

void Realtime::paintGL() {
    glClearColor(103/255.f, 142/255.f, 166/255.f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // First render all cylinders with wood texture
    glUseProgram(m_blockShaderProgram); // Use the shader that supports normal/bump mapping

    // Bind wood texture for cylinders
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_woodTexture);
    glUniform1i(glGetUniformLocation(m_blockShaderProgram, "diffuseTexture"), 0);

    // Set up camera and lighting
    glm::mat4 proj = m_camera.getProjMatrix();
    glm::mat4 view = m_camera.getViewMatrix();
    glm::vec3 camPos = m_camera.getPosition();

    glUniformMatrix4fv(m_blockProjLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(m_blockViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3fv(m_blockCameraPosLoc, 1, &camPos[0]);

    // Set lighting uniforms
    addLightsToBlockShader(m_lights);

    // Set k values
    glUniform1f(glGetUniformLocation(m_blockShaderProgram, "k_a"), m_globalData.ka);
    glUniform1f(glGetUniformLocation(m_blockShaderProgram, "k_d"), m_globalData.kd);
    glUniform1f(glGetUniformLocation(m_blockShaderProgram, "k_s"), m_globalData.ks);

    // Render cylinders from scene
    for (const auto &shape : m_shapes) {
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
            const ShapeData &data = m_shapeManager.getShapeData(shape.primitive.type);
            const SceneMaterial &mat = shape.primitive.material;

            glUniformMatrix4fv(m_blockModelLoc, 1, GL_FALSE, &shape.ctm[0][0]);

            // Set material uniforms
            GLuint mat_cAmbientLoc = glGetUniformLocation(m_blockShaderProgram, "material.cAmbient");
            GLuint mat_cDiffuseLoc = glGetUniformLocation(m_blockShaderProgram, "material.cDiffuse");
            GLuint mat_cSpecularLoc = glGetUniformLocation(m_blockShaderProgram, "material.cSpecular");
            GLuint mat_shinyLoc = glGetUniformLocation(m_blockShaderProgram, "material.shininess");

            glUniform4fv(mat_cAmbientLoc, 1, &mat.cAmbient[0]);
            glUniform4fv(mat_cDiffuseLoc, 1, &mat.cDiffuse[0]);
            glUniform4fv(mat_cSpecularLoc, 1, &mat.cSpecular[0]);
            glUniform1f(mat_shinyLoc, mat.shininess);

            glBindVertexArray(data.vao);
            glDrawArrays(GL_TRIANGLES, 0, data.numVertices);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    // Now render blocks with dirt texture
    if (m_activeMap != nullptr) {
        glUseProgram(m_blockShaderProgram);

        // Bind dirt texture for blocks
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dirtTexture);
        glUniform1i(glGetUniformLocation(m_blockShaderProgram, "diffuseTexture"), 0);

        // Set camera and lighting again
        glUniformMatrix4fv(m_blockProjLoc, 1, GL_FALSE, &proj[0][0]);
        glUniformMatrix4fv(m_blockViewLoc, 1, GL_FALSE, &view[0][0]);
        glUniform3fv(m_blockCameraPosLoc, 1, &camPos[0]);

        // Render blocks
        Rendering::renderMapBlocks(this);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }
}

void Realtime::resizeGL(int w, int h) {
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    m_camera.updateProjectionMatrix(aspect, settings.nearPlane, settings.farPlane);
}


void Realtime::sceneChanged() {
    glErrorCheck();

    glUseProgram(m_shaderProgram);

    RenderData metaData;
    SceneParser::parse(settings.sceneFilePath, metaData);
    m_globalData = metaData.globalData;
    glUniform1f(m_k_aLoc, metaData.globalData.ka);
    glUniform1f(m_k_dLoc, metaData.globalData.kd);
    glUniform1f(m_k_sLoc, metaData.globalData.ks);
    m_shapes = metaData.shapes;
    m_lights = metaData.lights;
    glErrorCheck();

    addLightsToShader(metaData.lights);
    float aspectRatio = static_cast<float>(width()) / static_cast<float>(height());
    m_camera = Camera(metaData.cameraData, aspectRatio, settings.nearPlane, settings.farPlane);
    glUseProgram(0);
    update();
}

void Realtime::addLightsToBlockShader(const std::vector<SceneLightData> &lights) {
    glUseProgram(m_blockShaderProgram);
    int numLights = std::min((int)lights.size(), 8);
    glUniform1i(m_blockNumLightsLoc, numLights);

    // Set k values
    glUniform1f(glGetUniformLocation(m_blockShaderProgram, "k_a"), m_globalData.ka);
    glUniform1f(glGetUniformLocation(m_blockShaderProgram, "k_d"), m_globalData.kd);
    glUniform1f(glGetUniformLocation(m_blockShaderProgram, "k_s"), m_globalData.ks);

    for (int i = 0; i < numLights; i++) {
        const SceneLightData &L = lights[i];
        std::string base = "lights[" + std::to_string(i) + "]";

        int lightType;
        if (L.type == LightType::LIGHT_DIRECTIONAL) lightType = 0;
        else if (L.type == LightType::LIGHT_POINT) lightType = 1;
        else lightType = 2;

        glUniform1i(glGetUniformLocation(m_blockShaderProgram, (base + ".type").c_str()), lightType);
        glUniform3fv(glGetUniformLocation(m_blockShaderProgram, (base + ".color").c_str()), 1, &L.color[0]);
        glUniform3fv(glGetUniformLocation(m_blockShaderProgram, (base + ".function").c_str()), 1, &L.function[0]);
        glUniform1f(glGetUniformLocation(m_blockShaderProgram, (base + ".angle").c_str()), L.angle);
        glUniform1f(glGetUniformLocation(m_blockShaderProgram, (base + ".penumbra").c_str()), L.penumbra);

        glUniform3fv(glGetUniformLocation(m_blockShaderProgram, (base + ".position").c_str()), 1, &L.pos[0]);
        glUniform3fv(glGetUniformLocation(m_blockShaderProgram, (base + ".direction").c_str()), 1, &L.dir[0]);
    }
}

void Realtime::addLightsToShader(const std::vector<SceneLightData> &lights) {
    glUseProgram(m_shaderProgram);
    int numLights = std::min((int)lights.size(), 8);
    glUniform1i(m_numLightsLoc, numLights);
    glErrorCheck();

    for (int i = 0; i < numLights; i++) {
        const SceneLightData &L = lights[i];
        std::string base = "lights[" + std::to_string(i) + "]";

        glUniform1i(glGetUniformLocation(m_shaderProgram, (base + ".type").c_str()), (int)L.type);
        glUniform3fv(glGetUniformLocation(m_shaderProgram, (base + ".color").c_str()), 1, &L.color[0]);
        glUniform3fv(glGetUniformLocation(m_shaderProgram, (base + ".function").c_str()), 1, &L.function[0]);
        glUniform1f(glGetUniformLocation(m_shaderProgram, (base + ".angle").c_str()), L.angle);
        glUniform1f(glGetUniformLocation(m_shaderProgram, (base + ".penumbra").c_str()), L.penumbra);

        if (L.type == LightType::LIGHT_DIRECTIONAL) {
            glUniform3fv(glGetUniformLocation(m_shaderProgram, (base + ".direction").c_str()), 1, &L.dir[0]);
            glUniform3fv(glGetUniformLocation(m_shaderProgram, (base + ".position").c_str()), 1, &L.dir[0]);
        } else {
            glUniform3fv(glGetUniformLocation(m_shaderProgram, (base + ".position").c_str()), 1, &L.pos[0]);
            glUniform3fv(glGetUniformLocation(m_shaderProgram, (base + ".direction").c_str()), 1, &L.dir[0]);
        }
        glErrorCheck();
    }
    glUseProgram(0);
}


void Realtime::settingsChanged() {
    m_camera.updateProjectionMatrix(
        float(width()) / float(height()),
        settings.nearPlane,
        settings.farPlane
        );
    makeCurrent();
    m_shapeManager.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    doneCurrent();
    update();
}

void Realtime::setActiveMap(Map* map) {
    m_activeMap = map;

    if (m_activeMap != nullptr) {
        int mapWidth = m_activeMap->getWidth();
        int mapDepth = m_activeMap->getHeight();
        int maxHeight = m_activeMap->getMaxHeight();

        if (mapWidth <= 0 || mapDepth <= 0) {
            return;
        }

        int middleX = mapWidth / 2;
        int middleZ = mapDepth / 2;

        int worldX = middleX - (mapWidth / 2);
        int worldZ = middleZ - (mapDepth / 2);

        auto blocks = m_activeMap->getBlocksToRender();
        if (blocks.empty()) {
            return;
        }

        int centerBlockY = 0;
        bool foundCenterBlock = false;
        for (const auto& block : blocks) {
            int x, y, z;
            BiomeType biome;
            std::tie(x, y, z, biome) = block;
            if (x == worldX && z == worldZ) {
                centerBlockY = y;
                foundCenterBlock = true;
                break;
            }
        }

        if (!foundCenterBlock && !blocks.empty()) {
            int x, y, z;
            BiomeType biome;
            std::tie(x, y, z, biome) = blocks[blocks.size() / 2];
            centerBlockY = y;
        }

        const float EYE_HEIGHT = 1.6f;
        int maxDimension = std::max(mapWidth, mapDepth);
        int cameraOffset = std::max(5, maxDimension / 4);
        glm::vec3 cameraPos(
            static_cast<float>(worldX + cameraOffset),
            static_cast<float>(centerBlockY + 1) + EYE_HEIGHT,
            static_cast<float>(worldZ + cameraOffset)
            );

        glm::vec3 lookTarget(
            static_cast<float>(worldX),
            static_cast<float>(centerBlockY),
            static_cast<float>(worldZ)
            );
        glm::vec3 lookDir = glm::normalize(lookTarget - cameraPos);

        m_camera.setPosition(cameraPos);
        m_camera.setLook(lookDir);
        m_camera.setUp(glm::vec3(0.0f, 1.0f, 0.0f));
        if (m_baseFOV == 0.0f) {
            m_baseFOV = 70.0f;
            m_currentFOV = m_baseFOV;
        }
        m_camera.setFOV(m_currentFOV);

        float aspect = static_cast<float>(width()) / static_cast<float>(height());
        if (aspect > 0) {
            m_camera.updateProjectionMatrix(aspect, settings.nearPlane, settings.farPlane);
        }

        update();
    }
}


void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;

    // Toggle normal mapping with 'N' key
    if (event->key() == Qt::Key_N) {
        m_useNormalMapping = !m_useNormalMapping;
        std::cout << "Normal mapping: " << (m_useNormalMapping ? "ON" : "OFF") << std::endl;
        update();
    }

    // Toggle bump mapping with 'B' key
    if (event->key() == Qt::Key_B) {
        m_useBumpMapping = !m_useBumpMapping;
        std::cout << "Bump mapping: " << (m_useBumpMapping ? "ON" : "OFF") << std::endl;
        update();
    }

    // Adjust bump strength with +/- keys
    if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
        m_bumpStrength += 2.0f;
        std::cout << "Bump strength: " << m_bumpStrength << std::endl;
        update();
    }
    if (event->key() == Qt::Key_Minus) {
        m_bumpStrength = std::max(0.0f, m_bumpStrength - 2.0f);
        std::cout << "Bump strength: " << m_bumpStrength << std::endl;
        update();
    }
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);
        m_camera.rotate(deltaX*0.003f, deltaY*0.003);
        update();
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    float moveSpeed = 5.0f * deltaTime;

    if (m_flyingMode) {
        if (m_keyMap[Qt::Key_W]) m_camera.moveForward(moveSpeed);
        if (m_keyMap[Qt::Key_S]) m_camera.moveForward(-moveSpeed);
        if (m_keyMap[Qt::Key_A]) m_camera.moveRight(-moveSpeed);
        if (m_keyMap[Qt::Key_D]) m_camera.moveRight(moveSpeed);
        if (m_keyMap[Qt::Key_Space]) m_camera.moveUp(moveSpeed);
        if (m_keyMap[Qt::Key_Control]) m_camera.moveUp(-moveSpeed);
    } else {
        Physics::updatePhysics(this, deltaTime);
    }

    bool isSprinting = m_keyMap[Qt::Key_Shift] &&
                       (m_keyMap[Qt::Key_W] || m_keyMap[Qt::Key_S] ||
                        m_keyMap[Qt::Key_A] || m_keyMap[Qt::Key_D]);

    float targetFOV = isSprinting ? m_baseFOV + 15.0f : m_baseFOV;
    float fovSpeed = 50.0f;
    if (m_currentFOV < targetFOV) {
        m_currentFOV = std::min(targetFOV, m_currentFOV + fovSpeed * deltaTime);
    } else if (m_currentFOV > targetFOV) {
        m_currentFOV = std::max(targetFOV, m_currentFOV - fovSpeed * deltaTime);
    }

    m_camera.setFOV(m_currentFOV);
    float aspect = static_cast<float>(width()) / static_cast<float>(height());
    if (aspect > 0) {
        m_camera.updateProjectionMatrix(aspect, settings.nearPlane, settings.farPlane);
    }

    updateTelemetry();
    update();
}

void Realtime::updateTelemetry() {
    glm::vec3 cameraPos = m_camera.getPosition();

    int chunkX = 0;
    int chunkZ = 0;

    if (m_activeMap != nullptr) {
        try {
            int chunkSize = m_activeMap->getChunkSize();
            if (chunkSize > 0) {
                int mapWidth = m_activeMap->getWidth();
                int mapDepth = m_activeMap->getHeight();

                if (mapWidth > 0 && mapDepth > 0) {
                    int centerX = mapWidth / 2;
                    int centerZ = mapDepth / 2;

                    float cameraChunkX = (cameraPos.x + static_cast<float>(centerX)) / static_cast<float>(chunkSize);
                    float cameraChunkZ = (cameraPos.z + static_cast<float>(centerZ)) / static_cast<float>(chunkSize);

                    if (std::isfinite(cameraChunkX) && std::isfinite(cameraChunkZ)) {
                        chunkX = static_cast<int>(std::floor(cameraChunkX));
                        chunkZ = static_cast<int>(std::floor(cameraChunkZ));
                    }
                }
            }
        } catch (...) {
            chunkX = 0;
            chunkZ = 0;
        }
    }

    emit telemetryUpdate(cameraPos.x, cameraPos.y, cameraPos.z, chunkX, chunkZ);
}

void Realtime::setFlyingMode(bool flying) {
    m_flyingMode = flying;
    if (flying) {
        m_velocity = glm::vec3(0.0f);
        m_onGround = false;
    }
}

void Realtime::setMovementSpeedMultiplier(double multiplier) {
    m_movementSpeedMultiplier = multiplier;
}

void Realtime::setJumpHeightMultiplier(double multiplier) {
    m_jumpHeightMultiplier = multiplier;
}

void Realtime::setCameraHeightMultiplier(double multiplier) {
    m_cameraHeightMultiplier = multiplier;
}

void Realtime::setPlayerLightColor(float r, float g, float b) {
    m_playerLightColor = glm::vec3(r, g, b);
}

void Realtime::setGravityMultiplier(double multiplier) {
    m_gravityMultiplier = multiplier;
}

void Realtime::setOverheadLightIntensity(double intensity) {
    m_overheadLightIntensity = intensity;
}


// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.flipped(Qt::Vertical);

    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
