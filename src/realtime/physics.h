#pragma once

#include <glm/glm.hpp>

class Realtime;

class Physics {
public:
    static bool checkCollision(const Realtime* realtime, const glm::vec3& pos);
    static glm::vec3 resolveCollision(Realtime* realtime, const glm::vec3& pos, const glm::vec3& oldPos);
    static void updatePhysics(Realtime* realtime, float deltaTime);
};

