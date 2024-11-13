#include <Bengine/ResourceManager.h>
#include "BlockMeshManager.h"
#include "DebugDraw.h"
#include "PerlinNoise.hpp"
#include <iostream>

BlockMeshManager::BlockMeshManager() {

}

BlockMeshManager::~BlockMeshManager() {

}

void BlockMeshManager::init() {
    m_spriteBatch.init();
}
void BlockMeshManager::buildMesh(const std::vector<std::vector<Chunk>>& chunks) {
    m_spriteBatch.begin();
    for (const auto& chunkRow : chunks) {
        for (const auto& chunk : chunkRow) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                for (int y = 0; y < CHUNK_WIDTH; ++y) {
                    const Block& block = chunk.blocks[x][y];
                    if (!block.isEmpty()) {
                        glm::vec4 destRect = block.getDestRect();
                        glm::vec4 uvRect = block.getUVRect();
                        GLuint textureID = block.getTextureID();
                        Bengine::ColorRGBA8 color = block.getColor();

                        m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, color, 0.0f);
                    }
                }
            }
        }
    }
    m_spriteBatch.end();
}
void BlockMeshManager::renderMesh() {
    m_spriteBatch.renderBatch();
}

void BlockManager::initializeChunks() {
    const float BLOCK_WIDTH = 1.0f;
    const float BLOCK_HEIGHT = 1.0f;

    // Resize the m_chunks vector to the correct size
    m_chunks.resize(WORLD_WIDTH_CHUNKS);
    for (int x = 0; x < WORLD_WIDTH_CHUNKS; ++x) {
        m_chunks[x].resize(WORLD_HEIGHT_CHUNKS);
    }

    // Create Perlin noise instance with random seed
    siv::PerlinNoise perlin(12345); // Can change this seed for different terrain

    // Parameters for terrain generation
    const float NOISE_SCALE = 0.05f;  // Controls how stretched the noise is
    const float AMPLITUDE = 10.0f;    // Controls the height variation
    const float SURFACE_Y = 64.0f;    // Base height of the surface

    Bengine::ColorRGBA8 textureColor(255, 255, 255, 255);

    // Generate terrain for each chunk
    for (int chunkX = 0; chunkX < WORLD_WIDTH_CHUNKS; ++chunkX) {
        for (int chunkY = 0; chunkY < WORLD_HEIGHT_CHUNKS; ++chunkY) {
            Chunk& chunk = m_chunks[chunkX][chunkY];
            chunk.m_worldPosition = glm::vec2(chunkX * CHUNK_WIDTH, chunkY * CHUNK_WIDTH);

            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                int worldX = chunkX * CHUNK_WIDTH + x;
                float noiseValue = perlin.noise1D(worldX * NOISE_SCALE);
                int height = static_cast<int>(SURFACE_Y + noiseValue * AMPLITUDE);
                const float DIRT_BOTTOM = height - 10;

                for (int y = 0; y < CHUNK_WIDTH; ++y) {
                    int worldY = chunkY * CHUNK_WIDTH + y;
                    glm::vec2 position(worldX, worldY);

                    if (worldY == height) { // Create surface (grass) blocks
                        Block surfaceBlock;
                        surfaceBlock.init(&m_world, glm::vec2(position.x, position.y * BLOCK_HEIGHT), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                            Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png"), textureColor);
                        chunk.blocks[x][y] = surfaceBlock;
                    }
                    else if (worldY < height && worldY > DIRT_BOTTOM) { // Fill blocks below surface with dirt
                        Block dirtBlock;
                        dirtBlock.init(&m_world, glm::vec2(position.x, y * BLOCK_HEIGHT), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                            Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png"), textureColor);
                        chunk.blocks[x][y] = dirtBlock;
                    } else if (worldY <= DIRT_BOTTOM) { // Fill deeper blocks with stone
                        Block stoneBlock;
                        stoneBlock.init(&m_world, glm::vec2(position.x, y * BLOCK_HEIGHT), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                            Bengine::ResourceManager::getTexture("Textures/connectedStoneBlock.png"), textureColor);
                        chunk.blocks[x][y] = stoneBlock;
                    }
                }
            }
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

    // Create and initialize the chunk if it doesn't exist
    /*
    Chunk& chunk = m_chunks[x][y];
    if (chunk.blocks[0][0].getTextureID() == 0) {
        chunk.m_worldPosition = glm::vec2(x * CHUNK_WIDTH, y * CHUNK_WIDTH);

        // Populate the chunk with blocks
        for (int i = 0; i < CHUNK_WIDTH; ++i) {
            for (int j = 0; j < CHUNK_WIDTH; ++j) {
                Block block;
                glm::vec2 blockPos(chunk.m_worldPosition.x + i, chunk.m_worldPosition.y + j);
                // Example: Simple block generation
                Bengine::ColorRGBA8 color(255, 255, 255, 255);
                block.init(&m_world, blockPos, glm::vec2(1.0f, 1.0f), Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png"), color, 0.0f);
                chunk.blocks[i][j] = block;
            }
        }
    }
    */
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

    // Determine the chunk coordinates around the player
    int startChunkX = static_cast<int>(playerPos.x) / CHUNK_WIDTH;
    int startChunkY = static_cast<int>(playerPos.y) / CHUNK_WIDTH;

    // Loop through the nearby chunks based on the range
    for (int dx = -range; dx <= range; ++dx) {
        for (int dy = -range; dy <= range; ++dy) {
            int chunkX = startChunkX + dx;
            int chunkY = startChunkY + dy;

            // Check if the chunk is loaded
            if (isChunkLoaded(chunkX, chunkY)) {
                // If the chunk is loaded, add its blocks to the range
                const Chunk& chunk = m_chunks[chunkX][chunkY];
                for (int x = 0; x < CHUNK_WIDTH; ++x) {
                    for (int y = 0; y < CHUNK_WIDTH; ++y) {
                        blocksInRange.push_back(chunk.blocks[x][y]);
                    }
                }
            }
        }
    }

    return blocksInRange;
}