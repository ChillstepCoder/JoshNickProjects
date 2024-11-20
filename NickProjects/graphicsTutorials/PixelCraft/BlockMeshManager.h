#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"

class DebugDraw;
class BlockManager;

const int CHUNK_WIDTH = 64;

class Chunk {
public:

    Block blocks[CHUNK_WIDTH][CHUNK_WIDTH];

    glm::vec2 m_worldPosition;
    Bengine::SpriteBatch m_spriteBatch;

    glm::vec2 getWorldPosition() {
        return m_worldPosition;
    }

    void init();
    void buildChunkMesh() ;
    void render();
    void destroy();
};

struct BlockHandle {
    Block* block;
    glm::ivec2 chunkCoords;
    glm::ivec2 blockOffset;
};

class BlockMeshManager
{
public:
    BlockMeshManager();
    ~BlockMeshManager();

    void init();
    void renderMesh(std::vector<std::vector<Chunk>>& chunks, BlockManager& blockManager);

private:
};

const int WORLD_WIDTH_CHUNKS = 32;
const int WORLD_HEIGHT_CHUNKS = 16;
const int loadRadius = 5;

class BlockManager {
public:
    BlockManager(BlockMeshManager& meshManager, b2WorldId worldId)
        : m_MeshManager(meshManager), m_world(worldId){}
    

    void renderBlocks() {
        m_MeshManager.renderMesh(m_chunks, *this);
    }

    void initializeChunks();

    BlockHandle getBlockAtPosition(glm::vec2 position);

    void destroyBlock(const BlockHandle& blockHandle);

    void breakBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos);

    bool isPositionInBlock(const glm::vec2& position, const Block& block);

    void loadNearbyChunks(const glm::vec2& playerPos);

    bool isChunkLoaded(int x, int y);

    void loadChunk(int x, int y);

    bool isChunkFarAway(const glm::vec2& playerPos, const glm::vec2& chunkPos);

    void unloadFarChunks(const glm::vec2& playerPos);

    void unloadChunk(int x, int y);

    std::vector<Block> getBlocksInRange(const glm::vec2& playerPos, int range);

private:
    std::vector<std::vector<Chunk>> m_chunks;

    BlockMeshManager& m_MeshManager;
    b2WorldId m_world;
};