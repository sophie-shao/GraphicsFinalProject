#include "particlesystem.h"
#include "utils/camera.h"
#include "utils/shaderloader.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

ParticleSystem::ParticleSystem()
    : m_maxParticles(200)
    , m_nextParticle(0)
    , m_particlesEnabled(true)
    , m_dirtParticlesEnabled(true)
    , m_fogWispsEnabled(true)
    , m_dirtSpawnRate(10.0f)
    , m_fogWispSpawnInterval(1.5f)
    , m_particleSpawnTimer(0.0f)
    , m_wispSpawnTimer(0.0f)
    , m_particleShader(0)
    , m_particleVAO(0)
    , m_particleVBO(0)
    , m_particleInstanceVBO(0)
    , m_dirtParticleTexture(0)
    , m_wispParticleTexture(0)
    , m_dustMountainTexture(0)
    , m_dustForestTexture(0)
    , m_dustGrasslandTexture(0)
    , m_leafParticleTexture(0)
    , m_dustSpawnTimer(0.0f)
    , m_leafSpawnTimer(0.0f)
    , m_dustSpawnInterval(0.1f)
    , m_leafSpawnInterval(0.4f)
{
}

ParticleSystem::~ParticleSystem() {
    cleanup();
}

void ParticleSystem::initialize() {
    m_particles.resize(m_maxParticles);
    
    // Generate instance-data VBO
    glGenBuffers(1, &m_particleInstanceVBO);
    
    float particle_quad[] = {
        -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_particleVAO);
    glGenBuffers(1, &m_particleVBO);
    
    glBindVertexArray(m_particleVAO);
    
    // Quad VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, pos));
    glVertexAttribDivisor(2, 1);
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, size));
    glVertexAttribDivisor(3, 1);
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, color));
    glVertexAttribDivisor(4, 1);
    
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_INT, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, type));
    glVertexAttribDivisor(5, 1);
    
    glBindVertexArray(0);
    
    // Load shader
    try {
        m_particleShader = ShaderLoader::createShaderProgram(
            ":/resources/shaders/particles.vert",
            ":/resources/shaders/particles.frag"
        );
        if (m_particleShader != 0) {
            glUseProgram(m_particleShader);
            glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);
            glUseProgram(0);
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "Error loading particle shader: " << e.what() << std::endl;
    }
    
    QImage dirtImg(":/resources/textures/particles/dirtparticle1.png");
    if (!dirtImg.isNull()) {
        dirtImg = dirtImg.convertToFormat(QImage::Format_RGBA8888);
        if (dirtImg.width() != 256 || dirtImg.height() != 256) {
            dirtImg = dirtImg.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        glGenTextures(1, &m_dirtParticleTexture);
        glBindTexture(GL_TEXTURE_2D, m_dirtParticleTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dirtImg.width(), dirtImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, dirtImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    QImage wispImg(":/resources/textures/particles/wisp.png");
    if (!wispImg.isNull()) {
        wispImg = wispImg.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
        if (wispImg.width() != 256 || wispImg.height() != 256) {
            wispImg = wispImg.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        glGenTextures(1, &m_wispParticleTexture);
        glBindTexture(GL_TEXTURE_2D, m_wispParticleTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wispImg.width(), wispImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, wispImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    QImage dustMountainImg(":/resources/textures/particles/dust_mountains.png");
    if (!dustMountainImg.isNull()) {
        dustMountainImg = dustMountainImg.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
        if (dustMountainImg.width() != 256 || dustMountainImg.height() != 256) {
            dustMountainImg = dustMountainImg.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        glGenTextures(1, &m_dustMountainTexture);
        glBindTexture(GL_TEXTURE_2D, m_dustMountainTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dustMountainImg.width(), dustMountainImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, dustMountainImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "Successfully loaded dust_mountains texture" << std::endl;
    } else {
        std::cerr << "Warning: dust_mountains.png not found" << std::endl;
    }
    
    QImage dustForestImg(":/resources/textures/particles/dust_forest.png");
    if (!dustForestImg.isNull()) {
        dustForestImg = dustForestImg.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
        if (dustForestImg.width() != 256 || dustForestImg.height() != 256) {
            dustForestImg = dustForestImg.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        glGenTextures(1, &m_dustForestTexture);
        glBindTexture(GL_TEXTURE_2D, m_dustForestTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dustForestImg.width(), dustForestImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, dustForestImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "Successfully loaded dust_forest texture" << std::endl;
    } else {
        std::cerr << "Warning: dust_forest.png not found" << std::endl;
    }
    
    QImage dustGrasslandImg(":/resources/textures/particles/dust_grasslands.png");
    if (!dustGrasslandImg.isNull()) {
        dustGrasslandImg = dustGrasslandImg.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
        if (dustGrasslandImg.width() != 256 || dustGrasslandImg.height() != 256) {
            dustGrasslandImg = dustGrasslandImg.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        glGenTextures(1, &m_dustGrasslandTexture);
        glBindTexture(GL_TEXTURE_2D, m_dustGrasslandTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dustGrasslandImg.width(), dustGrasslandImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, dustGrasslandImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "Successfully loaded dust_grasslands texture" << std::endl;
    } else {
        std::cerr << "Warning: dust_grasslands.png not found" << std::endl;
    }
    
    QImage leafImg(":/resources/textures/particles/dust_forest.png");
    if (!leafImg.isNull()) {
        leafImg = leafImg.convertToFormat(QImage::Format_RGBA8888);
        if (leafImg.width() != 256 || leafImg.height() != 256) {
            leafImg = leafImg.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        glGenTextures(1, &m_leafParticleTexture);
        glBindTexture(GL_TEXTURE_2D, m_leafParticleTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, leafImg.width(), leafImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, leafImg.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        std::cout << "Successfully loaded leaf texture" << std::endl;
    } else {
        std::cerr << "Warning: Leaf texture not found" << std::endl;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ParticleSystem::cleanup() {
    if (m_particleVAO != 0) {
        glDeleteVertexArrays(1, &m_particleVAO);
        m_particleVAO = 0;
    }
    if (m_particleVBO != 0) {
        glDeleteBuffers(1, &m_particleVBO);
        m_particleVBO = 0;
    }
    if (m_particleInstanceVBO != 0) {
        glDeleteBuffers(1, &m_particleInstanceVBO);
        m_particleInstanceVBO = 0;
    }
    if (m_particleShader != 0) {
        glDeleteProgram(m_particleShader);
        m_particleShader = 0;
    }
    if (m_dirtParticleTexture != 0) {
        glDeleteTextures(1, &m_dirtParticleTexture);
        m_dirtParticleTexture = 0;
    }
    if (m_wispParticleTexture != 0) {
        glDeleteTextures(1, &m_wispParticleTexture);
        m_wispParticleTexture = 0;
    }
    if (m_dustMountainTexture != 0) {
        glDeleteTextures(1, &m_dustMountainTexture);
        m_dustMountainTexture = 0;
    }
    if (m_dustForestTexture != 0) {
        glDeleteTextures(1, &m_dustForestTexture);
        m_dustForestTexture = 0;
    }
    if (m_dustGrasslandTexture != 0) {
        glDeleteTextures(1, &m_dustGrasslandTexture);
        m_dustGrasslandTexture = 0;
    }
    if (m_leafParticleTexture != 0) {
        glDeleteTextures(1, &m_leafParticleTexture);
        m_leafParticleTexture = 0;
    }
}

glm::vec3 ParticleSystem::getCameraFeetPosition(const Camera& camera, float cameraHeightMultiplier) const {
    glm::vec3 camPos = camera.getPosition();
    const float BASE_EYE_HEIGHT = 1.6f;
    float eyeHeight = BASE_EYE_HEIGHT * cameraHeightMultiplier;
    camPos.y -= eyeHeight;
    return camPos;
}

void ParticleSystem::spawnSingleDirtParticle(const Camera& camera, float cameraHeightMultiplier) {
    Particle &p = m_particles[m_nextParticle];
    
    glm::vec3 feet = getCameraFeetPosition(camera, cameraHeightMultiplier);
    
    float spread = 0.45f;
    float rX = ((rand() % 100) / 100.0f - 0.5f) * spread;
    float rZ = ((rand() % 100) / 100.0f - 0.5f) * spread;
    
    p.Position = feet + glm::vec3(rX, 0.0f, rZ);
    p.Velocity = glm::vec3(0.0f, 1.0f, 0.0f);
    p.Color = glm::vec4(0.4f, 0.3f, 0.2f, 1.0f);
    p.Life = 0.7f;
    p.StartLife = 0.7f;
    p.Size = 0.15f;
    p.pType = PARTICLE_DIRT;
    
    m_nextParticle = (m_nextParticle + 1) % m_maxParticles;
}

void ParticleSystem::spawnDirtParticles(float deltaTime, const Camera& camera, bool isMoving, float cameraHeightMultiplier) {
    if (!m_particlesEnabled || !m_dirtParticlesEnabled || !isMoving) {
        return;
    }
    
    float spawnInterval = 1.0f / m_dirtSpawnRate;
    m_particleSpawnTimer -= deltaTime;
    
    if (m_particleSpawnTimer <= 0.0f) {
        spawnSingleDirtParticle(camera, cameraHeightMultiplier);
        m_particleSpawnTimer = spawnInterval;
    }
}

void ParticleSystem::spawnFogWisp(float deltaTime, const Camera& camera) {
    if (!m_particlesEnabled || !m_fogWispsEnabled) {
        return;
    }
    
    m_wispSpawnTimer -= deltaTime;
    if (m_wispSpawnTimer > 0.0f) {
        return;
    }
    
    // Random spawn interval
    m_wispSpawnTimer = m_fogWispSpawnInterval + ((rand() % 100) / 100.0f) * 1.2f;
    
    Particle &p = m_particles[m_nextParticle];
    m_nextParticle = (m_nextParticle + 1) % m_maxParticles;
    
    p.Life = 8.0f + ((rand() % 100) / 100.0f) * 4.0f;
    p.StartLife = p.Life;
    p.pType = PARTICLE_FOG_WISP;
    
    glm::vec3 camPos = camera.getPosition();
    glm::vec3 forward = camera.getLook();
    
    float minDist = 25.0f;
    float maxDist = 35.0f;
    
    float baseAngle = atan2(forward.z, forward.x);
    float spread = glm::radians(55.0f);
    
    float angle = baseAngle + (((rand() % 100) / 100.0f - 0.5f) * 2.0f * spread);
    float dist = minDist + ((rand() % 100) / 100.0f) * (maxDist - minDist);
    
    float x = cos(angle) * dist;
    float z = sin(angle) * dist;
    float y = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
    
    p.Position = camPos + glm::vec3(x, y, z);
    
    float driftAngle = ((rand() % 100) / 100.0f) * 6.283185f;
    p.DriftDir = glm::vec3(cos(driftAngle), 0.0f, sin(driftAngle));
    
    p.Velocity = glm::vec3(
        ((rand() % 100) / 100.0f - 0.5f) * 0.01f,
        ((rand() % 100) / 100.0f - 0.5f) * 0.005f,
        ((rand() % 100) / 100.0f - 0.5f) * 0.01f
    );
    
    p.Color = glm::vec4(0.8f, 0.82f, 0.85f, 0.0f);
    p.Size = 3.0f + ((rand() % 100) / 100.0f) * 1.0f;
}

void ParticleSystem::spawnDustParticle(const Camera& camera, float cameraHeightMultiplier, int biomeType) {
    Particle &p = m_particles[m_nextParticle];
    m_nextParticle = (m_nextParticle + 1) % m_maxParticles;
    
    p.Life = 3.0f + ((rand() % 100) / 100.0f) * 2.0f;
    p.StartLife = p.Life;
    p.pType = PARTICLE_DUST;
    
    glm::vec3 camPos = camera.getPosition();
    glm::vec3 forward = camera.getLook();
    
    if (glm::length(camPos) < 0.1f || !std::isfinite(camPos.x) || !std::isfinite(camPos.y) || !std::isfinite(camPos.z)) {
        std::cerr << "Warning: Invalid camera position, skipping dust particle spawn" << std::endl;
        p.Life = 0.0f;
        return;
    }
    
    if (glm::length(forward) < 0.1f) {
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    forward = glm::normalize(forward);
    
    float minDist = 16.0f;
    float maxDist = 25.0f;
    
    float baseAngle = atan2(forward.z, forward.x);
    float spread = glm::radians(120.0f);
    
    float angle = baseAngle + (((rand() % 100) / 100.0f - 0.5f) * 2.0f * spread);
    float dist = minDist + ((rand() % 100) / 100.0f) * (maxDist - minDist);
    
    float x = cos(angle) * dist;
    float z = sin(angle) * dist;
    float y = -3.0f + ((rand() % 100) / 100.0f) * 0.8f;
    
    p.Position = camPos + glm::vec3(x, y, z);
    
    if (!std::isfinite(p.Position.x) || !std::isfinite(p.Position.y) || !std::isfinite(p.Position.z)) {
        std::cerr << "Warning: Invalid dust particle position calculated!" << std::endl;
        p.Life = 0.0f;
        return;
    }
    
    float driftAngle = ((rand() % 100) / 100.0f) * 6.283185f;
    p.DriftDir = glm::vec3(cos(driftAngle) * 0.3f, 0.7f, sin(driftAngle) * 0.3f);
    p.DriftDir = glm::normalize(p.DriftDir);
    
    p.Velocity = glm::vec3(
        ((rand() % 100) / 100.0f - 0.5f) * 0.2f,
        0.15f + ((rand() % 100) / 100.0f) * 0.2f,
        ((rand() % 100) / 100.0f - 0.5f) * 0.2f
    );
    
    p.Color = glm::vec4(0.6f, 0.55f, 0.45f, 0.0f);
    p.Size = 0.12f + ((rand() % 100) / 100.0f) * 0.12f;
    p.pType = PARTICLE_DUST_MOUNTAIN;
}

void ParticleSystem::spawnDustParticles(float deltaTime, const Camera& camera, float cameraHeightMultiplier, int biomeType) {
    if (!m_particlesEnabled) {
        return;
    }
    
    m_dustSpawnTimer -= deltaTime;
    if (m_dustSpawnTimer <= 0.0f) {
        spawnDustParticle(camera, cameraHeightMultiplier, biomeType);
        spawnDustParticle(camera, cameraHeightMultiplier, biomeType);
        if ((rand() % 100) < 70) {
            spawnDustParticle(camera, cameraHeightMultiplier, biomeType);
        }
        m_dustSpawnTimer = m_dustSpawnInterval + ((rand() % 100) / 100.0f) * 0.1f;
    }
}

void ParticleSystem::spawnLeafParticle(const Camera& camera, float cameraHeightMultiplier) {
    Particle &p = m_particles[m_nextParticle];
    
    glm::vec3 camPos = camera.getPosition();
    float spread = 1.5f;
    float rX = ((rand() % 100) / 100.0f - 0.5f) * spread;
    float rZ = ((rand() % 100) / 100.0f - 0.5f) * spread;
    float rY = 2.0f + ((rand() % 100) / 100.0f) * 3.0f;
    
    p.Position = camPos + glm::vec3(rX, rY, rZ);
    float driftAngle = ((rand() % 100) / 100.0f) * 6.283185f;
    float driftSpeed = 0.3f + ((rand() % 100) / 100.0f) * 0.3f;
    p.Velocity = glm::vec3(
        cos(driftAngle) * driftSpeed,
        -0.4f - ((rand() % 100) / 100.0f) * 0.3f,
        sin(driftAngle) * driftSpeed
    );
    p.Color = glm::vec4(0.3f, 0.5f, 0.2f, 1.0f);
    p.Life = 4.0f + ((rand() % 100) / 100.0f) * 2.0f;
    p.StartLife = p.Life;
    p.Size = 0.4f + ((rand() % 100) / 100.0f) * 0.3f;
    p.pType = PARTICLE_LEAF;
    p.DriftDir = glm::vec3(cos(driftAngle), 0.0f, sin(driftAngle));
    
    m_nextParticle = (m_nextParticle + 1) % m_maxParticles;
}

void ParticleSystem::spawnLeafParticles(float deltaTime, const Camera& camera, float cameraHeightMultiplier) {
    if (!m_particlesEnabled) {
        return;
    }
    
    m_leafSpawnTimer -= deltaTime;
    if (m_leafSpawnTimer <= 0.0f) {
        spawnLeafParticle(camera, cameraHeightMultiplier);
        m_leafSpawnTimer = m_leafSpawnInterval + ((rand() % 100) / 100.0f) * 0.3f;
        if ((rand() % 100) < 60) {
            spawnLeafParticle(camera, cameraHeightMultiplier);
        }
    }
}

void ParticleSystem::update(float deltaTime, const Camera& camera, bool isMoving, float cameraHeightMultiplier, int currentBiome) {
    if (!m_particlesEnabled) {
        return;
    }
    
    spawnDirtParticles(deltaTime, camera, isMoving, cameraHeightMultiplier);
    
    spawnDustParticles(deltaTime, camera, cameraHeightMultiplier, 1);
    
    if (currentBiome == 1) {
        spawnFogWisp(deltaTime, camera);
    }
    
    m_aliveFogInstances.clear();
    m_aliveDirtInstances.clear();
    m_aliveDustInstances.clear();
    m_aliveLeafInstances.clear();
    m_aliveFogInstances.reserve(m_maxParticles);
    m_aliveDirtInstances.reserve(m_maxParticles);
    m_aliveDustInstances.reserve(m_maxParticles);
    m_aliveLeafInstances.reserve(m_maxParticles);
    
    for (Particle &p : m_particles) {
        if (p.Life > 0.0f) {
            p.Life -= deltaTime;
            
            if (p.pType == PARTICLE_FOG_WISP) {
                float driftSpeed = 0.4f;
                p.Position += p.DriftDir * driftSpeed * deltaTime;
                
                float bobSpeed = 1.2f;
                float bobAmount = 0.25f;
                p.Position.y += sin((p.StartLife - p.Life) * bobSpeed) * bobAmount * deltaTime;
                
                glm::vec3 spread(
                    ((rand() % 100) / 100.0f - 0.5f) * 0.002f,
                    ((rand() % 100) / 100.0f - 0.5f) * 0.001f,
                    ((rand() % 100) / 100.0f - 0.5f) * 0.002f
                );
                p.Velocity += spread * deltaTime;
                
                float lifeRatio = p.Life / p.StartLife;
                if (lifeRatio > 0.8f) {
                    float t = (1.0f - lifeRatio) / 0.2f;
                    t = glm::clamp(t, 0.0f, 1.0f);
                    p.Color.a = glm::mix(0.0f, 0.15f, t);
                } else {
                    float t = lifeRatio / 0.8f;
                    t = glm::clamp(t, 0.0f, 1.0f);
                    p.Color.a = glm::mix(0.15f, 0.0f, 1.0f - t);
                }
            } else if (p.pType == PARTICLE_DUST || p.pType == PARTICLE_DUST_MOUNTAIN || 
                       p.pType == PARTICLE_DUST_FOREST || p.pType == PARTICLE_DUST_GRASSLAND) {
                float driftSpeed = 0.3f;
                p.Position += p.DriftDir * driftSpeed * deltaTime;
                
                float bobSpeed = 0.8f;
                float bobAmount = 0.15f;
                p.Position.y += sin((p.StartLife - p.Life) * bobSpeed) * bobAmount * deltaTime;
                
                glm::vec3 spread(
                    ((rand() % 100) / 100.0f - 0.5f) * 0.001f,
                    ((rand() % 100) / 100.0f - 0.5f) * 0.0005f,
                    ((rand() % 100) / 100.0f - 0.5f) * 0.001f
                );
                p.Velocity += spread * deltaTime;
                
                float lifeRatio = p.Life / p.StartLife;
                if (lifeRatio > 0.8f) {
                    float t = (1.0f - lifeRatio) / 0.2f;
                    t = glm::clamp(t, 0.0f, 1.0f);
                    p.Color.a = glm::mix(0.0f, 0.5f, t);
                } else {
                    float t = lifeRatio / 0.8f;
                    t = glm::clamp(t, 0.0f, 1.0f);
                    p.Color.a = glm::mix(0.5f, 0.0f, 1.0f - t);
                }
            } else if (p.pType == PARTICLE_LEAF) {
                p.Position += p.DriftDir * 0.2f * deltaTime;
                float rotation = sin((p.StartLife - p.Life) * 2.0f) * 0.1f;
                p.Velocity.x += cos(rotation) * 0.05f * deltaTime;
                p.Velocity.z += sin(rotation) * 0.05f * deltaTime;
                p.Color.a -= deltaTime * 0.25f;
            } else {
                p.Color.a -= deltaTime * 1.2f;
            }
            
            p.Position += p.Velocity * deltaTime;
            
            ParticleInstance inst;
            inst.pos = p.Position;
            inst.size = p.Size;
            inst.color = p.Color;
            inst.type = static_cast<int>(p.pType);
            
            if (p.pType == PARTICLE_FOG_WISP) {
                m_aliveFogInstances.push_back(inst);
            } else if (p.pType == PARTICLE_DUST || p.pType == PARTICLE_DUST_MOUNTAIN || 
                       p.pType == PARTICLE_DUST_FOREST || p.pType == PARTICLE_DUST_GRASSLAND) {
                m_aliveDustInstances.push_back(inst);
            } else if (p.pType == PARTICLE_LEAF) {
                m_aliveLeafInstances.push_back(inst);
            } else {
                m_aliveDirtInstances.push_back(inst);
            }
        }
    }
}

void ParticleSystem::draw(const Camera& camera) {
    if (!m_particlesEnabled || (m_aliveFogInstances.empty() && m_aliveDirtInstances.empty() && m_aliveDustInstances.empty() && m_aliveLeafInstances.empty())) {
        return;
    }
    
    if (m_particleShader == 0) {
        return;
    }
    
    glUseProgram(m_particleShader);
    
    // Camera matrices
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 proj = camera.getProjMatrix();
    glUniformMatrix4fv(glGetUniformLocation(m_particleShader, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_particleShader, "proj"), 1, GL_FALSE, &proj[0][0]);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    
    glBindVertexArray(m_particleVAO);
    
    if (!m_aliveFogInstances.empty() && m_fogWispsEnabled) {
        glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, m_aliveFogInstances.size() * sizeof(ParticleInstance), m_aliveFogInstances.data(), GL_DYNAMIC_DRAW);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_wispParticleTexture);
        glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(m_aliveFogInstances.size()));
    }
    
    if (!m_aliveDirtInstances.empty() && m_dirtParticlesEnabled) {
        glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, m_aliveDirtInstances.size() * sizeof(ParticleInstance), m_aliveDirtInstances.data(), GL_DYNAMIC_DRAW);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dirtParticleTexture);
        glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(m_aliveDirtInstances.size()));
    }
    
    if (!m_aliveDustInstances.empty() && m_dustMountainTexture != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, m_aliveDustInstances.size() * sizeof(ParticleInstance), m_aliveDustInstances.data(), GL_DYNAMIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_dustMountainTexture);
        glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(m_aliveDustInstances.size()));
    }
    
    if (!m_aliveLeafInstances.empty() && m_leafParticleTexture != 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_particleInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, m_aliveLeafInstances.size() * sizeof(ParticleInstance), m_aliveLeafInstances.data(), GL_DYNAMIC_DRAW);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_leafParticleTexture);
        glUniform1i(glGetUniformLocation(m_particleShader, "sprite"), 0);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(m_aliveLeafInstances.size()));
    }
    
    glBindVertexArray(0);
    
    glVertexAttribDivisor(2, 0);
    glVertexAttribDivisor(3, 0);
    glVertexAttribDivisor(4, 0);
    glVertexAttribDivisor(5, 0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUseProgram(0);
}

void ParticleSystem::setEnabled(bool enabled) {
    m_particlesEnabled = enabled;
}

void ParticleSystem::setDirtParticlesEnabled(bool enabled) {
    m_dirtParticlesEnabled = enabled;
}

void ParticleSystem::setFogWispsEnabled(bool enabled) {
    m_fogWispsEnabled = enabled;
}

void ParticleSystem::setDirtSpawnRate(float rate) {
    m_dirtSpawnRate = rate;
}

void ParticleSystem::setFogWispSpawnInterval(float interval) {
    m_fogWispSpawnInterval = interval;
}

void ParticleSystem::setMaxParticles(int maxParticles) {
    m_maxParticles = maxParticles;
    m_particles.resize(m_maxParticles);
}

