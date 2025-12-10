#pragma once

#include <GL/glew.h>
#include <QImage>
#include <QCoreApplication>

// Forward declaration
class Realtime;

namespace TextureLoader {
    // Load a texture from file with fallback paths
    GLuint loadTexture(const QStringList& paths, const unsigned char* defaultData = nullptr, 
                       int defaultWidth = 1, int defaultHeight = 1);
    
    // Load a solid color texture
    GLuint loadSolidColorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
    
    // Set standard texture parameters (linear filtering, repeat wrap)
    void setStandardTextureParams(GLuint texture);
    
    // Initialize all textures for Realtime
    void initializeTextures(Realtime* realtime);
}

