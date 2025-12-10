// Silence deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include "ui.h"
#include "../realtime.h"
#include "../utils/shaderloader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>

UI::UI()
    : m_realtime(nullptr)
    , m_uiShaderProgram(0)
    , m_quadVAO(0)
    , m_quadVBO(0)
    , m_screenWidth(0)
    , m_screenHeight(0)
    , m_initialized(false)
{
}

UI::~UI() {
    cleanup();
}

void UI::initialize(Realtime* realtime) {
    if (m_initialized) {
        return;
    }
    
    m_realtime = realtime;
    
    initializeShaders();
    initializeBuffers();
    
    m_initialized = true;
}

void UI::cleanup() {
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }
    
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }
    
    if (m_uiShaderProgram != 0) {
        glDeleteProgram(m_uiShaderProgram);
        m_uiShaderProgram = 0;
    }
    
    m_initialized = false;
}

void UI::initializeShaders() {
    try {
        m_uiShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/ui.vert",
            ":/resources/shaders/ui.frag"
        );
    } catch (const std::runtime_error &e) {
        std::cerr << "Failed to load UI shaders: " << e.what() << std::endl;
        m_uiShaderProgram = 0;
    }
}

void UI::initializeBuffers() {
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);
}

void UI::resize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
}

void UI::render() {
    if (!m_initialized || m_uiShaderProgram == 0 || m_realtime == nullptr) {
        return;
    }
    
    GLboolean depthTestWasEnabled;
    GLboolean blendWasEnabled;
    GLint currentViewport[4];
    GLint currentFBO;
    GLboolean scissorEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestWasEnabled);
    glGetBooleanv(GL_BLEND, &blendWasEnabled);
    glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled);
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    
    if (scissorEnabled) glDisable(GL_SCISSOR_TEST);
    
    if (m_screenWidth > 0 && m_screenHeight > 0) {
        glViewport(0, 0, m_screenWidth, m_screenHeight);
    }
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    
    glUseProgram(m_uiShaderProgram);
    glBindVertexArray(m_quadVAO);
    
    //bread and butter here lol
    renderChargeBar();
    renderCompletionCubeIndicators();
    
    glFlush();
    
    glBindVertexArray(0);
    glUseProgram(0);
    glDepthMask(GL_TRUE);
    if (blendWasEnabled) glEnable(GL_BLEND);
    if (depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
    if (scissorEnabled) glEnable(GL_SCISSOR_TEST);
    glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);
    
    //should already be correct, but just in case
    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
}

void UI::renderChargeBar() {
    if (m_uiShaderProgram == 0) {
        return;
    }
    
    float charge = glm::clamp(m_realtime->getFlashlightCharge(), 0.0f, 100.0f);
    bool flashlightOn = m_realtime->isFlashlightEnabled();
    

    float opacity = flashlightOn ? 0.7f : 0.4f;
    
    float barWidth = 0.028f;
    float barHeight = 0.135f;
    float barX = 0.75f;
    float barBottom = -0.85f;
    
    glm::vec4 bgColor(0.2f, 0.2f, 0.2f, 0.4f * opacity);
    
    glm::vec4 chargeColor;
    if (charge > 50.0f) {
        float t = (charge - 50.0f) / 50.0f;
        chargeColor = glm::mix(glm::vec4(1.0f, 1.0f, 0.0f, opacity), glm::vec4(0.0f, 1.0f, 0.0f, opacity), t);
    } else if (charge > 25.0f) {
        float t = (charge - 25.0f) / 25.0f;
        chargeColor = glm::mix(glm::vec4(1.0f, 0.5f, 0.0f, opacity), glm::vec4(1.0f, 1.0f, 0.0f, opacity), t);
    } else {
        float t = charge / 25.0f;
        chargeColor = glm::mix(glm::vec4(1.0f, 0.0f, 0.0f, opacity), glm::vec4(1.0f, 0.5f, 0.0f, opacity), t);
    }
    
    float chargeHeight = barHeight * (charge / 100.0f);
    
    glm::mat4 proj = glm::mat4(1.0f);
    
    GLint projLoc = glGetUniformLocation(m_uiShaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(m_uiShaderProgram, "color");
    GLint positionLoc = glGetUniformLocation(m_uiShaderProgram, "position");
    GLint sizeLoc = glGetUniformLocation(m_uiShaderProgram, "size");
    
    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);
    }
    
    if (positionLoc != -1 && sizeLoc != -1 && colorLoc != -1) {
        float bgBarY = barBottom + barHeight;
        glUniform2f(positionLoc, barX, bgBarY);
        glUniform2f(sizeLoc, barWidth, barHeight);
        glUniform4fv(colorLoc, 1, &bgColor[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    //lowk just added all of these catches bc the bottom kept moving up
    if (chargeHeight > 0.001f && positionLoc != -1 && sizeLoc != -1 && colorLoc != -1) {
        float chargeBarY = barBottom + chargeHeight;
        glUniform2f(positionLoc, barX, chargeBarY);
        glUniform2f(sizeLoc, barWidth, chargeHeight);
        glUniform4fv(colorLoc, 1, &chargeColor[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
}

void UI::renderCompletionCubeIndicators() {
    if (m_uiShaderProgram == 0 || m_realtime == nullptr) {
        return;
    }
    
    bool hasField = m_realtime->hasFieldCompletionCube();
    bool hasMountain = m_realtime->hasMountainCompletionCube();
    bool hasForest = m_realtime->hasForestCompletionCube();
    
    glm::vec3 fieldColor(1.0f, 0.0f, 0.0f);
    glm::vec3 mountainColor(0.0f, 0.0f, 1.0f);
    glm::vec3 forestColor(0.0f, 1.0f, 0.0f);
    
    float squareSize = 0.03f;
    float spacing = 0.070f;
    float startX = 0.85f;
    float yPos = -0.82f;
    
    //opacity: low (0.15) when not collected, higher (0.6) when collected
    float uncollectedOpacity = 0.15f;
    float collectedOpacity = 0.6f;
    
    GLint projLoc = glGetUniformLocation(m_uiShaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(m_uiShaderProgram, "color");
    GLint positionLoc = glGetUniformLocation(m_uiShaderProgram, "position");
    GLint sizeLoc = glGetUniformLocation(m_uiShaderProgram, "size");
    
    if (projLoc != -1) {
        glm::mat4 proj = glm::mat4(1.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);
    }
    
    if (positionLoc != -1 && sizeLoc != -1 && colorLoc != -1) {
        float opacity = hasField ? collectedOpacity : uncollectedOpacity;
        glm::vec4 color(fieldColor.x, fieldColor.y, fieldColor.z, opacity);
        glUniform2f(positionLoc, startX, yPos);
        glUniform2f(sizeLoc, squareSize, squareSize);
        glUniform4fv(colorLoc, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        opacity = hasForest ? collectedOpacity : uncollectedOpacity;
        color = glm::vec4(forestColor.x, forestColor.y, forestColor.z, opacity);
        glUniform2f(positionLoc, startX, yPos + spacing);
        glUniform2f(sizeLoc, squareSize, squareSize);
        glUniform4fv(colorLoc, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        opacity = hasMountain ? collectedOpacity : uncollectedOpacity;
        color = glm::vec4(mountainColor.x, mountainColor.y, mountainColor.z, opacity);
        glUniform2f(positionLoc, startX, yPos + spacing * 2.0f);
        glUniform2f(sizeLoc, squareSize, squareSize);
        glUniform4fv(colorLoc, 1, &color[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

