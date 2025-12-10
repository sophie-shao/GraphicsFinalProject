#pragma once

#include <vector>
#include <tuple>
#include "mapproperties.h"
#include "Tree.h"
#include "CompletionCube.h"

class Chunk {
public:
    Chunk(int chunkX, int chunkZ, int chunkSize = 16);
    ~Chunk();
    
    int getChunkX() const { return m_chunkX; }
    int getChunkZ() const { return m_chunkZ; }
    int getChunkSize() const { return m_chunkSize; }
    
    bool isPopulated() const { return m_populated; }
    void setPopulated(bool populated) { m_populated = populated; }
    
    const std::vector<std::tuple<int, int, int, BiomeType>>& getBlocks() const { return m_blocks; }
    const std::vector<Tree>& getTrees() const { return m_trees; }
    const std::vector<CompletionCube>& getCompletionCubes() const { return m_completionCubes; }
    std::vector<CompletionCube>& getCompletionCubesMutable() { return m_completionCubes; }
    
    void addBlock(int worldX, int worldY, int worldZ, BiomeType biome);
    void addTree(const Tree& tree);
    void addCompletionCube(const CompletionCube& completionCube);
    void clear();

private:
    int m_chunkX;
    int m_chunkZ;
    int m_chunkSize;
    bool m_populated;
    
    std::vector<std::tuple<int, int, int, BiomeType>> m_blocks;
    std::vector<Tree> m_trees;
    std::vector<CompletionCube> m_completionCubes;
};

