#pragma once

#include <vector>
#include <tuple>
#include <unordered_map>
#include <glm/glm.hpp>
#include "mapproperties.h"
#include "mapbuilder.h"
#include "Chunk.h"

class Map {
public:
    Map();
    ~Map();
    
    // Initialize from builder (for world builder visualization only)
    void initializeFromBuilder(const MapBuilder& builder);
    
    // Set noise parameters for procedural generation
    void setNoiseParams(const MapBuilderParams& params);
    const MapBuilderParams& getNoiseParams() const { return m_noiseParams; }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_depth; }
    int getMaxHeight() const { return m_maxHeight; }
    
    bool hasBlock(int x, int y, int z) const;
    BiomeType getBiomeAt(int x, int z) const;
    
    std::vector<std::tuple<int, int, int, BiomeType>> getBlocksToRender() const;
    std::vector<std::tuple<int, int, int, BiomeType>> getBlocksInRenderDistance(
        const glm::vec3& cameraPos, int renderDistance);
    
    int getBlockCount() const { return m_blockCount; }
    int getChunkSize() const { return m_chunkSize; }
    
    // Enable/disable endless generation mode
    void setEndlessMode(bool enabled) { m_endlessMode = enabled; }
    bool isEndlessMode() const { return m_endlessMode; }
    
    const std::unordered_map<int, Chunk*>& getChunks() const { return m_chunks; }
    
    void ensureChunkGenerated(int chunkX, int chunkZ);
    
    // Biome orb collection tracking
    bool hasCompletionCubeBeenCollected(BiomeType biome) const;
    void markCompletionCubeCollected(BiomeType biome);

private:
    void populateBlocks(const MapBuilder& builder);
    
    // Procedural chunk generation
    void generateChunk(int chunkX, int chunkZ);
    float sampleNoise(float x, float y) const;
    float sampleBiomeNoise(float x, float y) const;
    
    // Chunk management
    void unloadDistantChunks(const glm::vec3& cameraPos, int keepDistance);
    
    std::vector<std::vector<BiomeType>> m_blocks;
    std::vector<std::vector<bool>> m_blockExists;
    
    int m_width;
    int m_depth;
    int m_maxHeight;
    int m_blockCount;
    
    int m_centerX;
    int m_centerZ;
    
    int m_chunkSize;
    std::unordered_map<int, Chunk*> m_chunks;
    
    // Noise parameters for procedural generation
    MapBuilderParams m_noiseParams;
    bool m_endlessMode;
    bool m_initializedFromBuilder;
    
    int getChunkKey(int chunkX, int chunkZ) const;
    void populateChunks();
    void clearChunks();
    
    // Track which biome orbs have been collected (prevents regeneration)
    bool m_collectedCompletionCubes[3]; // Indexed by BiomeType
};

