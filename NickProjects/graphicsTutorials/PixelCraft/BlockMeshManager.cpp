#include <Bengine/ResourceManager.h>
#include "BlockMeshManager.h"
#include "DebugDraw.h"
#include "CellularAutomataManager.h"
#include "ConnectedTextureSet.h"
#include "PerlinNoise.hpp"
#include <iostream>
#include <fstream>
#include <filesystem> 
#include "FastNoise2/FastNoise/FastNoise.h"
#include "Profiler.h"
#include "Timer.h"
#include "LightingSystem.h"

void Chunk::init() {
    m_spriteBatch.init();
    std::cout << "Chunk initialized at position: " << m_worldPosition.x
        << ", " << m_worldPosition.y << std::endl;
    m_isLoaded = true;
}
void Chunk::buildChunkMesh(BlockManager& blockManager, const LightingSystem& lightingSystem) {
    m_spriteBatch.begin();
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        float worldX = CHUNK_WIDTH + x;
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            float worldY = CHUNK_WIDTH + y;

            const Block& block = blocks[x][y];
            if (!block.isEmpty()) {
                BlockDefRepository repository;
                BlockID id = block.getBlockID();
                BlockDef blockDef = repository.getDef(id);
                glm::vec2 blockPos = glm::vec2(getWorldPosition().x + x - 0.5f, getWorldPosition().y + y - 0.5f);

                if (id == BlockID::WATER) {
                    // Water rendering code (unchanged)
                    int waterAmt = block.getWaterAmount();
                    if (waterAmt > WATER_LEVELS) {
                        waterAmt = WATER_LEVELS;
                    }

                    float waterHeight = ((float)waterAmt / (float)WATER_LEVELS);

                    glm::vec4 destRect = glm::vec4(getWorldPosition().x + x - 0.5f, getWorldPosition().y + y - 0.5f, 1.0f, waterHeight);

                    glm::vec4 uvRect = BlockDefRepository::getUVRect(id);
                    GLuint textureID = BlockDefRepository::getTextureID(id);
                    Bengine::ColorRGBA8 color = BlockDefRepository::getColor(id);

                    Bengine::ColorRGBA8 lightedColor = lightingSystem.applyLighting(color, blockPos.x, blockPos.y);

                    m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, lightedColor, 0.0f);
                }
                else {
                    // 5 6 7
                    // 3   4
                    // 0 1 2
                    float blockAdj = 1.0f;

                    BlockAdjacencyRules blockAdjacencyRules;
                    for (int i = 0; i < 8; ++i) {
                        blockAdjacencyRules.Rules[i] = AdjacencyRule::AIR;
                    }

                    // Safely get adjacent blocks
                    BlockHandle block0 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x - 1.0f + blockAdj, blockPos.y - 1.0f + blockAdj));
                    if (block0.block && !block0.block->isEmpty()) {
                        blockAdjacencyRules.Rules[0] = getAdjacencyRuleForBlock(block0.block->getBlockID());
                    }

                    BlockHandle block1 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + blockAdj, blockPos.y - 1.0f + blockAdj));
                    if (block1.block && !block1.block->isEmpty()) {
                        blockAdjacencyRules.Rules[1] = getAdjacencyRuleForBlock(block1.block->getBlockID());
                    }

                    BlockHandle block2 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + 1.0f + blockAdj, blockPos.y - 1.0f + blockAdj));
                    if (block2.block && !block2.block->isEmpty()) {
                        blockAdjacencyRules.Rules[2] = getAdjacencyRuleForBlock(block2.block->getBlockID());
                    }

                    BlockHandle block3 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x - 1.0f + blockAdj, blockPos.y + blockAdj));
                    if (block3.block && !block3.block->isEmpty()) {
                        blockAdjacencyRules.Rules[3] = getAdjacencyRuleForBlock(block3.block->getBlockID());
                    }

                    BlockHandle block4 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + 1.0f + blockAdj, blockPos.y + blockAdj));
                    if (block4.block && !block4.block->isEmpty()) {
                        blockAdjacencyRules.Rules[4] = getAdjacencyRuleForBlock(block4.block->getBlockID());
                    }

                    BlockHandle block5 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x - 1.0f + blockAdj, blockPos.y + 1.0f + blockAdj));
                    if (block5.block && !block5.block->isEmpty()) {
                        blockAdjacencyRules.Rules[5] = getAdjacencyRuleForBlock(block5.block->getBlockID());
                    }

                    BlockHandle block6 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + blockAdj, blockPos.y + 1.0f + blockAdj));
                    if (block6.block && !block6.block->isEmpty()) {
                        blockAdjacencyRules.Rules[6] = getAdjacencyRuleForBlock(block6.block->getBlockID());
                    }

                    BlockHandle block7 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + 1.0f + blockAdj, blockPos.y + 1.0f + blockAdj));
                    if (block7.block && !block7.block->isEmpty()) {
                        blockAdjacencyRules.Rules[7] = getAdjacencyRuleForBlock(block7.block->getBlockID());
                    }

                    if (id == BlockID::DIRT) { // if the original block is dirt, ignore the dirt adjacency and treat them all as blocks instead.
                        for (int i = 0; i < 8; ++i) {
                            if (blockAdjacencyRules.Rules[i] == AdjacencyRule::DIRT) {
                                blockAdjacencyRules.Rules[i] = AdjacencyRule::BLOCK; // Replace DIRT with BLOCK
                            }
                        }
                    }




                    float pixelWidth = 0.00690f;
                    float pixelHeight = 0.00736f;
                    glm::vec4 uvRect = ConnectedTextureSet::getInstance().GetSubTextureUVForRules(blockAdjacencyRules, x , y);


                    float fixedsubUV_Y = 1.0f - uvRect.y - uvRect.z;

                    glm::vec4 destRect = glm::vec4(getWorldPosition().x + x - 0.5f, getWorldPosition().y + y - 0.5f, 1.0f, 1.0f);

                    glm::vec4 uvRectFixed = glm::vec4(uvRect.x, fixedsubUV_Y += pixelHeight, uvRect.z -= pixelWidth, uvRect.w -= pixelHeight); // need this because the .png is slightly incorrect


                    GLuint textureID = BlockDefRepository::getTextureID(id);

                    Bengine::setTextureFilterMode(textureID, Bengine::TextureFilterMode::Linear);

                    Bengine::ColorRGBA8 color = BlockDefRepository::getColor(id);

                    Bengine::ColorRGBA8 lightedColor = lightingSystem.applyLighting(color, blockPos.x, blockPos.y);


                    m_spriteBatch.draw(destRect, uvRectFixed, textureID, 0.0f, lightedColor, 0.0f);
                }

                //BlockRenderer::renderBlock(m_spriteBatch, repository.getDef(id),glm::vec2(x,y));
            }
        }
    }
    m_spriteBatch.end();
}

