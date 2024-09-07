#pragma once
#include "GLTexture.h"
#include <string>

namespace JAGEngine {
  class ImageLoader
  {
  public:
    static GLTexture loadPNG(std::string filePath);
  };
}

