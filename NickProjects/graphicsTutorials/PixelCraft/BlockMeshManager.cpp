#include <Bengine/ResourceManager.h>
#include "BlockMeshManager.h"
#include "DebugDraw.h"
#include "PerlinNoise.hpp"
#include <iostream>


void Chunk::init() {
    m_spriteBatch.init();
    std::cout << "Chunk initialized at position: " << m_worldPosition.x
        << ", " << m_worldPosition.y << std::endl;
}
void Chunk::buildMesh() {
    m_spriteBatch.begin();
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_WIDTH; ++y) {
            const Block& block = blocks[x][y];
            if (!block.isEmpty()) {
                glm::vec4 destRect = block.getDestRect();
                glm::vec4 uvRect = block.getUVRect();
                GLuint textureID = block.getTextureID();
                Bengine::ColorRGBA8 color = block.getColor();

                m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, color, 0.0f);
            }
        }
    }
    m_spriteBatch.end();
}
void Chunk::render() {
    m_spriteBatch.renderBatch();
}



BlockMeshManager::BlockMeshManager() {

}

BlockMeshManager::~BlockMeshManager() {

}

void BlockMeshManager::init() {
    m_spriteBatch.init();
}
void BlockMeshManager::buildMesh(std::vector<std::vector<Chunk>>& chunks, BlockManager& blockManager) {
    m_spriteBatch.begin();
    for (auto& chunkRow : chunks) {
        for (auto& chunk : chunkRow) {
            chunk.buildMesh();
        }
    }
    m_spriteBatch.end();
}
void BlockMeshManager::renderMesh(std::vector<std::vector<Chunk>>& chunks, BlockManager& blockManager) {
    // Only render chunks that are visible
    m_spriteBatch.begin();
    for (auto& chunkRow : chunks) {
        for (auto& chunk : chunkRow) {
            // Check if the chunk is visible (can be based on the camera's frustum or player position)
            if (!blockManager.isChunkFarAway(chunk.getWorldPosition(), glm::vec2(0.0f, 0.0f))) {
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
    m_spriteBatch.end();
}


void BlockManager::initializeChunks() {

    for (int chunkX = 0; chunkX < WORLD_WIDTH_CHUNKS; ++chunkX) {
        for (int chunkY = 0; chunkY < WORLD_HEIGHT_CHUNKS; ++chunkY) {
            Chunk& chunk = m_chunks[chunkX][chunkY];
            chunk.init(); // Explicitly call init for each chunk
            chunk.m_worldPosition = glm::vec2(chunkX * CHUNK_WIDTH, chunkY * CHUNK_WIDTH);
        }
    }

    // Create Perlin noise instance with random seed
    siv::PerlinNoise perlin(12345); // Can change this seed for different terrain

    // Parameters for terrain generation
    const float NOISE_SCALE = 0.05f;  // Controls how stretched the noise is
    const float AMPLITUDE = 10.0f;    // Controls the height variation
    const float BASE_SURFACE_Y = 384.0f;    // Base height of the surface (6 chunks of ground, 2 chunks of sky)

    Bengine::ColorRGBA8 textureColor(255, 255, 255, 255);

    // Generate terrain for each chunk
    for (int chunkX = 0; chunkX < WORLD_WIDTH_CHUNKS; ++chunkX) {
        for (int chunkY = 0; chunkY < WORLD_HEIGHT_CHUNKS; ++chunkY) {
            Chunk& chunk = m_chunks[chunkX][chunkY];
            chunk.m_worldPosition = glm::vec2(chunkX * CHUNK_WIDTH, chunkY * CHUNK_WIDTH);

            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                int worldX = chunkX * CHUNK_WIDTH + x;
                float noiseValue = perlin.noise1D(worldX * NOISE_SCALE);
                int height = static_cast<int>(BASE_SURFACE_Y + noiseValue * AMPLITUDE);

                const float DIRT_BOTTOM = height - 10;

                for (int y = 0; y < CHUNK_WIDTH; ++y) {
                    int worldY = chunkY * CHUNK_WIDTH + y;
                    glm::vec2 position(worldX, worldY);

                    if (worldY == height) { // Create surface (grass) blocks
                        Block surfaceBlock;
                        surfaceBlock.init(m_world, position,
                            Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png"), textureColor);
                        chunk.blocks[x][y] = surfaceBlock;
                    }
                    else if (worldY < height && worldY > DIRT_BOTTOM) { // Fill blocks below surface with dirt
                        Block dirtBlock;
                        dirtBlock.init(m_world, position,
                            Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png"), textureColor);
                        chunk.blocks[x][y] = dirtBlock;
                    } else if (worldY <= DIRT_BOTTOM) { // Fill deeper blocks with stone
                        Block stoneBlock;
                        stoneBlock.init(m_world, position,
                            Bengine::ResourceManager::getTexture("Textures/connectedStoneBlock.png"), textureColor);
                        chunk.blocks[x][y] = stoneBlock;
                    }
                }
            }
        }
    }
}

BlockHandle BlockManager::getBlockAtPosition(glm::vec2 position) {
    int worldX = std::floor(position.x);
    int worldY = std::floor(position.y);
    int chunkX = worldX / CHUNK_WIDTH;
    int chunkY = worldY / CHUNK_WIDTH;
    int offsetX = worldX % CHUNK_WIDTH;
    int offsetY = worldY % CHUNK_WIDTH;
    if (offsetX < 0) offsetX += CHUNK_WIDTH; // in case of negative coords
    if (offsetY < 0) offsetY += CHUNK_WIDTH; // in case of negative coords

    for (size_t i = 0; i < m_chunks.size(); ++i) {
        for (size_t j = 0; j < m_chunks[i].size(); ++j) {
            Chunk& chunk = m_chunks[i][j];
            glm::vec2 chunkWorldPos = chunk.getWorldPosition();

            // Check if the chunk's world position matches (consider the chunk size)
            if (chunkWorldPos.x == chunkX * CHUNK_WIDTH && chunkWorldPos.y == chunkY * CHUNK_WIDTH) {
                // Found the chunk, now get the block inside it
                Block* block = &chunk.blocks[offsetX][offsetY];
                return BlockHandle{ block, glm::ivec2(chunkX, chunkY), glm::ivec2(offsetX, offsetY) };
            }
        }
    }
    // If no chunk is found return a null BlockHandle
    return BlockHandle{ nullptr, glm::ivec2(chunkX, chunkY), glm::ivec2(offsetX, offsetY) };
}

void BlockManager::destroyBlock(const BlockHandle& blockHandle) {
    // check if the block exists
    if (blockHandle.block != nullptr && !blockHandle.block->isEmpty()) {
        // Destroy the block (remove physics body and reset visual state)
        b2DestroyBody(blockHandle.block->getID());
        // Now that the block is destroyed, we can remove it from the chunk
        // Access the chunk using chunkCoords and blockOffset to set the block to nullptr
        Chunk& chunk = m_chunks[blockHandle.chunkCoords.x][blockHandle.chunkCoords.y];
        chunk.blocks[blockHandle.blockOffset.x][blockHandle.blockOffset.y] = Block();  // Reset the block to a new instance (or nullptr if applicable)

        rebuildMesh();
    }
}

void BlockManager::breakBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos) {
    // Get the block at the given position
    BlockHandle blockHandle = getBlockAtPosition(position);

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


inline bool BlockManager::isPositionInBlock(const glm::vec2& position, const Block& block) {
    // Check if the position is within the block's bounding box
    glm::vec2 blockPos = block.getDestRect();
    blockPos.x = blockPos.x + 0.5;
    blockPos.y = blockPos.y + 0.5;
    glm::vec2 blockSize = block.getDimensions();
    return (position.x >= blockPos.x - blockSize.x / 2 && position.x <= blockPos.x + blockSize.x / 2 &&
        position.y >= blockPos.y - blockSize.y / 2 && position.y <= blockPos.y + blockSize.y / 2);
}

void BlockManager::loadNearbyChunks(const glm::vec2& playerPos) {
    // Calc player chunk position
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_WIDTH;
    int playerChunkY = static_cast<int>(playerPos.y) / CHUNK_WIDTH;

    // Loop through the chunks within the radius and load them
    for (int dx = -loadRadius; dx <= loadRadius; ++dx) {
        for (int dy = -loadRadius; dy <= loadRadius; ++dy) {
            int chunkX = playerChunkX + dx;
            int chunkY = playerChunkY + dy;
            if (!isChunkLoaded(chunkX, chunkY)) {
                loadChunk(chunkX, chunkY);
            }
        }
    }
}

bool BlockManager::isChunkLoaded(int x, int y) {
    // Prevent out-of-bounds access
    if (x < 0 || y < 0 || x >= WORLD_WIDTH_CHUNKS || y >= WORLD_HEIGHT_CHUNKS) {
        return false; // Out of bounds
    }

    // If the chunk already exists and has been initialized, return true
    if (m_chunks[x][y].blocks[0][0].getTextureID() != 0) {
        return true;
    }

    return false;

}

void BlockManager::loadChunk(int x, int y) {
    if (x < 0 || y < 0 || x >= WORLD_WIDTH_CHUNKS || y >= WORLD_HEIGHT_CHUNKS) {
        return; // Out of bounds
    }
}

bool BlockManager::isChunkFarAway(const glm::vec2& playerPos, const glm::vec2& chunkPos) {
    // Calcs the distance between the player and the chunk
    float distance = glm::distance(playerPos, chunkPos);

    const float farthestChunkAllowed = 10.0f;

    const float unloadDistance = farthestChunkAllowed * CHUNK_WIDTH;

    return distance > unloadDistance;

}

void BlockManager::unloadFarChunks(const glm::vec2& playerPos) {
    // Calc player chunk position
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_WIDTH;
    int playerChunkY = static_cast<int>(playerPos.y) / CHUNK_WIDTH;

    // Loop through all loaded chunks and unload ones that are far away
    for (int x = 0; x < WORLD_WIDTH_CHUNKS; ++x) {
        for (int y = 0; y <= WORLD_WIDTH_CHUNKS; ++y) {
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
    if (x < 0 || y < 0 || x >= WORLD_WIDTH_CHUNKS || y >= WORLD_HEIGHT_CHUNKS) {
        return; // Out of bounds
    }

    m_chunks[x][y] = Chunk(); // Reset chunk to default
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