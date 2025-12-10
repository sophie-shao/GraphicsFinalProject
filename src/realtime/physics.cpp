#include "physics.h"
#include "../realtime.h"
#include <algorithm>
#include <cmath>
#include <iostream>

bool Physics::checkCollision(const Realtime* realtime, const glm::vec3& pos) {
    if (realtime->m_activeMap == nullptr) {
        return false;
    }
    
    const float PLAYER_WIDTH = 0.6f;
    const float PLAYER_HEIGHT = 1.8f;
    const float BASE_EYE_HEIGHT = 1.6f;
    float cameraHeightMultiplier = static_cast<float>(realtime->m_cameraHeightMultiplier);
    cameraHeightMultiplier = std::max(0.25f, std::min(3.0f, cameraHeightMultiplier));
    float eyeHeight = BASE_EYE_HEIGHT * cameraHeightMultiplier;
    const float PLAYER_RADIUS = PLAYER_WIDTH / 2.0f;
    
    float feetY = pos.y - eyeHeight;
    
    std::vector<glm::vec3> checkPoints = {
        glm::vec3(pos.x - PLAYER_RADIUS, feetY, pos.z - PLAYER_RADIUS),
        glm::vec3(pos.x + PLAYER_RADIUS, feetY, pos.z - PLAYER_RADIUS),
        glm::vec3(pos.x - PLAYER_RADIUS, feetY, pos.z + PLAYER_RADIUS),
        glm::vec3(pos.x + PLAYER_RADIUS, feetY, pos.z + PLAYER_RADIUS),
        glm::vec3(pos.x - PLAYER_RADIUS, feetY + PLAYER_HEIGHT, pos.z - PLAYER_RADIUS),
        glm::vec3(pos.x + PLAYER_RADIUS, feetY + PLAYER_HEIGHT, pos.z - PLAYER_RADIUS),
        glm::vec3(pos.x - PLAYER_RADIUS, feetY + PLAYER_HEIGHT, pos.z + PLAYER_RADIUS),
        glm::vec3(pos.x + PLAYER_RADIUS, feetY + PLAYER_HEIGHT, pos.z + PLAYER_RADIUS),
        glm::vec3(pos.x, feetY, pos.z),
        glm::vec3(pos.x, feetY + PLAYER_HEIGHT, pos.z),
    };
    
    for (const auto& point : checkPoints) {
        int x = static_cast<int>(std::floor(point.x));
        int y = static_cast<int>(std::floor(point.y));
        int z = static_cast<int>(std::floor(point.z));
        
        if (realtime->m_activeMap->hasBlock(x, y, z)) {
            return true;
        }
    }
    
    return false;
}

glm::vec3 Physics::resolveCollision(Realtime* realtime, const glm::vec3& pos, const glm::vec3& oldPos) {
    glm::vec3 newPos = pos;
    
    if (realtime->m_activeMap == nullptr) {
        return newPos;
    }
    
    if (!checkCollision(realtime, newPos)) {
        return newPos;
    }
    
    glm::vec3 delta = newPos - oldPos;
    
    glm::vec3 testPosX = glm::vec3(newPos.x, oldPos.y, oldPos.z);
    if (!checkCollision(realtime, testPosX)) {
        newPos.x = testPosX.x;
    } else {
        newPos.x = oldPos.x;
        realtime->m_velocity.x = 0;
    }
    
    glm::vec3 testPosY = glm::vec3(newPos.x, newPos.y, oldPos.z);
    if (!checkCollision(realtime, testPosY)) {
        newPos.y = testPosY.y;
    } else {
        newPos.y = oldPos.y;
        if (delta.y >= 0) {
            realtime->m_velocity.y = 0;
        }
    }
    
    glm::vec3 testPosZ = glm::vec3(newPos.x, newPos.y, newPos.z);
    if (!checkCollision(realtime, testPosZ)) {
        newPos.z = testPosZ.z;
    } else {
        newPos.z = oldPos.z;
        realtime->m_velocity.z = 0;
    }
    
    return newPos;
}

