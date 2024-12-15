#include "Block.h"
#include <Bengine/ResourceManager.h>
#include <iostream>

BlockDef::BlockDef() {
    //Empty
}
BlockDef::~BlockDef() {
    //Empty
}


void BlockDef::init(glm::vec4 uvRect, Bengine::ColorRGBA8 color, Bengine::GLTexture texture, GLuint textureID) {
    m_uvRect = uvRect;
    m_color = color;
    m_texture = texture;
    m_textureID = textureID;
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






    m_blockDefs[(int)BlockID::AIR].init(uvRect, Bengine::ColorRGBA8(0,0,0,0), Bengine::GLTexture(), 0);
    m_blockDefs[(int)BlockID::GRASS].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png"), Bengine::ResourceManager::getTexture("Textures/connectedGrassBlock.png").id);
    m_blockDefs[(int)BlockID::DIRT].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png"), Bengine::ResourceManager::getTexture("Textures/connectedDirtBlock.png").id);
    m_blockDefs[(int)BlockID::STONE].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/connectedStoneBlock.png"), Bengine::ResourceManager::getTexture("Textures/connectedStoneBlock.png").id);
    m_blockDefs[(int)BlockID::WATER].init(uvRect, textureColor, Bengine::ResourceManager::getTexture("Textures/waterBlock.png"), Bengine::ResourceManager::getTexture("Textures/waterBlock.png").id);

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
