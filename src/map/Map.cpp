#include "Map.h"
#include "Chunk.h"
#include "Tree.h"
#include "CompletionCube.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <map>
#include <stdexcept>
#include <limits>
#include <iostream>
#include <glm/gtc/noise.hpp>
#include <random>

Map::Map() 
    : m_width(0)
    , m_depth(0)
    , m_maxHeight(0)
    , m_blockCount(0)
    , m_centerX(0)
    , m_centerZ(0)
    , m_chunkSize(16)
    , m_endlessMode(true)
    , m_initializedFromBuilder(false)
{
    // Initialize with default noise parameters
    m_noiseParams = MapBuilderParams();
    
    // Initialize collected orbs tracking (none collected initially)
    m_collectedCompletionCubes[BIOME_FIELD] = false;
    m_collectedCompletionCubes[BIOME_MOUNTAINS] = false;
    m_collectedCompletionCubes[BIOME_FOREST] = false;
}

Map::~Map() {
    clearChunks();
}

int Map::getChunkKey(int chunkX, int chunkZ) const {
    constexpr int MAX_CHUNK_COORD = 10000;
    constexpr int MIN_CHUNK_COORD = -10000;
    
    if (chunkX < MIN_CHUNK_COORD || chunkX > MAX_CHUNK_COORD ||
        chunkZ < MIN_CHUNK_COORD || chunkZ > MAX_CHUNK_COORD) {
        throw std::out_of_range("Chunk coordinates out of range");
    }
    
    long long key = static_cast<long long>(chunkX) * 10000LL + static_cast<long long>(chunkZ);
    
    if (key < std::numeric_limits<int>::min() || key > std::numeric_limits<int>::max()) {
        throw std::overflow_error("Chunk key overflow");
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
        throw std::invalid_argument("Invalid map dimensions");
    }
    
    const auto& normalizedHeights = builder.getNormalizedHeights();
    const auto& biomes = builder.getBiomes();
    
    if (normalizedHeights.empty() || biomes.empty()) {
        throw std::runtime_error("Map data is empty");
    }
    
    long long expectedSize = static_cast<long long>(builderWidth) * static_cast<long long>(builderHeight);
    if (static_cast<long long>(normalizedHeights.size()) != expectedSize ||
        static_cast<long long>(biomes.size()) != expectedSize) {
        throw std::runtime_error("Map data size mismatch");
    }
    
    // Store noise parameters from builder
    m_noiseParams = builder.getParams();
    
    m_blocks.clear();
    m_blockExists.clear();
    clearChunks();
    m_blockCount = 0;
    
    m_initializedFromBuilder = true;
    m_endlessMode = false; // Disable endless mode when using builder data
    
    populateBlocks(builder);
    populateChunks();
}

void Map::setNoiseParams(const MapBuilderParams& params) {
    m_noiseParams = params;
    m_endlessMode = true;
    m_initializedFromBuilder = false;
}

