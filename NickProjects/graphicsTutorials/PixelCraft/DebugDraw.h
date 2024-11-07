#pragma once
#include <box2d/box2d.h>
#include <Bengine/GLSLProgram.h>
#include <glm/glm.hpp>
#include <Bengine/vertex.h>
#include <vector>

class BlockManager;

struct DebugVertex {

    DebugVertex(b2Vec2 pos, float R, float G, float B, float A) :
        pos(pos.x, pos.y), color(GLubyte(R * 255.f), GLubyte(G * 255.f), GLubyte(B * 255.f), GLubyte(A * 255.f)) {
    }
    DebugVertex(b2Vec2 pos, GLubyte R, GLubyte G, GLubyte B, GLubyte A) :
        pos(pos.x, pos.y), color(R, G, B, A) {
    }

    glm::vec2 pos;
    Bengine::ColorRGBA8 color;
};

class DebugDraw {
public:
    static DebugDraw& getInstance() {
        static DebugDraw instance; // Guaranteed to be destroyed, instantiated on first use
        return instance;
    }

    DebugDraw(DebugDraw const&) = delete;
    void operator=(DebugDraw const&) = delete;
    ~DebugDraw();

    void init();
    void drawWorld(b2WorldId* world, const glm::mat4& projectionMatrix);
    void setAlpha(float alpha) { m_alpha = alpha; }

    void setVertexDataChanged(bool changed) {m_vertexDataChanged = changed; }

private:
    DebugDraw() = default;
    // Updated callback signatures to match Box2D 3.0
    static void drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context);
    static void drawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context);
    static void drawCircle(b2Vec2 center, float radius, b2HexColor color, void* context);
    static void drawSolidCircle(b2Transform xf, float radius, b2HexColor color, void* context);
    static void drawCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context);
    static void drawSolidPolygon(b2Transform xf, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context);
    static void drawTransform(b2Transform xf, void* context);
    static void drawString(b2Vec2 p, const char* s, void* context);
    static void drawPoint(b2Vec2 p, float size, b2HexColor color, void* context);
    static void setAttrib();

    float m_alpha = 0.1f;
    Bengine::GLSLProgram m_program;
    GLuint m_linesvboId = 0;
    GLuint m_linesvaoId = 0;
    GLuint m_trianglesvboId = 0;
    GLuint m_trianglesvaoId = 0;

    inline static std::vector<DebugVertex> m_lineVertexData;
    inline static std::vector<DebugVertex> m_triangleVertexData;

    bool m_vertexDataChanged = true;
};


