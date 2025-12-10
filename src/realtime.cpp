#include "realtime.h"
#include "realtime/physics.h"
#include "realtime/rendering.h"
#include "realtime/gbuffer.h"
#include "realtime/textures.h"
#include "realtime/fog.h"
#include "realtime/input.h"
#include "utils/sceneparser.h"
#include "utils/shapefactory.h"
#include "utils/shaderloader.h"
#include "utils/debug.h"
#include "utils/audiomanager.h"
#include "map/mapproperties.h"
#include "map/Map.h"
#include "blocks/Block.h"
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <deque>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImage>
#include <QCursor>
#include <QStandardPaths>
#include <QFile>
#include <QStringList>
#include <random>
#include <iostream>
#include "settings.h"

void Realtime::addPathWaypoint() {
    glm::vec3 pos = m_camera.getPosition();
    glm::vec3 look = m_camera.getLook();
    m_cameraPath.addWaypoint(pos, look);
}

void Realtime::startPathPlayback() {
    if (m_cameraPath.getWaypointCount() >= 2) {
        m_cameraPath.startPlayback(m_cameraPath.getPlaybackDuration());
    }
}

void Realtime::setPathDuration(float durationSeconds) {
    m_cameraPath.setPlaybackDuration(durationSeconds);
}

void Realtime::stopPathPlayback() {
    m_cameraPath.stopPlayback();
}

void Realtime::clearPath() {
    m_cameraPath.clear();
}


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

    m_flyingMode = false;
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
    m_motionBlurEnabled = false;
    m_motionBlurSamples = 3;
    m_motionBlurAutoEnabled = false;
    m_depthVisualizationEnabled = false;
    m_gbufferVisualizationMode = 0;
    m_fpsMode = false;
    m_pKeyPressed = false;
    m_ignoreNextMouseMove = false;
    m_fpsCenterTimer = 0;
    m_fogEnabled = true;
    m_flashlightEnabled = false;
    m_fogColor = glm::vec3(0.05f, 0.05f, 0.15f);
    m_targetFogColor = m_fogColor;
    m_fogIntensity = 0.4f;
    m_fogTransitionSpeed = 0.3f; 
    
    m_flashlightCharge = 100.0f;
    m_flashlightPenaltyTimer = 0.0f;
    m_flashlightFKeyPressed = false;
    m_flashlightFlickerIntensity = 1.0f;
    m_flashlightFlickerTime = 0.0f;
    
    m_flashlight.color = glm::vec3(1.0f, 1.0f, 0.6f);
    m_flashlight.coneAngle = glm::radians(20.0f);
    
    m_audioManager = new AudioManager();
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString onSoundPath = tempDir + "/flashlight_on.wav";
    QString offSoundPath = tempDir + "/flashlight_off.wav";
    QString ambientSoundPath = tempDir + "/ambientfinal.wav";
    
    if (!QFile::exists(onSoundPath)) {
        QFile::copy(":/resources/soundeffects/flashlight_on.wav", onSoundPath);
    }
    if (!QFile::exists(offSoundPath)) {
        QFile::copy(":/resources/soundeffects/flashlight_off.wav", offSoundPath);
    }
    if (!QFile::exists(ambientSoundPath)) {
        QFile::copy(":/resources/soundeffects/ambientfinal.wav", ambientSoundPath);
    }
    
    // Copy cube grab sound effect
    QString cubeGrabPath = tempDir + "/cube_grab.wav";
    if (!QFile::exists(cubeGrabPath)) {
        QFile::copy(":/resources/soundeffects/cube_grab.wav", cubeGrabPath);
    }
    
    if (m_audioManager != nullptr && QFile::exists(ambientSoundPath)) {
        m_audioManager->playLoopingSound(ambientSoundPath.toUtf8().constData());
    }
    
    m_randomAmbientTimer = 0.0f;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> intervalDist(15.0f, 25.0f);
    m_nextRandomAmbientInterval = intervalDist(gen);
    
    QStringList ambientSoundFiles = {
        "ambient_bells.wav",
        "ambient_chaos.wav",
        "ambient_crackle.wav",
        "ambient_crashwoosh.wav",
        "ambient_idkwwhatocallthis.wav",
        "ambient_steps.wav",
        "ambient_vibrate.wav",
        "ambient_woosh.wav"
    };
    
    for (const QString& fileName : ambientSoundFiles) {
        QString resourcePath = ":/resources/soundeffects/random_ambient/" + fileName;
        QString tempSoundPath = tempDir + "/" + fileName;
        
        if (QFile::exists(resourcePath)) {
            if (!QFile::exists(tempSoundPath)) {
                QFile::copy(resourcePath, tempSoundPath);
            }
            m_randomAmbientSounds.push_back(tempSoundPath);
        }
    }
    
    m_walkingTime = 0.0f;
    m_footstepCount = 0;
    m_isDead = false;
    QStringList footstepSoundFiles = {
        "step1.wav",
        "step2.wav",
        "step3.wav",
        "step4.wav"
    };
    
    for (const QString& fileName : footstepSoundFiles) {
        QString resourcePath = ":/resources/soundeffects/footsteps/" + fileName;
        QString tempSoundPath = tempDir + "/" + fileName;
        
        if (QFile::exists(resourcePath)) {
            if (!QFile::exists(tempSoundPath)) {
                QFile::copy(resourcePath, tempSoundPath);
            }
            m_footstepSounds.push_back(tempSoundPath);
        }
    }
    
    m_postShaderProgram = 0;
    m_postQuadVAO = 0;
    m_postQuadVBO = 0;
    
    m_filterShaderProgram = 0;
    m_filterQuadVAO = 0;
    m_filterQuadVBO = 0;
    m_filterFBO = 0;
    m_filterTexture = 0;
    
    // Initialize bloom pipeline
    m_bloomShaderProgram = 0;
    m_blurShaderProgram = 0;
    m_bloomExtractFBO = 0;
    m_bloomExtractTexture = 0;
    m_bloomBlurFBO[0] = 0;
    m_bloomBlurFBO[1] = 0;
    m_bloomBlurTexture[0] = 0;
    m_bloomBlurTexture[1] = 0;
    m_bloomInitialized = false;
    m_filterLUTTexture = 0;
    m_filterTime = 0.0f;
    m_filterMode = 7;
    m_grainOverlayEnabled = false;
    m_grainOpacity = 1.0f;
    m_playerHealth = 100.0f;
    m_healthRecoveryRate = 10.0f; // health per second
    m_timeSinceLastDamage = 0.0f;
    m_healingDelay = 2.0f; // 2 seconds delay before healing starts
    m_pixelateEnabled = true;
    m_bloomEnabled = true;
    m_lutChoice = 2;
    m_lutSize = 16.0f;
    
    m_pendingRotation = glm::vec2(0.0f);
    m_rotationSmoothingFactor = 0.35f;
    m_blockShaderProgram = 0;
    m_blockVAO = 0;
    m_blockVBO = 0;
    m_blockVertexCount = 0;
    m_treeVAO = 0;
    m_treeVBO = 0;
    m_treeVertexCount = 0;
    m_completionCubeVAO = 0;
    m_completionCubeVBO = 0;
    m_completionCubeVertexCount = 0;
    m_colorTexture = 0;
    m_normalMapTexture = 0;
    m_bumpMapTexture = 0;
    m_useNormalMapping = false;
    m_useBumpMapping = false;
    m_bumpStrength = 10.0f;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    m_shapeManager.destroyShapes();
    GBuffer::cleanup(this);
    m_particleSystem.cleanup();
    m_ui.cleanup();
    
    if (m_postShaderProgram != 0) {
        glDeleteProgram(m_postShaderProgram);
    }
    if (m_postQuadVAO != 0) {
        glDeleteVertexArrays(1, &m_postQuadVAO);
    }
    if (m_postQuadVBO != 0) {
        glDeleteBuffers(1, &m_postQuadVBO);
    }
    
    if (m_audioManager != nullptr) {
        delete m_audioManager;
        m_audioManager = nullptr;
    }
    
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

    try {
        m_shaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/phong.vert",
            ":/resources/shaders/phong.frag"
            );
    } catch (const std::runtime_error &e) {
    }

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

    // Create block shader with normal/bump mapping support
    try {
        m_blockShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/default.vert",
            ":/resources/shaders/default.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "Block shader error: " << e.what() << std::endl;
        m_blockShaderProgram = 0;
    }

    if (m_blockShaderProgram != 0) {
        m_blockModelLoc = glGetUniformLocation(m_blockShaderProgram, "modelMatrix");
        m_blockProjLoc = glGetUniformLocation(m_blockShaderProgram, "projMatrix");
        m_blockViewLoc = glGetUniformLocation(m_blockShaderProgram, "viewMatrix");
        m_blockCameraPosLoc = glGetUniformLocation(m_blockShaderProgram, "cameraPos");
        m_blockNumLightsLoc = glGetUniformLocation(m_blockShaderProgram, "numLights");
    }

    m_useNormalMapping = false;
    m_useBumpMapping = false;
    m_bumpStrength = 10.0f;

    m_blockVAO = 0;
    m_blockVBO = 0;
    m_blockVertexCount = 0;

    // Load all textures using helper
    TextureLoader::initializeTextures(this);

    makeCurrent();
    m_shapeManager.initShapes(settings.shapeParameter1, settings.shapeParameter2);
    GBuffer::initialize(this, size().width(), size().height());
    
    try {
        m_postShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/post.vert",
            ":/resources/shaders/post.frag"
        );
    } catch (const std::runtime_error &e) {
        std::cerr << "Error loading post-processing shader: " << e.what() << std::endl;
    }
    
    initializeFilterSystem();
    m_particleSystem.initialize();
    m_ui.initialize(this);
    m_ui.resize(size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    
    std::vector<GLfloat> fullscreen_quad_data = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f
    };
    
    glGenBuffers(1, &m_postQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_postQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size() * sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_postQuadVAO);
    glBindVertexArray(m_postQuadVAO);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    doneCurrent();
}

