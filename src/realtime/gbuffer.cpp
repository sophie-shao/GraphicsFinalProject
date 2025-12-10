#include "realtime/gbuffer.h"
#include "realtime.h"
#include "utils/shaderloader.h"
#include <iostream>

GLuint GBuffer::m_gbufferFBO = 0;
GLuint GBuffer::m_positionTexture = 0;
GLuint GBuffer::m_normalTexture = 0;
GLuint GBuffer::m_albedoTexture = 0;
GLuint GBuffer::m_velocityTexture = 0;
GLuint GBuffer::m_depthTexture = 0;

GLuint GBuffer::m_sceneFBO = 0;
GLuint GBuffer::m_sceneTexture = 0;
GLuint GBuffer::m_sceneDepthTexture = 0;

GLuint GBuffer::m_motionBlurFBO = 0;
GLuint GBuffer::m_motionBlurTexture = 0;

GLuint GBuffer::m_gbufferShaderProgram = 0;
GLuint GBuffer::m_motionBlurShaderProgram = 0;
GLuint GBuffer::m_depthVizShaderProgram = 0;
GLuint GBuffer::m_gbufferVizShaderProgram = 0;

//cached uniform locations
GLint GBuffer::m_gbufferProjLoc = -1;
GLint GBuffer::m_gbufferViewLoc = -1;
GLint GBuffer::m_gbufferPrevViewProjLoc = -1;
GLint GBuffer::m_gbufferUseColorTextureLoc = -1;
GLint GBuffer::m_gbufferModelLoc = -1;
GLint GBuffer::m_gbufferDiffuseLoc = -1;
GLint GBuffer::m_gbufferColorTextureLoc = -1;

GLuint GBuffer::m_quadVAO = 0;
GLuint GBuffer::m_quadVBO = 0;
GLuint GBuffer::m_quadEBO = 0;

glm::mat4 GBuffer::m_prevViewProj = glm::mat4(1.0f);
bool GBuffer::m_initialized = false;


