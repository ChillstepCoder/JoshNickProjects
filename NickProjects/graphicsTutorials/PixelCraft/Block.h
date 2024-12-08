#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>

enum class BlockID {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    WATER = 4,
    COUNT = 5
};

class BlockDef {
public:
    BlockDef();
    ~BlockDef();

    void init(glm::vec4 uvRect, Bengine::ColorRGBA8 color, Bengine::GLTexture texture, GLuint m_textureID);


    glm::vec4 m_uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texture;
    GLuint m_textureID;
private:
    BlockID m_blockID;
};


class BlockDefRepository {
public:
    BlockDefRepository();
    ~BlockDefRepository();

    static void initBlockDefs();

    const BlockDef& getDef(BlockID id) { return m_blockDefs[(int)id]; }
    static const glm::vec4 getUVRect(BlockID id) { 

        glm::vec4 UVRect = m_blockDefs[(int)id].m_uvRect;

        return UVRect;
    }
    static const GLuint getTextureID(BlockID id) {

        GLuint textureID = m_blockDefs[(int)id].m_textureID;

        return textureID;
    }
    static const Bengine::GLTexture getTexture(BlockID id) {

        Bengine::GLTexture texture = m_blockDefs[(int)id].m_texture;

        return texture;
    }
    static const Bengine::ColorRGBA8 getColor(BlockID id) {

        Bengine::ColorRGBA8 color = m_blockDefs[(int)id].m_color;

        return color;
    }

private:
    inline static std::vector<BlockDef> m_blockDefs;
};

class BlockRenderer {
public:
    static void renderBlock(Bengine::SpriteBatch& sb, const BlockDef& blockDef, glm::vec2 position);
};


class Block
{
public:
    Block();
    ~Block();

    void init(b2WorldId world, BlockID blockID, const glm::vec2& position);

    b2BodyId getBodyID() const { return m_BodyID; }
    BlockID getBlockID() const { return m_BlockID; }
    void setBlockID(BlockID id) { m_BlockID = id; }
    float getWaterAmount() { return m_waterAmount; }

    void setWaterAmount(float amount) {
        m_waterAmount = amount;
    }

    bool isEmpty() const { return B2_IS_NULL(m_BodyID); }

    void clearID() {
        m_BodyID.index1 = 0;
    }

private:
    b2BodyId m_BodyID = b2_nullBodyId;
    BlockID m_BlockID;
    float m_waterAmount;
};

