#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

class Realtime;

class GBuffer {
public:
    static void initialize(Realtime* realtime, int width, int height);
    static void resize(Realtime* realtime, int width, int height);
    static void cleanup(Realtime* realtime);
    
    static void beginGeometryPass(Realtime* realtime);
    static void endGeometryPass(Realtime* realtime);
    static void beginScenePass(Realtime* realtime);
    static void endScenePass(Realtime* realtime);



    static void renderMotionBlur(Realtime* realtime, bool renderToTexture = false);
    static void renderDepthVisualization(Realtime* realtime, float nearPlane, float farPlane);
    static void renderGBufferVisualization(Realtime* realtime, int mode);
    
    static GLuint m_gbufferFBO;
    static GLuint m_positionTexture;
    static GLuint m_normalTexture;
    static GLuint m_albedoTexture;
    static GLuint m_velocityTexture;
    static GLuint m_depthTexture;
    
    static GLuint m_sceneFBO;
    static GLuint m_sceneTexture;
    static GLuint m_sceneDepthTexture;
    
    static GLuint m_motionBlurFBO;
    static GLuint m_motionBlurTexture;
    
    static GLuint m_gbufferShaderProgram;
    static GLuint m_motionBlurShaderProgram;
    static GLuint m_depthVizShaderProgram;
    static GLuint m_gbufferVizShaderProgram;
    
    //cached uniform locations for GBuffer shader (performance optimization)
    static GLint m_gbufferProjLoc;
    static GLint m_gbufferViewLoc;
    static GLint m_gbufferPrevViewProjLoc;
    static GLint m_gbufferUseColorTextureLoc;
    static GLint m_gbufferModelLoc;
    static GLint m_gbufferDiffuseLoc;
    static GLint m_gbufferColorTextureLoc;
    
    static GLuint m_quadVAO;
    static GLuint m_quadVBO;
    static GLuint m_quadEBO;
    
    static glm::mat4 m_prevViewProj;
    static bool m_initialized;
    
private:
    static void makeFBOs(Realtime* realtime, int width = -1, int height = -1);
};