void Realtime::paintGL() {
    // Capture view/projection matrices at the START of the frame for consistent frame-to-frame comparison
    glm::mat4 proj = m_camera.getProjMatrix();
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 viewProj = proj * view;
    
        bool needsPostProcessing = (m_fogEnabled || m_flashlightEnabled) && m_postShaderProgram != 0;
        bool needsGBuffer = m_motionBlurEnabled || m_depthVisualizationEnabled || m_gbufferVisualizationMode != 0 || needsPostProcessing || m_filterMode != 0 || m_grainOverlayEnabled || m_pixelateEnabled || m_bloomEnabled;
    
    if (needsGBuffer) {
        if (!GBuffer::m_initialized) {
            GBuffer::initialize(this, size().width(), size().height());
        }
        
        if (GBuffer::m_gbufferFBO == 0 || GBuffer::m_sceneFBO == 0) {
            GBuffer::initialize(this, size().width(), size().height());
        }
        
        if (GBuffer::m_prevViewProj == glm::mat4(1.0f)) {
            GBuffer::m_prevViewProj = viewProj;
        }
        
        GBuffer::beginGeometryPass(this);
        
        if (GBuffer::m_gbufferShaderProgram == 0) {
            GBuffer::m_prevViewProj = viewProj;
            return;
        }
        
        glUseProgram(GBuffer::m_gbufferShaderProgram);
        
        // Use cached uniform locations (performance optimization)
        if (GBuffer::m_gbufferProjLoc != -1) {
            glUniformMatrix4fv(GBuffer::m_gbufferProjLoc, 1, GL_FALSE, &proj[0][0]);
        }
        if (GBuffer::m_gbufferViewLoc != -1) {
            glUniformMatrix4fv(GBuffer::m_gbufferViewLoc, 1, GL_FALSE, &view[0][0]);
        }
        if (GBuffer::m_gbufferPrevViewProjLoc != -1) {
            glUniformMatrix4fv(GBuffer::m_gbufferPrevViewProjLoc, 1, GL_FALSE, &GBuffer::m_prevViewProj[0][0]);
        }
        
        if (GBuffer::m_gbufferUseColorTextureLoc >= 0) {
            glUniform1i(GBuffer::m_gbufferUseColorTextureLoc, 0);
        }
        
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(4);
        
        for (const auto &shape : m_shapes) {
            const ShapeData &data = m_shapeManager.getShapeData(shape.primitive.type);
            const SceneMaterial &mat = shape.primitive.material;
            // Use cached uniform locations (performance optimization)
            if (GBuffer::m_gbufferModelLoc != -1) {
                glUniformMatrix4fv(GBuffer::m_gbufferModelLoc, 1, GL_FALSE, &shape.ctm[0][0]);
            }
            if (GBuffer::m_gbufferDiffuseLoc != -1) {
                glUniform4fv(GBuffer::m_gbufferDiffuseLoc, 1, &mat.cDiffuse[0]);
            }
            
            glBindVertexArray(data.vao);
            glDrawArrays(GL_TRIANGLES, 0, data.numVertices);
            
        }
        
        if (m_activeMap != nullptr) {
            glm::vec3 cameraPos = m_camera.getPosition();
            int renderDistance = 4;
            auto blocks = m_activeMap->getBlocksInRenderDistance(cameraPos, renderDistance);
            if (blocks.empty()) {
                blocks = m_activeMap->getBlocksToRender();
            }
            
            if (m_blockVAO == 0 && m_blockShaderProgram != 0) {
                Block block;
                const auto& vertexData = block.getVertexData();
                
                glGenVertexArrays(1, &m_blockVAO);
                glGenBuffers(1, &m_blockVBO);
                
                glBindVertexArray(m_blockVAO);
                glBindBuffer(GL_ARRAY_BUFFER, m_blockVBO);
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
                
                m_blockVertexCount = block.getVertexCount();
            }
            
            if (m_colorTexture != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_colorTexture);
                // Use cached uniform locations (performance optimization)
                if (GBuffer::m_gbufferColorTextureLoc >= 0) {
                    glUniform1i(GBuffer::m_gbufferColorTextureLoc, 0);
                }
                if (GBuffer::m_gbufferUseColorTextureLoc >= 0) {
                    glUniform1i(GBuffer::m_gbufferUseColorTextureLoc, 1);
                }
            }
            
            GLuint targetVAO = 0;
            int vertexCount = 0;
            
            if (m_blockVAO != 0) {
                targetVAO = m_blockVAO;
                vertexCount = m_blockVertexCount;
                glBindVertexArray(targetVAO);
            } else {
                const ShapeData &cubeData = m_shapeManager.getShapeData(PrimitiveType::PRIMITIVE_CUBE);
                targetVAO = cubeData.vao;
                vertexCount = cubeData.numVertices;
                glBindVertexArray(targetVAO);
            }
            
            // Cache white color outside loop (performance optimization)
            const glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
            
            for (const auto& block : blocks) {
                int x, y, z;
                BiomeType biome;
                std::tie(x, y, z, biome) = block;
                
                glm::mat4 ctm = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
                // Use cached uniform location (performance optimization)
                if (GBuffer::m_gbufferModelLoc != -1) {
                    glUniformMatrix4fv(GBuffer::m_gbufferModelLoc, 1, GL_FALSE, &ctm[0][0]);
                }
                
                SceneMaterial mat;
                unsigned char r, g, b;
                MapProperties::getBiomeColor(biome, r, g, b);
                glm::vec3 originalColor = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
                glm::vec3 pastelColor = originalColor * 0.5f + white * 0.5f;
                mat.cDiffuse = glm::vec4(pastelColor, 1.0f);
                
                // Use cached uniform location (performance optimization)
                if (GBuffer::m_gbufferDiffuseLoc != -1) {
                    glUniform4fv(GBuffer::m_gbufferDiffuseLoc, 1, &mat.cDiffuse[0]);
                }
                
                glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            }
        }
        
        GBuffer::endGeometryPass(this);
        
        if (m_depthVisualizationEnabled) {
            GBuffer::endGeometryPass(this);
            GBuffer::renderDepthVisualization(this, settings.nearPlane, settings.farPlane);
            GBuffer::m_prevViewProj = viewProj;
            return;
        }
        
        if (m_gbufferVisualizationMode != 0) {
            GBuffer::endGeometryPass(this);
            GBuffer::renderGBufferVisualization(this, m_gbufferVisualizationMode);
            GBuffer::m_prevViewProj = viewProj;
            return;
        }
        
        GBuffer::beginScenePass(this);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            GBuffer::endScenePass(this);
            if (!needsPostProcessing) {
                m_motionBlurEnabled = false;
            }
        } else {
            glClearColor(103/255.f, 142/255.f, 166/255.f, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glUseProgram(m_shaderProgram);
            glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, &proj[0][0]);
            glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, &view[0][0]);
    glm::vec3 camPos = m_camera.getPosition();
            glUniform3fv(m_cameraPosLoc, 1, &camPos[0]);
            glUniform1f(m_k_aLoc, m_globalData.ka);
            glUniform1f(m_k_dLoc, m_globalData.kd);
            glUniform1f(m_k_sLoc, m_globalData.ks);
            
            for (const auto &shape : m_shapes) {
                const ShapeData &data = m_shapeManager.getShapeData(shape.primitive.type);
                const SceneMaterial &mat = shape.primitive.material;
                glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, &shape.ctm[0][0]);
                glUniform4fv(mat_cAmbientLoc, 1, &mat.cAmbient[0]);
                glUniform4fv(mat_cDiffuseLoc, 1, &mat.cDiffuse[0]);
                glUniform4fv(mat_cSpecularLoc, 1, &mat.cSpecular[0]);
                glUniform1f(mat_shinyLoc, mat.shininess);
                
                glBindVertexArray(data.vao);
                glDrawArrays(GL_TRIANGLES, 0, data.numVertices);
            }
            
            float currentTime = m_elapsedTimer.elapsed() / 1000.0f;
            
            if (m_activeMap != nullptr) {
                Rendering::renderMapBlocks(this);
                Rendering::renderTrees(this);
                Rendering::renderCompletionCubes(this, currentTime);
            }
            
            Rendering::renderEnemies(this, currentTime);
            
            GBuffer::endScenePass(this);
            
            if (m_motionBlurEnabled) {
                bool renderToTexture = needsPostProcessing || m_filterMode != 0 || m_grainOverlayEnabled || m_pixelateEnabled;
                GBuffer::renderMotionBlur(this, renderToTexture);
                
                if (renderToTexture && !needsPostProcessing && GBuffer::m_motionBlurTexture != 0 && GBuffer::m_motionBlurFBO != 0) {
                    int w = width() * m_devicePixelRatio;
                    int h = height() * m_devicePixelRatio;
                    
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, GBuffer::m_motionBlurFBO);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
                    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
                }
                
                if (renderToTexture && needsPostProcessing) {
                    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
                }
            }
            
            if (needsPostProcessing) {
                updateFlashlightPosition();
                if (m_filterMode != 0 || m_grainOverlayEnabled || m_pixelateEnabled || m_bloomEnabled) {
                    renderPostProcessingToTexture();
                } else {
                    renderPostProcessing();
                }
            }
            
            if (m_filterMode != 0 || m_grainOverlayEnabled || m_pixelateEnabled || m_bloomEnabled) {
                glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
                renderPostFilters();
            }
            
            if (m_particleSystem.isEnabled()) {
                GLuint defaultFBO = defaultFramebufferObject();
                glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
                
                int w = width() * m_devicePixelRatio;
                int h = height() * m_devicePixelRatio;
                glViewport(0, 0, w, h);
                
                m_particleSystem.draw(m_camera);
            }
            
            GLuint defaultFBO = defaultFramebufferObject();
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
            int w = width() * m_devicePixelRatio;
            int h = height() * m_devicePixelRatio;
            glViewport(0, 0, w, h);
            m_ui.render();
            
        }
        
        GBuffer::m_prevViewProj = viewProj;
    } else {
        glClearColor(103/255.f, 142/255.f, 166/255.f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(m_shaderProgram);
        
        glm::vec3 camPos = m_camera.getPosition();
    glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, &proj[0][0]);
    glUniformMatrix4fv(m_viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3fv(m_cameraPosLoc, 1, &camPos[0]);
    glUniform1f(m_k_aLoc, m_globalData.ka);
    glUniform1f(m_k_dLoc, m_globalData.kd);
    glUniform1f(m_k_sLoc, m_globalData.ks);

    for (const auto &shape : m_shapes) {
        const ShapeData &data = m_shapeManager.getShapeData(shape.primitive.type);
        const SceneMaterial &mat = shape.primitive.material;
        glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, &shape.ctm[0][0]);
        glUniform4fv(mat_cAmbientLoc, 1, &mat.cAmbient[0]);
        glUniform4fv(mat_cDiffuseLoc, 1, &mat.cDiffuse[0]);
        glUniform4fv(mat_cSpecularLoc, 1, &mat.cSpecular[0]);
        glUniform1f(mat_shinyLoc, mat.shininess);

        glBindVertexArray(data.vao);
        glDrawArrays(GL_TRIANGLES, 0, data.numVertices);
    }
    
    float currentTime = m_elapsedTimer.elapsed() / 1000.0f;
    
    if (m_activeMap != nullptr) {
            Rendering::renderMapBlocks(this);
            Rendering::renderTrees(this);
            Rendering::renderCompletionCubes(this, currentTime);
    }

        Rendering::renderEnemies(this, currentTime);

    glUseProgram(0);
    
    if (m_particleSystem.isEnabled()) {
        GLuint defaultFBO = defaultFramebufferObject();
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        
        int w = width() * m_devicePixelRatio;
        int h = height() * m_devicePixelRatio;
        glViewport(0, 0, w, h);
        
        m_particleSystem.draw(m_camera);
    }
    
    // Render UI on top of everything (after all filters and particles)
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    int w = width() * m_devicePixelRatio;
    int h = height() * m_devicePixelRatio;
    glViewport(0, 0, w, h);
    m_ui.render();
}

}

