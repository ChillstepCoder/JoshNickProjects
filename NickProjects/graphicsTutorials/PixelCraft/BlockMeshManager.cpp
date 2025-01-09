#include <Bengine/ResourceManager.h>
#include "BlockMeshManager.h"
#include "DebugDraw.h"
#include "CellularAutomataManager.h"
#include "PerlinNoise.hpp"
#include <iostream>
#include <fstream>


void Chunk::init() {
    m_spriteBatch.init();
    std::cout << "Chunk initialized at position: " << m_worldPosition.x
        << ", " << m_worldPosition.y << std::endl;
    m_isLoaded = true;
}
void Chunk::buildChunkMesh() {
    m_spriteBatch.begin();
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        float worldX = CHUNK_WIDTH + x;
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            float worldY = CHUNK_WIDTH + y;

            const Block& block = blocks[x][y];
            if (!block.isEmpty()) {
                BlockDefRepository repository;
                BlockID id = block.getBlockID();

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

                    glm::vec4 destRect = glm::vec4(getWorldPosition().x + x - 0.5f, getWorldPosition().y + y - 0.5f, 1.0f, 1.0f);

                    glm::vec4 uvRect = BlockDefRepository::getUVRect(id);
                    GLuint textureID = BlockDefRepository::getTextureID(id);
                    Bengine::ColorRGBA8 color = BlockDefRepository::getColor(id);
                    m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, color, 0.0f);
                }

                //BlockRenderer::renderBlock(m_spriteBatch, repository.getDef(id),glm::vec2(x,y));
            }
        }
    }
    m_spriteBatch.end();
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

        if (blockHandle.block->getBlockID() == BlockID::WATER) {
            for (int i = 0; i < chunk.waterBlocks.size(); i++) {
                if (getBlockAtPosition(chunk.waterBlocks[i]) == blockHandle) {

                    chunk.waterBlocks[i] = chunk.waterBlocks.back();

                    chunk.waterBlocks.pop_back(); // Add to the list of water blocks.

                    break;
                }
            }
        }


        chunk.blocks[blockHandle.blockOffset.x][blockHandle.blockOffset.y] = Block();  // Reset the block to a new instance (or nullptr if applicable)


        //std::cout << "Block destroyed at X: " << blockHandle.blockOffset.x << "   Y: " << blockHandle.blockOffset.y << std::endl;

        chunk.m_isMeshDirty = true;
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

void BlockManager::loadNearbyChunks(const glm::vec2& playerPos) {

    // Calc player chunk position
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_WIDTH;
    int playerChunkY = static_cast<int>(playerPos.y) / CHUNK_WIDTH;

    // Loop through all chunks and load ones that are nearby
    for (int x = 0; x < WORLD_WIDTH_CHUNKS; ++x) {
        for (int y = 0; y < WORLD_HEIGHT_CHUNKS; ++y) {
            if (!isChunkLoaded(x, y)) {

                glm::vec2 chunkPos = m_chunks[x][y].getWorldPosition();
                if (!isChunkFarAway(playerPos, chunkPos)) {
                    loadChunk(x, y);
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

    static siv::PerlinNoise perlin(12345);  // Use a fixed seed to regenerate terrain consistently


    const float NOISE_SCALE = 0.05f;  // Controls how stretched the noise is
    const float AMPLITUDE = 10.0f;    // Controls the height variation
    const float BASE_SURFACE_Y = 384.0f;  // Base height for the surface (6 chunks of ground, 2 chunks of sky)

    Bengine::ColorRGBA8 textureColor(255, 255, 255, 255);

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        int worldX = chunkX * CHUNK_WIDTH + x;
        float noiseValue = perlin.noise1D(worldX * NOISE_SCALE);
        int height = static_cast<int>(BASE_SURFACE_Y + noiseValue * AMPLITUDE);

        const float DIRT_BOTTOM = height - 10;

        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            int worldY = chunkY * CHUNK_WIDTH + y;
            glm::vec2 position(worldX, worldY);

            if (worldY == height) {  // Surface block (grass)
                Block surfaceBlock;
                surfaceBlock.init(m_world, BlockID::GRASS, position);
                chunk.blocks[x][y] = surfaceBlock;
            }
            else if (worldY < height && worldY > DIRT_BOTTOM) {  // Dirt blocks
                Block dirtBlock;
                dirtBlock.init(m_world, BlockID::DIRT, position);
                chunk.blocks[x][y] = dirtBlock;
            }
            else if (worldY <= DIRT_BOTTOM) {  // Stone blocks
                Block stoneBlock;
                stoneBlock.init(m_world, BlockID::STONE, position);
                chunk.blocks[x][y] = stoneBlock;
            }
            else {  // Air blocks
                Block airBlock;
                airBlock.init(m_world, BlockID::AIR, position);
                chunk.blocks[x][y] = airBlock;
            }
        }
    }

}



void BlockManager::loadChunk(int chunkX, int chunkY) {
    Chunk& chunk = m_chunks[chunkX][chunkY];
    // Make sure we never double load
    assert(!chunk.isLoaded());


    chunk.init();

    if (!loadChunkFromFile(chunkX, chunkY, chunk)) {
        // If no saved chunk data exists, generate it
        generateChunk(chunkX, chunkY, chunk);
        saveChunkToFile(chunkX, chunkY, chunk);  // Save the generated chunk for later
    }
    chunk.buildChunkMesh();
    m_activeChunks.push_back(&chunk);
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
        }
    }

    file.close();
    return true;
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

std::vector<Block> BlockManager::getBlocksInRange(const glm::vec2& playerPos, int range) {
    std::vector<Block> blocksInRange;

    // Loop through the range of blocks around the player (in both x and y directions)
    for (int dx = -range; dx <= range; ++dx) {
        for (int dy = -range; dy <= range; ++dy) {
            // Calculate the world position for the block's center
            glm::vec2 blockPos = playerPos + glm::vec2(dx, dy);

            // Get the block at the position using the getBlockAtPosition function
            BlockHandle blockHandle = getBlockAtPosition(blockPos);

            // If a valid block is returned, add it to the list
            if (blockHandle.block != nullptr) {
                blocksInRange.push_back(*blockHandle.block);
            }
        }
    }

    return blocksInRange;
}