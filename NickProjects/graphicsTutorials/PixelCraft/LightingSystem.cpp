#include "LightingSystem.h"
#include <algorithm>
#include "BlockMeshManager.h"

LightingSystem::LightingSystem() : width(0), height(0), blockGrid(nullptr), m_blockManager(nullptr) {
}

LightingSystem::~LightingSystem() {
}

void LightingSystem::init(int worldWidth, int worldHeight) {
    width = worldWidth;
    height = worldHeight;

    // Initialize lighting map with zero light level
    lightMap.resize(width, std::vector<unsigned char>(height, 0));
}

void LightingSystem::setBlockManager(BlockManager* blockManager) {
    m_blockManager = blockManager;
}

void LightingSystem::updateLighting() {
    // Clear the lighting map
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            lightMap[x][y] = 0;
        }
    }

    // Clear the propagation queue
    lightPropagationQueue.clear();

    // Calculate lighting
    calculateLighting();
}

void LightingSystem::updateLightingOnBlockBreak(int x, int y) {
    // Skip if position is invalid
    if (!isValidPosition(x, y)) {
        return;
    }

    // Find the highest light level among neighboring blocks
    unsigned char highestNeighborLight = 0;

    // Check all adjacent blocks (including diagonals)
    const int dx[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    const int dy[] = { -1, -1, -1, 0, 0, 1, 1, 1 };

    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (isValidPosition(nx, ny)) {
            highestNeighborLight = std::max(highestNeighborLight, lightMap[nx][ny]);
        }
    }

    // If there's light nearby, propagate it to this block
    if (highestNeighborLight > 0) {
        // New block is air, so set it to the highest neighbor light level minus 1
        // Unless it would be 0, in which case we leave it at 0
        unsigned char newLightLevel = (highestNeighborLight > 1) ? (highestNeighborLight - 1) : 0;

        // If we're making this an air block, set it to max light
        if (getBlockIDAt(x, y) == BlockID::AIR) {
            newLightLevel = 10;
        }

        // Set the light level for the block
        lightMap[x][y] = newLightLevel;

        // Only propagate if we have light
        if (newLightLevel > 0) {
            // Clear propagation queue
            lightPropagationQueue.clear();

            // Add to propagation queue for further propagation
            enqueueLightNode(x, y, newLightLevel);

            // Propagate light from this block
            propagateLight();
        }
    }
    else if (getBlockIDAt(x, y) == BlockID::AIR) {
        // If breaking a block makes this position air, set it to max light
        lightMap[x][y] = 10;

        // Clear propagation queue
        lightPropagationQueue.clear();

        // Add to propagation queue for further propagation
        enqueueLightNode(x, y, 10);

        // Propagate light from this block
        propagateLight();
    }
}

void LightingSystem::updateLightingOnBlockAdd(int x, int y, BlockID previousBlockID) {
    // Skip if position is invalid
    if (!isValidPosition(x, y)) {
        return;
    }

    // Get the light level that was at this position before we added the block
    unsigned char prevLightLevel = lightMap[x][y];

    // Block was added, so it will block light. Set its light level to 0.
    // (unless it's a light source block, which you could handle here)
    lightMap[x][y] = 0;

    // If the previous block had light, we need to update neighboring blocks
    if (prevLightLevel > 0) {
        // Find blocks that need updating (lights that were influenced by this block)
        std::vector<LightNode> blocksToUpdate;

        // Check all adjacent blocks (including diagonals)
        const int dx[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
        const int dy[] = { -1, -1, -1, 0, 0, 1, 1, 1 };

        for (int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (isValidPosition(nx, ny)) {
                // If the neighboring block's light level is greater than 0 and less than maximum
                if (lightMap[nx][ny] > 0 && lightMap[nx][ny] < 10) {
                    // Add to list of blocks that need updating
                    LightNode node;
                    node.x = nx;
                    node.y = ny;
                    node.lightLevel = lightMap[nx][ny];
                    blocksToUpdate.push_back(node);

                    // Set the light to 0 temporarily so it can be recalculated
                    lightMap[nx][ny] = 0;
                }
            }
        }

        // If we have blocks to update, redo the light propagation for those blocks
        if (!blocksToUpdate.empty()) {
            // Recalculate light from all max light sources (air blocks)
            for (int nx = std::max(0, x - 10); nx < std::min(width, x + 10); nx++) {
                for (int ny = std::max(0, y - 10); ny < std::min(height, y + 10); ny++) {
                    if (getBlockIDAt(nx, ny) == BlockID::AIR) {
                        lightMap[nx][ny] = 10;  // Max light level for air
                        enqueueLightNode(nx, ny, 10);
                    }
                }
            }

            // Propagate light from these sources
            propagateLight();
        }
    }
}


glm::vec3 LightingSystem::getLightValue(int x, int y) const {
    if (!isValidPosition(x, y)) {
        return glm::vec3(0.0f);
    }

    // Convert light level to a value between 0 and 1
    float lightValue = lightMap[x][y] / 10.0f;
    return glm::vec3(lightValue, lightValue, lightValue);
}

Bengine::ColorRGBA8 LightingSystem::applyLighting(const Bengine::ColorRGBA8& blockColor, int x, int y) const {
    if (!isValidPosition(x, y)) {
        return blockColor;
    }

    // Apply lighting to the block color
    // Light level ranges from 0 to 10, where 10 is full brightness
    float lightFactor = lightMap[x][y] / 10.0f;

    return Bengine::ColorRGBA8(
        static_cast<GLubyte>(blockColor.r * lightFactor),
        static_cast<GLubyte>(blockColor.g * lightFactor),
        static_cast<GLubyte>(blockColor.b * lightFactor),
        blockColor.a
    );
}

void LightingSystem::calculateLighting() {
    // FIRST PASS: Set all air blocks to maximum light level (10)
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (isValidPosition(x, y) && getBlockIDAt(x, y) == BlockID::AIR) {
                lightMap[x][y] = 10;
            }
        }
    }

    // SECOND PASS: Find blocks adjacent to air blocks and add them to the propagation queue
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (isValidPosition(x, y) && getBlockIDAt(x, y) != BlockID::AIR) {
                bool adjacentToAir = false;

                // Check all adjacent blocks (including diagonals)
                const int dx[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
                const int dy[] = { -1, -1, -1, 0, 0, 1, 1, 1 };

                for (int i = 0; i < 8; i++) {
                    int nx = x + dx[i];
                    int ny = y + dy[i];

                    if (isValidPosition(nx, ny) && getBlockIDAt(nx, ny) == BlockID::AIR) {
                        adjacentToAir = true;
                        break;
                    }
                }

                if (adjacentToAir) {
                    // Set light level to 9 (one less than air blocks)
                    lightMap[x][y] = 9;

                    // Add to propagation queue for further propagation
                    enqueueLightNode(x, y, 9);
                }
            }
        }
    }

    // THIRD PASS: Propagate light from the queue
    propagateLight();
}

