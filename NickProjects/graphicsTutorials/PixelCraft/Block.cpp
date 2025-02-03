#include "Block.h"
#include <Bengine/ResourceManager.h>
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

glm::vec4 BlockDef::getSubUVRect(glm::ivec2 cellPos, glm::ivec2 dimsCells) {
    float uMin = float(cellPos.x) / dimsCells.x;  // Left edge of the tile
    float vMin = float(cellPos.y / dimsCells.y);  // Bottom edge of the tile

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

    Bengine::ColorRGBA8 waterColor(255, 255, 255, 155);



    m_blockDefs[(int)BlockID::AIR].init(uvRect, Bengine::ColorRGBA8(0,0,0,0), Bengine::GLTexture(), 0, false);
    m_blockDefs[(int)BlockID::GRASS].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Grass.png"), Bengine::ResourceManager::getTexture("Textures/Grass.png").id, true);
    m_blockDefs[(int)BlockID::DIRT].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Dirt.png"), Bengine::ResourceManager::getTexture("Textures/Dirt.png").id, true);
    m_blockDefs[(int)BlockID::STONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Stone.png"), Bengine::ResourceManager::getTexture("Textures/Stone.png").id, true);
    m_blockDefs[(int)BlockID::DEEPSTONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/DeepStone.png"), Bengine::ResourceManager::getTexture("Textures/DeepStone.png").id, true);
    m_blockDefs[(int)BlockID::DEEPERSTONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/DeeperStone.png"), Bengine::ResourceManager::getTexture("Textures/DeeperStone.png").id, true);
    m_blockDefs[(int)BlockID::COPPER].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Copper.png"), Bengine::ResourceManager::getTexture("Textures/Copper.png").id, true);
    m_blockDefs[(int)BlockID::IRON].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Iron.png"), Bengine::ResourceManager::getTexture("Textures/Iron.png").id, true);
    m_blockDefs[(int)BlockID::GOLD].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Gold.png"), Bengine::ResourceManager::getTexture("Textures/Gold.png").id, true);
    m_blockDefs[(int)BlockID::DIAMOND].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Diamond.png"), Bengine::ResourceManager::getTexture("Textures/Diamond.png").id, true);
    m_blockDefs[(int)BlockID::COBALT].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Cobalt.png"), Bengine::ResourceManager::getTexture("Textures/Cobalt.png").id, true);
    m_blockDefs[(int)BlockID::MYTHRIL].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Mythril.png"), Bengine::ResourceManager::getTexture("Textures/Mythril.png").id, true);
    m_blockDefs[(int)BlockID::ADAMANTITE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Adamantite.png"), Bengine::ResourceManager::getTexture("Textures/Adamantite.png").id, true);
    m_blockDefs[(int)BlockID::COSMILITE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Cosmilite.png"), Bengine::ResourceManager::getTexture("Textures/Cosmilite.png").id, true);
    m_blockDefs[(int)BlockID::PRIMORDIAL].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/Primordial.png"), Bengine::ResourceManager::getTexture("Textures/Primordial.png").id, true);
    m_blockDefs[(int)BlockID::WATER].init(uvRect, waterColor, Bengine::ResourceManager::getTexture("Textures/waterBlock.png"), Bengine::ResourceManager::getTexture("Textures/waterBlock.png").id, false);


    assert(m_blockDefs[(int)BlockID::STONE].m_color == textureColor);

}



void BlockRenderer::renderBlock(Bengine::SpriteBatch& sb, const BlockDef& blockDef, glm::vec2 position) {
    glm::vec4 destRect;

    destRect.x = (position.x - 0.5f);
    destRect.y = (position.y - 0.5f);
    destRect.z = 1.0f;
    destRect.w = 1.0f;

    sb.draw(destRect, blockDef.m_uvRect, blockDef.m_textureID, 0.0f, blockDef.m_color, 0.0f);
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