void Realtime::resizeGL(int w, int h) {
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    m_camera.updateProjectionMatrix(aspect, settings.nearPlane, settings.farPlane);
    makeCurrent();
    GBuffer::resize(this, w, h);
    m_ui.resize(w * m_devicePixelRatio, h * m_devicePixelRatio);
    doneCurrent();
}


void Realtime::sceneChanged() {
    glUseProgram(m_shaderProgram);

    RenderData metaData;
    SceneParser::parse(settings.sceneFilePath, metaData);
    m_globalData = metaData.globalData;
    glUniform1f(m_k_aLoc, metaData.globalData.ka);
    glUniform1f(m_k_dLoc, metaData.globalData.kd);
    glUniform1f(m_k_sLoc, metaData.globalData.ks);
    m_shapes = metaData.shapes;
    m_lights = metaData.lights;

    addLightsToShader(metaData.lights);
    float aspectRatio = static_cast<float>(width()) / static_cast<float>(height());
    m_camera = Camera(metaData.cameraData, aspectRatio, settings.nearPlane, settings.farPlane);
    glUseProgram(0);
    update();
}

void Realtime::addLightsToShader(const std::vector<SceneLightData> &lights) {
    glUseProgram(m_shaderProgram);

    int numLights = std::min((int)lights.size(), 8);
    glUniform1i(m_numLightsLoc, numLights);

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
    }
    glUseProgram(0);
}

void Realtime::addLightsToBlockShader(const std::vector<SceneLightData> &lights) {
    if (m_blockShaderProgram == 0) return;
    
    GLuint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&currentProgram));
    if (currentProgram != m_blockShaderProgram) {
        glUseProgram(m_blockShaderProgram);
    }
    
    GLint linkStatus = 0;
    glGetProgramiv(m_blockShaderProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        std::cerr << "[OpenGL Error] Block shader program is not linked!" << std::endl;
        return;
    }
    
    int numLights = std::min((int)lights.size(), 8);
    if (m_blockNumLightsLoc >= 0) {
        glUniform1i(m_blockNumLightsLoc, numLights);
    }

    GLint kaLoc = glGetUniformLocation(m_blockShaderProgram, "k_a");
    if (kaLoc >= 0) {
        glUniform1f(kaLoc, m_globalData.ka);
    }
    GLint kdLoc = glGetUniformLocation(m_blockShaderProgram, "k_d");
    if (kdLoc >= 0) {
        glUniform1f(kdLoc, m_globalData.kd);
    }
    GLint ksLoc = glGetUniformLocation(m_blockShaderProgram, "k_s");
    if (ksLoc >= 0) {
        glUniform1f(ksLoc, m_globalData.ks);
    }

    for (int i = 0; i < numLights; i++) {
        const SceneLightData &L = lights[i];
        std::string base = "lights[" + std::to_string(i) + "]";

        int lightType;
        if (L.type == LightType::LIGHT_DIRECTIONAL) lightType = 0;
        else if (L.type == LightType::LIGHT_POINT) lightType = 1;
        else lightType = 2;

        GLint typeLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".type").c_str());
        if (typeLoc >= 0) {
            glUniform1i(typeLoc, lightType);
        }
        
        GLint colorLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".color").c_str());
        if (colorLoc >= 0) {
            glUniform3fv(colorLoc, 1, &L.color[0]);
        }
        
        GLint functionLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".function").c_str());
        if (functionLoc >= 0) {
            glUniform3fv(functionLoc, 1, &L.function[0]);
        }
        
        GLint angleLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".angle").c_str());
        if (angleLoc >= 0) {
            glUniform1f(angleLoc, L.angle);
        }
        
        GLint penumbraLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".penumbra").c_str());
        if (penumbraLoc >= 0) {
            glUniform1f(penumbraLoc, L.penumbra);
        }

        GLint posLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".position").c_str());
        if (posLoc >= 0) {
            glUniform3fv(posLoc, 1, &L.pos[0]);
        }
        
        GLint dirLoc = glGetUniformLocation(m_blockShaderProgram, (base + ".direction").c_str());
        if (dirLoc >= 0) {
            glUniform3fv(dirLoc, 1, &L.dir[0]);
        }
    }
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
        // Enable endless mode for procedural generation
        m_activeMap->setEndlessMode(true);
        
        // If map was initialized from builder, we still want endless mode
        // but we can use the noise params that were set
        int mapWidth = m_activeMap->getWidth();
        int mapDepth = m_activeMap->getHeight();
        int maxHeight = m_activeMap->getMaxHeight();
        
        // In endless mode, width/depth might be 0, which is fine
        // In that case, start at origin
        if (mapWidth > 0 && mapDepth > 0) {
            int middleX = mapWidth / 2;
            int middleZ = mapDepth / 2;
            
            int worldX = middleX - (mapWidth / 2);
            int worldZ = middleZ - (mapDepth / 2);
            
            auto blocks = m_activeMap->getBlocksToRender();
            if (!blocks.empty()) {
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
            }
        }
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
    InputHandler::handleKeyPress(this, event);
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    InputHandler::handleKeyRelease(this, event);
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    InputHandler::handleMousePress(this, event);
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    InputHandler::handleMouseRelease(this, event);
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    InputHandler::handleMouseMove(this, event);
}

