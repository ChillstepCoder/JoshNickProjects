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

void ConnectedTextureSet::handleAnyRule(BlockAdjacencyRules& rules, int subUV_X, int subUV_Y) {
    // Vector to store positions of 'ANY' in the rules
    std::vector<int> anyPositions;

    // Identify positions with the 'ANY' rule and store their indices
    for (int i = 0; i < 8; i++) {
        if (rules.Rules[i] == AdjacencyRule::ANY) {
            anyPositions.push_back(i);
        }
    }

    // If no 'ANY' rule is present, we don't need to generate permutations
    if (anyPositions.empty()) {
        return;
    }

    // Generate all permutations of AIR (0) and BLOCK (1) for the identified 'ANY' positions
    int numPermutations = 1 << anyPositions.size();  // 2^n permutations
    for (int i = 0; i < numPermutations; i++) {
        std::vector<bool> isBlock(anyPositions.size());

        // Set each position of 'ANY' to AIR (0) or BLOCK (1) based on the current permutation
        for (int j = 0; j < anyPositions.size(); j++) {
            isBlock[j] = (i & (1 << j)) != 0;  // Extract the j-th bit for the current permutation
        }

        // Create a new rules set with the 'ANY' positions replaced
        BlockAdjacencyRules permutedRules = rules;
        for (int j = 0; j < anyPositions.size(); j++) {
            permutedRules.Rules[anyPositions[j]] = isBlock[j] ? AdjacencyRule::BLOCK : AdjacencyRule::AIR;
        }

        // Insert the permuted rules into the SubTextureLookup table
        SubTextureList& list = SubTextureLookup[permutedRules];
        list.push_back(SubTexture::getSubUVRect(glm::ivec2(subUV_X, subUV_Y), TILE_ATLAS_DIMS_CELLS));
    }
}

void ConnectedTextureSet::UpdateLookupTable()
{

    SubTextureLookup.clear();
    for (int x = 0; x < TILE_ATLAS_DIMS_CELLS.x; x++) {
        for (int y = 0; y < TILE_ATLAS_DIMS_CELLS.y; y++) {
            BlockAdjacencyRules& cellRules = SubTextureRules[x][y];

            handleAnyRule(cellRules, x, y);

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
    UpdateLookupTable();

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
