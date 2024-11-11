#include <Bengine/ResourceManager.h>
#include "BlockMeshManager.h"
#include "DebugDraw.h"
#include "PerlinNoise.hpp"

BlockMeshManager::BlockMeshManager() {

}

BlockMeshManager::~BlockMeshManager() {

}

void BlockMeshManager::init() {
    m_spriteBatch.init();
}
void BlockMeshManager::buildMesh(const std::vector<Block>& blocks) {
    m_spriteBatch.begin();
    for (const auto& block : blocks) {
        auto destRect = block.getDestRect();
        auto uvRect = block.getUVRect();
        auto textureID = block.getTextureID();
        auto color = block.getColor();

        m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, color, 0.0f);
    }
    m_spriteBatch.end();
}
void BlockMeshManager::renderMesh() {
    m_spriteBatch.renderBatch();
}

void BlockManager::generateChunks() {
    // Create Perlin noise instance with random seed
    siv::PerlinNoise perlin(12345); // Can change this seed for different terrain

    // Parameters for terrain generation
#ifdef _DEBUG
    // MAKE A WAY SMALLER WORLD IN DEBUG MODE SO ITS FASTER TO LOAD
    const int NUM_BLOCKS_X = 250;  // Width of the terrain
#else
    const int NUM_BLOCKS_X = 1500;  // Width of the terrain
#endif
    const float BLOCK_WIDTH = 1.0f;
    const float BLOCK_HEIGHT = 1.0f;
    const float START_X = -NUM_BLOCKS_X / 2;
    // Parameters for noise
    const float NOISE_SCALE = 0.05f;  // Controls how stretched the noise is
    const float AMPLITUDE = 10.0f;    // Controls the height variation
    const float SURFACE_Y = 14.0f;    // Base height of the surface

    Bengine::ColorRGBA8 textureColor;
    textureColor.r = 255;
    textureColor.g = 255;
    textureColor.b = 255;
    textureColor.a = 255;

    // Generate surface terrain
    std::vector<int> heightMap(NUM_BLOCKS_X);
    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        // Generate height using Perlin noise
        float noiseValue = perlin.noise1D(x * NOISE_SCALE);
        int height = static_cast<int>(SURFACE_Y + noiseValue * AMPLITUDE);
        heightMap[x] = height;

        // Create surface (grass) blocks
        float worldX = START_X + x * BLOCK_WIDTH;
        Block surfaceBlock;
        glm::vec2 position(worldX, height * BLOCK_HEIGHT);
        surfaceBlock.init(&m_world, position, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
            Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png"), textureColor, false);
        m_blocks.push_back(surfaceBlock);

        // Fill blocks below surface with dirt
        for (int y = height - 1; y > height - 10; y--) {
            Block dirtBlock;
            glm::vec2 dirtPos(worldX, y * BLOCK_HEIGHT);
            dirtBlock.init(&m_world, dirtPos, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png"), textureColor, false);
            m_blocks.push_back(dirtBlock);
        }

        // Fill deeper blocks with stone
        for (int y = height - 10; y > height - 50; y--) {
            Block stoneBlock;
            glm::vec2 stonePos(worldX, y * BLOCK_HEIGHT);
            stoneBlock.init(&m_world, stonePos, glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                Bengine::ResourceManager::getTexture("Textures/connectedStoneBlock.png"), textureColor, false);
            m_blocks.push_back(stoneBlock);
        }
    }

    rebuildMesh();

    /*
    // Generate caves using 2D Perlin noise
    const float CAVE_SCALE = 0.4f;
    const float CAVE_THRESHOLD = 0.4f; // Adjust this to control cave density

    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        for (int y = heightMap[x] - 8; y > heightMap[x] - 35; y--) {
            float caveNoise = perlin.noise2D(x * CAVE_SCALE, y * CAVE_SCALE);
            if (caveNoise > CAVE_THRESHOLD) {
                // Find and remove blocks at this position
                float worldX = START_X + x * BLOCK_WIDTH;
                float worldY = y * BLOCK_HEIGHT;

                auto it = std::remove_if(m_blockManager->getBlocks().begin(), m_blockManager->getBlocks().end(),
                    [worldX, worldY, BLOCK_WIDTH, BLOCK_HEIGHT](Block& block) {
                        b2Vec2 pos = block.getPosition();
                        return (pos.x >= worldX - BLOCK_WIDTH / 2 &&
                            pos.x <= worldX + BLOCK_WIDTH / 2 &&
                            pos.y >= worldY - BLOCK_HEIGHT / 2 &&
                            pos.y <= worldY + BLOCK_HEIGHT / 2);
                    });

                m_blockManager->getBlocks().erase(it, m_blockManager->getBlocks().end());
            }
        }
    }

    // Load the copper ore texture
    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedCopperBlock.png");

    // Parameters for ore vein generation
    const float ORE_VEIN_SCALE = 0.03f;  // Controls how stretched the ore veins are
    const float ORE_VEIN_THRESHOLD = 0.25f; // Adjust this to control ore vein density
    const int MIN_VEIN_LENGTH = 5;      // Minimum length of an ore vein
    const int MAX_VEIN_LENGTH = 9;     // Maximum length of an ore vein

    // Generate copper ore veins
    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        for (int y = heightMap[x] - 12; y > heightMap[x] - 35; y--) {
            float oreNoise = perlin.noise2D(x * ORE_VEIN_SCALE, y * ORE_VEIN_SCALE);
            if (oreNoise > ORE_VEIN_THRESHOLD) {
                // Start a new ore vein
                int veinLength = MIN_VEIN_LENGTH + rand() % (MAX_VEIN_LENGTH - MIN_VEIN_LENGTH + 1);
                int veinStartX = x;
                int veinStartY = y;

                for (int i = 0; i < veinLength; i++) {
                    // Create ore blocks along the vein
                    float worldX = START_X + veinStartX * BLOCK_WIDTH;
                    float worldY = veinStartY * BLOCK_HEIGHT;
                    Block oreBlock;
                    oreBlock.init(&m_world, glm::vec2(worldX, worldY), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                        m_texture, textureColor, false);
                    m_blockManager->addBlock(oreBlock);

                    // Move to the next position in the vein
                    veinStartX++;
                    if (veinStartX >= NUM_BLOCKS_X) {
                        // Wrap around to the beginning if we reach the end
                        veinStartX = 0;
                    }
                }
            }
        }
    }

    m_texture = Bengine::ResourceManager::getTexture("Textures/connectedIronBlock.png");

    // Parameters for iron vein generation
    const float ORE_VEIN_SCALE2 = 0.02f;  // Controls how stretched the ore veins are
    const float ORE_VEIN_THRESHOLD2 = 0.45f; // Adjust this to control ore vein density
    const int MIN_VEIN_LENGTH2 = 5;      // Minimum length of an ore vein
    const int MAX_VEIN_LENGTH2 = 9;     // Maximum length of an ore vein

    // Generate iron ore veins
    for (int x = 0; x < NUM_BLOCKS_X; x++) {
        for (int y = heightMap[x] - 12; y > heightMap[x] - 35; y--) {
            float oreNoise = perlin.noise2D(x * ORE_VEIN_SCALE2, y * ORE_VEIN_SCALE2);
            if (oreNoise > ORE_VEIN_THRESHOLD2) {
                // Start a new ore vein
                int veinLength = MIN_VEIN_LENGTH2 + rand() % (MAX_VEIN_LENGTH2 - MIN_VEIN_LENGTH2 + 1);
                int veinStartX = x;
                int veinStartY = y;

                for (int i = 0; i < veinLength; i++) {
                    // Create ore blocks along the vein
                    float worldX = START_X + veinStartX * BLOCK_WIDTH;
                    float worldY = veinStartY * BLOCK_HEIGHT;
                    Block oreBlock;
                    oreBlock.init(&m_world, glm::vec2(worldX, worldY), glm::vec2(BLOCK_WIDTH, BLOCK_HEIGHT),
                        m_texture, textureColor, false);
                    m_blockManager->addBlock(oreBlock);

                    // Move to the next position in the vein
                    veinStartX++;
                    if (veinStartX >= NUM_BLOCKS_X) {
                        // Wrap around to the beginning if we reach the end
                        veinStartX = 0;
                    }
                }
            }
        }
    }
    */
}


void BlockManager::breakBlockAtPosition(const glm::vec2& position) {  //destRect.x = (getPosition().x - ((0.5) * getDimensions().x));
    // Iterate through blocks and find the one that contains the given position

    for (int i = 0; i < m_blocks.size(); ++i) {
        const Block& block = m_blocks[i];
        if (isPositionInBlock(position, block)) {
            // Remove the block from the world
            b2DestroyBody(block.getID());
            m_blocks.erase(m_blocks.begin() + i);

            // Rebuild the mesh
            rebuildMesh();
            DebugDraw::getInstance().setVertexDataChanged(true);
            break;
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
    // Makes sure the chunk is within world bounds
    if (x < 0 || y < 0 || x >= WORLD_WIDTH_CHUNKS || y >= WORLD_HEIGHT_CHUNKS) {
        return false;
    }
    if (x >= 0 && x < m_chunks.size() && y >= 0 && y < m_chunks[x].size()) {
        return m_chunks[x][y].getWorldPosition() != glm::vec2(0.0f, 0.0f);
    }
    return false;
}

void BlockManager::loadChunk(int x, int y) {
    if (x < 0 || y < 0 || x >= WORLD_WIDTH_CHUNKS || y >= WORLD_HEIGHT_CHUNKS) {
        return; // Out of bounds
    }

    if (x >= m_chunks.size()) {
        m_chunks.resize(x + 1);  // Resize outer vector if x is out of bounds
    }

    if (y >= m_chunks[x].size()) {
        m_chunks[x].resize(y + 1);  // Resize the inner vector if y is out of bounds
    }


    Chunk chunk;
    chunk.m_worldPosition = glm::vec2(x * CHUNK_WIDTH, y * CHUNK_WIDTH);

    m_chunks[x][y] = chunk;
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