#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"

class DebugDraw;

class BlockMeshManager
{
public:
    BlockMeshManager();
    ~BlockMeshManager();

    void init();
    void buildMesh(const std::vector<Block>& blocks); // Accept blocks from BlockManager
    void renderMesh();

private:
    Bengine::SpriteBatch m_spriteBatch;
};

const int CHUNK_WIDTH = 64;

class Chunk {
public:

    Block blocks[CHUNK_WIDTH][CHUNK_WIDTH];
    glm::vec2 getWorldPosition() {
        return m_worldPosition;
    }

    void generateChunks();

    glm::vec2 m_worldPosition;

};

const int WORLD_WIDTH_CHUNKS = 128;
const int WORLD_HEIGHT_CHUNKS = 64;

class BlockManager {
public:
    BlockManager(BlockMeshManager& meshManager, b2WorldId worldId) : m_MeshManager(meshManager) {}

    void addBlock(const Block& block) {
        m_blocks.push_back(block);
    }

    void rebuildMesh() {
        m_MeshManager.buildMesh(m_blocks);
    }

    void renderBlocks() {
        m_MeshManager.renderMesh();
    }

    std::vector<Block>& getBlocks() {
        return m_blocks;
    }

    void breakBlockAtPosition(const glm::vec2& position);

    bool isPositionInBlock(const glm::vec2& position, const Block& block);

    void loadNearbyChunks(const glm::vec2& playerPos);

    bool isChunkLoaded(int x, int y);

    void loadChunk(int x, int y);

    bool isChunkFarAway(const glm::vec2& playerPos, const glm::vec2& chunkPos);

    void unloadChunks(const glm::vec2& playerPos);

    void unloadChunk(int x, int y);

private:
    std::vector<std::vector<Chunk>> m_chunks;

    BlockMeshManager& m_MeshManager;
    std::vector<Block> m_blocks;
    b2WorldId m_world = b2_nullWorldId;
};