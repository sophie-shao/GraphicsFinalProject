#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include "shapes/Shape.h"
#include "utils/camera.h"
#include "utils/sceneparser.h"
#include "utils/shapefactory.h"
#include "map/Map.h"
#include "map/mapproperties.h"
#include "realtime/physics.h"
#include "realtime/rendering.h"

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

signals:
    void telemetryUpdate(float x, float y, float z, int chunkX, int chunkZ);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void addLightsToShader(const std::vector<SceneLightData> &lights);

    int m_timer;
    QElapsedTimer m_elapsedTimer;

    bool m_mouseDown = false;
    glm::vec2 m_prev_mouse_pos;
    std::unordered_map<Qt::Key, bool> m_keyMap;

    double m_devicePixelRatio;

    ShapeFactory m_shapeManager;
    std::vector<RenderShapeData> m_shapes;
    std::vector<SceneLightData> m_lights;
    SceneGlobalData m_globalData;
    Camera m_camera;

    Map* m_activeMap;
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
    void updatePhysics(float deltaTime);

    friend class Physics;
    friend class Rendering;

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

};
