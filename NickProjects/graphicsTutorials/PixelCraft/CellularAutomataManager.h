#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "BlockMeshManager.h"

#pragma once
class CellularAutomataManager
{
public:
    CellularAutomataManager();
    ~CellularAutomataManager();

    void init();

    void simulateWater(Chunk& chunk, BlockManager& blockManager, LightingSystem& lightingSystem);

private:
    // Returns true if our original block has no more water
    bool moveWaterToBlock(BlockHandle& sourceBlock, BlockHandle& targetBlock, glm::vec2 targetPos, int amountToPush, BlockManager& blockManager, LightingSystem& lightingSystem);
    void splitWaterToEmpty(BlockHandle& sourceBlock, BlockHandle& targetBlock, glm::vec2 targetPos, BlockManager& blockManager, LightingSystem& lightingSystem);
    bool moveWaterDiagonally(BlockHandle& sourceBlock, BlockHandle& diagonalBlock, glm::vec2 diagonalPos, BlockHandle& adjacentBlock, glm::vec2 adjacentPos, BlockManager& blockManager, LightingSystem& lightingSystem);
    BlockHandle getBlockAtPositionSafely(BlockManager& blockManager, glm::vec2 position);

    std::vector <Block*> m_waterBlocks; // store a glm::ivec2 position
};

