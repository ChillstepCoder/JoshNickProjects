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
// 
// 
// 
// 
//     SpriteFont::SpriteFont(const char* font, int size, char cs, char ce) {
//         init(font, size, cs, ce);
//     }
// 
//     void SpriteFont::init(const char* font, int size) {
//         init(font, size, FIRST_PRINTABLE_CHAR, LAST_PRINTABLE_CHAR);
//     }
// 
//     void SpriteFont::init(const char* font, int size, char cs, char ce) {
//         // Initialize FreeType-GL
//         m_font = ftgl::texture_font_new_from_file,(font);
//         if (!m_font) {
//             fprintf(stderr, "Failed to load font: %s\n", font);
//             throw std::runtime_error("Failed to load font");
//         }
// 
//         m_fontHeight = size;
//         m_regStart = cs;
//         m_regLength = ce - cs + 1;
//         //int padding = size / 8;
// 
//         // Load all glyphs into the font
//         ftgl::texture_font_load_glyphs(m_font, cs, ce);
// 
//         // Additional setup for texture + glyphs if needed
//         setupTexture();
// 
// 
//     }
// 
//     void SpriteFont::setupTexture() {
//         // Any additional setup for the texture
//         glGenTextures(1, &m_texID);
//         glBindTexture(GL_TEXTURE_2D, m_texID);
//     }
// 
//     void SpriteFont::dispose() {
//         // Clean up FreeType-GL resources
//         ftgl::texture_font_delete(m_font);
//         glDeleteTextures(1, &m_texID);
//     }
// 
//     std::vector<int>* SpriteFont::createRows(glm::ivec4* rects, int rectsLength, int r, int padding, int& w) {
//         // Blank initialize
//         std::vector<int>* l = new std::vector<int>[r]();
//         int* cw = new int[r]();
//         for (int i = 0; i < r; i++) {
//             cw[i] = padding;
//         }
// 
//         // Loop through all glyphs
//         for (int i = 0; i < rectsLength; i++) {
//             // Find row for placement
//             int ri = 0;
//             for (int rii = 1; rii < r; rii++)
//                 if (cw[rii] < cw[ri]) ri = rii;
// 
//             // Add width to that row
//             cw[ri] += rects[i].z + padding;
// 
//             // Add glyph to the row list
//             l[ri].push_back(i);
//         }
// 
//         // Find the max width
//         w = 0;
//         for (int i = 0; i < r; i++) {
//             if (cw[i] > w) w = cw[i];
//         }
// 
//         return l;
//     }
// 
//     glm::vec2 SpriteFont::measure(const char* s) {
//         glm::vec2 size(0, m_fontHeight);
//         float cw = 0;
// 
//         for (int si = 0; s[si] != 0; si++) {
//             char c = s[si];
// 
//             if (c == '\n') {
//                 size.y += m_fontHeight;
//                 if (size.x < cw)
//                     size.x = cw;
//                 cw = 0;
//             }
//             else {
//                 // Check for correct glyph
//                 float charWidth = ftgl::texture_font_get_glyph(m_font, c);
//                 cw += charWidth;
//             }
//         }
//         if (size.x < cw)
//             size.x = cw;
//         return size;
//     }
// 
//     void SpriteFont::draw(SpriteBatch& batch, const char* s, glm::vec2 position, glm::vec2 scaling,
//         float depth, ColorRGBA8 tint, Justification just /* = Justification::LEFT */, float rotation) {
//         glm::vec2 tp = position;
//         // Apply justification
//         if (just == Justification::MIDDLE) {
//             tp.x -= measure(s).x * scaling.x / 2;
//         }
//         else if (just == Justification::RIGHT) {
//             tp.x -= measure(s).x * scaling.x;
//         }
//         for (int si = 0; s[si] != 0; si++) {
//             char c = s[si];
//             if (c == '\n') {
//                 tp.y += m_fontHeight * scaling.y;
//                 tp.x = position.x;
//             }
//             else {
//                 // Render the character
//                 ftgl::render_glyph(m_font, c, tp, scaling, depth, tint, rotation);
//                 float charWidth = ftgl::texture_font_get_glyph(m_font, c);
//                 tp.x += charWidth * scaling.x;
//             }
//         }
//     }
// 
// }