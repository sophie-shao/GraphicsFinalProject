#include "camera.h"
#include "transforms.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

Camera::Camera() {
    m_eye = glm::vec3(0.0f, 0.0f, 5.0f);
    m_look = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
    m_up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    m_fovY = 70.0f * glm::pi<float>() / 180.0f; // Default 70 degree FOV (Minecraft-like) in radians
    updateViewMatrix();
}

Camera::Camera(const SceneCameraData &data, float aspect, float nearPlane, float farPlane) {
    m_eye = glm::vec3(data.pos);
    m_look = glm::normalize(glm::vec3(data.look));
    m_up = glm::normalize(glm::vec3(data.up));
    m_fovY = data.heightAngle;

    updateViewMatrix();
    updateProjectionMatrix(aspect, nearPlane, farPlane);
}


void Camera::updateViewMatrix() {
    glm::vec3 center = m_eye + m_look;
    glm::vec3 w = glm::normalize(m_eye - center);
    glm::vec3 u = glm::normalize(glm::cross(m_up, w));
    glm::vec3 v = glm::cross(w, u);

    glm::mat4 rotation = glm::mat4(
        u.x, v.x, w.x, 0.0f,
        u.y, v.y, w.y, 0.0f,
        u.z, v.z, w.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glm::mat4 translation = Transform::translateMatrix(-m_eye);
    m_view = rotation * translation;
}


void Camera::updateProjectionMatrix(float aspect, float nearPlane, float farPlane) {
    float range = farPlane - nearPlane;
    float thetaH = m_fovY / 2.0f;

    float yScale = 1.0f / tan(thetaH);
    float xScale = yScale / aspect;
    float zScale = -(farPlane + nearPlane) / range;
    float zTrans = -(2.0f * farPlane * nearPlane) / range;

    m_proj = glm::mat4(0.0f);
    m_proj[0][0] = xScale;
    m_proj[1][1] = yScale;
    m_proj[2][2] = zScale;
    m_proj[2][3] = -1.0f;
    m_proj[3][2] = zTrans;
}

void Camera::moveForward(float dist) {
    m_eye += dist * m_look;
    updateViewMatrix();
}

void Camera::moveRight(float dist) {
    glm::vec3 right = glm::normalize(glm::cross(m_look, m_up));
    m_eye += dist * right;
    updateViewMatrix();
}

void Camera::moveUp(float dist) {
    m_eye += dist * m_up;
    updateViewMatrix();
}

void Camera::rotate(float deltaX, float deltaY) {
    glm::mat4 yaw = Transform::rotateMatrix(deltaX, m_up);
    m_look = glm::vec3(yaw * glm::vec4(m_look, 0.0f));
    glm::vec3 right = glm::normalize(glm::cross(m_look, m_up));
    glm::mat4 pitch = Transform::rotateMatrix(deltaY, right);
    m_look = glm::normalize(glm::vec3(pitch * glm::vec4(m_look, 0.0f)));
    updateViewMatrix();
}

void Camera::setPosition(const glm::vec3 &pos) {
    m_eye = pos;
    updateViewMatrix();
}

void Camera::setLook(const glm::vec3 &look) {
    m_look = glm::normalize(look);
    updateViewMatrix();
}

void Camera::setUp(const glm::vec3 &up) {
    m_up = glm::normalize(up);
    updateViewMatrix();
}

void Camera::setFOV(float fovDegrees) {
    m_fovY = fovDegrees * glm::pi<float>() / 180.0f;
}
