#include "CompletionCube.h"

CompletionCube::CompletionCube(glm::vec3 position, BiomeType biome)
    : m_position(position), m_biome(biome), m_collected(false)
{
}

glm::vec3 CompletionCube::getColor() const {
    // RGB colors for each biome
    switch (m_biome) {
        case BIOME_MOUNTAINS:
            return glm::vec3(0.0f, 0.5f, 1.0f); // Bright blue
            
        case BIOME_FOREST:
            return glm::vec3(0.0f, 1.0f, 0.0f); // Pure green
            
        case BIOME_FIELD:
            return glm::vec3(1.0f, 0.0f, 0.0f); // Bright red
            
        default:
            return glm::vec3(1.0f, 1.0f, 1.0f); // White fallback
    }
}

