#pragma once

#include "enemy.h"
#include <vector>
#include <memory>
#include <QElapsedTimer>
#include <QString>
#include <glm/glm.hpp>

class Map;
class AudioManager;

class EnemyManager {
public:
    EnemyManager();
    
    void update(float deltaTime, const glm::vec3& cameraPosition, Map* map, AudioManager* audioManager);
    void updateWithFlashlight(float deltaTime, const glm::vec3& cameraPosition, Map* map,
                              bool flashlightOn, const glm::vec3& flashlightPos,
                              const glm::vec3& flashlightDir, float flashlightConeAngle,
                              AudioManager* audioManager);
    void render();
    void spawnEnemy(const glm::vec3& position);
    void spawnEnemiesOnRing(const glm::vec3& cameraPosition, int count);
    void clear();
    void killAllEnemies();
    
    int getEnemyCount() const { return static_cast<int>(m_enemies.size()); }
    const Enemy* getEnemy(int index) const;
    
    void setSpawnDelay(float delay) { 
        m_baseSpawnInterval = delay;
        m_spawnInterval = delay;
    }
    float getSpawnDelay() const { return m_baseSpawnInterval; }
    void setAutoSpawnEnabled(bool enabled) { 
        m_autoSpawnEnabled = enabled; 
        if (enabled) {
            m_spawnTimer = 0.0f;
        }
    }
    bool isAutoSpawnEnabled() const { return m_autoSpawnEnabled; }
    
private:
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    float m_spawnTimer;
    float m_spawnInterval;
    float m_baseSpawnInterval;
    float m_currentSpawnInterval;
    bool m_autoSpawnEnabled;
    QElapsedTimer m_elapsedTimer;
    
    //sound system
    std::vector<QString> m_enemyHitSounds;
    std::vector<QString> m_enemyNoiseSounds;
    std::vector<QString> m_enemyDeathSounds;
    std::vector<float> m_enemySoundTimers;
    std::vector<float> m_enemyNoiseTimers;
    std::vector<bool> m_enemyDeathSoundPlayed;
    bool m_enemyHitSoundsLoaded;
    bool m_enemyNoiseSoundsLoaded;
    bool m_enemyDeathSoundsLoaded;
    
    void loadEnemyHitSounds();
    void loadEnemyNoiseSounds();
    void loadEnemyDeathSounds();
    float generateRandomSize() const;
};

