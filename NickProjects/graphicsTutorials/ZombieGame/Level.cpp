#include "Level.h"

#include <Bengine/Errors.h>
#include <fstream>
#include <iostream>
#include <Bengine/ResourceManager.h>
#include <map>
#include <utility> // For std::pair

std::map<char, std::pair<int, int>> tileSpriteMapping = {
    {'Q', {0, 9}}, // First Sprite Wall at (0, 9)
    {'W', {1, 9}}, // Second Sprite Wall at (1, 9)
    {'E', {2, 9}}, // Third Sprite Wall at (2, 9)
    {'R', {3, 9}}, // Fourth Sprite Wall at (3, 9)
    {'T', {4, 9}}, // Fifth Sprite Wall at (4, 9)
    {'Y', {5, 9}}, // Sixth Sprite Wall at (5, 9)
    {'U', {6, 9}}, // Seventh Sprite Wall at (6, 9)
    {'I', {7, 9}}, // Eighth Sprite Wall at (7, 9)
    // Add more mappings as necessary
};

Level::Level(const std::string& fileName) {

    std::ifstream file;
    file.open(fileName);

    // Error checking
    if (file.fail()) {
        Bengine::fatalError("Failed to open " + fileName);
    }

    // Throw away the first string in tmp
    std::string tmp;

    file >> tmp >> _numHumans;

    std::getline(file, tmp); // Throw away the rest of the first line

    // Read the level data
    while (std::getline(file, tmp)) {
        _levelData.push_back(tmp);
    }


    _spriteBatch.init();
    _spriteBatch.begin();

    Bengine::Color whiteColor;
    whiteColor.r = 255;
    whiteColor.g = 255;
    whiteColor.b = 255;
    whiteColor.a = 255;

    // Calculate UV dimensions
    float spriteWidth = 1.0f / 10; // 10 sprites per row
    float spriteHeight = 1.0f / 10; // 10 sprites per column

    // Render all the tiles
    for (int y = 0; y < _levelData.size(); y++) {
        for (int x = 0; x < _levelData[y].size(); x++) {
            // Grab the tile
            char tile = _levelData[y][x];

            // Get dest rect
            glm::vec4 destRect(x * TILE_WIDTH, y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH);

            //Initialize UV rect
            glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

            // Check if tile is in mapping
            if (tileSpriteMapping.find(tile) != tileSpriteMapping.end()) {
                std::pair<int, int> spriteIndices = tileSpriteMapping[tile];
                int spriteIndexX = spriteIndices.first;
                int spriteIndexY = spriteIndices.second;

                // Calculate UV offset
                float uOffset = spriteIndexX * spriteWidth;
                float vOffset = spriteIndexY * spriteHeight;

                uvRect = glm::vec4(uOffset, vOffset, spriteWidth, spriteHeight);
            }


            // Process the tile
            switch (tile) {
            case 'Q': // First Sprite Wall
            case 'W': // Second Sprite Wall
            case 'E': // Third Sprite Wall
            case 'R': // Fourth Sprite Wall
            case 'T': // Fifth Sprite Wall
            case 'Y': // Sixth Sprite Wall
            case 'U': // Seventh Sprite Wall
            case 'I': // Eighth Sprite Wall
                _spriteBatch.draw(destRect,
                    uvRect,
                    Bengine::ResourceManager::getTexture("Textures/Tiles/Tiny Top Down 32x32.png").id,
                    0.0f,
                    whiteColor, 0.0f);
                break;
            case '@': // Player
                _levelData[y][x] = '.'; // So we dont collide with a @
                _startPlayerPos.x = x * TILE_WIDTH;
                _startPlayerPos.y = y * TILE_WIDTH;
                break;
            case 'Z': // Zombie
                _levelData[y][x] = '.'; // So we dont collide with a Z
                _zombieStartPositions.emplace_back(x * TILE_WIDTH, y * TILE_WIDTH);
                break;
            case '.': // Nothing
                    break;
            default:
                std::printf("Unexpected symbol %c at (%d,%d)", tile, x, y);
                break;
            }
        }
    }


    _spriteBatch.end();

}
Level::~Level() {

}

void Level::draw() {
    _spriteBatch.renderBatch();
}