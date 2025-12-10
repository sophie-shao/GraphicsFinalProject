// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include "textures.h"
#include "../realtime.h"
#include <iostream>
#include <cerrno>

namespace TextureLoader {
    
    GLuint loadTexture(const QStringList& paths, const unsigned char* defaultData, 
                       int defaultWidth, int defaultHeight) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        QImage image;
        bool loaded = false;
        
        for (const QString& path : paths) {
            image = QImage(path);
            if (!image.isNull()) {
                loaded = true;
                break;
            }
        }
        
        if (!loaded && defaultData != nullptr) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, defaultWidth, defaultHeight, 
                        0, GL_RGBA, GL_UNSIGNED_BYTE, defaultData);
        } else if (loaded) {
            image = image.convertToFormat(QImage::Format_RGBA8888);
            //resize to 256x256 for performance
            if (image.width() != 256 || image.height() != 256) {
                image = image.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
                        0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
            std::cout << "Loaded texture: " << image.width() << "x" << image.height() << std::endl;
        } else {
            std::cerr << "Failed to load texture from all paths" << std::endl;
            unsigned char whiteTex[] = {255, 255, 255, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteTex);
        }
        
        setStandardTextureParams(texture);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return texture;
    }
    
    GLuint loadSolidColorTexture(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        unsigned char color[] = {r, g, b, a};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
        
        setStandardTextureParams(texture);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return texture;
    }
    
    void setStandardTextureParams(GLuint texture) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void initializeTextures(Realtime* realtime) {
        //loading for all textures

        QStringList dirtPaths = {
            ":/resources/textures/dirt.png",
            "scenefiles/maps/dirt.png",
            "src/tomerge/dirt.png"
        };
        unsigned char whiteTex[] = {255, 255, 255, 255};
        realtime->m_colorTexture = loadTexture(dirtPaths, whiteTex);
        
        QStringList sandPaths = {
            ":/resources/textures/sand.jpg",
            "resources/textures/sand.jpg",
            QCoreApplication::applicationDirPath() + "/resources/textures/sand.jpg",
            QCoreApplication::applicationDirPath() + "/../resources/textures/sand.jpg"
        };
        unsigned char sandColor[] = {255, 255, 0, 255};
        realtime->m_sandTexture = loadTexture(sandPaths, sandColor);
        
        QStringList normalPaths = {
            ":/resources/textures/wood_normal.png",
            "scenefiles/maps/dirt.png"
        };
        unsigned char flatNormal[] = {128, 128, 255, 255};
        realtime->m_normalMapTexture = loadTexture(normalPaths, flatNormal);
        
        QStringList bumpPaths = {
            ":/resources/textures/wood_bump.png",
            "scenefiles/maps/dirt.png"
        };
        unsigned char flatBump[] = {128, 128, 128, 255};
        realtime->m_bumpMapTexture = loadTexture(bumpPaths, flatBump);
        
        realtime->m_woodColorTexture = loadSolidColorTexture(139, 90, 43);
        
        QStringList woodBumpPaths = {":/resources/textures/wood_bump.png"};
        realtime->m_woodBumpTexture = loadTexture(woodBumpPaths, flatBump);
        
        QStringList woodNormalPaths = {":/resources/textures/wood_normal.png"};
        realtime->m_woodNormalTexture = loadTexture(woodNormalPaths, flatNormal);
    }
}

