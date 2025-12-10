#include "fog.h"
#include "../realtime.h"
#include "map/Map.h"

namespace FogSystem {
    
    glm::vec3 getBiomeFogColor(BiomeType biome) {
            switch (biome) {
                case BIOME_FIELD:
                    // Dark orange/red - desert atmosphere
                    return glm::vec3(0.4f, 0.25f, 0.15f); // TUNE
                
            case BIOME_MOUNTAINS: //now its void
                // Dark red almost black - ominous mountain atmosphere
                return glm::vec3(0.05f, 0.05f, 0.15f); // TUNE
                
            case BIOME_FOREST:
                // Light blue-gray - misty forest atmosphere
                return glm::vec3(0.4f, 0.45f, 0.5f); // TUNE
                
            default:
                return glm::vec3(0.05f, 0.05f, 0.15f); // Default dark blue
        }
    }
    
    void updateFogColor(Realtime* realtime, float deltaTime) {
        if (realtime->m_activeMap == nullptr || !realtime->m_fogEnabled) {
            return;
        }
        
        glm::vec3 cameraPos = realtime->m_camera.getPosition();
        int playerX = static_cast<int>(std::floor(cameraPos.x));
        int playerZ = static_cast<int>(std::floor(cameraPos.z));
        BiomeType currentBiome = realtime->m_activeMap->getBiomeAt(playerX, playerZ);
        
        glm::vec3 targetFogColor = getBiomeFogColor(currentBiome);
        realtime->m_targetFogColor = targetFogColor;
        
        //transition fog color towards target
        glm::vec3 colorDiff = targetFogColor - realtime->m_fogColor;
        float distance = glm::length(colorDiff);
        if (distance > 0.001f) {
            float maxChange = realtime->m_fogTransitionSpeed * deltaTime;
            if (distance > maxChange) {
                realtime->m_fogColor += glm::normalize(colorDiff) * maxChange;
            } else {
                realtime->m_fogColor = targetFogColor;
            }
        }
    }
}

