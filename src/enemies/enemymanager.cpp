#include "enemymanager.h"
#include "map/Map.h"
#include "map/mapproperties.h"
#include "utils/audiomanager.h"
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QResource>
#include <algorithm>
#include <random>
#include <cmath>
#include <iostream>

EnemyManager::EnemyManager() : m_spawnTimer(0.0f), m_spawnInterval(30.0f), m_baseSpawnInterval(30.0f), 
                                 m_currentSpawnInterval(30.0f), m_autoSpawnEnabled(true), 
                                 m_enemyHitSoundsLoaded(false), m_enemyNoiseSoundsLoaded(false),
                                 m_enemyDeathSoundsLoaded(false) {
    m_elapsedTimer.start();
    loadEnemyHitSounds();
    loadEnemyNoiseSounds();
    loadEnemyDeathSounds();
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> intervalVariation(0.9f, 1.1f);
    m_currentSpawnInterval = m_baseSpawnInterval * intervalVariation(gen);
}

void EnemyManager::update(float deltaTime, const glm::vec3& cameraPosition, Map* map,
                          AudioManager* audioManager) {
    if (m_autoSpawnEnabled) {
        bool isInMountains = false;
        if (map != nullptr) {
            int playerX = static_cast<int>(std::floor(cameraPosition.x));
            int playerZ = static_cast<int>(std::floor(cameraPosition.z));
            BiomeType currentBiome = map->getBiomeAt(playerX, playerZ);
            isInMountains = (currentBiome == BIOME_MOUNTAINS);
        }
        
        float spawnRateMultiplier = isInMountains ? 2.0f : 1.0f;
        int aliveEnemyCount = 0;
        for (const auto& enemy : m_enemies) {
            if (enemy->isAlive() && !enemy->isDying()) {
                aliveEnemyCount++;
            }
        }
        
        if (aliveEnemyCount < 15) {
            m_spawnTimer += deltaTime * spawnRateMultiplier;
            
            if (m_spawnTimer >= m_currentSpawnInterval) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
                std::uniform_real_distribution<float> heightOffset(-2.0f, 2.0f);
                std::uniform_real_distribution<float> intervalVariation(0.9f, 1.1f);
                
                float angle = angleDist(gen);
                const float SPAWN_RADIUS = 33.0f;
                const float SPAWN_HEIGHT_OFFSET = 20.0f;
                const float SPAWN_HEIGHT = cameraPosition.y + SPAWN_HEIGHT_OFFSET;
                
                float x = cameraPosition.x + SPAWN_RADIUS * cosf(angle);
                float z = cameraPosition.z + SPAWN_RADIUS * sinf(angle);
                float y = SPAWN_HEIGHT + heightOffset(gen);
                
                glm::vec3 spawnPos = glm::vec3(x, y, z);
                spawnEnemy(spawnPos);
                m_spawnTimer = 0.0f;
                m_currentSpawnInterval = m_baseSpawnInterval * intervalVariation(gen);
                std::cout << "[Enemy Spawn] Auto-spawned enemy. Next spawn delay: " << m_currentSpawnInterval << "s" << std::endl;
            }
        }
    }
    
    // Remove dead enemies (only after death animation completes) and their corresponding sound timers
    size_t writeIndex = 0;
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        if (!m_enemies[i]->shouldBeRemoved()) {
            if (writeIndex != i) {
                m_enemies[writeIndex] = std::move(m_enemies[i]);
                m_enemyNoiseTimers[writeIndex] = m_enemyNoiseTimers[i];
                m_enemyDeathSoundPlayed[writeIndex] = m_enemyDeathSoundPlayed[i];
            }
            writeIndex++;
        }
    }
    m_enemies.resize(writeIndex);
    m_enemyNoiseTimers.resize(writeIndex);
    m_enemyDeathSoundPlayed.resize(writeIndex);
    
    if (!m_enemyNoiseSoundsLoaded) {
        loadEnemyNoiseSounds();
    }
    
    m_enemyNoiseTimers.resize(m_enemies.size(), 0.0f);
    m_enemyDeathSoundPlayed.resize(m_enemies.size(), false);
    
    const float noiseInterval = 5.0f;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    if (!m_enemyDeathSoundsLoaded) {
        loadEnemyDeathSounds();
    }
    
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        auto& enemy = m_enemies[i];
        
        if (enemy->isDying() && !m_enemyDeathSoundPlayed[i] && audioManager != nullptr && !m_enemyDeathSounds.empty()) {
            std::uniform_int_distribution<size_t> deathDist(0, m_enemyDeathSounds.size() - 1);
            
            size_t randomIndex1 = deathDist(gen);
            QString selectedSound1 = m_enemyDeathSounds[randomIndex1];
            audioManager->playSound(selectedSound1.toUtf8().constData());
            
            if (m_enemyDeathSounds.size() > 1) {
                size_t randomIndex2 = deathDist(gen);
                while (randomIndex2 == randomIndex1 && m_enemyDeathSounds.size() > 1) {
                    randomIndex2 = deathDist(gen);
                }
                QString selectedSound2 = m_enemyDeathSounds[randomIndex2];
                audioManager->playSound(selectedSound2.toUtf8().constData());
            }
            
            m_enemyDeathSoundPlayed[i] = true;
        }
        
        if (enemy->isDying()) {
            enemy->update(deltaTime, cameraPosition, map);
        } else {
            enemy->update(deltaTime, cameraPosition, map);
        }
        
        if (enemy->isAlive() && !enemy->isDying()) {
            m_enemyNoiseTimers[i] += deltaTime;
            
            glm::vec3 enemyPos = enemy->getPosition();
            float distanceToPlayer = glm::distance(enemyPos, cameraPosition);
            
            if (m_enemyNoiseTimers[i] >= noiseInterval && audioManager != nullptr && !m_enemyNoiseSounds.empty() && distanceToPlayer <= 25.0f) {
                std::uniform_int_distribution<size_t> noiseDist(0, m_enemyNoiseSounds.size() - 1);
                size_t randomIndex = noiseDist(gen);
                QString selectedNoise = m_enemyNoiseSounds[randomIndex];
                
                audioManager->playSound(selectedNoise.toUtf8().constData());
                m_enemyNoiseTimers[i] = 0.0f;
            }
        }
    }
}

