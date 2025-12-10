#include "enemy.h"
#include "map/Map.h"
#include <algorithm>
#include <cmath>
#include <vector>

void Enemy::setAlive(bool alive) {
    if (!alive && m_alive && !m_isDying) {
        m_isDying = true;
        m_deathTimer = 0.0f;
        m_deathStartPosition = m_position;
    }
    m_alive = alive;
}

Enemy::Enemy(const glm::vec3& position, float sizeMultiplier)
    : m_position(position), m_velocity(0.0f), m_onGround(false), m_alive(true),
      m_health(100.0f), m_previousHealth(100.0f), m_isIlluminated(false), m_illuminationTime(0.0f),
      m_hasTakenDamage(false), m_timeSinceLastDamage(1.0f), 
      m_isDying(false), m_deathTimer(0.0f), m_deathStartPosition(position),
      m_sizeMultiplier(sizeMultiplier), m_jumpCooldown(JUMP_COOLDOWN), m_jumpTimer(0.0f) {
}

glm::vec3 Enemy::getBoundingBoxMin(const glm::vec3& pos) const {
    float width = getEnemyWidth();
    return glm::vec3(
        pos.x - width / 2.0f,
        pos.y,
        pos.z - width / 2.0f
    );
}

glm::vec3 Enemy::getBoundingBoxMax(const glm::vec3& pos) const {
    float width = getEnemyWidth();
    float height = getEnemyHeight();
    return glm::vec3(
        pos.x + width / 2.0f,
        pos.y + height,
        pos.z + width / 2.0f
    );
}

bool Enemy::checkCollision(Map* map, const glm::vec3& pos) {
    if (!map) return false;
    
    glm::vec3 min = getBoundingBoxMin(pos);
    glm::vec3 max = getBoundingBoxMax(pos);
    
    std::vector<glm::vec3> checkPoints = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z),
        glm::vec3(pos.x, min.y, pos.z),
        glm::vec3(pos.x, max.y, pos.z),
    };
    
    for (const auto& point : checkPoints) {
        int x = static_cast<int>(std::floor(point.x));
        int y = static_cast<int>(std::floor(point.y));
        int z = static_cast<int>(std::floor(point.z));
        
        if (map->hasBlock(x, y, z)) {
            return true;
        }
    }
    
    return false;
}

glm::vec3 Enemy::resolveCollision(Map* map, const glm::vec3& pos, const glm::vec3& oldPos) {
    glm::vec3 newPos = pos;
    
    if (!map) return newPos;
    
    if (!checkCollision(map, newPos)) {
        return newPos;
    }
    
    glm::vec3 delta = newPos - oldPos;
    
    glm::vec3 testPosX = glm::vec3(newPos.x, oldPos.y, oldPos.z);
    if (!checkCollision(map, testPosX)) {
        newPos.x = testPosX.x;
    } else {
        newPos.x = oldPos.x;
        m_velocity.x = 0.0f;
    }
    
    glm::vec3 testPosY = glm::vec3(newPos.x, newPos.y, oldPos.z);
    if (!checkCollision(map, testPosY)) {
        newPos.y = testPosY.y;
    } else {
        newPos.y = oldPos.y;
        if (delta.y < 0) {
            m_velocity.y = 0.0f;
            m_onGround = true;
        } else {
            m_velocity.y = 0.0f;
        }
    }
    
    glm::vec3 testPosZ = glm::vec3(newPos.x, newPos.y, newPos.z);
    if (!checkCollision(map, testPosZ)) {
        newPos.z = testPosZ.z;
    } else {
        newPos.z = oldPos.z;
        m_velocity.z = 0.0f;
    }
    
    glm::vec3 feetCheckPos = glm::vec3(newPos.x, newPos.y - 0.1f, newPos.z);
    int x = static_cast<int>(std::floor(feetCheckPos.x));
    int y = static_cast<int>(std::floor(feetCheckPos.y));
    int z = static_cast<int>(std::floor(feetCheckPos.z));
    
    if (map->hasBlock(x, y, z)) {
        m_onGround = true;
        if (m_velocity.y < 0) {
            m_velocity.y = 0;
        }
    }
    
    return newPos;
}