void Realtime::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_fpsCenterTimer) {
        if (m_fpsMode) {
            int centerX = width() / 2;
            int centerY = height() / 2;
            QPoint globalCenter = mapToGlobal(QPoint(centerX, centerY));
            QPoint currentPos = QCursor::pos();
            QPoint widgetPos = mapFromGlobal(currentPos);
            int deadZone = 100;
            
            if (std::abs(widgetPos.x() - centerX) > deadZone || std::abs(widgetPos.y() - centerY) > deadZone) {
                m_ignoreNextMouseMove = true;
                m_prev_mouse_pos = glm::vec2(centerX, centerY);
                QCursor::setPos(globalCenter);
            }
        }
        return;
    }
    
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();
    
    if (!m_motionBlurAutoEnabled && elapsedms >= 1000) {
        m_motionBlurEnabled = true;
        m_motionBlurAutoEnabled = true;
        emit motionBlurToggled(true);
    }

    m_filterTime += deltaTime;
    
    if (!m_randomAmbientSounds.empty() && m_audioManager != nullptr) {
        m_randomAmbientTimer += deltaTime;
        if (m_randomAmbientTimer >= m_nextRandomAmbientInterval) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> soundDist(0, m_randomAmbientSounds.size() - 1);
            size_t randomIndex = soundDist(gen);
            QString soundPath = m_randomAmbientSounds[randomIndex];
            
            m_audioManager->playSound(soundPath.toUtf8().constData());
            
            m_randomAmbientTimer = 0.0f;
            std::uniform_real_distribution<float> intervalDist(15.0f, 25.0f);
            m_nextRandomAmbientInterval = intervalDist(gen);
        }
    }
    
    bool isMoving = (m_keyMap[Qt::Key_W] || m_keyMap[Qt::Key_A] || m_keyMap[Qt::Key_S] || m_keyMap[Qt::Key_D]);
    bool shouldPlayFootstep = false;
    
    if (m_onGround && isMoving && !m_flyingMode && !m_footstepSounds.empty() && m_audioManager != nullptr) {
        m_walkingTime += deltaTime;
        
        float previousTime = m_walkingTime - deltaTime;
        int previousInterval = static_cast<int>(previousTime / FOOTSTEP_INTERVAL);
        int currentInterval = static_cast<int>(m_walkingTime / FOOTSTEP_INTERVAL);
        
        if (currentInterval > previousInterval) {
            shouldPlayFootstep = true;
        }
    } else {
        m_walkingTime = 0.0f;
    }
    
    if (shouldPlayFootstep) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> footstepDist(0, m_footstepSounds.size() - 1);
        size_t randomIndex = footstepDist(gen);
        QString footstepPath = m_footstepSounds[randomIndex];
        
        m_audioManager->playSound(footstepPath.toUtf8().constData());
        
        //ghost respawn system: every 4th footstep, add a waypoint
        m_footstepCount++;
        if (m_footstepCount >= 4 && !m_isDead) {
            m_footstepCount = 0;
            glm::vec3 pos = m_camera.getPosition();
            glm::vec3 look = m_camera.getLook();
            
            // Add to queue, remove oldest if at capacity
            m_ghostPathQueue.push_back(GhostWaypoint(pos, look));
            if (m_ghostPathQueue.size() > MAX_GHOST_PATH_CAPACITY) {
                m_ghostPathQueue.pop_front();
            }
        }
    }
    
    updateFlashlightCharge(deltaTime);

    if (glm::length(m_pendingRotation) > 0.0001f) {
        glm::vec2 rotationToApply = m_pendingRotation * m_rotationSmoothingFactor;
        m_camera.rotate(rotationToApply.x, rotationToApply.y);
        m_pendingRotation -= rotationToApply;
        if (glm::length(m_pendingRotation) < 0.0001f) {
            m_pendingRotation = glm::vec2(0.0f);
        }
    }

    glm::vec3 cameraPos = m_camera.getPosition();
    
    //update fog color based on biome
    FogSystem::updateFogColor(this, deltaTime);
    
    //update particles
    if (m_particleSystem.isEnabled()) {
        bool isMoving = (m_keyMap[Qt::Key_W] || m_keyMap[Qt::Key_A] || m_keyMap[Qt::Key_S] || m_keyMap[Qt::Key_D]);
        // Get current biome for particle spawning
        int currentBiome = 0; // Default to FIELD
        if (m_activeMap != nullptr) {
            glm::vec3 cameraPos = m_camera.getPosition();
            int playerX = static_cast<int>(std::floor(cameraPos.x));
            int playerZ = static_cast<int>(std::floor(cameraPos.z));
            BiomeType biome = m_activeMap->getBiomeAt(playerX, playerZ);
            currentBiome = static_cast<int>(biome);
        }
        m_particleSystem.update(deltaTime, m_camera, isMoving, static_cast<float>(m_cameraHeightMultiplier), currentBiome);
    }

    if (m_cameraPath.isPlaying()) {
        glm::vec3 newPos, newLook;
        bool wasPlaying = m_cameraPath.isPlaying();
        m_cameraPath.update(deltaTime, newPos, newLook);
        if (wasPlaying) {
            m_camera.setPosition(newPos);
            m_camera.setLook(newLook);
            cameraPos = newPos;
        }
        if (!m_cameraPath.isPlaying() && wasPlaying) {
            emit pathPlaybackFinished();
            // After ghost respawn path finishes, clear the queue and respawn
            if (m_isDead) {
                // Keep camera at final position (first waypoint in reversed queue, which is the oldest/respawn point)
                // The camera path ends at the first waypoint of the reversed queue (oldest waypoint)
                // Get the final waypoint from the path to ensure we're at the exact position
                if (m_cameraPath.getWaypointCount() > 0) {
                    // The path ends at the first waypoint (oldest), so get that position
                    // We need to get it from the original queue before it was reversed
                    if (!m_ghostPathQueue.empty()) {
                        // The first waypoint in the original queue is the respawn point
                        const GhostWaypoint& respawnPoint = m_ghostPathQueue.front();
                        m_camera.setPosition(respawnPoint.position);
                        m_camera.setLook(respawnPoint.lookDirection);
                    }
                }
                
                // Revert to warm LUT (choice 2)
                setLUTChoice(2); // Warm LUT
                
                m_ghostPathQueue.clear();
                m_isDead = false;
                m_footstepCount = 0;
                std::cout << "[Ghost] Respawn complete, cleared path queue" << std::endl;
            }
        }
        if (m_flashlightEnabled) {
            updateFlashlightPosition();
            m_enemyManager.updateWithFlashlight(deltaTime, cameraPos, m_activeMap,
                                                m_flashlightEnabled, m_flashlight.position,
                                                m_flashlight.direction, m_flashlight.coneAngle,
                                                m_audioManager);
        } else {
            m_enemyManager.update(deltaTime, cameraPos, m_activeMap, m_audioManager);
        }
        update();
        return;
    }

        //update enemies with flashlight state
        if (m_flashlightEnabled) {
            updateFlashlightPosition();
            m_enemyManager.updateWithFlashlight(deltaTime, cameraPos, m_activeMap,
                                                m_flashlightEnabled, m_flashlight.position,
                                                m_flashlight.direction, m_flashlight.coneAngle,
                                                m_audioManager);
        } else {
            m_enemyManager.update(deltaTime, cameraPos, m_activeMap, m_audioManager);
        }

    float moveSpeed = 1.125f * deltaTime;
    
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
    
    //made a cool sprint fov change
    // Death path FOV: increase by +30 when path is playing, then smoothly decrease when done
    float targetFOV = m_baseFOV;
    if (m_isDead && m_cameraPath.isPlaying()) {
        // Death path is playing - increase FOV by +30
        targetFOV = m_baseFOV + 30.0f;
    } else if (isSprinting) {
        // Sprint FOV increase
        targetFOV = m_baseFOV + 12.0f;
    }
    
    float fovSpeed = 70.0f;
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

    // Update player health system
    updatePlayerHealth(deltaTime);
    updateCompletionCubePenalties(deltaTime);
    
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
            //the show must go on so i just caught any errors lol
        } catch (...) {
            chunkX = 0;
            chunkZ = 0;
        }
    }
    
    emit telemetryUpdate(cameraPos.x, cameraPos.y, cameraPos.z, chunkX, chunkZ);
}

