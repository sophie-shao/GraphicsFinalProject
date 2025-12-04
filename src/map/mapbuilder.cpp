#include "mapbuilder.h"
#include <glm/gtc/noise.hpp>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

MapBuilder::MapBuilder() {
    m_params = MapBuilderParams();
}

void MapBuilder::setParams(const MapBuilderParams& params) {
    if (params.mapWidth <= 0 || params.mapHeight <= 0) {
        throw std::invalid_argument("Map dimensions must be positive");
    }
    
    if (params.mapWidth > 10000 || params.mapHeight > 10000) {
        throw std::invalid_argument("Map dimensions too large (max 10000)");
    }
    
    if (params.octaves <= 0 || params.octaves > 20) {
        throw std::invalid_argument("Octaves must be between 1 and 20");
    }
    
    if (params.biomeOctaves <= 0 || params.biomeOctaves > 20) {
        throw std::invalid_argument("Biome octaves must be between 1 and 20");
    }
    
    if (params.frequency <= 0.0f || !std::isfinite(params.frequency)) {
        throw std::invalid_argument("Frequency must be positive and finite");
    }
    
    if (params.biomeFrequency <= 0.0f || !std::isfinite(params.biomeFrequency)) {
        throw std::invalid_argument("Biome frequency must be positive and finite");
    }
    
    if (params.amplitude < 0.0f || !std::isfinite(params.amplitude)) {
        throw std::invalid_argument("Amplitude must be non-negative and finite");
    }
    
    if (params.persistence < 0.0f || params.persistence > 1.0f || !std::isfinite(params.persistence)) {
        throw std::invalid_argument("Persistence must be between 0 and 1");
    }
    
    if (!std::isfinite(params.biomeWarp)) {
        throw std::invalid_argument("Biome warp must be finite");
    }
    
    m_params = params;
}

void MapBuilder::setSeed(int seed) {
    if (seed < 0) {
        seed = std::abs(seed);
    }
    m_params.seed = seed;
}

void MapBuilder::setFrequency(float frequency) {
    if (frequency <= 0.0f || !std::isfinite(frequency)) {
        throw std::invalid_argument("Frequency must be positive and finite");
    }
    m_params.frequency = frequency;
}

void MapBuilder::setOctaves(int octaves) {
    if (octaves <= 0 || octaves > 20) {
        throw std::invalid_argument("Octaves must be between 1 and 20");
    }
    m_params.octaves = octaves;
}

void MapBuilder::setAmplitude(float amplitude) {
    if (amplitude < 0.0f || !std::isfinite(amplitude)) {
        throw std::invalid_argument("Amplitude must be non-negative and finite");
    }
    m_params.amplitude = amplitude;
}

void MapBuilder::setPersistence(float persistence) {
    if (persistence < 0.0f || persistence > 1.0f || !std::isfinite(persistence)) {
        throw std::invalid_argument("Persistence must be between 0 and 1");
    }
    m_params.persistence = persistence;
}

void MapBuilder::setBiomeFrequency(float frequency) {
    if (frequency <= 0.0f || !std::isfinite(frequency)) {
        throw std::invalid_argument("Biome frequency must be positive and finite");
    }
    m_params.biomeFrequency = frequency;
}

void MapBuilder::setBiomeOctaves(int octaves) {
    if (octaves <= 0 || octaves > 20) {
        throw std::invalid_argument("Biome octaves must be between 1 and 20");
    }
    m_params.biomeOctaves = octaves;
}

void MapBuilder::setBiomeWarp(float warp) {
    if (!std::isfinite(warp)) {
        throw std::invalid_argument("Biome warp must be finite");
    }
    m_params.biomeWarp = warp;
}

void MapBuilder::setMapSize(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Map dimensions must be positive");
    }
    
    if (width > 10000 || height > 10000) {
        throw std::invalid_argument("Map dimensions too large (max 10000)");
    }
    
    constexpr int MAX_SAFE_ARRAY_SIZE = 100000000;
    if (static_cast<long long>(width) * static_cast<long long>(height) > MAX_SAFE_ARRAY_SIZE) {
        throw std::invalid_argument("Map size would exceed maximum safe array size");
    }
    
    m_params.mapWidth = width;
    m_params.mapHeight = height;
}

