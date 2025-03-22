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