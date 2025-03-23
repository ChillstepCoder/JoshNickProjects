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

    void updateLightingOnBlockBreak(int x, int y);
    void updateLightingOnBlockAdd(int x, int y, BlockID previousBlockID);

    glm::vec3 getLightValue(int x, int y) const;
    Bengine::ColorRGBA8 applyLighting(const Bengine::ColorRGBA8& blockColor, int x, int y) const;

private:

    // Lighting computation methods
    void calculateLighting();
    void propagateLight();
    void enqueueLightNode(int x, int y, unsigned char lightLevel);

    // Helper methods
    bool isValidPosition(int x, int y) const;
    BlockID getBlockIDAt(int x, int y) const;
    bool isBlockSolid(int x, int y) const;
    bool isBlockTransparent(int x, int y) const;


    int width;
    int height;
    std::vector<std::vector<unsigned char>> lightMap; // Single light map instead of separate RGB
    std::vector<LightNode> lightPropagationQueue; // Queue for light propagation
    const Block(*blockGrid)[CHUNK_WIDTH];
    BlockManager* m_blockManager;
};