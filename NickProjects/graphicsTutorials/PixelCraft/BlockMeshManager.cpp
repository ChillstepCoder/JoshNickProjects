#include "BlockMeshManager.h"


BlockMeshManager::BlockMeshManager() {

}

BlockMeshManager::~BlockMeshManager() {

}

void BlockMeshManager::init() {
    m_spriteBatch.init();
}
void BlockMeshManager::buildMesh(const std::vector<Block>& blocks) {
    m_spriteBatch.begin();
    for (const auto& block : blocks) {
        m_spriteBatch.draw(
            block.getDestRect(), 
            block.getUVRect(),
            block.getTextureID(), 
            0.0f,        // Depth
            block.getColor(), 
            0.0f);       // Rotation
    }
    m_spriteBatch.end();
}
void BlockMeshManager::renderMesh() {
    m_spriteBatch.renderBatch();
}