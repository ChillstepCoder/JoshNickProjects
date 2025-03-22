#include "CellularAutomataManager.h"
#include <iostream>

CellularAutomataManager::CellularAutomataManager() {

}
CellularAutomataManager::~CellularAutomataManager() {

}

void CellularAutomataManager::init() {

}

void CellularAutomataManager::simulateWater(Chunk& chunk, BlockManager& blockManager, const LightingSystem& lightingSystem) {

    //Calculate and apply flow for each block
    for (int i = chunk.waterBlocks.size() - 1; i >= 0; --i) {

        const int waterPosX = chunk.waterBlocks[i].x + 0.5f;
        const int waterPosY = chunk.waterBlocks[i].y + 0.5f;

        const int downPosX = waterPosX;
        const int downPosY = (waterPosY - 1);

        const int downRightPosX = waterPosX + 1;
        const int downRightPosY = (waterPosY - 1);

        const int downLeftPosX = waterPosX - 1;
        const int downLeftPosY = (waterPosY - 1);

        const int leftPosX = (waterPosX - 1);
        const int leftPosY = waterPosY;

        const int rightPosX = (waterPosX + 1);
        const int rightPosY = waterPosY;

        const int upPosX = waterPosX;
        const int upPosY = (waterPosY + 1);

        BlockHandle waterBlock = blockManager.getBlockAtPosition(glm::vec2(waterPosX, waterPosY));

        BlockHandle downBlock = getBlockAtPositionSafely(blockManager, glm::vec2(downPosX, downPosY));

        if (downBlock.block == nullptr) {
            continue;
        }

        BlockHandle downRightBlock = getBlockAtPositionSafely(blockManager, glm::vec2(downRightPosX, downRightPosY));

        if (downRightBlock.block == nullptr) {
            continue;
        }

        BlockHandle downLeftBlock = getBlockAtPositionSafely(blockManager, glm::vec2(downLeftPosX, downLeftPosY));

        if (downLeftBlock.block == nullptr) {
            continue;
        }

        BlockHandle leftBlock = getBlockAtPositionSafely(blockManager, glm::vec2(leftPosX, leftPosY));

        if (leftBlock.block == nullptr) {
            continue;
        }
        
        BlockHandle rightBlock = getBlockAtPositionSafely(blockManager, glm::vec2(rightPosX, rightPosY));

        if (rightBlock.block == nullptr) {
            continue;
        }


        if (downBlock.block->getBlockID() == BlockID::AIR) { // If downBlock is air, transfers all water to downBlock

            if (moveWaterToBlock(waterBlock, downBlock, glm::vec2(downPosX, downPosY), waterBlock.block->getWaterAmount(), blockManager)) {
                continue;
            }

        } else if (downBlock.block->getBlockID() == BlockID::WATER) { // If downBlock is water, check if it is full
            if (downBlock.block->getWaterAmount() < WATER_LEVELS) { // If downBlock isnt full

                int waterDifference = WATER_LEVELS - downBlock.block->getWaterAmount();// How much water is missing from downBlock

                if (waterDifference < waterBlock.block->getWaterAmount()) { // The missing water is less than the amount in waterBlock
                    downBlock.block->setWaterAmount(downBlock.block->getWaterAmount() + waterDifference);
                    waterBlock.block->setWaterAmount(waterBlock.block->getWaterAmount() - waterDifference);
                    chunk.m_isMeshDirty = true;
                } else { // The waterBlock doesnt have enough to fill up downBlock
                    if (moveWaterToBlock(waterBlock, downBlock, glm::vec2(downPosX, downPosY), downBlock.block->getWaterAmount() + waterBlock.block->getWaterAmount(), blockManager)) {
                        continue;
                    }
                }
            }
        }

        if (moveWaterDiagonally(waterBlock, downRightBlock, glm::vec2(downRightPosX, downRightPosY), rightBlock, glm::vec2(rightPosX, rightPosY), blockManager)) {
            chunk.m_isMeshDirty = true;
            continue;
        }

        if (waterBlock.block->getWaterAmount() == 0) { // Check if there is water still left in the waterBlock
            blockManager.destroyBlock(waterBlock);
            continue;
        }
        
        if (moveWaterDiagonally(waterBlock, downLeftBlock, glm::vec2(downLeftPosX, downLeftPosY), leftBlock, glm::vec2(leftPosX, leftPosY), blockManager)) {
            chunk.m_isMeshDirty = true;
            continue;
        }

        if (waterBlock.block->getWaterAmount() == 0) { // Check if there is water still left in the waterBlock
            blockManager.destroyBlock(waterBlock);
            continue;
        }
    }

    if (chunk.m_isMeshDirty == true) {
        chunk.m_isMeshDirty = false;
        chunk.buildChunkMesh(blockManager, lightingSystem);
    }

}

