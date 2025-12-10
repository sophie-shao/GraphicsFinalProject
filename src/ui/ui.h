#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>

class Realtime;

class UI {
public:
    UI();
    ~UI();
    
    void initialize(Realtime* realtime);
    void cleanup();
    void render();
    
    void resize(int width, int height);

private:
    void initializeShaders();
    void initializeBuffers();
    void renderChargeBar();
    void renderCompletionCubeIndicators();
    
    Realtime* m_realtime;
    
    GLuint m_uiShaderProgram;

    GLuint m_quadVAO;
    GLuint m_quadVBO;
    
    int m_screenWidth;
    int m_screenHeight;
    
    bool m_initialized;
};



