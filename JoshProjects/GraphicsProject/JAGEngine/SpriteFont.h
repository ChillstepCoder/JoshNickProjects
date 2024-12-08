/* 
    This is a modified version of the SpriteFont class from the
    Seed Of Andromeda source code.
    Use it for any of your projects, commercial or otherwise,
    free of charge, but do not remove this disclaimer.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
    ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
    EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
    INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
    RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
    OF THIS SOFTWARE.

    November 28 2014
    Original Author: Cristian Zaloj
    Modified By: Benjamin Arnold
*/
// SpriteFont.h

#pragma once
#ifndef SpriteFont_h__
#define SpriteFont_h__

#include <glm/glm.hpp>
#include <map>
#include <string>
#include "Vertex.h"
#include <GL/freetype-gl.h>

namespace JAGEngine {

  class SpriteBatch;

#define FIRST_PRINTABLE_CHAR ((char)32)
#define LAST_PRINTABLE_CHAR ((char)126)

  /// For text justification
  enum class Justification {
    LEFT, MIDDLE, RIGHT
  };

  class SpriteFont {
  public:
    SpriteFont();

    SpriteFont(const char* font, int size);

    ~SpriteFont();

    void init(const char* font, int size);
    void dispose();

    bool isValid() const { return m_textureID != 0 && m_font != nullptr && m_atlas != nullptr; }

    /// Measures the dimensions of the text
    glm::vec2 measure(const char* s);

    /// Draws using a spritebatch
    void draw(SpriteBatch& batch, const char* s, glm::vec2 position, glm::vec2 scaling,
      float depth, ColorRGBA8 tint, Justification just = Justification::LEFT);

    //getters
    GLuint getTextureID() const { return m_textureID; }
    int getFontHeight() const;


  private:
    static constexpr bool DEBUG_OUTPUT = false;

    ftgl::texture_atlas_t* m_atlas;
    ftgl::texture_font_t* m_font;
    std::map<char, ftgl::texture_glyph_t*> m_glyphMap;

    int m_fontHeight;
    GLuint m_textureID; 

  };

}

#endif // SpriteFont_h__
