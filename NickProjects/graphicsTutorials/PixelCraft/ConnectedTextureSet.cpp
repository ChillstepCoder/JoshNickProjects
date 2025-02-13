#include "ConnectedTextureSet.h"
#include "Block.h"
#include <random>
#include <iostream>

ConnectedTextureSet::ConnectedTextureSet()
{
    SubTextureRules.resize(TILE_ATLAS_DIMS_CELLS.x);
    for (int x = 0; x < TILE_ATLAS_DIMS_CELLS.x; x++) {
        SubTextureRules[x].resize(TILE_ATLAS_DIMS_CELLS.y);
    }

}

ConnectedTextureSet::~ConnectedTextureSet()
{
}

void ConnectedTextureSet::UpdateLookupTable()
{

    SubTextureLookup.clear();
    for (int x = 0; x < TILE_ATLAS_DIMS_CELLS.x; x++) {
        for (int y = 0; y < TILE_ATLAS_DIMS_CELLS.y; y++) {
            BlockAdjacencyRules& cellRules = SubTextureRules[x][y];
            SubTextureList& list = SubTextureLookup[cellRules];
            list.push_back(SubTexture::getSubUVRect(glm::ivec2(x, y), TILE_ATLAS_DIMS_CELLS));
        }
    }

}

glm::vec4 ConnectedTextureSet::GetSubTextureUVForRules(BlockAdjacencyRules rules)
{
    SubTextureList& list = SubTextureLookup[rules];
    if (list.empty()) {

        return glm::vec4(0, 0, 1, 1); // Returns fallback texture, missing rule ----TODO REPLACE WITH PROPER FALLBACK TEXTURE
    }
    std::mt19937 randomEngine((unsigned int)time(nullptr));
    std::uniform_int_distribution<int> randomTexture(0, list.size()-1);

    int index = randomTexture(randomEngine);

    return list[index];
}

void ConnectedTextureSet::SaveRules()
{
    UpdateLookupTable();

    std::ofstream outFile("textureset.amap");


    if (!outFile) {
        std::cerr << "Failed to save rules to file!" << std::endl;
        assert(false);
        return;
    }

    // Write rules to file (for loading purposes)
    for (int x = 0; x < TILE_ATLAS_DIMS_CELLS.x; x++) {
        for (int y = 0; y < TILE_ATLAS_DIMS_CELLS.y; y++) {
            BlockAdjacencyRules& rules = SubTextureRules[x][y];
            outFile.write(reinterpret_cast<const char*>(&rules), sizeof(BlockAdjacencyRules));
        }
    }

    outFile.close();

}

void ConnectedTextureSet::LoadRules()
{
    std::ifstream inFile("textureset.amap");

    if (!inFile) {
        std::cerr << "Failed to open rules from file!" << std::endl;
        return;
    }

    // Read rules from file
    for (int x = 0; x < TILE_ATLAS_DIMS_CELLS.x; x++) {
        for (int y = 0; y < TILE_ATLAS_DIMS_CELLS.y; y++) {
            BlockAdjacencyRules& rules = SubTextureRules[x][y];
            inFile.read(reinterpret_cast<char*>(&rules), sizeof(BlockAdjacencyRules));
        }
    }

    inFile.close();

}
