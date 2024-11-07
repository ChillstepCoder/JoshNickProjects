#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"
#include <iostream>
#include "DebugDraw.h"

class BlockMeshManager
{
public:
    BlockMeshManager();
    ~BlockMeshManager();

    void init();
    void buildMesh(const std::vector<Block>& blocks); // Accept blocks from BlockManager
    void renderMesh();

private:
    Bengine::SpriteBatch m_spriteBatch;
};


class BlockManager {
public:
    BlockManager(BlockMeshManager& meshManager, b2WorldId worldId) : m_MeshManager(meshManager) {}

    void addBlock(const Block& block) {
        m_blocks.push_back(block);
    }

    void rebuildMesh() {
        m_MeshManager.buildMesh(m_blocks);
    }

    void renderBlocks() {
        m_MeshManager.renderMesh();
    }

    std::vector<Block>& getBlocks() {
        return m_blocks;
    }

    void breakBlockAtPosition(const glm::vec2& position) {  //destRect.x = (getPosition().x - ((0.5) * getDimensions().x));
        // Iterate through blocks and find the one that contains the given position
        auto it = std::find_if(m_blocks.begin(), m_blocks.end(), [&](const Block& block) {
            return isPositionInBlock(position, block);
            });

        if (it != m_blocks.end()) {
            // Remove the block from the world
            b2DestroyBody(it->getID());
            m_blocks.erase(it);

            // Rebuild the mesh
            rebuildMesh();
            DebugDraw::getInstance().setVertexDataChanged(true);
        }
    }

    bool isPositionInBlock(const glm::vec2& position, const Block& block) {
        // Check if the position is within the block's bounding box
        glm::vec2 blockPos = block.getDestRect();
        blockPos.x = blockPos.x + 1.25;
        blockPos.y = blockPos.y + 1.25;
        glm::vec2 blockSize = block.getDimensions();
        return (position.x >= blockPos.x - blockSize.x / 2 && position.x <= blockPos.x + blockSize.x / 2 &&
            position.y >= blockPos.y - blockSize.y / 2 && position.y <= blockPos.y + blockSize.y / 2);
    }

private:
    BlockMeshManager& m_MeshManager;
    std::vector<Block> m_blocks;
    b2WorldId m_world = b2_nullWorldId;
};