float MapBuilder::sampleNoise(float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        return 0.0f;
    }
    
    if (m_params.octaves <= 0 || m_params.octaves > 20) {
        return 0.0f;
    }
    
    if (m_params.frequency <= 0.0f || !std::isfinite(m_params.frequency)) {
        return 0.0f;
    }
    
    if (m_params.amplitude < 0.0f || !std::isfinite(m_params.amplitude)) {
        return 0.0f;
    }
    
    if (m_params.persistence < 0.0f || m_params.persistence > 1.0f || !std::isfinite(m_params.persistence)) {
        return 0.0f;
    }
    
    float value = 0.0f;
    float amplitude = m_params.amplitude;
    float frequency = m_params.frequency;
    
    float seedOffsetX = static_cast<float>(m_params.seed % 10000) * 0.1f;
    float seedOffsetY = static_cast<float>((m_params.seed / 10000) % 10000) * 0.1f;
    
    const float MAX_FREQUENCY = 1e6f;
    
    for (int i = 0; i < m_params.octaves; i++) {
        if (frequency > MAX_FREQUENCY) {
            break;
        }
        
        glm::vec2 pos(x * frequency + seedOffsetX, y * frequency + seedOffsetY);
        
        if (!std::isfinite(pos.x) || !std::isfinite(pos.y)) {
            break;
        }
        
        float noise = glm::perlin(pos);
        
        if (!std::isfinite(noise)) {
            break;
        }
        
        value += noise * amplitude;
        
        if (!std::isfinite(value)) {
            value = 0.0f;
            break;
        }
        
        amplitude *= m_params.persistence;
        frequency *= 2.0f;
    }
    
    if (!std::isfinite(value)) {
        return 0.0f;
    }
    
    return value;
}

float MapBuilder::sampleBiomeNoise(float x, float y) {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        return 0.0f;
    }
    
    if (m_params.biomeOctaves <= 0 || m_params.biomeOctaves > 20) {
        return 0.0f;
    }
    
    if (m_params.biomeFrequency <= 0.0f || !std::isfinite(m_params.biomeFrequency)) {
        return 0.0f;
    }
    
    if (!std::isfinite(m_params.biomeWarp)) {
        return 0.0f;
    }
    
    float seedOffsetX = static_cast<float>(m_params.seed % 10000) * 0.1f;
    float seedOffsetY = static_cast<float>((m_params.seed / 10000) % 10000) * 0.1f;
    
    float warpStrength = m_params.biomeWarp * 0.01f;
    
    const float MAX_WARP_STRENGTH = 1000.0f;
    if (warpStrength > MAX_WARP_STRENGTH) {
        warpStrength = MAX_WARP_STRENGTH;
    }
    
    glm::vec2 warpPos((x + seedOffsetX) * 0.01f, (y + seedOffsetY) * 0.01f);
    
    if (!std::isfinite(warpPos.x) || !std::isfinite(warpPos.y)) {
        return 0.0f;
    }
    
    float warpX = glm::perlin(warpPos + glm::vec2(seedOffsetX * 0.5f, 0.0f)) * warpStrength;
    float warpY = glm::perlin(warpPos + glm::vec2(0.0f, seedOffsetY * 0.5f)) * warpStrength;
    
    if (!std::isfinite(warpX)) warpX = 0.0f;
    if (!std::isfinite(warpY)) warpY = 0.0f;
    
    float warpedX = x + warpX;
    float warpedY = y + warpY;
    
    if (!std::isfinite(warpedX) || !std::isfinite(warpedY)) {
        return 0.0f;
    }
    
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = m_params.biomeFrequency;
    float persistence = 0.5f;
    
    const float MAX_FREQUENCY = 1e6f;
    
    for (int i = 0; i < m_params.biomeOctaves; i++) {
        if (frequency > MAX_FREQUENCY) {
            break;
        }
        
        glm::vec2 pos((warpedX + seedOffsetX) * frequency, (warpedY + seedOffsetY) * frequency);
        
        if (!std::isfinite(pos.x) || !std::isfinite(pos.y)) {
            break;
        }
        
        float noise = glm::perlin(pos);
        
        if (!std::isfinite(noise)) {
            break;
        }
        
        value += noise * amplitude;
        
        if (!std::isfinite(value)) {
            value = 0.0f;
            break;
        }
        
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    if (std::abs(1.0f - persistence) < 0.0001f) {
        value /= static_cast<float>(m_params.biomeOctaves);
    } else {
    float maxValue = (1.0f - std::pow(persistence, m_params.biomeOctaves)) / (1.0f - persistence);
        
        if (!std::isfinite(maxValue) || maxValue < 0.0001f) {
            value /= static_cast<float>(m_params.biomeOctaves);
        } else {
        value /= maxValue;
        }
    }
    
    if (!std::isfinite(value)) {
        return 0.0f;
    }
    
    return value;
}

