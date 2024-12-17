#include "CellularAutomataManager.h"
#include <iostream>

CellularAutomataManager::CellularAutomataManager() {

}
CellularAutomataManager::~CellularAutomataManager() {

}

void CellularAutomataManager::init() {

}

void CellularAutomataManager::simulateWater(Chunk& chunk, BlockManager& blockManager) {

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


        //int waterPushedThisStep = 0;
        //int waterStartLevel = waterBlock.block->getWaterAmount();


        if (downBlock.block->getBlockID() == BlockID::AIR) { // If downBlock is air, transfers all water to downBlock

            blockManager.placeBlock(downBlock, glm::vec2(downPosX, downPosY));
            downBlock.block->setWaterAmount(waterBlock.block->getWaterAmount());

            waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterBlock.block->getWaterAmount());
            blockManager.destroyBlock(waterBlock);

            //waterPushedThisStep = waterBlock.block->getWaterAmount();
            //int waterEndLevel = waterBlock.block->getWaterAmount();

            //std::cout << "waterPushedThisStep: " << waterPushedThisStep << "  waterEndLevel: " << waterEndLevel << "  waterStartLevel: " << waterStartLevel << std::endl;
            //assert(waterPushedThisStep + (waterEndLevel - waterStartLevel) == 0);

        } else if (downBlock.block->getBlockID() == BlockID::WATER) { // If downBlock is water, check if it is full
            if (downBlock.block->getWaterAmount() < WATER_LEVELS) { // If downBlock isnt full

                int waterDifference = WATER_LEVELS - downBlock.block->getWaterAmount();// How much water is missing from downBlock

                if (waterDifference < waterBlock.block->getWaterAmount()) { // The missing water is less than the amount in waterBlock
                    downBlock.block->setWaterAmount(downBlock.block->getWaterAmount() + waterDifference);
                    waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterDifference);
                    chunk.m_isMeshDirty = true;
                } else { // The waterBlock doesnt have enough to fill up downBlock
                    downBlock.block->setWaterAmount(downBlock.block->getWaterAmount() + waterBlock.block->getWaterAmount());

                    waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterBlock.block->getWaterAmount());
                    blockManager.destroyBlock(waterBlock); // Destroy old waterblock because it is empty

                }
            }
        }
        
        if (waterBlock.block->getWaterAmount() == 0) { // Check if there is water still left in the waterBlock 
            continue;
        }

        if (rightBlock.block->getBlockID() == BlockID::AIR) {
           
            if (downRightBlock.block->getBlockID() == BlockID::AIR) { // Check the downRightBlock first to see if water is placeable

                blockManager.placeBlock(downRightBlock, glm::vec2(downRightPosX, downRightPosY));
                downRightBlock.block->setWaterAmount(waterBlock.block->getWaterAmount());

                waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterBlock.block->getWaterAmount());
                blockManager.destroyBlock(waterBlock);


            } else if (downRightBlock.block->getBlockID() == BlockID::WATER) {
                if (downRightBlock.block->getWaterAmount() < WATER_LEVELS) { // If downRightBlock isnt full

                    int waterDifference = WATER_LEVELS - downRightBlock.block->getWaterAmount();// How much water is missing from downRightBlock

                    if (waterDifference < waterBlock.block->getWaterAmount()) { // The missing water is less than the amount in waterBlock
                        int leftOverWater = waterBlock.block->getWaterAmount() - waterDifference;

                        downRightBlock.block->setWaterAmount(downRightBlock.block->getWaterAmount() + waterDifference);

                        int avgLeftOverWater = (leftOverWater / 2);

                        waterBlock.block->setWaterAmount(avgLeftOverWater);

                        blockManager.placeBlock(rightBlock, glm::vec2(rightPosX, rightPosY));
                        rightBlock.block->setWaterAmount(avgLeftOverWater);
                        chunk.m_isMeshDirty = true;

                    }
                    else { // The waterBlock doesnt have enough to fill up downRightBlock
                        downRightBlock.block->setWaterAmount(downRightBlock.block->getWaterAmount() + waterBlock.block->getWaterAmount());
                        waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterBlock.block->getWaterAmount());
                        blockManager.destroyBlock(waterBlock); // Destroy old waterblock because it is empty

                    }
                } else { // downRightBlock is full

                    int avgAmt = (waterBlock.block->getWaterAmount()) / 2;

                    waterBlock.block->setWaterAmount(avgAmt);

                    blockManager.placeBlock(rightBlock, glm::vec2(rightPosX, rightPosY));
                    rightBlock.block->setWaterAmount(avgAmt);
                }
            } else { // downRightBlock is a solid block

                int avgAmt = (waterBlock.block->getWaterAmount()) / 2;

                waterBlock.block->setWaterAmount(avgAmt);

                blockManager.placeBlock(rightBlock, glm::vec2(rightPosX, rightPosY));
                rightBlock.block->setWaterAmount(avgAmt);
            }

        } else if (rightBlock.block->getBlockID() == BlockID::WATER) {
            if (rightBlock.block->getWaterAmount() + 5 < waterBlock.block->getWaterAmount()) { // If right water is less than waterBlock
                int avgAmt = (waterBlock.block->getWaterAmount() + rightBlock.block->getWaterAmount()) / 2;

                waterBlock.block->setWaterAmount(avgAmt); // set to avg water

                rightBlock.block->setWaterAmount(avgAmt); // set to avg water
                chunk.m_isMeshDirty = true;

            } else if (rightBlock.block->getWaterAmount() + 2 <= waterBlock.block->getWaterAmount()) { // If water is close to even

                waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - 1);
                rightBlock.block->setWaterAmount(rightBlock.block->getWaterAmount() + 1);
                chunk.m_isMeshDirty = true;
            }
        }

        if (waterBlock.block->getWaterAmount() == 0) { // Check if there is water still left in the waterBlock 
            continue;
        }

        if (leftBlock.block->getBlockID() == BlockID::AIR) {

            if (downLeftBlock.block->getBlockID() == BlockID::AIR) { // Check the downLeftBlock first to see if water is placeable

                blockManager.placeBlock(downLeftBlock, glm::vec2(downLeftPosX, downLeftPosY));
                downLeftBlock.block->setWaterAmount(waterBlock.block->getWaterAmount());

                waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterBlock.block->getWaterAmount());
                blockManager.destroyBlock(waterBlock);

            }
            else if (downLeftBlock.block->getBlockID() == BlockID::WATER) {
                if (downLeftBlock.block->getWaterAmount() < WATER_LEVELS) { // If downLeftBlock isnt full

                    int waterDifference = WATER_LEVELS - downLeftBlock.block->getWaterAmount();// How much water is missing from downLeftBlock

                    if (waterDifference < waterBlock.block->getWaterAmount()) { // The missing water is less than the amount in waterBlock
                        int leftOverWater = waterBlock.block->getWaterAmount() - waterDifference;

                        downLeftBlock.block->setWaterAmount(downLeftBlock.block->getWaterAmount() + waterDifference);

                        int avgLeftOverWater = (leftOverWater / 2);

                        waterBlock.block->setWaterAmount(avgLeftOverWater);

                        blockManager.placeBlock(leftBlock, glm::vec2(leftPosX, leftPosY));
                        leftBlock.block->setWaterAmount(avgLeftOverWater);
                        chunk.m_isMeshDirty = true;

                    }
                    else { // The waterBlock doesnt have enough to fill up downLeftBlock
                        downLeftBlock.block->setWaterAmount(downLeftBlock.block->getWaterAmount() + waterBlock.block->getWaterAmount());
                        waterBlock.block->setWaterAmount(0);
                        blockManager.destroyBlock(waterBlock); // Destroy old waterblock because it is empty

                    }
                }
                else { // downLeftBlock is full

                    int avgAmt = (waterBlock.block->getWaterAmount()) / 2;

                    waterBlock.block->setWaterAmount(avgAmt);

                    blockManager.placeBlock(leftBlock, glm::vec2(leftPosX, leftPosY));
                    leftBlock.block->setWaterAmount(avgAmt);
                }
            } else { // downLeftBlock is a solid block
                int avgAmt = (waterBlock.block->getWaterAmount()) / 2;

                waterBlock.block->setWaterAmount(avgAmt);

                blockManager.placeBlock(leftBlock, glm::vec2(leftPosX, leftPosY));
                leftBlock.block->setWaterAmount(avgAmt);
            }

        }
        else if (leftBlock.block->getBlockID() == BlockID::WATER) {
            if (leftBlock.block->getWaterAmount() + 5 < waterBlock.block->getWaterAmount()) { // If right water is less than waterBlock
                int avgAmt = (waterBlock.block->getWaterAmount() + leftBlock.block->getWaterAmount()) / 2;

                waterBlock.block->setWaterAmount(avgAmt); // set to avg water

                leftBlock.block->setWaterAmount(avgAmt); // set to avg water
                chunk.m_isMeshDirty = true;

            }
            else if (leftBlock.block->getWaterAmount() + 2 <= waterBlock.block->getWaterAmount()) { // If water is close to even

                waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - 1);
                leftBlock.block->setWaterAmount(leftBlock.block->getWaterAmount() + 1);
                chunk.m_isMeshDirty = true;
            }
        }


        if (waterBlock.block->getWaterAmount() == 0) { // Check if there is water still left in the waterBlock 
            continue;
        }
        /*
        if (waterBlockMass < WATER_LEVELS) { // If the water block isnt full, check above
            int neededWater = WATER_LEVELS - waterBlockMass;

            if (upBlock.block->getBlockID() == BlockID::WATER) {
                
                if (upBlock.block->getWaterAmount() > neededWater) { // The above block has enough water to fill in th 
                    waterBlock.block->setWaterAmount(WATER_LEVELS);
                    upBlock.block->setWaterAmount(upBlock.block->getWaterAmount() - neededWater);
                } else { // the above block doesnt have enough water, destroy it and move the water down
                    waterBlock.block->setWaterAmount(waterBlockMass + upBlock.block->getWaterAmount());
                    upBlock.block->setWaterAmount(upBlock.block->getWaterAmount() - neededWater);
                    blockManager.destroyBlock(upBlock);
                }
            }
        }a
        */
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