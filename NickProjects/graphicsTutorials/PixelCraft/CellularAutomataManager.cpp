#include "CellularAutomataManager.h"
#include <iostream>

CellularAutomataManager::CellularAutomataManager() {

}
CellularAutomataManager::~CellularAutomataManager() {

}

void CellularAutomataManager::init() {

}

void CellularAutomataManager::simulateWater(Chunk& chunk, BlockManager& blockManager) {
    /*
    glm::vec2 mass = glm::vec2(CHUNK_WIDTH, CHUNK_WIDTH);
    glm::vec2 newMass = glm::vec2(CHUNK_WIDTH, CHUNK_WIDTH);

    float Flow = 0.0f;
    float MinFlow = 0.01f;
    float MaxSpeed = 1.0f;   //max units of water moved out of one block to another, per timestep
    float remaining_mass = 0.0f;
    */





    //Calculate and apply flow for each block
    for (int i = chunk.waterBlocks.size() - 1; i >= 0; --i) {

        int waterPosX = chunk.waterBlocks[i].x + 0.5f;
        int waterPosY = chunk.waterBlocks[i].y + 0.5f;

        int downPosX = waterPosX;
        int downPosY = (waterPosY - 1);

        int downRightPosX = waterPosX + 1;
        int downRightPosY = (waterPosY - 1);

        int downLeftPosX = waterPosX - 1;
        int downLeftPosY = (waterPosY - 1);

        int leftPosX = (waterPosX - 1);
        int leftPosY = waterPosY;

        int rightPosX = (waterPosX + 1);
        int rightPosY = waterPosY;

        int upPosX = waterPosX;
        int upPosY = (waterPosY + 1);

        BlockHandle waterBlock = blockManager.getBlockAtPosition(glm::vec2(waterPosX, waterPosY));

        BlockHandle downBlock = blockManager.getBlockAtPosition(glm::vec2(downPosX, downPosY));

        BlockHandle downRightBlock = blockManager.getBlockAtPosition(glm::vec2(downRightPosX, downRightPosY));

        BlockHandle downLeftBlock = blockManager.getBlockAtPosition(glm::vec2(downLeftPosX, downLeftPosY));

        BlockHandle leftBlock = blockManager.getBlockAtPosition(glm::vec2(leftPosX, leftPosY));

        BlockHandle rightBlock = blockManager.getBlockAtPosition(glm::vec2(rightPosX, rightPosY));

        BlockHandle upBlock = blockManager.getBlockAtPosition(glm::vec2(upPosX, upPosY));

        float waterBlockMass = waterBlock.block->getWaterAmount();




        if (downBlock.block->getBlockID() == BlockID::AIR) {

            waterBlock.block->setWaterAmount(0.0f);
            blockManager.destroyBlock(waterBlock);

            downBlock.block->setWaterAmount(waterBlock.block->getWaterAmount());
            blockManager.placeBlock(downBlock, glm::vec2(downPosX, downPosY));

            continue;

        } else if (downRightBlock.block->getBlockID() == BlockID::AIR) {

            waterBlock.block->setWaterAmount(0.0f);
            blockManager.destroyBlock(waterBlock);

            downRightBlock.block->setWaterAmount(waterBlock.block->getWaterAmount());
            blockManager.placeBlock(downRightBlock, glm::vec2(downRightPosX, downRightPosY));

            continue;
        } else if (downLeftBlock.block->getBlockID() == BlockID::AIR) {

            waterBlock.block->setWaterAmount(0.0f);
            blockManager.destroyBlock(waterBlock);

            downLeftBlock.block->setWaterAmount(waterBlock.block->getWaterAmount());
            blockManager.placeBlock(downLeftBlock, glm::vec2(downLeftPosX, downLeftPosY));

            continue;
        } else if (rightBlock.block->getBlockID() == BlockID::AIR || rightBlock.block->getBlockID() == BlockID::WATER) {
            if (((float)((int)((rightBlock.block->getWaterAmount() * 10))) / 10) < ((float)((int)((waterBlock.block->getWaterAmount() * 10))) / 10)) {
                float waterAmt = ((float)((int)((waterBlock.block->getWaterAmount() * 10))) / 10);
                float rightWaterAmt = ((float)((int)((rightBlock.block->getWaterAmount() * 10))) / 10);

                std::cout << "waterAmt: " << waterAmt << "  rightWaterAmt: " << rightWaterAmt << std::endl;


                float avgAmt = (waterAmt + rightWaterAmt) / 2.0f;


                waterBlock.block->setWaterAmount(avgAmt);
 
                blockManager.placeBlock(rightBlock, glm::vec2(rightPosX, rightPosY));
                rightBlock.block->setWaterAmount(avgAmt);

                continue;
            } else {
                continue;
            }

        } else if (leftBlock.block->getBlockID() == BlockID::AIR || leftBlock.block->getBlockID() == BlockID::WATER) {
            if (((float)((int)((leftBlock.block->getWaterAmount() * 10))) / 10) < ((float)((int)((waterBlock.block->getWaterAmount() * 10))) / 10)) {
                float waterAmt = ((float)((int)((waterBlock.block->getWaterAmount() * 10))) / 10);
                float leftWaterAmt = ((float)((int)((leftBlock.block->getWaterAmount() * 10))) / 10);

                std::cout << "waterAmt: " << waterAmt << "  leftWaterAmt: " << leftWaterAmt << std::endl;

                float avgAmt = (waterAmt + leftWaterAmt) / 2.0f;

                waterBlock.block->setWaterAmount(avgAmt);
                blockManager.destroyBlock(waterBlock);

                blockManager.placeBlock(leftBlock, glm::vec2(leftPosX, leftPosY));
                leftBlock.block->setWaterAmount(avgAmt);

                continue;
            } else {
                continue;
            }

        }
        else {
            continue;
        }
    }

    if (chunk.m_isMeshDirty == true) {
        chunk.m_isMeshDirty = false;
        chunk.buildChunkMesh();
    }

}

















        /*

        //Custom push-only flow
        Flow = 0;
        remaining_mass = waterBlockMass;
        if (remaining_mass <= 0) continue;

        //If the block below this one is air or water,
        if (downBlock.block->getBlockID() == BlockID::AIR || downBlock.block->getBlockID() == BlockID::WATER) {
            Flow = getStableState(remaining_mass + downBlock.block->getWaterAmount()) - downBlock.block->getWaterAmount();
            if (Flow > MinFlow) {
                Flow *= 0.5; //leads to smoother flow
            }
            Flow = constrain(Flow, 0, std::min(MaxSpeed, remaining_mass));

            waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - Flow);
            downBlock.block->setWaterAmount(Flow);
            remaining_mass -= Flow;
        }

        if (remaining_mass <= 0) continue;

        //Left
        if (leftBlock.block->getBlockID() == BlockID::AIR || leftBlock.block->getBlockID() == BlockID::WATER) {
            //Equalize the amount of water in this block and it's neighbour
            Flow = (waterBlock.block->getWaterAmount() - leftBlock.block->getWaterAmount()) / 4;
            if (Flow > MinFlow) { Flow *= 0.5; }
            Flow = constrain(Flow, 0, remaining_mass);

            waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - Flow);
            leftBlock.block->setWaterAmount(Flow);
            remaining_mass -= Flow;
        }

        if (remaining_mass <= 0) continue;

        //Right
        if (rightBlock.block->getBlockID() == BlockID::AIR || rightBlock.block->getBlockID() == BlockID::WATER) {
            //Equalize the amount of water in this block and it's neighbour
            Flow = (waterBlock.block->getWaterAmount() - rightBlock.block->getWaterAmount()) / 4;
            if (Flow > MinFlow) { Flow *= 0.5; }
            Flow = constrain(Flow, 0, remaining_mass);

            waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - Flow);
            rightBlock.block->setWaterAmount(Flow);
            remaining_mass -= Flow;
        }

        if (remaining_mass <= 0) continue;

        //Up. Only compressed water flows upwards.
        if (upBlock.block->getBlockID() == BlockID::AIR || upBlock.block->getBlockID() == BlockID::WATER) {
            Flow = remaining_mass - getStableState(remaining_mass + upBlock.block->getWaterAmount());
            if (Flow > MinFlow) { Flow *= 0.5; }
            Flow = constrain(Flow, 0, std::min(MaxSpeed, remaining_mass));

            waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - Flow);
            upBlock.block->setWaterAmount(Flow);
            remaining_mass -= Flow;
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

*/