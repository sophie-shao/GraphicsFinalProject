#pragma once

#include <vector>
#include <glm/glm.hpp>

class Realtime;
struct SceneLightData;

class Rendering {
public:
    static void renderMapBlocks(Realtime* realtime);
    static void setupMapLights(Realtime* realtime, const glm::vec3& cameraPos, std::vector<SceneLightData>& lights);
};