bool Realtime::isCompletionCubeWithinOneBlock(const glm::vec3& cameraPos) {
    if (m_activeMap == nullptr) {
        return false;
    }
    
    const auto& chunks = m_activeMap->getChunks();
    const float DAMAGE_DISTANCE = 1.0f; // 1 block distance
    
    for (const auto& chunkPair : chunks) {
        if (chunkPair.second == nullptr || !chunkPair.second->isPopulated()) {
            continue;
        }
        
        const auto& completionCubes = chunkPair.second->getCompletionCubes();
        
        for (const auto& cube : completionCubes) {
            if (cube.isCollected()) {
                continue;
            }
            glm::vec3 cubePos = cube.getPosition();
            float distToCube = glm::length(cameraPos - cubePos);
            
            if (distToCube <= DAMAGE_DISTANCE) {
                return true;
            }
        }
    }
    
    return false;
}

void Realtime::damagePlayer(float damageAmount) {
    float previousHealth = m_playerHealth;
    m_playerHealth -= damageAmount;
    m_playerHealth = glm::clamp(m_playerHealth, 0.0f, 100.0f);
    
    // Reset healing delay timer
    m_timeSinceLastDamage = 0.0f;
    
    // Update opacity immediately
    float newOpacity = (100.0f - m_playerHealth) / 100.0f;
    m_grainOpacity = newOpacity;
    
    // Auto-enable grain overlay if opacity is over 0.1f (health below 90)
    if (newOpacity > 0.1f && !m_grainOverlayEnabled) {
        m_grainOverlayEnabled = true;
    }
}

void Realtime::updatePlayerHealth(float deltaTime) {
    float previousHealth = m_playerHealth;
    
    if (m_activeMap == nullptr) {
        // Recover health if no map
        m_playerHealth += m_healthRecoveryRate * deltaTime;
        m_playerHealth = glm::clamp(m_playerHealth, 0.0f, 100.0f);
        
        // Opacity: 0 for full health, 1 for no health
        float newOpacity = (100.0f - m_playerHealth) / 100.0f;
        m_grainOpacity = newOpacity;
        
        return;
    }
    
    glm::vec3 cameraPos = m_camera.getPosition();
    bool cubeNearby = isCompletionCubeWithinOneBlock(cameraPos);
    
    const float ENEMY_DAMAGE_DISTANCE = 1.2f;
    const float damagePerSecond = 45.0f;
    
    int enemyCount = m_enemyManager.getEnemyCount();
    bool enemyNearby = false;
    bool damageApplied = false;
    
    // Check all enemies - if any enemy is within 1.0f AND a cube is nearby, damage player
    for (int i = 0; i < enemyCount; ++i) {
        const Enemy* enemy = m_enemyManager.getEnemy(i);
        if (enemy != nullptr && enemy->isAlive()) {
            glm::vec3 enemyPos = enemy->getPosition();
            float distToEnemy = glm::length(cameraPos - enemyPos);
            
            if (distToEnemy <= ENEMY_DAMAGE_DISTANCE) {
                enemyNearby = true;
                
                if (!damageApplied) {
                    float damageThisFrame = damagePerSecond * deltaTime;
                    damagePlayer(damageThisFrame);
                    damageApplied = true;
                    return; // Exit early after dealing damage
                }
            }
        }
    }
    
    
    // Update time since last damage
    if (!damageApplied) {
        m_timeSinceLastDamage += deltaTime;
    }
    
    // Recover health only after delay has passed since last damage
    if (m_timeSinceLastDamage >= m_healingDelay) {
        m_playerHealth += m_healthRecoveryRate * deltaTime;
        m_playerHealth = glm::clamp(m_playerHealth, 0.0f, 100.0f);
    }
    
    // Opacity: 0 for full health (100), 1 for no health (0)
    float newOpacity = (100.0f - m_playerHealth) / 100.0f;
    
    m_grainOpacity = newOpacity;
    
    // Auto-enable grain overlay if opacity is over 0.1f (health below 90)
    if (newOpacity > 0.1f && !m_grainOverlayEnabled) {
        m_grainOverlayEnabled = true;
    }
    
    // Check for death (health < 0.01) and trigger ghost respawn
    if (m_playerHealth < 0.01f && !m_isDead) {
        handleDeath();
    }
}

void Realtime::updateCompletionCubePenalties(float deltaTime) {
    // Update field penalty
    if (m_fieldPenaltyTimer > 0.0f) {
        m_fieldPenaltyTimer -= deltaTime;
        // Calculate penalty value: 0.0 at start, 1.0 when timer reaches 0
        float elapsed = PENALTY_INCREASE_DURATION - m_fieldPenaltyTimer;
        m_fieldPenaltyValue = glm::clamp(elapsed / PENALTY_INCREASE_DURATION, 0.0f, 1.0f);
        if (m_fieldPenaltyTimer <= 0.0f) {
            m_fieldPenaltyTimer = 0.0f;
            m_fieldPenaltyValue = 1.0f;
        }
    }
    
    // Update mountain penalty
    if (m_mountainPenaltyTimer > 0.0f) {
        m_mountainPenaltyTimer -= deltaTime;
        // Calculate penalty value: 0.0 at start, 1.0 when timer reaches 0
        float elapsed = PENALTY_INCREASE_DURATION - m_mountainPenaltyTimer;
        m_mountainPenaltyValue = glm::clamp(elapsed / PENALTY_INCREASE_DURATION, 0.0f, 1.0f);
        if (m_mountainPenaltyTimer <= 0.0f) {
            m_mountainPenaltyTimer = 0.0f;
            m_mountainPenaltyValue = 1.0f;
        }
    }
    
    // Update forest penalty
    if (m_forestPenaltyTimer > 0.0f) {
        m_forestPenaltyTimer -= deltaTime;
        // Calculate penalty value: 0.0 at start, 1.0 when timer reaches 0
        float elapsed = PENALTY_INCREASE_DURATION - m_forestPenaltyTimer;
        m_forestPenaltyValue = glm::clamp(elapsed / PENALTY_INCREASE_DURATION, 0.0f, 1.0f);
        if (m_forestPenaltyTimer <= 0.0f) {
            m_forestPenaltyTimer = 0.0f;
            m_forestPenaltyValue = 1.0f;
        }
    }
}

void Realtime::handleDeath() {
    std::cout << "[Ghost] Player died! Starting ghost respawn sequence..." << std::endl;
    
    m_isDead = true;
    
    m_originalLUTChoice = m_lutChoice;
    setLUTChoice(3); //black and white
    
    //kill all enemies when player dies
    m_enemyManager.killAllEnemies();
    
    //remove grain effect by setting health to full
    m_playerHealth = 100.0f;
    m_grainOpacity = 0.0f;
    m_grainOverlayEnabled = false;
    
    //add current death position as final waypoint
    glm::vec3 deathPos = m_camera.getPosition();
    glm::vec3 deathLook = m_camera.getLook();
    m_ghostPathQueue.push_back(GhostWaypoint(deathPos, deathLook));
    
    //need at least 2 waypoints to play a path
    if (m_ghostPathQueue.size() < 2) {
        std::cout << "[Ghost] Not enough waypoints for ghost respawn" << std::endl;
        m_ghostPathQueue.clear();
        m_isDead = false;
        setLUTChoice(m_originalLUTChoice);
        return;
    }
    
    m_cameraPath.clear();
    
    std::deque<GhostWaypoint> reversedQueue = m_ghostPathQueue;
    std::reverse(reversedQueue.begin(), reversedQueue.end());
    
    for (const auto& waypoint : reversedQueue) {
        m_cameraPath.addWaypoint(waypoint.position, waypoint.lookDirection);
    }
    
    // Play the path backwards (from death point to oldest waypoint)
    float pathDuration = 7.0f;
    m_cameraPath.setPlaybackDuration(pathDuration);
    m_cameraPath.startPlayback(pathDuration);
    
    std::cout << "[Ghost] Playing ghost respawn path with " << reversedQueue.size() 
              << " waypoints over " << pathDuration << " seconds" << std::endl;
}

//all from ui control
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

void Realtime::setMotionBlurEnabled(bool enabled) {
    m_motionBlurEnabled = enabled;
    emit motionBlurToggled(enabled);
}

void Realtime::setMotionBlurSamples(int samples) {
    m_motionBlurSamples = samples;
}

void Realtime::setDepthVisualizationEnabled(bool enabled) {
    m_depthVisualizationEnabled = enabled;
}

void Realtime::setGBufferVisualizationMode(int mode) {
    m_gbufferVisualizationMode = mode;
}

void Realtime::setFpsMode(bool enabled) {
    m_fpsMode = enabled;
    if (enabled) {
        setCursor(Qt::BlankCursor);
        grabMouse();
        setFocus();
        int centerX = width() / 2;
        int centerY = height() / 2;
        m_prev_mouse_pos = glm::vec2(centerX, centerY);
        QPoint globalCenter = mapToGlobal(QPoint(centerX, centerY));
        QCursor::setPos(globalCenter);
        QCoreApplication::processEvents();
        m_ignoreNextMouseMove = true;
        m_fpsCenterTimer = startTimer(16);
    } else {
        if (m_fpsCenterTimer != 0) {
            killTimer(m_fpsCenterTimer);
            m_fpsCenterTimer = 0;
        }
        releaseMouse();
        setCursor(Qt::ArrowCursor);
        m_ignoreNextMouseMove = false;
    }
}