AdjacencyRule Chunk::getAdjacencyRuleForBlock(BlockID blockID) {
    if (blockID == BlockID::AIR || blockID == BlockID::WATER) {
        return AdjacencyRule::AIR;
    }
    //if (blockID == BlockID::DIRT) {
    //    return AdjacencyRule::DIRT;
    //}
    else {
        return AdjacencyRule::BLOCK; // Anything else (default to BLOCK)
    }
}


void Chunk::render() {

    m_spriteBatch.renderBatch();
}

void Chunk::save() {
    std::ofstream outFile("World/chunk_" + std::to_string(static_cast<int>(m_worldPosition.x)) +
        "_" + std::to_string(static_cast<int>(m_worldPosition.y)) + ".dat", std::ios::binary);

    if (!outFile) {
        std::cerr << "Failed to open file for saving chunk!" << std::endl;
        return;
    }

    // Write chunk position to file (for loading purposes)
    outFile.write(reinterpret_cast<const char*>(&m_worldPosition.x), sizeof(m_worldPosition.x));
    outFile.write(reinterpret_cast<const char*>(&m_worldPosition.y), sizeof(m_worldPosition.y));

    // Save each block in the chunk
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            blocks[x][y].saveToFile(outFile);
        }
    }

    outFile.close();
    std::cout << "Chunk saved!" << std::endl;
}

void Chunk::load() {
    std::ifstream inFile("World/chunk_" + std::to_string(static_cast<int>(m_worldPosition.x)) +
        "_" + std::to_string(static_cast<int>(m_worldPosition.y)) + ".dat", std::ios::binary);

    if (!inFile) {
        std::cerr << "Failed to open file for loading chunk!" << std::endl;
        return;
    }

    // Read chunk position from file
    inFile.read(reinterpret_cast<char*>(&m_worldPosition.x), sizeof(m_worldPosition.x));
    inFile.read(reinterpret_cast<char*>(&m_worldPosition.y), sizeof(m_worldPosition.y));

    // Load each block in the chunk
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            blocks[x][y].loadFromFile(inFile);
        }
    }

    inFile.close();
    std::cout << "Chunk loaded!" << std::endl;
}



void Chunk::destroy() {
    // Destroy physics bodies for all blocks in the chunk
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            Block& block = blocks[x][y];
            if (!block.isEmpty()) {
                b2DestroyBody(block.getBodyID());
            }
            block.clearID();
        }
    }

    // Clear the sprite batch
    m_spriteBatch.dispose();

    m_isLoaded = false;
}


glm::vec2 BlockHandle::getWorldPosition() {
    return (chunkCoords * CHUNK_WIDTH) + blockOffset;
}



BlockMeshManager::BlockMeshManager() {

}

BlockMeshManager::~BlockMeshManager() {

}

void BlockMeshManager::init() {

}

void BlockMeshManager::renderMesh(std::vector<std::vector<Chunk>>& chunks, BlockManager& blockManager) {
    // Only render chunks that are visible
    for (auto& chunkRow : chunks) {
        for (auto& chunk : chunkRow) {
            // Check if the chunk is visible (can be based on the camera's frustum or player position)
            // TODO: Only render chunks that are actually on screen (Check if their box intersects camera box)
            if (chunk.isLoaded()) {
                if (!chunk.m_spriteBatch.isInitialized()) {
                    std::cerr << "Warning: SpriteBatch not initialized for chunk at "
                              << chunk.getWorldPosition().x << ", "
                              << chunk.getWorldPosition().y << std::endl;
                    chunk.init(); // Initialize the SpriteBatch if it's not already
                }
                chunk.render();
            }
        }
    }
}