void Physics::updatePhysics(Realtime* realtime, float deltaTime) {
    if (realtime->m_activeMap == nullptr) {
        float moveSpeed = 12.5f * deltaTime;
        if (realtime->m_keyMap[Qt::Key_Shift]) {
            moveSpeed *= 2.0f;
        }
        if (realtime->m_keyMap[Qt::Key_W]) realtime->m_camera.moveForward(moveSpeed);
        if (realtime->m_keyMap[Qt::Key_S]) realtime->m_camera.moveForward(-moveSpeed);
        if (realtime->m_keyMap[Qt::Key_A]) realtime->m_camera.moveRight(-moveSpeed);
        if (realtime->m_keyMap[Qt::Key_D]) realtime->m_camera.moveRight(moveSpeed);
        if (realtime->m_keyMap[Qt::Key_Space]) realtime->m_camera.moveUp(moveSpeed);
        if (realtime->m_keyMap[Qt::Key_Control]) realtime->m_camera.moveUp(-moveSpeed);
        return;
    }
    
    glm::vec3 currentPos = realtime->m_camera.getPosition();
    
    const float BASE_GRAVITY = -20.0f;
    const float BASE_JUMP_SPEED = 8.0f;
    const float BASE_MOVE_SPEED = 4.6875f;
    const float GROUND_FRICTION = 0.92f;
    const float AIR_FRICTION = 0.95f;
    
    float moveSpeed = BASE_MOVE_SPEED * static_cast<float>(realtime->m_movementSpeedMultiplier);
    if (realtime->m_keyMap[Qt::Key_Shift]) {
        moveSpeed *= 2.0f;
    }
    float jumpSpeed = BASE_JUMP_SPEED * static_cast<float>(realtime->m_jumpHeightMultiplier);
    float gravity = BASE_GRAVITY * static_cast<float>(realtime->m_gravityMultiplier);
    
    glm::vec3 forward = glm::normalize(glm::vec3(realtime->m_camera.getLook().x, 0, realtime->m_camera.getLook().z));
    if (glm::length(forward) < 0.001f) {
        forward = glm::vec3(0, 0, -1);
    } else {
        forward = glm::normalize(forward);
    }
    
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    
    //general movement controls - set velocity directly
    glm::vec3 desiredVelocity = glm::vec3(0.0f);
    if (realtime->m_keyMap[Qt::Key_W]) {
        desiredVelocity += forward * moveSpeed;
    }
    if (realtime->m_keyMap[Qt::Key_S]) {
        desiredVelocity -= forward * moveSpeed;
    }
    if (realtime->m_keyMap[Qt::Key_A]) {
        desiredVelocity -= right * moveSpeed;
    }
    if (realtime->m_keyMap[Qt::Key_D]) {
        desiredVelocity += right * moveSpeed;
    }
    
    realtime->m_velocity.x = desiredVelocity.x;
    realtime->m_velocity.z = desiredVelocity.z;
    
    //check for jump input
    static bool debugJump = false;
    if (realtime->m_keyMap[Qt::Key_Space]) {
        if (debugJump) {
            std::cout << "=== JUMP DEBUG ===" << std::endl;
            std::cout << "  Space pressed: YES" << std::endl;
            std::cout << "  m_onGround: " << (realtime->m_onGround ? "TRUE" : "FALSE") << std::endl;
            std::cout << "  Current velocity.y: " << realtime->m_velocity.y << std::endl;
        }
        if (realtime->m_onGround) {
            realtime->m_velocity.y = jumpSpeed;
            realtime->m_onGround = false;
            if (debugJump) {
                std::cout << "  JUMP EXECUTED! New velocity.y: " << realtime->m_velocity.y << std::endl;
            }
        } else {
            if (debugJump) {
                std::cout << "  JUMP BLOCKED - not on ground!" << std::endl;
            }
        }
        debugJump = false; // Only debug once
    }
    
    float maxVelocity = std::max(std::abs(realtime->m_velocity.y), std::max(std::abs(realtime->m_velocity.x), std::abs(realtime->m_velocity.z)));
    int subSteps = 1;
    if (maxVelocity > 30.0f) {
        subSteps = 2;
    }
    if (maxVelocity > 60.0f) {
        subSteps = 4;
    }
    
    float subDeltaTime = deltaTime / static_cast<float>(subSteps);
    glm::vec3 resolvedPos = currentPos;
    
    for (int step = 0; step < subSteps; step++) {
        glm::vec3 stepPos = resolvedPos;
        
        if (!realtime->m_onGround) {
            realtime->m_velocity.y += gravity * subDeltaTime;
        }
        
        glm::vec3 newPos = stepPos + realtime->m_velocity * subDeltaTime;
        
        float velocityMagnitude = glm::length(realtime->m_velocity);
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
                glm::vec3 sweepResolved = resolveCollision(realtime, sweepPos, resolvedPos);
                
                if (glm::length(sweepResolved - resolvedPos) < 0.001f) {
                    break;
                }
                
                resolvedPos = sweepResolved;
                stepDist += stepSize;
            }
        } else {
            resolvedPos = resolveCollision(realtime, newPos, resolvedPos);
        }
        
        if (step < subSteps - 1) {
            const float BASE_EYE_HEIGHT = 1.6f;
            float cameraHeightMultiplier = static_cast<float>(realtime->m_cameraHeightMultiplier);
            cameraHeightMultiplier = std::max(0.25f, std::min(3.0f, cameraHeightMultiplier));
            float eyeHeight = BASE_EYE_HEIGHT * cameraHeightMultiplier;
            float feetY = resolvedPos.y - eyeHeight;
            
            glm::vec3 feetCheckPos = glm::vec3(resolvedPos.x, feetY - 0.1f, resolvedPos.z);
            int x = static_cast<int>(std::floor(feetCheckPos.x));
            int y = static_cast<int>(std::floor(feetCheckPos.y));
            int z = static_cast<int>(std::floor(feetCheckPos.z));
            
            if (realtime->m_activeMap->hasBlock(x, y, z)) {
                realtime->m_onGround = true;
                if (realtime->m_velocity.y < 0) {
                    realtime->m_velocity.y = 0;
                }
            } else {
                realtime->m_onGround = false;
            }
        }
    }
    
    const float BASE_EYE_HEIGHT = 1.6f; //TUNE ROSSSSSS

    float cameraHeightMultiplier = static_cast<float>(realtime->m_cameraHeightMultiplier);
    cameraHeightMultiplier = std::max(0.25f, std::min(3.0f, cameraHeightMultiplier));
    float eyeHeight = BASE_EYE_HEIGHT * cameraHeightMultiplier;
    const float PLAYER_WIDTH = 0.6f;
    const float PLAYER_RADIUS = PLAYER_WIDTH / 2.0f;
    float feetY = resolvedPos.y - eyeHeight;
    
    std::vector<glm::vec3> feetCheckPoints = {
        glm::vec3(resolvedPos.x - PLAYER_RADIUS, feetY - 0.1f, resolvedPos.z - PLAYER_RADIUS),
        glm::vec3(resolvedPos.x + PLAYER_RADIUS, feetY - 0.1f, resolvedPos.z - PLAYER_RADIUS),
        glm::vec3(resolvedPos.x - PLAYER_RADIUS, feetY - 0.1f, resolvedPos.z + PLAYER_RADIUS),
        glm::vec3(resolvedPos.x + PLAYER_RADIUS, feetY - 0.1f, resolvedPos.z + PLAYER_RADIUS),
        glm::vec3(resolvedPos.x, feetY - 0.1f, resolvedPos.z),
    };
    
    //start by assuming not on ground, then check
    realtime->m_onGround = false;
    
    for (const auto& feetPos : feetCheckPoints) {
        int x = static_cast<int>(std::floor(feetPos.x));
        int y = static_cast<int>(std::floor(feetPos.y));
        int z = static_cast<int>(std::floor(feetPos.z));
        
        if (realtime->m_activeMap->hasBlock(x, y, z)) {
            realtime->m_onGround = true;
            if (realtime->m_velocity.y < 0) {
                realtime->m_velocity.y = 0;
            }
            float blockTop = static_cast<float>(y + 1);
            float desiredFeetY = blockTop + 0.01f;
            resolvedPos.y = desiredFeetY + eyeHeight;
            break;
        }
    }
    
    
    if (realtime->m_onGround && realtime->m_velocity.y < 0) {
        realtime->m_velocity.y = 0;
    }
    
    if (checkCollision(realtime, resolvedPos)) {
        glm::vec3 testPos = resolvedPos;
        testPos.y += 0.1f;
        if (!checkCollision(realtime, testPos)) {
            resolvedPos = testPos;
        } else {
            resolvedPos = currentPos;
            realtime->m_velocity = glm::vec3(0.0f);
        }
    }
    
    realtime->m_camera.setPosition(resolvedPos);
}