void Realtime::setFogEnabled(bool enabled) {
    m_fogEnabled = enabled;
}

void Realtime::setFogColor(float r, float g, float b) {
    m_fogColor = glm::vec3(r, g, b);
    m_targetFogColor = m_fogColor;
}

glm::vec3 Realtime::getBiomeFogColor(BiomeType biome) const {
    return FogSystem::getBiomeFogColor(biome);
}

void Realtime::setFogIntensity(float intensity) {
    m_fogIntensity = intensity;
}

void Realtime::teleportToOrigin() {
    const float BASE_EYE_HEIGHT = 1.6f;
    float cameraHeightMultiplier = static_cast<float>(m_cameraHeightMultiplier);
    cameraHeightMultiplier = std::max(0.25f, std::min(3.0f, cameraHeightMultiplier));
    float eyeHeight = BASE_EYE_HEIGHT * cameraHeightMultiplier;
    
    glm::vec3 originPos(0.0f, eyeHeight, 0.0f);
    m_camera.setPosition(originPos);
    m_velocity = glm::vec3(0.0f);
}

void Realtime::setFilterMode(int mode) {
    m_filterMode = mode;
}

void Realtime::setGrainOverlayEnabled(bool enabled) {
    m_grainOverlayEnabled = enabled;
}

void Realtime::setGrainOpacity(float opacity) {
    m_grainOpacity = glm::clamp(opacity, 0.0f, 1.0f);
}

void Realtime::setPixelateEnabled(bool enabled) {
    m_pixelateEnabled = enabled;
}

void Realtime::setBloomEnabled(bool enabled) {
    m_bloomEnabled = enabled;
}

void Realtime::setLUTChoice(int choice) {
    m_lutChoice = choice;
    if (m_filterLUTTexture != 0) {
        loadLUT(choice);
    }
}

// Particle system setters
void Realtime::setParticlesEnabled(bool enabled) {m_particleSystem.setEnabled(enabled);}
void Realtime::setDirtParticlesEnabled(bool enabled) {m_particleSystem.setDirtParticlesEnabled(enabled);}
void Realtime::setFogWispsEnabled(bool enabled) {m_particleSystem.setFogWispsEnabled(enabled);}
void Realtime::setDirtSpawnRate(float rate) {m_particleSystem.setDirtSpawnRate(rate);}
void Realtime::setFogWispSpawnInterval(float interval) {m_particleSystem.setFogWispSpawnInterval(interval);}
void Realtime::setMaxParticles(int maxParticles) {m_particleSystem.setMaxParticles(maxParticles);}

void Realtime::setFlashlightEnabled(bool enabled) {
    bool previousState = m_flashlightEnabled;
    
    // Allow toggling on/off anytime (penalty only affects recharging)
    if (enabled) {
        // Can turn on if we have charge (penalty doesn't prevent toggling)
        if (m_flashlightCharge > 0.0f) {
            m_flashlightEnabled = true;
        } else {
            m_flashlightEnabled = false;
        }
    } else {
        // Flashlight is being turned off
        // If it was on and had charge, this is a manual turn-off (2 second penalty for recharging)
        // If it ran out of charge, penalty is already set in updateFlashlightCharge
        if (m_flashlightEnabled && m_flashlightCharge > 0.0f) {
            // Manual turn-off: apply 2 second penalty (only affects recharging)
            m_flashlightPenaltyTimer = 2.0f;
        }
        m_flashlightEnabled = false;
    }
    
    // Play sound effects when state changes
    if (m_flashlightEnabled != previousState) {
        if (m_audioManager != nullptr) {
            QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            if (m_flashlightEnabled) {
                m_audioManager->playSound((tempDir + "/flashlight_on.wav").toUtf8().constData());
            } else {
                m_audioManager->playSound((tempDir + "/flashlight_off.wav").toUtf8().constData());
            }
        }
        
        // Update fog intensity based on flashlight state
        if (m_flashlightEnabled) {
            m_fogIntensity = 0.20f;
        } else {
            m_fogIntensity = 0.13f;
        }
    }
}

void Realtime::updateFlashlightCharge(float deltaTime) {
    if (m_flashlightEnabled) {
        //deplete charge at 4% per second
        m_flashlightCharge -= 4.0f * deltaTime;
        
        if (m_flashlightCharge <= 0.0f) {
            m_flashlightCharge = 0.0f;
            bool wasEnabled = m_flashlightEnabled;
            m_flashlightEnabled = false; //turn off when charge reaches 0
            m_flashlightPenaltyTimer = 5.0f; //5 second penalty
            m_flashlightFlickerIntensity = 1.0f;
            
            if (wasEnabled) {
                m_fogIntensity = 0.13f;
            }
            
            if (wasEnabled && m_audioManager != nullptr) {
                QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                m_audioManager->playSound((tempDir + "/flashlight_off.wav").toUtf8().constData());
            }
        }
    } else {
        if (m_flashlightPenaltyTimer > 0.0f) {
            m_flashlightPenaltyTimer -= deltaTime;
            if (m_flashlightPenaltyTimer < 0.0f) {
                m_flashlightPenaltyTimer = 0.0f;
            }
        } else {
            m_flashlightCharge += 10.0f * deltaTime;
            

            if (m_flashlightCharge >= 100.0f) {
                m_flashlightCharge = 100.0f;
            }
        }
        m_flashlightFlickerIntensity = 1.0f; //no flicker when off
    }
    
    if (m_flashlightEnabled && m_flashlightCharge <= 25.0f) {
        m_flashlightFlickerTime += deltaTime;
        
        float chargeFactor = 1.0f - (m_flashlightCharge / 25.0f);
        float flickerAmount = 0.3f + chargeFactor * 0.7f;
        
        float fastFlicker = std::sin(m_flashlightFlickerTime * 12.0f);
        float mediumFlicker = std::sin(m_flashlightFlickerTime * 5.0f);
        float slowFlicker = std::sin(m_flashlightFlickerTime * 1.8f);
        
        float quantizedFast = std::floor(fastFlicker * 4.0f) / 4.0f;
        float quantizedMedium = std::floor(mediumFlicker * 3.0f) / 3.0f;
        float quantizedSlow = std::floor(slowFlicker * 2.0f) / 2.0f;
        
        quantizedFast = quantizedFast * 0.5f + 0.5f;
        quantizedMedium = quantizedMedium * 0.5f + 0.5f;
        quantizedSlow = quantizedSlow * 0.5f + 0.5f;
        
        float combinedFlicker = (quantizedFast * 0.45f + quantizedMedium * 0.35f + quantizedSlow * 0.2f);
        
        float dropPattern = std::sin(m_flashlightFlickerTime * 2.3f);
        float dropThreshold = 0.7f + chargeFactor * 0.25f;
        float dropFactor = (dropPattern > dropThreshold) ? 0.3f : 1.0f;
        
        combinedFlicker *= dropFactor;
        
        //flicker amount
        float finalFlicker = combinedFlicker * flickerAmount + (1.0f - flickerAmount);
        m_flashlightFlickerIntensity = std::max(0.15f, finalFlicker); // Never go completely dark, minimum 15%
    } else {
        // No flicker when charge is above 25% or flashlight is off
        m_flashlightFlickerIntensity = 1.0f;
        if (!m_flashlightEnabled) {
            m_flashlightFlickerTime = 0.0f; // Reset time when off
        }
    }
    
    // Emit signal to update UI (only update checkbox if state changed)
    static bool lastFlashlightState = false;
    if (m_flashlightEnabled != lastFlashlightState) {
        lastFlashlightState = m_flashlightEnabled;
        // Don't emit here - let the UI update via the charge signal
    }
    
    // Emit charge update signal
    emit flashlightChargeChanged(m_flashlightCharge, m_flashlightPenaltyTimer > 0.0f);
}

void Realtime::updateFlashlightPosition() {
    glm::vec3 right = glm::normalize(glm::cross(m_camera.getLook(), m_camera.getUp()));
    glm::vec3 camOffset(0.2f, -0.1f, 0.0f);
    glm::mat3 camBasis(right, glm::normalize(m_camera.getUp()), glm::normalize(m_camera.getLook()));
    m_flashlight.position = m_camera.getPosition() + camBasis * camOffset;
    m_flashlight.direction = glm::normalize(m_camera.getLook());
}

