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
   // float getStableState(float mass); //Returns the amount of water that should be in the bottom cell.
   // float constrain(float val, float min, float max);
  //  float minVal(float a, float b);

private:
    // Returns true if our original block has no more water
    bool moveWaterToBlock(BlockHandle& sourceBlock, BlockHandle& targetBlock, int amountToPush, BlockManager& blockManager);
    bool moveWaterDiagonally(BlockHandle& sourceBlock, BlockHandle& diagonalBlock, BlockHandle& adjacentBlock, BlockManager& blockManager);

    std::vector <Block*> m_waterBlocks; // store a glm::ivec2 position
};

