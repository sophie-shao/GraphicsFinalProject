#pragma once

enum BiomeType {
    BIOME_FIELD = 0,
    BIOME_MOUNTAINS = 1,
    BIOME_FOREST = 2
};

class MapProperties {
public:
    static constexpr int NUM_BIOMES = 3;
    
    static void getBiomeColor(BiomeType biome, unsigned char& r, unsigned char& g, unsigned char& b);
    static BiomeType getBiomeFromHeight(float normalizedHeight);
    static BiomeType getBiomeFromNoise(float normalizedNoise);
};