void Realtime::renderPostProcessing() {
    if (m_postShaderProgram == 0 || m_postQuadVAO == 0) {
        return;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width() * m_devicePixelRatio, height() * m_devicePixelRatio);
    
    glUseProgram(m_postShaderProgram);
    glBindVertexArray(m_postQuadVAO);
    
    glActiveTexture(GL_TEXTURE0);
    if (m_motionBlurEnabled && GBuffer::m_motionBlurTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, GBuffer::m_motionBlurTexture);
    } else {
        glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneTexture);
    }
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "colorTexture"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneDepthTexture);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "depthTexture"), 1);
    
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "nearPlane"), settings.nearPlane);
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "farPlane"), settings.farPlane);
    
    glm::vec3 camPos = m_camera.getPosition();
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "camPos"), 1, &camPos[0]);
    
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "enableFog"), m_fogEnabled ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "enableFlashlight"), m_flashlightEnabled ? 1 : 0);
    
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "fogColor"), 1, &m_fogColor[0]);
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "fogIntensity"), m_fogIntensity);
    
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "flashlightPos"), 1, &m_flashlight.position[0]);
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "flashlightDir"), 1, &m_flashlight.direction[0]);
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "flashlightConeAngle"), m_flashlight.coneAngle);
    
    // Apply flicker intensity to flashlight color
    glm::vec3 flickeredColor = m_flashlight.color * m_flashlightFlickerIntensity;
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "flashlightColor"), 1, &flickeredColor[0]);
    
    glm::mat4 invView = glm::inverse(m_camera.getViewMatrix());
    glm::mat4 invProj = glm::inverse(m_camera.getProjMatrix());
    glUniformMatrix4fv(glGetUniformLocation(m_postShaderProgram, "inverseViewMatrix"), 1, GL_FALSE, &invView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_postShaderProgram, "inverseProjectionMatrix"), 1, GL_FALSE, &invProj[0][0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
}


// Filter system implementation
void Realtime::initializeFilterSystem() {
    try {
        m_filterShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/postfilter.vert",
            ":/resources/shaders/postfilter.frag"
        );
    } catch (const std::runtime_error &e) {
        std::cerr << "Error loading filter shader: " << e.what() << std::endl;
        return;
    }
    
    // Create fullscreen quad for filter rendering (same as post-processing quad)
    std::vector<GLfloat> fullscreen_quad_data = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f
    };
    
    glGenBuffers(1, &m_filterQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_filterQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size() * sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_filterQuadVAO);
    glBindVertexArray(m_filterQuadVAO);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Set up shader uniforms
    if (m_filterShaderProgram != 0) {
        glUseProgram(m_filterShaderProgram);
        glUniform1i(glGetUniformLocation(m_filterShaderProgram, "colorTex"), 0);
        glUniform1i(glGetUniformLocation(m_filterShaderProgram, "depthTex"), 1);
        glUniform1i(glGetUniformLocation(m_filterShaderProgram, "lutTex"), 2);
        glUniform1f(glGetUniformLocation(m_filterShaderProgram, "near"), settings.nearPlane);
        glUniform1f(glGetUniformLocation(m_filterShaderProgram, "far"), settings.farPlane);
        glUseProgram(0);
    }
    
    // Create filter FBO and texture
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    
    glGenFramebuffers(1, &m_filterFBO);
    glGenTextures(1, &m_filterTexture);
    
    glBindTexture(GL_TEXTURE_2D, m_filterTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_filterFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_filterTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Load initial LUT
    loadLUT(2);
    
    // Initialize bloom pipeline
    initializeBloom();
}