void GBuffer::renderGBufferVisualization(Realtime* realtime, int mode) {
    if (!m_initialized || m_gbufferVizShaderProgram == 0) {
        return;
    }
    
    GLuint textureToUse = 0;
    if (mode == 1) {
        textureToUse = m_positionTexture;
    } else if (mode == 2) {
        textureToUse = m_normalTexture;
    } else if (mode == 3) {
        textureToUse = m_albedoTexture;
    } else if (mode == 4) {
        textureToUse = m_velocityTexture;
    } else {
        return;
    }
    
    if (textureToUse == 0) {
        return;
    }
    
    GLint currentFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    
    GLuint defaultFBO = realtime->defaultFramebufferObject();
    if (currentFBO != defaultFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
    }
    
    int w = realtime->size().width() * realtime->m_devicePixelRatio;
    int h = realtime->size().height() * realtime->m_devicePixelRatio;
    
    if (w <= 0 || h <= 0) {
        return;
    }
    
    glViewport(0, 0, w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    glUseProgram(m_gbufferVizShaderProgram);
    
    GLint texLoc = glGetUniformLocation(m_gbufferVizShaderProgram, "gbufferTexture");
    GLint modeLoc = glGetUniformLocation(m_gbufferVizShaderProgram, "visualizationMode");
    
    if (texLoc != -1 && modeLoc != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureToUse);
        
        //for float textures, ensure texture parameters are set
        if (mode == 1 || mode == 2) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        
        glUniform1i(texLoc, 0);
        glUniform1i(modeLoc, mode);
        
        glBindVertexArray(m_quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

void GBuffer::initialize(Realtime* realtime, int width, int height) {
    if (m_initialized) {
        return;
    }


    //catches just in case shader programs dont bind right when integrating
    if (m_gbufferShaderProgram == 0) {
        try {
            m_gbufferShaderProgram = ShaderLoader::createShaderProgram(
                ":/resources/shaders/gbuffer.vert",
                ":/resources/shaders/gbuffer.frag"
            );
        } catch (const std::runtime_error &e) {
        }
    }
    
    if (m_motionBlurShaderProgram == 0) {
        try {
            m_motionBlurShaderProgram = ShaderLoader::createShaderProgram(
                ":/resources/shaders/motionblur.vert",
                ":/resources/shaders/motionblur.frag"
            );
        } catch (const std::runtime_error &e) {
        }
    }
    
    if (m_depthVizShaderProgram == 0) {
        try {
            m_depthVizShaderProgram = ShaderLoader::createShaderProgram(
                ":/resources/shaders/depthviz.vert",
                ":/resources/shaders/depthviz.frag"
            );
        } catch (const std::runtime_error &e) {
        }
    }
    
    if (m_gbufferVizShaderProgram == 0) {
        try {
            m_gbufferVizShaderProgram = ShaderLoader::createShaderProgram(
                ":/resources/shaders/gbufferviz.vert",
                ":/resources/shaders/gbufferviz.frag"
            );
        } catch (const std::runtime_error &e) {
        }
    }
    
    //cache uniform locations for GBuffer shader (performance optimization)
    if (m_gbufferShaderProgram != 0) {
        m_gbufferProjLoc = glGetUniformLocation(m_gbufferShaderProgram, "projMatrix");
        m_gbufferViewLoc = glGetUniformLocation(m_gbufferShaderProgram, "viewMatrix");
        m_gbufferPrevViewProjLoc = glGetUniformLocation(m_gbufferShaderProgram, "prevViewProjMatrix");
        m_gbufferUseColorTextureLoc = glGetUniformLocation(m_gbufferShaderProgram, "useColorTexture");
        m_gbufferModelLoc = glGetUniformLocation(m_gbufferShaderProgram, "modelMatrix");
        m_gbufferDiffuseLoc = glGetUniformLocation(m_gbufferShaderProgram, "material.cDiffuse");
        m_gbufferColorTextureLoc = glGetUniformLocation(m_gbufferShaderProgram, "colorTexture");
    }
    
    if (m_quadVAO == 0) {
        float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };
        unsigned int quadIndices[] = {
            0, 1, 2,
            2, 3, 0
        };
        
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glGenBuffers(1, &m_quadEBO);
        
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }

    //lowk bad association but this isnt cs15
    makeFBOs(realtime);
    
    m_initialized = true;
}

void GBuffer::resize(Realtime* realtime, int width, int height) {
    if (!m_initialized) {
        initialize(realtime, width, height);
        return;
    }

    makeFBOs(realtime, width, height);
}

void GBuffer::cleanup(Realtime* realtime) {
    //not sure if im gonna need stuff here
}

void GBuffer::beginGeometryPass(Realtime* realtime) {
    if (!m_initialized || m_gbufferFBO == 0) {
        return;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_gbufferFBO);
    
    GLenum attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);
    
    int w = realtime->size().width() * realtime->m_devicePixelRatio;
    int h = realtime->size().height() * realtime->m_devicePixelRatio;
    glViewport(0, 0, w, h);
    
    // Reset vertex attribute state that particles might have modified
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glVertexAttribDivisor(2, 0);
    glVertexAttribDivisor(3, 0);
    glVertexAttribDivisor(4, 0);
    glVertexAttribDivisor(5, 0);
    
    // Reset buffer and texture bindings
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    for (int i = 0; i < 8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    
    glUseProgram(0);
    glBindVertexArray(0);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void GBuffer::endGeometryPass(Realtime* realtime) {
    GLuint defaultFBO = realtime->defaultFramebufferObject();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


//Tutorials suggested to digive the renders in different passes
void GBuffer::beginScenePass(Realtime* realtime) {
    if (!m_initialized || m_sceneFBO == 0) {
        return;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
    
    GLenum attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);
    
    int w = realtime->size().width() * realtime->m_devicePixelRatio;
    int h = realtime->size().height() * realtime->m_devicePixelRatio;
    glViewport(0, 0, w, h);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void GBuffer::endScenePass(Realtime* realtime) {
    glBindFramebuffer(GL_FRAMEBUFFER, realtime->defaultFramebufferObject());
}

void GBuffer::renderMotionBlur(Realtime* realtime, bool renderToTexture) {
    if (!m_initialized || m_motionBlurShaderProgram == 0 || m_sceneTexture == 0) {
        return;
    }
    
    int w = realtime->size().width() * realtime->m_devicePixelRatio;
    int h = realtime->size().height() * realtime->m_devicePixelRatio;
    
    if (w <= 0 || h <= 0) {
        return;
    }
    
    if (renderToTexture) {
        // Render to texture for post-processing/filters
        if (m_motionBlurFBO == 0) {
            glGenFramebuffers(1, &m_motionBlurFBO);
            glGenTextures(1, &m_motionBlurTexture);
        }
        
        // Only recreate texture if size changed, not every frame
        glBindTexture(GL_TEXTURE_2D, m_motionBlurTexture);
        GLint existingWidth = 0, existingHeight = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &existingWidth);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &existingHeight);
        
        if (existingWidth != w || existingHeight != h) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_motionBlurFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_motionBlurTexture, 0);
        
        GLenum attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);
    } else {
        // Render directly to default framebuffer (original behavior)
        GLuint defaultFBO = realtime->defaultFramebufferObject();
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        glViewport(0, 0, w, h);
    }
    
    if (renderToTexture) {
        glViewport(0, 0, w, h);
    }
    
    glDisable(GL_BLEND);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glUseProgram(m_motionBlurShaderProgram);
    
    GLint gPosLoc = glGetUniformLocation(m_motionBlurShaderProgram, "gPosition");
    GLint gNormalLoc = glGetUniformLocation(m_motionBlurShaderProgram, "gNormal");
    GLint gAlbedoLoc = glGetUniformLocation(m_motionBlurShaderProgram, "gAlbedo");
    GLint gVelocityLoc = glGetUniformLocation(m_motionBlurShaderProgram, "gVelocity");
    GLint sceneTexLoc = glGetUniformLocation(m_motionBlurShaderProgram, "sceneTexture");
    GLint numSamplesLoc = glGetUniformLocation(m_motionBlurShaderProgram, "numSamples");
    
    if (gPosLoc != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_positionTexture);
        glUniform1i(gPosLoc, 0);
    }
    
    if (gNormalLoc != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_normalTexture);
        glUniform1i(gNormalLoc, 1);
    }
    
    if (gAlbedoLoc != -1) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
        glUniform1i(gAlbedoLoc, 2);
    }
    
    if (gVelocityLoc != -1) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_velocityTexture);
        glUniform1i(gVelocityLoc, 3);
    }
    
    if (sceneTexLoc != -1) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(sceneTexLoc, 4);
    }
    
    if (numSamplesLoc != -1) {
        glUniform1i(numSamplesLoc, realtime->m_motionBlurSamples);
    }
    
    glBindVertexArray(m_quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // If we rendered to texture, unbind the FBO so the texture can be safely read
    if (renderToTexture) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glUseProgram(0);
}