void Enemy::update(float deltaTime, const glm::vec3& targetPosition, Map* map) {
    if (m_isDying) {
        m_deathTimer += deltaTime;
        m_alive = false;
        return;
    }
    
    if (!m_alive || !map) return;
    
    m_timeSinceLastDamage += deltaTime;
    
    m_jumpTimer += deltaTime;
    
    if (!m_onGround) {
        m_velocity.y -= GRAVITY * deltaTime;
    }
    
    glm::vec3 direction = targetPosition - m_position;
    direction.y = 0.0f;
    float distance = glm::length(direction);
    
    if (distance > 0.1f) {
        direction = glm::normalize(direction);
        float moveSpeed = getMoveSpeed();
        m_velocity.x = direction.x * moveSpeed;
        m_velocity.z = direction.z * moveSpeed;
    } else {
        m_velocity.x = 0.0f;
        m_velocity.z = 0.0f;
    }
    
    if (m_onGround && m_jumpTimer >= m_jumpCooldown) {
        m_velocity.y = getJumpSpeed();
        m_onGround = false;
        m_jumpTimer = 0.0f;
    }
    
    float maxVelocity = std::max(std::abs(m_velocity.y), std::max(std::abs(m_velocity.x), std::abs(m_velocity.z)));
    int subSteps = 1;
    if (maxVelocity > 30.0f) {
        subSteps = 2;
    }
    if (maxVelocity > 60.0f) {
        subSteps = 4;
    }
    
    float subDeltaTime = deltaTime / static_cast<float>(subSteps);
    glm::vec3 resolvedPos = m_position;
    
    for (int step = 0; step < subSteps; step++) {
        glm::vec3 stepPos = resolvedPos;
        glm::vec3 newPos = stepPos + m_velocity * subDeltaTime;
        
        float velocityMagnitude = glm::length(m_velocity);
        float maxStep = 0.1f;
        if (velocityMagnitude > 20.0f) {
            maxStep = 0.05f;
        }
        if (velocityMagnitude > 50.0f) {
            maxStep = 0.02f;
        }
        
        glm::vec3 remaining = newPos - stepPos;
        float remainingDist = glm::length(remaining);
        
        if (remainingDist > maxStep) {
            glm::vec3 direction = glm::normalize(remaining);
            float stepDist = 0.0f;
            
            while (stepDist < remainingDist) {
                float stepSize = std::min(maxStep, remainingDist - stepDist);
                glm::vec3 sweepPos = resolvedPos + direction * stepSize;
                glm::vec3 sweepResolved = resolveCollision(map, sweepPos, resolvedPos);
                
                if (glm::length(sweepResolved - resolvedPos) < 0.001f) {
                    break;
                }
                
                resolvedPos = sweepResolved;
                stepDist += stepSize;
            }
        } else {
            resolvedPos = resolveCollision(map, newPos, resolvedPos);
        }
        
        if (step < subSteps - 1) {
            glm::vec3 feetCheckPos = glm::vec3(resolvedPos.x, resolvedPos.y - 0.1f, resolvedPos.z);
            int x = static_cast<int>(std::floor(feetCheckPos.x));
            int y = static_cast<int>(std::floor(feetCheckPos.y));
            int z = static_cast<int>(std::floor(feetCheckPos.z));
            
            if (map->hasBlock(x, y, z)) {
                m_onGround = true;
                if (m_velocity.y < 0) {
                    m_velocity.y = 0;
                }
            } else {
                m_onGround = false;
            }
        }
    }
    
    m_position = resolvedPos;
    
    if (m_position.y < -100.0f && !m_isDying) {
        m_alive = false;
        m_isDying = true;
        m_deathTimer = 0.0f;
        m_deathStartPosition = m_position;
    }
}

bool Enemy::isInFlashlightCone(const glm::vec3& flashlightPos, const glm::vec3& flashlightDir, 
                                float flashlightConeAngle) const {
    glm::vec3 toEnemy = m_position - flashlightPos;
    float dist = glm::length(toEnemy);
    if (dist < 0.001f) return true;
    
    glm::vec3 toEnemyDir = toEnemy / dist;
    glm::vec3 flashDirNorm = glm::normalize(flashlightDir);
    
    float cosAngle = glm::dot(flashDirNorm, toEnemyDir);
    float cosConeAngle = std::cos(flashlightConeAngle);
    
    return cosAngle >= cosConeAngle;
}

bool Enemy::hasLineOfSight(Map* map, const glm::vec3& from, const glm::vec3& to) const {
    if (!map) return false;
    
    glm::vec3 direction = to - from;
    float distance = glm::length(direction);
    if (distance < 0.001f) return true;
    
    glm::vec3 dirNorm = direction / distance;
    
    const float stepSize = 0.5f;
    int numSteps = static_cast<int>(distance / stepSize) + 1;
    
    for (int i = 1; i < numSteps; i++) {
        float t = (static_cast<float>(i) / static_cast<float>(numSteps)) * distance;
        glm::vec3 samplePos = from + dirNorm * t;
        
        int x = static_cast<int>(std::floor(samplePos.x));
        int y = static_cast<int>(std::floor(samplePos.y));
        int z = static_cast<int>(std::floor(samplePos.z));
        
        if (map->hasBlock(x, y, z)) {
            return false;
        }
    }
    
    return true;
}