void BlockManager::initializeChunks(glm::vec2 playerPosition) {

    m_chunks.resize(WORLD_WIDTH_CHUNKS);
    for (int chunkX = 0; chunkX < WORLD_WIDTH_CHUNKS; ++chunkX) {
        m_chunks[chunkX].resize(WORLD_HEIGHT_CHUNKS);
        for (int chunkY = 0; chunkY < WORLD_HEIGHT_CHUNKS; ++chunkY) {
            Chunk& chunk = m_chunks[chunkX][chunkY];
            chunk.m_worldPosition = glm::vec2(chunkX * CHUNK_WIDTH, chunkY * CHUNK_WIDTH);
        }
    }
}

void BlockManager::update(BlockManager& blockManager, const LightingSystem& lightingSystem) {

    for (int i = 0; i < m_activeChunks.size(); i++) { // Simulate water for all active chunks
        m_cellularAutomataManager.simulateWater(*m_activeChunks[i], blockManager, lightingSystem);
    }

}

BlockHandle BlockManager::getBlockAtPosition(glm::vec2 position) {
    int blockPosX = std::floor(position.x);
    int blockPosY = std::floor(position.y);
    int chunkPosX = blockPosX / CHUNK_WIDTH;
    int chunkPosY = blockPosY / CHUNK_WIDTH;
    int blockOffsetX = blockPosX % CHUNK_WIDTH;
    int blockOffsetY = blockPosY % CHUNK_WIDTH;

    if (chunkPosX < WORLD_WIDTH_CHUNKS && chunkPosY < WORLD_HEIGHT_CHUNKS && chunkPosX >= 0 && chunkPosY >= 0) {
        Chunk& chunk = m_chunks[chunkPosX][chunkPosY];
        Block* block = &chunk.blocks[blockOffsetX][blockOffsetY];
        return BlockHandle{ block, glm::ivec2(chunkPosX, chunkPosY), glm::ivec2(blockOffsetX, blockOffsetY) };

    } else {
        std::cout << "Chunk out of bounds!!!";
        return BlockHandle{ nullptr, glm::ivec2(chunkPosX, chunkPosY), glm::ivec2(blockOffsetX, blockOffsetY) };
    }
}
Chunk* BlockManager::getChunkAtPosition(glm::vec2 position) {
    int blockPosX = std::floor(position.x);
    int blockPosY = std::floor(position.y);
    int chunkPosX = blockPosX / CHUNK_WIDTH;
    int chunkPosY = blockPosY / CHUNK_WIDTH;

    if (chunkPosX < WORLD_WIDTH_CHUNKS && chunkPosY < WORLD_HEIGHT_CHUNKS && chunkPosX >= 0 && chunkPosY >= 0) {
        Chunk& chunk = m_chunks[chunkPosX][chunkPosY];
        return &chunk;

    }
    else {
        Chunk blankChunk{};
        std::cout << "Chunk out of bounds!!!";
        return &blankChunk;
    }
}



glm::ivec2 BlockManager::getBlockWorldPos(glm::ivec2 chunkCoords, glm::ivec2 offset) {
    int chunkBlockTotalX = chunkCoords.x * CHUNK_WIDTH;
    int chunkBlockTotalY = chunkCoords.y * CHUNK_WIDTH;

    return glm::ivec2((chunkBlockTotalX + offset.x), (chunkBlockTotalY + offset.y));
}


void BlockManager::destroyBlock(const BlockHandle& blockHandle) {
    // Check if the block exists
    if (blockHandle.block != nullptr && !blockHandle.block->isEmpty()) {
        // Destroy the block (remove physics body and reset visual state)
        b2DestroyBody(blockHandle.block->getBodyID());

        // Check if chunk coordinates are valid
        if (blockHandle.chunkCoords.x < 0 || blockHandle.chunkCoords.x >= m_chunks.size() ||
            blockHandle.chunkCoords.y < 0 || blockHandle.chunkCoords.y >= m_chunks[0].size()) {
            return; // Invalid chunk coordinates
        }

        // Access the current chunk safely
        Chunk& chunk = m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y];

        // Handle water blocks
        if (blockHandle.block->getBlockID() == BlockID::WATER) {
            for (int i = 0; i < chunk.waterBlocks.size(); i++) {
                if (getBlockAtPosition(chunk.waterBlocks[i]) == blockHandle) {
                    chunk.waterBlocks[i] = chunk.waterBlocks.back();
                    chunk.waterBlocks.pop_back(); // Add to the list of water blocks.
                    break;
                }
            }
        }

        // Reset the broken block to Air
        chunk.blocks[blockHandle.blockOffset.x][blockHandle.blockOffset.y] = Block();

        // Mark the current chunk as dirty
        chunk.m_isMeshDirty = true;

        // Safely mark adjacent chunks as dirty only if they exist
        if (blockHandle.chunkCoords.x > 0) {
            m_chunks[blockHandle.chunkCoords.x - 1][blockHandle.chunkCoords.y].m_isMeshDirty = true; // Left
        }

        if (blockHandle.chunkCoords.x < m_chunks.size() - 1) {
            m_chunks[blockHandle.chunkCoords.x + 1][blockHandle.chunkCoords.y].m_isMeshDirty = true; // Right
        }

        if (blockHandle.chunkCoords.y < m_chunks[0].size() - 1) {
            m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y + 1].m_isMeshDirty = true; // Top
        }

        if (blockHandle.chunkCoords.y > 0) {
            m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y - 1].m_isMeshDirty = true; // Bottom
        }
    }
}

