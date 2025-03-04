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

void Chunk::init() {
    m_spriteBatch.init();
    std::cout << "Chunk initialized at position: " << m_worldPosition.x
        << ", " << m_worldPosition.y << std::endl;
    m_isLoaded = true;
}
void Chunk::buildChunkMesh(BlockManager& blockManager) {
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

                    int waterAmt = block.getWaterAmount();
                    if (waterAmt > WATER_LEVELS) {
                        waterAmt = WATER_LEVELS;
                    }

                    float waterHeight = ((float)waterAmt / (float)WATER_LEVELS);

                    glm::vec4 destRect = glm::vec4(getWorldPosition().x + x - 0.5f, getWorldPosition().y + y - 0.5f, 1.0f, waterHeight);

                    glm::vec4 uvRect = BlockDefRepository::getUVRect(id);
                    GLuint textureID = BlockDefRepository::getTextureID(id);
                    Bengine::ColorRGBA8 color = BlockDefRepository::getColor(id);
                    m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, color, 0.0f);
                } else {
                    // 5 6 7
                    // 3   4
                    // 0 1 2
                    float blockAdj = 1.0f;

                    BlockHandle block0 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x - 1.0f + blockAdj, blockPos.y - 1.0f + blockAdj));
                    BlockHandle block1 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + blockAdj, blockPos.y - 1.0f + blockAdj));
                    BlockHandle block2 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + 1.0f + blockAdj, blockPos.y - 1.0f + blockAdj));
                    BlockHandle block3 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x - 1.0f + blockAdj, blockPos.y + blockAdj));
                    BlockHandle block4 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + 1.0f + blockAdj, blockPos.y + blockAdj));
                    BlockHandle block5 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x - 1.0f + blockAdj, blockPos.y + 1.0f + blockAdj));
                    BlockHandle block6 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + blockAdj, blockPos.y + 1.0f + blockAdj));
                    BlockHandle block7 = blockManager.getBlockAtPosition(glm::vec2(blockPos.x + 1.0f + blockAdj, blockPos.y + 1.0f + blockAdj));


                    BlockAdjacencyRules blockAdjacencyRules;

                    blockAdjacencyRules.Rules[0] = getAdjacencyRuleForBlock(block0.block->getBlockID()); 
                    blockAdjacencyRules.Rules[1] = getAdjacencyRuleForBlock(block1.block->getBlockID()); 
                    blockAdjacencyRules.Rules[2] = getAdjacencyRuleForBlock(block2.block->getBlockID()); 
                    blockAdjacencyRules.Rules[3] = getAdjacencyRuleForBlock(block3.block->getBlockID()); 
                    blockAdjacencyRules.Rules[4] = getAdjacencyRuleForBlock(block4.block->getBlockID()); 
                    blockAdjacencyRules.Rules[5] = getAdjacencyRuleForBlock(block5.block->getBlockID()); 
                    blockAdjacencyRules.Rules[6] = getAdjacencyRuleForBlock(block6.block->getBlockID()); 
                    blockAdjacencyRules.Rules[7] = getAdjacencyRuleForBlock(block7.block->getBlockID()); 

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
                    m_spriteBatch.draw(destRect, uvRectFixed, textureID, 0.0f, color, 0.0f);
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

void BlockManager::update(BlockManager& blockManager) {

    for (int i = 0; i < m_activeChunks.size(); i++) { // Simulate water for all active chunks
        m_cellularAutomataManager.simulateWater(*m_activeChunks[i], blockManager);
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
    // check if the block exists
    if (blockHandle.block != nullptr && !blockHandle.block->isEmpty()) {

        // Destroy the block (remove physics body and reset visual state)
        b2DestroyBody(blockHandle.block->getBodyID());
        // Now that the block is destroyed, we can remove it from the chunk
        // Access the chunk using chunkCoords and blockOffset to set the block to nullptr
        Chunk& chunk = m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y];
        Chunk& chunkLeft = m_chunks[blockHandle.chunkCoords.x - 1][blockHandle.chunkCoords.y];
        Chunk& chunkRight = m_chunks[blockHandle.chunkCoords.x + 1][blockHandle.chunkCoords.y];
        Chunk& chunkTop = m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y + 1];
        Chunk& chunkBot = m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y - 1];

        if (blockHandle.block->getBlockID() == BlockID::WATER) {
            for (int i = 0; i < chunk.waterBlocks.size(); i++) {
                if (getBlockAtPosition(chunk.waterBlocks[i]) == blockHandle) {

                    chunk.waterBlocks[i] = chunk.waterBlocks.back();

                    chunk.waterBlocks.pop_back(); // Add to the list of water blocks.

                    break;
                }
            }
        }




        chunk.blocks[blockHandle.blockOffset.x][blockHandle.blockOffset.y] = Block(); // Reset the broken block to Air
        chunk.m_isMeshDirty = true;
        chunkLeft.m_isMeshDirty = true;
        chunkRight.m_isMeshDirty = true;
        chunkTop.m_isMeshDirty = true;
        chunkBot.m_isMeshDirty = true;
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

