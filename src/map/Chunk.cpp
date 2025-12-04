#include "Chunk.h"

Chunk::Chunk(int chunkX, int chunkZ, int chunkSize)
    : m_chunkX(chunkX)
    , m_chunkZ(chunkZ)
    , m_chunkSize(chunkSize)
    , m_populated(false)
{
}

Chunk::~Chunk() {
}

void Chunk::addBlock(int worldX, int worldY, int worldZ, BiomeType biome) {
    m_blocks.push_back(std::make_tuple(worldX, worldY, worldZ, biome));
}

void Chunk::clear() {
    m_blocks.clear();
    m_populated = false;
}

