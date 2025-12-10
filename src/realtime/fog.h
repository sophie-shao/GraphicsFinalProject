#pragma once

#include <glm/glm.hpp>
#include "map/mapproperties.h"

// Forward declaration
class Realtime;

namespace FogSystem {
    // Get fog color for a specific biome
    glm::vec3 getBiomeFogColor(BiomeType biome);
    void updateFogColor(Realtime* realtime, float deltaTime);
}