void BlockManager::breakBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos) {
    // Get the block at the given position
    float realpositionX = position.x + 0.5f;
    float realPositionY = position.y + 0.5f;

    BlockHandle blockHandle = getBlockAtPosition(glm::vec2(realpositionX,realPositionY));

    // Check if the block exists (not nullptr)
    if (blockHandle.block != nullptr) {
        float distance = glm::distance(position, playerPos);

        // If the block is within the specified range (e.g., 8 blocks radius)
        if (distance <= 8.0f) {
            // "Break" the block - this can involve various actions, like setting it to empty, destroying it, etc.
            destroyBlock(blockHandle);
        }
    }
}

void BlockManager::placeBlock(const BlockHandle& blockHandle, const glm::vec2& position) {
    float realpositionX = position.x - 0.5f;
    float realPositionY = position.y - 0.5f;

    Block waterBlock;
    waterBlock.init(m_world, BlockID::WATER, glm::vec2(realpositionX, realPositionY));

    Chunk& chunk = m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y];
    chunk.blocks[blockHandle.blockOffset.x][blockHandle.blockOffset.y] = waterBlock; 

    if (waterBlock.getBlockID() == BlockID::WATER) {
        // Ensure it doesn't already exist
        assert(std::find(chunk.waterBlocks.begin(), chunk.waterBlocks.end(), glm::ivec2(position.x, position.y)) == chunk.waterBlocks.end());
        chunk.waterBlocks.push_back(glm::vec2(position.x, position.y)); // Add to the list of water blocks.
    }

    //std::cout << "water placed at X: " << blockHandle.blockOffset.x << "   Y: " << blockHandle.blockOffset.y << std::endl;


    chunk.m_isMeshDirty = true;
}

void BlockManager::placeBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos) {
    // Get the block at the given position
    float realpositionX = position.x + 0.5f;
    float realPositionY = position.y + 0.5f;

    BlockHandle blockHandle = getBlockAtPosition(glm::vec2(realpositionX, realPositionY));


    // Check if the space is empty
    if (blockHandle.block->isEmpty()) {
        float distance = glm::distance(position, playerPos);

        // If the block is within the specified range (e.g., 8 blocks radius)
        if (distance <= 8.0f) {
            // "Break" the block - this can involve various actions, like setting it to empty, destroying it, etc.
            placeBlock(blockHandle, glm::vec2(realpositionX, realPositionY));
        }
    }
}




inline bool BlockManager::isPositionInBlock(const glm::vec2& position, const Block& block) {
    // Check if the position is within the block's bounding box
    glm::vec2 blockPos = glm::vec4((position.x - 0.5f), (position.y - 0.5f), 1.0f, 1.0f);
    blockPos.x = blockPos.x ;
    blockPos.y = blockPos.y ;
    glm::vec2 blockSize = glm::vec2(1.0f, 1.0f);
    return (position.x >= blockPos.x - blockSize.x / 2 && position.x <= blockPos.x + blockSize.x / 2 &&
        position.y >= blockPos.y - blockSize.y / 2 && position.y <= blockPos.y + blockSize.y / 2);
}

void BlockManager::loadNearbyChunks(const glm::vec2& playerPos, BlockManager& blockManager, const LightingSystem& lightingSystem) {

    // Calc player chunk position
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_WIDTH;
    int playerChunkY = static_cast<int>(playerPos.y) / CHUNK_WIDTH;

    // Loop through all chunks and load ones that are nearby
    for (int x = 0; x < WORLD_WIDTH_CHUNKS; ++x) {
        for (int y = 0; y < WORLD_HEIGHT_CHUNKS; ++y) {
            if (!isChunkLoaded(x, y)) {

                glm::vec2 chunkPos = m_chunks[x][y].getWorldPosition();
                if (!isChunkFarAway(playerPos, chunkPos)) {
                    {
                        PROFILE_SCOPE("LoadChunk");
                        loadChunk(x, y, blockManager, lightingSystem);
                    }
                }
            }
        }
    }
}

bool BlockManager::isChunkLoaded(int x, int y) {
    // Prevent out-of-bounds access
    assert(x >= 0 && y >= 0 && x < WORLD_WIDTH_CHUNKS && y < WORLD_HEIGHT_CHUNKS);

    // If the chunk already exists and has been initialized, return true
    return m_chunks[x][y].isLoaded();
}