void Map::populateBlocks(const MapBuilder& builder) {
    const auto& normalizedHeights = builder.getNormalizedHeights();
    const auto& biomes = builder.getBiomes();
    
    if (normalizedHeights.empty() || biomes.empty()) {
        throw std::runtime_error("Input data is empty");
    }
    
    m_width = builder.getMapWidth();
    m_depth = builder.getMapHeight();
    
    if (m_width <= 0 || m_depth <= 0) {
        throw std::runtime_error("Invalid map dimensions");
    }
    
    if (m_width > 10000 || m_depth > 10000) {
        throw std::runtime_error("Map dimensions too large");
    }
    
    long long expectedSize = static_cast<long long>(m_width) * static_cast<long long>(m_depth);
    if (static_cast<long long>(normalizedHeights.size()) != expectedSize ||
        static_cast<long long>(biomes.size()) != expectedSize) {
        throw std::runtime_error("Data size mismatch");
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
        throw std::runtime_error("Invalid max dimension");
    }
    
    m_maxHeight = static_cast<int>(std::round(maxHeight * maxDimension * 0.1f)) + 1;
    
    if (m_maxHeight <= 0) {
        m_maxHeight = 1;
    }
    
    if (m_maxHeight > 10000) {
        throw std::runtime_error("Max height too large");
    }
    
    long long arraySizePerZ = static_cast<long long>(m_maxHeight) * static_cast<long long>(m_width);
    constexpr long long MAX_SAFE_ARRAY_SIZE = 100000000LL;
    if (arraySizePerZ > MAX_SAFE_ARRAY_SIZE) {
        throw std::runtime_error("Array size too large");
    }
    
    try {
        m_blocks.resize(m_depth);
        m_blockExists.resize(m_depth);
        for (int z = 0; z < m_depth; z++) {
            m_blocks[z].resize(static_cast<size_t>(arraySizePerZ), BIOME_FIELD);
            m_blockExists[z].resize(static_cast<size_t>(arraySizePerZ), false);
        }
    } catch (const std::bad_alloc&) {
        throw std::runtime_error("Memory allocation failed");
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
            
            int worldY = static_cast<int>(std::round(worldYFloat)) - 10; // Spawn blocks 10 units lower
            
            // Array index needs to account for the offset (add 10 back for array indexing)
            int arrayY = worldY + 10;
            
            if (arrayY < 0 || arrayY >= m_maxHeight) {
                continue;
            }
            
            long long arrayIndex = static_cast<long long>(arrayY) * static_cast<long long>(m_width) + static_cast<long long>(x);
            
            if (arrayIndex < 0 || arrayIndex >= arraySizePerZ) {
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
    // In endless mode, check chunks
    if (m_endlessMode) {
        if (m_chunkSize <= 0) {
            return false;
        }
        
        int chunkX = static_cast<int>(std::floor(static_cast<float>(x) / static_cast<float>(m_chunkSize)));
        int chunkZ = static_cast<int>(std::floor(static_cast<float>(z) / static_cast<float>(m_chunkSize)));
        
        int chunkKey;
        try {
            chunkKey = getChunkKey(chunkX, chunkZ);
        } catch (const std::exception&) {
            return false;
        }
        
        auto it = m_chunks.find(chunkKey);
        if (it == m_chunks.end() || it->second == nullptr || !it->second->isPopulated()) {
            return false;
        }
        
        // Check if block exists in chunk
        const auto& chunkBlocks = it->second->getBlocks();
        for (const auto& block : chunkBlocks) {
            int bx, by, bz;
            BiomeType biome;
            std::tie(bx, by, bz, biome) = block;
            if (bx == x && by == y && bz == z) {
                return true;
            }
        }
        return false;
    }
    
    // Original logic for builder mode
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
    
    // Convert world Y to array Y (add 10 offset)
    int arrayY = y + 10;
    
    if (arrayY < 0 || arrayY >= m_maxHeight) {
        return false;
    }
    
    long long arrayIndex = static_cast<long long>(arrayY) * static_cast<long long>(m_width) + arrayX;
    
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
    // In endless mode, check chunks or generate biome from noise
    if (m_endlessMode) {
        if (m_chunkSize <= 0) {
            // Fallback: generate biome from noise directly
            float biomeNoise = sampleBiomeNoise(static_cast<float>(x), static_cast<float>(z));
            if (!std::isfinite(biomeNoise)) {
                biomeNoise = 0.0f;
            }
            biomeNoise = std::max(0.0f, std::min(1.0f, biomeNoise));
            return MapProperties::getBiomeFromNoise(biomeNoise);
        }
        
        int chunkX = static_cast<int>(std::floor(static_cast<float>(x) / static_cast<float>(m_chunkSize)));
        int chunkZ = static_cast<int>(std::floor(static_cast<float>(z) / static_cast<float>(m_chunkSize)));
        
        int chunkKey;
        try {
            chunkKey = getChunkKey(chunkX, chunkZ);
        } catch (const std::exception&) {
            // Fallback to noise
            float biomeNoise = sampleBiomeNoise(static_cast<float>(x), static_cast<float>(z));
            biomeNoise = std::max(0.0f, std::min(1.0f, biomeNoise));
            return MapProperties::getBiomeFromNoise(biomeNoise);
        }
        
        auto it = m_chunks.find(chunkKey);
        if (it != m_chunks.end() && it->second != nullptr && it->second->isPopulated()) {
            // Find the topmost block at this x,z position
            const auto& chunkBlocks = it->second->getBlocks();
            int maxY = std::numeric_limits<int>::lowest();
            BiomeType foundBiome = BIOME_FIELD;
            
            for (const auto& block : chunkBlocks) {
                int bx, by, bz;
                BiomeType biome;
                std::tie(bx, by, bz, biome) = block;
                if (bx == x && bz == z && by > maxY) {
                    maxY = by;
                    foundBiome = biome;
                }
            }
            
            if (maxY != std::numeric_limits<int>::lowest()) {
                return foundBiome;
            }
        }
        
        // Fallback: generate biome from noise
        float biomeNoise = sampleBiomeNoise(static_cast<float>(x), static_cast<float>(z));
        biomeNoise = std::max(0.0f, std::min(1.0f, biomeNoise));
        return MapProperties::getBiomeFromNoise(biomeNoise);
    }
    
    // Original logic for builder mode
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
    
    blocks.reserve(static_cast<size_t>(m_width) * static_cast<size_t>(m_depth));
    
    for (int z = 0; z < m_depth; z++) {
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
                    int worldY = y - 10; // Blocks are stored 10 units lower
                    
                    if (worldX < std::numeric_limits<int>::min() || worldX > std::numeric_limits<int>::max() ||
                        worldZ < std::numeric_limits<int>::min() || worldZ > std::numeric_limits<int>::max()) {
                        continue;
                    }
                    
                    blocks.push_back(std::make_tuple(static_cast<int>(worldX), worldY, static_cast<int>(worldZ), biome));
                    break;
                }
            }
        }
    }
    
    return blocks;
}