void MapBuilder::generateBiomeMap() {
    if (m_params.mapWidth <= 0 || m_params.mapHeight <= 0) {
        throw std::runtime_error("Cannot generate biome map: invalid map dimensions");
    }
    
    if (m_params.mapWidth > 10000 || m_params.mapHeight > 10000) {
        throw std::runtime_error("Cannot generate biome map: map dimensions too large");
    }
    
    constexpr int MAX_SAFE_ARRAY_SIZE = 100000000;
    long long arraySize = static_cast<long long>(m_params.mapWidth) * static_cast<long long>(m_params.mapHeight);
    if (arraySize > MAX_SAFE_ARRAY_SIZE) {
        throw std::runtime_error("Cannot generate biome map: array size would exceed maximum safe size");
    }
    
    try {
        m_biomes.resize(static_cast<size_t>(arraySize));
        m_biomeNoiseValues.resize(static_cast<size_t>(arraySize));
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Cannot generate biome map: memory allocation failed");
    }
    
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();
    
    for (int y = 0; y < m_params.mapHeight; y++) {
        for (int x = 0; x < m_params.mapWidth; x++) {
            int index = y * m_params.mapWidth + x;
            
            if (index < 0 || index >= static_cast<int>(m_biomeNoiseValues.size())) {
                continue;
            }
            
            float noise = sampleBiomeNoise(static_cast<float>(x), static_cast<float>(y));
            
            if (!std::isfinite(noise)) {
                noise = 0.0f;
            }
            
            m_biomeNoiseValues[index] = noise;
            minVal = std::min(minVal, noise);
            maxVal = std::max(maxVal, noise);
        }
    }
    
    if (!std::isfinite(minVal) || !std::isfinite(maxVal)) {
        throw std::runtime_error("Cannot generate biome map: invalid noise values generated");
    }
    
    float range = maxVal - minVal;
    if (range < 0.0001f) {
        range = 1.0f;
    }
    
    if (range <= 0.0f || !std::isfinite(range)) {
        range = 1.0f;
    }
    
    for (int y = 0; y < m_params.mapHeight; y++) {
        for (int x = 0; x < m_params.mapWidth; x++) {
            int index = y * m_params.mapWidth + x;
            
            if (index < 0 || index >= static_cast<int>(m_biomeNoiseValues.size()) ||
                index >= static_cast<int>(m_biomes.size())) {
                continue;
            }
            
            float normalized = (m_biomeNoiseValues[index] - minVal) / range;
            
            normalized = std::max(0.0f, std::min(1.0f, normalized));
            if (!std::isfinite(normalized)) {
                normalized = 0.0f;
            }
            
            m_biomeNoiseValues[index] = normalized;
            m_biomes[index] = MapProperties::getBiomeFromNoise(normalized);
        }
    }
}

