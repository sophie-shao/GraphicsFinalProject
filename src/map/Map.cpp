#include "Map.h"
#include "Chunk.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <stdexcept>
#include <limits>

Map::Map() 
    : m_width(0)
    , m_depth(0)
    , m_maxHeight(0)
    , m_blockCount(0)
    , m_centerX(0)
    , m_centerZ(0)
    , m_chunkSize(16)
{
}

Map::~Map() {
    clearChunks();
}

int Map::getChunkKey(int chunkX, int chunkZ) const {
    constexpr int MAX_CHUNK_COORD = 10000;
    constexpr int MIN_CHUNK_COORD = -10000;
    
    if (chunkX < MIN_CHUNK_COORD || chunkX > MAX_CHUNK_COORD ||
        chunkZ < MIN_CHUNK_COORD || chunkZ > MAX_CHUNK_COORD) {
        throw std::out_of_range("Chunk coordinates out of valid range");
    }
    
    long long key = static_cast<long long>(chunkX) * 10000LL + static_cast<long long>(chunkZ);
    
    if (key < std::numeric_limits<int>::min() || key > std::numeric_limits<int>::max()) {
        throw std::overflow_error("Chunk key calculation overflow");
    }
    
    return static_cast<int>(key);
}

void Map::clearChunks() {
    for (auto& pair : m_chunks) {
        delete pair.second;
    }
    m_chunks.clear();
}

void Map::initializeFromBuilder(const MapBuilder& builder) {
    int builderWidth = builder.getMapWidth();
    int builderHeight = builder.getMapHeight();
    
    if (builderWidth <= 0 || builderHeight <= 0) {
        throw std::invalid_argument("MapBuilder has invalid dimensions");
    }
    
    const auto& normalizedHeights = builder.getNormalizedHeights();
    const auto& biomes = builder.getBiomes();
    
    if (normalizedHeights.empty() || biomes.empty()) {
        throw std::runtime_error("MapBuilder data arrays are empty - generate maps first");
    }
    
    long long expectedSize = static_cast<long long>(builderWidth) * static_cast<long long>(builderHeight);
    if (static_cast<long long>(normalizedHeights.size()) != expectedSize ||
        static_cast<long long>(biomes.size()) != expectedSize) {
        throw std::runtime_error("MapBuilder data size mismatch with dimensions");
    }
    
    m_blocks.clear();
    m_blockExists.clear();
    clearChunks();
    m_blockCount = 0;
    
    populateBlocks(builder);
    populateChunks();
}