void LightingSystem::propagateLight() {
    // Process all light nodes in the queue
    while (!lightPropagationQueue.empty()) {
        LightNode node = lightPropagationQueue.front();
        lightPropagationQueue.erase(lightPropagationQueue.begin());

        int x = node.x;
        int y = node.y;
        unsigned char currentLightLevel = node.lightLevel;

        // Skip if light level is already 0
        if (currentLightLevel == 0) {
            continue;
        }

        // Calculate new light level (one less than current)
        unsigned char newLightLevel = currentLightLevel - 1;

        // Check all adjacent blocks (including diagonals)
        const int dx[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
        const int dy[] = { -1, -1, -1, 0, 0, 1, 1, 1 };

        for (int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (isValidPosition(nx, ny)) {
                // Skip air blocks (they were set to max light in first pass)
                if (getBlockIDAt(nx, ny) == BlockID::AIR) {
                    continue;
                }

                // If the block is a lower light level
                if (lightMap[nx][ny] < newLightLevel) {
                    // Update the light level
                    lightMap[nx][ny] = newLightLevel;

                    // Add to queue for further propagation
                    enqueueLightNode(nx, ny, newLightLevel);
                }
            }
        }
    }
}

void LightingSystem::enqueueLightNode(int x, int y, unsigned char lightLevel) {
    LightNode node;
    node.x = x;
    node.y = y;
    node.lightLevel = lightLevel;

    lightPropagationQueue.push_back(node);
}

bool LightingSystem::isValidPosition(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height);
}

BlockID LightingSystem::getBlockIDAt(int x, int y) const {
    if (m_blockManager) {
        BlockHandle blockHandle = m_blockManager->getBlockAtPosition(glm::vec2(x, y));
        if (blockHandle.block) {
            return blockHandle.block->getBlockID();
        }
    }
    return BlockID::AIR; // Default to air if not found
}

bool LightingSystem::isBlockSolid(int x, int y) const {
    if (m_blockManager) {
        BlockHandle blockHandle = m_blockManager->getBlockAtPosition(glm::vec2(x, y));
        if (blockHandle.block) {
            BlockID blockID = blockHandle.block->getBlockID();
            return blockID != BlockID::AIR && blockID != BlockID::WATER;
        }
    }
    return false;
}

bool LightingSystem::isBlockTransparent(int x, int y) const {
    if (m_blockManager) {
        BlockHandle blockHandle = m_blockManager->getBlockAtPosition(glm::vec2(x, y));
        if (blockHandle.block) {
            BlockID blockID = blockHandle.block->getBlockID();

            // Define which blocks are transparent (allow light to pass through)
            // This is a simplified example - you might want a more sophisticated approach
            return blockID == BlockID::AIR ||
                blockID == BlockID::WATER;
        }
    }
    return true; // Default to transparent if not found
}