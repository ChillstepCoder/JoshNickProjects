#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"

class DebugDraw;

const int CHUNK_WIDTH = 64;

class Chunk {
public:

    Block blocks[CHUNK_WIDTH][CHUNK_WIDTH];
    glm::vec2 getWorldPosition() {
        return m_worldPosition;
    }

    glm::vec2 m_worldPosition;

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
    void buildMesh(const std::vector<std::vector<Chunk>>& chunks); // Accept blocks from BlockManager
    void renderMesh();

private:
    Bengine::SpriteBatch m_spriteBatch;
};

const int WORLD_WIDTH_CHUNKS = 16;
const int WORLD_HEIGHT_CHUNKS = 8;
const int loadRadius = 5;

class BlockManager {
public:
    BlockManager(BlockMeshManager& meshManager, b2WorldId worldId) : m_MeshManager(meshManager), m_world(worldId) {}

    //void addBlock(const Block& block) {
    //    m_blocks.push_back(block);
    //}

    void rebuildMesh() {
        m_MeshManager.buildMesh(m_chunks);
    }

    void renderBlocks() {
        m_MeshManager.renderMesh();
    }

    //std::vector<Block>& getBlocks() {
    //    return m_blocks;
    //}

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