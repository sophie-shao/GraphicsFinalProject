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
#include "camera/camera.h"
#include "utils/sceneparser.h"
#include "shapes/Cone.h"
#include "shapes/Sphere.h"
#include "shapes/Cylinder.h"
#include "shapes/Cube.h"

enum ParticleType {
    PARTICLE_DIRT,
    PARTICLE_FOG_WISP,
    PARTICLE_BREATH
};

struct ParticleInstance {
    glm::vec3 pos;
    float size;
    glm::vec4 color;
    int type;
};


struct Particle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    glm::vec4 Color;
    float StartLife;
    float Life;
    float Size;
    ParticleType pType;
    glm::vec3 DriftDir;

    Particle()
        : Position(0.0f), Velocity(0.0f),
        Color(1.0f), Life(0.0f) {}
};


class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;

    RenderData m_renderData;
    bool m_glmInit = false;



    std::unordered_map<PrimitiveType, std::vector<float>> m_shapes_map;
    std::unordered_map<PrimitiveType, GLuint> m_vaos;
    std::unordered_map<PrimitiveType, GLuint> m_vbos;
    Camera m_cam;
    GLuint m_shader; // id of shader

    glm::mat4 m_view;
    glm::mat4 m_invView;
    glm::mat4 m_proj;

    int m_param1;
    int m_param2;

    // imma need smth for the lights baby

    float m_ka;
    float m_kd;
    float m_ks;

    void rebuildGeometry();
    void rebuildGeometryNumShapes(int numShapes);

    void renderSceneGeometry();

    // for proj 6
    int m_screen_width = 0;
    int m_screen_height = 0;
    int m_fbo_width = 0;
    int m_fbo_height = 0;

    float m_ppTime = 0.0f;


    // // Qt default framebuffer (the screen)
    GLuint m_defaultFBO = 2;

    // // Post-process shader (texture shader)
    GLuint m_texture_shader = 0;

    // // Fullscreen quad
    GLuint m_fullscreen_vbo = 0;
    GLuint m_fullscreen_vao = 0;

    // // FBO attachments
    GLuint m_fbo = 0;
    GLuint m_fbo_col_texture = 0;
    GLuint m_fbo_renderbuffer = 0;
    GLuint m_fbo_depth_texture = 0;
    GLuint m_fbo_lut_texture = 0;
    GLuint m_lutSize = 0;

    // // Toggle for enabling/disabling post-process
    // bool m_enablePostprocess = true;

    // void initPostProcessing();
    void makeFBO();
    void paintTexture(GLuint col_texture, GLuint depth_texture, GLuint lut_texture, GLuint bloom_texture, bool postProcess);
    void initializePostProcessingPipeline();

    void chooseLUT();

    int m_lut_curr = 0;


    // for particles
    std::vector<Particle> m_particles;
    int m_maxParticles = 200;

    // VAO/VBO for the particle quad
    GLuint m_particleVAO = 0;
    GLuint m_particleVBO = 0;

    GLuint m_particleInstanceVBO; // for optimization
    // std::vector<ParticleInstance> m_aliveInstances;
    std::vector<ParticleInstance> m_aliveFogInstances;
    std::vector<ParticleInstance> m_aliveDirtInstances;


    // The particle shader
    GLuint m_particleShader = 0;

    // Texture for particles
    GLuint m_dirtParticleTexture = 0;
    GLuint m_wispParticleTexture = 0;



    void initializeParticles();
    void updateParticles(float dt);
    void drawParticles();
    glm::vec3 getCameraFeetPosition();
    float m_particleSpawnTimer = 0.0f;
    float m_particleSpawnInterval = 0.10f; // one particle every 0.10s = 10 per second
    int m_nextParticle = 0; // ring buffer index
    void spawnDirtParticles(float dt);
    void spawnSingleDirtParticle();
    bool isMoving();

    // fog wisps
    float m_wispSpawnTimer = 0.0f;
    float m_wispSpawnInterval = 1.5f; // one fog wisp every ~1.5s

    void spawnFogWisp(float dt);

    // bloom
    GLuint m_bright_texture;
    void makeBloomFBOs();
    GLuint pingpongFBOs[2];
    GLuint pingpongBuffers[2];

    void bouncePingPong();
    GLuint m_blur_shader;
};
