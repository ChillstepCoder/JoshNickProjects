#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include "Block.h"

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
    BlockManager(BlockMeshManager& meshManager) : m_MeshManager(meshManager) {}

    void addBlock(const Block& block) {
        m_blocks.push_back(block);
    }

    void rebuildMesh() {
        m_MeshManager.buildMesh(m_blocks);
    }

    void renderBlocks() {
        m_MeshManager.renderMesh();
    }

private:
    BlockMeshManager& m_MeshManager;
    std::vector<Block> m_blocks;
};