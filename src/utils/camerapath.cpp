#include "camerapath.h"
#include <cmath>
#include <algorithm>

CameraPath::CameraPath() 
    : m_isPlaying(false), m_currentTime(0.0f), m_duration(10.0f), m_speed(1.0f) {
}

void CameraPath::addWaypoint(const glm::vec3& position, const glm::vec3& lookDirection) {
    m_waypoints.push_back(Waypoint(position, lookDirection));
}

void CameraPath::clear() {
    m_waypoints.clear();
    m_isPlaying = false;
    m_currentTime = 0.0f;
}

void CameraPath::startPlayback(float durationSeconds) {
    if (m_waypoints.size() < 2) {
        return;
    }
    m_duration = durationSeconds;
    float totalLength = calculateTotalPathLength();
    if (totalLength > 0.0f && m_duration > 0.0f) {
        m_speed = totalLength / m_duration;
    } else {
        m_speed = 1.0f;
    }
    m_isPlaying = true;
    m_currentTime = 0.0f;
    m_playbackTimer.restart();
}

void CameraPath::stopPlayback() {
    m_isPlaying = false;
    m_currentTime = 0.0f;
}

glm::vec3 CameraPath::bezierInterpolate(const glm::vec3& p0, const glm::vec3& p1, 
                                         const glm::vec3& p2, const glm::vec3& p3, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    
    return mt3 * p0 + 3.0f * mt2 * t * p1 + 3.0f * mt * t2 * p2 + t3 * p3;
}

float CameraPath::calculateSegmentLength(int segmentIndex) {
    if (segmentIndex < 0 || segmentIndex >= static_cast<int>(m_waypoints.size()) - 1) {
        return 0.0f;
    }
    
    const int numSamples = 20;
    float length = 0.0f;
    glm::vec3 prevPos = m_waypoints[segmentIndex].position;
    
    for (int i = 1; i <= numSamples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numSamples);
        
        glm::vec3 p0 = m_waypoints[segmentIndex].position;
        glm::vec3 p3 = m_waypoints[segmentIndex + 1].position;
        
        glm::vec3 p1, p2;
        if (segmentIndex > 0) {
            glm::vec3 dir = p3 - m_waypoints[segmentIndex - 1].position;
            p1 = p0 + dir * 0.3f;
        } else {
            p1 = p0;
        }
        
        if (segmentIndex + 2 < static_cast<int>(m_waypoints.size())) {
            glm::vec3 dir = m_waypoints[segmentIndex + 2].position - p0;
            p2 = p3 - dir * 0.3f;
        } else {
            p2 = p3;
        }
        
        glm::vec3 currentPos = bezierInterpolate(p0, p1, p2, p3, t);
        length += glm::length(currentPos - prevPos);
        prevPos = currentPos;
    }
    
    return length;
}

float CameraPath::calculateTotalPathLength() {
    float totalLength = 0.0f;
    for (int i = 0; i < static_cast<int>(m_waypoints.size()) - 1; ++i) {
        totalLength += calculateSegmentLength(i);
    }
    return totalLength;
}

void CameraPath::findSegmentAndT(float distance, int& outSegment, float& outT) {
    outSegment = 0;
    outT = 0.0f;
    
    if (m_waypoints.size() < 2) {
        return;
    }
    
    float totalLength = calculateTotalPathLength();
    if (totalLength <= 0.0f) {
        return;
    }
    
    float targetDistance = std::fmod(distance, totalLength);
    if (targetDistance < 0.0f) {
        targetDistance += totalLength;
    }
    
    float accumulatedDistance = 0.0f;
    for (int i = 0; i < static_cast<int>(m_waypoints.size()) - 1; ++i) {
        float segmentLength = calculateSegmentLength(i);
        
        if (accumulatedDistance + segmentLength >= targetDistance) {
            outSegment = i;
            float remainingDistance = targetDistance - accumulatedDistance;
            outT = remainingDistance / segmentLength;
            return;
        }
        
        accumulatedDistance += segmentLength;
    }
    
    outSegment = static_cast<int>(m_waypoints.size()) - 2;
    outT = 1.0f;
}

void CameraPath::update(float deltaTime, glm::vec3& outPosition, glm::vec3& outLookDirection) {
    if (!m_isPlaying || m_waypoints.size() < 2) {
        return;
    }
    
    float totalLength = calculateTotalPathLength();
    if (totalLength <= 0.0f) {
        stopPlayback();
        return;
    }
    
    m_currentTime += deltaTime;
    float distance = m_currentTime * m_speed;
    
    if (distance >= totalLength) {
        distance = totalLength;
        stopPlayback();
    }
    
    int segment;
    float t;
    findSegmentAndT(distance, segment, t);
    
    if (segment < 0 || segment >= static_cast<int>(m_waypoints.size()) - 1) {
        stopPlayback();
        return;
    }
    
    glm::vec3 p0 = m_waypoints[segment].position;
    glm::vec3 p3 = m_waypoints[segment + 1].position;
    
    glm::vec3 p1, p2;
    if (segment > 0) {
        glm::vec3 dir = p3 - m_waypoints[segment - 1].position;
        p1 = p0 + dir * 0.3f;
    } else {
        p1 = p0;
    }
    
    if (segment + 2 < static_cast<int>(m_waypoints.size())) {
        glm::vec3 dir = m_waypoints[segment + 2].position - p0;
        p2 = p3 - dir * 0.3f;
    } else {
        p2 = p3;
    }
    
    outPosition = bezierInterpolate(p0, p1, p2, p3, t);
    
    glm::vec3 look0 = m_waypoints[segment].lookDirection;
    glm::vec3 look3 = m_waypoints[segment + 1].lookDirection;
    
    glm::vec3 look1, look2;
    if (segment > 0) {
        glm::vec3 lookDir = look3 - m_waypoints[segment - 1].lookDirection;
        look1 = look0 + lookDir * 0.3f;
    } else {
        look1 = look0;
    }
    
    if (segment + 2 < static_cast<int>(m_waypoints.size())) {
        glm::vec3 lookDir = m_waypoints[segment + 2].lookDirection - look0;
        look2 = look3 - lookDir * 0.3f;
    } else {
        look2 = look3;
    }
    
    outLookDirection = bezierInterpolate(look0, look1, look2, look3, t);
    outLookDirection = glm::normalize(outLookDirection);
}