void Map::populateChunks() {
    if (m_chunkSize <= 0) {
        throw std::runtime_error("Invalid chunk size");
    }
    
    if (m_width <= 0 || m_depth <= 0) {
        throw std::runtime_error("Map not initialized");
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
        } catch (const std::exception&) {
            continue;
        }
        
        Chunk* chunk = nullptr;
        
        auto it = m_chunks.find(chunkKey);
        if (it == m_chunks.end()) {
            try {
                chunk = new Chunk(chunkX, chunkZ, m_chunkSize);
                m_chunks[chunkKey] = chunk;
            } catch (const std::bad_alloc&) {
                continue;
            } catch (const std::exception&) {
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
        } catch (const std::exception&) {
            continue;
        }
    }
    
    std::map<std::pair<int, int>, std::pair<int, BiomeType>> terrainData;
    for (const auto& block : allBlocks) {
        int bx, by, bz;
        BiomeType bbiome;
        std::tie(bx, by, bz, bbiome) = block;
        auto key = std::make_pair(bx, bz);
        if (terrainData.find(key) == terrainData.end() || by > terrainData[key].first) {
            terrainData[key] = std::make_pair(by, bbiome);
        }
    }
    
    std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> chunkTreePositions;
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> treeDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> completionCubeDist(0.0f, 1.0f);
    std::map<std::pair<std::pair<int, int>, int>, bool> chunkCompletionCubeSpawned; // Track if completion cube spawned per chunk per biome
    
    for (const auto& terrain : terrainData) {
        int x = terrain.first.first;
        int z = terrain.first.second;
        int blockY = terrain.second.first;
        BiomeType biome = terrain.second.second;
        
        //spawn completion cube (very low chance: 0.5%) - only if not already collected
        if (!hasCompletionCubeBeenCollected(biome)) {
            float chunkXFloat = (static_cast<float>(x) + static_cast<float>(m_centerX)) / static_cast<float>(m_chunkSize);
            float chunkZFloat = (static_cast<float>(z) + static_cast<float>(m_centerZ)) / static_cast<float>(m_chunkSize);
            int chunkX = static_cast<int>(std::floor(chunkXFloat));
            int chunkZ = static_cast<int>(std::floor(chunkZFloat));
            auto chunkKeyPair = std::make_pair(chunkX, chunkZ);
            auto completionCubeKey = std::make_pair(chunkKeyPair, static_cast<int>(biome));
            
            if (chunkCompletionCubeSpawned.find(completionCubeKey) == chunkCompletionCubeSpawned.end() && completionCubeDist(gen) < 0.0005f) { // 0.05% spawn rate (extremely rare)
                int chunkKey = 0;
                bool chunkKeyValid = false;
                try {
                    chunkKey = getChunkKey(chunkX, chunkZ);
                    chunkKeyValid = true;
                } catch (const std::exception&) {
                    // Skip completion cube spawning but continue to tree spawning
                }
                
                if (chunkKeyValid) {
                    auto it = m_chunks.find(chunkKey);
                    if (it != m_chunks.end() && it->second != nullptr) {
                        float cubeY = static_cast<float>(blockY) + 2.0f; // Increased height for visibility
                        glm::vec3 cubePos(static_cast<float>(x), cubeY, static_cast<float>(z));
                        CompletionCube completionCube(cubePos, biome);
                        it->second->addCompletionCube(completionCube);
                        chunkCompletionCubeSpawned[completionCubeKey] = true;
                    }
                }
            }
        }
        
        if (biome != BIOME_FOREST) {
            continue;
        }
        
        float chunkXFloat = (static_cast<float>(x) + static_cast<float>(m_centerX)) / static_cast<float>(m_chunkSize);
        float chunkZFloat = (static_cast<float>(z) + static_cast<float>(m_centerZ)) / static_cast<float>(m_chunkSize);
        int chunkX = static_cast<int>(std::floor(chunkXFloat));
        int chunkZ = static_cast<int>(std::floor(chunkZFloat));
        auto chunkKeyPair = std::make_pair(chunkX, chunkZ);
        
        bool tooClose = false;
        if (chunkTreePositions.find(chunkKeyPair) != chunkTreePositions.end()) {
            for (const auto& treePos : chunkTreePositions[chunkKeyPair]) {
                int dx = x - treePos.first;
                int dz = z - treePos.second;
                if (dx * dx + dz * dz < 100) {
                    tooClose = true;
                    break;
                }
            }
        }
        
        if (tooClose) continue;
        
        if (treeDist(gen) < 0.25f) {
            int chunkKey;
            try {
                chunkKey = getChunkKey(chunkX, chunkZ);
            } catch (const std::exception&) {
                continue;
            }
            
            auto it = m_chunks.find(chunkKey);
            if (it == m_chunks.end() || it->second == nullptr) {
                continue;
            }
            
            float treeBaseY = static_cast<float>(blockY) + 0.5f;
            glm::vec3 basePos(static_cast<float>(x), treeBaseY, static_cast<float>(z));
            
            //TEMPORARY single cylinder tree - tall stump for smoke effect
            Tree tree(basePos);
            float treeHeight = 25.0f; // Much taller
            float treeRadius = 0.5f;
            glm::vec3 treeCenter = basePos + glm::vec3(0.0f, treeHeight / 2.0f, 0.0f);
            TreePieceData trunk(treeCenter, glm::vec3(0.0f), glm::vec3(treeRadius * 2.0f, 1.0f, treeRadius * 2.0f));
            tree.addPiece(trunk);
            
            it->second->addTree(tree);
            chunkTreePositions[chunkKeyPair].push_back({x, z});
        }
    }
    
    int totalTrees = 0;
    for (const auto& chunkPair : m_chunks) {
        totalTrees += chunkPair.second->getTrees().size();
    }
}

