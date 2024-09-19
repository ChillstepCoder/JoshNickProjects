#include "SpriteBatch.h"

#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace Bengine {

    SpriteBatch::SpriteBatch() : _vbo(0), _vao(0)
    {

    }

    SpriteBatch::~SpriteBatch() {

    }

    void SpriteBatch::init() {
        createVertexArray();
    }

    void SpriteBatch::begin(GlyphSortType sortType /* GlyphSortType::TEXTURE*/) {
        _sortType = sortType;
        _renderBatches.clear();
        _glyphs.clear();
    }

    void SpriteBatch::end() {
        sortGlyphs();
        createRenderBatches();
    }

    void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color& color, float rotation) {

        Glyph* newGlyph = new Glyph;
        newGlyph->texture = texture;
        newGlyph->depth = depth;

        newGlyph->topLeft.color = color;
        newGlyph->topLeft.setPosition(destRect.x, destRect.y + destRect.w);
        newGlyph->topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

        newGlyph->bottomLeft.color = color;
        newGlyph->bottomLeft.setPosition(destRect.x, destRect.y);
        newGlyph->bottomLeft.setUV(uvRect.x, uvRect.y);

        newGlyph->bottomRight.color = color;
        newGlyph->bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
        newGlyph->bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

        newGlyph->topRight.color = color;
        newGlyph->topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
        newGlyph->topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

        newGlyph->rotation = rotation; //<Rotation

        _glyphs.push_back(newGlyph);

    }

    void SpriteBatch::renderBatch() {

        glBindVertexArray(_vao);

        for (int i = 0; i < _renderBatches.size(); i++) {
            glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);

            glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
        }

        glBindVertexArray(0);
    }

    glm::vec2 rotatePoint(const glm::vec2& pos, float angle) {
        // Convert the angle from degrees to radians
        float radians = glm::radians(angle);
        // Rotate the point
        glm::vec2 result = glm::rotate(pos, radians);
        return result;
    }

    void SpriteBatch::createRenderBatches() {
        std::vector <Vertex> vertices;
        vertices.resize(_glyphs.size() * 6);

        if (_glyphs.empty()) {
            return;
        }

        int offset = 0;
        int cv = 0; //current vertex

        for (int cg = 0; cg < _glyphs.size(); cg++) {
            Glyph* glyph = _glyphs[cg];

            // Calculate the center of the sprite

            glm::vec2 center = (glm::vec2(glyph->topLeft.position.x, glyph->topLeft.position.y) +
                                glm::vec2(glyph->bottomRight.position.x, glyph->bottomRight.position.y)) * 0.5f;

            // Rotate each vertex around the center
            glm::vec2 t1 = rotatePoint(glm::vec2(glyph->topLeft.position.x, glyph->topLeft.position.y) - center, glyph->rotation) + center;
            glm::vec2 b1 = rotatePoint(glm::vec2(glyph->bottomLeft.position.x, glyph->bottomLeft.position.y) - center, glyph->rotation) + center;
            glm::vec2 br = rotatePoint(glm::vec2(glyph->bottomRight.position.x, glyph->bottomRight.position.y) - center, glyph->rotation) + center;
            glm::vec2 tr = rotatePoint(glm::vec2(glyph->topRight.position.x, glyph->topRight.position.y) - center, glyph->rotation) + center;

            // Update vertex positions
            glyph->topLeft.position = { t1.x, t1.y };
            glyph->bottomLeft.position = { b1.x, b1.y };
            glyph->bottomRight.position = { br.x, br.y };
            glyph->topRight.position = { tr.x, tr.y };

            if (cg == 0 || _glyphs[cg]->texture != _glyphs[cg - 1]->texture) {
                _renderBatches.emplace_back(offset, 6, _glyphs[cg]->texture);
            } else {
                _renderBatches.back().numVertices += 6;
            }
            vertices[cv++] = _glyphs[cg]->topLeft;
            vertices[cv++] = _glyphs[cg]->bottomLeft;
            vertices[cv++] = _glyphs[cg]->bottomRight;
            vertices[cv++] = _glyphs[cg]->bottomRight;
            vertices[cv++] = _glyphs[cg]->topRight;
            vertices[cv++] = _glyphs[cg]->topLeft;
            offset += 6;
        }

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        //orphan the buffer
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
        //upload the data
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
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

        //Tell openGl that we want to use the first attribute array.
        //We only need one array right now since we are only using this position.
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        //This is the position attribute pointer
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        //This is the color attribute pointer
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        //This is the UV attribute pointer
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

        glBindVertexArray(0);

    }

    void SpriteBatch::sortGlyphs() {
        switch (_sortType) {
            case GlyphSortType::BACK_TO_FRONT:
                std::stable_sort(_glyphs.begin(), _glyphs.end(), compareBackToFront);
                break;
            case GlyphSortType::FRONT_TO_BACK:
                std::stable_sort(_glyphs.begin(), _glyphs.end(), compareFrontToBack);
                break;
            case GlyphSortType::TEXTURE:
                std::stable_sort(_glyphs.begin(), _glyphs.end(), compareTexture);
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