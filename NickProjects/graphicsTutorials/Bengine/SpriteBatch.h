#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "Vertex.h"

namespace Bengine {

    enum class GlyphSortType {
        NONE,
        FRONT_TO_BACK,
        BACK_TO_FRONT,
        TEXTURE
    };

    class Glyph {
    public:
        Glyph() : depth(0), rotation(0.0f), texture(0) {};
        Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& Color, float Rotation) :
            texture(Texture),
            depth(Depth),
            rotation(Rotation){

            topLeft.color = Color;
            topLeft.setPosition(destRect.x, destRect.y + destRect.w);
            topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

            bottomLeft.color = Color;
            bottomLeft.setPosition(destRect.x, destRect.y);
            bottomLeft.setUV(uvRect.x, uvRect.y);

            bottomRight.color = Color;
            bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
            bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

            topRight.color = Color;
            topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
            topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

            //m_glyphs.rotation = rotation; //<Rotation

        }

        GLuint texture;
        float depth;

        Vertex topLeft;
        Vertex bottomLeft;
        Vertex topRight;
        Vertex bottomRight;
        float rotation;
    };

    class RenderBatch {
    public:
        RenderBatch(GLuint Offset, GLuint NumVertices, GLuint Texture) : offset(Offset), 
            numVertices(NumVertices), texture(Texture) {

        }
        GLuint offset;
        GLuint numVertices;
        GLuint texture;
    };

    class SpriteBatch
    {
    public:
        SpriteBatch();
        ~SpriteBatch();

        void init();

        void begin(GlyphSortType sortType = GlyphSortType::TEXTURE);
        void end();

        void draw(const glm::vec4& destRect,const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, float rotation);

        void renderBatch();

        bool isInitialized() const {
            return m_initialized;
        }

    private:
        void createRenderBatches();
        void createVertexArray();
        void sortGlyphs();

        static bool compareFrontToBack(Glyph* a, Glyph* b);
        static bool compareBackToFront(Glyph* a, Glyph* b);
        static bool compareTexture(Glyph* a, Glyph* b);

        GLuint m_vbo;
        GLuint m_vao;

        GlyphSortType m_sortType;

        std::vector<Glyph*> m_glyphPointers; //< This is for sorting
        std::vector<Glyph> m_glyphs; //< These are the actual glyphs
        std::vector<RenderBatch> m_renderBatches;

        bool m_initialized = false;
    };

}