void BlockManager::generateChunk(int chunkX, int chunkY, Chunk& chunk) {
    assert(chunkX >= 0 && chunkY >= 0 && chunkX < WORLD_WIDTH_CHUNKS && chunkY < WORLD_HEIGHT_CHUNKS);

    static siv::PerlinNoise perlin(12345);
    static siv::PerlinNoise dirtThicknessNoise(54321);

    const float NOISE_SCALE = 0.05f;
    const float AMPLITUDE = 10.0f;
    const float BASE_SURFACE_Y = 1664.0f;

    const float SURFACE_CAVE_TAPER_START = 30.0f;
    const float MIN_DIRT_THICKNESS = 3.0f;
    const float MAX_DIRT_THICKNESS = 15.0f;
    const float DIRT_THICKNESS_NOISE_SCALE = 0.03f;

    float initialNoiseValue = perlin.noise1D(chunkX * CHUNK_WIDTH * NOISE_SCALE);
    int referenceHeight = static_cast<int>(BASE_SURFACE_Y + initialNoiseValue * AMPLITUDE);

    const float STONE_BOTTOM = referenceHeight - 400;
    const float DEEP_STONE_BOTTOM = referenceHeight - 800;

    struct OreParams {
        BlockID oreType;
        float maxDepth;    // Higher Y value (closer to surface)
        float minDepth;    // Lower Y value (deeper underground)
        float frequency;
        float veinSize;
        int maxVeinSize; // Maximum ores per vein
    };

    struct OreVein {
        int centerX, centerY;
        int radius;
        BlockID oreType;
        float density;
        float angle;
    };

    std::vector<OreVein> chunkOreVeins;
    chunkOreVeins.clear();
    assert(chunk.blocks != nullptr);

    // Adjusted parameters with corrected depth ranges
    std::vector<OreParams> oreTypes;
    oreTypes.clear();
    // Note: For each ore type, maxDepth the higher number (closer to surface)
    oreTypes.push_back(OreParams{ BlockID::COPPER,     static_cast<float>(referenceHeight - 20),  static_cast<float>(referenceHeight - 150), 0.3f, 0.35f, 30 });
    oreTypes.push_back(OreParams{ BlockID::IRON,       static_cast<float>(referenceHeight - 100), static_cast<float>(referenceHeight - 250), 0.26f, 0.38f, 25 });
    oreTypes.push_back(OreParams{ BlockID::GOLD,       static_cast<float>(referenceHeight - 200), static_cast<float>(referenceHeight - 350), 0.24f, 0.41f, 20 });
    oreTypes.push_back(OreParams{ BlockID::DIAMOND,    static_cast<float>(referenceHeight - 300), static_cast<float>(referenceHeight - 450), 0.22f, 0.44f, 15 });
    oreTypes.push_back(OreParams{ BlockID::COBALT,     static_cast<float>(referenceHeight - 400), static_cast<float>(referenceHeight - 650), 0.20f, 0.47f, 14 });
    oreTypes.push_back(OreParams{ BlockID::MYTHRIL,    static_cast<float>(referenceHeight - 600), static_cast<float>(referenceHeight - 750), 0.18f, 0.50f, 13 });
    oreTypes.push_back(OreParams{ BlockID::ADAMANTITE, static_cast<float>(referenceHeight - 700), static_cast<float>(referenceHeight - 850), 0.16f, 0.53f, 12 });
    oreTypes.push_back(OreParams{ BlockID::COSMILITE,  static_cast<float>(referenceHeight - 800), static_cast<float>(referenceHeight - 1300), 0.14f, 0.56f, 11 });
    oreTypes.push_back(OreParams{ BlockID::PRIMORDIAL, static_cast<float>(referenceHeight - 1100), static_cast<float>(referenceHeight - 1600), 0.10f, 0.60f, 10 });

    activeVeins.clear();
    for (const auto& ore : oreTypes) {
        activeVeins[ore.oreType] = std::vector<VeinTracker>();
    }

    // Create a 2D grid to map ore positions directly
    std::vector<std::vector<BlockID>> oreMap(CHUNK_WIDTH, std::vector<BlockID>(CHUNK_WIDTH, BlockID::COUNT));

    // Generate ore veins and directly mark affected blocks in the oreMap
    PROFILE_SCOPE("generateChunk: ChunkOreVeins");
    for (const auto& ore : oreTypes) {
        int veinAttempts = static_cast<int>(CHUNK_WIDTH * CHUNK_WIDTH * ore.frequency * 0.09f);

        for (int i = 0; i < veinAttempts; i++) {
            int localX = rand() % CHUNK_WIDTH;
            int localY = rand() % CHUNK_WIDTH;
            int worldX = chunkX * CHUNK_WIDTH + localX;
            int worldY = chunkY * CHUNK_WIDTH + localY;

            if (worldY <= ore.maxDepth && worldY >= ore.minDepth) {
                // Use the cached noise generator for this ore type
                float oreNoiseValue = m_oreNoiseGenerators[ore.oreType].getNoise2D(worldX, worldY);

                if (oreNoiseValue > ore.veinSize) { // Threshold for vein creation
                    int baseRadius = 1 + rand() % 2;
                    float density = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f; // 0.3-0.5
                    float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.28f; // Random angle in radians

                    // Chance for larger veins
                    if (rand() % 100 < 20) { // 20% chance for larger vein
                        baseRadius += 1 + rand() % 2; // Add 1-3 to radius
                    }

                    // Directly populate the oreMap with this vein
                    int maxRadius = baseRadius + 1; // Add 1 for safety
                    int startX = std::max(0, localX - maxRadius);
                    int endX = std::min(CHUNK_WIDTH - 1, localX + maxRadius);
                    int startY = std::max(0, localY - maxRadius);
                    int endY = std::min(CHUNK_WIDTH - 1, localY + maxRadius);

                    // Apply the vein to blocks in this area
                    for (int bx = startX; bx <= endX; bx++) {
                        for (int by = startY; by <= endY; by++) {
                            int blockWorldX = chunkX * CHUNK_WIDTH + bx;
                            int blockWorldY = chunkY * CHUNK_WIDTH + by;

                            // Get vein shape noise
                            float noiseX = m_veinShapeNoiseGenerators[ore.oreType].getNoise2D(blockWorldX, blockWorldY);
                            float noiseY = m_veinShapeNoiseGenerators[ore.oreType].getNoise2D(blockWorldY, blockWorldX);

                            // Calculate distance with directional stretching
                            int dx = blockWorldX - worldX;
                            int dy = blockWorldY - worldY;
                            float stretchedX = dx * cos(angle) - dy * sin(angle);
                            float stretchedY = dx * sin(angle) + dy * cos(angle);
                            stretchedX *= 1.0f + noiseX;
                            stretchedY *= 1.0f + noiseY;

                            float distance = sqrt(stretchedX * stretchedX + stretchedY * stretchedY);

                            // If within vein radius, mark for ore placement
                            if (distance <= baseRadius) {
                                float oreNoise = m_oreNoiseGenerators[ore.oreType].getNoise2D(blockWorldX + 1000, blockWorldY + 1000);

                                // More irregular placement condition
                                if (oreNoise > (1.0f - density) || (distance < baseRadius * 0.5f)) {
                                    // Only mark if not already marked, or if this ore is rarer (higher value)
                                    if (oreMap[bx][by] == BlockID::COUNT || oreMap[bx][by] < ore.oreType) {
                                        oreMap[bx][by] = ore.oreType;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // First pass - check for cave entrances at the surface
    // We'll store them to apply in the second pass
    std::vector<int> caveEntranceXPositions;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        int worldX = chunkX * CHUNK_WIDTH + x;
        float noiseValue = perlin.noise1D(worldX * NOISE_SCALE);
        int height = static_cast<int>(BASE_SURFACE_Y + noiseValue * AMPLITUDE);

        // Check for cave entrance at this x position
        // We check a few blocks below the surface
        bool hasCaveEntrance = false;
        for (int checkDepth = 1; checkDepth <= 5; checkDepth++) {
            int worldY = height - checkDepth;
            int chunkY_local = (worldY - chunkY * CHUNK_WIDTH);

            // Skip if out of chunk bounds
            if (chunkY_local < 0 || chunkY_local >= CHUNK_WIDTH) continue;

            // Check for potential cave
            float caveVal = m_caveNoise.getNoise2D(worldX, worldY);
            float medCaveVal = m_mediumCaveNoise.getNoise2D(worldX, worldY);
            float smallCaveVal = m_smallCaveNoise.getNoise2D(worldX, worldY);

            // If any of these indicate a cave close to the surface
            if (caveVal > m_baseCaveThreshold * 1.2f ||
                medCaveVal > m_baseCaveThreshold * 1.8f ||
                smallCaveVal > m_baseCaveThreshold * 2.4f) {

                // If close enough to surface, mark as cave entrance
                hasCaveEntrance = true;
                break;
            }
        }

        if (hasCaveEntrance) {
            caveEntranceXPositions.push_back(x);
        }
    }

    // Generate terrain and apply ore veins
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        PROFILE_SCOPE("generateChunk: Cave Gen, Blocks, Ore Gen");

        int worldX = chunkX * CHUNK_WIDTH + x;
        float noiseValue = perlin.noise1D(worldX * NOISE_SCALE);
        int height = static_cast<int>(BASE_SURFACE_Y + noiseValue * AMPLITUDE);

        float dirtThicknessValue = dirtThicknessNoise.noise2D(worldX * DIRT_THICKNESS_NOISE_SCALE, chunkX * DIRT_THICKNESS_NOISE_SCALE);
        float dirtThickness = MIN_DIRT_THICKNESS + (dirtThicknessValue + 1.0f) * 0.5f * (MAX_DIRT_THICKNESS - MIN_DIRT_THICKNESS);

        const float localDirtBottom = height - dirtThickness;
        const float localStoneBottom = height - 400;
        const float localDeepStoneBottom = height - 800;

        bool isCaveEntrance = std::find(caveEntranceXPositions.begin(), caveEntranceXPositions.end(), x) != caveEntranceXPositions.end();

        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            int worldY = chunkY * CHUNK_WIDTH + y;
            glm::vec2 position(worldX, worldY);
            Block currentBlock;
            bool shouldBeAir = false;

            // Cave generation using pre-created noise generators
            if (isCaveEntrance && worldY == height) {
                shouldBeAir = true;
            }
            else if (worldY < height) {
                // Generate cave noise using our cached generators
                float caveVal = m_caveNoise.getNoise2D(worldX, worldY);
                float medCaveVal = m_mediumCaveNoise.getNoise2D(worldX, worldY);
                float smallCaveVal = m_smallCaveNoise.getNoise2D(worldX, worldY);

                float depth = height - worldY;

                float caveThreshold = m_baseCaveThreshold; // 0.45f
                float medCaveThreshold = m_baseCaveThreshold + 0.1f;
                float smallCaveThreshold = m_baseCaveThreshold + 0.2f;

                // Apply tapering
                if (depth < SURFACE_CAVE_TAPER_START) {
                    float taperFactor = 1.0f - (depth / SURFACE_CAVE_TAPER_START);

                    caveThreshold += taperFactor * 0.2f;
                    medCaveThreshold += taperFactor * 0.25f;
                    smallCaveThreshold += taperFactor * 0.3f;

                    // Allow occasional breakthrough to surface
                    if (depth < 2.0f && caveVal > 0.75f) {
                        shouldBeAir = true;
                    }
                }

                // Primary cave generation
                if (caveVal > caveThreshold) {
                    shouldBeAir = true;
                }

                if (medCaveVal > medCaveThreshold) {
                    shouldBeAir = true;
                }

                if (smallCaveVal > smallCaveThreshold) {
                    shouldBeAir = true;
                }
            }

            if (!shouldBeAir) {
                // First set the base block type
                if (worldY == height) {
                    currentBlock.init(m_world, BlockID::GRASS, position);
                }
                else if (worldY < height && worldY > localDirtBottom) {
                    currentBlock.init(m_world, BlockID::DIRT, position);
                }
                else if (worldY <= localDirtBottom && worldY > localStoneBottom) {
                    currentBlock.init(m_world, BlockID::STONE, position);
                }
                else if (worldY <= localStoneBottom && worldY > localDeepStoneBottom) {
                    currentBlock.init(m_world, BlockID::DEEPSTONE, position);
                }
                else if (worldY <= localDeepStoneBottom) {
                    currentBlock.init(m_world, BlockID::DEEPERSTONE, position);
                }

                // NEW APPROACH: Check if this block has an ore in the oreMap
                if (oreMap[x][y] != BlockID::COUNT) {
                    // Only replace stone and deeper blocks with ore
                    BlockID currentType = currentBlock.getBlockID();
                    if (currentType == BlockID::STONE ||
                        currentType == BlockID::DEEPSTONE ||
                        currentType == BlockID::DEEPERSTONE) {
                        currentBlock.init(m_world, oreMap[x][y], position);
                    }
                }
            }
            else {
                currentBlock.init(m_world, BlockID::AIR, position);
            }
            chunk.blocks[x][y] = currentBlock;
        }
    }
}

void BlockManager::regenerateWorld(float caveScale, float baseCaveThreshold, float detailScale, float detailInfluence, float minCaveDepth, float surfaceZone, float deepZone, float maxSurfaceBonus, float maxDepthPenalty) {
    m_caveScale = caveScale;
    m_baseCaveThreshold = baseCaveThreshold;
    m_detailScale = detailScale;
    m_detailInfluence = detailInfluence;
    m_minCaveDepth = minCaveDepth;
    m_surfaceZone = surfaceZone;
    m_deepZone = deepZone;
    m_maxSurfaceBonus = maxSurfaceBonus;
    m_maxDepthPenalty = maxDepthPenalty;

    activeVeins.clear();

    clearWorldFiles();

    for (int chunkX = 0; chunkX < WORLD_WIDTH_CHUNKS; ++chunkX) {
        for (int chunkY = 0; chunkY < WORLD_HEIGHT_CHUNKS; ++chunkY) {

            Chunk& chunk = m_chunks[chunkX][chunkY]; // Assuming getChunk retrieves the chunk reference
            chunk.destroy();
            m_activeChunks.clear();

            generateChunk(chunkX, chunkY, chunk); // Regenerate the chunk
        }
    }
}



void BlockManager::loadChunk(int chunkX, int chunkY, BlockManager& blockManager, const LightingSystem& lightingSystem) {
    // Check if coordinates are in valid range
    if (chunkX < 0 || chunkX >= m_chunks.size() ||
        chunkY < 0 || chunkY >= m_chunks[0].size()) {
        return; // Out of bounds, don't load
    }

    Chunk& chunk = m_chunks[chunkX][chunkY];

    // Make sure we never double load
    assert(!chunk.isLoaded());

    {
        PROFILE_SCOPE("Chunk init");
        chunk.init();
    }

    if (!loadChunkFromFile(chunkX, chunkY, chunk)) {
        // If no saved chunk data exists, generate it
        {
            PROFILE_SCOPE("generateChunk");
            generateChunk(chunkX, chunkY, chunk);
        }
        {
            PROFILE_SCOPE("saveChunkToFile");
            saveChunkToFile(chunkX, chunkY, chunk);  // Save the generated chunk for later
        }
    }

    {
        PROFILE_SCOPE("buildChunkMesh");
        chunk.buildChunkMesh(blockManager, lightingSystem);
    }

    m_activeChunks.push_back(&chunk);
    chunk.m_isMeshDirty = true;

    // Only mark adjacent chunks as dirty if they exist
    if (chunkX > 0) {
        m_chunks[chunkX - 1][chunkY].m_isMeshDirty = true; // Left
    }
    if (chunkX < m_chunks.size() - 1) {
        m_chunks[chunkX + 1][chunkY].m_isMeshDirty = true; // Right
    }
    if (chunkY < m_chunks[0].size() - 1) {
        m_chunks[chunkX][chunkY + 1].m_isMeshDirty = true; // Top
    }
    if (chunkY > 0) {
        m_chunks[chunkX][chunkY - 1].m_isMeshDirty = true; // Bottom
    }
}

bool BlockManager::saveChunkToFile(int chunkX, int chunkY, Chunk& chunk) {
    std::ofstream file("World/chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkY) + ".dat", std::ios::binary | std::ios::trunc);

    if (!file) {
        std::cerr << "Failed to open file for saving chunk " << chunkX << ", " << chunkY << std::endl;
        return false;
    }

    // Save block data 
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            Block& block = chunk.blocks[x][y];
            BlockID blockID = block.getBlockID(); 
            file.write(reinterpret_cast<char*>(&blockID), sizeof(BlockID));

            if (blockID == BlockID::WATER) {
                // Save water amount if the block is water
                int waterAmount = block.getWaterAmount();
                file.write(reinterpret_cast<char*>(&waterAmount), sizeof(int));
            }
        }
    }

    file.close();
    return true;
}

// Load chunk data from a binary file
bool BlockManager::loadChunkFromFile(int chunkX, int chunkY, Chunk& chunk) {
    std::ifstream file("World/chunk_" + std::to_string(chunkX) + "_" + std::to_string(chunkY) + ".dat", std::ios::binary);

    if (!file) {
        return false;  // Return false if the file doesn't exist
    }

    // Load block data
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            BlockID blockID;
            file.read(reinterpret_cast<char*>(&blockID), sizeof(BlockID));
            chunk.blocks[x][y].init(m_world, blockID, glm::vec2(chunk.getWorldPosition().x + x, chunk.getWorldPosition().y + y));  // Reinitialize the block with the loaded BlockID

            if (blockID == BlockID::WATER) {
                // Load water amount if the block is water
                int waterAmount;
                file.read(reinterpret_cast<char*>(&waterAmount), sizeof(int));
                chunk.blocks[x][y].setWaterAmount(waterAmount);

                // Add to the water blocks vector
                chunk.waterBlocks.push_back(glm::vec2(chunk.getWorldPosition().x + x, chunk.getWorldPosition().y + y));
            }
        }
    }

    file.close();
    return true;
}

void BlockManager::clearWorldFiles() {
    std::filesystem::path worldDir = "World"; // Path to your world folder

    // Iterate through all files in the "World" folder
    for (const auto& entry : std::filesystem::directory_iterator(worldDir)) {
        if (entry.path().extension() == ".dat") {
            std::filesystem::remove(entry.path());  // Delete the file
        }
    }

    std::cout << "All chunk files have been cleared." << std::endl;
}


bool BlockManager::isChunkFarAway(const glm::vec2& playerPos, const glm::vec2& chunkPos) {
    // Calcs the distance between the player and the chunk
    float distance = glm::distance(playerPos, chunkPos);
        
    const float farthestChunkAllowed = 3.0f;

    const float unloadDistance = farthestChunkAllowed * CHUNK_WIDTH;
    return distance > unloadDistance;

}

void BlockManager::unloadFarChunks(const glm::vec2& playerPos) {
    // Calc player chunk position
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_WIDTH;
    int playerChunkY = static_cast<int>(playerPos.y) / CHUNK_WIDTH;

    // Loop through all loaded chunks and unload ones that are far away
    for (int x = 0; x < WORLD_WIDTH_CHUNKS; ++x) {
        for (int y = 0; y < WORLD_HEIGHT_CHUNKS; ++y) {
            if (isChunkLoaded(x, y)) {
                glm::vec2 chunkPos = m_chunks[x][y].getWorldPosition();
                if (isChunkFarAway(playerPos, chunkPos)) {
                    unloadChunk(x, y);
                }
            }
        }
    }
}

void BlockManager::unloadChunk(int x, int y) {
    assert(x >= 0 && y >= 0 && x < WORLD_WIDTH_CHUNKS && y < WORLD_HEIGHT_CHUNKS);

    Chunk& chunk = m_chunks[x][y];

    saveChunkToFile(x, y, chunk); //Save the chunk before unloading

    for (int i = 0; i < CHUNK_WIDTH; i++) {
        for (int j = 0; j < CHUNK_WIDTH; j++) {

            float realpositionX = (x * CHUNK_WIDTH) + i + 0.5f;
            float realPositionY = (y * CHUNK_WIDTH) + j + 0.5f;

            BlockHandle blockHandle = getBlockAtPosition(glm::vec2(realpositionX, realPositionY));


            destroyBlock(blockHandle);
        }
    }

    for (int i = 0; i < m_activeChunks.size(); i++) { // Fixes the list of active chunks
        if (m_activeChunks[i] == &m_chunks[x][y]) {
            m_activeChunks[i] = m_activeChunks.back();
            m_activeChunks.pop_back();
            break;
        }
    }
    m_chunks[x][y].waterBlocks.clear();
    m_chunks[x][y].m_isLoaded = false;
}

std::vector<BlockHandle> BlockManager::getBlocksInRange(const glm::vec2& playerPos, int range) {
    std::vector<BlockHandle> blocksInRange;
    // Loop through the range of blocks around the player (in both x and y directions)
    for (int dx = -range; dx <= range; ++dx) {
        for (int dy = -range; dy <= range; ++dy) {
            // Calculate the world position for the block's center
            glm::vec2 blockPos = playerPos + glm::vec2(dx, dy);
            // Get the block at the position using the getBlockAtPosition function
            BlockHandle blockHandle = getBlockAtPosition(blockPos);
            // Check if the block handle is valid before trying to access it
            if (blockHandle.block && blockHandle.block->getBlockID() != BlockID::AIR) {
                blocksInRange.push_back(blockHandle);
            }
        }
    }
    return blocksInRange;
}