#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <deque>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include "shapes/Shape.h"
#include "utils/camera.h"
#include "utils/camerapath.h"
#include "utils/sceneparser.h"
#include "utils/shapefactory.h"
#include "map/Map.h"
#include "map/mapproperties.h"
#include "realtime/physics.h"
#include "realtime/rendering.h"
#include "realtime/gbuffer.h"
#include "enemies/enemymanager.h"
#include "particlesystem/particlesystem.h"
#include "ui/ui.h"

// Forward declarations
class AudioManager;
class QKeyEvent;
class QMouseEvent;

// Forward declarations for helper namespaces
namespace TextureLoader {
    void initializeTextures(Realtime* realtime);
}
namespace FogSystem {
    void updateFogColor(Realtime* realtime, float deltaTime);
}
namespace InputHandler {
    void handleKeyPress(Realtime* realtime, QKeyEvent* event);
    void handleKeyRelease(Realtime* realtime, QKeyEvent* event);
    void handleMousePress(Realtime* realtime, QMouseEvent* event);
    void handleMouseRelease(Realtime* realtime, QMouseEvent* event);
    void handleMouseMove(Realtime* realtime, QMouseEvent* event);
}

class Realtime : public QOpenGLWidget
{
    Q_OBJECT

public:
    Realtime(QWidget *parent = nullptr);
    void finish();
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    
    void setActiveMap(Map* map);
    void setFlyingMode(bool flying);
    void setMovementSpeedMultiplier(double multiplier);
    void setJumpHeightMultiplier(double multiplier);
    void setCameraHeightMultiplier(double multiplier);
    void setPlayerLightColor(float r, float g, float b);
    void setGravityMultiplier(double multiplier);
    void setOverheadLightIntensity(double intensity);
    void setMotionBlurEnabled(bool enabled);
    void setMotionBlurSamples(int samples);
    void setDepthVisualizationEnabled(bool enabled);
    void setGBufferVisualizationMode(int mode);
    void setFpsMode(bool enabled);
    void setFogEnabled(bool enabled);
    void setFlashlightEnabled(bool enabled);
    void setFogColor(float r, float g, float b);
    void setFogIntensity(float intensity);
    void teleportToOrigin();
    void setFilterMode(int mode);
    void setGrainOverlayEnabled(bool enabled);
    void setGrainOpacity(float opacity);
    void setPixelateEnabled(bool enabled);
    void setBloomEnabled(bool enabled);
    void setLUTChoice(int choice);
    void setParticlesEnabled(bool enabled);
    void setDirtParticlesEnabled(bool enabled);
    void setFogWispsEnabled(bool enabled);
    void setDirtSpawnRate(float rate);
    void setFogWispSpawnInterval(float interval);
    void setMaxParticles(int maxParticles);
    int getLUTChoice() const { return m_lutChoice; }
    
        float getFlashlightCharge() const { return m_flashlightCharge; }
        bool isFlashlightEnabled() const { return m_flashlightEnabled; }
        float getFlashlightPenaltyTimer() const { return m_flashlightPenaltyTimer; }
        
        bool hasFieldCompletionCube() const { return m_fieldPenaltyTimer > 0.0f || m_fieldPenaltyValue >= 1.0f; }
        bool hasMountainCompletionCube() const { return m_mountainPenaltyTimer > 0.0f || m_mountainPenaltyValue >= 1.0f; }
        bool hasForestCompletionCube() const { return m_forestPenaltyTimer > 0.0f || m_forestPenaltyValue >= 1.0f; }
        
        //getters for penalty values
        float getFieldPenaltyValue() const { return m_fieldPenaltyValue; }
        float getMountainPenaltyValue() const { return m_mountainPenaltyValue; }
        float getForestPenaltyValue() const { return m_forestPenaltyValue; }
        
        //getter for player health
        float getPlayerHealth() const { return m_playerHealth; }
    
    void addPathWaypoint();
    void startPathPlayback();
    void stopPathPlayback();
    void clearPath();
    void setPathDuration(float durationSeconds);
    bool isPathPlaying() const { return m_cameraPath.isPlaying(); }
    int getPathWaypointCount() const { return m_cameraPath.getWaypointCount(); }
    
    EnemyManager& getEnemyManager() { return m_enemyManager; }
    const EnemyManager& getEnemyManager() const { return m_enemyManager; }
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }
    
    void damagePlayer(float damageAmount);