bool CellularAutomataManager::moveWaterToBlock(BlockHandle& sourceBlock, BlockHandle& targetBlock, glm::vec2 targetPos, int amountToPush, BlockManager& blockManager) {
    // This function will move amountToPush water from sourceBlock to targetBlock, and will destroy sourceBlock if it is empty at the end, and return true if sourceBlock is empty

    if (targetBlock.block->getBlockID() == BlockID::AIR) {
        blockManager.placeBlock(targetBlock, glm::vec2(targetPos.x, targetPos.y));
    }
    targetBlock.block->setWaterAmount(amountToPush);

    if (amountToPush >= sourceBlock.block->getWaterAmount()) {
        blockManager.destroyBlock(sourceBlock);
        return true;
    } else {
        sourceBlock.block->setWaterAmount(sourceBlock.block->getWaterAmount() - amountToPush);
        return false;
    }

}

void CellularAutomataManager::splitWaterToEmpty(BlockHandle& sourceBlock, BlockHandle& targetBlock, glm::vec2 targetPos, BlockManager& blockManager) {

    int avgAmt = (sourceBlock.block->getWaterAmount()) / 2;

    sourceBlock.block->setWaterAmount(avgAmt);

    blockManager.placeBlock(targetBlock, glm::vec2(targetPos.x, targetPos.y));
    targetBlock.block->setWaterAmount(avgAmt);
}


bool CellularAutomataManager::moveWaterDiagonally(BlockHandle& sourceBlock, BlockHandle& diagonalBlock, glm::vec2 diagonalPos, BlockHandle& adjacentBlock, glm::vec2 adjacentPos, BlockManager& blockManager) {
    bool isMeshDirty = false;

    // Should implement the common diagonal logic, and USE moveWaterToBlock to do the actual moving of water

    if (adjacentBlock.block == nullptr) {
        return isMeshDirty;
    }


    if (adjacentBlock.block->getBlockID() == BlockID::AIR) {

        if (diagonalBlock.block->getBlockID() == BlockID::AIR) { // Check the diagonalBlock first to see if water is placeable

            if (moveWaterToBlock(sourceBlock, diagonalBlock, glm::vec2(diagonalPos.x, diagonalPos.y), sourceBlock.block->getWaterAmount(), blockManager)) {
                isMeshDirty = true;
                return isMeshDirty;
            }

        }
        else if (diagonalBlock.block->getBlockID() == BlockID::WATER) {
            if (diagonalBlock.block->getWaterAmount() < WATER_LEVELS) { // If diagonalBlock isnt full

                int waterDifference = WATER_LEVELS - diagonalBlock.block->getWaterAmount();// How much water is missing from diagonalBlock

                if (waterDifference < sourceBlock.block->getWaterAmount()) { // The missing water is less than the amount in sourceBlock
                    int leftOverWater = sourceBlock.block->getWaterAmount() - waterDifference;

                    diagonalBlock.block->setWaterAmount(diagonalBlock.block->getWaterAmount() + waterDifference);

                    int avgLeftOverWater = (leftOverWater / 2);

                    sourceBlock.block->setWaterAmount(avgLeftOverWater);

                    blockManager.placeBlock(adjacentBlock, glm::vec2(adjacentPos.x, adjacentPos.y));
                    adjacentBlock.block->setWaterAmount(avgLeftOverWater);
                    isMeshDirty = true;
                    return isMeshDirty;
                }
                else { // The waterBlock doesnt have enough to fill up downRightBlock
                    moveWaterToBlock(sourceBlock, diagonalBlock, glm::vec2(diagonalPos.x, diagonalPos.y), diagonalBlock.block->getWaterAmount() + sourceBlock.block->getWaterAmount(), blockManager);
                    isMeshDirty = true;
                    return isMeshDirty;
                }
            }
            else { // downRightBlock is full
                splitWaterToEmpty(sourceBlock, adjacentBlock, glm::vec2(adjacentPos.x, adjacentPos.y), blockManager);
            }
        }
        else { // downRightBlock is a solid block
            splitWaterToEmpty(sourceBlock, adjacentBlock, glm::vec2(adjacentPos.x, adjacentPos.y), blockManager);
        }
    }
    else if (adjacentBlock.block->getBlockID() == BlockID::WATER) {
        if (adjacentBlock.block->getWaterAmount() + 5 < sourceBlock.block->getWaterAmount()) { // If right water is less than waterBlock
            int avgAmt = (sourceBlock.block->getWaterAmount() + adjacentBlock.block->getWaterAmount()) / 2;

            sourceBlock.block->setWaterAmount(avgAmt); // set to avg water

            adjacentBlock.block->setWaterAmount(avgAmt); // set to avg water
            isMeshDirty = true;
        }
        else if (adjacentBlock.block->getWaterAmount() + 2 <= sourceBlock.block->getWaterAmount()) { // If water is close to even

            sourceBlock.block->setWaterAmount(sourceBlock.block->getWaterAmount() - 1);
            adjacentBlock.block->setWaterAmount(adjacentBlock.block->getWaterAmount() + 1);
            isMeshDirty = true;
        }
    }
    return isMeshDirty;
}

BlockHandle CellularAutomataManager::getBlockAtPositionSafely(BlockManager& blockManager, glm::vec2 position) {
    Chunk* neighboringChunk = blockManager.getChunkAtPosition(position);

    if (neighboringChunk->isLoaded() == false) {
        // Chunk is not loaded, return a default block handle (e.g., air or null block)
        return BlockHandle();
    }

    return blockManager.getBlockAtPosition(position);
}
