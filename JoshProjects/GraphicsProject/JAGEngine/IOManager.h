// IOManager.h

#pragma once

#include <vector>
#include <string>

namespace JAGEngine {
  class IOManager
  {
  public:
    static bool readFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer);
  };
}