void EnemyManager::updateWithFlashlight(float deltaTime, const glm::vec3& cameraPosition, Map* map,
                                       bool flashlightOn, const glm::vec3& flashlightPos,
                                       const glm::vec3& flashlightDir, float flashlightConeAngle,
                                       AudioManager* audioManager) {
    if (m_autoSpawnEnabled) {
        bool isInMountains = false;
        if (map != nullptr) {
            int playerX = static_cast<int>(std::floor(cameraPosition.x));
            int playerZ = static_cast<int>(std::floor(cameraPosition.z));
            BiomeType currentBiome = map->getBiomeAt(playerX, playerZ);
            isInMountains = (currentBiome == BIOME_MOUNTAINS);
        }
        
        float spawnRateMultiplier = isInMountains ? 2.0f : 1.0f;
        int aliveEnemyCount = 0;
        for (const auto& enemy : m_enemies) {
            if (enemy->isAlive() && !enemy->isDying()) {
                aliveEnemyCount++;
            }
        }
        
        if (aliveEnemyCount < 15) {
            m_spawnTimer += deltaTime * spawnRateMultiplier;
            
            if (m_spawnTimer >= m_currentSpawnInterval) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
                std::uniform_real_distribution<float> heightOffset(-2.0f, 2.0f);
                std::uniform_real_distribution<float> intervalVariation(0.9f, 1.1f);
                
                float angle = angleDist(gen);
                const float SPAWN_RADIUS = 30.0f;
                const float SPAWN_HEIGHT_OFFSET = 20.0f;
                const float SPAWN_HEIGHT = cameraPosition.y + SPAWN_HEIGHT_OFFSET;
                
                float x = cameraPosition.x + SPAWN_RADIUS * cosf(angle);
                float z = cameraPosition.z + SPAWN_RADIUS * sinf(angle);
                float y = SPAWN_HEIGHT + heightOffset(gen);
                
                glm::vec3 spawnPos = glm::vec3(x, y, z);
                spawnEnemy(spawnPos);
                m_spawnTimer = 0.0f;
                m_currentSpawnInterval = m_baseSpawnInterval * intervalVariation(gen);
                std::cout << "[Enemy Spawn] Auto-spawned enemy. Next spawn delay: " << m_currentSpawnInterval << "s" << std::endl;
            }
        }
    }
    
    // Remove dead enemies (only after death animation completes) and their corresponding sound timers
    size_t writeIndex = 0;
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        if (!m_enemies[i]->shouldBeRemoved()) {
            if (writeIndex != i) {
                m_enemies[writeIndex] = std::move(m_enemies[i]);
                m_enemySoundTimers[writeIndex] = m_enemySoundTimers[i];
                m_enemyNoiseTimers[writeIndex] = m_enemyNoiseTimers[i];
                m_enemyDeathSoundPlayed[writeIndex] = m_enemyDeathSoundPlayed[i];
            }
            writeIndex++;
        }
    }
    m_enemies.resize(writeIndex);
    m_enemySoundTimers.resize(writeIndex);
    m_enemyNoiseTimers.resize(writeIndex);
    m_enemyDeathSoundPlayed.resize(writeIndex);
    
    if (!m_enemyHitSoundsLoaded) {
        loadEnemyHitSounds();
    }
    
    if (!m_enemyNoiseSoundsLoaded) {
        loadEnemyNoiseSounds();
    }
    
    if (!m_enemyDeathSoundsLoaded) {
        loadEnemyDeathSounds();
    }
    
    m_enemySoundTimers.resize(m_enemies.size(), 0.0f);
    m_enemyNoiseTimers.resize(m_enemies.size(), 0.0f);
    m_enemyDeathSoundPlayed.resize(m_enemies.size(), false);
    
    const float soundInterval = 0.1f;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    const float noiseInterval = 5.0f;
    
    for (size_t i = 0; i < m_enemies.size(); ++i) {
        auto& enemy = m_enemies[i];
        
        if (enemy->isDying() && !m_enemyDeathSoundPlayed[i] && audioManager != nullptr && !m_enemyDeathSounds.empty()) {
            std::uniform_int_distribution<size_t> deathDist(0, m_enemyDeathSounds.size() - 1);
            
            size_t randomIndex1 = deathDist(gen);
            QString selectedSound1 = m_enemyDeathSounds[randomIndex1];
            audioManager->playSound(selectedSound1.toUtf8().constData());
            
            if (m_enemyDeathSounds.size() > 1) {
                size_t randomIndex2 = deathDist(gen);
                while (randomIndex2 == randomIndex1 && m_enemyDeathSounds.size() > 1) {
                    randomIndex2 = deathDist(gen);
                }
                QString selectedSound2 = m_enemyDeathSounds[randomIndex2];
                audioManager->playSound(selectedSound2.toUtf8().constData());
            }
            
            m_enemyDeathSoundPlayed[i] = true;
        }
        
        enemy->updateWithFlashlight(deltaTime, cameraPosition, map, flashlightOn, 
                                    flashlightPos, flashlightDir, flashlightConeAngle);
        
        m_enemySoundTimers[i] += deltaTime;
        
        if (enemy->isAlive() && !enemy->isDying()) {
            m_enemyNoiseTimers[i] += deltaTime;
            
            glm::vec3 enemyPos = enemy->getPosition();
            float distanceToPlayer = glm::distance(enemyPos, cameraPosition);
            
            if (m_enemyNoiseTimers[i] >= noiseInterval && audioManager != nullptr && !m_enemyNoiseSounds.empty() && distanceToPlayer <= 25.0f) {
                std::uniform_int_distribution<size_t> noiseDist(0, m_enemyNoiseSounds.size() - 1);
                size_t randomIndex = noiseDist(gen);
                QString selectedNoise = m_enemyNoiseSounds[randomIndex];
                
                audioManager->playSound(selectedNoise.toUtf8().constData());
                m_enemyNoiseTimers[i] = 0.0f;
            }
        }
        
        bool isTakingDamage = enemy->isIlluminated() && enemy->isRecentlyDamaged();
        
        if (isTakingDamage && audioManager != nullptr && !m_enemyHitSounds.empty()) {
            if (m_enemySoundTimers[i] >= soundInterval) {
                std::uniform_int_distribution<size_t> soundDist(0, m_enemyHitSounds.size() - 1);
                size_t randomIndex = soundDist(gen);
                QString selectedSound = m_enemyHitSounds[randomIndex];
                
                audioManager->playSound(selectedSound.toUtf8().constData());
                m_enemySoundTimers[i] = 0.0f;
            }
        } else {
            m_enemySoundTimers[i] = 0.0f;
        }
    }
}

