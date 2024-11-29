#include "CellularAutomataManager.h"

CellularAutomataManager::CellularAutomataManager() {

}
CellularAutomataManager::~CellularAutomataManager() {

}

void CellularAutomataManager::init(std::vector <Block&> waterblocks) {
    m_waterBlocks = waterblocks;
}

void CellularAutomataManager::simulateWater(Chunk& chunk, BlockManager& blockManager) {


    blockHandle.block
    const Block& block = blocks[x][y];

    glm::vec2 mass = glm::vec2(CHUNK_WIDTH + 2.0f, CHUNK_WIDTH + 2.0f);
    glm::vec2 newMass = glm::vec2(CHUNK_WIDTH + 2.0f, CHUNK_WIDTH + 2.0f);

    float Flow = 0.0f;
    float MinFlow = 0.01;
    float MaxSpeed = 1;   //max units of water moved out of one block to another, per timestep
    float remaining_mass = 0.0f;

    //Calculate and apply flow for each block
    for (int x = 1; x <= CHUNK_WIDTH; x++) {
        for (int y = 1; y <= CHUNK_WIDTH; y++) {
            float realpositionX = position.x + 0.5f;
            float realPositionY = position.y + 0.5f;

            BlockHandle blockHandle = getBlockAtPosition(glm::vec2(realpositionX, realPositionY));



            //Skip inert ground blocks
            if (blocks[x][y] == GROUND) continue;

            //Custom push-only flow
            Flow = 0;
            remaining_mass = mass[x][y];
            if (remaining_mass <= 0) continue;

            //The block below this one
            if ((blocks[x][y - 1] != GROUND)) {
                Flow = get_stable_state_b(remaining_mass + mass[x][y - 1]) - mass[x][y - 1];
                if (Flow > MinFlow) {
                    Flow *= 0.5; //leads to smoother flow
                }
                Flow = constrain(Flow, 0, std::min(MaxSpeed, remaining_mass));

                new_mass[x][y] -= Flow;
                new_mass[x][y - 1] += Flow;
                remaining_mass -= Flow;
            }

            if (remaining_mass <= 0) continue;

            //Left
            if (blocks[x - 1][y] != GROUND) {
                //Equalize the amount of water in this block and it's neighbour
                Flow = (mass[x][y] - mass[x - 1][y]) / 4;
                if (Flow > MinFlow) { Flow *= 0.5; }
                Flow = constrain(Flow, 0, remaining_mass);

                new_mass[x][y] -= Flow;
                new_mass[x - 1][y] += Flow;
                remaining_mass -= Flow;
            }

            if (remaining_mass <= 0) continue;

            //Right
            if (blocks[x + 1][y] != GROUND) {
                //Equalize the amount of water in this block and it's neighbour
                Flow = (mass[x][y] - mass[x + 1][y]) / 4;
                if (Flow > MinFlow) { Flow *= 0.5; }
                Flow = constrain(Flow, 0, remaining_mass);

                new_mass[x][y] -= Flow;
                new_mass[x + 1][y] += Flow;
                remaining_mass -= Flow;
            }

            if (remaining_mass <= 0) continue;

            //Up. Only compressed water flows upwards.
            if (blocks[x][y + 1] != GROUND) {
                Flow = remaining_mass - get_stable_state_b(remaining_mass + mass[x][y + 1]);
                if (Flow > MinFlow) { Flow *= 0.5; }
                Flow = constrain(Flow, 0, min(MaxSpeed, remaining_mass));

                new_mass[x][y] -= Flow;
                new_mass[x][y + 1] += Flow;
                remaining_mass -= Flow;
            }


        }
    }

    //Copy the new mass values to the mass array
    for (int x = 0; x < CHUNK_WIDTH + 2; x++) {
        for (int y = 0; y < CHUNK_WIDTH + 2; y++) {
            mass[x][y] = new_mass[x][y];
        }
    }

    for (int x = 1; x <= CHUNK_WIDTH; x++) {
        for (int y = 1; y <= CHUNK_WIDTH; y++) {
            //Skip ground blocks
            if (blocks[x][y] == GROUND) continue;
            //Flag/unflag water blocks
            if (mass[x][y] > m_minMass) {
                blocks[x][y] = WATER;
            }
            else {
                blocks[x][y] = AIR;
            }
        }
    }

    //Remove any water that has left the map
    for (int x = 0; x < CHUNK_WIDTH + 2; x++) {
        mass[x][0] = 0;
        mass[x][CHUNK_WIDTH + 1] = 0;
    }
    for (int y = 1; y < CHUNK_WIDTH + 1; y++) {
        mass[0][y] = 0;
        mass[CHUNK_WIDTH + 1][y] = 0;
    }

}

float CellularAutomataManager::getStableState(float mass) {
    if (mass <= 1.0f) {
        return 1.0f;
    }
    else if (mass < 2.0f * m_maxLiquid + m_maxCompress) {
        return (m_maxLiquid * m_maxLiquid + mass * m_maxCompress) / (m_maxLiquid + m_maxCompress);
    }
    else {
        return (mass + m_maxCompress) / 2.0f;
    }
}

float CellularAutomataManager::constrain(float val, float min, float max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

float CellularAutomataManager::minVal(float a, float b)
{
    return a > b ? b : a;
}