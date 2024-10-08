#include "SpriteFont.h"
#include <stdexcept>

#include <freetype-gl/freetype-gl.h>

int closestPow2(int i) {
    i--;
    int pi = 1;
    while (i > 0) {
        i >>= 1;
        pi <<= 1;
    }
    return pi;
}

#define MAX_TEXTURE_RES 4096

namespace Bengine {

    // Initialize static members
    ftgl::texture_font_t* SpriteFont::m_font = nullptr;
    ftgl::texture_atlas_t* SpriteFont::m_atlas = nullptr;
    int SpriteFont::m_fontHeight = 0;
    unsigned int SpriteFont::m_textureID = 0;

    SpriteFont::~SpriteFont() {
        dispose();
    }

    void SpriteFont::init(const char* font, int size) {
        init(font, size, FIRST_PRINTABLE_CHAR, LAST_PRINTABLE_CHAR);
    }

    void SpriteFont::init(const char* font, int size, char startChar, char endChar) {
        // Create texture atlas
        m_atlas = ftgl::texture_atlas_new(512, 512, 1);
        if (!m_atlas) {
            throw std::runtime_error("Failed to create texture atlas");
        }

        // Create font from file
        m_font = ftgl::texture_font_new_from_file(m_atlas, size, font);
        if (!m_font) {
            ftgl::texture_atlas_delete(m_atlas);
            throw std::runtime_error("Failed to load font");
        }

        m_fontHeight = size;

        // Load glyphs
        const char* text = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}\\|;:'\",.<>/?`~";
        size_t missedGlyphs = ftgl::texture_font_load_glyphs(m_font, text);
        if (missedGlyphs != 0) {
            throw std::runtime_error("Failed to load glyphs");
        }

        // Create OpenGL texture
        createTexture();
    }

    void SpriteFont::createTexture() {
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                     m_atlas->width, m_atlas->height, 0,
                     GL_RED, GL_UNSIGNED_BYTE, m_atlas->data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    void SpriteFont::dispose() {
        if (m_font) {
            ftgl::texture_font_delete(m_font);
            m_font = nullptr;
        }
        if (m_atlas) {
            ftgl::texture_atlas_delete(m_atlas);
            m_atlas = nullptr;
        }
        if (m_textureID) {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        }
    }

    glm::vec2 SpriteFont::measure(const char* s) {
        if (!s || !m_font) return glm::vec2(0.0f);

        glm::vec2 size(0, m_fontHeight);
        float currentWidth = 0.0f;

        for (const char* p = s; *p; ++p) {
            if (*p == '\n') {
                size.y += m_fontHeight;
                size.x = std::max(size.x, currentWidth);
                currentWidth = 0.0f;
                continue;
            }

            ftgl::texture_glyph_t* glyph = ftgl::texture_font_get_glyph(m_font, p);
            if (glyph) {
                currentWidth += glyph->advance_x;
            }
        }

        size.x = std::max(size.x, currentWidth);
        return size;
    }

    void SpriteFont::draw(SpriteBatch& batch, const char* s, glm::vec2 position,
        glm::vec2 scaling, float depth, ColorRGBA8 tint, Justification just, float rotation) {

        if (!s || !m_font) return;

        glm::vec2 pos = position;
        if (just != Justification::LEFT) {
            glm::vec2 size = measure(s);
            if (just == Justification::MIDDLE) {
                pos.x -= (size.x * scaling.x) / 2.0f;
            }
            else if (just == Justification::RIGHT) {
                pos.x -= size.x * scaling.x;
            }
        }

        float pen_x = pos.x;
        float pen_y = pos.y;

        for (const char* p = s; *p; ++p) {
            if (*p == '\n') {
                pen_x = pos.x;
                pen_y += m_fontHeight * scaling.y;
                continue;
            }

            ftgl::texture_glyph_t* glyph = ftgl::texture_font_get_glyph(m_font, p);
            if (!glyph) continue;

            float x0 = pen_x + glyph->offset_x * scaling.x;
            float y0 = pen_y + glyph->offset_y * scaling.y;
            float x1 = x0 + glyph->width * scaling.x;
            float y1 = y0 - glyph->height * scaling.y;

            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;

            glm::vec4 destRect(x0, y1, x1 - x0, y0 - y1);
            glm::vec4 uvRect(s0, t0, s1 - s0, t1 - t0);

            batch.draw(destRect, uvRect, m_textureID, depth, tint, rotation);

            pen_x += glyph->advance_x * scaling.x;
        }
    }

}