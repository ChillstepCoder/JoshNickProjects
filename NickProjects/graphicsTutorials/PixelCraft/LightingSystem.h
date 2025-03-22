#pragma once

#include <vector>
#include <queue>
#include <glm/glm.hpp>
#include "Bengine/Vertex.h"
#include "Block.h"
#include "GameConstants.h"

class BlockManager;

struct LightNode {
    int x;
    int y;
    unsigned char lightLevel;
};

class LightingSystem {
public:
    LightingSystem();
    ~LightingSystem();

    void init(int worldWidth, int worldHeight);
    void setBlockManager(BlockManager* blockManager);

    void updateLighting();
    glm::vec3 getLightValue(int x, int y) const;
    Bengine::ColorRGBA8 applyLighting(const Bengine::ColorRGBA8& blockColor, int x, int y) const;

private:
    int width;
    int height;
    const Block(*blockGrid)[CHUNK_WIDTH];
    BlockManager* m_blockManager;

    // Single light map instead of separate RGB
    std::vector<std::vector<unsigned char>> lightMap;

    // Queue for light propagation
    std::vector<LightNode> lightPropagationQueue;

    // Lighting computation methods
    void calculateLighting();
    void propagateLight();
    void enqueueLightNode(int x, int y, unsigned char lightLevel);

    // Helper methods
    bool isValidPosition(int x, int y) const;
    BlockID getBlockIDAt(int x, int y) const;
    bool isBlockSolid(int x, int y) const;
    bool isBlockTransparent(int x, int y) const;
};