void GBuffer::renderDepthVisualization(Realtime* realtime, float nearPlane, float farPlane) {
    if (m_depthVizShaderProgram == 0) {
        try {
            m_depthVizShaderProgram = ShaderLoader::createShaderProgram(
                ":/resources/shaders/depthviz.vert",
                ":/resources/shaders/depthviz.frag"
            );
        } catch (const std::runtime_error &e) {
            return;
        }
    }
    
    if (m_quadVAO == 0) {
        float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };
        unsigned int quadIndices[] = {
            0, 1, 2,
            2, 3, 0
        };
        
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glGenBuffers(1, &m_quadEBO);
        
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }


    GLint currentFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    
    GLuint defaultFBO = realtime->defaultFramebufferObject();
    if (currentFBO != defaultFBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
    }
    
    int w = realtime->size().width() * realtime->m_devicePixelRatio;
    int h = realtime->size().height() * realtime->m_devicePixelRatio;
    
    if (w <= 0|| h <= 0) {
        return;
    }
    
    glViewport(0, 0, w, h);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    glUseProgram(m_depthVizShaderProgram);
    
    GLint depthTexLoc = glGetUniformLocation(m_depthVizShaderProgram, "depthTexture");
    GLint nearLoc = glGetUniformLocation(m_depthVizShaderProgram, "nearPlane");
    GLint farLoc = glGetUniformLocation(m_depthVizShaderProgram, "farPlane");
    
    // Use G-buffer depth texture (not scene depth) for visualization
    // Scene depth is for post-processing, G-buffer depth is what we want to visualize
    GLuint depthTexToUse = m_depthTexture;
    
    if (depthTexToUse == 0) {
        return;
    }
    
    if (depthTexLoc != -1 && nearLoc != -1 && farLoc != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthTexToUse);
        glUniform1i(depthTexLoc, 0);
        glUniform1f(nearLoc, nearPlane);
        glUniform1f(farLoc, farPlane);
        
        glBindVertexArray(m_quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

void GBuffer::makeFBOs(Realtime* realtime, int width, int height) {
    int w, h;
    if (width > 0 && height > 0) {
        w = width * realtime->m_devicePixelRatio;
        h = height * realtime->m_devicePixelRatio;
    } else {
        w = realtime->size().width() * realtime->m_devicePixelRatio;
        h = realtime->size().height() * realtime->m_devicePixelRatio;
    }
    
    if (w <= 0 || h <= 0) {
        return;
    }
    
    //delete existing FBOs and textures if needed
    if (m_gbufferFBO != 0) {
        glDeleteFramebuffers(1, &m_gbufferFBO);
        glDeleteTextures(1, &m_positionTexture);
        glDeleteTextures(1, &m_normalTexture);
        glDeleteTextures(1, &m_albedoTexture);
        glDeleteTextures(1, &m_velocityTexture);
        glDeleteTextures(1, &m_depthTexture);
    }
    
    if (m_sceneFBO != 0) {
        glDeleteFramebuffers(1, &m_sceneFBO);
        glDeleteTextures(1, &m_sceneTexture);
        glDeleteTextures(1, &m_sceneDepthTexture);
    }
    
    if (m_motionBlurFBO != 0) {
        glDeleteFramebuffers(1, &m_motionBlurFBO);
        glDeleteTextures(1, &m_motionBlurTexture);
    }
    
    //g-buffer FBO
    glGenFramebuffers(1, &m_gbufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gbufferFBO);
    
    //position texture
    glGenTextures(1, &m_positionTexture);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_positionTexture, 0);
    
    //normal texture
    glGenTextures(1, &m_normalTexture);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normalTexture, 0);
    
    //albedo texture
    glGenTextures(1, &m_albedoTexture);
    glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_albedoTexture, 0);
    
    //velocity texture
    glGenTextures(1, &m_velocityTexture);
    glBindTexture(GL_TEXTURE_2D, m_velocityTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, w, h, 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_velocityTexture, 0);
    
    //depth texture
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);


    GLenum attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: G-buffer FBO not complete! Status: " << status << std::endl;
    }
    
    glGenFramebuffers(1, &m_sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFBO);
    
    glGenTextures(1, &m_sceneTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);
    
    glGenTextures(1, &m_sceneDepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_sceneDepthTexture, 0);
    
    GLenum sceneAttachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, sceneAttachments);
    
    glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
    glBindFramebuffer(GL_FRAMEBUFFER, realtime->defaultFramebufferObject());
}