float Map::sampleNoise(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        return 0.0f;
    }
    
    if (m_noiseParams.octaves <= 0 || m_noiseParams.octaves > 20) {
        return 0.0f;
    }
    
    if (m_noiseParams.frequency <= 0.0f || !std::isfinite(m_noiseParams.frequency)) {
        return 0.0f;
    }
    
    if (m_noiseParams.amplitude < 0.0f || !std::isfinite(m_noiseParams.amplitude)) {
        return 0.0f;
    }
    
    if (m_noiseParams.persistence < 0.0f || m_noiseParams.persistence > 1.0f || !std::isfinite(m_noiseParams.persistence)) {
        return 0.0f;
    }
    
    float value = 0.0f;
    float amplitude = m_noiseParams.amplitude;
    float frequency = m_noiseParams.frequency;
    
    float seedOffsetX = static_cast<float>(m_noiseParams.seed % 10000) * 0.1f;
    float seedOffsetY = static_cast<float>((m_noiseParams.seed / 10000) % 10000) * 0.1f;
    
    const float MAX_FREQUENCY = 1e6f;
    
    for (int i = 0; i < m_noiseParams.octaves; i++) {
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
        
        amplitude *= m_noiseParams.persistence;
        frequency *= 2.0f;
    }
    
    if (!std::isfinite(value)) {
        return 0.0f;
    }
    
    return value;
}

