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
        auto destRect = block.getDestRect();
        auto uvRect = block.getUVRect();
        auto textureID = block.getTextureID();
        auto color = block.getColor();

        m_spriteBatch.draw(destRect, uvRect, textureID, 0.0f, color, 0.0f);
    }
    m_spriteBatch.end();
}
void BlockMeshManager::renderMesh() {
    m_spriteBatch.renderBatch();
}