void EnemyManager::spawnEnemy(const glm::vec3& position) {
    float sizeMultiplier = generateRandomSize();
    m_enemies.push_back(std::make_unique<Enemy>(position, sizeMultiplier));
    m_enemySoundTimers.push_back(0.0f);
    m_enemyDeathSoundPlayed.push_back(false);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> offsetDist(0.0f, 5.0f);
    m_enemyNoiseTimers.push_back(offsetDist(gen));
}

void EnemyManager::spawnEnemiesOnRing(const glm::vec3& cameraPosition, int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> heightOffset(-2.0f, 2.0f);
    
    const float SPAWN_RADIUS = 30.0f;
    const float SPAWN_HEIGHT_OFFSET = 20.0f;
    const float SPAWN_HEIGHT = cameraPosition.y + SPAWN_HEIGHT_OFFSET;
    
    for (int i = 0; i < count; i++) {
        float angle = angleDist(gen);
        float x = cameraPosition.x + SPAWN_RADIUS * cosf(angle);
        float z = cameraPosition.z + SPAWN_RADIUS * sinf(angle);
        float y = SPAWN_HEIGHT + heightOffset(gen);
        
        glm::vec3 spawnPos = glm::vec3(x, y, z);
        spawnEnemy(spawnPos);
    }
    std::cout << "[Enemy Spawn] Spawned " << count << " enemies from cube collection. Current auto-spawn delay: " << m_currentSpawnInterval << "s" << std::endl;
}