signals:
    void telemetryUpdate(float x, float y, float z, int chunkX, int chunkZ);
    void pathPlaybackFinished();
    void fpsModeToggled(bool enabled);
    void flashlightChargeChanged(float charge, bool inPenalty);
    void motionBlurToggled(bool enabled);

    // Friend declarations for helper functions
    friend void TextureLoader::initializeTextures(Realtime* realtime);
    friend void FogSystem::updateFogColor(Realtime* realtime, float deltaTime);
    friend void InputHandler::handleKeyPress(Realtime* realtime, QKeyEvent* event);
    friend void InputHandler::handleKeyRelease(Realtime* realtime, QKeyEvent* event);
    friend void InputHandler::handleMousePress(Realtime* realtime, QMouseEvent* event);
    friend void InputHandler::handleMouseRelease(Realtime* realtime, QMouseEvent* event);
    friend void InputHandler::handleMouseMove(Realtime* realtime, QMouseEvent* event);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    
    //members accessible by helper functions (InputHandler, TextureLoader, FogSystem, etc.)
    bool m_fpsMode;
    bool m_pKeyPressed;
    bool m_flashlightFKeyPressed;
    bool m_flashlightEnabled;
    float m_flashlightCharge;
    float m_flashlightPenaltyTimer;
    bool m_useNormalMapping;
    bool m_useBumpMapping;
    float m_bumpStrength;
    bool m_mouseDown;
    glm::vec2 m_prev_mouse_pos;
    bool m_ignoreNextMouseMove;
    glm::vec2 m_pendingRotation;
    CameraPath m_cameraPath;
    std::unordered_map<Qt::Key, bool> m_keyMap;
    
    //texture members (accessible by TextureLoader)
    GLuint m_colorTexture;
    GLuint m_sandTexture;
    GLuint m_normalMapTexture;
    GLuint m_bumpMapTexture;
    GLuint m_woodColorTexture;
    GLuint m_woodBumpTexture;
    GLuint m_woodNormalTexture;
    
    //fog members (accessible by FogSystem)
    bool m_fogEnabled;
    glm::vec3 m_fogColor;
    glm::vec3 m_targetFogColor;
    float m_fogIntensity;
    float m_fogTransitionSpeed;
    
    //map and camera (accessible by FogSystem)
    Map* m_activeMap;
    Camera m_camera;

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void addLightsToShader(const std::vector<SceneLightData> &lights);
    void updateFlashlightPosition();
    void updateFlashlightCharge(float deltaTime);
    void updateCompletionCubePenalties(float deltaTime);
    void renderPostProcessing();

    int m_timer;
    QElapsedTimer m_elapsedTimer;
    
    float m_rotationSmoothingFactor;

    double m_devicePixelRatio;

    ShapeFactory m_shapeManager;
    std::vector<RenderShapeData> m_shapes;
    std::vector<SceneLightData> m_lights;
    SceneGlobalData m_globalData;
    EnemyManager m_enemyManager;
    void renderMapBlocks();
    
    void updateTelemetry();
    
    glm::vec3 m_playerLightColor;
    bool m_flyingMode;
    glm::vec3 m_velocity;
    bool m_onGround;
    double m_movementSpeedMultiplier;
    double m_jumpHeightMultiplier;
    double m_cameraHeightMultiplier;
    double m_gravityMultiplier;
    double m_overheadLightIntensity;
    float m_baseFOV;
    float m_currentFOV;
    bool m_motionBlurEnabled;
    int m_motionBlurSamples;
    bool m_motionBlurAutoEnabled;
    bool m_depthVisualizationEnabled;
    int m_gbufferVisualizationMode;
    int m_fpsCenterTimer;
    
    float m_fieldPenaltyValue = 0.0f;
    float m_mountainPenaltyValue = 0.0f;
    float m_forestPenaltyValue = 0.0f;
    
    float m_fieldPenaltyTimer = 0.0f;
    float m_mountainPenaltyTimer = 0.0f;
    float m_forestPenaltyTimer = 0.0f;
    
    static constexpr float PENALTY_INCREASE_DURATION = 2.0f; // 2 seconds to reach max
    
    glm::vec3 getBiomeFogColor(BiomeType biome) const; // Get target fog color for biome
    void updatePhysics(float deltaTime);
    void updatePlayerHealth(float deltaTime);
    void handleDeath(); // Handle player death and ghost respawn
    
    float m_flashlightFlickerIntensity; // Current flicker intensity (0.0 to 1.0)
    float m_flashlightFlickerTime; // Time accumulator for flicker animation
    
    struct Flashlight {
        glm::vec3 position;
        glm::vec3 direction;
        float coneAngle;
        glm::vec3 color;
    };
    Flashlight m_flashlight;
    
    GLuint m_postShaderProgram;
    GLuint m_postQuadVAO;
    GLuint m_postQuadVBO;
    
    //post-processing
    GLuint m_filterShaderProgram;
    GLuint m_filterQuadVAO;
    GLuint m_filterQuadVBO;
    GLuint m_filterFBO;
    GLuint m_filterTexture;
    GLuint m_filterLUTTexture;
    float m_filterTime;
    int m_filterMode;
    bool m_grainOverlayEnabled;
    float m_grainOpacity;
    bool m_pixelateEnabled;
    bool m_bloomEnabled;
    int m_lutChoice;
    float m_lutSize;
    
    //for bloom pipeline
    GLuint m_bloomShaderProgram;
    GLuint m_blurShaderProgram;
    GLuint m_bloomExtractFBO;
    GLuint m_bloomExtractTexture;
    GLuint m_bloomBlurFBO[2];
    GLuint m_bloomBlurTexture[2];
    bool m_bloomInitialized;
    
    // Particle system
    ParticleSystem m_particleSystem;
    
    // UI system
    UI m_ui;
    
    // Sound effects (using miniaudio)
    class AudioManager* m_audioManager;
    
    // Random ambient sound system
    std::vector<QString> m_randomAmbientSounds;
    float m_randomAmbientTimer;
    float m_nextRandomAmbientInterval;
    
    // Footstep sound system
    std::vector<QString> m_footstepSounds;
    float m_walkingTime;
    const float FOOTSTEP_INTERVAL = 0.5f; // Play footstep every 0.5 seconds
    int m_footstepCount;
    
    //ghost respawn system - queue of camera path points (max 20)
    struct GhostWaypoint {
        glm::vec3 position;
        glm::vec3 lookDirection;
        GhostWaypoint(const glm::vec3& pos, const glm::vec3& look) 
            : position(pos), lookDirection(look) {}
    };
    std::deque<GhostWaypoint> m_ghostPathQueue;
    static constexpr int MAX_GHOST_PATH_CAPACITY = 10;
    bool m_isDead; // Track if player is dead
    int m_originalLUTChoice; // Store original LUT choice before death sequence
    
    // Player health system (noise opacity = health)
    float m_playerHealth;
    float m_healthRecoveryRate;
    float m_timeSinceLastDamage;
    float m_healingDelay;
    
    void initializeFilterSystem();
    void initializeBloom();
    void renderBloom();
    void renderPostFilters();
    void renderPostProcessingToTexture();
    void loadLUT(int choice);
    bool isMoving();
    
    friend class Physics;
    friend class Rendering;
    friend class GBuffer;

    GLuint m_shader;
    GLuint m_vbo;
    GLuint m_vao;
    GLuint m_shaderProgram;
    GLint m_projLoc;
    GLint m_modelLoc;
    GLint m_viewLoc;
    GLint m_cameraPosLoc;
    GLint m_numLightsLoc;
    GLint mat_cAmbientLoc;
    GLint mat_cDiffuseLoc;
    GLint mat_cSpecularLoc;
    GLint mat_shinyLoc;
    GLint m_k_aLoc;
    GLint m_k_dLoc;
    GLint m_k_sLoc;
    GLint m_shininessLoc;

    //block shader for bump mapping
    GLuint m_blockShaderProgram;
    GLint m_blockModelLoc;
    GLint m_blockProjLoc;
    GLint m_blockViewLoc;
    GLint m_blockCameraPosLoc;
    GLint m_blockNumLightsLoc;

    //block VAO/VBO
    GLuint m_blockVAO;
    GLuint m_blockVBO;
    int m_blockVertexCount;

    GLuint m_treeVAO;
    GLuint m_treeVBO;
    int m_treeVertexCount;
    
    GLuint m_completionCubeVAO;
    GLuint m_completionCubeVBO;
    int m_completionCubeVertexCount;

    void addLightsToBlockShader(const std::vector<SceneLightData> &lights);
    
    bool isCompletionCubeWithinOneBlock(const glm::vec3& cameraPos);

};
