#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <QImage>

class Camera;

class ParticleSystem {
public:
    enum ParticleType {
        PARTICLE_DIRT,
        PARTICLE_FOG_WISP,
        PARTICLE_BREATH,
        PARTICLE_DUST,
        PARTICLE_DUST_MOUNTAIN,
        PARTICLE_DUST_FOREST,
        PARTICLE_DUST_GRASSLAND,
        PARTICLE_LEAF
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
    
    ParticleSystem();
    ~ParticleSystem();
    
    void initialize();
    void cleanup();
    void update(float deltaTime, const Camera& camera, bool isMoving, float cameraHeightMultiplier, int currentBiome);
    void draw(const Camera& camera);
    
    // Settings
    void setEnabled(bool enabled);
    void setDirtParticlesEnabled(bool enabled);
    void setFogWispsEnabled(bool enabled);
    void setDirtSpawnRate(float rate);
    void setFogWispSpawnInterval(float interval);
    void setMaxParticles(int maxParticles);
    
    bool isEnabled() const { return m_particlesEnabled; }
    
private:
    void spawnSingleDirtParticle(const Camera& camera, float cameraHeightMultiplier);
    void spawnDirtParticles(float deltaTime, const Camera& camera, bool isMoving, float cameraHeightMultiplier);
    void spawnFogWisp(float deltaTime, const Camera& camera);
    void spawnDustParticle(const Camera& camera, float cameraHeightMultiplier, int biomeType);
    void spawnDustParticles(float deltaTime, const Camera& camera, float cameraHeightMultiplier, int biomeType);
    void spawnLeafParticle(const Camera& camera, float cameraHeightMultiplier);
    void spawnLeafParticles(float deltaTime, const Camera& camera, float cameraHeightMultiplier);
    glm::vec3 getCameraFeetPosition(const Camera& camera, float cameraHeightMultiplier) const;
    
    std::vector<Particle> m_particles;
    int m_maxParticles;
    int m_nextParticle;
    bool m_particlesEnabled;
    bool m_dirtParticlesEnabled;
    bool m_fogWispsEnabled;
    float m_dirtSpawnRate;
    float m_fogWispSpawnInterval;
    float m_particleSpawnTimer;
    float m_wispSpawnTimer;
    
    GLuint m_particleShader;
    GLuint m_particleVAO;
    GLuint m_particleVBO;
    GLuint m_particleInstanceVBO;
    std::vector<ParticleInstance> m_aliveFogInstances;
    std::vector<ParticleInstance> m_aliveDirtInstances;
    std::vector<ParticleInstance> m_aliveDustInstances;
    std::vector<ParticleInstance> m_aliveLeafInstances;
    GLuint m_dirtParticleTexture;
    GLuint m_wispParticleTexture;
    GLuint m_dustMountainTexture;
    GLuint m_dustForestTexture;
    GLuint m_dustGrasslandTexture;
    GLuint m_leafParticleTexture;
    float m_dustSpawnTimer;
    float m_leafSpawnTimer;
    float m_dustSpawnInterval;
    float m_leafSpawnInterval;
};

