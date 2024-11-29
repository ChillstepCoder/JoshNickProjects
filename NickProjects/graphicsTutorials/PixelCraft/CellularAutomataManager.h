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

    void init(std::vector <Block&> waterblocks);

    void simulateWater(Chunk& chunk, BlockManager& blockManager);
    float getStableState(float mass); //Returns the amount of water that should be in the bottom cell.
    float constrain(float val, float min, float max);
    float minVal(float a, float b);

private:
    std::vector <Block&> m_waterBlocks; // store a glm::ivec2 position
    float m_maxLiquid = 1.0; //The normal, un-pressurized mass of a full water cell
    float m_maxCompress = 0.02; //How much excess water a cell can store, compared to the cell above it
    float m_minMass = 0.0001;  //Ignore cells that are almost dry
};