void Realtime::initializeBloom() {
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    
    if (width <= 0 || height <= 0) {
        std::cerr << "Bloom initialization: Invalid dimensions" << std::endl;
        return;
    }
    
    //bloom shaders
    try {
        m_bloomShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/postfilter.vert",
            ":/resources/shaders/bloomcombine.frag"
        );
        m_blurShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/blur.vert",
            ":/resources/shaders/blur.frag"
        );
        if (m_bloomShaderProgram == 0 || m_blurShaderProgram == 0) {
            std::cerr << "Bloom shader compilation failed" << std::endl;
            return;
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "Error loading bloom shaders: " << e.what() << std::endl;
        return;
    }
    
    // Create bloom extraction FBO and texture
    glGenFramebuffers(1, &m_bloomExtractFBO);
    glGenTextures(1, &m_bloomExtractTexture);
    
    glBindTexture(GL_TEXTURE_2D, m_bloomExtractTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_bloomExtractFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomExtractTexture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Bloom extract FBO incomplete: " << status << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    for (int i = 0; i < 2; i++) {
        glGenFramebuffers(1, &m_bloomBlurFBO[i]);
        glGenTextures(1, &m_bloomBlurTexture[i]);
        
        glBindTexture(GL_TEXTURE_2D, m_bloomBlurTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_bloomBlurFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomBlurTexture[i], 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Bloom blur FBO[" << i << "] incomplete: " << status << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    //blur shader uniforms
    if (m_blurShaderProgram != 0) {
        glUseProgram(m_blurShaderProgram);
        glUniform1i(glGetUniformLocation(m_blurShaderProgram, "image"), 0);
        glUseProgram(0);
    }
    
    //bloom combine shader uniforms
    if (m_bloomShaderProgram != 0) {
        glUseProgram(m_bloomShaderProgram);
        glUniform1i(glGetUniformLocation(m_bloomShaderProgram, "sceneTexture"), 0);
        glUniform1i(glGetUniformLocation(m_bloomShaderProgram, "bloomTexture"), 1);
        glUniform1f(glGetUniformLocation(m_bloomShaderProgram, "bloomIntensity"), 1.0f); // Higher intensity = more exaggerated bloom
        glUseProgram(0);
    }
    
    m_bloomInitialized = true;
}

void Realtime::renderBloom() {
    if (!m_bloomEnabled || !m_bloomInitialized || m_blurShaderProgram == 0 || m_bloomShaderProgram == 0) {
        return;
    }
    
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    
    //resize bloom textures if needed
    if (m_bloomExtractTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, m_bloomExtractTexture);
        GLint texWidth, texHeight;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
        if (texWidth != width || texHeight != height) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    for (int i = 0; i < 2; i++) {
        if (m_bloomBlurTexture[i] != 0) {
            glBindTexture(GL_TEXTURE_2D, m_bloomBlurTexture[i]);
            GLint texWidth, texHeight;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
            if (texWidth != width || texHeight != height) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    
    // Step 1: Extract bright colors from the scene
    static GLuint extractShader = 0;
    if (extractShader == 0) {
        try {
            extractShader = ShaderLoader::createShaderProgram(
                ":/resources/shaders/postfilter.vert",
                ":/resources/shaders/bloomextract.frag"
            );
        } catch (const std::runtime_error &e) {
            std::cerr << "Error loading bloom extract shader: " << e.what() << std::endl;
            return;
        }
    }
    
    if (extractShader != 0 && m_bloomExtractFBO != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_bloomExtractFBO);
        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, width, height);
        
        glUseProgram(extractShader);
        glBindVertexArray(m_filterQuadVAO);
        
        glActiveTexture(GL_TEXTURE0);
        bool needsPostProcessing = (m_fogEnabled || m_flashlightEnabled) && m_postShaderProgram != 0;
        if (needsPostProcessing && m_filterTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, m_filterTexture);
        } else if (m_motionBlurEnabled && GBuffer::m_motionBlurTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, GBuffer::m_motionBlurTexture);
        } else {
            glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneTexture);
        }
        glUniform1i(glGetUniformLocation(extractShader, "sceneTexture"), 0);
        glUniform1f(glGetUniformLocation(extractShader, "bloomThreshold"), 0.6f); // Lower threshold = more colors extracted
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    // Step 2: Two-pass Gaussian blur (horizontal then vertical)
    bool horizontal = true;
    bool firstIteration = true;
    int blurAmount = 15; // Number of blur iterations (more = more spread-out glow)
    int finalBufferIndex = 0; // Track which buffer has the final result
    
    glUseProgram(m_blurShaderProgram);
    glBindVertexArray(m_filterQuadVAO);
    
    for (int i = 0; i < blurAmount; i++) {
        int targetBuffer = horizontal ? 1 : 0;
        finalBufferIndex = targetBuffer; // Track the last buffer we wrote to
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_bloomBlurFBO[targetBuffer]);
        glUniform1i(glGetUniformLocation(m_blurShaderProgram, "horizontal"), horizontal ? 1 : 0);
        
        glActiveTexture(GL_TEXTURE0);
        if (firstIteration) {
            glBindTexture(GL_TEXTURE_2D, m_bloomExtractTexture);
            firstIteration = false;
        } else {
            // Read from the opposite buffer (the one we didn't write to)
            glBindTexture(GL_TEXTURE_2D, m_bloomBlurTexture[horizontal ? 0 : 1]);
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
    }
    
    // Store which buffer has the final result for use in combine step
    // After 15 iterations (odd), finalBufferIndex will be 1 (last write was to buffer 1)
    
    glBindVertexArray(0);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Realtime::loadLUT(int choice) {
    QString lutPath;
    switch(choice) {
        case 0: lutPath = ":/resources/luts/neutral.png"; break;
        case 1: lutPath = ":/resources/luts/fancy.png"; break;
        case 2: lutPath = ":/resources/luts/warm.png"; break;
        case 3: lutPath = ":/resources/luts/blackandwhite.png"; break;
        case 4: lutPath = ":/resources/luts/cool.png"; break;
        case 5: lutPath = ":/resources/luts/green.png"; break;
        case 6: lutPath = ":/resources/luts/blue.png"; break;
        case 7: lutPath = ":/resources/luts/horror.png"; break;
        default: lutPath = ":/resources/luts/neutral.png"; break;
    }
    
    QImage lutImage(lutPath);
    if (lutImage.isNull()) {
        std::cerr << "Failed to load LUT: " << lutPath.toStdString() << std::endl;
        return;
    }
    
    QImage lut = lutImage.convertToFormat(QImage::Format_RGBA8888);
    m_lutSize = 16.0f;
    
    if (m_filterLUTTexture == 0) {
        glGenTextures(1, &m_filterLUTTexture);
    }
    
    glBindTexture(GL_TEXTURE_2D, m_filterLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 lut.width(), lut.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, lut.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Realtime::renderPostFilters() {
    if ((m_filterMode == 0 && !m_grainOverlayEnabled && !m_pixelateEnabled && !m_bloomEnabled) || m_filterShaderProgram == 0 || m_filterQuadVAO == 0) {
        return;
    }
    
    bool needsPostProcessing = (m_fogEnabled || m_flashlightEnabled) && m_postShaderProgram != 0;
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    
    if (m_bloomEnabled && m_blurShaderProgram != 0 && m_bloomInitialized) {
        renderBloom();
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    
    if (m_bloomEnabled && m_bloomShaderProgram != 0 && m_bloomInitialized) {
        glUseProgram(m_bloomShaderProgram);
        glBindVertexArray(m_filterQuadVAO);
        
        glActiveTexture(GL_TEXTURE0);
        if (needsPostProcessing && m_filterTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, m_filterTexture);
        } else if (m_motionBlurEnabled && GBuffer::m_motionBlurTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, GBuffer::m_motionBlurTexture);
        } else {
            glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneTexture);
        }
        glUniform1i(glGetUniformLocation(m_bloomShaderProgram, "sceneTexture"), 0);
        
        glActiveTexture(GL_TEXTURE1);

        if (m_bloomBlurTexture[1] != 0) {
            glBindTexture(GL_TEXTURE_2D, m_bloomBlurTexture[1]);
        } else if (m_bloomBlurTexture[0] != 0) {
            glBindTexture(GL_TEXTURE_2D, m_bloomBlurTexture[0]);
        } else {
            glBindVertexArray(0);
            glUseProgram(0);
            glEnable(GL_DEPTH_TEST);
            return;
        }
        glUniform1i(glGetUniformLocation(m_bloomShaderProgram, "bloomTexture"), 1);
        glUniform1f(glGetUniformLocation(m_bloomShaderProgram, "bloomIntensity"), 1.0f);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        if (m_filterMode != 0 || m_grainOverlayEnabled || m_pixelateEnabled) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_filterFBO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
        } else {
            glBindVertexArray(0);
            glUseProgram(0);
            glEnable(GL_DEPTH_TEST);
            return;
        }
    }
    
    glUseProgram(m_filterShaderProgram);
    glBindVertexArray(m_filterQuadVAO);
    
    glActiveTexture(GL_TEXTURE0);
    if (m_bloomEnabled && m_bloomShaderProgram != 0) {
        glBindTexture(GL_TEXTURE_2D, m_filterTexture);
    } else if (needsPostProcessing && m_filterTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, m_filterTexture);
    } else if (m_motionBlurEnabled && GBuffer::m_motionBlurTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, GBuffer::m_motionBlurTexture);
    } else {
        glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneTexture);
    }
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneDepthTexture);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_filterLUTTexture);
    
    glUniform1i(glGetUniformLocation(m_filterShaderProgram, "postProcessing"), (m_filterMode != 0 || m_grainOverlayEnabled || m_pixelateEnabled) ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_filterShaderProgram, "ppMode"), m_filterMode);
    glUniform1f(glGetUniformLocation(m_filterShaderProgram, "near"), settings.nearPlane);
    glUniform1f(glGetUniformLocation(m_filterShaderProgram, "far"), settings.farPlane);
    glUniform1f(glGetUniformLocation(m_filterShaderProgram, "offset"), m_filterTime * 2.0f * 3.14159f * 0.75f);
    glUniform1f(glGetUniformLocation(m_filterShaderProgram, "time"), m_filterTime);
    glUniform1f(glGetUniformLocation(m_filterShaderProgram, "lutSize"), m_lutSize);
    GLint grainOverlayLoc = glGetUniformLocation(m_filterShaderProgram, "enableGrainOverlay");
    if (grainOverlayLoc >= 0) {
        glUniform1i(grainOverlayLoc, m_grainOverlayEnabled ? 1 : 0);
    }
    
    GLint grainSizeLoc = glGetUniformLocation(m_filterShaderProgram, "grainSize");
    if (grainSizeLoc >= 0) {
        glUniform1f(grainSizeLoc, 0.1f);
    }
    
    GLint grainOpacityLoc = glGetUniformLocation(m_filterShaderProgram, "grainOpacity");
    if (grainOpacityLoc >= 0) {
        glUniform1f(grainOpacityLoc, m_grainOpacity);
    }
    GLint pixelateLoc = glGetUniformLocation(m_filterShaderProgram, "enablePixelate");
    if (pixelateLoc >= 0) {
        glUniform1i(pixelateLoc, m_pixelateEnabled ? 1 : 0);
    }
    
    GLint bloomLoc = glGetUniformLocation(m_filterShaderProgram, "enableBloom");
    if (bloomLoc >= 0) {
        glUniform1i(bloomLoc, m_bloomEnabled ? 1 : 0);
    }
    
    GLint fieldPenaltyLoc = glGetUniformLocation(m_filterShaderProgram, "fieldPenalty");
    if (fieldPenaltyLoc >= 0) {
        glUniform1f(fieldPenaltyLoc, m_fieldPenaltyValue);
    }
    
    GLint forestPenaltyLoc = glGetUniformLocation(m_filterShaderProgram, "forestPenalty");
    if (forestPenaltyLoc >= 0) {
        glUniform1f(forestPenaltyLoc, m_forestPenaltyValue);
    }
    
    GLint mountainPenaltyLoc = glGetUniformLocation(m_filterShaderProgram, "mountainPenalty");
    if (mountainPenaltyLoc >= 0) {
        glUniform1f(mountainPenaltyLoc, m_mountainPenaltyValue);
    }
    
    GLint colorPenaltyStrengthLoc = glGetUniformLocation(m_filterShaderProgram, "colorPenaltyStrength");
    if (colorPenaltyStrengthLoc >= 0) {
        glUniform1f(colorPenaltyStrengthLoc, 0.5f);
    }
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
}

void Realtime::renderPostProcessingToTexture() {
    if (m_postShaderProgram == 0 || m_postQuadVAO == 0) {
        return;
    }
    
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    
    if (m_filterTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, m_filterTexture);
        GLint texWidth, texHeight;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
        if (texWidth != width || texHeight != height) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_filterFBO);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    
    glUseProgram(m_postShaderProgram);
    glBindVertexArray(m_postQuadVAO);
    
    glActiveTexture(GL_TEXTURE0);
    if (m_motionBlurEnabled && GBuffer::m_motionBlurTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, GBuffer::m_motionBlurTexture);
    } else {
        glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneTexture);
    }
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "colorTexture"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GBuffer::m_sceneDepthTexture);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "depthTexture"), 1);
    
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "nearPlane"), settings.nearPlane);
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "farPlane"), settings.farPlane);
    
    glm::vec3 camPos = m_camera.getPosition();
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "camPos"), 1, &camPos[0]);
    
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "enableFog"), m_fogEnabled ? 1 : 0);
    glUniform1i(glGetUniformLocation(m_postShaderProgram, "enableFlashlight"), m_flashlightEnabled ? 1 : 0);
    
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "fogColor"), 1, &m_fogColor[0]);
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "fogIntensity"), m_fogIntensity);
    
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "flashlightPos"), 1, &m_flashlight.position[0]);
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "flashlightDir"), 1, &m_flashlight.direction[0]);
    glUniform1f(glGetUniformLocation(m_postShaderProgram, "flashlightConeAngle"), m_flashlight.coneAngle);
    
    //flicker intensity to flashlight color
    glm::vec3 flickeredColor = m_flashlight.color * m_flashlightFlickerIntensity;
    glUniform3fv(glGetUniformLocation(m_postShaderProgram, "flashlightColor"), 1, &flickeredColor[0]);
    
    glm::mat4 invView = glm::inverse(m_camera.getViewMatrix());
    glm::mat4 invProj = glm::inverse(m_camera.getProjMatrix());
    glUniformMatrix4fv(glGetUniformLocation(m_postShaderProgram, "inverseViewMatrix"), 1, GL_FALSE, &invView[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_postShaderProgram, "inverseProjectionMatrix"), 1, GL_FALSE, &invProj[0][0]);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
}

bool Realtime::isMoving() {
    return (m_keyMap[Qt::Key_W] || m_keyMap[Qt::Key_A] || m_keyMap[Qt::Key_S] || m_keyMap[Qt::Key_D]);
}

void Realtime::saveViewportImage(std::string filePath) {
    //make sure we have the right context and everything has been drawn
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
