#pragma once

#include <vector>
#include <glm/glm.hpp>

class Realtime;
struct SceneLightData;

class Rendering {
public:
    static void setupMapLights(Realtime* realtime, const glm::vec3& camPos, std::vector<SceneLightData>& lights);
    static void renderMapBlocks(Realtime* realtime);
    static void renderMapBlocksWithBumpMapping(Realtime* realtime);
    static void addLightsToBlockShader(Realtime* realtime, const std::vector<SceneLightData>& lights);
};