void Map::populateBlocks(const MapBuilder& builder) {
    const auto& normalizedHeights = builder.getNormalizedHeights();
    const auto& biomes = builder.getBiomes();
    
    if (normalizedHeights.empty() || biomes.empty()) {
        throw std::runtime_error("Cannot populate blocks: input data is empty");
    }
    
    m_width = builder.getMapWidth();
    m_depth = builder.getMapHeight();
    
    if (m_width <= 0 || m_depth <= 0) {
        throw std::runtime_error("Cannot populate blocks: invalid map dimensions");
    }
    
    if (m_width > 10000 || m_depth > 10000) {
        throw std::runtime_error("Cannot populate blocks: map dimensions too large");
    }
    
    long long expectedSize = static_cast<long long>(m_width) * static_cast<long long>(m_depth);
    if (static_cast<long long>(normalizedHeights.size()) != expectedSize ||
        static_cast<long long>(biomes.size()) != expectedSize) {
        throw std::runtime_error("Cannot populate blocks: data size mismatch");
    }
    
    m_centerX = m_width / 2;
    m_centerZ = m_depth / 2;
    
    float maxHeight = 0.0f;
    for (float h : normalizedHeights) {
        if (!std::isfinite(h)) {
            continue;
        }
        maxHeight = std::max(maxHeight, h);
    }
    
    if (!std::isfinite(maxHeight) || maxHeight < 0.0f) {
        maxHeight = 0.0f;
    }
    maxHeight = std::min(1.0f, maxHeight);
    
    int maxDimension = std::max(m_width, m_depth);
    if (maxDimension <= 0) {
        throw std::runtime_error("Cannot populate blocks: invalid max dimension");
    }
    
    m_maxHeight = static_cast<int>(std::round(maxHeight * maxDimension * 0.1f)) + 1;
    
    if (m_maxHeight <= 0) {
        m_maxHeight = 1;
    }
    
    if (m_maxHeight > 10000) {
        throw std::runtime_error("Cannot populate blocks: calculated max height too large");
    }
    
    long long arraySizePerZ = static_cast<long long>(m_maxHeight) * static_cast<long long>(m_width);
    constexpr long long MAX_SAFE_ARRAY_SIZE = 100000000LL;
    if (arraySizePerZ > MAX_SAFE_ARRAY_SIZE) {
        throw std::runtime_error("Cannot populate blocks: array size would exceed maximum safe size");
    }
    
    try {
    m_blocks.resize(m_depth);
    m_blockExists.resize(m_depth);
    for (int z = 0; z < m_depth; z++) {
            m_blocks[z].resize(static_cast<size_t>(arraySizePerZ), BIOME_FIELD);
            m_blockExists[z].resize(static_cast<size_t>(arraySizePerZ), false);
        }
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Cannot populate blocks: memory allocation failed");
    }
    
    m_blockCount = 0;
    for (int z = 0; z < m_depth; z++) {
        for (int x = 0; x < m_width; x++) {
            int index = z * m_width + x;
            
            if (index < 0 || index >= static_cast<int>(normalizedHeights.size()) ||
                index >= static_cast<int>(biomes.size())) {
                continue;
            }
            
            float height = normalizedHeights[index];
            BiomeType biome = biomes[index];
            
            if (!std::isfinite(height)) {
                continue;
            }
            height = std::max(0.0f, std::min(1.0f, height));
            
            if (biome < BIOME_FIELD || biome > BIOME_FOREST) {
                biome = BIOME_FIELD;
            }
            
            float worldYFloat = height * maxDimension * 0.1f;
            
            if (!std::isfinite(worldYFloat)) {
                continue;
            }
            
            int worldY = static_cast<int>(std::round(worldYFloat));
            
            if (worldY < 0 || worldY >= m_maxHeight) {
                continue;
            }
            
            long long arrayIndex = static_cast<long long>(worldY) * static_cast<long long>(m_width) + static_cast<long long>(x);
            
            if (arrayIndex < 0 || arrayIndex >= arraySizePerZ) {
                continue;
            }
            
            if (z < 0 || z >= m_depth) {
                continue;
            }
            
            size_t arrayIndexSize = static_cast<size_t>(arrayIndex);
            if (arrayIndexSize < m_blocks[z].size() && arrayIndexSize < m_blockExists[z].size()) {
                m_blocks[z][arrayIndexSize] = biome;
                m_blockExists[z][arrayIndexSize] = true;
                m_blockCount++;
            }
        }
    }
}

bool Map::hasBlock(int x, int y, int z) const {
    if (m_width <= 0 || m_depth <= 0 || m_maxHeight <= 0) {
        return false;
    }
    
    if (m_blocks.empty() || m_blockExists.empty()) {
        return false;
    }
    
    long long arrayX = static_cast<long long>(x) + static_cast<long long>(m_centerX);
    long long arrayZ = static_cast<long long>(z) + static_cast<long long>(m_centerZ);
    
    if (arrayX < 0 || arrayX >= m_width || arrayZ < 0 || arrayZ >= m_depth) {
        return false;
    }
    
    if (y < 0 || y >= m_maxHeight) {
        return false;
    }
    
    if (arrayZ < 0 || arrayZ >= static_cast<long long>(m_blockExists.size())) {
        return false;
    }
    
    long long arrayIndex = static_cast<long long>(y) * static_cast<long long>(m_width) + arrayX;
    
    if (arrayIndex < 0) {
        return false;
    }
    
    size_t arrayIndexSize = static_cast<size_t>(arrayIndex);
    size_t arrayZSize = static_cast<size_t>(arrayZ);
    
    if (arrayZSize >= m_blockExists.size() || arrayIndexSize >= m_blockExists[arrayZSize].size()) {
        return false;
    }
    
    return m_blockExists[arrayZSize][arrayIndexSize];
}

