#pragma once

#include <glm/glm.hpp>
#include "mapproperties.h"

class CompletionCube {
public:
    CompletionCube(glm::vec3 position, BiomeType biome);
    
    glm::vec3 getPosition() const { return m_position; }
    BiomeType getBiome() const { return m_biome; }
    bool isCollected() const { return m_collected; }
    void collect() { m_collected = true; }
    
    glm::vec3 getColor() const;
    
private:
    glm::vec3 m_position;
    BiomeType m_biome;
    bool m_collected;
};



