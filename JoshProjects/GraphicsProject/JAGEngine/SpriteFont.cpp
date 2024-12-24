//SpriteFont.cpp

#include "SpriteFont.h"
#include "SpriteBatch.h"
#include <iostream>
#include <string>

namespace JAGEngine {

  SpriteFont::SpriteFont() : m_atlas(nullptr), m_font(nullptr), m_fontHeight(0), m_textureID(0) {}

  SpriteFont::SpriteFont(const char* font, int size) : m_atlas(nullptr), m_font(nullptr), m_fontHeight(0), m_textureID(0) {
    init(font, size);
  }

  SpriteFont::~SpriteFont() {
    dispose();
  }

  void SpriteFont::init(const char* font, int size) {
    m_atlas = ftgl::texture_atlas_new(512, 512, 1);
    ftgl::texture_atlas_clear(m_atlas);  // Clear atlas first
    m_font = ftgl::texture_font_new_from_file(m_atlas, size, font);
    if (DEBUG_OUTPUT) {
      std::cout << "Atlas created: size=" << size
        << " width=" << m_atlas->width
        << " height=" << m_atlas->height
        << " depth=" << m_atlas->depth << std::endl;
    }
    if (!m_font) {
      if (DEBUG_OUTPUT) {
        std::cerr << "Failed to load font: " << font << std::endl;
      }
      return;
    }

    m_fontHeight = m_font->height;
    if (DEBUG_OUTPUT) {
      std::cout << "Font height: " << m_fontHeight << std::endl;
    }

    // Load all ASCII printable characters
    const char* cache = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    // Debug each character's metrics before loading
    
      for (const char* p = cache; *p; ++p) {
        ftgl::texture_glyph_t* glyph = ftgl::texture_font_get_glyph(m_font, p);
        if (DEBUG_OUTPUT) {
        if (glyph) {
          std::cout << "Pre-load metrics for '" << *p << "': "
            << "width=" << glyph->width
            << " height=" << glyph->height
            << " offset_x=" << glyph->offset_x
            << " offset_y=" << glyph->offset_y << std::endl;
        }
      }
    }

    size_t missedGlyphs = ftgl::texture_font_load_glyphs(m_font, cache);
    // Create OpenGL texture
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_atlas->width, m_atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, m_atlas->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Store glyph information
    for (int i = 0; cache[i] != '\0'; ++i) {
      char c = cache[i];
      ftgl::texture_glyph_t* glyph = ftgl::texture_font_get_glyph(m_font, &c);
      if (glyph) {
        m_glyphMap[c] = glyph;
      }
    }
    // After loading glyphs
    if (DEBUG_OUTPUT) {
      for (const auto& pair : m_glyphMap) {
        auto glyph = pair.second;
        if (DEBUG_OUTPUT) {
          std::cout << "Glyph '" << pair.first << "' UV coords: "
            << glyph->s0 << ", " << glyph->t0 << ", "
            << glyph->s1 << ", " << glyph->t1 << std::endl;
        }
      }
    }

  }

  void SpriteFont::dispose() {
    if (m_textureID != 0) {
      glDeleteTextures(1, &m_textureID);
      m_textureID = 0;
    }
    if (m_font) {
      ftgl::texture_font_delete(m_font);
      m_font = nullptr;
    }
    if (m_atlas) {
      ftgl::texture_atlas_delete(m_atlas);
      m_atlas = nullptr;
    }
    m_glyphMap.clear();
  }

  int SpriteFont::getFontHeight() const {
    return m_fontHeight;
  }

  glm::vec2 SpriteFont::measure(const char* s) {
    glm::vec2 size(0, m_fontHeight);
    float xpos = 0;
    for (int i = 0; s[i] != 0; i++) {
      auto it = m_glyphMap.find(s[i]);
      if (it != m_glyphMap.end()) {
        ftgl::texture_glyph_t* glyph = it->second;
        xpos += glyph->advance_x;
      }
    }
    size.x = xpos;
    return size;
  }

  void SpriteFont::draw(SpriteBatch& batch, const char* s, glm::vec2 position, glm::vec2 scaling,
    float depth, ColorRGBA8 tint, Justification just) {

    if (DEBUG_OUTPUT) {
      std::cout << "Draw called with text: '" << s << "' at pos: "
        << position.x << "," << position.y << std::endl;
    }

    if (!m_font || !m_textureID) {
      if (DEBUG_OUTPUT) {
        std::cout << "Invalid font state: font=" << (m_font != nullptr)
          << " textureID=" << m_textureID << std::endl;
      }
      return;
    }

    glm::vec2 pen = position;

    // Apply justification
    if (just == Justification::MIDDLE) {
      pen.x -= measure(s).x * scaling.x / 2;
    }
    else if (just == Justification::RIGHT) {
      pen.x -= measure(s).x * scaling.x;
    }

    float baseline = pen.y;

    for (const char* p = s; *p; ++p) {
      auto it = m_glyphMap.find(*p);
      if (it != m_glyphMap.end()) {
        ftgl::texture_glyph_t* glyph = it->second;
        float x = pen.x + glyph->offset_x * scaling.x;
        float y = baseline - (glyph->height - glyph->offset_y) * scaling.y;
        float w = glyph->width * scaling.x;
        float h = glyph->height * scaling.y;

        glm::vec4 destRect(x, y, w, h);
        glm::vec4 uvRect(glyph->s0, glyph->t0, glyph->s1 - glyph->s0, glyph->t1 - glyph->t0);

        if (DEBUG_OUTPUT) {
          std::cout << "  Glyph '" << *p << "' at: " << destRect.x << ", " << destRect.y
            << " size: " << destRect.z << "x" << destRect.w
            << " UV: " << uvRect.x << ", " << uvRect.y << ", " << uvRect.z << ", " << uvRect.w
            << " Texture ID: " << m_textureID << std::endl;
        }

        // TODO: REMOVE
        batch.draw(destRect, uvRect, m_textureID, depth, tint);

        pen.x += glyph->advance_x * scaling.x;
      }
    }
  }

}
