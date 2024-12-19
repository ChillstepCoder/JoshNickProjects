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

    void simulateWater(Chunk& chunk, BlockManager& blockManager);

private:
    // Returns true if our original block has no more water
    bool moveWaterToBlock(BlockHandle& sourceBlock, BlockHandle& targetBlock, glm::vec2 targetPos, int amountToPush, BlockManager& blockManager);
    bool moveWaterDiagonally(BlockHandle& sourceBlock, BlockHandle& diagonalBlock, glm::vec2 diagonalPos, BlockHandle& adjacentBlock, glm::vec2 adjacentPos, BlockManager& blockManager);

    std::vector <Block*> m_waterBlocks; // store a glm::ivec2 position
};

