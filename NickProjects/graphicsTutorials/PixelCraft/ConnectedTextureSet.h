#pragma once
#include <array>
#include <map>
#include "Block.h"

enum class AdjacencyRule {
    AIR,
    BLOCK,
    DIRT,
    ANY
};

struct BlockAdjacencyRules {

    auto operator<=>(const BlockAdjacencyRules&)const = default; // SPACESHIP OPERATOR

    std::array<AdjacencyRule, 8> Rules = {};
};

using SubTextureList = std::vector<glm::vec4>; // this is a list of subUVs so we can make a subtexture

class ConnectedTextureSet
{
public:

    ConnectedTextureSet();
    ~ConnectedTextureSet();

    static ConnectedTextureSet& getInstance() {
        static ConnectedTextureSet instance;
        return instance;
    };

    void handleAnyRule(BlockAdjacencyRules& rules, int subUV_X, int subUV_Y);

    void UpdateLookupTable();

    glm::vec4 GetSubTextureUVForRules(BlockAdjacencyRules rules, int x, int y);

    void SaveRules();

    void LoadRules();

    std::map<BlockAdjacencyRules, SubTextureList> SubTextureLookup; // fast lookup of what texture to use based on the adjacency rules
    std::vector<std::vector<BlockAdjacencyRules>> SubTextureRules; // represents the rules for each subtexture in x,y pairing

private:

};

