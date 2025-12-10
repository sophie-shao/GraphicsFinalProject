#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "scenedata.h"

class Camera {
public:
    Camera();
    Camera(const SceneCameraData &data, float aspect, float nearPlane, float farPlane);

    void updateViewMatrix();
    void updateProjectionMatrix(float aspect, float nearPlane, float farPlane);
    void setFOV(float fovDegrees);

    //movement
    void moveForward(float dist);
    void moveRight(float dist);
    void moveUp(float dist);
    void rotate(float deltaX, float deltaY);

    const glm::mat4 &getViewMatrix() const { return m_view; }
    const glm::mat4 &getProjMatrix() const { return m_proj; }
    const glm::vec3 &getPosition() const { return m_eye; }
    const glm::vec3 &getLook() const { return m_look; }
    const glm::vec3 &getUp() const { return m_up; }
    
    //setters for camera positioning
    void setPosition(const glm::vec3 &pos);
    void setLook(const glm::vec3 &look);
    void setUp(const glm::vec3 &up);

private:
    glm::vec3 m_eye;
    glm::vec3 m_look;
    glm::vec3 m_up;
    glm::mat4 m_view;

    glm::mat4 m_proj;
    float m_fovY;
};
