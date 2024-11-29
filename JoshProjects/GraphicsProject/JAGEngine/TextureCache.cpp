// TextureCache.cpp

#include "TextureCache.h"
#include "ImageLoader.h"
#include <iostream>

namespace JAGEngine {

  TextureCache::TextureCache() {

  }
  TextureCache::~TextureCache() {

  }

  GLTexture TextureCache::getTexture(std::string texturePath) {
    auto mit = _textureMap.find(texturePath);
    if (mit == _textureMap.end()) {
      GLTexture newTexture = ImageLoader::loadPNG(texturePath);

      // Set proper texture parameters for transparency
      glBindTexture(GL_TEXTURE_2D, newTexture.id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      _textureMap.insert(std::make_pair(texturePath, newTexture));
      return newTexture;
    }
    return mit->second;
  }
}
