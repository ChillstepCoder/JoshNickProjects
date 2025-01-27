#pragma once
#include <Box2D/box2d.h>
#include <glm/glm.hpp>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>
#include <fstream>

enum class BlockID {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    DEEPSTONE = 4,
    DEEPERSTONE = 5,
    COPPER = 6,
    IRON = 7,
    GOLD = 8,
    DIAMOND = 9,
    COBALT = 10,
    MYTHRIL = 11,
    ADAMANTITE = 12,
    COSMILITE = 13,
    PRIMORDIAL = 14,
    WATER = 15,
    COUNT = 16
};

struct SubTexture {
    SubTexture(unsigned int textureID, glm::vec4 uvRect) : m_textureID(textureID), m_uvRect(uvRect) {}

    unsigned int m_textureID;   
    glm::vec4 m_uvRect;
};

class BlockDef {
public:
    BlockDef();
    ~BlockDef();

    void init(glm::vec4 uvRect, Bengine::ColorRGBA8 color, Bengine::GLTexture texture, GLuint m_textureID, bool isConnectedTexture);


    glm::vec4 m_uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    Bengine::ColorRGBA8 m_color;
    bool m_isConnectedTexture;
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
    int getWaterAmount() const { return m_waterAmount; }

    void setWaterAmount(float amount) {
        m_waterAmount = amount;
    }

    bool isEmpty() const { return B2_IS_NULL(m_BodyID); }

    void saveToFile(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&m_BlockID), sizeof(m_BlockID));
        out.write(reinterpret_cast<const char*>(&m_waterAmount), sizeof(m_waterAmount));
    }

    void loadFromFile(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&m_BlockID), sizeof(m_BlockID));
        in.read(reinterpret_cast<char*>(&m_waterAmount), sizeof(m_waterAmount));
    }


    void clearID() {
        m_BodyID.index1 = 0;
    }

private:
    b2BodyId m_BodyID = b2_nullBodyId;
    BlockID m_BlockID = BlockID::AIR;
    int m_waterAmount = 0;
};