float Map::sampleBiomeNoise(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        return 0.0f;
    }
    
    if (m_noiseParams.biomeOctaves <= 0 || m_noiseParams.biomeOctaves > 20) {
        return 0.0f;
    }
    
    if (m_noiseParams.biomeFrequency <= 0.0f || !std::isfinite(m_noiseParams.biomeFrequency)) {
        return 0.0f;
    }
    
    if (!std::isfinite(m_noiseParams.biomeWarp)) {
        return 0.0f;
    }
    
    float seedOffsetX = static_cast<float>(m_noiseParams.seed % 10000) * 0.1f;
    float seedOffsetY = static_cast<float>((m_noiseParams.seed / 10000) % 10000) * 0.1f;
    
    float warpStrength = m_noiseParams.biomeWarp * 0.01f;
    
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
    float frequency = m_noiseParams.biomeFrequency;
    float persistence = 0.5f;
    
    const float MAX_FREQUENCY = 1e6f;
    
    for (int i = 0; i < m_noiseParams.biomeOctaves; i++) {
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
        value /= static_cast<float>(m_noiseParams.biomeOctaves);
    } else {
        float maxValue = (1.0f - std::pow(persistence, m_noiseParams.biomeOctaves)) / (1.0f - persistence);
        
        if (!std::isfinite(maxValue) || maxValue < 0.0001f) {
            value /= static_cast<float>(m_noiseParams.biomeOctaves);
        } else {
            value /= maxValue;
        }
    }
    
    if (!std::isfinite(value)) {
        return 0.0f;
    }
    
    return value;
}