BiomeType Map::getBiomeAt(int x, int z) const {
    if (m_width <= 0 || m_depth <= 0 || m_maxHeight <= 0) {
        return BIOME_FIELD;
    }
    
    if (m_blocks.empty() || m_blockExists.empty()) {
        return BIOME_FIELD;
    }
    
    long long arrayX = static_cast<long long>(x) + static_cast<long long>(m_centerX);
    long long arrayZ = static_cast<long long>(z) + static_cast<long long>(m_centerZ);
    
    if (arrayX < 0 || arrayX >= m_width || arrayZ < 0 || arrayZ >= m_depth) {
        return BIOME_FIELD;
    }
    
    size_t arrayZSize = static_cast<size_t>(arrayZ);
    if (arrayZSize >= m_blockExists.size() || arrayZSize >= m_blocks.size()) {
        return BIOME_FIELD;
    }
    
    for (int y = m_maxHeight - 1; y >= 0; y--) {
        long long arrayIndex = static_cast<long long>(y) * static_cast<long long>(m_width) + arrayX;
        
        if (arrayIndex < 0) {
            continue;
        }
        
        size_t arrayIndexSize = static_cast<size_t>(arrayIndex);
        
        if (arrayIndexSize >= m_blockExists[arrayZSize].size() ||
            arrayIndexSize >= m_blocks[arrayZSize].size()) {
            continue;
        }
        
        if (m_blockExists[arrayZSize][arrayIndexSize]) {
            BiomeType biome = m_blocks[arrayZSize][arrayIndexSize];
            if (biome >= BIOME_FIELD && biome <= BIOME_FOREST) {
                return biome;
    }
            return BIOME_FIELD;
        }
    }
    
    return BIOME_FIELD;
}

std::vector<std::tuple<int, int, int, BiomeType>> Map::getBlocksToRender() const {
    std::vector<std::tuple<int, int, int, BiomeType>> blocks;
    
    if (m_width <= 0 || m_depth <= 0 || m_maxHeight <= 0) {
        return blocks;
    }
    
    if (m_blocks.empty() || m_blockExists.empty()) {
        return blocks;
    }
    
    try {
        blocks.reserve(static_cast<size_t>(m_width) * static_cast<size_t>(m_depth));
    } catch (const std::bad_alloc& e) {
    }
    
    for (int z = 0; z < m_depth; z++) {
        if (z < 0 || z >= static_cast<int>(m_blockExists.size()) ||
            z >= static_cast<int>(m_blocks.size())) {
            continue;
        }
        
        for (int x = 0; x < m_width; x++) {
            for (int y = m_maxHeight - 1; y >= 0; y--) {
                long long arrayIndex = static_cast<long long>(y) * static_cast<long long>(m_width) + static_cast<long long>(x);
                
                if (arrayIndex < 0) {
                    continue;
                }
                
                size_t arrayIndexSize = static_cast<size_t>(arrayIndex);
                
                if (arrayIndexSize >= m_blockExists[z].size() ||
                    arrayIndexSize >= m_blocks[z].size()) {
                    continue;
                }
                
                if (m_blockExists[z][arrayIndexSize]) {
                    BiomeType biome = m_blocks[z][arrayIndexSize];
                    
                    if (biome < BIOME_FIELD || biome > BIOME_FOREST) {
                        biome = BIOME_FIELD;
                    }
                    
                    long long worldX = static_cast<long long>(x) - static_cast<long long>(m_centerX);
                    long long worldZ = static_cast<long long>(z) - static_cast<long long>(m_centerZ);
                    int worldY = y;
                    
                    if (worldX < std::numeric_limits<int>::min() || worldX > std::numeric_limits<int>::max() ||
                        worldZ < std::numeric_limits<int>::min() || worldZ > std::numeric_limits<int>::max()) {
                        continue;
                    }
                    
                    try {
                        blocks.push_back(std::make_tuple(static_cast<int>(worldX), worldY, static_cast<int>(worldZ), biome));
                    } catch (const std::bad_alloc& e) {
                        return blocks;
                    }
                    break;
                }
            }
        }
    }
    
    return blocks;
}

