#pragma once
#include <string>
#include <vector>
class IOManager
{
public:
    static bool readFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer);
};