float EnemyManager::generateRandomSize() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.75f, 1.5f);
    return dis(gen);
}

void EnemyManager::loadEnemyHitSounds() {
    if (m_enemyHitSoundsLoaded) {
        return;
    }
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString enemyHitsDir = tempDir + "/enemyhits";
    
    QDir dir;
    if (!dir.exists(enemyHitsDir)) {
        dir.mkpath(enemyHitsDir);
    }
    
    QStringList resourceFiles = {
        ":/resources/soundeffects/enemyhits/enemyhit.wav",
        ":/resources/soundeffects/enemyhits/enemyhit2.wav",
        ":/resources/soundeffects/enemyhits/enemyhit3.wav",
        ":/resources/soundeffects/enemyhits/enemyhit4.wav",
        ":/resources/soundeffects/enemyhits/enemyhit5.wav",
        ":/resources/soundeffects/enemyhits/enemyhit6.wav",
        ":/resources/soundeffects/enemyhits/enemyhit7.wav",
    };
    
    for (const QString& resourcePath : resourceFiles) {
        if (QFile::exists(resourcePath)) {
            QString fileName = QFileInfo(resourcePath).fileName();
            QString tempPath = enemyHitsDir + "/" + fileName;
            
            if (!QFile::exists(tempPath)) {
                QFile::copy(resourcePath, tempPath);
            }
            
            m_enemyHitSounds.push_back(tempPath);
        }
    }
    
    if (m_enemyHitSounds.empty()) {
        QString fallbackPath = tempDir + "/enemyhit.wav";
        if (QFile::exists(":/resources/soundeffects/enemyhit.wav")) {
            if (!QFile::exists(fallbackPath)) {
                QFile::copy(":/resources/soundeffects/enemyhit.wav", fallbackPath);
            }
            m_enemyHitSounds.push_back(fallbackPath);
        }
    }
    
    m_enemyHitSoundsLoaded = true;
}

