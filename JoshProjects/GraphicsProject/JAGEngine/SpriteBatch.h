//SpriteBatch.h

#pragma once

#include "Vertex.h"
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


// These are for checkGLError, move with it when moving to new file
#include <string>
#include <iostream>
// Checks the output of glGetError and prints an appropriate error message if needed.
// TODO: Put this in a separate file
inline bool checkGlError(const char* errorLocation) {
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        switch (error) {
            case GL_INVALID_ENUM:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1280: GL_INVALID_ENUM");
                break;
            case GL_INVALID_VALUE:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1281: GL_INVALID_VALUE");
                break;
            case GL_INVALID_OPERATION:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1282: GL_INVALID_OPERATION");
                break;
            case GL_STACK_OVERFLOW:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1283: GL_STACK_OVERFLOW");
                break;
            case GL_STACK_UNDERFLOW:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1284: GL_STACK_UNDERFLOW");
                break;
            case GL_OUT_OF_MEMORY:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1285: GL_OUT_OF_MEMORY");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code 1285: GL_INVALID_FRAMEBUFFER_OPERATION");
                break;
            default:
                std::cout << std::string("At " + std::string(errorLocation) + ". Error code " + std::to_string(error) + ": UNKNOWN");
                break;
        }
        //__debugbreak(); // This automatically triggers a breakpoint if the debugger is attached
        return true;
    }
    return false;
}

namespace JAGEngine {


  enum class GlyphSortType {
    NONE,
    FRONT_TO_BACK,
    BACK_TO_FRONT,
    TEXTURE
  };

  class Glyph {
  public:
    Glyph() {};
    Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& color);
    Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& color, float angle);

    GLuint texture;
    float depth;

    Vertex topLeft;
    Vertex bottomLeft;
    Vertex topRight;
    Vertex bottomRight;
  private:
    glm::vec2 rotatePoint(glm::vec2 pos, float angle);
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
    void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color);

    void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, float angle);

    void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, const glm::vec2& dir);

    void renderBatch();

  private:
    void createRenderBatches();
    void createVertexArray();
    void sortGlyphs();
    void createWhiteTexture();

    static bool compareFrontToBack(Glyph* a, Glyph* b);
    static bool compareBackToFront(Glyph* a, Glyph* b);
    static bool compareTexture(Glyph* a, Glyph* b);

    GLuint _vbo;
    GLuint _vao;

    GlyphSortType _sortType;

    std::vector<Glyph*> _glyphPointers; //for sorting
    std::vector<Glyph> _glyphs; //aCTUAL GLYPHS
    std::vector<RenderBatch> _renderBatches;

    GLuint m_whiteTexture;
  };
}