void MapBuilder::generateHeightMap() {
    if (m_params.mapWidth <= 0 || m_params.mapHeight <= 0) {
        throw std::runtime_error("Cannot generate height map: invalid map dimensions");
    }
    
    if (m_params.mapWidth > 10000 || m_params.mapHeight > 10000) {
        throw std::runtime_error("Cannot generate height map: map dimensions too large");
    }
    
    if (m_biomeNoiseValues.empty()) {
        throw std::runtime_error("Cannot generate height map: biome map must be generated first");
    }
    
    long long expectedSize = static_cast<long long>(m_params.mapWidth) * static_cast<long long>(m_params.mapHeight);
    if (static_cast<long long>(m_biomeNoiseValues.size()) != expectedSize) {
        throw std::runtime_error("Cannot generate height map: biome map size mismatch");
    }
    
    constexpr int MAX_SAFE_ARRAY_SIZE = 100000000;
    if (expectedSize > MAX_SAFE_ARRAY_SIZE) {
        throw std::runtime_error("Cannot generate height map: array size would exceed maximum safe size");
    }
    
    try {
        m_normalizedHeights.resize(static_cast<size_t>(expectedSize));
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Cannot generate height map: memory allocation failed");
    }
    
    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();
    
    auto smoothstep = [](float t) -> float {
        t = std::max(0.0f, std::min(1.0f, t));
        if (!std::isfinite(t)) {
            return 0.0f;
        }
        float result = t * t * (3.0f - 2.0f * t);
        return std::isfinite(result) ? result : 0.0f;
    };
    
    std::vector<float> noiseValues;
    try {
        noiseValues.resize(static_cast<size_t>(expectedSize));
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Cannot generate height map: memory allocation failed for noise values");
    }
    
    for (int y = 0; y < m_params.mapHeight; y++) {
        for (int x = 0; x < m_params.mapWidth; x++) {
            int index = y * m_params.mapWidth + x;
            
            if (index < 0 || index >= static_cast<int>(noiseValues.size()) ||
                index >= static_cast<int>(m_biomeNoiseValues.size())) {
                continue;
            }
            
            float baseNoise = sampleNoise(static_cast<float>(x), static_cast<float>(y));
            
            if (!std::isfinite(baseNoise)) {
                baseNoise = 0.0f;
            }
            
            float biomeValue = m_biomeNoiseValues[index];
            
            if (!std::isfinite(biomeValue)) {
                biomeValue = 0.0f;
            }
            biomeValue = std::max(0.0f, std::min(1.0f, biomeValue));
            
            float fieldScale = 0.8f;
            float fieldOffset = 0.1f;
            float forestScale = 0.2f;
            float forestOffset = 0.3f;
            float mountainScale = 0.4f;
            float mountainOffset = 0.6f;
            
            float scale, offset;
            if (biomeValue < 0.5f) {
                float t = smoothstep(biomeValue * 2.0f);
                scale = fieldScale + (forestScale - fieldScale) * t;
                offset = fieldOffset + (forestOffset - fieldOffset) * t;
            } else {
                float t = smoothstep((biomeValue - 0.5f) * 2.0f);
                scale = forestScale + (mountainScale - forestScale) * t;
                offset = forestOffset + (mountainOffset - forestOffset) * t;
            }
            
            if (!std::isfinite(scale)) scale = 0.5f;
            if (!std::isfinite(offset)) offset = 0.3f;
            
            float influencedNoise = baseNoise * scale + offset;
            
            if (!std::isfinite(influencedNoise)) {
                influencedNoise = 0.3f;
            }
            
            noiseValues[index] = influencedNoise;
            minVal = std::min(minVal, influencedNoise);
            maxVal = std::max(maxVal, influencedNoise);
        }
    }
    
    if (!std::isfinite(minVal) || !std::isfinite(maxVal)) {
        throw std::runtime_error("Cannot generate height map: invalid noise values generated");
    }
    
    float range = maxVal - minVal;
    if (range < 0.0001f || !std::isfinite(range)) {
        range = 1.0f;
    }
    
    int maxDimension = std::max(m_params.mapWidth, m_params.mapHeight);
    if (maxDimension <= 0) {
        throw std::runtime_error("Cannot generate height map: invalid max dimension");
    }
    
    int numHeightLevels = maxDimension;
    
    if (numHeightLevels <= 0) {
        numHeightLevels = 1;
    }
    
    for (int y = 0; y < m_params.mapHeight; y++) {
        for (int x = 0; x < m_params.mapWidth; x++) {
            int index = y * m_params.mapWidth + x;
            
            if (index < 0 || index >= static_cast<int>(noiseValues.size()) ||
                index >= static_cast<int>(m_normalizedHeights.size())) {
                continue;
            }
            
            float normalized = (noiseValues[index] - minVal) / range;
            
            if (!std::isfinite(normalized)) {
                normalized = 0.0f;
            }
            normalized = std::max(0.0f, std::min(1.0f, normalized));
            
            int heightLevel = static_cast<int>(std::round(normalized * (numHeightLevels - 1)));
            heightLevel = std::max(0, std::min(numHeightLevels - 1, heightLevel));
            
            if (numHeightLevels > 1) {
                m_normalizedHeights[index] = static_cast<float>(heightLevel) / static_cast<float>(numHeightLevels - 1);
            } else {
                m_normalizedHeights[index] = 0.0f;
            }
            
            if (!std::isfinite(m_normalizedHeights[index])) {
                m_normalizedHeights[index] = 0.0f;
            }
        }
    }
}

