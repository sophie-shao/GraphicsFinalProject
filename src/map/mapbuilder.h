#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "mapproperties.h"

struct MapBuilderParams {
    int seed = 0;
    float frequency = 0.01f;
    int octaves = 4;
    float amplitude = 1.0f;
    float persistence = 0.5f;
    float biomeFrequency = 0.005f;
    int biomeOctaves = 3;
    float biomeWarp = 50.0f;
    int mapWidth = 100;
    int mapHeight = 100;
};

class MapBuilder {
public:
    MapBuilder();
    
    void setParams(const MapBuilderParams& params);
    void setSeed(int seed);
    void setFrequency(float frequency);
    void setOctaves(int octaves);
    void setAmplitude(float amplitude);
    void setPersistence(float persistence);
    void setBiomeFrequency(float frequency);
    void setBiomeOctaves(int octaves);
    void setBiomeWarp(float warp);
    void setMapSize(int width, int height);
    
    void generateBiomeMap();
    void generateHeightMap();
    
    const std::vector<BiomeType>& getBiomes() const { return m_biomes; }
    const std::vector<float>& getNormalizedHeights() const { return m_normalizedHeights; }
    const std::vector<float>& getBiomeNoiseValues() const { return m_biomeNoiseValues; }
    
    int getMapWidth() const { return m_params.mapWidth; }
    int getMapHeight() const { return m_params.mapHeight; }
    const MapBuilderParams& getParams() const { return m_params; }
    
private:
    float sampleNoise(float x, float y);
    float sampleBiomeNoise(float x, float y);
    
    MapBuilderParams m_params;
    std::vector<BiomeType> m_biomes;
    std::vector<float> m_normalizedHeights;
    std::vector<float> m_biomeNoiseValues;
};

