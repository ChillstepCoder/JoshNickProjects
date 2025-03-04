#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"
#include "unordered_map"

class DebugDraw;
class BlockManager;
class CellularAutomataManager;
class GameplayScreen;
enum class AdjacencyRule;

const int CHUNK_WIDTH = 64;
const int WATER_LEVELS = 100;

class Chunk {
public:
    void init();
    void buildChunkMesh(BlockManager& blockManager);
    AdjacencyRule getAdjacencyRuleForBlock(BlockID blockID);
    void render();
    void save();
    void load();
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
    bool m_isMeshDirty = true;
};

struct BlockHandle {
    auto operator<=>(const BlockHandle&) const = default;
    Block* block;

    glm::vec2 getWorldPosition();

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
const int WORLD_HEIGHT_CHUNKS = 32;
const int loadRadius = 5;

class BlockManager {
public:
    BlockManager(BlockMeshManager& meshManager, b2WorldId worldId, CellularAutomataManager& cellularAutomataManager)
        : m_MeshManager(meshManager), m_world(worldId), m_cellularAutomataManager(cellularAutomataManager) {}
    

    struct VeinTracker {
        int oreCount;
        int centerX;
        int centerY;
    };
    std::unordered_map<BlockID, std::vector<VeinTracker>> activeVeins;


    void renderBlocks() {
        m_MeshManager.renderMesh(m_chunks, *this);
    }

    void initializeChunks(glm::vec2 playerPosition);

    void update(BlockManager& blockManager);

    BlockHandle getBlockAtPosition(glm::vec2 position);

    Chunk* getChunkAtPosition(glm::vec2 position);

    glm::ivec2 getBlockWorldPos(glm::ivec2 chunkCoords, glm::ivec2 offset);

    void destroyBlock(const BlockHandle& blockHandle);
    void breakBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos);

    void placeBlock(const BlockHandle& blockHandle, const glm::vec2& position);
    void placeBlockAtPosition(const glm::vec2& position, const glm::vec2& playerPos);

    bool isPositionInBlock(const glm::vec2& position, const Block& block);

    void loadNearbyChunks(const glm::vec2& playerPos, BlockManager& blockManager);

    bool isChunkLoaded(int x, int y);

    void generateChunk(int chunkX, int chunkY, Chunk& chunk);

    float generateFractalNoise(int worldX, int worldY, float frequency, float persistence, int octaves, int seed);

    void regenerateWorld(float caveScale, float baseCaveThreshold, float detailScale, float detailInfluence, float minCaveDepth, float surfaceZone, float deepZone, float maxSurfaceBonus, float maxDepthPenalty);

    void loadChunk(int x, int y, BlockManager& blockManager);

    bool saveChunkToFile(int chunkX, int chunkY, Chunk& chunk);

    bool loadChunkFromFile(int chunkX, int chunkY, Chunk& chunk);

    void clearWorldFiles();

    bool isChunkFarAway(const glm::vec2& playerPos, const glm::vec2& chunkPos);

    void unloadFarChunks(const glm::vec2& playerPos);

    void unloadChunk(int x, int y);

    int getConnectedTextureIndex(const Chunk& chunk, int x, int y, BlockID blockID);

    std::vector<BlockHandle> getBlocksInRange(const glm::vec2& playerPos, int range);

    std::vector<Chunk*> m_activeChunks;

private:
    std::vector<std::vector<Chunk>> m_chunks;

    float m_caveScale = 0.005419f;        // higher number = smaller cave
    float m_baseCaveThreshold = 0.3f; // Higher = less caves
    float m_detailScale = 0.09320f;       // Scale for additional cave detail
    float m_detailInfluence = 0.77f;   // How much the detail affects the main cave shape
    float m_minCaveDepth = 20.0f;      // Minimum depth below surface for caves to start
    float m_surfaceZone = 100.0f;      // Depth range for surface cave adjustment
    float m_deepZone = 600.0f;         // Depth where deep cave adjustment begins
    float m_maxSurfaceBonus = 0.02f;   // Maximum bonus for surface caves
    float m_maxDepthPenalty = 0.01f;   // Maximum penalty for deep caves

    CellularAutomataManager& m_cellularAutomataManager;
    BlockMeshManager& m_MeshManager;
    b2WorldId m_world;
};