#include "CellularAutomataManager.h"

CellularAutomataManager::CellularAutomataManager() {

}
CellularAutomataManager::~CellularAutomataManager() {

}

void CellularAutomataManager::init(std::vector <Block&> waterblocks) {
    m_waterBlocks = waterblocks;
}

void CellularAutomataManager::simulateWater(Chunk& chunk, BlockManager& blockManager) {

    glm::vec2 mass = glm::vec2(CHUNK_WIDTH + 2.0f, CHUNK_WIDTH + 2.0f);
    glm::vec2 newMass = glm::vec2(CHUNK_WIDTH + 2.0f, CHUNK_WIDTH + 2.0f);

    float Flow = 0.0f;
    float MinFlow = 0.01;
    float MaxSpeed = 1;   //max units of water moved out of one block to another, per timestep
    float remaining_mass = 0.0f;


    for (int i = 0; i < blockManager.m_activeChunks.size(); i++) { // Loop through the active chunks to simulate water physics



        //Calculate and apply flow for each block
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            for (int y = 0; y < CHUNK_WIDTH; y++) {


                //Skip inert ground blocks
                if (blockManager.m_activeChunks[x][y].blocks[x][y].getBlockID() != BlockID::WATER) continue;


                float waterBlockMass = blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount();


                //Custom push-only flow
                Flow = 0;
                remaining_mass = waterBlockMass;
                if (remaining_mass <= 0) continue;

                //If the block below this one is air or water,
                if (blockManager.m_activeChunks[x][y].blocks[x][y - 1].getBlockID() == BlockID::AIR || blockManager.m_activeChunks[x][y].blocks[x][y - 1].getBlockID() == BlockID::WATER) {
                    Flow = getStableState(remaining_mass + blockManager.m_activeChunks[x][y].blocks[x][y - 1].getWaterAmount()) - blockManager.m_activeChunks[x][y].blocks[x][y - 1].getWaterAmount();
                    if (Flow > MinFlow) {
                        Flow *= 0.5; //leads to smoother flow
                    }
                    Flow = constrain(Flow, 0, std::min(MaxSpeed, remaining_mass));

                    blockManager.m_activeChunks[x][y].blocks[x][y].setWaterAmount(blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() - Flow);
                    blockManager.m_activeChunks[x][y].blocks[x][y - 1].setWaterAmount(Flow);
                    remaining_mass -= Flow;
                }

                if (remaining_mass <= 0) continue;

                //Left
                if (blockManager.m_activeChunks[x][y].blocks[x - 1][y].getBlockID() == BlockID::AIR || blockManager.m_activeChunks[x][y].blocks[x - 1][y].getBlockID() == BlockID::WATER) {
                    //Equalize the amount of water in this block and it's neighbour
                    Flow = (blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() - blockManager.m_activeChunks[x][y].blocks[x - 1][y].getWaterAmount()) / 4;
                    if (Flow > MinFlow) { Flow *= 0.5; }
                    Flow = constrain(Flow, 0, remaining_mass);

                    blockManager.m_activeChunks[x][y].blocks[x][y].setWaterAmount(blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() - Flow);
                    blockManager.m_activeChunks[x][y].blocks[x - 1][y].setWaterAmount(Flow);
                    remaining_mass -= Flow;
                }

                if (remaining_mass <= 0) continue;

                //Right
                if (blockManager.m_activeChunks[x][y].blocks[x + 1][y].getBlockID() == BlockID::AIR || blockManager.m_activeChunks[x][y].blocks[x + 1][y].getBlockID() == BlockID::WATER) {
                    //Equalize the amount of water in this block and it's neighbour
                    Flow = (blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() - blockManager.m_activeChunks[x][y].blocks[x + 1][y].getWaterAmount()) / 4;
                    if (Flow > MinFlow) { Flow *= 0.5; }
                    Flow = constrain(Flow, 0, remaining_mass);

                    blockManager.m_activeChunks[x][y].blocks[x][y].setWaterAmount(blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() - Flow);
                    blockManager.m_activeChunks[x][y].blocks[x + 1][y].setWaterAmount(Flow);
                    remaining_mass -= Flow;
                }

                if (remaining_mass <= 0) continue;

                //Up. Only compressed water flows upwards.
                if (blockManager.m_activeChunks[x][y].blocks[x][y + 1].getBlockID() == BlockID::AIR || blockManager.m_activeChunks[x][y].blocks[x][y + 1].getBlockID() == BlockID::WATER) {
                    Flow = remaining_mass - getStableState(remaining_mass + blockManager.m_activeChunks[x][y].blocks[x][y + 1].getWaterAmount());
                    if (Flow > MinFlow) { Flow *= 0.5; }
                    Flow = constrain(Flow, 0, std::min(MaxSpeed, remaining_mass));

                    blockManager.m_activeChunks[x][y].blocks[x][y].setWaterAmount(blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() - Flow);
                    blockManager.m_activeChunks[x][y].blocks[x][y + 1].setWaterAmount(Flow);
                    remaining_mass -= Flow;
                }
            }
        }


        //Copy the new mass values to the mass array
        for (int x = 0; x < CHUNK_WIDTH + 2; x++) {
            for (int y = 0; y < CHUNK_WIDTH + 2; y++) {
                blockManager.m_activeChunks[x][y].blocks[x][y] = blockManager.m_activeChunks[x][y].blocks[x][y];
            }
        }

        for (int x = 1; x <= CHUNK_WIDTH; x++) {
            for (int y = 1; y <= CHUNK_WIDTH; y++) {
                //Skip ground blocks
                if (blockManager.m_activeChunks[x][y].blocks[x][y].getBlockID() != BlockID::WATER) continue;
                //Flag/unflag water blocks
                if (blockManager.m_activeChunks[x][y].blocks[x][y].getWaterAmount() > m_minMass) {
                    blockManager.m_activeChunks[x][y].blocks[x][y].setBlockID(BlockID::WATER);
                }
                else {
                    blockManager.m_activeChunks[x][y].blocks[x][y].setBlockID(BlockID::AIR);
                }
            }
        }
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