void EnemyManager::loadEnemyNoiseSounds() {
    if (m_enemyNoiseSoundsLoaded) {
        return;
    }
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    
    QStringList resourceFiles = {
        ":/resources/soundeffects/enemynoises/enemy_noise1.wav",
        ":/resources/soundeffects/enemynoises/enemy_noise2.wav",
        ":/resources/soundeffects/enemynoises/enemy_noise3.wav",
        ":/resources/soundeffects/enemynoises/enemy_noise4.wav",
        ":/resources/soundeffects/enemynoises/enemy_noise5.wav",
    };
    
    for (const QString& resourcePath : resourceFiles) {
        if (QFile::exists(resourcePath)) {
            QString fileName = QFileInfo(resourcePath).fileName();
            QString tempPath = tempDir + "/" + fileName;
            
            if (!QFile::exists(tempPath)) {
                QFile::copy(resourcePath, tempPath);
            }
            
            m_enemyNoiseSounds.push_back(tempPath);
        }
    }
    
    m_enemyNoiseSoundsLoaded = true;
}

void EnemyManager::loadEnemyDeathSounds() {
    if (m_enemyDeathSoundsLoaded) {
        return;
    }
    
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString enemyDieDir = tempDir + "/enemydie";
    
    QDir dir;
    if (!dir.exists(enemyDieDir)) {
        dir.mkpath(enemyDieDir);
    }
    
    QStringList resourceFiles = {
        ":/resources/soundeffects/enemydie/enemy_die.wav",
        ":/resources/soundeffects/enemydie/enemy_die1.wav",
        ":/resources/soundeffects/enemydie/enemy_die2.wav",
        ":/resources/soundeffects/enemydie/enemy_die3.wav",
        ":/resources/soundeffects/enemydie/enemy_die4.wav",
    };
    
    for (const QString& resourcePath : resourceFiles) {
        if (QFile::exists(resourcePath)) {
            QString fileName = QFileInfo(resourcePath).fileName();
            QString tempPath = enemyDieDir + "/" + fileName;
            
            if (!QFile::exists(tempPath)) {
                QFile::copy(resourcePath, tempPath);
            }
            
            m_enemyDeathSounds.push_back(tempPath);
        }
    }
    
    m_enemyDeathSoundsLoaded = true;
}

void EnemyManager::clear() {
    m_enemies.clear();
    m_enemySoundTimers.clear();
    m_enemyNoiseTimers.clear();
    m_enemyDeathSoundPlayed.clear();
    m_spawnTimer = 0.0f;
}

void EnemyManager::killAllEnemies() {
    for (auto& enemy : m_enemies) {
        if (enemy->isAlive() && !enemy->isDying()) {
            enemy->setAlive(false);
        }
    }
}

void EnemyManager::render() {
}

const Enemy* EnemyManager::getEnemy(int index) const {
    if (index >= 0 && index < static_cast<int>(m_enemies.size())) {
        return m_enemies[index].get();
    }
    return nullptr;
}

