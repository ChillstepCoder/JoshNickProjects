// ResourceManager.cpp

#include "ResourceManager.h"
#include <iostream>

namespace JAGEngine {

  TextureCache ResourceManager::_textureCache;

  GLTexture ResourceManager::getTexture(std::string texturePath) {
    return _textureCache.getTexture(texturePath);
  }
}
