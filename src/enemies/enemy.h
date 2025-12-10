#pragma once

#include <glm/glm.hpp>
#include <QElapsedTimer>

class Map;

class Enemy {
public:
    Enemy(const glm::vec3& position, float sizeMultiplier = 1.0f);
    
    void update(float deltaTime, const glm::vec3& targetPosition, Map* map);
    void updateWithFlashlight(float deltaTime, const glm::vec3& targetPosition, Map* map,
                              bool flashlightOn, const glm::vec3& flashlightPos, 
                              const glm::vec3& flashlightDir, float flashlightConeAngle);
    void render();
    
    glm::vec3 getPosition() const { return m_position; }
    bool isAlive() const { return m_alive; }
    void setAlive(bool alive);
    float getSizeMultiplier() const { return m_sizeMultiplier; }
    float getHealth() const { return m_health; }
    void setHealth(float health) { m_health = health; }
    bool isIlluminated() const { return m_isIlluminated; }
    float getIlluminationTime() const { return m_illuminationTime; }
    glm::vec3 getVibrationOffset(float time) const;
    glm::vec4 getFlashColor(float time) const;
    glm::vec4 getRandomFlashColor(float time) const;
    bool hasTakenDamage() const { return m_hasTakenDamage; }
    bool checkDamageThisFrame();
    float getTimeSinceLastDamage() const { return m_timeSinceLastDamage; }
    bool isRecentlyDamaged() const { return m_timeSinceLastDamage < 0.25f; }
    
    bool isDying() const { return m_isDying; }
    float getDeathProgress() const { return m_deathTimer / DEATH_ANIMATION_DURATION; }
    bool shouldBeRemoved() const { return m_isDying && m_deathTimer >= DEATH_ANIMATION_DURATION; }
    glm::vec3 getDeathStartPosition() const { return m_deathStartPosition; }
    
private:
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    bool m_onGround;
    bool m_alive;
    float m_health;
    float m_previousHealth;
    bool m_isIlluminated;
    float m_illuminationTime;
    bool m_hasTakenDamage;
    float m_timeSinceLastDamage;
    
    bool m_isDying;
    float m_deathTimer;
    glm::vec3 m_deathStartPosition;
    static constexpr float DEATH_ANIMATION_DURATION = 0.3f;
    
    float m_sizeMultiplier;
    float m_jumpCooldown;
    float m_jumpTimer;
    
    static constexpr float BASE_ENEMY_WIDTH = 0.6f;
    static constexpr float BASE_ENEMY_HEIGHT = 1.8f;
    static constexpr float GRAVITY = 9.8f;
    static constexpr float BASE_JUMP_SPEED = 5.0f;
    static constexpr float BASE_MOVE_SPEED = 3.6f;
    static constexpr float JUMP_COOLDOWN = 2.0f;
    
    float getEnemyWidth() const { return BASE_ENEMY_WIDTH * m_sizeMultiplier; }
    float getEnemyHeight() const { return BASE_ENEMY_HEIGHT * m_sizeMultiplier; }
    float getMoveSpeed() const { return BASE_MOVE_SPEED / m_sizeMultiplier; }
    float getJumpSpeed() const { return BASE_JUMP_SPEED * m_sizeMultiplier; }
    
    bool checkCollision(Map* map, const glm::vec3& pos);
    glm::vec3 resolveCollision(Map* map, const glm::vec3& pos, const glm::vec3& oldPos);
    glm::vec3 getBoundingBoxMin(const glm::vec3& pos) const;
    glm::vec3 getBoundingBoxMax(const glm::vec3& pos) const;
    
    bool isInFlashlightCone(const glm::vec3& flashlightPos, const glm::vec3& flashlightDir, 
                            float flashlightConeAngle) const;
    bool hasLineOfSight(Map* map, const glm::vec3& from, const glm::vec3& to) const;
};

