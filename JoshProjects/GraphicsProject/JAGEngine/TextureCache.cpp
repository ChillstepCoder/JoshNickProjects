#include "TextureCache.h"
#include "ImageLoader.h"
#include <iostream>

namespace JAGEngine {

  TextureCache::TextureCache() {

  }
  TextureCache::~TextureCache() {

  }

  GLTexture TextureCache::getTexture(std::string texturePath) {
    //check if in map
    auto mit = _textureMap.find(texturePath);

    if (mit == _textureMap.end()) {
      GLTexture newTexture = ImageLoader::loadPNG(texturePath);
      _textureMap.insert(make_pair(texturePath, newTexture));

      std::cout << "Loaded Texture!\n";
      return newTexture;
    }
    std::cout << "Used Cached Texture!\n";


    return mit->second;
  }
}