void Map::generateChunk(int chunkX, int chunkZ) {
    int chunkKey;
    try {
        chunkKey = getChunkKey(chunkX, chunkZ);
    } catch (const std::exception&) {
        return;
    }
    
    auto it = m_chunks.find(chunkKey);
    if (it != m_chunks.end() && it->second != nullptr && it->second->isPopulated()) {
        return;
    }
    
    Chunk* chunk = nullptr;
    if (it == m_chunks.end()) {
        try {
            chunk = new Chunk(chunkX, chunkZ, m_chunkSize);
            m_chunks[chunkKey] = chunk;
        } catch (const std::bad_alloc&) {
            return;
        } catch (const std::exception&) {
            return;
        }
    } else {
        chunk = it->second;
        chunk->clear();
    }
    
    if (chunk == nullptr) {
        return;
    }
    
    int chunkStartX = chunkX * m_chunkSize;
    int chunkStartZ = chunkZ * m_chunkSize;
    
    int maxDimension = 200;
    
    for (int localZ = 0; localZ < m_chunkSize; localZ++) {
        for (int localX = 0; localX < m_chunkSize; localX++) {
            int worldX = chunkStartX + localX;
            int worldZ = chunkStartZ + localZ;
            
            float rawBiomeNoise = sampleBiomeNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
            if (!std::isfinite(rawBiomeNoise)) {
                rawBiomeNoise = 0.0f;
            }

            float biomeNoise;
            if (rawBiomeNoise >= -1.0f && rawBiomeNoise <= 1.0f) {
                biomeNoise = (rawBiomeNoise + 1.0f) * 0.5f;
            } else if (rawBiomeNoise >= 0.0f && rawBiomeNoise <= 1.0f) {
                biomeNoise = std::pow(rawBiomeNoise, 0.7f); // Slight curve to spread values
            } else {
                biomeNoise = std::max(0.0f, std::min(1.0f, (rawBiomeNoise + 1.0f) * 0.5f));
            }
            
            biomeNoise = std::max(0.0f, std::min(1.0f, biomeNoise));
            
            BiomeType biome = MapProperties::getBiomeFromNoise(biomeNoise);
            
            float baseNoise = sampleNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
            if (!std::isfinite(baseNoise)) {
                baseNoise = 0.0f;
            }
            
            auto smoothstep = [](float t) -> float {
                t = std::max(0.0f, std::min(1.0f, t));
                if (!std::isfinite(t)) {
                    return 0.0f;
                }
                float result = t * t * (3.0f - 2.0f * t);
                return std::isfinite(result) ? result : 0.0f;
            };
            
            float fieldScale = 0.8f;
            float fieldOffset = 0.1f;
            float forestScale = 0.2f;
            float forestOffset = 0.3f;
            float mountainScale = 0.4f;
            float mountainOffset = 0.6f;
            
            float scale, offset;
            if (biomeNoise < 0.5f) {
                float t = smoothstep(biomeNoise * 2.0f);
                scale = fieldScale + (forestScale - fieldScale) * t;
                offset = fieldOffset + (forestOffset - fieldOffset) * t;
            } else {
                float t = smoothstep((biomeNoise - 0.5f) * 2.0f);
                scale = forestScale + (mountainScale - forestScale) * t;
                offset = forestOffset + (mountainOffset - forestOffset) * t;
            }
            
            if (!std::isfinite(scale)) scale = 0.5f;
            if (!std::isfinite(offset)) offset = 0.3f;
            
            float influencedNoise = baseNoise * scale + offset;
            if (!std::isfinite(influencedNoise)) {
                influencedNoise = 0.3f;
            }
            
            float normalized = (influencedNoise + 1.0f) * 0.5f;
            normalized = std::max(0.0f, std::min(1.0f, normalized));
            
            float worldYFloat = normalized * maxDimension * 0.1f;
            if (!std::isfinite(worldYFloat)) {
                continue;
            }
            
            int worldY = static_cast<int>(std::round(worldYFloat)) - 20;
            
            try {
                chunk->addBlock(worldX, worldY, worldZ, biome);
            } catch (const std::exception&) {
                continue;
            }
        }
    }
    
    std::map<std::pair<int, int>, std::pair<int, BiomeType>> terrainData;
    for (const auto& block : chunk->getBlocks()) {
        int bx, by, bz;
        BiomeType bbiome;
        std::tie(bx, by, bz, bbiome) = block;
        auto key = std::make_pair(bx, bz);
        if (terrainData.find(key) == terrainData.end() || by > terrainData[key].first) {
            terrainData[key] = std::make_pair(by, bbiome);
        }
    }
    
    std::vector<std::pair<int, int>> treePositions;
    std::mt19937 gen(static_cast<unsigned int>(chunkX * 10000 + chunkZ));
    std::uniform_real_distribution<float> treeDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> completionCubeDist(0.0f, 1.0f);
    
    int forestBlocks = 0;
    int treesGenerated = 0;
    bool completionCubeSpawned[3] = {false, false, false}; // Track if completion cube spawned for each biome type
    
    for (const auto& terrain : terrainData) {
        int x = terrain.first.first;
        int z = terrain.first.second;
        int blockY = terrain.second.first;
        BiomeType biome = terrain.second.second;
        
        // Spawn completion cube (very low chance: 0.5%) - only if not already collected
        if (!hasCompletionCubeBeenCollected(biome) && !completionCubeSpawned[biome] && completionCubeDist(gen) < 0.0005f) { // 0.05% spawn rate (extremely rare)
            float cubeY = static_cast<float>(blockY) + 2.0f; // 2 blocks above the block for better visibility
            glm::vec3 cubePos(static_cast<float>(x), cubeY, static_cast<float>(z));
            CompletionCube completionCube(cubePos, biome);
            chunk->addCompletionCube(completionCube);
            completionCubeSpawned[biome] = true;
        }
        
        // Only spawn trees in forest biome
        if (biome != BIOME_FOREST) {
            continue;
        }
        
        forestBlocks++;
        
        bool tooClose = false;
        for (const auto& treePos : treePositions) {
            int dx = x - treePos.first;
            int dz = z - treePos.second;
            if (dx * dx + dz * dz < 25) {
                tooClose = true;
                break;
            }
        }
        
        if (tooClose) continue;
        
        if (treeDist(gen) < 0.25f) {
            float treeBaseY = static_cast<float>(blockY) + 0.5f;
            glm::vec3 basePos(static_cast<float>(x), treeBaseY, static_cast<float>(z));
            
            // Simple single cylinder tree
            Tree tree(basePos);
            float treeHeight = 25.0f;
            float treeRadius = 0.5f;
            glm::vec3 treeCenter = basePos + glm::vec3(0.0f, treeHeight / 2.0f, 0.0f);
            TreePieceData trunk(treeCenter, glm::vec3(0.0f), glm::vec3(treeRadius * 2.0f, 1.0f, treeRadius * 2.0f));
            tree.addPiece(trunk);
            
            chunk->addTree(tree);
            treePositions.push_back({x, z});
            treesGenerated++;
        }
    }
    
    
    chunk->setPopulated(true);
}