void Map::populateChunks() {
    if (m_chunkSize <= 0) {
        throw std::runtime_error("Cannot populate chunks: invalid chunk size");
    }
    
    if (m_width <= 0 || m_depth <= 0) {
        throw std::runtime_error("Cannot populate chunks: map not initialized");
    }
    
    clearChunks();
    
    auto allBlocks = getBlocksToRender();
    
    for (const auto& block : allBlocks) {
        int worldX, worldY, worldZ;
        BiomeType biome;
        std::tie(worldX, worldY, worldZ, biome) = block;
        
        if (biome < BIOME_FIELD || biome > BIOME_FOREST) {
            continue;
        }
        
        if (!std::isfinite(static_cast<float>(worldX)) || !std::isfinite(static_cast<float>(worldZ))) {
            continue;
        }
        
        if (m_chunkSize == 0) {
            continue;
        }
        
        float chunkXFloat = (static_cast<float>(worldX) + static_cast<float>(m_centerX)) / static_cast<float>(m_chunkSize);
        float chunkZFloat = (static_cast<float>(worldZ) + static_cast<float>(m_centerZ)) / static_cast<float>(m_chunkSize);
        
        if (!std::isfinite(chunkXFloat) || !std::isfinite(chunkZFloat)) {
            continue;
        }
        
        int chunkX = static_cast<int>(std::floor(chunkXFloat));
        int chunkZ = static_cast<int>(std::floor(chunkZFloat));
        
        int chunkKey;
        try {
            chunkKey = getChunkKey(chunkX, chunkZ);
        } catch (const std::exception& e) {
            continue;
        }
        
        Chunk* chunk = nullptr;
        
        auto it = m_chunks.find(chunkKey);
        if (it == m_chunks.end()) {
            try {
            chunk = new Chunk(chunkX, chunkZ, m_chunkSize);
            m_chunks[chunkKey] = chunk;
            } catch (const std::bad_alloc& e) {
                continue;
            } catch (const std::exception& e) {
                continue;
            }
        } else {
            chunk = it->second;
        }
        
        if (chunk == nullptr) {
            continue;
        }
        
        try {
        chunk->addBlock(worldX, worldY, worldZ, biome);
        chunk->setPopulated(true);
        } catch (const std::exception& e) {
            continue;
        }
    }
}

std::vector<std::tuple<int, int, int, BiomeType>> Map::getBlocksInRenderDistance(
    const glm::vec3& cameraPos, int renderDistance) const {
    
    std::vector<std::tuple<int, int, int, BiomeType>> blocks;
    
    if (m_width <= 0 || m_depth <= 0) {
        return blocks;
    }
    
    if (m_chunkSize <= 0) {
        return blocks;
    }
    
    if (renderDistance < 0) {
        return blocks;
    }
    
    if (renderDistance > 100) {
        renderDistance = 100;
    }
    
    if (!std::isfinite(cameraPos.x) || !std::isfinite(cameraPos.y) || !std::isfinite(cameraPos.z)) {
        return blocks;
    }
    
    if (m_chunkSize == 0) {
        return blocks;
    }
    
    float cameraChunkXFloat = (cameraPos.x + static_cast<float>(m_centerX)) / static_cast<float>(m_chunkSize);
    float cameraChunkZFloat = (cameraPos.z + static_cast<float>(m_centerZ)) / static_cast<float>(m_chunkSize);
    
    if (!std::isfinite(cameraChunkXFloat) || !std::isfinite(cameraChunkZFloat)) {
        return blocks;
    }
    
    int cameraChunkX = static_cast<int>(std::floor(cameraChunkXFloat));
    int cameraChunkZ = static_cast<int>(std::floor(cameraChunkZFloat));
    
    for (int dz = -renderDistance; dz <= renderDistance; dz++) {
        for (int dx = -renderDistance; dx <= renderDistance; dx++) {
            int chunkX = cameraChunkX + dx;
            int chunkZ = cameraChunkZ + dz;
            
            int chunkKey;
            try {
                chunkKey = getChunkKey(chunkX, chunkZ);
            } catch (const std::exception& e) {
                continue;
            }
            
            auto it = m_chunks.find(chunkKey);
            
            if (it != m_chunks.end() && it->second != nullptr && it->second->isPopulated()) {
                try {
                const auto& chunkBlocks = it->second->getBlocks();
                blocks.insert(blocks.end(), chunkBlocks.begin(), chunkBlocks.end());
                } catch (const std::bad_alloc& e) {
                    return blocks;
                } catch (const std::exception& e) {
                    continue;
                }
            }
        }
    }
    
    return blocks;
}

