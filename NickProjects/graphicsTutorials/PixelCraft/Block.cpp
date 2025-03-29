#include "Block.h"
#include <Bengine/ResourceManager.h>
#include "LightingSystem.h"
#include <iostream>



BlockDef::BlockDef() {
    //Empty
}
BlockDef::~BlockDef() {
    //Empty
}


void BlockDef::init(glm::vec4 uvRect, Bengine::ColorRGBA8 color, Bengine::GLTexture texture, GLuint textureID, bool isConnectedTexture) {
    m_uvRect = uvRect;
    m_color = color;
    m_texture = texture;
    m_textureID = textureID;
    m_isConnectedTexture = isConnectedTexture;
}

glm::vec4 SubTexture::getSubUVRect(glm::ivec2 cellPos, glm::ivec2 dimsCells) {
    float uMin = float(cellPos.x) / dimsCells.x;  // Left edge of the tile
    float vMin = float(cellPos.y) / dimsCells.y;  // Bottom edge of the tile

    float width = 1.0f / dimsCells.x;
    float height = 1.0f / dimsCells.y;

    return glm::vec4(uMin, vMin, width, height);  // UV Rect: {uMin, vMin, width, height}
}



BlockDefRepository::BlockDefRepository() {
    //Empty
}
BlockDefRepository::~BlockDefRepository() {
    //Empty
}


void BlockDefRepository::initBlockDefs() {
    m_blockDefs.resize((int)BlockID::COUNT);

    Bengine::ColorRGBA8 textureColor(255, 255, 255, 255);

    glm::vec4 uvRect(0.0f, 0.0f, 1.0f, 1.0f);

    Bengine::TextureFilterMode filterMode = Bengine::TextureFilterMode::Nearest;

    m_blockDefs[(int)BlockID::AIR].init(uvRect, Bengine::ColorRGBA8(0,0,0,0), Bengine::GLTexture(), 0, false);
    m_blockDefs[(int)BlockID::GRASS].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Grass.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Grass.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::DIRT].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Dirt.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Dirt.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::STONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Stone.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Stone.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::DEEPSTONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/DeepStone.png", filterMode), Bengine::ResourceManager::getTexture("Textures/DeepStone.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::DEEPERSTONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/DeeperStone.png", filterMode), Bengine::ResourceManager::getTexture("Textures/DeeperStone.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::COPPER].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Copper.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Copper.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::IRON].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Iron.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Iron.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::GOLD].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Gold.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Gold.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::DIAMOND].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Diamond.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Diamond.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::COBALT].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Cobalt.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Cobalt.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::MYTHRIL].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Mythril.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Mythril.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::ADAMANTITE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Adamantite.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Adamantite.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::COSMILITE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Cosmilite.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Cosmilite.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::PRIMORDIAL].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Primordial.png", filterMode), Bengine::ResourceManager::getTexture("Textures/Primordial.png", filterMode).id, true);
    m_blockDefs[(int)BlockID::WATER].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/waterBlock.png", filterMode), Bengine::ResourceManager::getTexture("Textures/waterBlock.png", filterMode).id, false);


    assert(m_blockDefs[(int)BlockID::STONE].m_color == textureColor);

}


Block::Block() {

}
Block::~Block() {

}

void Block::init(b2WorldId world, BlockID blockID, const glm::vec2& position) {
    m_BlockID = blockID;

    if (blockID == BlockID::AIR) {
        return;
    }

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.position = b2Vec2(position.x, position.y);
    bodyDef.fixedRotation = true;
    m_BodyID = b2CreateBody(world, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(1.0f / 2.0f, 1.0f / 2.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();

    if (blockID == BlockID::WATER) {
        m_waterAmount = 100;
        return;
    }

    shapeDef.density = 1.0f;
    shapeDef.friction = 0.0f;
    shapeDef.restitution = 0.0f;
    b2CreatePolygonShape(m_BodyID, &shapeDef, &dynamicBox);

}