void Enemy::updateWithFlashlight(float deltaTime, const glm::vec3& targetPosition, Map* map,
                                 bool flashlightOn, const glm::vec3& flashlightPos, 
                                 const glm::vec3& flashlightDir, float flashlightConeAngle) {
    if (m_isDying) {
        m_deathTimer += deltaTime;
    }
    
    if (!m_alive || !map) return;
    
    bool isIlluminated = false;
    if (flashlightOn) {
        if (isInFlashlightCone(flashlightPos, flashlightDir, flashlightConeAngle)) {
            glm::vec3 enemyCenter = m_position + glm::vec3(0.0f, getEnemyHeight() * 0.5f, 0.0f);
            if (hasLineOfSight(map, flashlightPos, enemyCenter)) {
                isIlluminated = true;
            }
        }
    }
    
    m_timeSinceLastDamage += deltaTime;
    
    if (m_isDying) {
        m_deathTimer += deltaTime;
        m_alive = false;
        return;
    }
    
    m_isIlluminated = isIlluminated;
    if (isIlluminated) {
        m_illuminationTime += deltaTime;
    } else {
        m_illuminationTime = 0.0f;
    }
    
    if (isIlluminated) {
        m_velocity.x = 0.0f;
        m_velocity.z = 0.0f;
        
        const float damagePerSecond = 50.0f;
        float oldHealth = m_health;
        m_health -= damagePerSecond * deltaTime;
        
        if (m_health < oldHealth && m_timeSinceLastDamage >= 0.25f) {
            m_hasTakenDamage = true;
            m_previousHealth = oldHealth;
            m_timeSinceLastDamage = 0.0f;
        }
        
        if (m_health <= 0.0f && !m_isDying) {
            m_health = 0.0f;
            m_alive = false;
            m_isDying = true;
            m_deathTimer = 0.0f;
            m_deathStartPosition = m_position;
            return;
        }
    } else {
        update(deltaTime, targetPosition, map);
    }
}

glm::vec3 Enemy::getVibrationOffset(float time) const {
    if (!m_isIlluminated) return glm::vec3(0.0f);
    
    float baseIntensity = isRecentlyDamaged() ? 0.28f : 0.2f;
    const float vibrationIntensity = baseIntensity;
    const float frequency1 = 50.0f;
    const float frequency2 = 45.0f;
    const float frequency3 = 55.0f;
    
    float intensityMultiplier = isRecentlyDamaged() ? (1.0f + std::sin(time * 100.0f) * 0.5f) : 1.0f;
    
    float x = std::sin(time * frequency1) * vibrationIntensity * intensityMultiplier;
    float y = std::cos(time * frequency2) * vibrationIntensity * intensityMultiplier;
    float z = std::sin(time * frequency3) * vibrationIntensity * intensityMultiplier;
    
    return glm::vec3(x, y, z);
}

glm::vec4 Enemy::getRandomFlashColor(float time) const {
    float seed = (m_position.x + m_position.y * 3.14159f + m_position.z * 7.0f) * 100.0f;
    float randomValue = std::fmod(std::sin(time * 12.0f + seed) * 43758.5453f, 1.0f);
    
    if (randomValue > 0.5f) {
        return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

glm::vec4 Enemy::getFlashColor(float time) const {
    if (!m_isIlluminated) {
        return getRandomFlashColor(time);
    }
    
    const float baseFlashSpeed = 30.0f;
    const float damageFlashSpeed = 120.0f;
    float flashSpeed = isRecentlyDamaged() ? damageFlashSpeed : baseFlashSpeed;
    float cycle = std::fmod(time * flashSpeed, 6.0f);
    if (cycle < 1.0f) {
        return glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    } else if (cycle < 2.0f) {
        return glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    } else if (cycle < 3.0f) {
        return glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    } else if (cycle < 4.0f) {
        return glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
    } else if (cycle < 5.0f) {
        return glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    } else {
        return glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
    }
}

bool Enemy::checkDamageThisFrame() {
    bool tookDamage = (m_health < m_previousHealth);
    m_previousHealth = m_health;
    return tookDamage;
}

void Enemy::render() {
}

