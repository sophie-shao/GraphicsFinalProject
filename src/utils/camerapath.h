#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <QElapsedTimer>

struct Waypoint {
    glm::vec3 position;
    glm::vec3 lookDirection;
    
    Waypoint() : position(0.0f), lookDirection(0.0f, 0.0f, -1.0f) {}
    Waypoint(const glm::vec3& pos, const glm::vec3& look) 
        : position(pos), lookDirection(glm::normalize(look)) {}
};

class CameraPath {
public:
    CameraPath();
    
    void addWaypoint(const glm::vec3& position, const glm::vec3& lookDirection);
    void clear();
    int getWaypointCount() const { return static_cast<int>(m_waypoints.size()); }
    
    void startPlayback(float durationSeconds = 10.0f);
    void stopPlayback();
    bool isPlaying() const { return m_isPlaying; }
    
    void update(float deltaTime, glm::vec3& outPosition, glm::vec3& outLookDirection);
    
    float getPlaybackDuration() const { return m_duration; }
    void setPlaybackDuration(float durationSeconds) { m_duration = durationSeconds; }
    
private:
    std::vector<Waypoint> m_waypoints;
    bool m_isPlaying;
    float m_currentTime;
    float m_duration;
    float m_speed;
    QElapsedTimer m_playbackTimer;
    
    glm::vec3 bezierInterpolate(const glm::vec3& p0, const glm::vec3& p1, 
                                const glm::vec3& p2, const glm::vec3& p3, float t);
    float calculateSegmentLength(int segmentIndex);
    float calculateTotalPathLength();
    void findSegmentAndT(float distance, int& outSegment, float& outT);
};