void BlockManager::loadNearbyChunks(const glm::vec2& playerPos, BlockManager& blockManager) {

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
                        loadChunk(x, y, blockManager);
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
    static siv::PerlinNoise oreNoise(67890);
    static siv::PerlinNoise veinShapeNoise(11111);

    const float NOISE_SCALE = 0.05f;
    const float AMPLITUDE = 10.0f;
    const float BASE_SURFACE_Y = 1664.0f;

    int fractalOctaves = 7;
    float fractalPersistence = 0.5f;
    float fractalFrequency = 0.0067f;


    float initialNoiseValue = perlin.noise1D(chunkX * CHUNK_WIDTH * NOISE_SCALE);
    int referenceHeight = static_cast<int>(BASE_SURFACE_Y + initialNoiseValue * AMPLITUDE);
    const float DIRT_BOTTOM = referenceHeight - 10;
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
    oreTypes.push_back(OreParams{ BlockID::COPPER,     static_cast<float>(referenceHeight - 20),  static_cast<float>(referenceHeight - 150), 0.5f, 0.35f, 30 }); // upper depth, lower depth, density of vein, frequency of ore veins, max vein amount(doesnt work lol)
    oreTypes.push_back(OreParams{ BlockID::IRON,       static_cast<float>(referenceHeight - 100), static_cast<float>(referenceHeight - 250), 0.49f, 0.38f, 25 });
    oreTypes.push_back(OreParams{ BlockID::GOLD,       static_cast<float>(referenceHeight - 200), static_cast<float>(referenceHeight - 350), 0.48f, 0.41f, 20 });
    oreTypes.push_back(OreParams{ BlockID::DIAMOND,    static_cast<float>(referenceHeight - 300), static_cast<float>(referenceHeight - 450), 0.47f, 0.44f, 15 });
    oreTypes.push_back(OreParams{ BlockID::COBALT,     static_cast<float>(referenceHeight - 400), static_cast<float>(referenceHeight - 650), 0.46f, 0.47f, 14 });
    oreTypes.push_back(OreParams{ BlockID::MYTHRIL,    static_cast<float>(referenceHeight - 600), static_cast<float>(referenceHeight - 750), 0.45f, 0.50f, 13 });
    oreTypes.push_back(OreParams{ BlockID::ADAMANTITE, static_cast<float>(referenceHeight - 700), static_cast<float>(referenceHeight - 850), 0.44f, 0.53f, 12 });
    oreTypes.push_back(OreParams{ BlockID::COSMILITE,  static_cast<float>(referenceHeight - 800), static_cast<float>(referenceHeight - 1300), 0.43f, 0.56f, 11 });
    oreTypes.push_back(OreParams{ BlockID::PRIMORDIAL, static_cast<float>(referenceHeight - 1100), static_cast<float>(referenceHeight - 1600), 0.42f, 0.60f, 10 });

    activeVeins.clear();
    for (const auto& ore : oreTypes) {
        activeVeins[ore.oreType] = std::vector<VeinTracker>();
    }
    

    for (const auto& ore : oreTypes) {
        PROFILE_SCOPE("generateChunk: ChunkOreVeins");
        int veinAttempts = static_cast<int>(CHUNK_WIDTH * CHUNK_WIDTH * ore.frequency * 0.09f);

        for (int i = 0; i < veinAttempts; i++) {
            int localX = rand() % CHUNK_WIDTH;
            int localY = rand() % CHUNK_WIDTH;
            int worldX = chunkX * CHUNK_WIDTH + localX;
            int worldY = chunkY * CHUNK_WIDTH + localY;

            if (worldY <= ore.maxDepth && worldY >= ore.minDepth) {
                float oreNoiseValue = generateFractalNoise(worldX, worldY, fractalFrequency, fractalPersistence, fractalOctaves, 72839);

                if (oreNoiseValue > ore.veinSize) { // Reduced threshold for more veins

                    int baseRadius = 1 + rand() % 2;
                    float density = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f; // 0.3-0.5
                    float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.28f; // Random angle in radians

                    // Chance for much larger veins
                    if (rand() % 100 < 20) { // 20% chance for larger vein
                        baseRadius += 1 + rand() % 2; // Add 1-3 to radius
                    }

                    chunkOreVeins.push_back({ worldX, worldY, baseRadius, ore.oreType, density, angle});
                }
            }
        }
    }


    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        PROFILE_SCOPE("generateChunk: Cave Gen, Blocks, Ore Gen");


        int worldX = chunkX * CHUNK_WIDTH + x;
        float noiseValue = perlin.noise1D(worldX * NOISE_SCALE);
        int height = static_cast<int>(BASE_SURFACE_Y + noiseValue * AMPLITUDE);
        const float localDirtBottom = height - 10;
        const float localStoneBottom = height - 400;
        const float localDeepStoneBottom = height - 800;

        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            int worldY = chunkY * CHUNK_WIDTH + y;
            glm::vec2 position(worldX, worldY);
            Block currentBlock;
            bool shouldBeAir = false;

            // Cave generation
            if (worldY < height - m_minCaveDepth) {
                // Generate base cave noise
                float caveVal = generateFractalNoise(worldX, worldY, fractalFrequency, fractalPersistence, fractalOctaves, 12345);
                float medCaveVal = generateFractalNoise(worldX, worldY, fractalFrequency, fractalPersistence, fractalOctaves, 54793);
                float smallCaveVal = generateFractalNoise(worldX, worldY, fractalFrequency, fractalPersistence, fractalOctaves, 65492);

                // Primary cave generation
                if (caveVal > m_baseCaveThreshold) { // m_baseCaveThreshold = 0.3f
                    shouldBeAir = true;
                }

                if (medCaveVal > m_baseCaveThreshold * 1.75f) { // m_baseCaveThreshold = 0.3f
                    shouldBeAir = true;
                }

                if (smallCaveVal > m_baseCaveThreshold * 2.5f) { // small caves
                    shouldBeAir = true;
                }
            }

            if (!shouldBeAir) {
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

                // ore generation
                //assert(!chunkOreVeins.empty());
                for (const auto& vein : chunkOreVeins) {

                    float oreFrequency = (fractalFrequency / 3.0f);

                    // Calculate distance from vein center
                    int dx = worldX - vein.centerX;
                    int dy = worldY - vein.centerY;

                    // Calculate distance with directional stretching
                    float stretchedX = dx * cos(vein.angle) - dy * sin(vein.angle);
                    float stretchedY = dx * sin(vein.angle) + dy * cos(vein.angle);
                    stretchedX *= 1.0f + generateFractalNoise(worldX, worldY, oreFrequency, fractalPersistence, fractalOctaves, 35367);
                    stretchedY *= 1.0f + generateFractalNoise(worldX, worldY, oreFrequency, fractalPersistence, fractalOctaves, 56758);

                    float distance = sqrt(stretchedX * stretchedX + stretchedY * stretchedY);

                    // If within vein radius, place ore
                    if (distance <= vein.radius) {
                        float oreNoise  = generateFractalNoise(worldX, worldY, oreFrequency, fractalPersistence, fractalOctaves, 12345);

                        // More irregular placement condition
                        if (oreNoise > (1.0f - vein.density)||
                            (distance < vein.radius)) {
                            currentBlock.init(m_world, vein.oreType, position);
                            break;
                        }
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

float BlockManager::generateFractalNoise(int worldX, int worldY, float frequency, float persistence, int octaves, int seed) {
    // Create a FractalRidged node
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
    auto fractalNode = FastNoise::New<FastNoise::FractalRidged>();

    fractalNode->SetSource(fnSimplex);
    fractalNode->SetOctaveCount(octaves);
    fractalNode->SetGain(persistence);  // persistence controls the gain
    fractalNode->SetLacunarity(2.0f); // frequency controls the lacunarity

    // Generate the fractal noise at the given world coordinates
    return fractalNode->GenSingle2D(worldX * frequency, worldY * frequency, seed); // Returns the generated noise value
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



void BlockManager::loadChunk(int chunkX, int chunkY, BlockManager& blockManager) {
    Chunk& chunk = m_chunks[chunkX][chunkY];
    Chunk& chunkLeft = m_chunks[chunkX - 1][chunkY];
    Chunk& chunkRight = m_chunks[chunkX + 1][chunkY];
    Chunk& chunkTop = m_chunks[chunkX][chunkY + 1];
    Chunk& chunkBot = m_chunks[chunkX][chunkY - 1];
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
        chunk.buildChunkMesh(blockManager);

    }
    m_activeChunks.push_back(&chunk);

    chunk.m_isMeshDirty = true;
    chunkLeft.m_isMeshDirty = true;
    chunkRight.m_isMeshDirty = true;
    chunkTop.m_isMeshDirty = true;
    chunkBot.m_isMeshDirty = true;
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

            // If a valid block is returned, add it to the list
            if (blockHandle.block->getBlockID() != BlockID::AIR) {
                blocksInRange.push_back(blockHandle);
            }
        }
    }

    return blocksInRange;
}