bool Map::hasCompletionCubeBeenCollected(BiomeType biome) const {
    if (biome >= BIOME_FIELD && biome <= BIOME_FOREST) {
        return m_collectedCompletionCubes[biome];
    }
    return false;
}

void Map::markCompletionCubeCollected(BiomeType biome) {
    if (biome >= BIOME_FIELD && biome <= BIOME_FOREST) {
        m_collectedCompletionCubes[biome] = true;
    }
}

void Map::unloadDistantChunks(const glm::vec3& cameraPos, int keepDistance) {
    if (m_chunkSize <= 0 || keepDistance < 0) {
        return;
    }
    
    if (!std::isfinite(cameraPos.x) || !std::isfinite(cameraPos.y) || !std::isfinite(cameraPos.z)) {
        return;
    }
    
    float cameraChunkXFloat = cameraPos.x / static_cast<float>(m_chunkSize);
    float cameraChunkZFloat = cameraPos.z / static_cast<float>(m_chunkSize);
    
    if (!std::isfinite(cameraChunkXFloat) || !std::isfinite(cameraChunkZFloat)) {
        return;
    }
    
    int cameraChunkX = static_cast<int>(std::floor(cameraChunkXFloat));
    int cameraChunkZ = static_cast<int>(std::floor(cameraChunkZFloat));
    
    std::vector<int> chunksToRemove;
    
    for (const auto& pair : m_chunks) {
        Chunk* chunk = pair.second;
        if (chunk == nullptr) {
            continue;
        }
        
        int chunkX = chunk->getChunkX();
        int chunkZ = chunk->getChunkZ();
        
        int dx = std::abs(chunkX - cameraChunkX);
        int dz = std::abs(chunkZ - cameraChunkZ);
        
        if (dx > keepDistance || dz > keepDistance) {
            chunksToRemove.push_back(pair.first);
        }
    }
    
    for (int key : chunksToRemove) {
        auto it = m_chunks.find(key);
        if (it != m_chunks.end()) {
            delete it->second;
            m_chunks.erase(it);
        }
    }
}

std::vector<std::tuple<int, int, int, BiomeType>> Map::getBlocksInRenderDistance(
    const glm::vec3& cameraPos, int renderDistance) {
    
    std::vector<std::tuple<int, int, int, BiomeType>> blocks;
    
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
    
    float cameraChunkXFloat, cameraChunkZFloat;
    if (m_endlessMode) {
        cameraChunkXFloat = cameraPos.x / static_cast<float>(m_chunkSize);
        cameraChunkZFloat = cameraPos.z / static_cast<float>(m_chunkSize);
    } else {
        cameraChunkXFloat = (cameraPos.x + static_cast<float>(m_centerX)) / static_cast<float>(m_chunkSize);
        cameraChunkZFloat = (cameraPos.z + static_cast<float>(m_centerZ)) / static_cast<float>(m_chunkSize);
    }
    
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
            } catch (const std::exception&) {
                continue;
            }
            
            auto it = m_chunks.find(chunkKey);
            
            if (m_endlessMode && (it == m_chunks.end() || it->second == nullptr || !it->second->isPopulated())) {
                generateChunk(chunkX, chunkZ);
                it = m_chunks.find(chunkKey);
            }
            
            if (it != m_chunks.end() && it->second != nullptr && it->second->isPopulated()) {
                try {
                    const auto& chunkBlocks = it->second->getBlocks();
                    blocks.insert(blocks.end(), chunkBlocks.begin(), chunkBlocks.end());
                } catch (const std::bad_alloc&) {
                    return blocks;
                } catch (const std::exception&) {
                    continue;
                }
            }
        }
    }
    
    //unload distant chunks (only in endless mode)
    if (m_endlessMode) {
        unloadDistantChunks(cameraPos, renderDistance + 2);
    }
    
    return blocks;
}

void Map::ensureChunkGenerated(int chunkX, int chunkZ) {
    int chunkKey;
    try {
        chunkKey = getChunkKey(chunkX, chunkZ);
    } catch (const std::exception&) {
        return;
    }
    
    auto it = m_chunks.find(chunkKey);
    if (m_endlessMode && (it == m_chunks.end() || it->second == nullptr || !it->second->isPopulated())) {
        generateChunk(chunkX, chunkZ);
    }
}

