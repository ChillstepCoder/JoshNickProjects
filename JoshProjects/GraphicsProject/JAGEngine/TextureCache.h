#pragma once
#include <string>
#include <map>
#include "GLTexture.h"

namespace JAGEngine {
  class TextureCache
  {
  public:
    TextureCache();
    ~TextureCache();

    GLTexture getTexture(std::string texturePath);
  private:
    std::map<std::string, GLTexture> _textureMap;
  };
}
