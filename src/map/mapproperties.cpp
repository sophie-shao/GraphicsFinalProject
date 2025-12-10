#include "mapproperties.h"

void MapProperties::getBiomeColor(BiomeType biome, unsigned char& r, unsigned char& g, unsigned char& b)
{
    switch (biome) {
        case BIOME_FIELD:
            r = 220; // Darker yellow
            g = 200; // Darker yellow
            b = 0;
            break;
        case BIOME_MOUNTAINS:
            r = 0;
            g = 0;
            b = 255;
            break;
        case BIOME_FOREST:
            r = 0;
            g = 255;
            b = 0;
            break;
    }
}

BiomeType MapProperties::getBiomeFromHeight(float normalizedHeight)
{
    if (normalizedHeight < 0.33f) {
        return BIOME_FIELD;
    } else if (normalizedHeight < 0.66f) {
        return BIOME_FOREST;
    } else {
        return BIOME_MOUNTAINS;
    }
}

BiomeType MapProperties::getBiomeFromNoise(float normalizedNoise)
{
    if (normalizedNoise < 0.33f) {
        return BIOME_FIELD;
    } else if (normalizedNoise < 0.66f) {
        return BIOME_FOREST;
    } else {
        return BIOME_MOUNTAINS;
    }
}

