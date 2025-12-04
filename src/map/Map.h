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
    
    void initializeFromBuilder(const MapBuilder& builder);
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_depth; }
    int getMaxHeight() const { return m_maxHeight; }
    
    bool hasBlock(int x, int y, int z) const;
    BiomeType getBiomeAt(int x, int z) const;
    
    std::vector<std::tuple<int, int, int, BiomeType>> getBlocksToRender() const;
    std::vector<std::tuple<int, int, int, BiomeType>> getBlocksInRenderDistance(
        const glm::vec3& cameraPos, int renderDistance) const;
    
    int getBlockCount() const { return m_blockCount; }
    int getChunkSize() const { return m_chunkSize; }

private:
    void populateBlocks(const MapBuilder& builder);
    
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
    
    int getChunkKey(int chunkX, int chunkZ) const;
    void populateChunks();
    void clearChunks();
};

