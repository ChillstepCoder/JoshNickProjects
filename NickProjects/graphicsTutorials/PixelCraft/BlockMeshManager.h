#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"

class DebugDraw;
class BlockManager;
class CellularAutomataManager;

const int CHUNK_WIDTH = 64;
const int WATER_LEVELS = 10;

class Chunk {
public:
    void init();
    void buildChunkMesh() ;
    void render();
    void destroy();

    bool isLoaded() const { return m_isLoaded; }
    glm::vec2 getWorldPosition() {
        return m_worldPosition;
    }
    

    Block blocks[CHUNK_WIDTH][CHUNK_WIDTH];
    std::vector<glm::ivec2> waterBlocks;

    glm::vec2 m_worldPosition;
    Bengine::SpriteBatch m_spriteBatch;
    bool m_isLoaded = false;
    bool m_isMeshDirty = false;
};

struct BlockHandle {
    auto operator<=>(const BlockHandle&) const = default;
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
    BlockManager(BlockMeshManager& meshManager, b2WorldId worldId, CellularAutomataManager& cellularAutomataManager)
        : m_MeshManager(meshManager), m_world(worldId), m_cellularAutomataManager(cellularAutomataManager) {}
    

    void renderBlocks() {
        m_MeshManager.renderMesh(m_chunks, *this);
    }

    void initializeChunks(glm::vec2 playerPosition);

    void update(BlockManager& blockManager);

    BlockHandle getBlockAtPosition(glm::vec2 position);

    glm::ivec2 getBlockWorldPos(glm::ivec2 chunkCoords, glm::ivec2 offset);

    void destroyBlock(const BlockHandle& blockHandle);
    void breakBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos);

    void placeBlock(const BlockHandle& blockHandle, const glm::vec2& position);
    void placeBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos);

    bool isPositionInBlock(const glm::vec2& position, const Block& block);

    void loadNearbyChunks(const glm::vec2& playerPos);

    bool isChunkLoaded(int x, int y);

    void loadChunk(int x, int y);

    bool isChunkFarAway(const glm::vec2& playerPos, const glm::vec2& chunkPos);

    void unloadFarChunks(const glm::vec2& playerPos);

    void unloadChunk(int x, int y);

    std::vector<Block> getBlocksInRange(const glm::vec2& playerPos, int range);

    std::vector<Chunk*> m_activeChunks;

private:
    std::vector<std::vector<Chunk>> m_chunks;

    CellularAutomataManager& m_cellularAutomataManager;
    BlockMeshManager& m_MeshManager;
    b2WorldId m_world;
};