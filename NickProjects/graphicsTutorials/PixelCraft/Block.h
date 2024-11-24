#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>

enum class BlockID {
    GRASS = 0,
    DIRT = 1,
    STONE = 2,
    WATER = 3,
    COUNT = 4
};

class BlockDef {
public:
    BlockDef();
    ~BlockDef();

    void init(glm::vec4 uvRect, Bengine::ColorRGBA8 color, Bengine::GLTexture texture);


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
    bool isEmpty() const { return B2_IS_NULL(m_BodyID); }

    void clearID() {
        m_BodyID.index1 = 0;
    }

private:
    b2BodyId m_BodyID = b2_nullBodyId;
    BlockID m_BlockID;

};

