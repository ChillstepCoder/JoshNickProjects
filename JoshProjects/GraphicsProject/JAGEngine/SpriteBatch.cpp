//SpriteBatch.cpp

#include "SpriteBatch.h"
#include <algorithm>
#include <iostream>

namespace JAGEngine {

   Glyph::Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& color) :
    texture(Texture), depth(Depth) {
    topLeft.color = color;
    topLeft.setPosition(destRect.x, destRect.y + destRect.w);
    topLeft.setUV(uvRect.x, uvRect.y);

    bottomLeft.color = color;
    bottomLeft.setPosition(destRect.x, destRect.y);
    bottomLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

    bottomRight.color = color;
    bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
    bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

    topRight.color = color;
    topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
    topRight.setUV(uvRect.x + uvRect.z, uvRect.y);
  }

  Glyph::Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& color, float angle) :
    texture(Texture), depth(Depth) {

    glm::vec2 halfDims(destRect.z / 2.0f,destRect.w / 2.0f);

    //get points centered at origin
    glm::vec2 tl(-halfDims.x, halfDims.y);
    glm::vec2 bl(-halfDims.x, -halfDims.y);
    glm::vec2 br(halfDims.x, -halfDims.y);
    glm::vec2 tr(halfDims.x, halfDims.y);

    //rotate the points
    tl = rotatePoint(tl, angle) + halfDims;
    bl = rotatePoint(bl, angle) + halfDims;
    br = rotatePoint(br, angle) + halfDims;
    tr = rotatePoint(tr, angle) + halfDims;

    topLeft.color = color;
    topLeft.setPosition(destRect.x + tl.x, destRect.y + tl.y);
    topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

    bottomLeft.color = color;
    bottomLeft.setPosition(destRect.x + bl.x, destRect.y + bl.y);
    bottomLeft.setUV(uvRect.x, uvRect.y);

    bottomRight.color = color;
    bottomRight.setPosition(destRect.x + br.x, destRect.y + br.y);
    bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

    topRight.color = color;
    topRight.setPosition(destRect.x + tr.x, destRect.y + tr.y);
    topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
  }

  glm::vec2 Glyph::rotatePoint(glm::vec2 pos, float angle) {
    glm::vec2 newv;
    newv.x = pos.x * cos(angle) - pos.y * sin(angle);
    newv.y = pos.x * sin(angle) + pos.y * cos(angle);
    return newv;
  }

  SpriteBatch::SpriteBatch() : _vbo(0), _vao(0)
  {

  }
  SpriteBatch::~SpriteBatch() {

  }

  void SpriteBatch::init() {
    createVertexArray();
  }

  void SpriteBatch::begin(GlyphSortType sortType /*GlyphSortType::TEXTURE*/ ) {
    _sortType = sortType;
    _renderBatches.clear();
    _glyphs.clear();
  }

  void SpriteBatch::end() {

    //set up all pointers for glyph sorting
    _glyphPointers.resize(_glyphs.size());
    for (int i = 0; i < _glyphs.size(); i++) {
      _glyphPointers[i] = &_glyphs[i];
    }

    sortGlyphs();
    createRenderBatches();
  }

  void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color) {

    _glyphs.emplace_back(destRect, uvRect, texture, depth, color);

  }

  void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, float angle) {
    _glyphs.emplace_back(destRect, uvRect, texture, depth, color, angle);
  }

  void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, const glm::vec2& dir) {

    //compute the angle
    const glm::vec2 right(1.0f, 0.0f);
    float angle = acos(glm::dot(right, dir));
    if (dir.y < 0.0f) angle = -angle;

    _glyphs.emplace_back(destRect, uvRect, texture, depth, color, angle);
  }

  void SpriteBatch::renderBatch() {
    glBindVertexArray(_vao);
    for (int i = 0; i < _renderBatches.size(); i++) {
      glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);
      std::cout << "Rendering batch " << i << " with texture ID: " << _renderBatches[i].texture
        << ", offset: " << _renderBatches[i].offset
        << ", num vertices: " << _renderBatches[i].numVertices << std::endl;
      glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
    }
    glBindVertexArray(0);
  }

  void SpriteBatch::createRenderBatches() {
    std::vector <Vertex> vertices;
    vertices.resize(_glyphPointers.size() * 6);


    if (_glyphPointers.empty()) {
      return;
    }

    int offset = 0;
    int cv = 0; //current vertex
    _renderBatches.emplace_back(0, 6, _glyphPointers[0]->texture);
    vertices[cv++] = _glyphPointers[0]->topLeft;
    vertices[cv++] = _glyphPointers[0]->bottomLeft;
    vertices[cv++] = _glyphPointers[0]->bottomRight;
    vertices[cv++] = _glyphPointers[0]->bottomRight;
    vertices[cv++] = _glyphPointers[0]->topRight;
    vertices[cv++] = _glyphPointers[0]->topLeft;
    offset += 6;

     //current Glyph
    for (int cg = 1; cg < _glyphPointers.size(); cg++) {
      if (_glyphPointers[cg]->texture != _glyphPointers[cg - 1]->texture) {
        _renderBatches.emplace_back(offset, 6, _glyphPointers[cg]->texture);
      } else {
        _renderBatches.back().numVertices += 6;
      }
      
      vertices[cv++] = _glyphPointers[cg]->topLeft;
      vertices[cv++] = _glyphPointers[cg]->bottomLeft;
      vertices[cv++] = _glyphPointers[cg]->bottomRight;
      vertices[cv++] = _glyphPointers[cg]->bottomRight;
      vertices[cv++] = _glyphPointers[cg]->topRight;
      vertices[cv++] = _glyphPointers[cg]->topLeft;

      offset += 6;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    //orphan the buffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    //upload the data
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
     
  }

  void SpriteBatch::createVertexArray() {

    if (_vao == 0) {
      glGenVertexArrays(1, &_vao);
    }
    glBindVertexArray(_vao);

    if (_vbo == 0) {
      glGenBuffers(1, &_vbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);

  }

  void SpriteBatch::sortGlyphs() {
    switch (_sortType) {
    case GlyphSortType::BACK_TO_FRONT:
      std::stable_sort(_glyphPointers.begin(), _glyphPointers.end(), compareBackToFront);
      break;
    case GlyphSortType::FRONT_TO_BACK:
      std::stable_sort(_glyphPointers.begin(), _glyphPointers.end(), compareFrontToBack);
      break;
    case GlyphSortType::TEXTURE:
      std::stable_sort(_glyphPointers.begin(), _glyphPointers.end(), compareTexture);
      break;
    }
  }

  bool SpriteBatch::compareFrontToBack(Glyph* a, Glyph* b) {
    return (a->depth < b->depth);
  }

  bool SpriteBatch::compareBackToFront(Glyph* a, Glyph* b) {
    return (a->depth > b->depth);
  }

  bool SpriteBatch::compareTexture(Glyph* a, Glyph* b) {
    return (a->texture < b->texture);
  }

}


