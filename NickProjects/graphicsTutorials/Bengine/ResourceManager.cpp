#include "ResourceManager.h"

namespace Bengine {

    TextureCache ResourceManager::m_textureCache;

    GLTexture ResourceManager::getTexture(std::string texturePath, Bengine::TextureFilterMode filterMode) {
        return m_textureCache.getTexture(texturePath, filterMode);
    }

}