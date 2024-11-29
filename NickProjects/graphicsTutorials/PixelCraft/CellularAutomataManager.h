#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#pragma once
class CellularAutomataManager
{
public:
    CellularAutomataManager();
    ~CellularAutomataManager();



private:
    glm::ivec2 m_waterBlocks;
};

